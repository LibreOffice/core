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

#include "uiitems.hxx"

#include "userlist.hxx"
#include "dpsave.hxx"
#include "queryparam.hxx"

#include <editeng/editobj.hxx>

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScInputStatusItem,    SfxPoolItem);
TYPEINIT1(ScSortItem,           SfxPoolItem);
TYPEINIT1(ScQueryItem,          SfxPoolItem);
TYPEINIT1(ScSubTotalItem,       SfxPoolItem);
TYPEINIT1(ScUserListItem,       SfxPoolItem);
TYPEINIT1(ScConsolidateItem,    SfxPoolItem);
TYPEINIT1(ScPivotItem,          SfxPoolItem);
TYPEINIT1(ScSolveItem,          SfxPoolItem);
TYPEINIT1(ScTabOpItem,          SfxPoolItem);

TYPEINIT1(ScTablesHint,         SfxHint);
TYPEINIT1(ScEditViewHint,       SfxHint);
TYPEINIT1(ScIndexHint,          SfxHint);

// -----------------------------------------------------------------------
//      ScInputStatusItem - Status-Update fuer Eingabezeile
// -----------------------------------------------------------------------

ScInputStatusItem::ScInputStatusItem(
    sal_uInt16 nWhichP, const ScAddress& rCurPos, const ScAddress& rStartPos,
    const ScAddress& rEndPos, const OUString& rString, const EditTextObject* pData ) :
    SfxPoolItem ( nWhichP ),
    aCursorPos  ( rCurPos ),
    aStartPos   ( rStartPos ),
    aEndPos     ( rEndPos ),
    aString     ( rString ),
    pEditData   ( pData ? pData->Clone() : NULL ),
    mpMisspellRanges(NULL)
{
}

ScInputStatusItem::ScInputStatusItem( const ScInputStatusItem& rItem ) :
    SfxPoolItem ( rItem ),
    aCursorPos  ( rItem.aCursorPos ),
    aStartPos   ( rItem.aStartPos ),
    aEndPos     ( rItem.aEndPos ),
    aString     ( rItem.aString ),
    pEditData   ( rItem.pEditData ? rItem.pEditData->Clone() : NULL ),
    mpMisspellRanges(rItem.mpMisspellRanges)
{
}

ScInputStatusItem::~ScInputStatusItem()
{
    delete pEditData;
}

OUString ScInputStatusItem::GetValueText() const
{
    return OUString("InputStatus");
}

int ScInputStatusItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    return (    (aStartPos  == ((ScInputStatusItem&)rItem).aStartPos)
             && (aEndPos    == ((ScInputStatusItem&)rItem).aEndPos)
             && (aCursorPos == ((ScInputStatusItem&)rItem).aCursorPos)
             && (aString    == ((ScInputStatusItem&)rItem).aString) );
                                                        //! Edit-Daten vergleichen!
}

SfxPoolItem* ScInputStatusItem::Clone( SfxItemPool * ) const
{
    return new ScInputStatusItem( *this );
}

void ScInputStatusItem::SetMisspellRanges( const std::vector<editeng::MisspellRanges>* pRanges )
{
    mpMisspellRanges = pRanges;
}

const std::vector<editeng::MisspellRanges>* ScInputStatusItem::GetMisspellRanges() const
{
    return mpMisspellRanges;
}

//
//  ScPaintHint ist nach schints.cxx verschoben
//

// -----------------------------------------------------------------------
//  ScTablesHint - Views anpassen, wenn Tabellen eingefuegt / geloescht
// -----------------------------------------------------------------------

ScTablesHint::ScTablesHint(sal_uInt16 nNewId, SCTAB nTable1, SCTAB nTable2) :
    nId( nNewId ),
    nTab1( nTable1 ),
    nTab2( nTable2 )
{
}

ScTablesHint::~ScTablesHint()
{
}


// -----------------------------------------------------------------------
//  ScIndexHint
// -----------------------------------------------------------------------

ScIndexHint::ScIndexHint(sal_uInt16 nNewId, sal_uInt16 nIdx) :
    nId( nNewId ),
    nIndex( nIdx )
{
}

ScIndexHint::~ScIndexHint()
{
}


// -----------------------------------------------------------------------
//      ScEditViewHint - neue EditView fuer Cursorposition anlegen
// -----------------------------------------------------------------------

