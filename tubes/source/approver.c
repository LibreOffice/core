/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <glib.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include <telepathy-glib/telepathy-glib.h>
#include <telepathy-glib/debug.h>
#include <telepathy-glib/simple-approver.h>

#include <tubes/constants.h>

GMainLoop *mainloop = NULL;

static void
handle_with_cb (GObject *source,
    GAsyncResult *result,
    gpointer user_data)
{
  TpChannelDispatchOperation *cdo = TP_CHANNEL_DISPATCH_OPERATION (source);
  GtkMessageDialog *dialog = GTK_MESSAGE_DIALOG (user_data);
  GError *error = NULL;

  if (!tp_channel_dispatch_operation_handle_with_finish (cdo, result, &error))
    {
      g_print ("HandleWith() failed: %s\n", error->message);
      gtk_message_dialog_format_secondary_markup (dialog,
          "<b>Error</b>\n\nAsking LibreOffice to accept the session failed: <i>%s</i>",
          error->message);
      g_error_free (error);
      return;
    }

  g_print ("HandleWith() succeeded\n");
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
close_cb (GObject *source,
    GAsyncResult *result,
    gpointer user_data)

{
  TpChannelDispatchOperation *cdo = TP_CHANNEL_DISPATCH_OPERATION (source);
  GError *error = NULL;

  (void)user_data;      /* suppress unused-parameter warning */

  if (!tp_channel_dispatch_operation_close_channels_finish (cdo, result, &error))
    {
      g_print ("Rejecting channels failed: %s\n", error->message);
      g_error_free (error);
      return;
    }

  g_print ("Rejected all the things!\n");
}

static void
dialog_response_cb (
    GtkWidget *dialog,
    gint response_id,
    gpointer user_data)
{
    TpSimpleApprover *self = TP_SIMPLE_APPROVER (g_object_get_data (G_OBJECT (dialog), "client"));
    TpChannelDispatchOperation *cdo = TP_CHANNEL_DISPATCH_OPERATION (user_data);

    (void)self;     /* suppress unused-parameter warning (could remove TP_SIMPLE_APPROVER above?) */

    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        g_print ("Approve channels\n");

        tp_channel_dispatch_operation_handle_with_async (cdo, NULL,
            handle_with_cb, dialog);

        gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT, FALSE);
        gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_REJECT, FALSE);
    }
    else
    {
        g_print ("Reject channels\n");

        tp_channel_dispatch_operation_close_channels_async (cdo, close_cb, dialog);
        gtk_widget_destroy (dialog);
    }

    g_object_unref (cdo);
}

static void
show_dialog (
    TpSimpleApprover *self,
    TpChannelDispatchOperation *cdo,
    TpContact *target)
{
    GFile *avatar_file = tp_contact_get_avatar_file (target);
    GtkWidget *dialog = gtk_message_dialog_new_with_markup (NULL,
        0, /* flags */
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_NONE,
        "<b>%s</b> (<i>%s</i>) would like to edit a spreadsheet in LibreOffice "
        "with you.",
        tp_contact_get_alias (target),
        tp_contact_get_identifier (target));

    if (avatar_file != NULL)
    {
        GtkWidget *avatar = gtk_image_new_from_file (g_file_get_path (avatar_file));

        gtk_message_dialog_set_image (GTK_MESSAGE_DIALOG (dialog), avatar);
    }

    gtk_dialog_add_buttons (GTK_DIALOG (dialog),
        "_Reject", GTK_RESPONSE_REJECT,
        "_Accept", GTK_RESPONSE_ACCEPT,
        NULL);

    g_object_set_data_full (G_OBJECT (dialog), "client", g_object_ref (self), g_object_unref);
    g_signal_connect (dialog, "response", G_CALLBACK (dialog_response_cb), g_object_ref (cdo));

    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), FALSE);

    gtk_widget_show_all (dialog);
}

static void
add_dispatch_operation_cb (TpSimpleApprover *self,
    TpAccount *account,
    TpConnection *connection,
    GList *channels,
    TpChannelDispatchOperation *cdo,
    TpAddDispatchOperationContext *context,
    gpointer user_data)
{
    TpContact *target = NULL;
    GList *l;

    (void)account;      /* suppress unused-parameter warning */
    (void)connection;   /* suppress unused-parameter warning */
    (void)user_data;    /* suppress unused-parameter warning */

    g_print ("Approving this batch of channels:\n");

    for (l = channels; l != NULL; l = g_list_next (l))
    {
        TpChannel *channel = l->data;

        if (TP_IS_DBUS_TUBE_CHANNEL (channel))
        {
            target = tp_channel_get_target_contact (channel);
            break;
        }
    }

    if (target == NULL)
    {
        g_critical ("Hmm. No 1-1 D-Bus tube in cdo %s, so why did we get it?",
            tp_proxy_get_object_path (cdo));
        g_return_if_reached ();
    }

    tp_add_dispatch_operation_context_accept (context);
    show_dialog (self, cdo, target);
}

int
main (int argc,
      char **argv)
{
  TpAccountManager *manager;
  TpSimpleClientFactory *factory;
  TpBaseClient *approver;
  GError *error = NULL;

  gtk_init (&argc, &argv);
  tp_debug_set_flags (g_getenv ("LIBO_APPROVER_DEBUG"));

  manager = tp_account_manager_dup ();

    factory = tp_proxy_get_factory (manager);
    /* We want the target contact on channels to be available... */
    tp_simple_client_factory_add_channel_features_varargs (factory,
        TP_CHANNEL_FEATURE_CONTACTS,
        0);
    /* ...and for it to have its alias and avatar available */
    tp_simple_client_factory_add_contact_features_varargs (factory,
        TP_CONTACT_FEATURE_ALIAS,
        TP_CONTACT_FEATURE_AVATAR_DATA,
        TP_CONTACT_FEATURE_INVALID);

  approver = tp_simple_approver_new_with_am (manager, "LibreOfficeApprover",
      FALSE, add_dispatch_operation_cb, NULL, NULL);

  tp_base_client_take_approver_filter (approver, tp_asv_new (
        TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING,
          TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
        TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT,
          TP_HANDLE_TYPE_CONTACT,
        TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING,
          LIBO_DTUBE_SERVICE,
        NULL));

  if (!tp_base_client_register (approver, &error))
    {
      g_warning ("Failed to register Approver: %s\n", error->message);
      g_error_free (error);
      goto out;
    }

  g_print ("Start approving\n");

  mainloop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (mainloop);
  /* TODO: time out after 5 seconds of inactivity? */

  if (mainloop != NULL)
    g_main_loop_unref (mainloop);

out:
  g_object_unref (manager);
  g_object_unref (approver);

  return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
