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

#include <unx/gtk/gtksalmenu.hxx>

#include <unx/gtk/glomenu.h>
#include <unx/gtk/gloactiongroup.h>
#include <vcl/menu.hxx>
#include <unx/gtk/gtkinst.hxx>

#if GTK_CHECK_VERSION(3,0,0)
#  include <gdk/gdkkeysyms-compat.h>
#endif

#include <svtools/menuoptions.hxx>

#include <framework/menuconfiguration.hxx>

#include <sal/log.hxx>

static gchar* GetCommandForSpecialItem( GtkSalMenuItem* pSalMenuItem )
{
    gchar* aCommand = NULL;

    sal_uInt16 nId = pSalMenuItem->mnId;

    // If item belongs to window list, generate a command with "window-(id)" format.
    if ( ( nId >= START_ITEMID_WINDOWLIST ) && ( nId <= END_ITEMID_WINDOWLIST ) )
    {
        aCommand = g_strdup_printf( "window-%d", nId );
    }

    return aCommand;
}

static void KeyCodeToGdkKey ( const KeyCode& rKeyCode, guint* pGdkKeyCode, GdkModifierType *pGdkModifiers )
{
    if ( pGdkKeyCode == NULL || pGdkModifiers == NULL )
        return;

    // Get GDK key modifiers
    GdkModifierType nModifiers = (GdkModifierType) 0;

    if ( rKeyCode.IsShift() )
        nModifiers = (GdkModifierType) ( nModifiers | GDK_SHIFT_MASK );

    if ( rKeyCode.IsMod1() )
        nModifiers = (GdkModifierType) ( nModifiers | GDK_CONTROL_MASK );

    if ( rKeyCode.IsMod2() )
        nModifiers = (GdkModifierType) ( nModifiers | GDK_MOD1_MASK );

    *pGdkModifiers = nModifiers;

    // Get GDK keycode.
    guint nKeyCode = 0;

    guint nCode = rKeyCode.GetCode();

    if ( nCode >= KEY_0 && nCode <= KEY_9 )
        nKeyCode = ( nCode - KEY_0 ) + GDK_0;
    else if ( nCode >= KEY_A && nCode <= KEY_Z )
        nKeyCode = ( nCode - KEY_A ) + GDK_A;
    else if ( nCode >= KEY_F1 && nCode <= KEY_F26 )
        nKeyCode = ( nCode - KEY_F1 ) + GDK_F1;
    else
    {
        switch( nCode )
        {
        case KEY_DOWN:          nKeyCode = GDK_Down;            break;
        case KEY_UP:            nKeyCode = GDK_Up;              break;
        case KEY_LEFT:          nKeyCode = GDK_Left;            break;
        case KEY_RIGHT:         nKeyCode = GDK_Right;           break;
        case KEY_HOME:          nKeyCode = GDK_Home;            break;
        case KEY_END:           nKeyCode = GDK_End;             break;
        case KEY_PAGEUP:        nKeyCode = GDK_Page_Up;         break;
        case KEY_PAGEDOWN:      nKeyCode = GDK_Page_Down;       break;
        case KEY_RETURN:        nKeyCode = GDK_Return;          break;
        case KEY_ESCAPE:        nKeyCode = GDK_Escape;          break;
        case KEY_TAB:           nKeyCode = GDK_Tab;             break;
        case KEY_BACKSPACE:     nKeyCode = GDK_BackSpace;       break;
        case KEY_SPACE:         nKeyCode = GDK_space;           break;
        case KEY_INSERT:        nKeyCode = GDK_Insert;          break;
        case KEY_DELETE:        nKeyCode = GDK_Delete;          break;
        case KEY_ADD:           nKeyCode = GDK_plus;            break;
        case KEY_SUBTRACT:      nKeyCode = GDK_minus;           break;
        case KEY_MULTIPLY:      nKeyCode = GDK_asterisk;        break;
        case KEY_DIVIDE:        nKeyCode = GDK_slash;           break;
        case KEY_POINT:         nKeyCode = GDK_period;          break;
        case KEY_COMMA:         nKeyCode = GDK_comma;           break;
        case KEY_LESS:          nKeyCode = GDK_less;            break;
        case KEY_GREATER:       nKeyCode = GDK_greater;         break;
        case KEY_EQUAL:         nKeyCode = GDK_equal;           break;
        case KEY_FIND:          nKeyCode = GDK_Find;            break;
        case KEY_CONTEXTMENU:   nKeyCode = GDK_Menu;            break;
        case KEY_HELP:          nKeyCode = GDK_Help;            break;
        case KEY_UNDO:          nKeyCode = GDK_Undo;            break;
        case KEY_REPEAT:        nKeyCode = GDK_Redo;            break;
        case KEY_DECIMAL:       nKeyCode = GDK_KP_Decimal;      break;
        case KEY_TILDE:         nKeyCode = GDK_asciitilde;      break;
        case KEY_QUOTELEFT:     nKeyCode = GDK_quoteleft;       break;
        case KEY_BRACKETLEFT:   nKeyCode = GDK_bracketleft;     break;
        case KEY_BRACKETRIGHT:  nKeyCode = GDK_bracketright;    break;
        case KEY_SEMICOLON:     nKeyCode = GDK_semicolon;       break;

        // Special cases
        case KEY_COPY:          nKeyCode = GDK_Copy;            break;
        case KEY_CUT:           nKeyCode = GDK_Cut;             break;
        case KEY_PASTE:         nKeyCode = GDK_Paste;           break;
        case KEY_OPEN:          nKeyCode = GDK_Open;            break;
        }
    }

    *pGdkKeyCode = nKeyCode;
}

