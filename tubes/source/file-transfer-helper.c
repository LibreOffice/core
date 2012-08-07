/*
 * empathy-ft-handler.c - Source for EmpathyFTHandler
 * Copyright © 2009, 2012 Collabora Ltd.
 * Copyright © 2009 Frédéric Péters
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

/* empathy-ft-handler.c */

#include <glib.h>
#include <glib/gi18n.h>
#include <telepathy-glib/account-channel-request.h>
#include <telepathy-glib/util.h>
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/interfaces.h>

#include <tubes/file-transfer-helper.h>

#define DEBUG(...)

/**
 * SECTION:empathy-ft-handler
 * @title: EmpathyFTHandler
 * @short_description: an object representing a File Transfer
 * @include: libempathy/empathy-ft-handler
 *
 * #EmpathyFTHandler is the object which represents a File Transfer with all
 * its properties.
 * The creation of an #EmpathyFTHandler is done with
 * empathy_ft_handler_new_outgoing() or empathy_ft_handler_new_incoming(),
 * even though clients should not need to call them directly, as
 * #EmpathyFTFactory does it for them. Remember that for the file transfer
 * to work with an incoming handler,
 * empathy_ft_handler_incoming_set_destination() should be called after
 * empathy_ft_handler_new_incoming(). #EmpathyFTFactory does this
 * automatically.
 * It's important to note that, as the creation of the handlers is async, once
 * an handler is created, it already has all the interesting properties set,
 * like filename, total bytes, content type and so on, making it useful
 * to be displayed in an UI.
 * The transfer API works like a state machine; it has three signals,
 * ::transfer-started, ::transfer-progress, ::transfer-done, which will be
 * emitted in the relevant phases.
 * In addition, if the handler is created with checksumming enabled,
 * other three signals (::hashing-started, ::hashing-progress, ::hashing-done)
 * will be emitted before or after the transfer, depending on the direction
 * (respectively outgoing and incoming) of the handler.
 * At any time between the call to empathy_ft_handler_start_transfer() and
 * the last signal, a ::transfer-error can be emitted, indicating that an
 * error has happened in the operation. The message of the error is localized
 * to use in an UI.
 */

G_DEFINE_TYPE (EmpathyFTHandler, empathy_ft_handler, G_TYPE_OBJECT)

#define BUFFER_SIZE 4096

enum {
  PROP_CHANNEL = 1,
  PROP_G_FILE,
  PROP_ACCOUNT,
  PROP_CONTACT,
  PROP_CONTENT_TYPE,
  PROP_DESCRIPTION,
  PROP_FILENAME,
  PROP_MODIFICATION_TIME,
  PROP_TOTAL_BYTES,
  PROP_TRANSFERRED_BYTES,
  PROP_USER_ACTION_TIME
};

enum {
  HASHING_STARTED,
  HASHING_PROGRESS,
  HASHING_DONE,
  TRANSFER_STARTED,
  TRANSFER_PROGRESS,
  TRANSFER_DONE,
  TRANSFER_ERROR,
  LAST_SIGNAL
};

typedef struct {
  GInputStream *stream;
  GError *error /* comment to make the style checker happy */;
  guchar *buffer;
  GChecksum *checksum;
  gssize total_read;
  guint64 total_bytes;
  EmpathyFTHandler *handler;
} HashingData;

typedef struct {
  EmpathyFTHandlerReadyCallback callback;
  gpointer user_data;
  EmpathyFTHandler *handler;
} CallbacksData;

/* private data */
struct _EmpathyFTHandlerPriv {
  gboolean dispose_run;

  GFile *gfile;
  TpFileTransferChannel *channel;
  GCancellable *cancellable;
  gboolean use_hash;

  /* request for the new transfer */
  GHashTable *request;

  /* transfer properties */
  TpAccount *account;
  TpContact *contact;
  gchar *content_type;
  gchar *filename;
  gchar *description;
  guint64 total_bytes;
  guint64 transferred_bytes;
  guint64 mtime;
  gchar *content_hash;
  TpFileHashType content_hash_type;
  gchar *service_name;

  gint64 user_action_time;

  /* time and speed */
  gdouble speed;
  guint remaining_time;
  gint64 last_update_time;

  gboolean is_completed;
};

static guint signals[LAST_SIGNAL] = { 0 };

static gboolean do_hash_job_incoming (GIOSchedulerJob *job,
    GCancellable *cancellable, gpointer user_data);