ScEditViewHint::ScEditViewHint( ScEditEngineDefaulter* pEngine, const ScAddress& rCurPos ) :
    pEditEngine( pEngine ),
    aCursorPos( rCurPos )
{
}

ScEditViewHint::~ScEditViewHint()
{
}

// -----------------------------------------------------------------------
//      ScSortItem - Daten fuer den Sortierdialog
// -----------------------------------------------------------------------

ScSortItem::ScSortItem( sal_uInt16              nWhichP,
                        ScViewData*         ptrViewData,
                        const ScSortParam*  pSortData ) :
        SfxPoolItem ( nWhichP ),
        pViewData   ( ptrViewData )
{
    if ( pSortData ) theSortData = *pSortData;
}

//------------------------------------------------------------------------

ScSortItem::ScSortItem( sal_uInt16              nWhichP,
                        const ScSortParam*  pSortData ) :
        SfxPoolItem ( nWhichP ),
        pViewData   ( NULL )
{
    if ( pSortData ) theSortData = *pSortData;
}

//------------------------------------------------------------------------

ScSortItem::ScSortItem( const ScSortItem& rItem ) :
        SfxPoolItem ( rItem ),
        pViewData   ( rItem.pViewData ),
        theSortData ( rItem.theSortData )
{
}

ScSortItem::~ScSortItem()
{
}

//------------------------------------------------------------------------

OUString ScSortItem::GetValueText() const
{
    return OUString("SortItem");
}

//------------------------------------------------------------------------

int ScSortItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScSortItem& rOther = (const ScSortItem&)rItem;

    return (   (pViewData   == rOther.pViewData)
            && (theSortData == rOther.theSortData) );
}

//------------------------------------------------------------------------

SfxPoolItem* ScSortItem::Clone( SfxItemPool * ) const
{
    return new ScSortItem( *this );
}

//------------------------------------------------------------------------

bool ScSortItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /* nMemberUd */ ) const
{
    // Return empty value as there is no useful conversion
    rVal = com::sun::star::uno::Any();
    return true;
}

// -----------------------------------------------------------------------
//      ScQueryItem - Daten fuer den Filterdialog
// -----------------------------------------------------------------------

ScQueryItem::ScQueryItem( sal_uInt16                nWhichP,
                          ScViewData*           ptrViewData,
                          const ScQueryParam*   pQueryData ) :
        SfxPoolItem ( nWhichP ),
        mpQueryData(NULL),
        pViewData   ( ptrViewData ),
        bIsAdvanced ( false )
{
    if (pQueryData)
        mpQueryData.reset(new ScQueryParam(*pQueryData));
    else
        mpQueryData.reset(new ScQueryParam);
}

//------------------------------------------------------------------------

ScQueryItem::ScQueryItem( sal_uInt16                nWhichP,
                          const ScQueryParam*   pQueryData ) :
        SfxPoolItem ( nWhichP ),
        mpQueryData(NULL),
        pViewData   ( NULL ),
        bIsAdvanced ( false )
{
    if (pQueryData)
        mpQueryData.reset(new ScQueryParam(*pQueryData));
    else
        mpQueryData.reset(new ScQueryParam);
}

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

OUString ScQueryItem::GetValueText() const
{
    return OUString("QueryItem");
}

//------------------------------------------------------------------------

int ScQueryItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScQueryItem& rQueryItem = (const ScQueryItem&)rItem;

    return (   (pViewData    == rQueryItem.pViewData)
            && (bIsAdvanced  == rQueryItem.bIsAdvanced)
            && (aAdvSource   == rQueryItem.aAdvSource)
            && (*mpQueryData == *rQueryItem.mpQueryData) );
}

//------------------------------------------------------------------------

SfxPoolItem* ScQueryItem::Clone( SfxItemPool * ) const
{
    return new ScQueryItem( *this );
}

// -----------------------------------------------------------------------
//      ScSubTotalItem - Daten fuer den Zwischenergebnisdialog
// -----------------------------------------------------------------------

ScSubTotalItem::ScSubTotalItem( sal_uInt16                  nWhichP,
                                ScViewData*             ptrViewData,
                                const ScSubTotalParam*  pSubTotalData ) :
        SfxPoolItem ( nWhichP ),
        pViewData   ( ptrViewData )
{
    if ( pSubTotalData ) theSubTotalData = *pSubTotalData;
}

