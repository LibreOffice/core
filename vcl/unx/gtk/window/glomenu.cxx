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

#include <unx/gtk/glomenu.h>

struct _GLOMenu
{
    GMenuModel  parent_instance;

    GArray      *items;
};

typedef GMenuModelClass GLOMenuClass;

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
G_DEFINE_TYPE (GLOMenu, g_lo_menu, G_TYPE_MENU_MODEL);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

struct item
{
    GHashTable*     attributes;     // Item attributes.
    GHashTable*     links;          // Item links.
};

static void
g_lo_menu_struct_item_init (struct item *menu_item)
{
    menu_item->attributes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, reinterpret_cast<GDestroyNotify>(g_variant_unref));
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
g_lo_menu_is_mutable (GMenuModel*)
{
    // Menu is always mutable.
    return TRUE;
}

static gint
g_lo_menu_get_n_items (GMenuModel *model)
{
    g_return_val_if_fail (model != nullptr, 0);
    GLOMenu *menu = G_LO_MENU (model);
    g_return_val_if_fail (menu->items != nullptr, 0);

    return menu->items->len;
}

gint
g_lo_menu_get_n_items_from_section (GLOMenu *menu,
                                    gint     section)
{
    g_return_val_if_fail (0 <= section && section < (gint) menu->items->len, 0);

    GLOMenu *model = g_lo_menu_get_section (menu, section);

    g_return_val_if_fail (model != nullptr, 0);

    gint length = model->items->len;

    g_object_unref (model);

    return length;
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
g_lo_menu_insert (GLOMenu     *menu,
                  gint         position,
                  const gchar *label)
{
    g_lo_menu_insert_section (menu, position, label, nullptr);
}

void
g_lo_menu_insert_in_section (GLOMenu     *menu,
                             gint         section,
                             gint         position,
                             const gchar *label)
{
    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (0 <= section && section < (gint) menu->items->len);

    GLOMenu *model = g_lo_menu_get_section (menu, section);

    g_return_if_fail (model != nullptr);

    g_lo_menu_insert (model, position, label);

    g_object_unref (model);
}

GLOMenu *
g_lo_menu_new()
{
    return G_LO_MENU( g_object_new (G_TYPE_LO_MENU, nullptr) );
}

void
g_lo_menu_set_attribute_value (GLOMenu     *menu,
                               gint         position,
                               const gchar *attribute,
                               GVariant    *value)
{
    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (attribute != nullptr);
    g_return_if_fail (valid_attribute_name (attribute));

    if (position >= (gint) menu->items->len)
        return;

    struct item menu_item = g_array_index (menu->items, struct item, position);

    if (value != nullptr)
        g_hash_table_insert (menu_item.attributes, g_strdup (attribute), g_variant_ref_sink (value));
    else
        g_hash_table_remove (menu_item.attributes, attribute);
}

GVariant*
g_lo_menu_get_attribute_value_from_item_in_section (GLOMenu            *menu,
                                                    gint                section,
                                                    gint                position,
                                                    const gchar        *attribute,
                                                    const GVariantType *type)
{
    GMenuModel *model = G_MENU_MODEL (g_lo_menu_get_section (menu, section));

    g_return_val_if_fail (model != nullptr, NULL);

    GVariant *value = g_menu_model_get_item_attribute_value (model,
                                                             position,
                                                             attribute,
                                                             type);

    g_object_unref (model);

    return value;
}

void
g_lo_menu_set_label (GLOMenu     *menu,
                     gint         position,
                     const gchar *label)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    GVariant *value;

    if (label != nullptr)
        value = g_variant_new_string (label);
    else
        value = nullptr;

    g_lo_menu_set_attribute_value (menu, position, G_MENU_ATTRIBUTE_LABEL, value);
}

void
g_lo_menu_set_label_to_item_in_section (GLOMenu     *menu,
                                        gint         section,
                                        gint         position,
                                        const gchar *label)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    GLOMenu *model = g_lo_menu_get_section (menu, section);

    g_return_if_fail (model != nullptr);

    g_lo_menu_set_label (model, position, label);

    // Notify the update.
    g_menu_model_items_changed (G_MENU_MODEL (model), position, 1, 1);

    g_object_unref (model);
}

