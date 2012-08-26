/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * Copyright © 2011 Canonical Ltd.
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
 * Author: Antonio Fernández <antonio.fernandez@aentos.es>
 */

#ifndef __G_LO_MENU_H__
#define __G_LO_MENU_H__

#include <gio/gio.h>

#define G_LO_MENU_ATTRIBUTE_ACCELERATOR     "accel"

G_BEGIN_DECLS

#define G_TYPE_LO_MENU          (g_lo_menu_get_type ())
#define G_LO_MENU(inst)         (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                 G_TYPE_LO_MENU, GLOMenu))
#define G_IS_LO_MENU(inst)      (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                 G_TYPE_LO_MENU))

typedef struct _GLOMenu GLOMenu;

class GtkSalMenuItem;

GLIB_AVAILABLE_IN_2_32
GType       g_lo_menu_get_type                                          (void) G_GNUC_CONST;
GLIB_AVAILABLE_IN_2_32
GLOMenu *   g_lo_menu_new                                               (void);

void        g_lo_menu_insert                                            (GLOMenu     *menu,
                                                                         gint         position,
                                                                         const char  *label);

void        g_lo_menu_insert_in_section                                 (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position,
                                                                         const gchar *label);

void        g_lo_menu_new_section                                       (GLOMenu     *menu,
                                                                         gint         position,
                                                                         const gchar *label);

void        g_lo_menu_insert_section                                    (GLOMenu     *menu,
                                                                         gint         position,
                                                                         const gchar *label,
                                                                         GMenuModel  *section);

GLOMenu *   g_lo_menu_get_section                                       (GLOMenu     *menu,
                                                                         gint         section);

void        g_lo_menu_remove                                            (GLOMenu     *menu,
                                                                         gint         position);

void        g_lo_menu_remove_from_section                               (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position);

void        g_lo_menu_set_label                                         (GLOMenu     *menu,
                                                                         gint         position,
                                                                         const gchar *label);

void        g_lo_menu_set_label_to_item_in_section                      (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position,
                                                                         const gchar *label);

gchar *     g_lo_menu_get_label_from_item_in_section                    (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position);

void        g_lo_menu_set_action_and_target_value                       (GLOMenu     *menu,
                                                                         gint         position,
                                                                         const gchar *command,
                                                                         GVariant    *target_value);

void        g_lo_menu_set_action_and_target_value_to_item_in_section    (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position,
                                                                         const gchar *command,
                                                                         GVariant    *target_value);

void        g_lo_menu_set_accelerator_to_item_in_section                (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position,
                                                                         const gchar *accelerator);

void        g_lo_menu_set_submenu_to_item_in_section                    (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position,
                                                                         GMenuModel  *submenu);

G_END_DECLS

#endif /* __G_LO_MENU_H__ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