//------------------------------------------------------------------------

ScSubTotalItem::ScSubTotalItem( sal_uInt16                  nWhichP,
                                const ScSubTotalParam*  pSubTotalData ) :
        SfxPoolItem ( nWhichP ),
        pViewData   ( NULL )
{
    if ( pSubTotalData ) theSubTotalData = *pSubTotalData;
}

//------------------------------------------------------------------------

ScSubTotalItem::ScSubTotalItem( const ScSubTotalItem& rItem ) :
        SfxPoolItem     ( rItem ),
        pViewData       ( rItem.pViewData ),
        theSubTotalData ( rItem.theSubTotalData )
{
}

ScSubTotalItem::~ScSubTotalItem()
{
}

//------------------------------------------------------------------------

OUString ScSubTotalItem::GetValueText() const
{
    return OUString("SubTotalItem");
}

//------------------------------------------------------------------------

int ScSubTotalItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScSubTotalItem& rSTItem = (const ScSubTotalItem&)rItem;

    return (   (pViewData       == rSTItem.pViewData)
            && (theSubTotalData == rSTItem.theSubTotalData) );
}

//------------------------------------------------------------------------

SfxPoolItem* ScSubTotalItem::Clone( SfxItemPool * ) const
{
    return new ScSubTotalItem( *this );
}

//------------------------------------------------------------------------

bool ScSubTotalItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /* nMemberUd */ ) const
{
    // Return empty value as there is no useful conversion
    rVal = com::sun::star::uno::Any();
    return true;
}

// -----------------------------------------------------------------------
//      ScUserListItem - Transporter fuer den Benutzerlisten-TabPage
// -----------------------------------------------------------------------

ScUserListItem::ScUserListItem( sal_uInt16 nWhichP )
    :   SfxPoolItem ( nWhichP ),
        pUserList   ( NULL )
{
}

//------------------------------------------------------------------------

ScUserListItem::ScUserListItem( const ScUserListItem& rItem )
    :   SfxPoolItem ( rItem )
{
    if ( rItem.pUserList )
        pUserList = new ScUserList( *(rItem.pUserList) );
    else
        pUserList = NULL;
}

ScUserListItem::~ScUserListItem()
{
    delete pUserList;
}

//------------------------------------------------------------------------

OUString ScUserListItem::GetValueText() const
{
    return OUString("ScUserListItem");
}

//------------------------------------------------------------------------

int ScUserListItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScUserListItem& r = (const ScUserListItem&)rItem;
    sal_Bool bEqual = false;

    if ( !pUserList || !(r.pUserList) )
        bEqual = ( !pUserList && !(r.pUserList) );
    else
        bEqual = ( *pUserList == *(r.pUserList) );

    return bEqual;
}


//------------------------------------------------------------------------

SfxPoolItem* ScUserListItem::Clone( SfxItemPool * ) const
{
    return new ScUserListItem( *this );
}

//------------------------------------------------------------------------

void ScUserListItem::SetUserList( const ScUserList& rUserList )
{
    delete pUserList;
    pUserList = new ScUserList( rUserList );
}

// -----------------------------------------------------------------------
//      ScConsolidateItem - Daten fuer den Konsolidieren-Dialog
// -----------------------------------------------------------------------

ScConsolidateItem::ScConsolidateItem(
                            sal_uInt16                      nWhichP,
                            const ScConsolidateParam*   pConsolidateData ) :
        SfxPoolItem ( nWhichP )
{
    if ( pConsolidateData ) theConsData = *pConsolidateData;
}

//------------------------------------------------------------------------

ScConsolidateItem::ScConsolidateItem( const ScConsolidateItem& rItem ) :
        SfxPoolItem ( rItem ),
        theConsData ( rItem.theConsData )
{
}

//------------------------------------------------------------------------

ScConsolidateItem::~ScConsolidateItem()
{
}

//------------------------------------------------------------------------

OUString ScConsolidateItem::GetValueText() const
{
    return OUString("ScConsolidateItem");
}

//------------------------------------------------------------------------

int ScConsolidateItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScConsolidateItem& rCItem = (const ScConsolidateItem&)rItem;

    return ( theConsData == rCItem.theConsData);
}

//------------------------------------------------------------------------

SfxPoolItem* ScConsolidateItem::Clone( SfxItemPool * ) const
{
    return new ScConsolidateItem( *this );
}


