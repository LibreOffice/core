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

#include <uiitems.hxx>

#include <userlist.hxx>
#include <dpsave.hxx>
#include <queryparam.hxx>

#include <osl/diagnose.h>
#include <editeng/editobj.hxx>

/**
 * Status update for entry field
 */
ScInputStatusItem::ScInputStatusItem(
    sal_uInt16 nWhichP, const ScAddress& rCurPos, const ScAddress& rStartPos,
    const ScAddress& rEndPos, const OUString& rString, const EditTextObject* pData ) :
    SfxPoolItem ( nWhichP ),
    aCursorPos  ( rCurPos ),
    aStartPos   ( rStartPos ),
    aEndPos     ( rEndPos ),
    aString     ( rString ),
    pEditData   ( pData ? pData->Clone() : nullptr ),
    mpMisspellRanges(nullptr)
{
}

ScInputStatusItem::ScInputStatusItem( const ScInputStatusItem& rItem ) :
    SfxPoolItem ( rItem ),
    aCursorPos  ( rItem.aCursorPos ),
    aStartPos   ( rItem.aStartPos ),
    aEndPos     ( rItem.aEndPos ),
    aString     ( rItem.aString ),
    pEditData   ( rItem.pEditData ? rItem.pEditData->Clone() : nullptr ),
    mpMisspellRanges(rItem.mpMisspellRanges)
{
}

ScInputStatusItem::~ScInputStatusItem()
{
}

bool ScInputStatusItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    return (aStartPos  == static_cast<const ScInputStatusItem&>(rItem).aStartPos)
             && (aEndPos    == static_cast<const ScInputStatusItem&>(rItem).aEndPos)
             && (aCursorPos == static_cast<const ScInputStatusItem&>(rItem).aCursorPos)
             && (aString    == static_cast<const ScInputStatusItem&>(rItem).aString);
             //TODO: Compare Edit data!
}

ScInputStatusItem* ScInputStatusItem::Clone( SfxItemPool * ) const
{
    return new ScInputStatusItem( *this );
}

void ScInputStatusItem::SetMisspellRanges( const std::vector<editeng::MisspellRanges>* pRanges )
{
    mpMisspellRanges = pRanges;
}

// ScPaintHint was moved to hints.cxx

/**
 * Adapt Views when inserting/deleting a table
 */
ScTablesHint::ScTablesHint(sal_uInt16 nNewId, SCTAB nTable1, SCTAB nTable2) :
    nId( nNewId ),
    nTab1( nTable1 ),
    nTab2( nTable2 )
{
}

ScTablesHint::~ScTablesHint()
{
}

ScIndexHint::ScIndexHint(SfxHintId nNewId, sal_uInt16 nIdx) :
    SfxHint( nNewId ),
    nIndex( nIdx )
{
}

ScIndexHint::~ScIndexHint()
{
}

/**
 * Create new EditView for Cursorposition
 */
ScEditViewHint::ScEditViewHint( ScEditEngineDefaulter* pEngine, const ScAddress& rCurPos ) :
    pEditEngine( pEngine ),
    aCursorPos( rCurPos )
{
}

ScEditViewHint::~ScEditViewHint()
{
}

/**
 * Data for the sorting dialog
 */
ScSortItem::ScSortItem( sal_uInt16              nWhichP,
                        ScViewData*         ptrViewData,
                        const ScSortParam*  pSortData ) :
        SfxPoolItem ( nWhichP ),
        pViewData   ( ptrViewData )
{
    if ( pSortData ) theSortData = *pSortData;
}

ScSortItem::ScSortItem( sal_uInt16              nWhichP,
                        const ScSortParam*  pSortData ) :
        SfxPoolItem ( nWhichP ),
        pViewData   ( nullptr )
{
    if ( pSortData ) theSortData = *pSortData;
}

bool ScSortItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScSortItem& rOther = static_cast<const ScSortItem&>(rItem);

    return (   (pViewData   == rOther.pViewData)
            && (theSortData == rOther.theSortData) );
}

