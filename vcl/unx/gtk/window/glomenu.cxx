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

#include <stdio.h>
#include <string.h>

#include <unx/gtk/gtksalmenu.hxx>

#include <unx/gtk/glomenu.h>

struct _GLOMenu
{
    GMenuModel  parent_instance;

    GArray      *items;
};

typedef GMenuModelClass GLOMenuClass;

G_DEFINE_TYPE (GLOMenu, g_lo_menu, G_TYPE_MENU_MODEL);

struct item
{
    GHashTable*     attributes;     // Item attributes.
    GHashTable*     links;          // Item links.
};


static void
g_lo_menu_struct_item_init (struct item *menu_item)
{
    menu_item->attributes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_variant_unref);
    menu_item->links = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
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

/*
 * GLOMenu
 */

static gboolean
g_lo_menu_is_mutable (GMenuModel *model)
{
    // Menu is always mutable.
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
g_lo_menu_insert (GLOMenu        *menu,
                  gint            position,
                  const gchar    *label)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    if (position < 0 || position > (gint) menu->items->len)
        position = menu->items->len;

    struct item menu_item;

    g_lo_menu_struct_item_init(&menu_item);

    g_array_insert_val (menu->items, position, menu_item);

    g_lo_menu_set_label (menu, position, label);

    g_menu_model_items_changed (G_MENU_MODEL (menu), position, 0, 1);
}

void
g_lo_menu_insert_in_section (GLOMenu        *menu,
                             gint            section,
                             gint            position,
                             const gchar    *label)
{
    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (0 <= section && section < (gint) menu->items->len);

    GLOMenu *model = G_LO_MENU (G_MENU_MODEL_CLASS (g_lo_menu_parent_class)
                                ->get_item_link (G_MENU_MODEL (menu), section, G_MENU_LINK_SECTION));

    g_return_if_fail (model != NULL);

    g_lo_menu_insert (model, position, label);
}

GLOMenu *
g_lo_menu_new (void)
{
    return G_LO_MENU( g_object_new (G_TYPE_LO_MENU, NULL) );
}

void
g_lo_menu_set_attribute_value (GLOMenu     *menu,
                               gint         position,
                               const gchar *attribute,
                               GVariant    *value)
{
    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (attribute != NULL);
    g_return_if_fail (valid_attribute_name (attribute));

    if (position >= menu->items->len)
        return;

    struct item menu_item = g_array_index (menu->items, struct item, position);

    if (value != NULL)
        g_hash_table_insert (menu_item.attributes, g_strdup (attribute), g_variant_ref_sink (value));
    else
        g_hash_table_remove (menu_item.attributes, attribute);

}

void
g_lo_menu_set_label (GLOMenu     *menu,
                     gint         position,
                     const gchar *label)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    GVariant *value;

    if (label != NULL)
        value = g_variant_new_string (label);
    else
        value = NULL;

    g_lo_menu_set_attribute_value (menu, position, G_MENU_ATTRIBUTE_LABEL, value);
}

void
g_lo_menu_set_label_to_item_in_section (GLOMenu     *menu,
                                        gint         section,
                                        gint         position,
                                        const gchar *label)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    GLOMenu *model = G_LO_MENU (G_MENU_MODEL_CLASS (g_lo_menu_parent_class)
                                ->get_item_link (G_MENU_MODEL (menu), section, G_MENU_LINK_SECTION));

    g_return_if_fail (model != NULL);

    g_lo_menu_set_label (model, position, label);

    // Notify the update.
    g_menu_model_items_changed (G_MENU_MODEL (model), position, 1, 1);
}

gchar *
g_lo_menu_get_label_from_item_in_section (GLOMenu     *menu,
                                          gint         section,
                                          gint         position)
{
    g_return_val_if_fail (G_IS_LO_MENU (menu), NULL);

    GLOMenu *model = G_LO_MENU (G_MENU_MODEL_CLASS (g_lo_menu_parent_class)
                                ->get_item_link (G_MENU_MODEL (menu), section, G_MENU_LINK_SECTION));

    g_return_val_if_fail (model != NULL, NULL);

    struct item menu_item = g_array_index (model->items, struct item, position);



    GVariant *current_label = (GVariant*) g_hash_table_lookup (menu_item.attributes, G_MENU_ATTRIBUTE_LABEL);

    if (current_label)
        return (gchar*) g_variant_get_string (current_label, NULL);
    else
        return NULL;
}

