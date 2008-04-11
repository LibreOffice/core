/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: addonmenu.cxx,v $
 * $Revision: 1.14 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include "classes/addonmenu.hxx"
#include "classes/addonsoptions.hxx"
#include <general.h>
#include <macros/debug/assertion.hxx>
#include <helper/imageproducer.hxx>
#include <classes/menuconfiguration.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <tools/config.hxx>
#include <vcl/svapp.hxx>
#include <svtools/menuoptions.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

// Please look at sfx2/inc/sfxsids.hrc the values are defined there. Due to build dependencies
// we cannot include the header file.
const USHORT SID_SFX_START           = 5000;
const USHORT SID_HELPMENU            = (SID_SFX_START + 410);
const USHORT SID_ONLINE_REGISTRATION = (SID_SFX_START + 1537);

namespace framework
{

AddonMenu::AddonMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame ) :
    m_xFrame( rFrame )
{
}

AddonMenu::~AddonMenu()
{
    for ( USHORT i = 0; i < GetItemCount(); i++ )
    {
        if ( GetItemType( i ) != MENUITEM_SEPARATOR )
        {
            // delete user attributes created with new!
            USHORT nId = GetItemId( i );
            MenuConfiguration::Attributes* pUserAttributes = (MenuConfiguration::Attributes*)GetUserValue( nId );
            delete pUserAttributes;
            delete GetPopupMenu( nId );
        }
    }
}

// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// Check if command URL string has the unique prefix to identify addon popup menus
sal_Bool AddonPopupMenu::IsCommandURLPrefix( const OUString& aCmdURL )
{
    const char aPrefixCharBuf[] = ADDONSPOPUPMENU_URL_PREFIX_STR;

    return aCmdURL.matchAsciiL( aPrefixCharBuf, sizeof( aPrefixCharBuf )-1, 0 );
}

AddonPopupMenu::AddonPopupMenu( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame ) :
    m_xFrame( rFrame )
{
}

AddonPopupMenu::~AddonPopupMenu()
{
    for ( USHORT i = 0; i < GetItemCount(); i++ )
    {
        if ( GetItemType( i ) != MENUITEM_SEPARATOR )
        {
            // delete user attributes created with new!
            USHORT nId = GetItemId( i );
            MenuConfiguration::Attributes* pUserAttributes = (MenuConfiguration::Attributes*)GetUserValue( nId );
            delete pUserAttributes;
            delete GetPopupMenu( nId );
        }
    }
}

// ------------------------------------------------------------------------

static Reference< XModel > GetModelFromFrame( const Reference< XFrame >& rFrame )
{
    // Query for the model to get check the context information
    Reference< XModel > xModel;
    if ( rFrame.is() )
    {
        Reference< XController > xController( rFrame->getController(), UNO_QUERY );
        if ( xController.is() )
            xModel = xController->getModel();
    }

    return xModel;
}

// ------------------------------------------------------------------------

sal_Bool AddonMenuManager::HasAddonMenuElements()
{
    return AddonsOptions().HasAddonsMenu();
}

sal_Bool AddonMenuManager::HasAddonHelpMenuElements()
{
    return AddonsOptions().HasAddonsHelpMenu();
}

// Factory method to create different Add-On menu types
PopupMenu* AddonMenuManager::CreatePopupMenuType( MenuType eMenuType, const Reference< XFrame >& rFrame )
{
    if ( eMenuType == ADDON_MENU )
        return new AddonMenu( rFrame );
    else if ( eMenuType == ADDON_POPUPMENU )
        return new AddonPopupMenu( rFrame );
    else
        return NULL;
}

