/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * empathy-ft-handler.h - Header for EmpathyFTHandler
 * Copyright (C) 2009 Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: Cosimo Cecchi <cosimo.cecchi@collabora.co.uk>
 */

/* empathy-ft-handler.h */

#ifndef __EMPATHY_FT_HANDLER_H__
#define __EMPATHY_FT_HANDLER_H__

#include <config_lgpl.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <telepathy-glib/contact.h>
#include <telepathy-glib/file-transfer-channel.h>

G_BEGIN_DECLS

#define EMPATHY_TYPE_FT_HANDLER empathy_ft_handler_get_type()
#define EMPATHY_FT_HANDLER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
   EMPATHY_TYPE_FT_HANDLER, EmpathyFTHandler))
#define EMPATHY_FT_HANDLER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
   EMPATHY_TYPE_FT_HANDLER, EmpathyFTHandlerClass))
#define EMPATHY_IS_FT_HANDLER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EMPATHY_TYPE_FT_HANDLER))
#define EMPATHY_IS_FT_HANDLER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), EMPATHY_TYPE_FT_HANDLER))
#define EMPATHY_FT_HANDLER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
   EMPATHY_TYPE_FT_HANDLER, EmpathyFTHandlerClass))

typedef struct _EmpathyFTHandlerPriv EmpathyFTHandlerPriv;

typedef struct {
  GObject parent;
  EmpathyFTHandlerPriv *priv;
} EmpathyFTHandler;

typedef struct {
  GObjectClass parent_class;
} EmpathyFTHandlerClass;

#define EMPATHY_FT_ERROR_QUARK g_quark_from_static_string ("EmpathyFTError")

typedef enum {
	EMPATHY_FT_ERROR_FAILED,
	EMPATHY_FT_ERROR_HASH_MISMATCH,
	EMPATHY_FT_ERROR_TP_ERROR,
	EMPATHY_FT_ERROR_SOCKET,
	EMPATHY_FT_ERROR_NOT_SUPPORTED,
	EMPATHY_FT_ERROR_INVALID_SOURCE_FILE,
	EMPATHY_FT_ERROR_EMPTY_SOURCE_FILE
} EmpathyFTErrorEnum;

/**
 * EmpathyFTHandlerReadyCallback:
 * @handler: the handler which is now ready
 * @error: a #GError if the operation failed, or %NULL
 * @user_data: user data passed to the callback
 */
typedef void (* EmpathyFTHandlerReadyCallback) (EmpathyFTHandler *handler,
    GError *error,
    gpointer user_data);

GType empathy_ft_handler_get_type (void);

/* public methods */
void empathy_ft_handler_new_outgoing (
    TpAccount *account,
    TpContact *contact,
    GFile *source,
    gint64 action_time,
    EmpathyFTHandlerReadyCallback callback,
    gpointer user_data);
void empathy_ft_handler_set_service_name (
    EmpathyFTHandler *self,
    const gchar *service_name);
void empathy_ft_handler_set_description (
    EmpathyFTHandler *self,
    const gchar *description);


void empathy_ft_handler_new_incoming (TpFileTransferChannel *channel,
    EmpathyFTHandlerReadyCallback callback,
    gpointer user_data);
void empathy_ft_handler_incoming_set_destination (EmpathyFTHandler *handler,
    GFile *destination);

void empathy_ft_handler_start_transfer (EmpathyFTHandler *handler);
void empathy_ft_handler_cancel_transfer (EmpathyFTHandler *handler);

/* properties of the transfer */
const char * empathy_ft_handler_get_filename (EmpathyFTHandler *handler);
const char * empathy_ft_handler_get_content_type (EmpathyFTHandler *handler);
TpContact * empathy_ft_handler_get_contact (EmpathyFTHandler *handler);
GFile * empathy_ft_handler_get_gfile (EmpathyFTHandler *handler);
const char *empathy_ft_handler_get_description(EmpathyFTHandler*);
gboolean empathy_ft_handler_get_use_hash (EmpathyFTHandler *handler);
gboolean empathy_ft_handler_is_incoming (EmpathyFTHandler *handler);
guint64 empathy_ft_handler_get_transferred_bytes (EmpathyFTHandler *handler);
guint64 empathy_ft_handler_get_total_bytes (EmpathyFTHandler *handler);
gboolean empathy_ft_handler_is_completed (EmpathyFTHandler *handler);
gboolean empathy_ft_handler_is_cancelled (EmpathyFTHandler *handler);

G_END_DECLS

#endif /* __EMPATHY_FT_HANDLER_H__ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
