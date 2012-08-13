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

#include <stdio.h>
#include <string.h>

#include <unx/gtk/glomenu.h>


struct _GLOMenuItem
{
    GObject parent_instance;

    GHashTable *attributes;
    GHashTable *links;
    gboolean    cow;
};

typedef GObjectClass GLOMenuItemClass;

struct _GLOMenu
{
    GMenuModel parent_instance;

    GArray   *items;
//    gboolean  mutable;
};

typedef GMenuModelClass GLOMenuClass;

G_DEFINE_TYPE (GLOMenu, g_lo_menu, G_TYPE_MENU_MODEL)
G_DEFINE_TYPE (GLOMenuItem, g_lo_menu_item, G_TYPE_OBJECT)

struct item
{
    GHashTable *attributes;
    GHashTable *links;
};

static gboolean
g_lo_menu_is_mutable (GMenuModel *model)
{
//    GMenu *menu = G_MENU (model);

//    return menu->mutable;
    return TRUE;
}

static gint
g_lo_menu_get_n_items (GMenuModel *model)
{
    GLOMenu *menu = G_LO_MENU (model);

    return menu->items->len;
}

static void
g_lo_menu_get_item_attributes (GMenuModel  *model,
                            gint         position,
                            GHashTable **table)
{
    GLOMenu *menu = G_LO_MENU (model);

    *table = g_hash_table_ref (g_array_index (menu->items, struct item, position).attributes);
}

static void
g_lo_menu_get_item_links (GMenuModel  *model,
                       gint         position,
                       GHashTable **table)
{
    GLOMenu *menu = G_LO_MENU (model);

    *table = g_hash_table_ref (g_array_index (menu->items, struct item, position).links);
}

void
g_lo_menu_insert_item (GLOMenu     *menu,
                    gint       position,
                    GLOMenuItem *item)
{
    struct item new_item;

    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (G_IS_LO_MENU_ITEM (item));

    if (position < 0 || position > (gint) menu->items->len)
        position = menu->items->len;

    new_item.attributes = g_hash_table_ref (item->attributes);
    new_item.links = g_hash_table_ref (item->links);
    item->cow = TRUE;

    g_array_insert_val (menu->items, position, new_item);
    g_menu_model_items_changed (G_MENU_MODEL (menu), position, 0, 1);
}

void
g_lo_menu_prepend_item (GLOMenu     *menu,
                     GLOMenuItem *item)
{
    g_lo_menu_insert_item (menu, 0, item);
}

void
g_lo_menu_append_item (GLOMenu     *menu,
                    GLOMenuItem *item)
{
    g_lo_menu_insert_item (menu, -1, item);
}

void
g_lo_menu_freeze (GLOMenu *menu)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

//    menu->mutable = FALSE;
}

GLOMenu *
g_lo_menu_new (void)
{
    return G_LO_MENU( g_object_new (G_TYPE_LO_MENU, NULL) );
}

void
g_lo_menu_insert (GLOMenu     *menu,
                  gint         position,
                  const gchar *label,
                  const gchar *detailed_action)
{
    GLOMenuItem *menu_item;

    menu_item = g_lo_menu_item_new (label, detailed_action);
    g_lo_menu_insert_item (menu, position, menu_item);
    g_object_unref (menu_item);
}

void
g_lo_menu_prepend (GLOMenu     *menu,
                   const gchar *label,
                   const gchar *detailed_action)
{
    g_lo_menu_insert (menu, 0, label, detailed_action);
}

void
g_lo_menu_append (GLOMenu     *menu,
                  const gchar *label,
                  const gchar *detailed_action)
{
    g_lo_menu_insert (menu, -1, label, detailed_action);
}

void
g_lo_menu_insert_section (GLOMenu     *menu,
                          gint         position,
                          const gchar *label,
                          GMenuModel  *section)
{
    GLOMenuItem *menu_item;

    menu_item = g_lo_menu_item_new_section (label, section);
    g_lo_menu_insert_item (menu, position, menu_item);
    g_object_unref (menu_item);
}

void
g_lo_menu_prepend_section (GLOMenu     *menu,
                           const gchar *label,
                           GMenuModel  *section)
{
    g_lo_menu_insert_section (menu, 0, label, section);
}

void
g_lo_menu_append_section (GLOMenu     *menu,
                          const gchar *label,
                          GMenuModel  *section)
{
    g_lo_menu_insert_section (menu, -1, label, section);
}

void
g_lo_menu_insert_submenu (GLOMenu     *menu,
                          gint         position,
                          const gchar *label,
                          GMenuModel  *submenu)
{
    GLOMenuItem *menu_item;

    menu_item = g_lo_menu_item_new_submenu (label, submenu);
    g_lo_menu_insert_item (menu, position, menu_item);
    g_object_unref (menu_item);
}

void
g_lo_menu_prepend_submenu (GLOMenu     *menu,
                           const gchar *label,
                           GMenuModel  *submenu)
{
    g_lo_menu_insert_submenu (menu, 0, label, submenu);
}

