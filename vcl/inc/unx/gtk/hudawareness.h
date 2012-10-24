/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#ifndef _hudawareness_h_
#define _hudawareness_h_

#include <gio/gio.h>

G_BEGIN_DECLS

typedef void         (* HudAwarenessCallback)                           (gboolean               hud_active,
                                                                         gpointer               user_data);


guint                   hud_awareness_register                          (GDBusConnection       *connection,
                                                                         const gchar           *object_path,
                                                                         HudAwarenessCallback   callback,
                                                                         gpointer               user_data,
                                                                         GDestroyNotify         notify,
                                                                         GError               **error);

void                    hud_awareness_unregister                        (GDBusConnection       *connection,
                                                                         guint                  awareness_id);

G_END_DECLS

#endif /* _hudawareness_h_ */