bool GtkSalMenu::PrepUpdate()
{
    const GtkSalFrame* pFrame = GetFrame();
    if (pFrame)
    {
        const GObject* pWindow = G_OBJECT(gtk_widget_get_window( GTK_WIDGET(pFrame->getWindow()) ));
        if(!pWindow)
        {
            SAL_INFO("vcl.unity", "not updating menu model, I have no frame " << mpMenuModel);
            return false;
        }
    
        // the root menu does not have its own model and has to use the one owned by the frame
        if(mbMenuBar)
        {
            mpMenuModel = G_MENU_MODEL( g_object_get_data( G_OBJECT( pWindow ), "g-lo-menubar" ) );
            mpActionGroup = G_ACTION_GROUP( g_object_get_data( G_OBJECT( pWindow ), "g-lo-action-group" ) );
        
            if(!mpMenuModel || !mpActionGroup)
                return false;
        } 
        
        SAL_INFO("vcl.unity", "updating menu model" << mpMenuModel);
        
        return true;
    }

    return false;
}

/*
 * Menu updating methods
 */

void RemoveSpareItemsFromNativeMenu( GLOMenu* pMenu, GList** pOldCommandList, unsigned nSection, unsigned nValidItems )
{
    sal_Int32 nSectionItems = g_lo_menu_get_n_items_from_section( pMenu, nSection );

    while ( nSectionItems > (sal_Int32) nValidItems )
    {
        gchar* aCommand = g_lo_menu_get_command_from_item_in_section( pMenu, nSection, --nSectionItems );

        if ( aCommand != NULL && pOldCommandList != NULL )
            *pOldCommandList = g_list_append( *pOldCommandList, g_strdup( aCommand ) );

        g_free( aCommand );

        g_lo_menu_remove_from_section( pMenu, nSection, nSectionItems );
    }
}

void RemoveSpareSectionsFromNativeMenu( GLOMenu* pMenu, GList** pOldCommandList, unsigned nLastSection )
{
    if ( pMenu == NULL || pOldCommandList == NULL )
        return;

    sal_Int32 n = g_menu_model_get_n_items( G_MENU_MODEL( pMenu ) ) - 1;

    for ( ; n > (sal_Int32) nLastSection; n-- )
    {
        RemoveSpareItemsFromNativeMenu( pMenu, pOldCommandList, n, 0 );
        g_lo_menu_remove( pMenu, n );
    }
}

gint CompareStr( gpointer str1, gpointer str2 )
{
    return g_strcmp0( (const gchar*) str1, (const gchar*) str2 );
}

