#include <unx/gtk/gloactiongroup.h>

#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtksalmenu.hxx>
#include <vcl/menu.hxx>

#include <stdio.h>
#include <iostream>

using namespace std;

struct _GLOActionGroupPrivate
{
    GHashTable *table;  /* string -> GtkSalMenuItem* */
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
//    printf("%s - %s\n", __FUNCTION__, action_name);

    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (group);
    GtkSalMenuItem* item_info;

    item_info = static_cast< GtkSalMenuItem* >( g_hash_table_lookup (loGroup->priv->table, action_name) );

    if (item_info == NULL)
        return FALSE;

    if (enabled) {
        sal_Bool bEnabled = item_info->mpVCLMenu->IsItemEnabled( item_info->mnId );
        *enabled = (bEnabled) ? TRUE : FALSE;
    }

    if (parameter_type)
        *parameter_type = NULL;

    if (state_type)
        *state_type = item_info->mpStateType;

    if (state_hint)
        *state_hint = NULL;

    if (state)
        *state = item_info->mpState;

    return TRUE;
}

static void
g_lo_action_group_change_state (GActionGroup *group,
                                const gchar  *action_name,
                                GVariant     *value)
{
    if (!action_name || !value)
        return;

    GLOActionGroup* lo_group = G_LO_ACTION_GROUP (group);
    GtkSalMenuItem* item_info;

    item_info = static_cast<GtkSalMenuItem*>( g_hash_table_lookup (lo_group->priv->table, action_name) );

    if (!item_info)
        return;

    if (!item_info->mpStateType) {
        item_info->mpStateType = g_variant_type_copy(g_variant_get_type(value));
    }

    if (g_variant_is_of_type(value, item_info->mpStateType)) {
        item_info->mpState = g_variant_ref_sink(value);
        g_action_group_action_state_changed(group, action_name, value);
    }
}

static void
g_lo_action_group_activate (GActionGroup *group,
                            const gchar  *action_name,
                            GVariant     *parameter)
{
    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (group);
    GtkSalMenuItem *pSalMenuItem;

    pSalMenuItem = static_cast< GtkSalMenuItem* >( g_hash_table_lookup (loGroup->priv->table, action_name) );

    if (pSalMenuItem == NULL || pSalMenuItem->mpSubMenu )
        return;

    GTK_YIELD_GRAB();

    const GtkSalFrame *pFrame = pSalMenuItem->mpParentMenu ? pSalMenuItem->mpParentMenu->GetFrame() : NULL;

    if ( pFrame && !pFrame->GetParent() ) {
        ((PopupMenu*) pSalMenuItem->mpVCLMenu)->SetSelectedEntry( pSalMenuItem->mnId );
        SalMenuEvent aMenuEvt( pSalMenuItem->mnId, pSalMenuItem->mpVCLMenu );
        pFrame->CallCallback( SALEVENT_MENUCOMMAND, &aMenuEvt );
    }
    else if ( pSalMenuItem->mpVCLMenu )
    {
        // if an item from submenu was selected. the corresponding Window does not exist because
        // we use native popup menus, so we have to set the selected menuitem directly
        // incidentally this of course works for top level popup menus, too
        PopupMenu * pPopupMenu = dynamic_cast<PopupMenu *>(pSalMenuItem->mpVCLMenu);
        if( pPopupMenu )
        {
            // FIXME: revise this ugly code

            // select handlers in vcl are dispatch on the original menu
            // if not consumed by the select handler of the current menu
            // however since only the starting menu ever came into Execute
            // the hierarchy is not build up. Workaround this by getting
            // the menu it should have been

            // get started from hierarchy in vcl menus
            GtkSalMenu* pParentMenu = pSalMenuItem->mpParentMenu;
            Menu* pCurMenu = pSalMenuItem->mpVCLMenu;
            while( pParentMenu && pParentMenu->GetMenu() )
            {
                pCurMenu = pParentMenu->GetMenu();
                pParentMenu = pParentMenu->GetParentSalMenu();
            }

            pPopupMenu->SetSelectedEntry( pSalMenuItem->mnId );
            pPopupMenu->ImplSelectWithStart( pCurMenu );
        }
        else
        {
            OSL_FAIL( "menubar item without frame !" );
        }
    }
}

void
g_lo_action_group_insert (GLOActionGroup *group,
                          const gchar    *action_name,
                          gpointer        action_info)
{
    g_return_if_fail (G_IS_LO_ACTION_GROUP (group));

    gpointer old_action;

    old_action = g_hash_table_lookup (group->priv->table, action_name);

    if (old_action != action_info)
    {
        if (old_action != NULL)
            g_action_group_action_removed (G_ACTION_GROUP (group), action_name);

        g_hash_table_insert (group->priv->table, g_strdup (action_name), action_info);

        g_action_group_action_added (G_ACTION_GROUP (group), action_name);
    }
}

static void
g_lo_action_group_finalize (GObject *object)
{
    GLOActionGroup *loGroup = G_LO_ACTION_GROUP (object);

    g_hash_table_unref (loGroup->priv->table);

    G_OBJECT_CLASS (g_lo_action_group_parent_class)->finalize (object);
}

static void
g_lo_action_group_init (GLOActionGroup *group)
{
    group->priv = G_TYPE_INSTANCE_GET_PRIVATE (group,
                                                 G_TYPE_LO_ACTION_GROUP,
                                                 GLOActionGroupPrivate);
    group->priv->table = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free, NULL );
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
g_lo_action_group_new (void)
{
    return G_LO_ACTION_GROUP( g_object_new (G_TYPE_LO_ACTION_GROUP, NULL) );
}

void
g_lo_action_group_set_action_enabled (GLOActionGroup *group,
                                      const gchar    *action_name,
                                      gboolean        enabled)
{
    g_return_if_fail (G_IS_LO_ACTION_GROUP (group));

    g_action_group_action_enabled_changed(G_ACTION_GROUP(group),
                                          action_name,
                                          enabled);

}

void
g_lo_action_group_remove (GLOActionGroup *group,
                          const gchar    *action_name)
{
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
    g_return_if_fail (G_IS_LO_ACTION_GROUP (group));

    GList* keys = g_hash_table_get_keys(group->priv->table);

    for ( GList* list = g_list_first(keys); list; list = g_list_next(list) ) {
        gchar* action_name = (gchar*) list->data;

        g_lo_action_group_remove( group, action_name );
    }
}
