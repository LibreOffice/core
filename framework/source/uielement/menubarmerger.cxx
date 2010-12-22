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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <uielement/menubarmerger.hxx>
#include <framework/addonsoptions.hxx>

using namespace ::com::sun::star;

static const char SEPARATOR_STRING[]               = "private:separator";
static const sal_uInt32 SEPARATOR_STRING_LEN       = 17;

static const char MERGECOMMAND_ADDAFTER[]          = "AddAfter";
static const sal_uInt32 MERGECOMMAND_ADDAFTER_LEN  = 8;
static const char MERGECOMMAND_ADDBEFORE[]         = "AddBefore";
static const sal_uInt32 MERGECOMMAND_ADDBEFORE_LEN = 9;
static const char MERGECOMMAND_REPLACE[]           = "Replace";
static const sal_uInt32 MERGECOMMAND_REPLACE_LEN   = 7;
static const char MERGECOMMAND_REMOVE[]            = "Remove";
static const sal_uInt32 MERGECOMMAND_REMOVE_LEN    = 6;

static const char MERGEFALLBACK_ADDPATH[]           = "AddPath";
static const char MERGEFALLBACK_ADDPATH_LEN         = 7;
static const char MERGEFALLBACK_IGNORE[]            = "Ignore";
static const char MERGEFALLBACK_IGNORE_LEN          = 6;


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
     IDL description of com::sun::star::frame::XModuleManager

  @param
     rModuleIdentifier

     A string describing a module identifier. See IDL
     description of com::sun::star::frame::XModuleManager.

*/
bool MenuBarMerger::IsCorrectContext( const ::rtl::OUString& rContext, const ::rtl::OUString& rModuleIdentifier )
{
    return (( rContext.getLength() == 0 ) || ( rContext.indexOf( rModuleIdentifier ) >= 0 ));
}

void MenuBarMerger::RetrieveReferencePath(
    const ::rtl::OUString& rReferencePathString,
    ::std::vector< ::rtl::OUString >& rReferencePath )
{
    const sal_Char aDelimiter = '\\';

    rReferencePath.clear();
    sal_Int32 nIndex( 0 );
    do
    {
        ::rtl::OUString aToken = rReferencePathString.getToken( 0, aDelimiter, nIndex );
        if ( aToken.getLength() > 0 )
            rReferencePath.push_back( aToken );
    }
    while ( nIndex >= 0 );
}

