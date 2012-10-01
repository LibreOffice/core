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

//#include <gtk/gtk.h>
#include <unx/gtk/glomenu.h>
#include <unx/gtk/gloactiongroup.h>
#include <vcl/menu.hxx>
#include <unx/gtk/gtkinst.hxx>

#include <framework/menuconfiguration.hxx>

#include <iostream>

using namespace std;


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

// FIXME: Check for missing keys. Maybe translating keycodes would be safer...
rtl::OUString GetGtkKeyName( rtl::OUString keyName )
{
    rtl::OUString aGtkKeyName("");

    sal_Int32 nIndex = 0;

    do
    {
        rtl::OUString token = keyName.getToken( 0, '+', nIndex );

        if ( token == "Ctrl" ) {
            aGtkKeyName += "<Control>";
        } else if ( token == "Alt" ) {
            aGtkKeyName += "<Alt>";
        } else if ( token == "Shift" ) {
            aGtkKeyName += "<Shift>";
        } else {
            aGtkKeyName += token;
        }
    } while ( nIndex >= 0 );

    return aGtkKeyName;
}

bool GtkSalMenu::CanUpdate()
{
    const GtkSalFrame* pFrame = GetFrame();
    if (!pFrame)
    {
        std::cout << "not updating menu model, I have no frame " << mpMenuModel << std::endl;
        return false;
    }
    const GObject* pWindow = G_OBJECT(gtk_widget_get_window( GTK_WIDGET(pFrame->getWindow()) ));
    if(!pWindow)
    {
        std::cout << "not updating menu model, I have no frame " << mpMenuModel << std::endl;
        return false;
    }
    // the root menu does not have its own model and has to use the one owned by the frame
    if(mbMenuBar)
    {
        mpMenuModel = G_MENU_MODEL( g_object_get_data( G_OBJECT( pWindow ), "g-lo-menubar" ) );
        mpActionGroup = G_ACTION_GROUP( g_object_get_data( G_OBJECT( pWindow ), "g-lo-action-group" ) );
    }
    if(!mpMenuModel || !mpActionGroup)
        return false;
    std::cout << "updating menu model" << mpMenuModel << std::endl;
    return true;
}

void GtkSalMenu::UpdateNativeMenu2()
{
    UpdateNativeMenu();
}

void GtkSalMenu::UpdateNativeMenu( )
{
    if(!CanUpdate())
        return;
    Menu* pVCLMenu = GetMenu();
    GLOMenu* pLOMenu = G_LO_MENU( GetMenuModel() );
    GActionGroup* pActionGroup = GetActionGroup();

    sal_uInt16 nLOMenuSize = g_menu_model_get_n_items( G_MENU_MODEL( pLOMenu ) );

    if ( nLOMenuSize == 0 )
        g_lo_menu_new_section( pLOMenu, 0, NULL );

    sal_uInt16 nSection = 0;
    sal_uInt16 nItemPos = 0;
    sal_uInt16 validItems = 0;
    sal_uInt16 nItem;

    for ( nItem = 0; nItem < GetItemCount(); nItem++ ) {
        GtkSalMenuItem *pSalMenuItem = GetItemAtPos( nItem );
        sal_uInt16 nId = pSalMenuItem->mnId;

        if ( pSalMenuItem->mnType == MENUITEM_SEPARATOR )
        {
            nSection++;
            nItemPos = 0;

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
        }

        GtkSalMenu* pSubmenu = pSalMenuItem->mpSubMenu;

        if ( pSubmenu && pSubmenu->GetMenu() )
        {
            NativeSetItemCommand( nSection, nItemPos, nId, aNativeCommand, itemBits, FALSE, TRUE );

            GLOMenu* pSubMenuModel = g_lo_menu_get_submenu_from_item_in_section( pLOMenu, nSection, nItemPos );

            if ( pSubMenuModel == NULL )
            {
                pSubMenuModel = g_lo_menu_new();
                g_lo_menu_set_submenu_to_item_in_section( pLOMenu, nSection, nItemPos, G_MENU_MODEL( pSubMenuModel ) );
            }

            g_object_unref( pSubMenuModel );

            pSubmenu->GetMenu()->Activate();
            pSubmenu->GetMenu()->Deactivate();

            pSubmenu->SetMenuModel( G_MENU_MODEL( pSubMenuModel ) );
            pSubmenu->SetActionGroup( pActionGroup );
            pSubmenu->UpdateNativeMenu();
        }

        g_free( aNativeCommand );

        nItemPos++;
        validItems++;
    }
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
    
void ObjectDestroyedNotify( gpointer data )
{
    if ( data ) {
        g_object_unref( data );
    }
}



/*
 * GtkSalMenu
 */

GtkSalMenu::GtkSalMenu( sal_Bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mbVisible( sal_False ),
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
//        g_source_remove_by_user_data( this );

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
    std::cout << "GtkSalMenu set to frame" << std::endl;
    mpFrame = static_cast< const GtkSalFrame* >( pFrame );
    GtkSalFrame* pFrameNonConst = const_cast<GtkSalFrame*>(mpFrame);
    // if we had a menu on the GtkSalMenu we have to free it as we generate a
    // full menu anyway and we might need to reuse an existing model and
    // actiongroup
    if(mpMenuModel)
    {
        g_lo_menu_remove(G_LO_MENU(mpMenuModel), 0);
        mpMenuModel = NULL;
    }
    if(mpActionGroup)
    {
        g_lo_action_group_clear( G_LO_ACTION_GROUP(mpActionGroup) );
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
        {
            pCheckValue = ( bCheck == TRUE ) ? g_variant_new_string( aCommand ) : g_variant_new_string( "" );
        }
        else
        {
            // By default, all checked items are checkmark buttons.
            if ( bCheck == TRUE || ( ( bCheck == FALSE ) && pCurrentState != NULL ) )
                pCheckValue = g_variant_new_boolean( bCheck );
        }

        if ( pCheckValue != NULL && ( pCurrentState == NULL || g_variant_equal( pCurrentState, pCheckValue ) == FALSE ) )
            g_action_group_change_action_state( mpActionGroup, aCommand, pCheckValue );

        if ( pCurrentState )
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

    rtl::OString aAccelerator = rtl::OUStringToOString( GetGtkKeyName( rKeyName ), RTL_TEXTENCODING_UTF8 );

    gchar* aCurrentAccel = g_lo_menu_get_accelerator_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos );

    if ( aCurrentAccel == NULL && g_strcmp0( aCurrentAccel, aAccelerator.getStr() ) != 0 )
        g_lo_menu_set_accelerator_to_item_in_section ( G_LO_MENU( mpMenuModel ), nSection, nItemPos, aAccelerator.getStr() );

    if ( aCurrentAccel )
        g_free( aCurrentAccel );
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
        pSalSubMenu->mpVCLMenu->Activate();
        pSalSubMenu->UpdateNativeMenu2();
    }
}

void GtkSalMenu::Deactivate( const gchar* aMenuCommand )
{
    if ( mbMenuBar != TRUE )
        return;

    GtkSalMenu* pSalSubMenu = GetMenuForItemCommand( (gchar*) aMenuCommand, TRUE );

    if ( pSalSubMenu != NULL ) {
        pSalSubMenu->mpVCLMenu->Deactivate();
    }
}

void GtkSalMenu::CheckItem( unsigned nPos, sal_Bool bCheck )
{
}

void GtkSalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
{
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
    mpVCLMenu( pItemData->pMenu ),
    mpParentMenu( NULL ),
    mpSubMenu( NULL )
{
}

GtkSalMenuItem::~GtkSalMenuItem()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