gchar *
g_lo_menu_get_label_from_item_in_section (GLOMenu *menu,
                                          gint     section,
                                          gint     position)
{
    g_return_val_if_fail (G_IS_LO_MENU (menu), NULL);

    GVariant *label_value = g_lo_menu_get_attribute_value_from_item_in_section (menu,
                                                                                section,
                                                                                position,
                                                                                G_MENU_ATTRIBUTE_LABEL,
                                                                                G_VARIANT_TYPE_STRING);

    gchar *label = nullptr;

    if (label_value)
    {
        label = g_variant_dup_string (label_value, nullptr);
        g_variant_unref (label_value);
    }

    return label;
}

void
g_lo_menu_set_action_and_target_value (GLOMenu     *menu,
                                       gint         position,
                                       const gchar *action,
                                       GVariant    *target_value)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    GVariant *action_value;

    if (action != nullptr)
    {
        action_value = g_variant_new_string (action);
    }
    else
    {
        action_value = nullptr;
        target_value = nullptr;
    }

    g_lo_menu_set_attribute_value (menu, position, G_MENU_ATTRIBUTE_ACTION, action_value);
    g_lo_menu_set_attribute_value (menu, position, G_MENU_ATTRIBUTE_TARGET, target_value);

    g_menu_model_items_changed (G_MENU_MODEL (menu), position, 1, 1);
}

void
g_lo_menu_set_action_and_target_value_to_item_in_section (GLOMenu     *menu,
                                                          gint         section,
                                                          gint         position,
                                                          const gchar *command,
                                                          GVariant    *target_value)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    GLOMenu *model = g_lo_menu_get_section (menu, section);

    g_return_if_fail (model != nullptr);

    g_lo_menu_set_action_and_target_value (model, position, command, target_value);

    g_object_unref (model);
}

void
g_lo_menu_set_accelerator_to_item_in_section (GLOMenu     *menu,
                                              gint         section,
                                              gint         position,
                                              const gchar *accelerator)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    GLOMenu *model = g_lo_menu_get_section (menu, section);

    g_return_if_fail (model != nullptr);

    GVariant *value;

    if (accelerator != nullptr)
        value = g_variant_new_string (accelerator);
    else
        value = nullptr;

    g_lo_menu_set_attribute_value (model, position, G_LO_MENU_ATTRIBUTE_ACCELERATOR, value);

    // Notify the update.
    g_menu_model_items_changed (G_MENU_MODEL (model), position, 1, 1);

    g_object_unref (model);
}

gchar *
g_lo_menu_get_accelerator_from_item_in_section (GLOMenu *menu,
                                                gint     section,
                                                gint     position)
{
    g_return_val_if_fail (G_IS_LO_MENU (menu), NULL);

    GVariant *accel_value = g_lo_menu_get_attribute_value_from_item_in_section (menu,
                                                                                section,
                                                                                position,
                                                                                G_LO_MENU_ATTRIBUTE_ACCELERATOR,
                                                                                G_VARIANT_TYPE_STRING);

    gchar *accel = nullptr;

    if (accel_value != nullptr)
    {
        accel = g_variant_dup_string (accel_value, nullptr);
        g_variant_unref (accel_value);
    }

    return accel;
}

void
g_lo_menu_set_command_to_item_in_section (GLOMenu     *menu,
                                          gint         section,
                                          gint         position,
                                          const gchar *command)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    GLOMenu *model = g_lo_menu_get_section (menu, section);

    g_return_if_fail (model != nullptr);

    GVariant *value;

    if (command != nullptr)
        value = g_variant_new_string (command);
    else
        value = nullptr;

    g_lo_menu_set_attribute_value (model, position, G_LO_MENU_ATTRIBUTE_COMMAND, value);

    // Notify the update.
    g_menu_model_items_changed (G_MENU_MODEL (model), position, 1, 1);

    g_object_unref (model);
}

gchar *
g_lo_menu_get_command_from_item_in_section (GLOMenu *menu,
                                            gint     section,
                                            gint     position)
{
    g_return_val_if_fail (G_IS_LO_MENU (menu), NULL);

    GVariant *command_value = g_lo_menu_get_attribute_value_from_item_in_section (menu,
                                                                                  section,
                                                                                  position,
                                                                                  G_LO_MENU_ATTRIBUTE_COMMAND,
                                                                                  G_VARIANT_TYPE_STRING);

    gchar *command = nullptr;

    if (command_value != nullptr)
    {
        command = g_variant_dup_string (command_value, nullptr);
        g_variant_unref (command_value);
    }

    return command;
}