ReferencePathInfo MenuBarMerger::FindReferencePath(
    const ::std::vector< ::rtl::OUString >& rReferencePath,
    Menu* pMenu )
{
    sal_uInt32       i( 0 );
    const sal_uInt32 nCount( rReferencePath.size() );
    Menu*            pCurrMenu( pMenu );
    RPResultInfo     eResult( RP_OK );

    sal_Int32  nLevel( - 1 );
    sal_uInt16 nPos( MENU_ITEM_NOTFOUND );
    do
    {
        ++nLevel;
        ::rtl::OUString aCmd( rReferencePath[i] );

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
                if ( pTmpMenu != 0 )
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
    while (( pCurrMenu != 0 ) && ( i < nCount ) && ( eResult == RP_OK ));

    ReferencePathInfo aResult;
    aResult.pPopupMenu = pCurrMenu;
    aResult.nPos       = nPos;
    aResult.nLevel     = nLevel;
    aResult.eResult    = eResult;

    return aResult;
}

sal_uInt16 MenuBarMerger::FindMenuItem( const ::rtl::OUString& rCmd, Menu* pCurrMenu )
{
    for ( sal_uInt16 i = 0; i < pCurrMenu->GetItemCount(); i++ )
    {
        const sal_uInt16 nItemId = pCurrMenu->GetItemId( i );
        if ( nItemId > 0 )
        {
            if ( rCmd == ::rtl::OUString( pCurrMenu->GetItemCommand( nItemId )))
                return i;
        }
    }

    return MENU_ITEM_NOTFOUND;
}

bool MenuBarMerger::CreateSubMenu(
    Menu*                     pSubMenu,
    sal_uInt16&               nItemId,
    const ::rtl::OUString&    rModuleIdentifier,
    const AddonMenuContainer& rAddonSubMenu )
{
    const sal_uInt32 nSize = rAddonSubMenu.size();
    for ( sal_uInt32 i = 0; i < nSize; i++ )
    {
        const AddonMenuItem& rMenuItem = rAddonSubMenu[i];

        if ( IsCorrectContext( rMenuItem.aContext, rModuleIdentifier ))
        {
            if ( rMenuItem.aURL.equalsAsciiL( SEPARATOR_STRING, SEPARATOR_STRING_LEN ))
            {
                pSubMenu->InsertSeparator( MENU_APPEND );
            }
            else
            {
                pSubMenu->InsertItem( nItemId, rMenuItem.aTitle, 0, MENU_APPEND );
                pSubMenu->SetItemCommand( nItemId, rMenuItem.aURL );
                if ( !rMenuItem.aSubMenu.empty() )
                {
                    PopupMenu* pPopupMenu = new PopupMenu();
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
    const ::rtl::OUString&    rModuleIdentifier,
    const AddonMenuContainer& rAddonMenuItems )
{
    sal_uInt16       nIndex( 0 );
    const sal_uInt32 nSize = rAddonMenuItems.size();
    for ( sal_uInt32 i = 0; i < nSize; i++ )
    {
        const AddonMenuItem& rMenuItem = rAddonMenuItems[i];

        if ( IsCorrectContext( rMenuItem.aContext, rModuleIdentifier ))
        {
            if ( rMenuItem.aURL.equalsAsciiL( SEPARATOR_STRING, SEPARATOR_STRING_LEN ))
            {
                pMenu->InsertSeparator( nPos+nModIndex+nIndex );
            }
            else
            {
                pMenu->InsertItem( nItemId, rMenuItem.aTitle, 0, nPos+nModIndex+nIndex );
                pMenu->SetItemCommand( nItemId, rMenuItem.aURL );
                if ( !rMenuItem.aSubMenu.empty() )
                {
                    PopupMenu* pSubMenu = new PopupMenu();
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
    const ::rtl::OUString&    rModuleIdentifier,
    const AddonMenuContainer& rAddonMenuItems )
{
    // There is no replace available. Therfore we first have to
    // remove the old menu entry,
    pMenu->RemoveItem( nPos );

    return MergeMenuItems( pMenu, nPos, 0, rItemId, rModuleIdentifier, rAddonMenuItems );
}

bool MenuBarMerger::RemoveMenuItems(
    Menu*                     pMenu,
    sal_uInt16                nPos,
    const ::rtl::OUString&    rMergeCommandParameter )
{
    const sal_uInt16 nParam( sal_uInt16( rMergeCommandParameter.toInt32() ));
    sal_uInt16       nCount( 1 );

    nCount = std::max( nParam, nCount );

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
    const ::rtl::OUString&    rMergeCommand,
    const ::rtl::OUString&    rMergeCommandParameter,
    const ::rtl::OUString&    rModuleIdentifier,
    const AddonMenuContainer& rAddonMenuItems )
{
    sal_uInt16 nModIndex( 0 );

    if ( rMergeCommand.equalsAsciiL( MERGECOMMAND_ADDBEFORE, MERGECOMMAND_ADDBEFORE_LEN ))
    {
        nModIndex = 0;
        return MergeMenuItems( pMenu, nPos, nModIndex, nItemId, rModuleIdentifier, rAddonMenuItems );
    }
    else if ( rMergeCommand.equalsAsciiL( MERGECOMMAND_ADDAFTER, MERGECOMMAND_ADDAFTER_LEN ))
    {
        nModIndex = 1;
        return MergeMenuItems( pMenu, nPos, nModIndex, nItemId, rModuleIdentifier, rAddonMenuItems );
    }
    else if ( rMergeCommand.equalsAsciiL( MERGECOMMAND_REPLACE, MERGECOMMAND_REPLACE_LEN ))
    {
        return ReplaceMenuItem( pMenu, nPos, nItemId, rModuleIdentifier, rAddonMenuItems );
    }
    else if ( rMergeCommand.equalsAsciiL( MERGECOMMAND_REMOVE, MERGECOMMAND_REMOVE_LEN ))
    {
        return RemoveMenuItems( pMenu, nPos, rMergeCommandParameter );
    }

    return false;
}

bool MenuBarMerger::ProcessFallbackOperation(
    const ReferencePathInfo&                aRefPathInfo,
    sal_uInt16&                             rItemId,
    const ::rtl::OUString&                  rMergeCommand,
    const ::rtl::OUString&                  rMergeFallback,
    const ::std::vector< ::rtl::OUString >& rReferencePath,
    const ::rtl::OUString&                  rModuleIdentifier,
    const AddonMenuContainer&               rAddonMenuItems )
{
    if (( rMergeFallback.equalsAsciiL( MERGEFALLBACK_IGNORE, MERGEFALLBACK_IGNORE_LEN )) ||
        ( rMergeCommand.equalsAsciiL(  MERGECOMMAND_REPLACE, MERGECOMMAND_REPLACE_LEN )) ||
        ( rMergeCommand.equalsAsciiL(  MERGECOMMAND_REMOVE, MERGECOMMAND_REMOVE_LEN   )) )
    {
        return true;
    }
    else if ( rMergeFallback.equalsAsciiL( MERGEFALLBACK_ADDPATH, MERGEFALLBACK_ADDPATH_LEN ))
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
                        if ( rMenuItem.aURL.equalsAsciiL( SEPARATOR_STRING, SEPARATOR_STRING_LEN ))
                            pCurrMenu->InsertSeparator( MENU_APPEND );
                        else
                        {
                            pCurrMenu->InsertItem( rItemId, rMenuItem.aTitle, 0, MENU_APPEND );
                            pCurrMenu->SetItemCommand( rItemId, rMenuItem.aURL );
                            ++rItemId;
                        }
                    }
                }
            }
            else
            {
                const ::rtl::OUString aCmd( rReferencePath[nLevel] );

                sal_uInt16 nInsPos( MENU_APPEND );
                PopupMenu* pPopupMenu( new PopupMenu );

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
                    pCurrMenu->InsertItem( rItemId, ::rtl::OUString(), 0, MENU_APPEND );
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

    for ( sal_Int32 i = 0; i < rAddonMenuEntry.getLength(); i++ )
    {
        ::rtl::OUString aMenuEntryPropName = rAddonMenuEntry[i].Name;
        if ( aMenuEntryPropName.equalsAsciiL( ADDONSMENUITEM_STRING_URL, ADDONSMENUITEM_URL_LEN ))
            rAddonMenuEntry[i].Value >>= rAddonMenuItem.aURL;
        else if ( aMenuEntryPropName.equalsAsciiL( ADDONSMENUITEM_STRING_TITLE, ADDONSMENUITEM_TITLE_LEN ))
            rAddonMenuEntry[i].Value >>= rAddonMenuItem.aTitle;
        else if ( aMenuEntryPropName.equalsAsciiL( ADDONSMENUITEM_STRING_TARGET, ADDONSMENUITEM_TARGET_LEN ))
            rAddonMenuEntry[i].Value >>= rAddonMenuItem.aTarget;
        else if ( aMenuEntryPropName.equalsAsciiL( ADDONSMENUITEM_STRING_SUBMENU, ADDONSMENUITEM_SUBMENU_LEN ))
        {
            uno::Sequence< uno::Sequence< beans::PropertyValue > > aSubMenu;
            rAddonMenuEntry[i].Value >>= aSubMenu;
            GetSubMenu( aSubMenu, rAddonMenuItem.aSubMenu );
        }
        else if ( aMenuEntryPropName.equalsAsciiL( ADDONSMENUITEM_STRING_CONTEXT, ADDONSMENUITEM_CONTEXT_LEN ))
            rAddonMenuEntry[i].Value >>= rAddonMenuItem.aContext;
        else if ( aMenuEntryPropName.equalsAsciiL( ADDONSMENUITEM_STRING_IMAGEIDENTIFIER, ADDONSMENUITEM_IMAGEIDENTIFIER_LEN ))
            rAddonMenuEntry[i].Value >>= rAddonMenuItem.aImageId;
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
