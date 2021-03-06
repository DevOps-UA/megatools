/*
 *  megatools - Mega.nz client library and tools
 *  Copyright (C) 2013  Ondřej Jirman <megous@megous.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __OLD_MEGA_H
#define __OLD_MEGA_H

#include <glib.h>
#include <gio/gio.h>

// API error domain

#define MEGA_ERROR mega_error_quark()

enum 
{
  MEGA_ERROR_OTHER
};

// forward typedefs

typedef struct _mega_sesssion mega_session;
typedef struct _mega_node mega_node;
typedef struct _mega_share_key mega_share_key;
typedef struct _mega_user_quota mega_user_quota;
typedef struct _mega_status_data mega_status_data;
typedef struct _mega_reg_state mega_reg_state;
typedef struct _mega_download_data_params mega_download_data_params;

// status callback

enum
{
  MEGA_STATUS_PROGRESS = 1,
  MEGA_STATUS_FILEINFO,
  MEGA_STATUS_DATA
};

struct _mega_status_data
{
  gint type;

  union
  {
    struct
    {
      guint64 total;
      guint64 done;
      guint64 last;
      gint64 span;
    } progress;

    struct
    {
      const gchar* name;
      guint64 size;
    } fileinfo;

    struct
    {
      const guchar* buf;
      guint64 size;
    } data;
  };
};

typedef gboolean (*mega_status_callback)(mega_status_data* data, gpointer userdata);

// session data types

enum
{
  MEGA_NODE_FILE = 0,
  MEGA_NODE_FOLDER = 1,
  MEGA_NODE_ROOT = 2,
  MEGA_NODE_INBOX = 3,
  MEGA_NODE_TRASH = 4,
  MEGA_NODE_NETWORK = 9,
  MEGA_NODE_CONTACT = 8
};

struct _mega_share_key
{
  gchar* node_handle;
  guchar* key;
};

struct _mega_node 
{
  gchar* name;
  gchar* handle;
  gchar* parent_handle;
  gchar* user_handle;
  gchar* su_handle;
  gsize key_len;
  guchar* key;
  gint type;
  guint64 size;
  glong timestamp;

  // call addlinks after refresh to get links populated
  gchar* link;

  mega_session* s;
  mega_node* parent;
};

struct _mega_user_quota 
{
  guint64 total;
  guint64 used;
};

struct _mega_reg_state
{
  gchar* user_handle;
  guchar password_key[16];
  guchar challenge[16];
};

struct _mega_download_data_params
{
  guchar node_key[32];

  gchar* download_url;

  gchar* node_handle;
  gchar* node_name;
  gsize node_size; 
};

#define MEGA_DEBUG_API    0x01
#define MEGA_DEBUG_CACHE  0x02
#define MEGA_DEBUG_FS     0x04

extern gint mega_debug;


GQuark              mega_error_quark                (void);

mega_session*       mega_session_new                (void);
void                mega_session_free               (mega_session* s);

void                mega_session_set_speed          (mega_session* s, gint ul, gint dl);
void                mega_session_set_proxy          (mega_session* s, const gchar* proxy);
void                mega_session_set_resume         (mega_session* s, gboolean enabled);

void                mega_session_watch_status       (mega_session* s, mega_status_callback cb, gpointer userdata);
void                mega_session_enable_previews    (mega_session* s, gboolean enable);

// this has side effect of the current session being closed
gboolean            mega_session_open               (mega_session* s, const gchar* un, const gchar* pw, const gchar* sid, GError** err);
void                mega_session_close              (mega_session* s);
const gchar*        mega_session_get_sid            (mega_session* s);

gboolean            mega_session_save               (mega_session* s, GError** err);
// this has side effect of the current session being closed
gboolean            mega_session_load               (mega_session* s, const gchar* un, const gchar* pw, gint max_age, gchar** last_sid, GError** err);

gboolean            mega_session_get_user           (mega_session* s, GError** err);
gboolean            mega_session_refresh            (mega_session* s, GError** err);
gboolean            mega_session_addlinks           (mega_session* s, GSList* nodes, GError** err);
mega_user_quota*    mega_session_user_quota         (mega_session* s, GError** err);

GSList*             mega_session_ls_all             (mega_session* s);
GSList*             mega_session_ls                 (mega_session* s, const gchar* path, gboolean recursive);
GSList*             mega_session_get_node_chilren   (mega_session* s, mega_node* node);
mega_node*          mega_session_stat               (mega_session* s, const gchar* path);
mega_node*          mega_session_mkdir              (mega_session* s, const gchar* path, GError** err);
gboolean            mega_session_rm                 (mega_session* s, const gchar* path, GError** err);
mega_node*          mega_session_put                (mega_session* s, mega_node* parent_node, GFile* file, GError** err);
gchar*              mega_session_new_node_attribute (mega_session* s, const guchar* data, gsize len, const gchar* type, const guchar* key, GError** err);
gboolean            mega_session_get                (mega_session* s, GFile* file, mega_node* node, GError** err);

gboolean            mega_session_open_exp_folder    (mega_session* s, const gchar* n, const gchar* key, const gchar* specific, GError** err);
gboolean            mega_session_dl_prepare         (mega_session* s, mega_download_data_params* get_params, const gchar* handle, const gchar* key, GError** err);

gboolean            mega_session_download_data      (mega_session* s, mega_download_data_params* params, GFile* file, GError** err);
void                mega_download_data_free         (mega_download_data_params* params);

gboolean            mega_node_is_writable           (mega_session* s, mega_node* n);

gboolean            mega_node_is_container          (mega_node* n);
gchar*              mega_node_get_link              (mega_node* n, gboolean include_key);
gchar*              mega_node_get_key               (mega_node* n);
gboolean            mega_node_get_path              (mega_node* n, gchar* buf, gsize len);
gchar*              mega_node_get_path_dup          (mega_node* n);

gboolean            mega_session_register           (mega_session* s, const gchar* email, const gchar* password, const gchar* name, mega_reg_state** state, GError** err);
gboolean            mega_session_register_verify    (mega_session* s, mega_reg_state* state, const gchar* signup_key, GError** err);

// Compatibility / deprecated:

mega_node*          mega_session_put_compat         (mega_session* s, const gchar* remote_path, const gchar* local_path, GError** err);
gboolean            mega_session_get_compat         (mega_session* s, const gchar* local_path, const gchar* remote_path, GError** err);
gboolean            mega_session_dl_compat          (mega_session* s, const gchar* handle, const gchar* key, const gchar* local_path, GError** err);


#endif
