/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include "framework/addonmenu.hxx"
#include "framework/addonsoptions.hxx"
#include <general.h>
#include <macros/debug/assertion.hxx>
#include <framework/imageproducer.hxx>
#include <framework/menuconfiguration.hxx>
#include <services.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
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
const sal_uInt16 SID_HELP_SUPPORTPAGE = (SID_SFX_START + 1683);

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

static ::rtl::OUString GetModuleIdentifier( const Reference< XMultiServiceFactory >& rServiceManager,
                                            const Reference< XFrame >& rFrame )
{
    Reference< XModuleManager > xModuleManager( rServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY );
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
    return ::rtl::OUString();
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
AddonMenu* AddonMenuManager::CreateAddonMenu( const Reference< XFrame >& rFrame,
                                              const Reference< XMultiServiceFactory >& rServiceManager )
{
    AddonsOptions aOptions;
    AddonMenu*  pAddonMenu      = NULL;
    sal_uInt16      nUniqueMenuId   = ADDONMENU_ITEMID_START;

    const Sequence< Sequence< PropertyValue > >& rAddonMenuEntries = aOptions.GetAddonsMenu();
    if ( rAddonMenuEntries.getLength() > 0 )
    {
        pAddonMenu = (AddonMenu *)AddonMenuManager::CreatePopupMenuType( ADDON_MENU, rFrame );
        ::rtl::OUString aModuleIdentifier = GetModuleIdentifier( rServiceManager, rFrame );
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
void AddonMenuManager::MergeAddonHelpMenu( const Reference< XFrame >& rFrame,
                                           MenuBar* pMergeMenuBar,
                                           const Reference< XMultiServiceFactory >& rServiceManager )
{
    if ( pMergeMenuBar )
    {
        PopupMenu* pHelpMenu = pMergeMenuBar->GetPopupMenu( SID_HELPMENU );
        if ( !pHelpMenu )
        {
            sal_uInt16 nId = FindMenuId( pMergeMenuBar, String::CreateFromAscii( ".uno:HelpMenu" ));
            if ( nId != USHRT_MAX )
                pHelpMenu = pMergeMenuBar->GetPopupMenu( nId );
        }

        if ( pHelpMenu )
        {
            static const char REFERENCECOMMAND_AFTER[]  = ".uno:HelpSupport";
            static const char REFERENCECOMMAND_BEFORE[] = ".uno:About";

            // Add-Ons help menu items should be inserted after the "registration" menu item
            bool   bAddAfter        = true;
            sal_uInt16 nItemCount       = pHelpMenu->GetItemCount();
            sal_uInt16 nSupPos          = pHelpMenu->GetItemPos( SID_HELP_SUPPORTPAGE );
            sal_uInt16 nInsPos          = nSupPos;
            sal_uInt16 nInsSepAfterPos  = MENU_APPEND;
            sal_uInt16 nUniqueMenuId    = ADDONMENU_ITEMID_START;
            AddonsOptions aOptions;

            if ( nSupPos == USHRT_MAX )
            {
                // try to detect the online registration dialog menu item with the command URL
                sal_uInt16 nId = FindMenuId( pHelpMenu, String::CreateFromAscii( REFERENCECOMMAND_AFTER ));
                nSupPos    = pHelpMenu->GetItemPos( nId );
                nInsPos    = nSupPos;
            }

            if ( nSupPos == USHRT_MAX )
            {
                // second try:
                // try to detect the about menu item with the command URL
                sal_uInt16 nId = FindMenuId( pHelpMenu, String::CreateFromAscii( REFERENCECOMMAND_BEFORE ));
                nSupPos    = pHelpMenu->GetItemPos( nId );
                nInsPos    = nSupPos;
                bAddAfter  = false;
            }

            Sequence< Sequence< PropertyValue > > aAddonSubMenu;
            const Sequence< Sequence< PropertyValue > >& rAddonHelpMenuEntries = aOptions.GetAddonsHelpMenu();

            nInsPos = bAddAfter ? AddonMenuManager::GetNextPos( nInsPos ) : nInsPos;
            if ( nInsPos < nItemCount && pHelpMenu->GetItemType( nInsPos ) != MENUITEM_SEPARATOR )
                nInsSepAfterPos = nInsPos;

            ::rtl::OUString aModuleIdentifier = GetModuleIdentifier( rServiceManager, rFrame );
            AddonMenuManager::BuildMenu( pHelpMenu, ADDON_MENU, nInsPos, nUniqueMenuId, rAddonHelpMenuEntries, rFrame, aModuleIdentifier );

            if ( pHelpMenu->GetItemCount() > nItemCount )
            {
                if ( nInsSepAfterPos < MENU_APPEND )
                {
                    nInsSepAfterPos += ( pHelpMenu->GetItemCount() - nItemCount );
                    if ( pHelpMenu->GetItemType( nInsSepAfterPos ) != MENUITEM_SEPARATOR )
                        pHelpMenu->InsertSeparator( nInsSepAfterPos );
                }
                if ( nSupPos < MENU_APPEND )
                    pHelpMenu->InsertSeparator( nSupPos+1 );
                else
                    pHelpMenu->InsertSeparator( nItemCount );
            }
        }
    }
}

// Merge the addon popup menus into the given menu bar at the provided pos.
void AddonMenuManager::MergeAddonPopupMenus( const Reference< XFrame >& rFrame,
                                             sal_uInt16               nMergeAtPos,
                                             MenuBar*             pMergeMenuBar,
                                             const Reference< XMultiServiceFactory >& rServiceManager )
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
        ::rtl::OUString                              aModuleIdentifier = GetModuleIdentifier( rServiceManager, rFrame );

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
                 AddonMenuManager::IsCorrectContext( aModuleIdentifier, aContext ))
            {
                sal_uInt16          nId             = nUniqueMenuId++;
                AddonPopupMenu* pAddonPopupMenu = (AddonPopupMenu *)AddonMenuManager::CreatePopupMenuType( ADDON_POPUPMENU, rFrame );

                AddonMenuManager::BuildMenu( pAddonPopupMenu, ADDON_MENU, MENU_APPEND, nUniqueMenuId, aAddonSubMenu, rFrame, aModuleIdentifier );

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
                                  const ::rtl::OUString&               rModuleIdentifier )
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

        if ( !IsCorrectContext( rModuleIdentifier, aContext ) || ( !aTitle.getLength() && !aURL.getLength() ))
            continue;

        if ( aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:separator" )))
            bInsertSeparator = sal_True;
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
sal_Bool AddonMenuManager::IsCorrectContext( const ::rtl::OUString& rModuleIdentifier, const ::rtl::OUString& aContext )
{
    if ( aContext.isEmpty() )
        return sal_True;

    if ( !rModuleIdentifier.isEmpty() )
    {
        sal_Int32 nIndex = aContext.indexOf( rModuleIdentifier );
        return ( nIndex >= 0 );
    }

    return sal_False;
}

}