void
g_lo_menu_append_submenu (GLOMenu     *menu,
                          const gchar *label,
                          GMenuModel  *submenu)
{
    g_lo_menu_insert_submenu (menu, -1, label, submenu);
}

static void
g_lo_menu_clear_item (struct item *item)
{
    if (item->attributes != NULL)
        g_hash_table_unref (item->attributes);
    if (item->links != NULL)
        g_hash_table_unref (item->links);
}

void
g_lo_menu_remove (GLOMenu *menu,
                  gint     position)
{
    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (0 <= position && position < (gint) menu->items->len);

    g_lo_menu_clear_item (&g_array_index (menu->items, struct item, position));
    g_array_remove_index (menu->items, position);
    g_menu_model_items_changed (G_MENU_MODEL (menu), position, 1, 0);
}

static void
g_lo_menu_finalize (GObject *object)
{
    GLOMenu *menu = G_LO_MENU (object);
    struct item *items;
    gint n_items;
    gint i;

    n_items = menu->items->len;
    items = (struct item *) g_array_free (menu->items, FALSE);
    for (i = 0; i < n_items; i++)
        g_lo_menu_clear_item (&items[i]);
    g_free (items);

    G_OBJECT_CLASS (g_lo_menu_parent_class)
            ->finalize (object);
}

static void
g_lo_menu_init (GLOMenu *menu)
{
    menu->items = g_array_new (FALSE, FALSE, sizeof (struct item));
//    menu->mutable = TRUE;
}

static void
g_lo_menu_class_init (GLOMenuClass *klass)
{
    GMenuModelClass *model_class = G_MENU_MODEL_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = g_lo_menu_finalize;

    model_class->is_mutable = g_lo_menu_is_mutable;
    model_class->get_n_items = g_lo_menu_get_n_items;
    model_class->get_item_attributes = g_lo_menu_get_item_attributes;
    model_class->get_item_links = g_lo_menu_get_item_links;
}


static void
g_lo_menu_item_clear_cow (GLOMenuItem *menu_item)
{
    if (menu_item->cow)
    {
        GHashTableIter iter;
        GHashTable *newHash;
        gpointer key;
        gpointer val;

        newHash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_variant_unref);
        g_hash_table_iter_init (&iter, menu_item->attributes);
        while (g_hash_table_iter_next (&iter, &key, &val))
            g_hash_table_insert (newHash, g_strdup ((gchar*) key), g_variant_ref ((GVariant*) val));
        g_hash_table_unref (menu_item->attributes);
        menu_item->attributes = newHash;

        newHash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_object_unref);
        g_hash_table_iter_init (&iter, menu_item->links);
        while (g_hash_table_iter_next (&iter, &key, &val))
            g_hash_table_insert (newHash, g_strdup ((gchar*) key), g_object_ref (val));
        g_hash_table_unref (menu_item->links);
        menu_item->links = newHash;

        menu_item->cow = FALSE;
    }
}

static void
g_lo_menu_item_finalize (GObject *object)
{
    GLOMenuItem *menu_item = G_LO_MENU_ITEM (object);

    g_hash_table_unref (menu_item->attributes);
    g_hash_table_unref (menu_item->links);

    G_OBJECT_CLASS (g_lo_menu_item_parent_class)
            ->finalize (object);
}

static void
g_lo_menu_item_init (GLOMenuItem *menu_item)
{
    menu_item->attributes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_variant_unref);
    menu_item->links = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
    menu_item->cow = FALSE;
}

static void
g_lo_menu_item_class_init (GLOMenuItemClass *klass)
{
    klass->finalize = g_lo_menu_item_finalize;
}

/* We treat attribute names the same as GSettings keys:
 * - only lowercase ascii, digits and '-'
 * - must start with lowercase
 * - must not end with '-'
 * - no consecutive '-'
 * - not longer than 1024 chars
 */
static gboolean
valid_attribute_name (const gchar *name)
{
    gint i;

    if (!g_ascii_islower (name[0]))
        return FALSE;

    for (i = 1; name[i]; i++)
    {
        if (name[i] != '-' &&
                !g_ascii_islower (name[i]) &&
                !g_ascii_isdigit (name[i]))
            return FALSE;

        if (name[i] == '-' && name[i + 1] == '-')
            return FALSE;
    }

    if (name[i - 1] == '-')
        return FALSE;

    if (i > 1024)
        return FALSE;

    return TRUE;
}

void
g_lo_menu_item_set_attribute_value (GLOMenuItem *menu_item,
                                    const gchar *attribute,
                                    GVariant    *value)
{
    g_return_if_fail (G_IS_LO_MENU_ITEM (menu_item));
    g_return_if_fail (attribute != NULL);
    g_return_if_fail (valid_attribute_name (attribute));

    g_lo_menu_item_clear_cow (menu_item);

    if (value != NULL)
        g_hash_table_insert (menu_item->attributes, g_strdup (attribute), g_variant_ref_sink (value));
    else
        g_hash_table_remove (menu_item->attributes, attribute);
}

