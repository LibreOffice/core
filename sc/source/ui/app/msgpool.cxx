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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "scitems.hxx"
#include <svx/dialogs.hrc>

#include "sc.hrc"
#include "docpool.hxx"
#include "msgpool.hxx"

//------------------------------------------------------------------------

static SfxItemInfo const aMsgItemInfos[] =
{
    { 0,                         SFX_ITEM_POOLABLE },   // SCITEM_STRING
    { 0,                         SFX_ITEM_POOLABLE },   // SCITEM_SEARCHDATA - nicht mehr benutzt !!!
    { SID_SORT,                  SFX_ITEM_POOLABLE },   // SCITEM_SORTDATA
    { SID_QUERY,                 SFX_ITEM_POOLABLE },   // SCITEM_QUERYDATA
    { SID_SUBTOTALS,             SFX_ITEM_POOLABLE },   // SCITEM_SUBTDATA
    { SID_CONSOLIDATE,           SFX_ITEM_POOLABLE },   // SCITEM_CONSOLIDATEDATA
    { SID_PIVOT_TABLE,           SFX_ITEM_POOLABLE },   // SCITEM_PIVOTDATA
    { SID_SOLVE,                 SFX_ITEM_POOLABLE },   // SCITEM_SOLVEDATA
    { SID_SCUSERLISTS,           SFX_ITEM_POOLABLE },   // SCITEM_USERLIST
    { SID_PRINTER_NOTFOUND_WARN, SFX_ITEM_POOLABLE }    // SCITEM_PRINTWARN
};

//------------------------------------------------------------------------

ScMessagePool::ScMessagePool()
    :   SfxItemPool         ( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScMessagePool")),
                              MSGPOOL_START, MSGPOOL_END,
                              aMsgItemInfos, NULL ),
    //
    aGlobalStringItem       ( SfxStringItem         ( SCITEM_STRING, String() ) ),
    aGlobalSearchItem       ( SvxSearchItem         ( SCITEM_SEARCHDATA ) ),
    aGlobalSortItem         ( ScSortItem            ( SCITEM_SORTDATA, NULL ) ),
    aGlobalQueryItem        ( ScQueryItem           ( SCITEM_QUERYDATA, NULL, NULL ) ),
    aGlobalSubTotalItem     ( ScSubTotalItem        ( SCITEM_SUBTDATA, NULL, NULL ) ),
    aGlobalConsolidateItem  ( ScConsolidateItem     ( SCITEM_CONSOLIDATEDATA, NULL ) ),
    aGlobalPivotItem        ( ScPivotItem           ( SCITEM_PIVOTDATA, NULL, NULL, false ) ),
    aGlobalSolveItem        ( ScSolveItem           ( SCITEM_SOLVEDATA, NULL ) ),
    aGlobalUserListItem     ( ScUserListItem        ( SCITEM_USERLIST ) ),
    //
    aPrintWarnItem          ( SfxBoolItem           ( SCITEM_PRINTWARN, false ) )
{
    ppPoolDefaults = new SfxPoolItem*[MSGPOOL_END - MSGPOOL_START + 1];

    ppPoolDefaults[SCITEM_STRING            - MSGPOOL_START] = &aGlobalStringItem;
    ppPoolDefaults[SCITEM_SEARCHDATA        - MSGPOOL_START] = &aGlobalSearchItem;
    ppPoolDefaults[SCITEM_SORTDATA          - MSGPOOL_START] = &aGlobalSortItem;
    ppPoolDefaults[SCITEM_QUERYDATA         - MSGPOOL_START] = &aGlobalQueryItem;
    ppPoolDefaults[SCITEM_SUBTDATA          - MSGPOOL_START] = &aGlobalSubTotalItem;
    ppPoolDefaults[SCITEM_CONSOLIDATEDATA   - MSGPOOL_START] = &aGlobalConsolidateItem;
    ppPoolDefaults[SCITEM_PIVOTDATA         - MSGPOOL_START] = &aGlobalPivotItem;
    ppPoolDefaults[SCITEM_SOLVEDATA         - MSGPOOL_START] = &aGlobalSolveItem;
    ppPoolDefaults[SCITEM_USERLIST          - MSGPOOL_START] = &aGlobalUserListItem;
    ppPoolDefaults[SCITEM_PRINTWARN         - MSGPOOL_START] = &aPrintWarnItem;

    SetDefaults( ppPoolDefaults );

    pDocPool = new ScDocumentPool;

    SetSecondaryPool( pDocPool );
}


ScMessagePool::~ScMessagePool()
{
    Delete();
    SetSecondaryPool( NULL );       // before deleting defaults (accesses defaults)

    for ( sal_uInt16 i=0; i <= MSGPOOL_END-MSGPOOL_START; i++ )
        SetRefCount( *ppPoolDefaults[i], 0 );

    delete[] ppPoolDefaults;

    SfxItemPool::Free(pDocPool);
}


SfxMapUnit ScMessagePool::GetMetric( sal_uInt16 nWhich ) const
{
    //  eigene Attribute: Twips, alles andere 1/100 mm

    if ( nWhich >= ATTR_STARTINDEX && nWhich <= ATTR_ENDINDEX )
        return SFX_MAPUNIT_TWIP;
    else
        return SFX_MAPUNIT_100TH_MM;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
