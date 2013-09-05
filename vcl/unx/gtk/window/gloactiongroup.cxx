/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unx/gtk/gtksalmenu.hxx>

#ifdef ENABLE_GMENU_INTEGRATION

#include <unx/gtk/gloactiongroup.h>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkframe.hxx>


/*
 * GLOAction
 */

#define G_TYPE_LO_ACTION                                (g_lo_action_get_type ())
#define G_LO_ACTION(inst)                               (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                         G_TYPE_LO_ACTION, GLOAction))

struct _GLOAction
{
    GObject         parent_instance;

    gint            item_id;            // Menu item ID.
    gboolean        submenu;            // TRUE if action is a submenu action.
    gboolean        enabled;            // TRUE if action is enabled.
    GVariantType*   parameter_type;     // A GVariantType with the action parameter type.
    GVariantType*   state_type;         // A GVariantType with item state type
    GVariant*       state_hint;         // A GVariant with state hints.
    GVariant*       state;              // A GVariant with current item state
};

typedef GObjectClass GLOActionClass;
typedef struct _GLOAction GLOAction;

G_DEFINE_TYPE (GLOAction, g_lo_action, G_TYPE_OBJECT);

GLOAction*
g_lo_action_new (void)
{
    return G_LO_ACTION (g_object_new (G_TYPE_LO_ACTION, NULL));
}

static void
g_lo_action_init (GLOAction *action)
{
    action->item_id = -1;
    action->submenu = FALSE;
    action->enabled = TRUE;
    action->parameter_type = NULL;
    action->state_type = NULL;
    action->state_hint = NULL;
    action->state = NULL;
}

static void
g_lo_action_finalize (GObject *object)
{
    GLOAction* action = G_LO_ACTION(object);

    if (action->parameter_type)
        g_variant_type_free (action->parameter_type);

    if (action->state_type)
        g_variant_type_free (action->state_type);

    if (action->state_hint)
        g_variant_unref (action->state_hint);

    if (action->state)
        g_variant_unref (action->state);

    G_OBJECT_CLASS (g_lo_action_parent_class)->finalize (object);
}

static void
g_lo_action_class_init (GLOActionClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = g_lo_action_finalize;
}

/*
 * GLOActionGroup
 */

struct _GLOActionGroupPrivate
{
    GHashTable  *table;  /* string -> GLOAction */
    GtkSalFrame *frame;  /* Frame to which GActionGroup is associated. */
};

static void g_lo_action_group_iface_init (GActionGroupInterface *);

G_DEFINE_TYPE_WITH_CODE (GLOActionGroup,
    g_lo_action_group, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_GROUP,
                           g_lo_action_group_iface_init));

static gchar **
g_lo_action_group_list_actions (GActionGroup *group)
{
    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (group);
    GHashTableIter iter;
    gint n, i = 0;
    gchar **keys;
    gpointer key;

    n = g_hash_table_size (loGroup->priv->table);
    keys = g_new (gchar *, n + 1);

    g_hash_table_iter_init (&iter, loGroup->priv->table);
    while (g_hash_table_iter_next (&iter, &key, NULL))
        keys[i++] = g_strdup ((gchar*) key);
    g_assert_cmpint (i, ==, n);
    keys[n] = NULL;

    return keys;
}

static gboolean
g_lo_action_group_query_action (GActionGroup        *group,
                                const gchar         *action_name,
                                gboolean            *enabled,
                                const GVariantType **parameter_type,
                                const GVariantType **state_type,
                                GVariant           **state_hint,
                                GVariant           **state)
{
    //SAL_INFO("vcl.unity", "g_lo_action_group_query_action on " << group);
    GLOActionGroup *lo_group = G_LO_ACTION_GROUP (group);
    GLOAction* action;

    action = G_LO_ACTION (g_hash_table_lookup (lo_group->priv->table, action_name));

    if (action == NULL)
        return FALSE;


    if (enabled)
    {
        GtkSalFrame* pFrame = lo_group->priv->frame;
        if (pFrame) {
            pFrame->EnsureDbusMenuSynced();
        }
        *enabled = action->enabled;
    }

    if (parameter_type)
        *parameter_type = action->parameter_type;

    if (state_type)
        *state_type = action->state_type;

    if (state_hint)
        *state_hint = (action->state_hint) ? g_variant_ref (action->state_hint) : NULL;

    if (state)
        *state = (action->state) ? g_variant_ref (action->state) : NULL;

    return TRUE;
}

