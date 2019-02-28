/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_UNX_GTK_GLOMENU_H
#define INCLUDED_VCL_INC_UNX_GTK_GLOMENU_H

#include <gio/gio.h>

#define G_LO_MENU_ATTRIBUTE_ACCELERATOR     "accel"
#define G_LO_MENU_ATTRIBUTE_COMMAND         "command"
#define G_LO_MENU_ATTRIBUTE_SUBMENU_ACTION  "submenu-action"

G_BEGIN_DECLS

#define G_TYPE_LO_MENU          (g_lo_menu_get_type ())
#define G_LO_MENU(inst)         (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                 G_TYPE_LO_MENU, GLOMenu))
#define G_IS_LO_MENU(inst)      (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                 G_TYPE_LO_MENU))

struct GLOMenu;

class GtkSalMenuItem;

GLIB_AVAILABLE_IN_2_32
GType       g_lo_menu_get_type                                          (void) G_GNUC_CONST;
GLIB_AVAILABLE_IN_2_32
GLOMenu *   g_lo_menu_new                                               (void);

gint       g_lo_menu_get_n_items_from_section                           (GLOMenu     *menu,
                                                                         gint         section);

void        g_lo_menu_insert                                            (GLOMenu     *menu,
                                                                         gint         position,
                                                                         const gchar *label);

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

void        g_lo_menu_set_icon                                         (GLOMenu     *menu,
                                                                        gint         position,
                                                                        const GIcon *icon);


void        g_lo_menu_set_label_to_item_in_section                      (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position,
                                                                         const gchar *label);

void        g_lo_menu_set_icon_to_item_in_section                      (GLOMenu     *menu,
                                                                        gint         section,
                                                                        gint         position,
                                                                        const GIcon *icon);

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

void        g_lo_menu_set_command_to_item_in_section                    (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position,
                                                                         const gchar *command);

gchar *     g_lo_menu_get_command_from_item_in_section                  (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position);

void        g_lo_menu_set_accelerator_to_item_in_section                (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position,
                                                                         const gchar *accelerator);

gchar *     g_lo_menu_get_accelerator_from_item_in_section              (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position);

void        g_lo_menu_new_submenu_in_item_in_section                    (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position);

GLOMenu *   g_lo_menu_get_submenu_from_item_in_section                  (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position);

void        g_lo_menu_set_submenu_action_to_item_in_section             (GLOMenu     *menu,
                                                                         gint         section,
                                                                         gint         position,
                                                                         const gchar *action);

G_END_DECLS

#endif // INCLUDED_VCL_INC_UNX_GTK_GLOMENU_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