// Create the Add-Ons menu
AddonMenu* AddonMenuManager::CreateAddonMenu( const Reference< XFrame >& rFrame )
{
    AddonsOptions aOptions;
    AddonMenu*  pAddonMenu      = NULL;
    USHORT      nUniqueMenuId   = ADDONMENU_ITEMID_START;

    const Sequence< Sequence< PropertyValue > >& rAddonMenuEntries = aOptions.GetAddonsMenu();
    if ( rAddonMenuEntries.getLength() > 0 )
    {
        pAddonMenu = (AddonMenu *)AddonMenuManager::CreatePopupMenuType( ADDON_MENU, rFrame );
        Reference< XModel > xModel = GetModelFromFrame( rFrame );
        AddonMenuManager::BuildMenu( pAddonMenu, ADDON_MENU, MENU_APPEND, nUniqueMenuId, rAddonMenuEntries, rFrame, xModel );

        // Don't return an empty Add-On menu
        if ( pAddonMenu->GetItemCount() == 0 )
        {
            delete pAddonMenu;
            pAddonMenu = NULL;
        }
    }

    return pAddonMenu;
}

// Returns the next insert position from nPos.
USHORT AddonMenuManager::GetNextPos( USHORT nPos )
{
    return ( nPos == MENU_APPEND ) ? MENU_APPEND : ( nPos+1 );
}