static void
g_lo_action_group_perform_submenu_action (GLOActionGroup *group,
                                          const gchar    *action_name,
                                          GVariant       *state)
{

    GtkSalFrame* pFrame = group->priv->frame;
    SAL_INFO("vcl.unity", "g_lo_action_group_perform_submenu_action on " << group << " for frame " << pFrame);

    if (pFrame == NULL)
        return;

    GtkSalMenu* pSalMenu = static_cast<GtkSalMenu*> (pFrame->GetMenu());
    SAL_INFO("vcl.unity", "g_lo_action_group_perform_submenu_action on " << group << " for menu " << pSalMenu);

    if (pSalMenu != NULL) {
        gboolean bState = g_variant_get_boolean (state);
        SAL_INFO("vcl.unity", "g_lo_action_group_perform_submenu_action on " << group << " to " << bState);

        if (bState)
            pSalMenu->Activate();
        else
            pSalMenu->Deactivate (action_name);
    }
}

static void
g_lo_action_group_change_state (GActionGroup *group,
                                const gchar  *action_name,
                                GVariant     *value)
{
    SAL_INFO("vcl.unity", "g_lo_action_group_change_state on " << group );
    g_return_if_fail (value != NULL);

    g_variant_ref_sink (value);

    if (action_name != NULL)
    {
        GLOActionGroup* lo_group = G_LO_ACTION_GROUP (group);
        GLOAction* action = G_LO_ACTION (g_hash_table_lookup (lo_group->priv->table, action_name));

        if (action != NULL)
        {
            if (action->submenu)
                g_lo_action_group_perform_submenu_action (lo_group, action_name, value);
            else
            {
                gboolean is_new = FALSE;

                /* If action already exists but has no state, it should be removed and added again. */
                if (action->state_type == NULL)
                {
                    g_action_group_action_removed (G_ACTION_GROUP (group), action_name);
                    action->state_type = g_variant_type_copy (g_variant_get_type(value));
                    is_new = TRUE;
                }

                if (g_variant_is_of_type (value, action->state_type))
                {
                    if (action->state)
                        g_variant_unref(action->state);

                    action->state = g_variant_ref (value);

                    if (is_new)
                        g_action_group_action_added (G_ACTION_GROUP (group), action_name);
                    else
                        g_action_group_action_state_changed (group, action_name, value);
                }
            }
        }
    }

    g_variant_unref (value);
}

static void
g_lo_action_group_activate (GActionGroup *group,
                            const gchar  *action_name,
                            GVariant     *parameter)
{
    GLOActionGroup *lo_group = G_LO_ACTION_GROUP (group);
    GtkSalFrame *pFrame = lo_group->priv->frame;
    SAL_INFO("vcl.unity", "g_lo_action_group_activate on group " << group << " for frame " << pFrame << " with parameter " << parameter);

    if ( parameter != NULL )
        g_action_group_change_action_state( group, action_name, parameter );

    if ( pFrame != NULL )
    {
        GtkSalMenu* pSalMenu = static_cast< GtkSalMenu* >( pFrame->GetMenu() );
        SAL_INFO("vcl.unity", "g_lo_action_group_activate for menu " << pSalMenu);

        if ( pSalMenu != NULL )
        {
            GLOAction* action = G_LO_ACTION (g_hash_table_lookup (lo_group->priv->table, action_name));
            SAL_INFO("vcl.unity", "g_lo_action_group_activate dispatching action " << action << " named " << action_name << " on menu " << pSalMenu);
            pSalMenu->DispatchCommand( action->item_id, action_name );
        }
    }
}

void
g_lo_action_group_insert (GLOActionGroup *group,
                          const gchar    *action_name,
                          gint            item_id,
                          gboolean        submenu)
{
    g_lo_action_group_insert_stateful (group, action_name, item_id, submenu, NULL, NULL, NULL, NULL);
}