void RemoveUnusedCommands( GLOActionGroup* pActionGroup, GList* pOldCommandList, GList* pNewCommandList )
{
    if ( pActionGroup == NULL || pOldCommandList == NULL )
        return;

    while ( pNewCommandList != NULL )
    {
        GList* pNewCommand = g_list_first( pNewCommandList );
        pNewCommandList = g_list_remove_link( pNewCommandList, pNewCommand );

        gpointer aCommand = g_list_nth_data( pNewCommand, 0 );

        GList* pOldCommand = g_list_find_custom( pOldCommandList, aCommand, (GCompareFunc) CompareStr );

        if ( pOldCommand != NULL )
        {
            pOldCommandList = g_list_remove_link( pOldCommandList, pOldCommand );
            g_list_free_full( pOldCommand, g_free );
        }

        g_list_free_full( pNewCommand, g_free );
    }

    while ( pOldCommandList != NULL )
    {
        GList* pCommand = g_list_first( pOldCommandList );
        pOldCommandList = g_list_remove_link( pOldCommandList, pCommand );

        gchar* aCommand = (gchar*) g_list_nth_data( pCommand, 0 );

        g_lo_action_group_remove( pActionGroup, aCommand );

        g_list_free_full( pCommand, g_free );
    }
}

void GtkSalMenu::UpdateNativeMenu()
{
    if( !PrepUpdate() )
        return;

//    SolarMutexGuard aGuard;

    Menu* pVCLMenu = mpVCLMenu; // pMenu->GetMenu();
    GLOMenu* pLOMenu = G_LO_MENU( mpMenuModel ); // G_LO_MENU( pMenu->GetMenuModel() );
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( mpActionGroup ); // G_LO_ACTION_GROUP( pMenu->GetActionGroup() );
    GList *pOldCommandList = NULL;
    GList *pNewCommandList = NULL;

    sal_uInt16 nLOMenuSize = g_menu_model_get_n_items( G_MENU_MODEL( pLOMenu ) );

    if ( nLOMenuSize == 0 )
        g_lo_menu_new_section( pLOMenu, 0, NULL );

    sal_Int32 nSection = 0;
    sal_Int32 nItemPos = 0;
    sal_Int32 validItems = 0;
    sal_Int32 nItem;

    for ( nItem = 0; nItem < ( sal_Int32 ) GetItemCount(); nItem++ ) {
        if ( IsItemVisible( nItem ) == sal_False )
            continue;

        GtkSalMenuItem *pSalMenuItem = GetItemAtPos( nItem );
        sal_uInt16 nId = pSalMenuItem->mnId;

        if ( pSalMenuItem->mnType == MENUITEM_SEPARATOR )
        {
            // Delete extra items from current section.
            RemoveSpareItemsFromNativeMenu( pLOMenu, &pOldCommandList, nSection, validItems );

            nSection++;
            nItemPos = 0;
            validItems = 0;

            if ( nLOMenuSize <= nSection )
            {
                g_lo_menu_new_section( pLOMenu, nSection, NULL );
                nLOMenuSize++;
            }

            continue;
        }

        if ( nItemPos >= g_lo_menu_get_n_items_from_section( pLOMenu, nSection ) )
            g_lo_menu_insert_in_section( pLOMenu, nSection, nItemPos, "EMPTY STRING" );

        // Get internal menu item values.
        String aText = pVCLMenu->GetItemText( nId );
        rtl::OUString aCommand( pVCLMenu->GetItemCommand( nId ) );
        sal_Bool itemEnabled = pVCLMenu->IsItemEnabled( nId );
        KeyCode nAccelKey = pVCLMenu->GetAccelKey( nId );
        sal_Bool itemChecked = pVCLMenu->IsItemChecked( nId );
        MenuItemBits itemBits = pVCLMenu->GetItemBits( nId );

        // Convert internal values to native values.
        gboolean bChecked = ( itemChecked == sal_True ) ? TRUE : FALSE;
        gboolean bEnabled = ( itemEnabled == sal_True ) ? TRUE : FALSE;
        gchar* aNativeCommand = g_strdup( rtl::OUStringToOString( aCommand, RTL_TEXTENCODING_UTF8 ).getStr() );

        // Store current item command in command list.
        gchar *aCurrentCommand = g_lo_menu_get_command_from_item_in_section( pLOMenu, nSection, nItemPos );

        if ( aCurrentCommand != NULL )
            pOldCommandList = g_list_append( pOldCommandList, aCurrentCommand );

        // Force updating of native menu labels.
        NativeSetItemText( nSection, nItemPos, aText );
        NativeSetAccelerator( nSection, nItemPos, nAccelKey, nAccelKey.GetName( GetFrame()->GetWindow() ) );

        // Some items are special, so they have different commands.
        if ( g_strcmp0( aNativeCommand, "" ) == 0 )
        {
            gchar *aSpecialItemCmd = GetCommandForSpecialItem( pSalMenuItem );

            if ( aSpecialItemCmd != NULL )
            {
                g_free( aNativeCommand );
                aNativeCommand = aSpecialItemCmd;
            }
        }

        if ( g_strcmp0( aNativeCommand, "" ) != 0 && pSalMenuItem->mpSubMenu == NULL )
        {
            NativeSetItemCommand( nSection, nItemPos, nId, aNativeCommand, itemBits, bChecked, FALSE );
            NativeCheckItem( nSection, nItemPos, itemBits, bChecked );
            NativeSetEnableItem( aNativeCommand, bEnabled );

            pNewCommandList = g_list_append( pNewCommandList, g_strdup( aNativeCommand ) );
        }

        GtkSalMenu* pSubmenu = pSalMenuItem->mpSubMenu;
        GLOMenu* pSubMenuModel = g_lo_menu_get_submenu_from_item_in_section( pLOMenu, nSection, nItemPos );

        if ( pSubmenu && pSubmenu->GetMenu() )
        {
            NativeSetItemCommand( nSection, nItemPos, nId, aNativeCommand, itemBits, FALSE, TRUE );
            pNewCommandList = g_list_append( pNewCommandList, g_strdup( aNativeCommand ) );

            //GLOMenu* pSubMenuModel = g_lo_menu_get_submenu_from_item_in_section( pLOMenu, nSection, nItemPos );

            if ( pSubMenuModel == NULL )
            {
                pSubMenuModel = g_lo_menu_new();
                g_lo_menu_set_submenu_to_item_in_section( pLOMenu, nSection, nItemPos, G_MENU_MODEL( pSubMenuModel ) );
            }

            g_object_unref( pSubMenuModel );

            pSubmenu->SetMenuModel( G_MENU_MODEL( pSubMenuModel ) );
            pSubmenu->SetActionGroup( G_ACTION_GROUP( pActionGroup ) );

            pSubmenu->GetMenu()->Activate();
            pSubmenu->GetMenu()->Deactivate();

            pSubmenu->UpdateNativeMenu();
        }
        else if (pSubMenuModel)
        {
            g_lo_menu_set_submenu_to_item_in_section( pLOMenu, nSection, nItemPos, NULL );
        };

        g_free( aNativeCommand );

        ++nItemPos;
        ++validItems;
    }
//    while ( nItemPos < g_lo_menu_get_n_items_from_section( pLOMenu, nSection ) )
//        g_lo_menu_remove_from_section( pLOMenu, nSection, nItemPos );
//    ++nSection;
//    if ( nSection < g_menu_model_get_n_items( G_MENU_MODEL( pLOMenu ) ) )
//    {
//        SAL_INFO("vcl.unity", "nSection " << nSection << " model sections " << g_menu_model_get_n_items( G_MENU_MODEL( pLOMenu ) ));
//        g_lo_menu_remove(pLOMenu, nSection );
//    }

    // Delete extra items in last section.
    RemoveSpareItemsFromNativeMenu( pLOMenu, &pOldCommandList, nSection, validItems );

    // Delete extra sections.
    RemoveSpareSectionsFromNativeMenu( pLOMenu, &pOldCommandList, nSection );

    // Delete unused commands.
    RemoveUnusedCommands( pActionGroup, pOldCommandList, pNewCommandList );
}