/* GObject implementations */
static void
do_get_property (GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec)
{
  EmpathyFTHandler *self = EMPATHY_FT_HANDLER (object);
  EmpathyFTHandlerPriv *priv = self->priv;

  switch (property_id)
    {
      case PROP_ACCOUNT:
        g_value_set_object (value, priv->account);
        break;
      case PROP_CONTACT:
        g_value_set_object (value, priv->contact);
        break;
      case PROP_CONTENT_TYPE:
        g_value_set_string (value, priv->content_type);
        break;
      case PROP_DESCRIPTION:
        g_value_set_string (value, priv->description);
        break;
      case PROP_FILENAME:
        g_value_set_string (value, priv->filename);
        break;
      case PROP_MODIFICATION_TIME:
        g_value_set_uint64 (value, priv->mtime);
        break;
      case PROP_TOTAL_BYTES:
        g_value_set_uint64 (value, priv->total_bytes);
        break;
      case PROP_TRANSFERRED_BYTES:
        g_value_set_uint64 (value, priv->transferred_bytes);
        break;
      case PROP_G_FILE:
        g_value_set_object (value, priv->gfile);
        break;
      case PROP_CHANNEL:
        g_value_set_object (value, priv->channel);
        break;
      case PROP_USER_ACTION_TIME:
        g_value_set_int64 (value, priv->user_action_time);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
do_set_property (GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec)
{
  EmpathyFTHandler *self = EMPATHY_FT_HANDLER (object);
  EmpathyFTHandlerPriv *priv = self->priv;

  switch (property_id)
    {
      case PROP_ACCOUNT:
        priv->account = g_value_dup_object (value);
        break;
      case PROP_CONTACT:
        priv->contact = g_value_dup_object (value);
        break;
      case PROP_CONTENT_TYPE:
        priv->content_type = g_value_dup_string (value);
        break;
      case PROP_DESCRIPTION:
        priv->description = g_value_dup_string (value);
        break;
      case PROP_FILENAME:
        priv->filename = g_value_dup_string (value);
        break;
      case PROP_MODIFICATION_TIME:
        priv->mtime = g_value_get_uint64 (value);
        break;
      case PROP_TOTAL_BYTES:
        priv->total_bytes = g_value_get_uint64 (value);
        break;
      case PROP_TRANSFERRED_BYTES:
        priv->transferred_bytes = g_value_get_uint64 (value);
        break;
      case PROP_G_FILE:
        priv->gfile = g_value_dup_object (value);
        break;
      case PROP_CHANNEL:
        priv->channel = g_value_dup_object (value);
        break;
      case PROP_USER_ACTION_TIME:
        priv->user_action_time = g_value_get_int64 (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
do_dispose (GObject *object)
{
  EmpathyFTHandler *self = EMPATHY_FT_HANDLER (object);
  EmpathyFTHandlerPriv *priv = self->priv;

  if (priv->dispose_run)
    return;

  priv->dispose_run = TRUE;

  if (priv->account != NULL) {
    g_object_unref (priv->account);
    priv->account = NULL;
  }

  if (priv->contact != NULL) {
    g_object_unref (priv->contact);
    priv->contact = NULL;
  }

  if (priv->gfile != NULL) {
    g_object_unref (priv->gfile);
    priv->gfile = NULL;
  }

  if (priv->channel != NULL) {
    tp_channel_close_async (TP_CHANNEL (priv->channel), NULL, NULL);
    g_object_unref (priv->channel);
    priv->channel = NULL;
  }

  if (priv->cancellable != NULL) {
    g_object_unref (priv->cancellable);
    priv->cancellable = NULL;
  }

  if (priv->request != NULL)
    {
      g_hash_table_unref (priv->request);
      priv->request = NULL;
    }

  G_OBJECT_CLASS (empathy_ft_handler_parent_class)->dispose (object);
}

static void
do_finalize (GObject *object)
{
  EmpathyFTHandler *self = EMPATHY_FT_HANDLER (object);
  EmpathyFTHandlerPriv *priv = self->priv;

  DEBUG ("%p", object);

  g_free (priv->content_type);
  priv->content_type = NULL;

  g_free (priv->filename);
  priv->filename = NULL;

  g_free (priv->description);
  priv->description = NULL;

  g_free (priv->content_hash);
  priv->content_hash = NULL;

  g_free (priv->service_name);
  priv->service_name = NULL;

  G_OBJECT_CLASS (empathy_ft_handler_parent_class)->finalize (object);
}

static void
empathy_ft_handler_class_init (EmpathyFTHandlerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *param_spec;

  g_type_class_add_private (klass, sizeof (EmpathyFTHandlerPriv));

  object_class->get_property = do_get_property;
  object_class->set_property = do_set_property;
  object_class->dispose = do_dispose;
  object_class->finalize = do_finalize;

  /* properties */

  /**
   * EmpathyFTHandler:account:
   *
   * The local #TpAccount for the file transfer
   */
  param_spec = g_param_spec_object ("account",
    "account", "The remote account",
    TP_TYPE_ACCOUNT,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_ACCOUNT, param_spec);

  /**
   * EmpathyFTHandler:contact:
   *
   * The remote #TpContact for the transfer
   */
  param_spec = g_param_spec_object ("contact",
    "contact", "The remote contact",
    TP_TYPE_CONTACT,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CONTACT, param_spec);

  /**
   * EmpathyFTHandler:content-type:
   *
   * The content type of the file being transferred
   */
  param_spec = g_param_spec_string ("content-type",
    "content-type", "The content type of the file", NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class,
      PROP_CONTENT_TYPE, param_spec);

  /**
   * EmpathyFTHandler:description:
   *
   * The description of the file being transferred
   */
  param_spec = g_param_spec_string ("description",
    "description", "The description of the file", NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class,
      PROP_DESCRIPTION, param_spec);

  /**
   * EmpathyFTHandler:filename:
   *
   * The name of the file being transferred
   */
  param_spec = g_param_spec_string ("filename",
    "filename", "The name of the file", NULL,
    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class,
      PROP_FILENAME, param_spec);

  /**
   * EmpathyFTHandler:modification-time:
   *
   * The modification time of the file being transferred
   */
  param_spec = g_param_spec_uint64 ("modification-time",
    "modification-time", "The mtime of the file", 0,
    G_MAXUINT64, 0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class,
      PROP_MODIFICATION_TIME, param_spec);

  /**
   * EmpathyFTHandler:total-bytes:
   *
   * The size (in bytes) of the file being transferred
   */
  param_spec = g_param_spec_uint64 ("total-bytes",
    "total-bytes", "The size of the file", 0,
    G_MAXUINT64, 0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class,
      PROP_TOTAL_BYTES, param_spec);

  /**
   * EmpathyFTHandler:transferred-bytes:
   *
   * The number of the bytes already transferred
   */
  param_spec = g_param_spec_uint64 ("transferred-bytes",
    "transferred-bytes", "The number of bytes already transferred", 0,
    G_MAXUINT64, 0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class,
      PROP_TRANSFERRED_BYTES, param_spec);

  /**
   * EmpathyFTHandler:gfile:
   *
   * The #GFile object where the transfer actually happens
   */
  param_spec = g_param_spec_object ("gfile",
    "gfile", "The GFile we're handling",
    G_TYPE_FILE,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_G_FILE, param_spec);

  /**
   * EmpathyFTHandler:channel:
   *
   * The underlying #TpFileTransferChannel managing the transfer
   */
  param_spec = g_param_spec_object ("channel",
    "channel", "The file transfer channel",
    TP_TYPE_FILE_TRANSFER_CHANNEL,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CHANNEL, param_spec);

  param_spec = g_param_spec_int64 ("user-action-time", "user action time",
    "User action time",
    0, G_MAXINT64, 0,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_USER_ACTION_TIME,
      param_spec);

  /* signals */

  /**
   * EmpathyFTHandler::transfer-started
   * @handler: the object which has received the signal
   * @channel: the #TpFileTransferChannel for which the transfer has started
   *
   * This signal is emitted when the actual transfer starts.
   */
  signals[TRANSFER_STARTED] =
    g_signal_new ("transfer-started", G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_generic,
        G_TYPE_NONE,
        1, TP_TYPE_FILE_TRANSFER_CHANNEL);

  /**
   * EmpathyFTHandler::transfer-done
   * @handler: the object which has received the signal
   * @channel: the #TpFileTransferChannel for which the transfer has started
   *
   * This signal will be emitted when the actual transfer is completed
   * successfully.
   */
  signals[TRANSFER_DONE] =
    g_signal_new ("transfer-done", G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_generic,
        G_TYPE_NONE,
        1, TP_TYPE_FILE_TRANSFER_CHANNEL);

  /**
   * EmpathyFTHandler::transfer-error
   * @handler: the object which has received the signal
   * @error: a #GError
   *
   * This signal can be emitted anytime between the call to
   * empathy_ft_handler_start_transfer() and the last expected signal
   * (::transfer-done or ::hashing-done), and it's guaranteed to be the last
   * signal coming from the handler, meaning that no other operation will
   * take place after this signal.
   */
  signals[TRANSFER_ERROR] =
    g_signal_new ("transfer-error", G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_generic,
        G_TYPE_NONE,
        1, G_TYPE_POINTER);

  /**
   * EmpathyFTHandler::transfer-progress
   * @handler: the object which has received the signal
   * @current_bytes: the bytes currently transferred
   * @total_bytes: the total bytes of the handler
   * @remaining_time: the number of seconds remaining for the transfer
   * to be completed
   * @speed: the current speed of the transfer (in KB/s)
   *
   * This signal is emitted to notify clients of the progress of the
   * transfer.
   */
  signals[TRANSFER_PROGRESS] =
    g_signal_new ("transfer-progress", G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_generic,
        G_TYPE_NONE,
        4, G_TYPE_UINT64, G_TYPE_UINT64, G_TYPE_UINT, G_TYPE_DOUBLE);

  /**
   * EmpathyFTHandler::hashing-started
   * @handler: the object which has received the signal
   *
   * This signal is emitted when the hashing operation of the handler
   * is started. Note that this might happen or not, depending on the CM
   * and remote contact capabilities. Clients shoud use
   * empathy_ft_handler_get_use_hash() before calling
   * empathy_ft_handler_start_transfer() to know whether they should connect
   * to this signal.
   */
  signals[HASHING_STARTED] =
    g_signal_new ("hashing-started", G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_generic,
        G_TYPE_NONE, 0);

  /**
   * EmpathyFTHandler::hashing-progress
   * @handler: the object which has received the signal
   * @current_bytes: the bytes currently hashed
   * @total_bytes: the total bytes of the handler
   *
   * This signal is emitted to notify clients of the progress of the
   * hashing operation.
   */
  signals[HASHING_PROGRESS] =
    g_signal_new ("hashing-progress", G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_generic,
        G_TYPE_NONE,
        2, G_TYPE_UINT64, G_TYPE_UINT64);

  /**
   * EmpathyFTHandler::hashing-done
   * @handler: the object which has received the signal
   *
   * This signal is emitted when the hashing operation of the handler
   * is completed.
   */
  signals[HASHING_DONE] =
    g_signal_new ("hashing-done", G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_LAST, 0, NULL, NULL,
        g_cclosure_marshal_generic,
        G_TYPE_NONE, 0);
}

static void
empathy_ft_handler_init (EmpathyFTHandler *self)
{
  EmpathyFTHandlerPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
    EMPATHY_TYPE_FT_HANDLER, EmpathyFTHandlerPriv);

  self->priv = priv;
  priv->cancellable = g_cancellable_new ();
}

/* private functions */

static void
hash_data_free (HashingData *data)
{
  g_free (data->buffer);

  if (data->stream != NULL)
    g_object_unref (data->stream);

  if (data->checksum != NULL)
    g_checksum_free (data->checksum);

  if (data->error != NULL)
    g_error_free (data->error);

  if (data->handler != NULL)
    g_object_unref (data->handler);

  g_slice_free (HashingData, data);
}

static GChecksumType
tp_file_hash_to_g_checksum (TpFileHashType type)
{
  GChecksumType retval;

  switch (type)
    {
      case TP_FILE_HASH_TYPE_MD5:
        retval = G_CHECKSUM_MD5;
        break;
      case TP_FILE_HASH_TYPE_SHA1:
        retval = G_CHECKSUM_SHA1;
        break;
      case TP_FILE_HASH_TYPE_SHA256:
        retval = G_CHECKSUM_SHA256;
        break;
      case TP_FILE_HASH_TYPE_NONE:
      default:
        g_assert_not_reached ();
        break;
    }

  return retval;
}

static void
check_hash_incoming (EmpathyFTHandler *handler)
{
  HashingData *hash_data;
  EmpathyFTHandlerPriv *priv = handler->priv;

  if (!tp_str_empty (priv->content_hash))
    {
      hash_data = g_slice_new0 (HashingData);
      hash_data->total_bytes = priv->total_bytes;
      hash_data->handler = g_object_ref (handler);
      hash_data->checksum = g_checksum_new
        (tp_file_hash_to_g_checksum (priv->content_hash_type));

      g_signal_emit (handler, signals[HASHING_STARTED], 0);

      g_io_scheduler_push_job (do_hash_job_incoming, hash_data, NULL,
                               G_PRIORITY_DEFAULT, priv->cancellable);
    }
}

static void
emit_error_signal (EmpathyFTHandler *handler,
    const GError *error)
{
  EmpathyFTHandlerPriv *priv = handler->priv;

  DEBUG ("Error in transfer: %s\n", error->message);

  if (!g_cancellable_is_cancelled (priv->cancellable))
    g_cancellable_cancel (priv->cancellable);

  g_signal_emit (handler, signals[TRANSFER_ERROR], 0, error);
}

static gint64
time_get_current (void)
{
  GDateTime *now;
  gint64 result;

  now = g_date_time_new_now_utc ();
  result = g_date_time_to_unix (now);
  g_date_time_unref (now);

  return result;
}

static void
update_remaining_time_and_speed (EmpathyFTHandler *handler,
    guint64 transferred_bytes)
{
  EmpathyFTHandlerPriv *priv = handler->priv;
  gint64 elapsed_time, current_time;
  guint64 transferred, last_transferred_bytes;
  gdouble speed;
  gint remaining_time;

  last_transferred_bytes = priv->transferred_bytes;
  priv->transferred_bytes = transferred_bytes;

  current_time = time_get_current ();
  elapsed_time = current_time - priv->last_update_time;

  if (elapsed_time >= 1)
    {
      transferred = transferred_bytes - last_transferred_bytes;
      speed = (gdouble) transferred / (gdouble) elapsed_time;
      remaining_time = (priv->total_bytes - priv->transferred_bytes) / speed;
      priv->speed = speed;
      priv->remaining_time = remaining_time;
      priv->last_update_time = current_time;
    }
}

static void
ft_transfer_transferred_bytes_cb (TpFileTransferChannel *channel,
    GParamSpec *pspec,
    EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv = handler->priv;
  guint64 bytes;

  (void)pspec;      /* suppress unused-parameter warning */

  if (empathy_ft_handler_is_cancelled (handler))
    return;

  bytes = tp_file_transfer_channel_get_transferred_bytes (channel);

  if (priv->transferred_bytes == 0)
    {
      priv->last_update_time = time_get_current ();
      g_signal_emit (handler, signals[TRANSFER_STARTED], 0, channel);
    }

  if (priv->transferred_bytes != bytes)
    {
      update_remaining_time_and_speed (handler, bytes);

      g_signal_emit (handler, signals[TRANSFER_PROGRESS], 0,
          bytes, priv->total_bytes, priv->remaining_time,
          priv->speed);
    }
}

static void
ft_transfer_provide_cb (GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  TpFileTransferChannel *channel = TP_FILE_TRANSFER_CHANNEL (source);
  EmpathyFTHandler *handler = user_data;
  GError *error = NULL;

  if (!tp_file_transfer_channel_provide_file_finish (channel, result, &error))
    {
      emit_error_signal (handler, error);
      g_clear_error (&error);
    }
}

static void
ft_transfer_accept_cb (GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  TpFileTransferChannel *channel = TP_FILE_TRANSFER_CHANNEL (source);
  EmpathyFTHandler *handler = user_data;
  GError *error = NULL;

  if (!tp_file_transfer_channel_accept_file_finish (channel, result, &error))
    {
      emit_error_signal (handler, error);
      g_clear_error (&error);
    }
}

static GError *
error_from_state_change_reason (TpFileTransferStateChangeReason reason)
{
  const char *string;
  GError *retval = NULL;

  string = NULL;

  switch (reason)
    {
      case TP_FILE_TRANSFER_STATE_CHANGE_REASON_NONE:
        string = _("No reason was specified");
        break;
      case TP_FILE_TRANSFER_STATE_CHANGE_REASON_REQUESTED:
        string = _("The change in state was requested");
        break;
      case TP_FILE_TRANSFER_STATE_CHANGE_REASON_LOCAL_STOPPED:
        string = _("You canceled the file transfer");
        break;
      case TP_FILE_TRANSFER_STATE_CHANGE_REASON_REMOTE_STOPPED:
        string = _("The other participant canceled the file transfer");
        break;
      case TP_FILE_TRANSFER_STATE_CHANGE_REASON_LOCAL_ERROR:
        string = _("Error while trying to transfer the file");
        break;
      case TP_FILE_TRANSFER_STATE_CHANGE_REASON_REMOTE_ERROR:
        string = _("The other participant is unable to transfer the file");
        break;
      default:
        string = _("Unknown reason");
        break;
    }

  retval = g_error_new_literal (EMPATHY_FT_ERROR_QUARK,
      EMPATHY_FT_ERROR_TP_ERROR, string);

  return retval;
}

static void
ft_transfer_state_cb (TpFileTransferChannel *channel,
    GParamSpec *pspec,
    EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv = handler->priv;
  TpFileTransferStateChangeReason reason;
  TpFileTransferState state = tp_file_transfer_channel_get_state (
      channel, &reason);

  (void)pspec;      /* suppress unused-parameter warning */

  if (state == TP_FILE_TRANSFER_STATE_COMPLETED)
    {
      priv->is_completed = TRUE;
      g_signal_emit (handler, signals[TRANSFER_DONE], 0, channel);

      tp_channel_close_async (TP_CHANNEL (channel), NULL, NULL);

      if (empathy_ft_handler_is_incoming (handler) && priv->use_hash)
        {
          check_hash_incoming (handler);
        }
    }
  else if (state == TP_FILE_TRANSFER_STATE_CANCELLED)
    {
      GError *error = error_from_state_change_reason (reason);
      emit_error_signal (handler, error);
      g_clear_error (&error);
    }
}

static void
ft_handler_create_channel_cb (GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  EmpathyFTHandler *handler = user_data;
  EmpathyFTHandlerPriv *priv = handler->priv;
  GError *error = NULL;
  TpChannel *channel;

  DEBUG ("Dispatcher create channel CB");

  channel = tp_account_channel_request_create_and_handle_channel_finish (
        TP_ACCOUNT_CHANNEL_REQUEST (source), result, NULL, &error);

  if (channel == NULL)
    DEBUG ("Failed to request FT channel: %s", error->message);
  else
    g_cancellable_set_error_if_cancelled (priv->cancellable, &error);

  if (error != NULL)
    {
      emit_error_signal (handler, error);

      g_clear_object (&channel);
      g_error_free (error);
      return;
    }

  priv->channel = TP_FILE_TRANSFER_CHANNEL (channel);

  tp_g_signal_connect_object (priv->channel, "notify::state",
      G_CALLBACK (ft_transfer_state_cb), handler, 0);
  tp_g_signal_connect_object (priv->channel, "notify::transferred-bytes",
      G_CALLBACK (ft_transfer_transferred_bytes_cb), handler, 0);

  tp_file_transfer_channel_provide_file_async (priv->channel, priv->gfile,
      ft_transfer_provide_cb, handler);
}

static void
ft_handler_push_to_dispatcher (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv = handler->priv;
  TpAccountChannelRequest *req;

  DEBUG ("Pushing request to the dispatcher");

  req = tp_account_channel_request_new (priv->account, priv->request,
      priv->user_action_time);

  tp_account_channel_request_create_and_handle_channel_async (req, NULL,
      ft_handler_create_channel_cb, handler);

  g_object_unref (req);
}

static void
ft_handler_populate_outgoing_request (EmpathyFTHandler *handler)
{
  guint contact_handle;
  EmpathyFTHandlerPriv *priv = handler->priv;
  gchar *uri;

  contact_handle = tp_contact_get_handle (priv->contact);
  uri = g_file_get_uri (priv->gfile);

  priv->request = tp_asv_new (
      TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING,
        TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,
      TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT,
        TP_HANDLE_TYPE_CONTACT,
      TP_PROP_CHANNEL_TARGET_HANDLE, G_TYPE_UINT,
        contact_handle,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_CONTENT_TYPE, G_TYPE_STRING,
        priv->content_type,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_FILENAME, G_TYPE_STRING,
        priv->filename,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_SIZE, G_TYPE_UINT64,
        priv->total_bytes,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_DATE, G_TYPE_UINT64,
        priv->mtime,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_URI, G_TYPE_STRING, uri,
      NULL);

  if (priv->service_name != NULL)
    tp_asv_set_string (priv->request, TP_PROP_CHANNEL_INTERFACE_FILE_TRANSFER_METADATA_SERVICE_NAME, priv->service_name);

  g_free (uri);
}

static gboolean
hash_job_done (gpointer user_data)
{
  HashingData *hash_data = user_data;
  EmpathyFTHandler *handler = hash_data->handler;
  EmpathyFTHandlerPriv *priv;
  GError *error = NULL;

  DEBUG ("Closing stream after hashing.");

  priv = handler->priv;

  if (hash_data->error != NULL)
    {
      error = hash_data->error;
      hash_data->error = NULL;
      goto cleanup;
    }

  DEBUG ("Got file hash %s", g_checksum_get_string (hash_data->checksum));

  if (empathy_ft_handler_is_incoming (handler))
    {
      if (g_strcmp0 (g_checksum_get_string (hash_data->checksum),
                     priv->content_hash))
        {
          DEBUG ("Hash mismatch when checking incoming handler: "
                 "received %s, calculated %s", priv->content_hash,
                 g_checksum_get_string (hash_data->checksum));

          error = g_error_new_literal (EMPATHY_FT_ERROR_QUARK,
              EMPATHY_FT_ERROR_HASH_MISMATCH,
              _("File transfer completed, but the file was corrupted"));
          goto cleanup;
        }
      else
        {
          DEBUG ("Hash verification matched, received %s, calculated %s",
                 priv->content_hash,
                 g_checksum_get_string (hash_data->checksum));
        }
    }
  else
    {
      /* set the checksum in the request...
       * org.freedesktop.Telepathy.Channel.Type.FileTransfer.ContentHash
       */
      tp_asv_set_string (priv->request,
          TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_CONTENT_HASH,
          g_checksum_get_string (hash_data->checksum));
    }

cleanup:

  if (error != NULL)
    {
      emit_error_signal (handler, error);
      g_clear_error (&error);
    }
  else
    {
      g_signal_emit (handler, signals[HASHING_DONE], 0);

      if (!empathy_ft_handler_is_incoming (handler))
        /* the request is complete now, push it to the dispatcher */
        ft_handler_push_to_dispatcher (handler);
    }

  hash_data_free (hash_data);

  return FALSE;
}

static gboolean
emit_hashing_progress (gpointer user_data)
{
  HashingData *hash_data = user_data;

  g_signal_emit (hash_data->handler, signals[HASHING_PROGRESS], 0,
      (guint64) hash_data->total_read, (guint64) hash_data->total_bytes);

  return FALSE;
}

static gboolean
do_hash_job (GIOSchedulerJob *job,
    GCancellable *cancellable,
    gpointer user_data)
{
  HashingData *hash_data = user_data;
  gssize bytes_read;
  GError *error = NULL;

again:
  if (hash_data->buffer == NULL)
    hash_data->buffer = g_malloc0 (BUFFER_SIZE);

  bytes_read = g_input_stream_read (hash_data->stream, hash_data->buffer,
                                    BUFFER_SIZE, cancellable, &error);
  if (error != NULL)
    goto out;

  hash_data->total_read += bytes_read;

  /* we now have the chunk */
  if (bytes_read > 0)
    {
      g_checksum_update (hash_data->checksum, hash_data->buffer, bytes_read);
      g_io_scheduler_job_send_to_mainloop_async (job, emit_hashing_progress,
          hash_data, NULL);

      g_free (hash_data->buffer);
      hash_data->buffer = NULL;

      goto again;
    }
  else
  {
    g_input_stream_close (hash_data->stream, cancellable, &error);
  }

out:
  if (error != NULL)
    hash_data->error = error;

  g_io_scheduler_job_send_to_mainloop_async (job, hash_job_done,
      hash_data, NULL);

  return FALSE;
}

static gboolean
do_hash_job_incoming (GIOSchedulerJob *job,
    GCancellable *cancellable,
    gpointer user_data)
{
  HashingData *hash_data = user_data;
  EmpathyFTHandler *handler = hash_data->handler;
  EmpathyFTHandlerPriv *priv = handler->priv;
  GError *error = NULL;

  DEBUG ("checking integrity for incoming handler");

  /* need to get the stream first */
  hash_data->stream =
    G_INPUT_STREAM (g_file_read (priv->gfile, cancellable, &error));

  if (error != NULL)
    {
      hash_data->error = error;
      g_io_scheduler_job_send_to_mainloop_async (job, hash_job_done,
          hash_data, NULL);
      return FALSE;
    }

  return do_hash_job (job, cancellable, user_data);
}

static void
ft_handler_read_async_cb (GObject *source,
    GAsyncResult *res,
    gpointer user_data)
{
  GFileInputStream *stream;
  GError *error = NULL;
  HashingData *hash_data;
  EmpathyFTHandler *handler = user_data;
  EmpathyFTHandlerPriv *priv = handler->priv;

  (void)source;     /* suppress unused-parameter warning */

  DEBUG ("GFile read async CB.");

  stream = g_file_read_finish (priv->gfile, res, &error);
  if (error != NULL)
    {
      emit_error_signal (handler, error);
      g_clear_error (&error);

      return;
    }

  hash_data = g_slice_new0 (HashingData);
  hash_data->stream = G_INPUT_STREAM (stream);
  hash_data->total_bytes = priv->total_bytes;
  hash_data->handler = g_object_ref (handler);
  /* FIXME: MD5 is the only ContentHashType supported right now */
  hash_data->checksum = g_checksum_new (G_CHECKSUM_MD5);

  tp_asv_set_uint32 (priv->request,
      TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_CONTENT_HASH_TYPE,
      TP_FILE_HASH_TYPE_MD5);

  g_signal_emit (handler, signals[HASHING_STARTED], 0);

  g_io_scheduler_push_job (do_hash_job, hash_data, NULL,
      G_PRIORITY_DEFAULT, priv->cancellable);
}

static void
callbacks_data_free (gpointer user_data)
{
  CallbacksData *data = user_data;

  if (data->handler != NULL)
    g_object_unref (data->handler);

  g_slice_free (CallbacksData, data);
}

static gint
cmp_uint (
    gconstpointer a,
    gconstpointer b)
{
  return *(guint *) a - *(guint *) b;
}

static gboolean
set_content_hash_type_from_classes (EmpathyFTHandler *handler,
    GPtrArray *classes)
{
  GArray *possible_values;
  guint value;
  gboolean valid;
  EmpathyFTHandlerPriv *priv = handler->priv;
  gboolean support_ft = FALSE;
  guint i;

  possible_values = g_array_new (TRUE, TRUE, sizeof (guint));

  for (i = 0; i < classes->len; i++)
    {
      GHashTable *fixed;
      GStrv allowed;
      const gchar *chan_type;

      tp_value_array_unpack (g_ptr_array_index (classes, i), 2,
          &fixed, &allowed);

      chan_type = tp_asv_get_string (fixed, TP_PROP_CHANNEL_CHANNEL_TYPE);

      if (tp_strdiff (chan_type, TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER))
        continue;

      if (tp_asv_get_uint32 (fixed, TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, NULL) !=
          TP_HANDLE_TYPE_CONTACT)
        continue;

      support_ft = TRUE;

      value = tp_asv_get_uint32
        (fixed, TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_CONTENT_HASH_TYPE,
         &valid);

      if (valid)
        g_array_append_val (possible_values, value);
    }

  if (!support_ft)
    {
      g_array_unref (possible_values);
      return FALSE;
    }

  if (possible_values->len == 0)
    {
      /* there are no channel classes with hash support, disable it. */
      priv->use_hash = FALSE;
      priv->content_hash_type = TP_FILE_HASH_TYPE_NONE;

      goto out;
    }

  priv->use_hash = TRUE;

  if (possible_values->len == 1)
    {
      priv->content_hash_type = g_array_index (possible_values, guint, 0);
    }
  else
    {
      /* order the array and pick the first non zero, so that MD5
       * is the preferred value.
       */
      g_array_sort (possible_values, cmp_uint);

      if (g_array_index (possible_values, guint, 0) == 0)
        priv->content_hash_type = g_array_index (possible_values, guint, 1);
      else
        priv->content_hash_type = g_array_index (possible_values, guint, 0);
    }

out:
  g_array_unref (possible_values);

  DEBUG ("Hash enabled %s; setting content hash type as %u",
         priv->use_hash ? "True" : "False", priv->content_hash_type);

  return TRUE;
}

static void
check_hashing (CallbacksData *data)
{
  EmpathyFTHandler *handler = data->handler;
  EmpathyFTHandlerPriv *priv = handler->priv;
  GError *myerr = NULL;
  TpCapabilities *caps;
  GPtrArray *classes;
  TpConnection *conn;

  conn = tp_account_get_connection (priv->account);

  caps = tp_connection_get_capabilities (conn);
  if (caps == NULL)
    {
      data->callback (handler, NULL, data->user_data);
      goto out;
    }

  classes = tp_capabilities_get_channel_classes (caps);

  /* set whether we support hash and the type of it */
  if (!set_content_hash_type_from_classes (handler, classes))
    {
      g_set_error_literal (&myerr, EMPATHY_FT_ERROR_QUARK,
          EMPATHY_FT_ERROR_NOT_SUPPORTED,
          _("File transfer not supported by remote contact"));

      if (!g_cancellable_is_cancelled (priv->cancellable))
        g_cancellable_cancel (priv->cancellable);

      data->callback (handler, myerr, data->user_data);
      g_clear_error (&myerr);
    }
  else
    {
      /* get back to the caller now */
      data->callback (handler, NULL, data->user_data);
    }

out:
  callbacks_data_free (data);
}

static void
ft_handler_complete_request (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv = handler->priv;

  /* populate the request table with all the known properties */
  ft_handler_populate_outgoing_request (handler);

  if (priv->use_hash)
    /* start hashing the file */
    g_file_read_async (priv->gfile, G_PRIORITY_DEFAULT,
        priv->cancellable, ft_handler_read_async_cb, handler);
  else
    /* push directly the handler to the dispatcher */
    ft_handler_push_to_dispatcher (handler);
}

static void
ft_handler_gfile_ready_cb (GObject *source,
    GAsyncResult *res,
    CallbacksData *cb_data)
{
  GFileInfo *info;
  GError *error = NULL;
  GTimeVal mtime;
  EmpathyFTHandlerPriv *priv = cb_data->handler->priv;

  (void)source;     /* suppress unused-parameter warning */

  DEBUG ("Got GFileInfo.");

  info = g_file_query_info_finish (priv->gfile, res, &error);

  if (error != NULL)
    goto out;

  if (g_file_info_get_file_type (info) != G_FILE_TYPE_REGULAR)
    {
      error = g_error_new_literal (EMPATHY_FT_ERROR_QUARK,
          EMPATHY_FT_ERROR_INVALID_SOURCE_FILE,
          _("The selected file is not a regular file"));
      goto out;
    }

  priv->total_bytes = g_file_info_get_size (info);
  if (priv->total_bytes == 0)
    {
      error = g_error_new_literal (EMPATHY_FT_ERROR_QUARK,
          EMPATHY_FT_ERROR_EMPTY_SOURCE_FILE,
          _("The selected file is empty"));
      goto out;
    }

  priv->content_type = g_strdup (g_file_info_get_content_type (info));
  priv->filename = g_strdup (g_file_info_get_display_name (info));
  g_file_info_get_modification_time (info, &mtime);
  priv->mtime = mtime.tv_sec;
  priv->transferred_bytes = 0;
  priv->description = NULL;

  g_object_unref (info);

out:
  if (error != NULL)
    {
      if (!g_cancellable_is_cancelled (priv->cancellable))
        g_cancellable_cancel (priv->cancellable);

      cb_data->callback (cb_data->handler, error, cb_data->user_data);
      g_error_free (error);

      callbacks_data_free (cb_data);
    }
  else
    {
      /* see if FT/hashing are allowed */
      check_hashing (cb_data);
    }
}

static void
channel_prepared_cb (
    GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  TpFileTransferChannel *channel = TP_FILE_TRANSFER_CHANNEL (source);
  CallbacksData *cb_data = user_data;
  EmpathyFTHandler *handler = cb_data->handler;
  EmpathyFTHandlerPriv *priv = handler->priv;
  GHashTable *properties;
  GError *error = NULL;

  if (!tp_proxy_prepare_finish (channel, result, &error))
    {
      if (!g_cancellable_is_cancelled (priv->cancellable))
        g_cancellable_cancel (priv->cancellable);

      cb_data->callback (handler, error, cb_data->user_data);
      g_clear_error (&error);
      callbacks_data_free (cb_data);
      return;
    }

  properties = tp_channel_borrow_immutable_properties (TP_CHANNEL (channel));

  priv->content_hash = g_strdup (
      tp_asv_get_string (properties, "ContentHash"));

  priv->content_hash_type = tp_asv_get_uint32 (
      properties, "ContentHashType", NULL);

  priv->contact = g_object_ref (tp_channel_get_target_contact (TP_CHANNEL (channel)));

  cb_data->callback (handler, NULL, cb_data->user_data);
}


/* public methods */

/**
 * empathy_ft_handler_new_outgoing:
 * @account: the #TpAccount to send @source to
 * @contact: the #TpContact to send @source to
 * @source: the #GFile to send
 * @callback: callback to be called when the handler has been created
 * @user_data: user data to be passed to @callback
 *
 * Triggers the creation of a new #EmpathyFTHandler for an outgoing transfer.
 */
void
empathy_ft_handler_new_outgoing (
    TpAccount *account,
    TpContact *contact,
    GFile *source,
    gint64 action_time,
    EmpathyFTHandlerReadyCallback callback,
    gpointer user_data)
{
  EmpathyFTHandler *handler;
  CallbacksData *data;
  EmpathyFTHandlerPriv *priv;

  DEBUG ("New handler outgoing");

  g_return_if_fail (TP_IS_ACCOUNT (account));
  g_return_if_fail (TP_IS_CONTACT (contact));
  g_return_if_fail (G_IS_FILE (source));

  handler = g_object_new (EMPATHY_TYPE_FT_HANDLER,
      "account", account,
      "contact", contact,
      "gfile", source,
      "user-action-time", action_time,
      NULL);

  priv = handler->priv;

  data = g_slice_new0 (CallbacksData);
  data->callback = callback;
  data->user_data = user_data;
  data->handler = g_object_ref (handler);

  /* start collecting info about the file */
  g_file_query_info_async (priv->gfile,
      G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
      G_FILE_ATTRIBUTE_STANDARD_SIZE ","
      G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
      G_FILE_ATTRIBUTE_STANDARD_TYPE ","
      G_FILE_ATTRIBUTE_TIME_MODIFIED,
      G_FILE_QUERY_INFO_NONE, G_PRIORITY_DEFAULT,
      NULL, (GAsyncReadyCallback) ft_handler_gfile_ready_cb, data);
}

void
empathy_ft_handler_set_service_name (
    EmpathyFTHandler *self,
    const gchar *service_name)
{
  g_free (self->priv->service_name);
  self->priv->service_name = g_strdup (service_name);
}

/**
 * empathy_ft_handler_new_incoming:
 * @channel: the #TpFileTransferChannel proxy to the incoming channel
 * @callback: callback to be called when the handler has been created
 * @user_data: user data to be passed to @callback
 *
 * Triggers the creation of a new #EmpathyFTHandler for an incoming transfer.
 * Note that for the handler to be useful, you will have to set a destination
 * file with empathy_ft_handler_incoming_set_destination() after the handler
 * is ready.
 */
void
empathy_ft_handler_new_incoming (TpFileTransferChannel *channel,
    EmpathyFTHandlerReadyCallback callback,
    gpointer user_data)
{
  EmpathyFTHandler *handler;
  CallbacksData *data;
  EmpathyFTHandlerPriv *priv;
  GQuark features[] = { TP_CHANNEL_FEATURE_CONTACTS, 0 };

  g_return_if_fail (TP_IS_FILE_TRANSFER_CHANNEL (channel));

  handler = g_object_new (EMPATHY_TYPE_FT_HANDLER,
      "channel", channel, NULL);

  priv = handler->priv;

  data = g_slice_new0 (CallbacksData);
  data->callback = callback;
  data->user_data = user_data;
  data->handler = g_object_ref (handler);

  priv->total_bytes = tp_file_transfer_channel_get_size (channel);

  priv->transferred_bytes = tp_file_transfer_channel_get_transferred_bytes (
      channel);

  priv->filename = g_strdup (tp_file_transfer_channel_get_filename (channel));

  priv->content_type = g_strdup (tp_file_transfer_channel_get_mime_type (
      channel));

  priv->description = g_strdup (tp_file_transfer_channel_get_description (
      channel));

  tp_proxy_prepare_async (channel, features,
      channel_prepared_cb, data);
}

/**
 * empathy_ft_handler_start_transfer:
 * @handler: an #EmpathyFTHandler
 *
 * Starts the transfer machinery. After this call, the transfer and hashing
 * signals will be emitted by the handler.
 */
void
empathy_ft_handler_start_transfer (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_if_fail (EMPATHY_IS_FT_HANDLER (handler));

  priv = handler->priv;

  if (priv->channel == NULL)
    {
      ft_handler_complete_request (handler);
    }
  else
    {
      /* TODO: add support for resume. */
      tp_file_transfer_channel_accept_file_async (priv->channel,
          priv->gfile, 0, ft_transfer_accept_cb, handler);

      tp_g_signal_connect_object (priv->channel, "notify::state",
          G_CALLBACK (ft_transfer_state_cb), handler, 0);
      tp_g_signal_connect_object (priv->channel, "notify::transferred-bytes",
          G_CALLBACK (ft_transfer_transferred_bytes_cb), handler, 0);
    }
}

/**
 * empathy_ft_handler_cancel_transfer:
 * @handler: an #EmpathyFTHandler
 *
 * Cancels an ongoing handler operation. Note that this doesn't destroy
 * the object, which will keep all the properties, altough it won't be able
 * to do any more I/O.
 */
void
empathy_ft_handler_cancel_transfer (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_if_fail (EMPATHY_IS_FT_HANDLER (handler));

  priv = handler->priv;

  /* if we don't have a channel, we are hashing, so
   * we can just cancel the GCancellable to stop it.
   */
  if (priv->channel == NULL)
    g_cancellable_cancel (priv->cancellable);
  else
    tp_channel_close_async (TP_CHANNEL (priv->channel), NULL, NULL);
}

/**
 * empathy_ft_handler_incoming_set_destination:
 * @handler: an #EmpathyFTHandler
 * @destination: the #GFile where the transfer should be saved
 *
 * Sets the destination of the incoming handler to be @destination.
 * Note that calling this method is mandatory before starting the transfer
 * for incoming handlers.
 */
void
empathy_ft_handler_incoming_set_destination (EmpathyFTHandler *handler,
    GFile *destination)
{
  EmpathyFTHandlerPriv *priv;

  g_return_if_fail (EMPATHY_IS_FT_HANDLER (handler));
  g_return_if_fail (G_IS_FILE (destination));

  priv = handler->priv;

  g_object_set (handler, "gfile", destination, NULL);

  /* check if hash is supported. if it isn't, set use_hash to FALSE
   * anyway, so that clients won't be expecting us to checksum.
   */
  if (tp_str_empty (priv->content_hash) ||
      priv->content_hash_type == TP_FILE_HASH_TYPE_NONE)
    priv->use_hash = FALSE;
  else
    priv->use_hash = TRUE;
}

/**
 * empathy_ft_handler_get_filename:
 * @handler: an #EmpathyFTHandler
 *
 * Returns the name of the file being transferred.
 *
 * Return value: the name of the file being transferred
 */
const char *
empathy_ft_handler_get_filename (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_val_if_fail (EMPATHY_IS_FT_HANDLER (handler), NULL);

  priv = handler->priv;

  return priv->filename;
}

/**
 * empathy_ft_handler_get_content_type:
 * @handler: an #EmpathyFTHandler
 *
 * Returns the content type of the file being transferred.
 *
 * Return value: the content type of the file being transferred
 */
const char *
empathy_ft_handler_get_content_type (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_val_if_fail (EMPATHY_IS_FT_HANDLER (handler), NULL);

  priv = handler->priv;

  return priv->content_type;
}

/**
 * empathy_ft_handler_get_contact:
 * @handler: an #EmpathyFTHandler
 *
 * Returns the remote #TpContact at the other side of the transfer.
 *
 * Return value: the remote #TpContact for @handler
 */
TpContact *
empathy_ft_handler_get_contact (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_val_if_fail (EMPATHY_IS_FT_HANDLER (handler), NULL);

  priv = handler->priv;

  return priv->contact;
}

/**
 * empathy_ft_handler_get_gfile:
 * @handler: an #EmpathyFTHandler
 *
 * Returns the #GFile where the transfer is being read/saved.
 *
 * Return value: the #GFile where the transfer is being read/saved
 */
GFile *
empathy_ft_handler_get_gfile (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_val_if_fail (EMPATHY_IS_FT_HANDLER (handler), NULL);

  priv = handler->priv;

  return priv->gfile;
}

/**
 * empathy_ft_handler_get_use_hash:
 * @handler: an #EmpathyFTHandler
 *
 * Returns whether @handler has checksumming enabled. This can depend on
 * the CM and the remote contact capabilities.
 *
 * Return value: %TRUE if the handler has checksumming enabled,
 * %FALSE otherwise.
 */
gboolean
empathy_ft_handler_get_use_hash (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_val_if_fail (EMPATHY_IS_FT_HANDLER (handler), FALSE);

  priv = handler->priv;

  return priv->use_hash;
}

/**
 * empathy_ft_handler_is_incoming:
 * @handler: an #EmpathyFTHandler
 *
 * Returns whether @handler is incoming or outgoing.
 *
 * Return value: %TRUE if the handler is incoming, %FALSE otherwise.
 */
gboolean
empathy_ft_handler_is_incoming (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_val_if_fail (EMPATHY_IS_FT_HANDLER (handler), FALSE);

  priv = handler->priv;

  if (priv->channel == NULL)
    return FALSE;

  return !tp_channel_get_requested ((TpChannel *) priv->channel);
}

/**
 * empathy_ft_handler_get_transferred_bytes:
 * @handler: an #EmpathyFTHandler
 *
 * Returns the number of bytes already transferred by the handler.
 *
 * Return value: the number of bytes already transferred by the handler.
 */
guint64
empathy_ft_handler_get_transferred_bytes (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_val_if_fail (EMPATHY_IS_FT_HANDLER (handler), 0);

  priv = handler->priv;

  return priv->transferred_bytes;
}

/**
 * empathy_ft_handler_get_total_bytes:
 * @handler: an #EmpathyFTHandler
 *
 * Returns the total size of the file being transferred by the handler.
 *
 * Return value: a number of bytes indicating the total size of the file being
 * transferred by the handler.
 */
guint64
empathy_ft_handler_get_total_bytes (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_val_if_fail (EMPATHY_IS_FT_HANDLER (handler), 0);

  priv = handler->priv;

  return priv->total_bytes;
}

/**
 * empathy_ft_handler_is_completed:
 * @handler: an #EmpathyFTHandler
 *
 * Returns whether the transfer for @handler has been completed succesfully.
 *
 * Return value: %TRUE if the handler has been transferred correctly, %FALSE
 * otherwise
 */
gboolean
empathy_ft_handler_is_completed (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_val_if_fail (EMPATHY_IS_FT_HANDLER (handler), FALSE);

  priv = handler->priv;

  return priv->is_completed;
}

/**
 * empathy_ft_handler_is_cancelled:
 * @handler: an #EmpathyFTHandler
 *
 * Returns whether the transfer for @handler has been cancelled or has stopped
 * due to an error.
 *
 * Return value: %TRUE if the transfer for @handler has been cancelled
 * or has stopped due to an error, %FALSE otherwise.
 */
gboolean
empathy_ft_handler_is_cancelled (EmpathyFTHandler *handler)
{
  EmpathyFTHandlerPriv *priv;

  g_return_val_if_fail (EMPATHY_IS_FT_HANDLER (handler), FALSE);

  priv = handler->priv;

  return g_cancellable_is_cancelled (priv->cancellable);
}