void
g_lo_menu_item_set_attribute (GLOMenuItem *menu_item,
                              const gchar *attribute,
                              const gchar *format_string,
                              ...)
{
    GVariant *value;

    if (format_string != NULL)
    {
        va_list ap;

        va_start (ap, format_string);
        value = g_variant_new_va (format_string, NULL, &ap);
        va_end (ap);
    }
    else
        value = NULL;

    g_lo_menu_item_set_attribute_value (menu_item, attribute, value);
}

void
g_lo_menu_item_set_link (GLOMenuItem *menu_item,
                         const gchar *link,
                         GMenuModel  *model)
{
    g_return_if_fail (G_IS_LO_MENU_ITEM (menu_item));
    g_return_if_fail (link != NULL);
    g_return_if_fail (valid_attribute_name (link));

    g_lo_menu_item_clear_cow (menu_item);

    if (model != NULL)
        g_hash_table_insert (menu_item->links, g_strdup (link), g_object_ref (model));
    else
        g_hash_table_remove (menu_item->links, link);
}

void
g_lo_menu_item_set_label (GLOMenuItem *menu_item,
                          const gchar *label)
{
    GVariant *value;

    if (label != NULL)
        value = g_variant_new_string (label);
    else
        value = NULL;

    g_lo_menu_item_set_attribute_value (menu_item, G_MENU_ATTRIBUTE_LABEL, value);
}

void
g_lo_menu_item_set_submenu (GLOMenuItem *menu_item,
                            GMenuModel  *submenu)
{
    g_lo_menu_item_set_link (menu_item, G_MENU_LINK_SUBMENU, submenu);
}

void
g_lo_menu_item_set_section (GLOMenuItem *menu_item,
                            GMenuModel  *section)
{
    g_lo_menu_item_set_link (menu_item, G_MENU_LINK_SECTION, section);
}

void
g_lo_menu_item_set_action_and_target_value (GLOMenuItem *menu_item,
                                            const gchar *action,
                                            GVariant    *target_value)
{
    GVariant *action_value;

    if (action != NULL)
    {
        action_value = g_variant_new_string (action);
    }
    else
    {
        action_value = NULL;
        target_value = NULL;
    }

    g_lo_menu_item_set_attribute_value (menu_item, G_MENU_ATTRIBUTE_ACTION, action_value);
    g_lo_menu_item_set_attribute_value (menu_item, G_MENU_ATTRIBUTE_TARGET, target_value);
}

void
g_lo_menu_item_set_action_and_target (GLOMenuItem *menu_item,
                                      const gchar *action,
                                      const gchar *format_string,
                                      ...)
{
    GVariant *value;

    if (format_string != NULL)
    {
        va_list ap;

        va_start (ap, format_string);
        value = g_variant_new_va (format_string, NULL, &ap);
        va_end (ap);
    }
    else
        value = NULL;

    g_lo_menu_item_set_action_and_target_value (menu_item, action, value);
}

void
g_lo_menu_item_set_detailed_action (GLOMenuItem *menu_item,
                                    const gchar *detailed_action)
{
    const gchar *sep;

    sep = strstr (detailed_action, "::");

    if (sep != NULL)
    {
        gchar *action;

        action = g_strndup (detailed_action, sep - detailed_action);
        g_lo_menu_item_set_action_and_target (menu_item, action, "s", sep + 2);
        g_free (action);
    }

    else
        g_lo_menu_item_set_action_and_target_value (menu_item, detailed_action, NULL);
}

GLOMenuItem *
g_lo_menu_item_new (const gchar *label,
                    const gchar *detailed_action)
{
    GLOMenuItem *menu_item;

    menu_item = G_LO_MENU_ITEM( g_object_new (G_TYPE_LO_MENU_ITEM, NULL) );

    if (label != NULL)
        g_lo_menu_item_set_label (menu_item, label);

    if (detailed_action != NULL)
        g_lo_menu_item_set_detailed_action (menu_item, detailed_action);

    return menu_item;
}

GLOMenuItem *
g_lo_menu_item_new_submenu (const gchar *label,
                            GMenuModel  *submenu)
{
    GLOMenuItem *menu_item;

    menu_item = G_LO_MENU_ITEM( g_object_new (G_TYPE_LO_MENU_ITEM, NULL) );

    if (label != NULL)
        g_lo_menu_item_set_label (menu_item, label);

    g_lo_menu_item_set_submenu (menu_item, submenu);

    return menu_item;
}

GLOMenuItem *
g_lo_menu_item_new_section (const gchar *label,
                            GMenuModel  *section)
{
    GLOMenuItem *menu_item;

    menu_item = G_LO_MENU_ITEM( g_object_new (G_TYPE_LO_MENU_ITEM, NULL) );

    if (label != NULL)
        g_lo_menu_item_set_label (menu_item, label);

    g_lo_menu_item_set_section (menu_item, section);

    return menu_item;
}