void GtkSalMenu::DisconnectFrame()
{
    if(mbMenuBar)
    {
        mpMenuModel = NULL;
        mpActionGroup = NULL;
        mpFrame = NULL;
    }
}


/*
 * GtkSalMenu
 */

//void GtkSalMenu::UpdateNativeMenu()
//{
//    UpdateNativeSubMenu();
//}

GtkSalMenu::GtkSalMenu( sal_Bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mpVCLMenu( NULL ),
    mpParentSalMenu( NULL ),
    mpFrame( NULL ),
    mpMenuModel( NULL ),
    mpActionGroup( NULL )
{
}

GtkSalMenu::~GtkSalMenu()
{
    if ( mbMenuBar == sal_True ) {
        ((GtkSalFrame*) mpFrame)->SetMenu( NULL );
    }

    maItems.clear();
}

sal_Bool GtkSalMenu::VisibleMenuBar()
{
    return true;
}

void GtkSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    GtkSalMenuItem *pItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );

    if ( nPos == MENU_APPEND )
        maItems.push_back( pItem );
    else
        maItems.insert( maItems.begin() + nPos, pItem );

    pItem->mpParentMenu = this;
}

void GtkSalMenu::RemoveItem( unsigned nPos )
{
    maItems.erase( maItems.begin() + nPos );
}

void GtkSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
    GtkSalMenuItem *pItem = static_cast< GtkSalMenuItem* >( pSalMenuItem );
    GtkSalMenu *pGtkSubMenu = static_cast< GtkSalMenu* >( pSubMenu );

    if ( pGtkSubMenu == NULL )
        return;

    pGtkSubMenu->mpParentSalMenu = this;
    pItem->mpSubMenu = pGtkSubMenu;
}

void GtkSalMenu::SetFrame( const SalFrame* pFrame )
{
    SolarMutexGuard aGuard;
    assert(mbMenuBar);
    SAL_INFO("vcl.unity", "GtkSalMenu set to frame");
    mpFrame = static_cast< const GtkSalFrame* >( pFrame );
    GtkSalFrame* pFrameNonConst = const_cast<GtkSalFrame*>(mpFrame);
    // if we had a menu on the GtkSalMenu we have to free it as we generate a
    // full menu anyway and we might need to reuse an existing model and
    // actiongroup
    if(mpMenuModel)
    {
        g_object_unref(G_OBJECT(mpMenuModel));
        mpMenuModel = NULL;
    }
    if(mpActionGroup)
    {
        g_object_unref(G_OBJECT(mpActionGroup));
        mpActionGroup = NULL;
    }
    pFrameNonConst->SetMenu( this );
    mpFrame = static_cast< const GtkSalFrame* >( pFrame );
    pFrameNonConst->EnsureAppMenuWatch();
    // Generate the main menu structure.
    UpdateNativeMenu();
}

const GtkSalFrame* GtkSalMenu::GetFrame() const
{
    const GtkSalMenu* pMenu = this;
    while( pMenu && ! pMenu->mpFrame )
        pMenu = pMenu->mpParentSalMenu;
    return pMenu ? pMenu->mpFrame : NULL;
}

void GtkSalMenu::NativeCheckItem( unsigned nSection, unsigned nItemPos, MenuItemBits bits, gboolean bCheck )
{
    SolarMutexGuard aGuard;
    
    if ( mpActionGroup == NULL )
        return;

    gchar* aCommand = g_lo_menu_get_command_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos );

    if ( aCommand != NULL || g_strcmp0( aCommand, "" ) != 0 )
    {
        GVariant *pCheckValue = NULL;
        GVariant *pCurrentState = g_action_group_get_action_state( mpActionGroup, aCommand );

        if ( bits & MIB_RADIOCHECK )
            pCheckValue = ( bCheck == TRUE ) ? g_variant_new_string( aCommand ) : g_variant_new_string( "" );
        else
        {
            // By default, all checked items are checkmark buttons.
            if ( bCheck == TRUE || ( ( bCheck == FALSE ) && pCurrentState != NULL ) )
                pCheckValue = g_variant_new_boolean( bCheck );
        }

        if ( pCheckValue != NULL && ( pCurrentState == NULL || g_variant_equal( pCurrentState, pCheckValue ) == FALSE ) )
            g_action_group_change_action_state( mpActionGroup, aCommand, pCheckValue );

        if ( pCurrentState != NULL )
            g_variant_unref( pCurrentState );
    }

    if ( aCommand )
        g_free( aCommand );
}

void GtkSalMenu::NativeSetEnableItem( gchar* aCommand, gboolean bEnable )
{
    SolarMutexGuard aGuard;
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( mpActionGroup );

    if ( g_action_group_get_action_enabled( G_ACTION_GROUP( pActionGroup ), aCommand ) != bEnable )
        g_lo_action_group_set_action_enabled( pActionGroup, aCommand, bEnable );
}

