/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "framework/addonmenu.hxx"
#include "framework/addonsoptions.hxx"
#include <general.h>
#include <macros/debug/assertion.hxx>
#include <framework/imageproducer.hxx>
#include <framework/menuconfiguration.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <tools/config.hxx>
#include <vcl/svapp.hxx>
#include <svtools/menuoptions.hxx>
#include <svl/solar.hrc>
//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

// Please look at sfx2/inc/sfxsids.hrc the values are defined there. Due to build dependencies
// we cannot include the header file.
const sal_uInt16 SID_HELPMENU            = (SID_SFX_START + 410);

namespace framework
{

AddonMenu::AddonMenu( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame ) :
    m_xFrame( rFrame )
{
}

AddonMenu::~AddonMenu()
{
    for ( sal_uInt16 i = 0; i < GetItemCount(); i++ )
    {
        if ( GetItemType( i ) != MENUITEM_SEPARATOR )
        {
            // delete user attributes created with new!
            sal_uInt16 nId = GetItemId( i );
            MenuConfiguration::Attributes* pUserAttributes = (MenuConfiguration::Attributes*)GetUserValue( nId );
            delete pUserAttributes;
            delete GetPopupMenu( nId );
        }
    }
}

// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// Check if command URL string has the unique prefix to identify addon popup menus
sal_Bool AddonPopupMenu::IsCommandURLPrefix( const ::rtl::OUString& aCmdURL )
{
    const char aPrefixCharBuf[] = ADDONSPOPUPMENU_URL_PREFIX_STR;

    return aCmdURL.matchAsciiL( aPrefixCharBuf, sizeof( aPrefixCharBuf )-1, 0 );
}

AddonPopupMenu::AddonPopupMenu( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame ) :
    AddonMenu( rFrame )
{
}

AddonPopupMenu::~AddonPopupMenu()
{
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
    sal_uInt16      nUniqueMenuId   = ADDONMENU_ITEMID_START;

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
sal_uInt16 AddonMenuManager::GetNextPos( sal_uInt16 nPos )
{
    return ( nPos == MENU_APPEND ) ? MENU_APPEND : ( nPos+1 );
}


static sal_uInt16 FindMenuId( Menu* pMenu, const String aCommand )
{
    sal_uInt16 nPos = 0;
    String aCmd;
    for ( nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
    {
        sal_uInt16 nId = pMenu->GetItemId( nPos );
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
            sal_uInt16 nId = FindMenuId(pMergeMenuBar, rtl::OUString(".uno:HelpMenu"));
            if ( nId != USHRT_MAX )
                pHelpMenu = pMergeMenuBar->GetPopupMenu( nId );
        }

        if ( pHelpMenu )
        {
            // Add-Ons help menu items should be inserted after the "registration" menu item
            sal_uInt16 nItemCount       = pHelpMenu->GetItemCount();
            sal_uInt16 nInsSepAfterPos  = MENU_APPEND;
            sal_uInt16 nUniqueMenuId    = ADDONMENU_ITEMID_START;
            AddonsOptions aOptions;

            // try to detect the about menu item with the command URL
            sal_uInt16 nId = FindMenuId(pHelpMenu, rtl::OUString(".uno:About"));
            sal_uInt16 nInsPos = pHelpMenu->GetItemPos( nId );

            Sequence< Sequence< PropertyValue > > aAddonSubMenu;
            const Sequence< Sequence< PropertyValue > >& rAddonHelpMenuEntries = aOptions.GetAddonsHelpMenu();

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
                pHelpMenu->InsertSeparator( nItemCount );
            }
        }
    }
}

// Merge the addon popup menus into the given menu bar at the provided pos.
void AddonMenuManager::MergeAddonPopupMenus( const Reference< XFrame >& rFrame,
                                             const Reference< XModel >& rModel,
                                             sal_uInt16               nMergeAtPos,
                                             MenuBar*             pMergeMenuBar )
{
    if ( pMergeMenuBar )
    {
        AddonsOptions   aAddonsOptions;
        sal_uInt16          nInsertPos = nMergeAtPos;

        ::rtl::OUString                              aTitle;
        ::rtl::OUString                              aURL;
        ::rtl::OUString                              aTarget;
        ::rtl::OUString                              aImageId;
        ::rtl::OUString                              aContext;
        Sequence< Sequence< PropertyValue > > aAddonSubMenu;
        sal_uInt16                                nUniqueMenuId = ADDONMENU_ITEMID_START;

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
            if ( !aTitle.isEmpty() &&
                 !aURL.isEmpty()   &&
                 aAddonSubMenu.getLength() > 0 &&
                 AddonMenuManager::IsCorrectContext( rModel, aContext ))
            {
                sal_uInt16          nId             = nUniqueMenuId++;
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
                                  sal_uInt16                                nInsPos,
                                  sal_uInt16&                               nUniqueMenuId,
                                  Sequence< Sequence< PropertyValue > > aAddonMenuDefinition,
                                  const Reference< XFrame >&            rFrame,
                                  const Reference< XModel >&            rModel )
{
    Sequence< Sequence< PropertyValue > >   aAddonSubMenu;
    sal_Bool                                    bInsertSeparator    = sal_False;
    sal_uInt32                                  i                   = 0;
    sal_uInt32                                  nElements           = 0;
    sal_uInt32                                  nCount              = aAddonMenuDefinition.getLength();
    AddonsOptions                           aAddonsOptions;

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aTarget;
    ::rtl::OUString aImageId;
    ::rtl::OUString aContext;

    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry( aAddonMenuDefinition[i], aTitle, aURL, aTarget, aImageId, aContext, aAddonSubMenu );

        if ( !IsCorrectContext( rModel, aContext ) || ( aTitle.isEmpty() && aURL.isEmpty() ))
            continue;

        if ( aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:separator" )))
            bInsertSeparator = sal_True;
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
                bInsertSeparator = sal_False;
                pCurrentMenu->InsertSeparator( nInsPos );
                nInsPos = AddonMenuManager::GetNextPos( nInsPos );
            }

            sal_uInt16 nId = nUniqueMenuId++;
            pCurrentMenu->InsertItem( nId, aTitle, 0, nInsPos );
            nInsPos = AddonMenuManager::GetNextPos( nInsPos );

            ++nElements;

            // Store values from configuration to the New and Wizard menu entries to enable
            // sfx2 based code to support high contrast mode correctly!
            pCurrentMenu->SetUserValue( nId, sal_uIntPtr( new MenuConfiguration::Attributes( aTarget, aImageId )) );
            pCurrentMenu->SetItemCommand( nId, aURL );

            if ( pSubMenu )
                pCurrentMenu->SetPopupMenu( nId, pSubMenu );
        }
    }
}

// Retrieve the menu entry property values from a sequence
void AddonMenuManager::GetMenuEntry( const Sequence< PropertyValue >& rAddonMenuEntry,
                                     ::rtl::OUString& rTitle,
                                     ::rtl::OUString& rURL,
                                     ::rtl::OUString& rTarget,
                                     ::rtl::OUString& rImageId,
                                     ::rtl::OUString& rContext,
                                     Sequence< Sequence< PropertyValue > >& rAddonSubMenu )
{
    // Reset submenu parameter
    rAddonSubMenu   = Sequence< Sequence< PropertyValue > >();

    for ( int i = 0; i < rAddonMenuEntry.getLength(); i++ )
    {
        ::rtl::OUString aMenuEntryPropName = rAddonMenuEntry[i].Name;
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
sal_Bool AddonMenuManager::IsCorrectContext( const Reference< XModel >& rModel, const ::rtl::OUString& aContext )
{
    if ( rModel.is() )
    {
        Reference< com::sun::star::lang::XServiceInfo > xServiceInfo( rModel, UNO_QUERY );
        if ( xServiceInfo.is() )
        {
            sal_Int32 nIndex = 0;
            do
            {
                ::rtl::OUString aToken = aContext.getToken( 0, ',', nIndex );

                if ( xServiceInfo->supportsService( aToken ))
                    return sal_True;
            }
            while ( nIndex >= 0 );
        }
    }

    return ( aContext.isEmpty() );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