void
g_lo_menu_set_action_and_target_value (GLOMenu     *menu,
                                       gint         position,
                                       const gchar *action,
                                       GVariant    *target_value)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

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

    g_lo_menu_set_attribute_value (menu, position, G_MENU_ATTRIBUTE_ACTION, action_value);
    g_lo_menu_set_attribute_value (menu, position, G_MENU_ATTRIBUTE_TARGET, target_value);
}

void
g_lo_menu_set_action_and_target_value_to_item_in_section (GLOMenu     *menu,
                                                          gint         section,
                                                          gint         position,
                                                          const gchar *command,
                                                          GVariant    *target_value)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    struct item menu_item = g_array_index (menu->items, struct item, section);

    GLOMenu *model = G_LO_MENU (g_hash_table_lookup (menu_item.links, G_MENU_LINK_SECTION));

    g_return_if_fail (model != NULL);

    g_lo_menu_set_action_and_target_value (model, position, command, target_value);

    // Notify the update.
    g_menu_model_items_changed (G_MENU_MODEL (model), position, 1, 1);
}

void
g_lo_menu_set_link (GLOMenu     *menu,
                    gint         position,
                    const gchar *link,
                    GMenuModel  *model)
{
    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (link != NULL);
    g_return_if_fail (valid_attribute_name (link));

    if (position < 0 || position >= (gint) menu->items->len)
        position = menu->items->len - 1;

    struct item menu_item = g_array_index (menu->items, struct item, position);

    if (model != NULL)
        g_hash_table_insert (menu_item.links, g_strdup (link), g_object_ref (model));
    else
        g_hash_table_remove (menu_item.links, link);
}

void
g_lo_menu_insert_section (GLOMenu *menu,
                          gint position,
                          const gchar *label,
                          GMenuModel* section)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    if (position < 0 || position > (gint) menu->items->len)
        position = menu->items->len;

    struct item menu_item;

    g_lo_menu_struct_item_init(&menu_item);

    g_array_insert_val (menu->items, position, menu_item);

    g_lo_menu_set_label (menu, position, label);
    g_lo_menu_set_link (menu, position, G_MENU_LINK_SECTION, section);

    g_menu_model_items_changed (G_MENU_MODEL (menu), position, 0, 1);
}

void
g_lo_menu_new_section (GLOMenu     *menu,
                       gint         position,
                       const gchar *label)
{
    GMenuModel *section = G_MENU_MODEL (g_lo_menu_new());

    g_lo_menu_insert_section (menu, position, label, section);

//    g_object_unref (section);
}



GLOMenu *
g_lo_menu_get_section (GLOMenu *menu,
                       gint section)
{
    g_return_val_if_fail (G_IS_LO_MENU (menu), NULL);

    return G_LO_MENU (G_MENU_MODEL_CLASS (g_lo_menu_parent_class)
                      ->get_item_link (G_MENU_MODEL (menu), section, G_MENU_LINK_SECTION));
}

void
g_lo_menu_set_submenu_to_item_in_section (GLOMenu    *menu,
                                          gint        section,
                                          gint        position,
                                          GMenuModel *submenu)
{
//    puts(__FUNCTION__);
    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (0 <= section && section < (gint) menu->items->len);

    GLOMenu *model = G_LO_MENU (G_MENU_MODEL_CLASS (g_lo_menu_parent_class)
                                ->get_item_link (G_MENU_MODEL (menu), section, G_MENU_LINK_SECTION));

    g_return_if_fail (model != NULL);

    g_lo_menu_set_link (model, position, G_MENU_LINK_SUBMENU, submenu);
}

static void
g_lo_menu_clear_item (struct item *menu_item)
{
    if (menu_item->attributes != NULL)
        g_hash_table_unref (menu_item->attributes);
    if (menu_item->links != NULL)
        g_hash_table_unref (menu_item->links);
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

void
g_lo_menu_remove_from_section (GLOMenu *menu,
                               gint     section,
                               gint     position)
{
    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (0 <= section && section < (gint) menu->items->len);

    GLOMenu *model = G_LO_MENU (G_MENU_MODEL_CLASS (g_lo_menu_parent_class)
                                ->get_item_link (G_MENU_MODEL (menu), section, G_MENU_LINK_SECTION));

    g_return_if_fail (model != NULL);

    g_lo_menu_remove (model, position);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