void GtkSalMenu::NativeSetItemText( unsigned nSection, unsigned nItemPos, const rtl::OUString& rText )
{
    SolarMutexGuard aGuard;
    // Replace the '~' character with '_'.
    rtl::OUString aText = rText.replace( '~', '_' );
    rtl::OString aConvertedText = OUStringToOString( aText, RTL_TEXTENCODING_UTF8 );

    // Update item text only when necessary.
    gchar* aLabel = g_lo_menu_get_label_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos );

    if ( aLabel == NULL || g_strcmp0( aLabel, aConvertedText.getStr() ) != 0 )
        g_lo_menu_set_label_to_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos, aConvertedText.getStr() );

    if ( aLabel )
        g_free( aLabel );
}

void GtkSalMenu::NativeSetAccelerator( unsigned nSection, unsigned nItemPos, const KeyCode& rKeyCode, const rtl::OUString& rKeyName )
{
    SolarMutexGuard aGuard;

    if ( rKeyName.isEmpty() )
        return;

    guint nKeyCode;
    GdkModifierType nModifiers;

    KeyCodeToGdkKey( rKeyCode, &nKeyCode, &nModifiers );

    gchar* aAccelerator = gtk_accelerator_name( nKeyCode, nModifiers );

    gchar* aCurrentAccel = g_lo_menu_get_accelerator_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos );

    if ( aCurrentAccel == NULL && g_strcmp0( aCurrentAccel, aAccelerator ) != 0 )
        g_lo_menu_set_accelerator_to_item_in_section ( G_LO_MENU( mpMenuModel ), nSection, nItemPos, aAccelerator );

    g_free( aAccelerator );
}

void GtkSalMenu::NativeSetItemCommand( unsigned nSection,
                                       unsigned nItemPos,
                                       sal_uInt16 nId,
                                       const gchar* aCommand,
                                       MenuItemBits nBits,
                                       gboolean bChecked,
                                       gboolean bIsSubmenu )
{
    SolarMutexGuard aGuard;
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( mpActionGroup );

    GVariant *pTarget = NULL;

    if ( g_action_group_has_action( mpActionGroup, aCommand ) == FALSE ) {
        if ( ( nBits & MIB_CHECKABLE ) || ( bIsSubmenu == TRUE ) )
        {
            // Item is a checkmark button.
            GVariantType* pStateType = g_variant_type_new( (gchar*) G_VARIANT_TYPE_BOOLEAN );
            GVariant* pState = g_variant_new_boolean( bChecked );

            g_lo_action_group_insert_stateful( pActionGroup, aCommand, nId, bIsSubmenu, NULL, pStateType, NULL, pState );
        }
        else if ( nBits & MIB_RADIOCHECK )
        {
            // Item is a radio button.
            GVariantType* pParameterType = g_variant_type_new( (gchar*) G_VARIANT_TYPE_STRING );
            GVariantType* pStateType = g_variant_type_new( (gchar*) G_VARIANT_TYPE_STRING );
            GVariant* pState = g_variant_new_string( "" );
            pTarget = g_variant_new_string( aCommand );

            g_lo_action_group_insert_stateful( pActionGroup, aCommand, nId, FALSE, pParameterType, pStateType, NULL, pState );
        }
        else
        {
            // Item is not special, so insert a stateless action.
            g_lo_action_group_insert( pActionGroup, aCommand, nId, FALSE );
        }
    }

    GLOMenu* pMenu = G_LO_MENU( mpMenuModel );

    // Menu item is not updated unless it's necessary.
    gchar* aCurrentCommand = g_lo_menu_get_command_from_item_in_section( pMenu, nSection, nItemPos );

    if ( aCurrentCommand == NULL || g_strcmp0( aCurrentCommand, aCommand ) != 0 )
    {
        g_lo_menu_set_command_to_item_in_section( pMenu, nSection, nItemPos, aCommand );

        gchar* aItemCommand = g_strconcat("win.", aCommand, NULL );

        if ( bIsSubmenu == TRUE )
            g_lo_menu_set_submenu_action_to_item_in_section( pMenu, nSection, nItemPos, aItemCommand );
        else
            g_lo_menu_set_action_and_target_value_to_item_in_section( pMenu, nSection, nItemPos, aItemCommand, pTarget );

        g_free( aItemCommand );
    }

    if ( aCurrentCommand )
        g_free( aCurrentCommand );
}

