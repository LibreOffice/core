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

#include <uielement/menubarmerger.hxx>
#include <framework/addonsoptions.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::com::sun::star;

static const char SEPARATOR_STRING[]               = "private:separator";

static const char MERGECOMMAND_ADDAFTER[]          = "AddAfter";
static const char MERGECOMMAND_ADDBEFORE[]         = "AddBefore";
static const char MERGECOMMAND_REPLACE[]           = "Replace";
static const char MERGECOMMAND_REMOVE[]            = "Remove";

static const char MERGEFALLBACK_ADDPATH[]           = "AddPath";
static const char MERGEFALLBACK_IGNORE[]            = "Ignore";

namespace framework
{

/**
 Check whether a module identifier is part of a context
 defined by a colon separated list of module identifier.

 @param
     rContext

     Describes a context string list where all contexts
     are delimited by a colon. For more information about
     the module identifier used as context strings see the
     IDL description of css::frame::XModuleManager

  @param
     rModuleIdentifier

     A string describing a module identifier. See IDL
     description of css::frame::XModuleManager.

*/
bool MenuBarMerger::IsCorrectContext( const OUString& rContext, const OUString& rModuleIdentifier )
{
    return ( rContext.isEmpty() || ( rContext.indexOf( rModuleIdentifier ) >= 0 ));
}

void MenuBarMerger::RetrieveReferencePath(
    const OUString& rReferencePathString,
    ::std::vector< OUString >& rReferencePath )
{
    const char aDelimiter = '\\';

    rReferencePath.clear();
    sal_Int32 nIndex( 0 );
    do
    {
        OUString aToken = rReferencePathString.getToken( 0, aDelimiter, nIndex );
        if ( !aToken.isEmpty() )
            rReferencePath.push_back( aToken );
    }
    while ( nIndex >= 0 );
}

ReferencePathInfo MenuBarMerger::FindReferencePath(
    const ::std::vector< OUString >& rReferencePath,
    Menu* pMenu )
{
    sal_uInt32       i( 0 );
    const sal_uInt32 nCount( rReferencePath.size() );

    ReferencePathInfo aResult;
    if ( !nCount )
    {
        aResult.pPopupMenu = nullptr;
        aResult.nPos = 0;
        aResult.nLevel = -1;
        aResult.eResult = RP_MENUITEM_NOT_FOUND;
        return aResult;
    }

    Menu*            pCurrMenu( pMenu );
    RPResultInfo     eResult( RP_OK );

    sal_Int32  nLevel( - 1 );
    sal_uInt16 nPos( MENU_ITEM_NOTFOUND );
    do
    {
        ++nLevel;
        OUString aCmd( rReferencePath[i] );

        if ( i == nCount-1 )
        {
            // Check last reference path element. Must be a leave (menu item).
            sal_uInt16 nTmpPos = FindMenuItem( aCmd, pCurrMenu );
            if ( nTmpPos != MENU_ITEM_NOTFOUND )
                nPos = nTmpPos;
            eResult = ( nTmpPos != MENU_ITEM_NOTFOUND ) ? RP_OK : RP_MENUITEM_NOT_FOUND;
        }
        else
        {
            // Check reference path element. Must be a node (popup menu)!
            sal_uInt16 nTmpPos = FindMenuItem( aCmd, pCurrMenu );
            if ( nTmpPos != MENU_ITEM_NOTFOUND )
            {
                sal_uInt16 nItemId = pCurrMenu->GetItemId( nTmpPos );
                Menu* pTmpMenu     = pCurrMenu->GetPopupMenu( nItemId );
                if ( pTmpMenu != nullptr )
                    pCurrMenu = pTmpMenu;
                else
                {
                    nPos    = nTmpPos;
                    eResult = RP_MENUITEM_INSTEAD_OF_POPUPMENU_FOUND;
                }
            }
            else
                eResult = RP_POPUPMENU_NOT_FOUND;
        }
        i++;
    }
    while ((i < nCount) && (eResult == RP_OK));

    aResult.pPopupMenu = pCurrMenu;
    aResult.nPos       = nPos;
    aResult.nLevel     = nLevel;
    aResult.eResult    = eResult;

    return aResult;
}

sal_uInt16 MenuBarMerger::FindMenuItem( const OUString& rCmd, Menu const * pCurrMenu )
{
    for ( sal_uInt16 i = 0; i < pCurrMenu->GetItemCount(); i++ )
    {
        const sal_uInt16 nItemId = pCurrMenu->GetItemId( i );
        if ( nItemId > 0 )
        {
            if ( rCmd == pCurrMenu->GetItemCommand( nItemId ) )
                return i;
        }
    }

    return MENU_ITEM_NOTFOUND;
}

bool MenuBarMerger::CreateSubMenu(
    Menu*                     pSubMenu,
    sal_uInt16&               nItemId,
    const OUString&    rModuleIdentifier,
    const AddonMenuContainer& rAddonSubMenu )
{
    const sal_uInt32 nSize = rAddonSubMenu.size();
    for ( sal_uInt32 i = 0; i < nSize; i++ )
    {
        const AddonMenuItem& rMenuItem = rAddonSubMenu[i];

        if ( IsCorrectContext( rMenuItem.aContext, rModuleIdentifier ))
        {
            if ( rMenuItem.aURL == SEPARATOR_STRING )
            {
                pSubMenu->InsertSeparator();
            }
            else
            {
                pSubMenu->InsertItem(nItemId, rMenuItem.aTitle);
                pSubMenu->SetItemCommand( nItemId, rMenuItem.aURL );
                if ( !rMenuItem.aSubMenu.empty() )
                {
                    VclPtr<PopupMenu> pPopupMenu = VclPtr<PopupMenu>::Create();
                    pSubMenu->SetPopupMenu( nItemId, pPopupMenu );
                    ++nItemId;

                    CreateSubMenu( pPopupMenu, nItemId, rModuleIdentifier, rMenuItem.aSubMenu );
                }
                else
                    ++nItemId;
            }
        }
    }

    return true;
}

bool MenuBarMerger::MergeMenuItems(
    Menu*                     pMenu,
    sal_uInt16                nPos,
    sal_uInt16                nModIndex,
    sal_uInt16&               nItemId,
    const OUString&    rModuleIdentifier,
    const AddonMenuContainer& rAddonMenuItems )
{
    sal_uInt16       nIndex( 0 );
    const sal_uInt32 nSize = rAddonMenuItems.size();
    for ( sal_uInt32 i = 0; i < nSize; i++ )
    {
        const AddonMenuItem& rMenuItem = rAddonMenuItems[i];

        if ( IsCorrectContext( rMenuItem.aContext, rModuleIdentifier ))
        {
            if ( rMenuItem.aURL == SEPARATOR_STRING )
            {
                pMenu->InsertSeparator(OString(), nPos+nModIndex+nIndex);
            }
            else
            {
                pMenu->InsertItem(nItemId, rMenuItem.aTitle, MenuItemBits::NONE, OString(), nPos+nModIndex+nIndex);
                pMenu->SetItemCommand( nItemId, rMenuItem.aURL );
                if ( !rMenuItem.aSubMenu.empty() )
                {
                    VclPtr<PopupMenu> pSubMenu = VclPtr<PopupMenu>::Create();
                    pMenu->SetPopupMenu( nItemId, pSubMenu );
                    ++nItemId;

                    CreateSubMenu( pSubMenu, nItemId, rModuleIdentifier, rMenuItem.aSubMenu );
                }
                else
                    ++nItemId;
            }
            ++nIndex;
        }
    }

    return true;
}

bool MenuBarMerger::ReplaceMenuItem(
    Menu*                     pMenu,
    sal_uInt16                nPos,
    sal_uInt16&               rItemId,
    const OUString&    rModuleIdentifier,
    const AddonMenuContainer& rAddonMenuItems )
{
    // There is no replace available. Therefore we first have to
    // remove the old menu entry,
    pMenu->RemoveItem( nPos );

    return MergeMenuItems( pMenu, nPos, 0, rItemId, rModuleIdentifier, rAddonMenuItems );
}

bool MenuBarMerger::RemoveMenuItems(
    Menu*                     pMenu,
    sal_uInt16                nPos,
    const OUString&    rMergeCommandParameter )
{
    const sal_uInt16 nParam( sal_uInt16( rMergeCommandParameter.toInt32() ));
    sal_uInt16       nCount = std::max( nParam, sal_uInt16(1) );

    sal_uInt16 i = 0;
    while (( nPos < pMenu->GetItemCount() ) && ( i < nCount ))
    {
        pMenu->RemoveItem( nPos );
        ++i;
    }

    return true;
}

bool MenuBarMerger::ProcessMergeOperation(
    Menu*                     pMenu,
    sal_uInt16                nPos,
    sal_uInt16&               nItemId,
    const OUString&    rMergeCommand,
    const OUString&    rMergeCommandParameter,
    const OUString&    rModuleIdentifier,
    const AddonMenuContainer& rAddonMenuItems )
{
    sal_uInt16 nModIndex( 0 );

    if ( rMergeCommand == MERGECOMMAND_ADDBEFORE )
    {
        nModIndex = 0;
        return MergeMenuItems( pMenu, nPos, nModIndex, nItemId, rModuleIdentifier, rAddonMenuItems );
    }
    else if ( rMergeCommand == MERGECOMMAND_ADDAFTER )
    {
        nModIndex = 1;
        return MergeMenuItems( pMenu, nPos, nModIndex, nItemId, rModuleIdentifier, rAddonMenuItems );
    }
    else if ( rMergeCommand == MERGECOMMAND_REPLACE )
    {
        return ReplaceMenuItem( pMenu, nPos, nItemId, rModuleIdentifier, rAddonMenuItems );
    }
    else if ( rMergeCommand == MERGECOMMAND_REMOVE )
    {
        return RemoveMenuItems( pMenu, nPos, rMergeCommandParameter );
    }

    return false;
}

bool MenuBarMerger::ProcessFallbackOperation(
    const ReferencePathInfo&                aRefPathInfo,
    sal_uInt16&                             rItemId,
    const OUString&                  rMergeCommand,
    const OUString&                  rMergeFallback,
    const ::std::vector< OUString >& rReferencePath,
    const OUString&                  rModuleIdentifier,
    const AddonMenuContainer&               rAddonMenuItems )
{
    if (( rMergeFallback == MERGEFALLBACK_IGNORE ) ||
        ( rMergeCommand  == MERGECOMMAND_REPLACE ) ||
        ( rMergeCommand  == MERGECOMMAND_REMOVE  ) )
    {
        return true;
    }
    else if ( rMergeFallback == MERGEFALLBACK_ADDPATH )
    {
        Menu*            pCurrMenu( aRefPathInfo.pPopupMenu );
        sal_Int32        nLevel( aRefPathInfo.nLevel );
        const sal_Int32  nSize( rReferencePath.size() );
        bool             bFirstLevel( true );

        while ( nLevel < nSize )
        {
            if ( nLevel == nSize-1 )
            {
                const sal_uInt32 nCount = rAddonMenuItems.size();
                for ( sal_uInt32 i = 0; i < nCount; ++i )
                {
                    const AddonMenuItem& rMenuItem = rAddonMenuItems[i];
                    if ( IsCorrectContext( rMenuItem.aContext, rModuleIdentifier ))
                    {
                        if ( rMenuItem.aURL == SEPARATOR_STRING )
                            pCurrMenu->InsertSeparator();
                        else
                        {
                            pCurrMenu->InsertItem(rItemId, rMenuItem.aTitle);
                            pCurrMenu->SetItemCommand( rItemId, rMenuItem.aURL );
                            ++rItemId;
                        }
                    }
                }
            }
            else
            {
                const OUString aCmd( rReferencePath[nLevel] );

                sal_uInt16 nInsPos( MENU_APPEND );
                VclPtr<PopupMenu> pPopupMenu = VclPtr<PopupMenu>::Create();

                if ( bFirstLevel && ( aRefPathInfo.eResult == RP_MENUITEM_INSTEAD_OF_POPUPMENU_FOUND ))
                {
                    // special case: menu item without popup
                    nInsPos = aRefPathInfo.nPos;
                    sal_uInt16 nSetItemId = pCurrMenu->GetItemId( nInsPos );
                    pCurrMenu->SetItemCommand( nSetItemId, aCmd );
                    pCurrMenu->SetPopupMenu( nSetItemId, pPopupMenu );
                }
                else
                {
                    // normal case: insert a new item with popup
                    pCurrMenu->InsertItem(rItemId, OUString());
                    pCurrMenu->SetItemCommand( rItemId, aCmd );
                    pCurrMenu->SetPopupMenu( rItemId, pPopupMenu );
                }

                pCurrMenu = pPopupMenu;
                ++rItemId;
                bFirstLevel = false;
            }
            ++nLevel;
        }
        return true;
    }

    return false;
}

void MenuBarMerger::GetMenuEntry(
    const uno::Sequence< beans::PropertyValue >& rAddonMenuEntry,
    AddonMenuItem&                               rAddonMenuItem )
{
    // Reset submenu member
    rAddonMenuItem.aSubMenu.clear();

    for ( const beans::PropertyValue& rProp : rAddonMenuEntry )
    {
        OUString aMenuEntryPropName = rProp.Name;
        if ( aMenuEntryPropName == ADDONSMENUITEM_STRING_URL )
            rProp.Value >>= rAddonMenuItem.aURL;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_STRING_TITLE )
            rProp.Value >>= rAddonMenuItem.aTitle;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_STRING_SUBMENU )
        {
            uno::Sequence< uno::Sequence< beans::PropertyValue > > aSubMenu;
            rProp.Value >>= aSubMenu;
            GetSubMenu( aSubMenu, rAddonMenuItem.aSubMenu );
        }
        else if ( aMenuEntryPropName == ADDONSMENUITEM_STRING_CONTEXT )
            rProp.Value >>= rAddonMenuItem.aContext;
    }
}

void MenuBarMerger::GetSubMenu(
    const uno::Sequence< uno::Sequence< beans::PropertyValue > >& rSubMenuEntries,
    AddonMenuContainer&                                           rSubMenu )
{
    rSubMenu.clear();

    const sal_Int32 nCount = rSubMenuEntries.getLength();
    rSubMenu.reserve(rSubMenu.size() + nCount);
    for ( sal_Int32 i = 0; i < nCount; i++ )
    {
        const uno::Sequence< beans::PropertyValue >& rMenuEntry = rSubMenuEntries[ i ];

        AddonMenuItem aMenuItem;
        GetMenuEntry( rMenuEntry, aMenuItem );
        rSubMenu.push_back( aMenuItem );
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
