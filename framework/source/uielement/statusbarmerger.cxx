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
#include <uielement/statusbarmerger.hxx>

using com::sun::star::beans::PropertyValue;
using com::sun::star::uno::Sequence;

namespace framework
{
namespace {

static const char MERGECOMMAND_ADDAFTER[]       = "AddAfter";
static const char MERGECOMMAND_ADDBEFORE[]      = "AddBefore";
static const char MERGECOMMAND_REPLACE[]        = "Replace";
static const char MERGECOMMAND_REMOVE[]         = "Remove";

void lcl_ConvertSequenceToValues(
    const Sequence< PropertyValue > &rSequence,
    AddonStatusbarItem &rItem )
{
    OUString sAlignment;
    bool bAutoSize = false;
    bool bOwnerDraw = false;
    bool bMandatory = true;

    for ( PropertyValue const & aPropVal : rSequence )
    {
        if ( aPropVal.Name == "URL" )
            aPropVal.Value >>= rItem.aCommandURL;
        else if ( aPropVal.Name == "Title" )
            aPropVal.Value >>= rItem.aLabel;
        else if ( aPropVal.Name == "Context" )
            aPropVal.Value >>= rItem.aContext;
        else if ( aPropVal.Name == "Alignment" )
            aPropVal.Value >>= sAlignment;
        else if ( aPropVal.Name == "AutoSize" )
            aPropVal.Value >>= bAutoSize;
        else if ( aPropVal.Name == "OwnerDraw" )
            aPropVal.Value >>= bOwnerDraw;
        else if ( aPropVal.Name == "Mandatory" )
            aPropVal.Value >>= bMandatory;
        else if ( aPropVal.Name == "Width" )
        {
            sal_Int32 aWidth = 0;
            aPropVal.Value >>= aWidth;
            rItem.nWidth = sal_uInt16( aWidth );
        }
    }

    StatusBarItemBits nItemBits(StatusBarItemBits::NONE);
    if ( bAutoSize )
        nItemBits |= StatusBarItemBits::AutoSize;
    if ( bOwnerDraw )
        nItemBits |= StatusBarItemBits::UserDraw;
    if ( bMandatory )
        nItemBits |= StatusBarItemBits::Mandatory;
    if ( sAlignment == "center" )
        nItemBits |= StatusBarItemBits::Center;
    else if ( sAlignment == "right" )
        nItemBits |= StatusBarItemBits::Right;
    else
        // if unset, defaults to left alignment
        nItemBits |= StatusBarItemBits::Left;
    rItem.nItemBits = nItemBits;
}

void lcl_CreateStatusbarItem( StatusBar* pStatusbar,
                                     sal_uInt16 nPos,
                                     sal_uInt16 nItemId,
                                     const AddonStatusbarItem& rAddonItem )
{
    pStatusbar->InsertItem( nItemId,
                            rAddonItem.nWidth,
                            rAddonItem.nItemBits,
                            STATUSBAR_OFFSET,
                            nPos );
    pStatusbar->SetItemCommand( nItemId, rAddonItem.aCommandURL );
    pStatusbar->SetQuickHelpText( nItemId, rAddonItem.aLabel );
    pStatusbar->SetAccessibleName( nItemId, rAddonItem.aLabel );

    // add-on specific data
    AddonStatusbarItemData *pUserData = new AddonStatusbarItemData;
    pUserData->aLabel = rAddonItem.aLabel;
    pStatusbar->SetItemData( nItemId, pUserData );
}

bool lcl_MergeItems( StatusBar* pStatusbar,
                            sal_uInt16 nPos,
                            sal_uInt16 nModIndex,
                            sal_uInt16& rItemId,
                            const AddonStatusbarItemContainer& rAddonItems )
{
    const sal_uInt16 nSize( rAddonItems.size() );
    for ( sal_Int32 i = 0; i < nSize; i++ )
    {
        const AddonStatusbarItem& rItem = rAddonItems[i];
        if ( !StatusbarMerger::IsCorrectContext( rItem.aContext ) )
            continue;

        sal_uInt16 nInsPos = nPos + nModIndex + i;
        if ( nInsPos > pStatusbar->GetItemCount() )
            nInsPos = STATUSBAR_APPEND;

        lcl_CreateStatusbarItem( pStatusbar, nInsPos, rItemId, rItem );
        ++rItemId;
    }

    return true;
}

bool lcl_ReplaceItem( StatusBar* pStatusbar,
                             sal_uInt16 nPos,
                             sal_uInt16& rItemId,
                             const AddonStatusbarItemContainer& rAddonToolbarItems )
{
    pStatusbar->RemoveItem( pStatusbar->GetItemId( nPos ) );
    return lcl_MergeItems( pStatusbar, nPos, 0, rItemId, rAddonToolbarItems );
}

bool lcl_RemoveItems( StatusBar* pStatusbar,
                             sal_uInt16 nPos,
                             const OUString& rMergeCommandParameter )
{
    sal_Int32 nCount = rMergeCommandParameter.toInt32();
    if ( nCount > 0 )
    {
        for ( sal_Int32 i = 0; i < nCount; i++ )
        {
            if ( nPos < pStatusbar->GetItemCount() )
                pStatusbar->RemoveItem( nPos );
        }
    }
    return true;
}

}

bool StatusbarMerger::IsCorrectContext(
    const OUString& rContext )
{
    return rContext.isEmpty();
}

bool StatusbarMerger::ConvertSeqSeqToVector(
    const Sequence< Sequence< PropertyValue > > &rSequence,
    AddonStatusbarItemContainer& rContainer )
{
    for ( auto const & i : rSequence )
    {
        AddonStatusbarItem aStatusBarItem;
        lcl_ConvertSequenceToValues( i, aStatusBarItem );
        rContainer.push_back( aStatusBarItem );
    }

    return true;
}

sal_uInt16 StatusbarMerger::FindReferencePos(
    StatusBar* pStatusbar,
    const OUString& rReferencePoint )
{
    for ( sal_uInt16 nPos = 0; nPos < pStatusbar->GetItemCount(); nPos++ )
    {
        const OUString rCmd = pStatusbar->GetItemCommand( pStatusbar->GetItemId( nPos ) );
        if ( rReferencePoint == rCmd )
            return nPos;
    }

    return STATUSBAR_ITEM_NOTFOUND;
}

bool StatusbarMerger::ProcessMergeOperation(
    StatusBar* pStatusbar,
    sal_uInt16 nPos,
    sal_uInt16& rItemId,
    const OUString& rMergeCommand,
    const OUString& rMergeCommandParameter,
    const AddonStatusbarItemContainer& rItems )
{
    if ( rMergeCommand == MERGECOMMAND_ADDAFTER )
        return lcl_MergeItems( pStatusbar, nPos, 1, rItemId, rItems );
    else if ( rMergeCommand == MERGECOMMAND_ADDBEFORE )
        return lcl_MergeItems( pStatusbar, nPos, 0, rItemId, rItems );
    else if ( rMergeCommand == MERGECOMMAND_REPLACE )
        return lcl_ReplaceItem( pStatusbar, nPos, rItemId, rItems );
    else if ( rMergeCommand == MERGECOMMAND_REMOVE )
        return lcl_RemoveItems( pStatusbar, nPos, rMergeCommandParameter );

    return false;
}

bool StatusbarMerger::ProcessMergeFallback(
    StatusBar* pStatusbar,
    sal_uInt16& rItemId,
    const OUString& rMergeCommand,
    const OUString& rMergeFallback,
    const AddonStatusbarItemContainer& rItems )
{
    // fallback IGNORE or REPLACE/REMOVE item not found
    if (( rMergeFallback == "Ignore" ) ||
            ( rMergeCommand == MERGECOMMAND_REPLACE ) ||
            ( rMergeCommand == MERGECOMMAND_REMOVE  ) )
    {
        return true;
    }
    else if (( rMergeCommand == MERGECOMMAND_ADDBEFORE ) ||
             ( rMergeCommand == MERGECOMMAND_ADDAFTER ) )
    {
        if ( rMergeFallback == "AddFirst" )
            return lcl_MergeItems( pStatusbar, 0, 0, rItemId, rItems );
        else if ( rMergeFallback == "AddLast" )
            return lcl_MergeItems( pStatusbar, STATUSBAR_APPEND, 0, rItemId, rItems );
    }

    return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