GtkSalMenu* GtkSalMenu::GetMenuForItemCommand( gchar* aCommand, gboolean bGetSubmenu )
{
    SolarMutexGuard aGuard;
    GtkSalMenu* pMenu = NULL;

    for ( sal_uInt16 nPos = 0; nPos < maItems.size(); nPos++ )
    {
        GtkSalMenuItem *pSalItem = maItems[ nPos ];

        String aItemCommand = mpVCLMenu->GetItemCommand( pSalItem->mnId );
        gchar* aItemCommandStr = (gchar*) rtl::OUStringToOString( aItemCommand, RTL_TEXTENCODING_UTF8 ).getStr();

        if ( g_strcmp0( aItemCommandStr, aCommand ) == 0 )
        {
            pMenu = ( bGetSubmenu == TRUE ) ? pSalItem->mpSubMenu : this;
            break;
        }
        else
        {
            if ( pSalItem->mpSubMenu != NULL )
                pMenu = pSalItem->mpSubMenu->GetMenuForItemCommand( aCommand, bGetSubmenu );

            if ( pMenu != NULL )
               break;
        }
    }

    return pMenu;
}

void GtkSalMenu::DispatchCommand( gint itemId, const gchar *aCommand )
{
    SolarMutexGuard aGuard;
    // Only the menubar is allowed to dispatch commands.
    if ( mbMenuBar != TRUE )
        return;

    GtkSalMenu* pSalSubMenu = GetMenuForItemCommand( (gchar*) aCommand, FALSE );
    Menu* pSubMenu = ( pSalSubMenu != NULL ) ? pSalSubMenu->GetMenu() : NULL;

    MenuBar* pMenuBar = static_cast< MenuBar* >( mpVCLMenu );

    pMenuBar->HandleMenuCommandEvent( pSubMenu, itemId );
}

void GtkSalMenu::Activate( const gchar* aMenuCommand )
{
    if ( mbMenuBar != TRUE )
        return;

    GtkSalMenu* pSalSubMenu = GetMenuForItemCommand( (gchar*) aMenuCommand, TRUE );

    if ( pSalSubMenu != NULL ) {
        MenuBar* pMenuBar = static_cast< MenuBar* >( mpVCLMenu );
        pMenuBar->HandleMenuActivateEvent( pSalSubMenu->mpVCLMenu );
        pSalSubMenu->UpdateNativeMenu();
    }
}

void GtkSalMenu::Deactivate( const gchar* aMenuCommand )
{
    if ( mbMenuBar != TRUE )
        return;

    GtkSalMenu* pSalSubMenu = GetMenuForItemCommand( (gchar*) aMenuCommand, TRUE );

    if ( pSalSubMenu != NULL ) {
        MenuBar* pMenuBar = static_cast< MenuBar* >( mpVCLMenu );
        pMenuBar->HandleMenuDeActivateEvent( pSalSubMenu->mpVCLMenu );
    }
}

sal_Bool GtkSalMenu::IsItemVisible( unsigned nPos )
{
    sal_Bool bVisible = sal_False;

    if ( nPos < maItems.size() )
        bVisible = ( ( GtkSalMenuItem* ) maItems[ nPos ])->mbVisible;

    return bVisible;
}

void GtkSalMenu::CheckItem( unsigned nPos, sal_Bool bCheck )
{
}

void GtkSalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
{
}

void GtkSalMenu::ShowItem( unsigned nPos, sal_Bool bShow )
{
    if ( nPos < maItems.size() )
        ( ( GtkSalMenuItem* ) maItems[ nPos ] )->mbVisible = bShow;
}


void GtkSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& rText )
{
}

void GtkSalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage)
{
}

void GtkSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const rtl::OUString& rKeyName )
{
}

void GtkSalMenu::SetItemCommand( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& aCommandStr )
{
}

void GtkSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
}

void GtkSalMenu::Freeze()
{
}

// =======================================================================

/*
 * GtkSalMenuItem
 */

GtkSalMenuItem::GtkSalMenuItem( const SalItemParams* pItemData ) :
    mnId( pItemData->nId ),
    mnType( pItemData->eType ),
    mbVisible( sal_True ),
    mpVCLMenu( pItemData->pMenu ),
    mpParentMenu( NULL ),
    mpSubMenu( NULL )
{
}

GtkSalMenuItem::~GtkSalMenuItem()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
