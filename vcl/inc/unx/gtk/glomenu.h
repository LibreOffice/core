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
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#ifndef __G_LO_MENU_H__
#define __G_LO_MENU_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define G_TYPE_LO_MENU          (g_lo_menu_get_type ())
#define G_LO_MENU(inst)         (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                 G_TYPE_LO_MENU, GLOMenu))
#define G_IS_LO_MENU(inst)      (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                 G_TYPE_LO_MENU))

#define G_TYPE_LO_MENU_ITEM     (g_lo_menu_item_get_type ())
#define G_LO_MENU_ITEM(inst)    (G_TYPE_CHECK_INSTANCE_CAST ((inst), \
                                 G_TYPE_LO_MENU_ITEM, GLOMenuItem))
#define G_IS_LO_MENU_ITEM(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
                                 G_TYPE_LO_MENU_ITEM))

typedef struct _GLOMenuItem GLOMenuItem;
typedef struct _GLOMenu     GLOMenu;

GLIB_AVAILABLE_IN_2_32
GType           g_lo_menu_get_type                          (void) G_GNUC_CONST;
GLIB_AVAILABLE_IN_2_32
GLOMenu *       g_lo_menu_new                               (void);

void            g_lo_menu_freeze                            (GLOMenu     *menu);

void            g_lo_menu_insert_item                       (GLOMenu     *menu,
                                                             gint         position,
                                                             GLOMenuItem *item);
void            g_lo_menu_prepend_item                      (GLOMenu     *menu,
                                                             GLOMenuItem *item);
void            g_lo_menu_append_item                       (GLOMenu     *menu,
                                                             GLOMenuItem *item);
void            g_lo_menu_remove                            (GLOMenu     *menu,
                                                             gint         position);

void            g_lo_menu_insert                            (GLOMenu     *menu,
                                                             gint         position,
                                                             const gchar *label,
                                                             const gchar *detailed_action);
void            g_lo_menu_prepend                           (GLOMenu     *menu,
                                                             const gchar *label,
                                                             const gchar *detailed_action);
void            g_lo_menu_append                            (GLOMenu     *menu,
                                                             const gchar *label,
                                                             const gchar *detailed_action);

void            g_lo_menu_insert_section                    (GLOMenu     *menu,
                                                             gint         position,
                                                             const gchar *label,
                                                             GMenuModel  *section);
void            g_lo_menu_prepend_section                   (GLOMenu     *menu,
                                                             const gchar *label,
                                                             GMenuModel  *section);
void            g_lo_menu_append_section                    (GLOMenu     *menu,
                                                             const gchar *label,
                                                             GMenuModel  *section);

void            g_lo_menu_insert_submenu                    (GLOMenu     *menu,
                                                             gint         position,
                                                             const gchar *label,
                                                             GMenuModel  *submenu);
void            g_lo_menu_prepend_submenu                   (GLOMenu     *menu,
                                                             const gchar *label,
                                                             GMenuModel  *submenu);
void            g_lo_menu_append_submenu                    (GLOMenu     *menu,
                                                             const gchar *label,
                                                             GMenuModel  *submenu);


GType           g_lo_menu_item_get_type                     (void) G_GNUC_CONST;
GLOMenuItem *   g_lo_menu_item_new                          (const gchar *label,
                                                             const gchar *detailed_action);

GLOMenuItem *   g_lo_menu_item_new_submenu                  (const gchar *label,
                                                             GMenuModel  *submenu);

GLOMenuItem *   g_lo_menu_item_new_section                  (const gchar *label,
                                                             GMenuModel  *section);

void            g_lo_menu_item_set_attribute_value          (GLOMenuItem *menu_item,
                                                             const gchar *attribute,
                                                             GVariant    *value);
void            g_lo_menu_item_set_attribute                (GLOMenuItem *menu_item,
                                                             const gchar *attribute,
                                                             const gchar *format_string,
                                                             ...);
void            g_lo_menu_item_set_link                     (GLOMenuItem *menu_item,
                                                             const gchar *link,
                                                             GMenuModel  *model);
void            g_lo_menu_item_set_label                    (GLOMenuItem *menu_item,
                                                             const gchar *label);
void            g_lo_menu_item_set_submenu                  (GLOMenuItem *menu_item,
                                                             GMenuModel  *submenu);
void            g_lo_menu_item_set_section                  (GLOMenuItem *menu_item,
                                                             GMenuModel  *section);
void            g_lo_menu_item_set_action_and_target_value  (GLOMenuItem *menu_item,
                                                             const gchar *action,
                                                             GVariant    *target_value);
void            g_lo_menu_item_set_action_and_target        (GLOMenuItem *menu_item,
                                                             const gchar *action,
                                                             const gchar *format_string,
                                                             ...);
void            g_lo_menu_item_set_detailed_action          (GLOMenuItem *menu_item,
                                                             const gchar *detailed_action);

G_END_DECLS

#endif /* __G_LO_MENU_H__ */

