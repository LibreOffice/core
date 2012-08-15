#include <unx/gtk/gloactiongroup.h>

//#include "gsimpleaction.h"
//#include "gactionmap.h"
//#include "gaction.h"

#include <stdio.h>


struct _GLOActionGroupPrivate
{
    GHashTable *table;  /* string -> GAction */
};

static void g_lo_action_group_iface_init (GActionGroupInterface *);
static void g_lo_action_group_map_iface_init (GActionMapInterface *);

G_DEFINE_TYPE_WITH_CODE (GLOActionGroup,
    g_lo_action_group, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_GROUP,
                           g_lo_action_group_iface_init);
    G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_MAP,
                           g_lo_action_group_map_iface_init))

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
    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (group);
    GAction *action;

    action = G_ACTION( g_hash_table_lookup (loGroup->priv->table, action_name) );

    if (action == NULL)
        return FALSE;

    if (enabled)
        *enabled = g_action_get_enabled (action);

    if (parameter_type)
        *parameter_type = g_action_get_parameter_type (action);

    if (state_type)
        *state_type = g_action_get_state_type (action);

    if (state_hint)
        *state_hint = g_action_get_state_hint (action);

    if (state)
        *state = g_action_get_state (action);

    return TRUE;
}

static void
g_lo_action_group_change_state (GActionGroup *group,
                                const gchar  *action_name,
                                GVariant     *value)
{
    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (group);
    GAction *action;

    action = G_ACTION( g_hash_table_lookup (loGroup->priv->table, action_name) );

    if (action == NULL)
        return;

    g_action_change_state (action, value);
}

static void
g_lo_action_group_activate (GActionGroup *group,
                            const gchar  *action_name,
                            GVariant     *parameter)
{
    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (group);
    GAction *action;

    action = G_ACTION( g_hash_table_lookup (loGroup->priv->table, action_name) );

    if (action == NULL)
        return;

    g_action_activate (action, parameter);
}

static void
action_enabled_notify (GAction     *action,
                       GParamSpec  *pspec,
                       gpointer     user_data)
{
    g_action_group_action_enabled_changed (G_ACTION_GROUP( user_data ),
                                           g_action_get_name (action),
                                           g_action_get_enabled (action));
}

static void
action_state_notify (GAction    *action,
                     GParamSpec *pspec,
                     gpointer    user_data)
{
    GVariant *value;

    value = g_action_get_state (action);
    g_action_group_action_state_changed (G_ACTION_GROUP( user_data ),
                                         g_action_get_name (action),
                                         value);
    g_variant_unref (value);
}

static void
g_lo_action_group_disconnect (gpointer key,
                              gpointer value,
                              gpointer user_data)
{
    g_signal_handlers_disconnect_by_func (value, (gpointer) action_enabled_notify,
                                          user_data);
    g_signal_handlers_disconnect_by_func (value, (gpointer) action_state_notify,
                                          user_data);
}

static GAction *
g_lo_action_group_lookup_action (GActionMap     *action_map,
                                 const gchar    *action_name)
{
    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (action_map);

    return G_ACTION( g_hash_table_lookup (loGroup->priv->table, action_name) );
}

static void
g_lo_action_group_add_action (GActionMap *action_map,
                              GAction    *action)
{
    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (action_map);
    const gchar *action_name;
    GAction *old_action;

    action_name = g_action_get_name (action);
    old_action = G_ACTION( g_hash_table_lookup (loGroup->priv->table, action_name) );

    if (old_action != action)
    {
        if (old_action != NULL)
        {
            g_action_group_action_removed (G_ACTION_GROUP (loGroup),
                                           action_name);
            g_lo_action_group_disconnect (NULL, old_action, loGroup);
        }

        g_signal_connect (action, "notify::enabled",
                          G_CALLBACK (action_enabled_notify), loGroup);

        if (g_action_get_state_type (action) != NULL)
            g_signal_connect (action, "notify::state",
                              G_CALLBACK (action_state_notify), loGroup);

        g_hash_table_insert (loGroup->priv->table,
                             g_strdup (action_name),
                             g_object_ref (action));

        g_action_group_action_added (G_ACTION_GROUP (loGroup), action_name);
    }
}

static void
g_lo_action_group_remove_action (GActionMap  *action_map,
                                 const gchar *action_name)
{
    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (action_map);
    GAction *action;

    action = G_ACTION( g_hash_table_lookup (loGroup->priv->table, action_name) );

    if (action != NULL)
    {
        g_action_group_action_removed (G_ACTION_GROUP (loGroup), action_name);
        g_lo_action_group_disconnect (NULL, action, loGroup);
        g_hash_table_remove (loGroup->priv->table, action_name);
    }
}

static void
g_lo_action_group_finalize (GObject *object)
{
    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (object);

    g_hash_table_foreach (loGroup->priv->table,
                          g_lo_action_group_disconnect,
                          loGroup);
    g_hash_table_unref (loGroup->priv->table);

    G_OBJECT_CLASS (g_lo_action_group_parent_class)
            ->finalize (object);
}

static void
g_lo_action_group_init (GLOActionGroup *group)
{
    group->priv = G_TYPE_INSTANCE_GET_PRIVATE (group,
                                                 G_TYPE_LO_ACTION_GROUP,
                                                 GLOActionGroupPrivate);
    group->priv->table = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free, g_object_unref);
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

static void
g_lo_action_group_map_iface_init (GActionMapInterface *iface)
{
    iface->add_action = g_lo_action_group_add_action;
    iface->remove_action = g_lo_action_group_remove_action;
    iface->lookup_action = g_lo_action_group_lookup_action;
}

GLOActionGroup *
g_lo_action_group_new (void)
{
    return G_LO_ACTION_GROUP( g_object_new (G_TYPE_LO_ACTION_GROUP, NULL) );
}

GAction *
g_lo_action_group_lookup (GLOActionGroup *group,
                          const gchar    *action_name)
{
    g_return_val_if_fail (G_IS_LO_ACTION_GROUP (group), NULL);

    return g_action_map_lookup_action (G_ACTION_MAP (group), action_name);
}

void
g_lo_action_group_insert (GLOActionGroup *group,
                          GAction        *action)
{
    g_return_if_fail (G_IS_LO_ACTION_GROUP (group));

    g_action_map_add_action (G_ACTION_MAP (group), action);
}

void
g_lo_action_group_remove (GLOActionGroup *group,
                          const gchar    *action_name)
{
    g_return_if_fail (G_IS_LO_ACTION_GROUP (group));

    g_action_map_remove_action (G_ACTION_MAP (group), action_name);
}

// This function has been added to make current implementation of GtkSalMenu work.
void
g_lo_action_group_clear (GLOActionGroup  *group)
{
    g_return_if_fail (G_IS_LO_ACTION_GROUP (group));

    GAction *action;

    GList* keys = g_hash_table_get_keys(group->priv->table);

    for ( GList* list = g_list_first(keys); list; list = g_list_next(list)) {
        gchar* action_name = (gchar*) list->data;
        action = G_ACTION( g_hash_table_lookup (group->priv->table, action_name) );

        if (action != NULL)
        {
            g_action_group_action_removed (G_ACTION_GROUP (group), action_name);
            g_lo_action_group_disconnect (NULL, action, group);
            g_hash_table_remove (group->priv->table, action_name);
        }
    }
}

void
g_lo_action_group_add_entries (GLOActionGroup     *group,
                               const GActionEntry *entries,
                               gint                n_entries,
                               gpointer            user_data)
{
    g_action_map_add_action_entries (G_ACTION_MAP (group), entries, n_entries, user_data);
}

