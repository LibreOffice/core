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

#include "scitems.hxx"
#include <svx/dialogs.hrc>

#include "sc.hrc"
#include "docpool.hxx"
#include "msgpool.hxx"



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



ScMessagePool::ScMessagePool()
    :   SfxItemPool         ( OUString("ScMessagePool"),
                              MSGPOOL_START, MSGPOOL_END,
                              aMsgItemInfos, NULL ),
    //
    aGlobalStringItem       ( SfxStringItem         ( SCITEM_STRING, OUString() ) ),
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