void
g_lo_action_group_insert_stateful (GLOActionGroup     *group,
                                   const gchar        *action_name,
                                   gint                item_id,
                                   gboolean            submenu,
                                   const GVariantType *parameter_type,
                                   const GVariantType *state_type,
                                   GVariant           *state_hint,
                                   GVariant           *state)
{
    g_return_if_fail (G_IS_LO_ACTION_GROUP (group));

    GLOAction* old_action = G_LO_ACTION (g_hash_table_lookup (group->priv->table, action_name));

    if (old_action == NULL || old_action->item_id != item_id)
    {
        if (old_action != NULL)
            g_lo_action_group_remove (group, action_name);
//            g_action_group_action_removed (G_ACTION_GROUP (group), action_name);

        GLOAction* action = g_lo_action_new();

        g_hash_table_insert (group->priv->table, g_strdup (action_name), action);

        action->item_id = item_id;
        action->submenu = submenu;

        if (parameter_type)
            action->parameter_type = (GVariantType*) parameter_type;

        if (state_type)
            action->state_type = (GVariantType*) state_type;

        if (state_hint)
            action->state_hint = g_variant_ref_sink (state_hint);

        if (state)
            action->state = g_variant_ref_sink (state);

        g_action_group_action_added (G_ACTION_GROUP (group), action_name);
    }
}

static void
g_lo_action_group_finalize (GObject *object)
{
    GLOActionGroup *lo_group = G_LO_ACTION_GROUP (object);

    g_hash_table_unref (lo_group->priv->table);

    G_OBJECT_CLASS (g_lo_action_group_parent_class)->finalize (object);
}

static void
g_lo_action_group_init (GLOActionGroup *group)
{
    SAL_INFO("vcl.unity", "g_lo_action_group_init on " << group);
    group->priv = G_TYPE_INSTANCE_GET_PRIVATE (group,
                                                 G_TYPE_LO_ACTION_GROUP,
                                                 GLOActionGroupPrivate);
    group->priv->table = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free, g_object_unref);
    group->priv->frame = NULL;
}

static void
g_lo_action_group_class_init (GLOActionGroupClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = g_lo_action_group_finalize;

    g_type_class_add_private (klass, sizeof (GLOActionGroupPrivate));
}

static void
g_lo_action_group_iface_init (GActionGroupInterface *iface)
{
    iface->list_actions = g_lo_action_group_list_actions;
    iface->query_action = g_lo_action_group_query_action;
    iface->change_action_state = g_lo_action_group_change_state;
    iface->activate_action = g_lo_action_group_activate;
}

GLOActionGroup *
g_lo_action_group_new (gpointer frame)
{
    GLOActionGroup* group = G_LO_ACTION_GROUP (g_object_new (G_TYPE_LO_ACTION_GROUP, NULL));
    group->priv->frame = static_cast< GtkSalFrame* > (frame);

    return group;
}

void
g_lo_action_group_set_action_enabled (GLOActionGroup *group,
                                      const gchar    *action_name,
                                      gboolean        enabled)
{
    SAL_INFO("vcl.unity", "g_lo_action_group_set_action_enabled on " << group);
    g_return_if_fail (G_IS_LO_ACTION_GROUP (group));
    g_return_if_fail (action_name != NULL);

    GLOAction* action = G_LO_ACTION (g_hash_table_lookup (group->priv->table, action_name));

    if (action == NULL)
        return;

    action->enabled = enabled;

    g_action_group_action_enabled_changed (G_ACTION_GROUP (group), action_name, enabled);
}

void
g_lo_action_group_remove (GLOActionGroup *group,
                          const gchar    *action_name)
{
    SAL_INFO("vcl.unity", "g_lo_action_group_remove on " << group);
    g_return_if_fail (G_IS_LO_ACTION_GROUP (group));

    if (action_name != NULL)
    {
        g_action_group_action_removed (G_ACTION_GROUP (group), action_name);
        g_hash_table_remove (group->priv->table, action_name);
    }
}

void
g_lo_action_group_clear (GLOActionGroup  *group)
{
    SAL_INFO("vcl.unity", "g_lo_action_group_clear on " << group);
    g_return_if_fail (G_IS_LO_ACTION_GROUP (group));

    GList* keys = g_hash_table_get_keys (group->priv->table);

    for (GList* element = g_list_first (keys); element != NULL; element = g_list_next (element))
    {
        g_lo_action_group_remove (group, (gchar*) element->data);
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