// -----------------------------------------------------------------------
//      ScPivotItem - Daten fuer den Pivot-Dialog
// -----------------------------------------------------------------------

ScPivotItem::ScPivotItem( sal_uInt16 nWhichP, const ScDPSaveData* pData,
                             const ScRange* pRange, sal_Bool bNew ) :
        SfxPoolItem ( nWhichP )
{
    //  pSaveData must always exist
    if ( pData )
        pSaveData = new ScDPSaveData(*pData);
    else
        pSaveData = new ScDPSaveData;
    if ( pRange ) aDestRange = *pRange;
    bNewSheet = bNew;
}

//------------------------------------------------------------------------

ScPivotItem::ScPivotItem( const ScPivotItem& rItem ) :
        SfxPoolItem ( rItem ),
        aDestRange  ( rItem.aDestRange ),
        bNewSheet   ( rItem.bNewSheet )
{
    OSL_ENSURE(rItem.pSaveData, "pSaveData");
    pSaveData = new ScDPSaveData(*rItem.pSaveData);
}

//------------------------------------------------------------------------

ScPivotItem::~ScPivotItem()
{
    delete pSaveData;
}

//------------------------------------------------------------------------

OUString ScPivotItem::GetValueText() const
{
    return OUString("ScPivotItem");
}

//------------------------------------------------------------------------

int ScPivotItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScPivotItem& rPItem = (const ScPivotItem&)rItem;
    OSL_ENSURE( pSaveData && rPItem.pSaveData, "pSaveData" );
    return ( *pSaveData == *rPItem.pSaveData &&
             aDestRange == rPItem.aDestRange &&
             bNewSheet  == rPItem.bNewSheet );
}

//------------------------------------------------------------------------

SfxPoolItem* ScPivotItem::Clone( SfxItemPool * ) const
{
    return new ScPivotItem( *this );
}


// -----------------------------------------------------------------------
//      ScSolveItem - Daten fuer den Solver-Dialog
// -----------------------------------------------------------------------

ScSolveItem::ScSolveItem( sal_uInt16                nWhichP,
                          const ScSolveParam*   pSolveData )
    :   SfxPoolItem ( nWhichP )
{
    if ( pSolveData ) theSolveData = *pSolveData;
}

//------------------------------------------------------------------------

ScSolveItem::ScSolveItem( const ScSolveItem& rItem )
    :   SfxPoolItem     ( rItem ),
        theSolveData    ( rItem.theSolveData )
{
}

//------------------------------------------------------------------------

ScSolveItem::~ScSolveItem()
{
}

//------------------------------------------------------------------------

OUString ScSolveItem::GetValueText() const
{
    return OUString("ScSolveItem");
}

//------------------------------------------------------------------------

int ScSolveItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScSolveItem& rPItem = (const ScSolveItem&)rItem;

    return ( theSolveData == rPItem.theSolveData );
}

//------------------------------------------------------------------------

SfxPoolItem* ScSolveItem::Clone( SfxItemPool * ) const
{
    return new ScSolveItem( *this );
}

// -----------------------------------------------------------------------
//      ScTabOpItem - Daten fuer den TabOp-Dialog
// -----------------------------------------------------------------------

ScTabOpItem::ScTabOpItem( sal_uInt16                nWhichP,
                          const ScTabOpParam*   pTabOpData )
    :   SfxPoolItem ( nWhichP )
{
    if ( pTabOpData ) theTabOpData = *pTabOpData;
}

//------------------------------------------------------------------------

ScTabOpItem::ScTabOpItem( const ScTabOpItem& rItem )
    :   SfxPoolItem     ( rItem ),
        theTabOpData    ( rItem.theTabOpData )
{
}

//------------------------------------------------------------------------

ScTabOpItem::~ScTabOpItem()
{
}

//------------------------------------------------------------------------

OUString ScTabOpItem::GetValueText() const
{
    return OUString("ScTabOpItem");
}

//------------------------------------------------------------------------

int ScTabOpItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScTabOpItem& rPItem = (const ScTabOpItem&)rItem;

    return ( theTabOpData == rPItem.theTabOpData );
}

//------------------------------------------------------------------------

SfxPoolItem* ScTabOpItem::Clone( SfxItemPool * ) const
{
    return new ScTabOpItem( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