static USHORT FindMenuId( Menu* pMenu, const String aCommand )
{
    USHORT nPos = 0;
    String aCmd;
    for ( nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
    {
        USHORT nId = pMenu->GetItemId( nPos );
        aCmd = pMenu->GetItemCommand( nId );
        if ( aCmd == aCommand )
            return nId;
    }

    return USHRT_MAX;
}


// Merge the Add-Ons help menu items into the given menu bar at a defined pos
void AddonMenuManager::MergeAddonHelpMenu( const Reference< XFrame >& rFrame, MenuBar* pMergeMenuBar )
{
    if ( pMergeMenuBar )
    {
        PopupMenu* pHelpMenu = pMergeMenuBar->GetPopupMenu( SID_HELPMENU );
        if ( !pHelpMenu )
        {
            USHORT nId = FindMenuId( pMergeMenuBar, String::CreateFromAscii( ".uno:HelpMenu" ));
            if ( nId != USHRT_MAX )
                pHelpMenu = pMergeMenuBar->GetPopupMenu( nId );
        }

        if ( pHelpMenu )
        {
            static const char REFERENCECOMMAND_AFTER[]  = ".uno:OnlineRegistrationDlg";
            static const char REFERENCECOMMAND_BEFORE[] = ".uno:About";

            // Add-Ons help menu items should be inserted after the "registration" menu item
            bool   bAddAfter        = true;
            USHORT nItemCount       = pHelpMenu->GetItemCount();
            USHORT nRegPos          = pHelpMenu->GetItemPos( SID_ONLINE_REGISTRATION );
            USHORT nInsPos          = nRegPos;
            USHORT nInsSepAfterPos  = MENU_APPEND;
            USHORT nUniqueMenuId    = ADDONMENU_ITEMID_START;
            AddonsOptions aOptions;

            if ( nRegPos == USHRT_MAX )
            {
                // try to detect the online registration dialog menu item with the command URL
                USHORT nId = FindMenuId( pHelpMenu, String::CreateFromAscii( REFERENCECOMMAND_AFTER ));
                nRegPos    = pHelpMenu->GetItemPos( nId );
                nInsPos    = nRegPos;
            }

            if ( nRegPos == USHRT_MAX )
            {
                // second try:
                // try to detect the about menu item with the command URL
                USHORT nId = FindMenuId( pHelpMenu, String::CreateFromAscii( REFERENCECOMMAND_BEFORE ));
                nRegPos    = pHelpMenu->GetItemPos( nId );
                nInsPos    = nRegPos;
                bAddAfter  = false;
            }

            Sequence< Sequence< PropertyValue > > aAddonSubMenu;
            const Sequence< Sequence< PropertyValue > >& rAddonHelpMenuEntries = aOptions.GetAddonsHelpMenu();

            nInsPos = bAddAfter ? AddonMenuManager::GetNextPos( nInsPos ) : nInsPos;
            if ( nInsPos < nItemCount && pHelpMenu->GetItemType( nInsPos ) != MENUITEM_SEPARATOR )
                nInsSepAfterPos = nInsPos;

            Reference< XModel > xModel = GetModelFromFrame( rFrame );
            AddonMenuManager::BuildMenu( pHelpMenu, ADDON_MENU, nInsPos, nUniqueMenuId, rAddonHelpMenuEntries, rFrame, xModel );

            if ( pHelpMenu->GetItemCount() > nItemCount )
            {
                if ( nInsSepAfterPos < MENU_APPEND )
                {
                    nInsSepAfterPos += ( pHelpMenu->GetItemCount() - nItemCount );
                    if ( pHelpMenu->GetItemType( nInsSepAfterPos ) != MENUITEM_SEPARATOR )
                        pHelpMenu->InsertSeparator( nInsSepAfterPos );
                }
                if ( nRegPos < MENU_APPEND )
                    pHelpMenu->InsertSeparator( nRegPos+1 );
                else
                    pHelpMenu->InsertSeparator( nItemCount );
            }
        }
    }
}

// Merge the addon popup menus into the given menu bar at the provided pos.
void AddonMenuManager::MergeAddonPopupMenus( const Reference< XFrame >& rFrame,
                                             const Reference< XModel >& rModel,
                                             USHORT               nMergeAtPos,
                                             MenuBar*             pMergeMenuBar )
{
    if ( pMergeMenuBar )
    {
        AddonsOptions   aAddonsOptions;
        USHORT          nInsertPos = nMergeAtPos;

        OUString                              aTitle;
        OUString                              aURL;
        OUString                              aTarget;
        OUString                              aImageId;
        OUString                              aContext;
        Sequence< Sequence< PropertyValue > > aAddonSubMenu;
        USHORT                                nUniqueMenuId = ADDONMENU_ITEMID_START;

        const Sequence< Sequence< PropertyValue > >&    rAddonMenuEntries = aAddonsOptions.GetAddonsMenuBarPart();
        for ( sal_Int32 i = 0; i < rAddonMenuEntries.getLength(); i++ )
        {
            AddonMenuManager::GetMenuEntry( rAddonMenuEntries[i],
                                            aTitle,
                                            aURL,
                                            aTarget,
                                            aImageId,
                                            aContext,
                                            aAddonSubMenu );
            if ( aTitle.getLength() > 0 &&
                 aURL.getLength() > 0 &&
                 aAddonSubMenu.getLength() > 0 &&
                 AddonMenuManager::IsCorrectContext( rModel, aContext ))
            {
                USHORT          nId             = nUniqueMenuId++;
                AddonPopupMenu* pAddonPopupMenu = (AddonPopupMenu *)AddonMenuManager::CreatePopupMenuType( ADDON_POPUPMENU, rFrame );

                AddonMenuManager::BuildMenu( pAddonPopupMenu, ADDON_MENU, MENU_APPEND, nUniqueMenuId, aAddonSubMenu, rFrame, rModel );

                if ( pAddonPopupMenu->GetItemCount() > 0 )
                {
                    pAddonPopupMenu->SetCommandURL( aURL );
                    pMergeMenuBar->InsertItem( nId, aTitle, 0, nInsertPos++ );
                    pMergeMenuBar->SetPopupMenu( nId, pAddonPopupMenu );

                    // Store the command URL into the VCL menu bar for later identification
                    pMergeMenuBar->SetItemCommand( nId, aURL );
                }
                else
                    delete pAddonPopupMenu;
            }
        }
    }
}

// Insert the menu and sub menu entries into pCurrentMenu with the aAddonMenuDefinition provided
void AddonMenuManager::BuildMenu( PopupMenu*                            pCurrentMenu,
                                  MenuType                              nSubMenuType,
                                  USHORT                                nInsPos,
                                  USHORT&                               nUniqueMenuId,
                                  Sequence< Sequence< PropertyValue > > aAddonMenuDefinition,
                                  const Reference< XFrame >&            rFrame,
                                  const Reference< XModel >&            rModel )
{
    Sequence< Sequence< PropertyValue > >   aAddonSubMenu;
    BOOL                                    bInsertSeparator    = FALSE;
    UINT32                                  i                   = 0;
    UINT32                                  nElements           = 0;
    UINT32                                  nCount              = aAddonMenuDefinition.getLength();
    AddonsOptions                           aAddonsOptions;

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aTarget;
    ::rtl::OUString aImageId;
    ::rtl::OUString aContext;

    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry( aAddonMenuDefinition[i], aTitle, aURL, aTarget, aImageId, aContext, aAddonSubMenu );

        if ( !IsCorrectContext( rModel, aContext ) || ( !aTitle.getLength() && !aURL.getLength() ))
            continue;

        if ( aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:separator" )))
            bInsertSeparator = TRUE;
        else
        {
            PopupMenu* pSubMenu = NULL;
            if ( aAddonSubMenu.getLength() > 0 )
            {
                pSubMenu = AddonMenuManager::CreatePopupMenuType( nSubMenuType, rFrame );
                AddonMenuManager::BuildMenu( pSubMenu, nSubMenuType, MENU_APPEND, nUniqueMenuId, aAddonSubMenu, rFrame, rModel );

                // Don't create a menu item for an empty sub menu
                if ( pSubMenu->GetItemCount() == 0 )
                {
                    delete pSubMenu;
                    pSubMenu =  NULL;
                    continue;
                }
            }

            if ( bInsertSeparator && nElements > 0 )
            {
                // Insert a separator only when we insert a new element afterwards and we
                // have already one before us
                nElements = 0;
                bInsertSeparator = FALSE;
                pCurrentMenu->InsertSeparator( nInsPos );
                nInsPos = AddonMenuManager::GetNextPos( nInsPos );
            }

            USHORT nId = nUniqueMenuId++;
            pCurrentMenu->InsertItem( nId, aTitle, 0, nInsPos );
            nInsPos = AddonMenuManager::GetNextPos( nInsPos );

            ++nElements;

            // Store values from configuration to the New and Wizard menu entries to enable
            // sfx2 based code to support high contrast mode correctly!
            pCurrentMenu->SetUserValue( nId, ULONG( new MenuConfiguration::Attributes( aTarget, aImageId )) );
            pCurrentMenu->SetItemCommand( nId, aURL );

            if ( pSubMenu )
                pCurrentMenu->SetPopupMenu( nId, pSubMenu );
        }
    }
}

// Retrieve the menu entry property values from a sequence
void AddonMenuManager::GetMenuEntry( const Sequence< PropertyValue >& rAddonMenuEntry,
                                     OUString& rTitle,
                                     OUString& rURL,
                                     OUString& rTarget,
                                     OUString& rImageId,
                                     OUString& rContext,
                                     Sequence< Sequence< PropertyValue > >& rAddonSubMenu )
{
    // Reset submenu parameter
    rAddonSubMenu   = Sequence< Sequence< PropertyValue > >();

    for ( int i = 0; i < rAddonMenuEntry.getLength(); i++ )
    {
        OUString aMenuEntryPropName = rAddonMenuEntry[i].Name;
        if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_URL )
            rAddonMenuEntry[i].Value >>= rURL;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_TITLE )
            rAddonMenuEntry[i].Value >>= rTitle;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_TARGET )
            rAddonMenuEntry[i].Value >>= rTarget;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_IMAGEIDENTIFIER )
            rAddonMenuEntry[i].Value >>= rImageId;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_SUBMENU )
            rAddonMenuEntry[i].Value >>= rAddonSubMenu;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_CONTEXT )
            rAddonMenuEntry[i].Value >>= rContext;
    }
}

// Check if the context string matches the provided xModel context
sal_Bool AddonMenuManager::IsCorrectContext( const Reference< XModel >& rModel, const OUString& aContext )
{
    if ( rModel.is() )
    {
        Reference< com::sun::star::lang::XServiceInfo > xServiceInfo( rModel, UNO_QUERY );
        if ( xServiceInfo.is() )
        {
            sal_Int32 nIndex = 0;
            do
            {
                OUString aToken = aContext.getToken( 0, ',', nIndex );

                if ( xServiceInfo->supportsService( aToken ))
                    return sal_True;
            }
            while ( nIndex >= 0 );
        }
    }

    return ( aContext.getLength() == 0 );
}

}
