/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <framework/addonmenu.hxx>
#include <framework/addonsoptions.hxx>
#include <general.h>
#include <framework/imageproducer.hxx>
#include <framework/menuconfiguration.hxx>
#include <services.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>

#include <tools/config.hxx>
#include <vcl/svapp.hxx>
#include <svtools/menuoptions.hxx>
#include <svl/solar.hrc>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

// Please look at sfx2/inc/sfxsids.hrc the values are defined there. Due to build dependencies
// we cannot include the header file.
const sal_uInt16 SID_HELPMENU            = (SID_SFX_START + 410);

namespace framework
{

AddonMenu::AddonMenu( const css::uno::Reference< css::frame::XFrame >& rFrame ) :
    m_xFrame( rFrame )
{
}

AddonMenu::~AddonMenu()
{
    for ( sal_uInt16 i = 0; i < GetItemCount(); i++ )
    {
        if ( GetItemType( i ) != MenuItemType::SEPARATOR )
        {
            sal_uInt16 nId = GetItemId( i );
            delete GetPopupMenu( nId );
        }
    }
}

// Check if command URL string has the unique prefix to identify addon popup menus
bool AddonPopupMenu::IsCommandURLPrefix( const OUString& aCmdURL )
{
    return aCmdURL.startsWith( ADDONSPOPUPMENU_URL_PREFIX_STR );
}

AddonPopupMenu::AddonPopupMenu( const css::uno::Reference< css::frame::XFrame >& rFrame ) :
    AddonMenu( rFrame )
{
}

AddonPopupMenu::~AddonPopupMenu()
{
}

static OUString GetModuleIdentifier(const Reference<XComponentContext>& rContext,
    const Reference< XFrame >& rFrame)
{
    Reference< XModuleManager > xModuleManager(ModuleManager::create(rContext));
    if ( xModuleManager.is() )
    {
        try
        {
            return xModuleManager->identify( rFrame );
        }
        catch ( Exception& )
        {
        }
    }
    return OUString();
}

bool AddonMenuManager::HasAddonMenuElements()
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
AddonMenu* AddonMenuManager::CreateAddonMenu( const Reference< XFrame >& rFrame,
                                              const Reference< XComponentContext >& rContext )
{
    AddonsOptions aOptions;
    AddonMenu*  pAddonMenu      = NULL;
    sal_uInt16      nUniqueMenuId   = ADDONMENU_ITEMID_START;

    const Sequence< Sequence< PropertyValue > >& rAddonMenuEntries = aOptions.GetAddonsMenu();
    if ( rAddonMenuEntries.getLength() > 0 )
    {
        pAddonMenu = static_cast<AddonMenu *>(AddonMenuManager::CreatePopupMenuType( ADDON_MENU, rFrame ));
        ::rtl::OUString aModuleIdentifier = GetModuleIdentifier( rContext, rFrame );
        AddonMenuManager::BuildMenu( pAddonMenu, ADDON_MENU, MENU_APPEND, nUniqueMenuId, rAddonMenuEntries, rFrame, aModuleIdentifier );

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

static sal_uInt16 FindMenuId( Menu* pMenu, const OUString& aCommand )
{
    sal_uInt16 nPos = 0;
    OUString aCmd;
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
void AddonMenuManager::MergeAddonHelpMenu( const Reference< XFrame >& rFrame,
                                           MenuBar* pMergeMenuBar,
                                           const Reference<XComponentContext>& rContext )
{
    if ( pMergeMenuBar )
    {
        PopupMenu* pHelpMenu = pMergeMenuBar->GetPopupMenu( SID_HELPMENU );
        if ( !pHelpMenu )
        {
            sal_uInt16 nId = FindMenuId(pMergeMenuBar, OUString(".uno:HelpMenu"));
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
            sal_uInt16 nId = FindMenuId(pHelpMenu, OUString(".uno:About"));
            sal_uInt16 nInsPos = pHelpMenu->GetItemPos( nId );

            const Sequence< Sequence< PropertyValue > >& rAddonHelpMenuEntries = aOptions.GetAddonsHelpMenu();

            if ( nInsPos < nItemCount && pHelpMenu->GetItemType( nInsPos ) != MenuItemType::SEPARATOR )
                nInsSepAfterPos = nInsPos;

            ::rtl::OUString aModuleIdentifier = GetModuleIdentifier(rContext, rFrame);
            AddonMenuManager::BuildMenu( pHelpMenu, ADDON_MENU, nInsPos, nUniqueMenuId, rAddonHelpMenuEntries, rFrame, aModuleIdentifier );

            if ( pHelpMenu->GetItemCount() > nItemCount )
            {
                if ( nInsSepAfterPos < MENU_APPEND )
                {
                    nInsSepAfterPos += ( pHelpMenu->GetItemCount() - nItemCount );
                    if ( pHelpMenu->GetItemType( nInsSepAfterPos ) != MenuItemType::SEPARATOR )
                        pHelpMenu->InsertSeparator(OString(), nInsSepAfterPos);
                }
                pHelpMenu->InsertSeparator(OString(), nItemCount);
            }
        }
    }
}

// Merge the addon popup menus into the given menu bar at the provided pos.
void AddonMenuManager::MergeAddonPopupMenus( const Reference< XFrame >& rFrame,
                                             sal_uInt16               nMergeAtPos,
                                             MenuBar*             pMergeMenuBar,
                                             const Reference< XComponentContext >& rContext )
{
    if ( pMergeMenuBar )
    {
        AddonsOptions   aAddonsOptions;
        sal_uInt16          nInsertPos = nMergeAtPos;

        OUString                              aTitle;
        OUString                              aURL;
        OUString                              aTarget;
        OUString                              aImageId;
        OUString                              aContext;
        Sequence< Sequence< PropertyValue > > aAddonSubMenu;
        sal_uInt16                            nUniqueMenuId = ADDONMENU_ITEMID_START;
        OUString                              aModuleIdentifier = GetModuleIdentifier(rContext, rFrame);

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
                 AddonMenuManager::IsCorrectContext( aModuleIdentifier, aContext ))
            {
                sal_uInt16          nId             = nUniqueMenuId++;
                AddonPopupMenu* pAddonPopupMenu = static_cast<AddonPopupMenu *>(AddonMenuManager::CreatePopupMenuType( ADDON_POPUPMENU, rFrame ));

                AddonMenuManager::BuildMenu( pAddonPopupMenu, ADDON_MENU, MENU_APPEND, nUniqueMenuId, aAddonSubMenu, rFrame, aModuleIdentifier );

                if ( pAddonPopupMenu->GetItemCount() > 0 )
                {
                    pAddonPopupMenu->SetCommandURL( aURL );
                    pMergeMenuBar->InsertItem( nId, aTitle, MenuItemBits::NONE, OString(), nInsertPos++ );
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
                                  sal_uInt16                            nInsPos,
                                  sal_uInt16&                           nUniqueMenuId,
                                  const Sequence< Sequence< PropertyValue > >& aAddonMenuDefinition,
                                  const Reference< XFrame >&            rFrame,
                                  const ::rtl::OUString&               rModuleIdentifier )
{
    Sequence< Sequence< PropertyValue > >   aAddonSubMenu;
    bool                                    bInsertSeparator    = false;
    sal_uInt32                              i                   = 0;
    sal_uInt32                              nElements           = 0;
    sal_uInt32                              nCount              = aAddonMenuDefinition.getLength();
    AddonsOptions                           aAddonsOptions;

    OUString aTitle;
    OUString aURL;
    OUString aTarget;
    OUString aImageId;
    OUString aContext;

    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry( aAddonMenuDefinition[i], aTitle, aURL, aTarget, aImageId, aContext, aAddonSubMenu );

        if ( !IsCorrectContext( rModuleIdentifier, aContext ) || ( aTitle.isEmpty() && aURL.isEmpty() ))
            continue;

        if ( aURL == "private:separator" )
            bInsertSeparator = true;
        else
        {
            PopupMenu* pSubMenu = NULL;
            if ( aAddonSubMenu.getLength() > 0 )
            {
                pSubMenu = AddonMenuManager::CreatePopupMenuType( nSubMenuType, rFrame );
                AddonMenuManager::BuildMenu( pSubMenu, nSubMenuType, MENU_APPEND, nUniqueMenuId, aAddonSubMenu, rFrame, rModuleIdentifier );

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
                bInsertSeparator = false;
                pCurrentMenu->InsertSeparator(OString(), nInsPos);
                nInsPos = AddonMenuManager::GetNextPos( nInsPos );
            }

            sal_uInt16 nId = nUniqueMenuId++;
            pCurrentMenu->InsertItem(nId, aTitle, MenuItemBits::NONE, OString(), nInsPos);
            nInsPos = AddonMenuManager::GetNextPos( nInsPos );

            ++nElements;

            // Store values from configuration to the New and Wizard menu entries to enable
            // sfx2 based code to support high contrast mode correctly!
            sal_uIntPtr nAttributePtr = MenuAttributes::CreateAttribute(aTarget, aImageId);
            pCurrentMenu->SetUserValue(nId, nAttributePtr, MenuAttributes::ReleaseAttribute);
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
        if ( aMenuEntryPropName == ADDONSMENUITEM_STRING_URL )
            rAddonMenuEntry[i].Value >>= rURL;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_STRING_TITLE )
            rAddonMenuEntry[i].Value >>= rTitle;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_STRING_TARGET )
            rAddonMenuEntry[i].Value >>= rTarget;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_STRING_IMAGEIDENTIFIER )
            rAddonMenuEntry[i].Value >>= rImageId;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_STRING_SUBMENU )
            rAddonMenuEntry[i].Value >>= rAddonSubMenu;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_STRING_CONTEXT )
            rAddonMenuEntry[i].Value >>= rContext;
    }
}

// Check if the context string matches the provided xModel context
bool AddonMenuManager::IsCorrectContext( const OUString& rModuleIdentifier, const OUString& rContext )
{
    if ( rContext.isEmpty() )
        return true;

    if ( !rModuleIdentifier.isEmpty() )
    {
        sal_Int32 nIndex = rContext.indexOf( rModuleIdentifier );
        return ( nIndex >= 0 );
    }

    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