void
g_lo_menu_set_link (GLOMenu     *menu,
                    gint         position,
                    const gchar *link,
                    GMenuModel  *model)
{
    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (link != nullptr);
    g_return_if_fail (valid_attribute_name (link));

    if (position < 0 || position >= (gint) menu->items->len)
        position = menu->items->len - 1;

    struct item menu_item = g_array_index (menu->items, struct item, position);

    if (model != nullptr)
        g_hash_table_insert (menu_item.links, g_strdup (link), g_object_ref (model));
    else
        g_hash_table_remove (menu_item.links, link);
}

void
g_lo_menu_insert_section (GLOMenu     *menu,
                          gint         position,
                          const gchar *label,
                          GMenuModel  *section)
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

    g_object_unref (section);
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
g_lo_menu_new_submenu_in_item_in_section (GLOMenu *menu,
                                          gint     section,
                                          gint     position)
{
    g_return_if_fail (G_IS_LO_MENU (menu));
    g_return_if_fail (0 <= section && section < (gint) menu->items->len);

    GLOMenu* model = g_lo_menu_get_section (menu, section);

    g_return_if_fail (model != nullptr);

    if (0 <= position && position < (gint) model->items->len) {
        GMenuModel* submenu = G_MENU_MODEL (g_lo_menu_new());

        g_lo_menu_set_link (model, position, G_MENU_LINK_SUBMENU, submenu);

        g_object_unref (submenu);

        g_menu_model_items_changed (G_MENU_MODEL (model), position, 1, 1);

        g_object_unref (model);
    }
}

GLOMenu *
g_lo_menu_get_submenu_from_item_in_section (GLOMenu *menu,
                                            gint     section,
                                            gint     position)
{
    g_return_val_if_fail (G_IS_LO_MENU (menu), NULL);
    g_return_val_if_fail (0 <= section && section < (gint) menu->items->len, NULL);

    GLOMenu *model = g_lo_menu_get_section (menu, section);

    g_return_val_if_fail (model != nullptr, NULL);

    GLOMenu *submenu = nullptr;

    if (0 <= position && position < (gint) model->items->len)
        submenu = G_LO_MENU (G_MENU_MODEL_CLASS (g_lo_menu_parent_class)
                ->get_item_link (G_MENU_MODEL (model), position, G_MENU_LINK_SUBMENU));
        //submenu = g_menu_model_get_item_link (G_MENU_MODEL (model), position, G_MENU_LINK_SUBMENU);

    g_object_unref (model);

    return submenu;
}

void
g_lo_menu_set_submenu_action_to_item_in_section (GLOMenu     *menu,
                                                 gint         section,
                                                 gint         position,
                                                 const gchar *action)
{
    g_return_if_fail (G_IS_LO_MENU (menu));

    GMenuModel *model = G_MENU_MODEL (g_lo_menu_get_section (menu, section));

    g_return_if_fail (model != nullptr);

    GVariant *value;

    if (action != nullptr)
        value = g_variant_new_string (action);
    else
        value = nullptr;

    g_lo_menu_set_attribute_value (G_LO_MENU (model), position, G_LO_MENU_ATTRIBUTE_SUBMENU_ACTION, value);

    // Notify the update.
    g_menu_model_items_changed (model, position, 1, 1);

    g_object_unref (model);
}

static void
g_lo_menu_clear_item (struct item *menu_item)
{
    if (menu_item->attributes != nullptr)
        g_hash_table_unref (menu_item->attributes);
    if (menu_item->links != nullptr)
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

    GLOMenu *model = g_lo_menu_get_section (menu, section);

    g_return_if_fail (model != nullptr);

    g_lo_menu_remove (model, position);

    g_object_unref (model);
}

static void
g_lo_menu_finalize (GObject *object)
{
    GLOMenu *menu = G_LO_MENU (object);
    struct item *items;
    gint n_items;
    gint i;

    n_items = menu->items->len;
    items = reinterpret_cast<struct item *>(g_array_free (menu->items, FALSE));
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
