/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string.h>

#include <unx/gtk/gtksalmenu.hxx>

#ifdef ENABLE_GMENU_INTEGRATION

#include <unx/gtk/hudawareness.h>

struct HudAwarenessHandle
{
  GDBusConnection *connection;
  HudAwarenessCallback callback;
  gpointer user_data;
  GDestroyNotify notify;
};

static void
hud_awareness_method_call (GDBusConnection       * /* connection */,
                           const gchar           * /* sender */,
                           const gchar           * /* object_path */,
                           const gchar           * /* interface_name */,
                           const gchar           *method_name,
                           GVariant              *parameters,
                           GDBusMethodInvocation *invocation,
                           gpointer               user_data)
{
  HudAwarenessHandle *handle = static_cast<HudAwarenessHandle*>(user_data);

  if (g_str_equal (method_name, "HudActiveChanged"))
    {
      gboolean active;

      g_variant_get (parameters, "(b)", &active);

      (* handle->callback) (active, handle->user_data);
    }

  g_dbus_method_invocation_return_value (invocation, nullptr);
}

guint
hud_awareness_register (GDBusConnection       *connection,
                        const gchar           *object_path,
                        HudAwarenessCallback   callback,
                        gpointer               user_data,
                        GDestroyNotify         notify,
                        GError               **error)
{
  static GDBusInterfaceInfo *iface;
  static GDBusNodeInfo *info;
  GDBusInterfaceVTable vtable;
  HudAwarenessHandle *handle;
  guint object_id;

  memset (static_cast<void *>(&vtable), 0, sizeof (vtable));
  vtable.method_call = hud_awareness_method_call;

  if G_UNLIKELY (iface == nullptr)
    {
      GError *local_error = nullptr;

      info = g_dbus_node_info_new_for_xml ("<node>"
                                             "<interface name='com.canonical.hud.Awareness'>"
                                               "<method name='CheckAwareness'/>"
                                               "<method name='HudActiveChanged'>"
                                                 "<arg type='b'/>"
                                               "</method>"
                                             "</interface>"
                                           "</node>",
                                           &local_error);
      g_assert_no_error (local_error);
      iface = g_dbus_node_info_lookup_interface (info, "com.canonical.hud.Awareness");
      g_assert (iface != nullptr);
    }

  handle = static_cast<HudAwarenessHandle*>(g_malloc (sizeof (HudAwarenessHandle)));

  object_id = g_dbus_connection_register_object (connection, object_path, iface, &vtable, handle, &g_free, error);

  if (object_id == 0)
    {
      g_free (handle);
      return 0;
    }

  handle->connection = static_cast<GDBusConnection*>(g_object_ref (connection));
  handle->callback = callback;
  handle->user_data = user_data;
  handle->notify = notify;

  return object_id;
}

void
hud_awareness_unregister (GDBusConnection *connection,
                          guint            subscription_id)
{
  g_dbus_connection_unregister_object (connection, subscription_id);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