ScSortItem* ScSortItem::Clone( SfxItemPool * ) const
{
    return new ScSortItem( *this );
}

bool ScSortItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /* nMemberUd */ ) const
{
    // Return empty value as there is no useful conversion
    rVal = css::uno::Any();
    return true;
}

/**
 * Data for the Filter dialog
 */
ScQueryItem::ScQueryItem( sal_uInt16                nWhichP,
                          ScViewData*           ptrViewData,
                          const ScQueryParam*   pQueryData ) :
        SfxPoolItem ( nWhichP ),
        pViewData   ( ptrViewData ),
        bIsAdvanced ( false )
{
    if (pQueryData)
        mpQueryData.reset(new ScQueryParam(*pQueryData));
    else
        mpQueryData.reset(new ScQueryParam);
}

ScQueryItem::ScQueryItem( sal_uInt16                nWhichP,
                          const ScQueryParam*   pQueryData ) :
        SfxPoolItem ( nWhichP ),
        pViewData   ( nullptr ),
        bIsAdvanced ( false )
{
    if (pQueryData)
        mpQueryData.reset(new ScQueryParam(*pQueryData));
    else
        mpQueryData.reset(new ScQueryParam);
}

ScQueryItem::ScQueryItem( const ScQueryItem& rItem ) :
        SfxPoolItem ( rItem ),
        mpQueryData(new ScQueryParam(*rItem.mpQueryData)),
        pViewData   ( rItem.pViewData ),
        aAdvSource  ( rItem.aAdvSource ),
        bIsAdvanced ( rItem.bIsAdvanced )
{
}

ScQueryItem::~ScQueryItem()
{
}

void ScQueryItem::SetAdvancedQuerySource(const ScRange* pSource)
{
    if (pSource)
    {
        aAdvSource = *pSource;
        bIsAdvanced = true;
    }
    else
        bIsAdvanced = false;
}

const ScQueryParam& ScQueryItem::GetQueryData() const
{
    return *mpQueryData;
}

bool ScQueryItem::GetAdvancedQuerySource(ScRange& rSource) const
{
    rSource = aAdvSource;
    return bIsAdvanced;
}

bool ScQueryItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScQueryItem& rQueryItem = static_cast<const ScQueryItem&>(rItem);

    return (   (pViewData    == rQueryItem.pViewData)
            && (bIsAdvanced  == rQueryItem.bIsAdvanced)
            && (aAdvSource   == rQueryItem.aAdvSource)
            && (*mpQueryData == *rQueryItem.mpQueryData) );
}

ScQueryItem* ScQueryItem::Clone( SfxItemPool * ) const
{
    return new ScQueryItem( *this );
}

/**
 * Data for the SubTotal dialog
 */
ScSubTotalItem::ScSubTotalItem( sal_uInt16                  nWhichP,
                                ScViewData*             ptrViewData,
                                const ScSubTotalParam*  pSubTotalData ) :
        SfxPoolItem ( nWhichP ),
        pViewData   ( ptrViewData )
{
    if ( pSubTotalData ) theSubTotalData = *pSubTotalData;
}

bool ScSubTotalItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScSubTotalItem& rSTItem = static_cast<const ScSubTotalItem&>(rItem);

    return (   (pViewData       == rSTItem.pViewData)
            && (theSubTotalData == rSTItem.theSubTotalData) );
}

ScSubTotalItem* ScSubTotalItem::Clone( SfxItemPool * ) const
{
    return new ScSubTotalItem( *this );
}

bool ScSubTotalItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /* nMemberUd */ ) const
{
    // Return empty value as there is no useful conversion
    rVal = css::uno::Any();
    return true;
}

/**
 * Transporter for the UserLIst dialog
 */
ScUserListItem::ScUserListItem( sal_uInt16 nWhichP )
    :   SfxPoolItem ( nWhichP )
{
}

ScUserListItem::ScUserListItem( const ScUserListItem& rItem )
    :   SfxPoolItem ( rItem )
{
    if ( rItem.pUserList )
        pUserList.reset( new ScUserList( *(rItem.pUserList) ) );
}

