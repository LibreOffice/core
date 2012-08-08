#ifndef GLOMENU_H
#define GLOMENU_H

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define G_TYPE_LO_MENU                  (g_lo_menu_get_type())
#define G_LO_MENU(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_LO_MENU, GLOMenu))
#define G_IS_LO_MENU(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_LO_MENU))

#define G_TYPE_LO_MENU_ITEM             (g_lo_menu_item_get_type())
#define G_LO_MENU_ITEM(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_LO_MENU_ITEM, GLOMenuItem))
#define G_IS_LO_MENU_ITEM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_LO_MENU_ITEM))

typedef struct _GLOMenuItem GLOMenuItem;
typedef struct _GLOMenu     GLOMenu;

GLIB_AVAILABLE_IN_2_32
GType           g_lo_menu_get_type                          (void) G_GNUC_CONST;
GLIB_AVAILABLE_IN_2_32

GLOMenu *       g_lo_menu_new                               (void);
void            g_lo_menu_insert_item                       (GLOMenu        *menu,
                                                             gint            position,
                                                             GLOMenuItem    *item);
void            g_lo_menu_insert                            (GLOMenu        *menu,
                                                             gint            position,
                                                             const gchar    *label,
                                                             const gchar    *detailed_action);
void            g_lo_menu_append                            (GLOMenu        *menu,
                                                             const gchar    *label,
                                                             const gchar    *detailed_action);
void            g_lo_menu_append_item                       (GLOMenu        *menu,
                                                             GLOMenuItem    *item);
void            g_lo_menu_insert_section                    (GLOMenu        *menu,
                                                             gint            position,
                                                             const gchar    *label,
                                                             GMenuModel     *section);
void            g_lo_menu_prepend_section                   (GLOMenu        *menu,
                                                             const gchar    *label,
                                                             GMenuModel     *section);
void            g_lo_menu_append_section                    (GLOMenu        *menu,
                                                             const gchar    *label,
                                                             GMenuModel     *section);
void            g_lo_menu_insert_submenu                    (GLOMenu        *menu,
                                                             gint            position,
                                                             const gchar    *label,
                                                             GMenuModel     *submenu);
void            g_lo_menu_append_submenu                    (GLOMenu        *menu,
                                                             const gchar    *label,
                                                             GMenuModel     *submenu);

GType           g_lo_menu_item_get_type                     (void) G_GNUC_CONST;
GLOMenuItem *   g_lo_menu_item_new                          (const gchar    *label,
                                                             const gchar    *detailed_action);
GLOMenuItem *   g_lo_menu_item_new_submenu                  (const gchar    *label,
                                                             GMenuModel     *submenu);
GLOMenuItem *   g_lo_menu_item_new_section                  (const gchar    *label,
                                                             GMenuModel     *section);
void            g_lo_menu_item_set_attribute_value          (GLOMenuItem    *menu_item,
                                                             const gchar    *attribute,
                                                             GVariant       *value);
void            g_lo_menu_item_set_link                     (GLOMenuItem    *menu_item,
                                                             const gchar    *link,
                                                             GMenuModel     *model);
void            g_lo_menu_item_set_label                    (GLOMenuItem    *menu_item,
                                                             const gchar    *label);
void            g_lo_menu_item_set_submenu                  (GLOMenuItem    *menu_item,
                                                             GMenuModel     *submenu);
void            g_lo_menu_item_set_section                  (GLOMenuItem    *menu_item,
                                                             GMenuModel     *section);
void            g_lo_menu_item_set_action_and_target_value  (GLOMenuItem    *menu_item,
                                                             const gchar    *action,
                                                             GVariant       *target_value);
void            g_lo_menu_item_set_action_and_target        (GLOMenuItem    *menu_item,
                                                             const gchar    *action,
                                                             const gchar    *format_string,
                                                             ...);
void            g_lo_menu_item_set_detailed_action          (GLOMenuItem    *menu_item,
                                                             const gchar    *detailed_action);

G_END_DECLS


#endif // GLOMENU_H