ScUserListItem::~ScUserListItem()
{
}

bool ScUserListItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScUserListItem& r = static_cast<const ScUserListItem&>(rItem);
    bool bEqual = false;

    if ( !pUserList || !r.pUserList )
        bEqual = ( !pUserList && !r.pUserList );
    else
        bEqual = ( *pUserList == *(r.pUserList) );

    return bEqual;
}

ScUserListItem* ScUserListItem::Clone( SfxItemPool * ) const
{
    return new ScUserListItem( *this );
}

void ScUserListItem::SetUserList( const ScUserList& rUserList )
{
    pUserList.reset( new ScUserList( rUserList ) );
}

/**
 * Data for the Consolidate dialog
 */
ScConsolidateItem::ScConsolidateItem(
                            sal_uInt16                      nWhichP,
                            const ScConsolidateParam*   pConsolidateData ) :
        SfxPoolItem ( nWhichP )
{
    if ( pConsolidateData ) theConsData = *pConsolidateData;
}

bool ScConsolidateItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScConsolidateItem& rCItem = static_cast<const ScConsolidateItem&>(rItem);

    return ( theConsData == rCItem.theConsData);
}

ScConsolidateItem* ScConsolidateItem::Clone( SfxItemPool * ) const
{
    return new ScConsolidateItem( *this );
}

/**
 * Data for the Pivot dialog
 */
ScPivotItem::ScPivotItem( sal_uInt16 nWhichP, const ScDPSaveData* pData,
                             const ScRange* pRange, bool bNew ) :
        SfxPoolItem ( nWhichP )
{
    // pSaveData must always exist
    if ( pData )
        pSaveData.reset( new ScDPSaveData(*pData) );
    else
        pSaveData.reset( new ScDPSaveData );
    if ( pRange ) aDestRange = *pRange;
    bNewSheet = bNew;
}

ScPivotItem::ScPivotItem( const ScPivotItem& rItem ) :
        SfxPoolItem ( rItem ),
        aDestRange  ( rItem.aDestRange ),
        bNewSheet   ( rItem.bNewSheet )
{
    assert(rItem.pSaveData && "pSaveData");
    pSaveData.reset( new ScDPSaveData(*rItem.pSaveData) );
}

ScPivotItem::~ScPivotItem()
{
}

bool ScPivotItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScPivotItem& rPItem = static_cast<const ScPivotItem&>(rItem);
    OSL_ENSURE( pSaveData && rPItem.pSaveData, "pSaveData" );
    return ( *pSaveData == *rPItem.pSaveData &&
             aDestRange == rPItem.aDestRange &&
             bNewSheet  == rPItem.bNewSheet );
}

ScPivotItem* ScPivotItem::Clone( SfxItemPool * ) const
{
    return new ScPivotItem( *this );
}

/**
 * Data for the Solver dialog
 */
ScSolveItem::ScSolveItem( sal_uInt16                nWhichP,
                          const ScSolveParam*   pSolveData )
    :   SfxPoolItem ( nWhichP )
{
    if ( pSolveData ) theSolveData = *pSolveData;
}

bool ScSolveItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScSolveItem& rPItem = static_cast<const ScSolveItem&>(rItem);

    return ( theSolveData == rPItem.theSolveData );
}

ScSolveItem* ScSolveItem::Clone( SfxItemPool * ) const
{
    return new ScSolveItem( *this );
}

/**
 * Data for the TabOp dialog
 */
ScTabOpItem::ScTabOpItem( sal_uInt16                nWhichP,
                          const ScTabOpParam*   pTabOpData )
    :   SfxPoolItem ( nWhichP )
{
    if ( pTabOpData ) theTabOpData = *pTabOpData;
}

bool ScTabOpItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScTabOpItem& rPItem = static_cast<const ScTabOpItem&>(rItem);

    return ( theTabOpData == rPItem.theTabOpData );
}

ScTabOpItem* ScTabOpItem::Clone( SfxItemPool * ) const
{
    return new ScTabOpItem( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
