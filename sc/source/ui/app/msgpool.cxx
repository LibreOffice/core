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

#include <scitems.hxx>
#include <svx/dialogs.hrc>

#include <sc.hrc>
#include <docpool.hxx>
#include <msgpool.hxx>

static SfxItemInfo const aMsgItemInfos[] =
{
    { 0,                         true },   // SCITEM_STRING
    { 0,                         true },   // SCITEM_SEARCHDATA - stop using this!
    { SID_SORT,                  true },   // SCITEM_SORTDATA
    { SID_QUERY,                 true },   // SCITEM_QUERYDATA
    { SID_SUBTOTALS,             true },   // SCITEM_SUBTDATA
    { SID_CONSOLIDATE,           true },   // SCITEM_CONSOLIDATEDATA
    { SID_PIVOT_TABLE,           true },   // SCITEM_PIVOTDATA
    { SID_SOLVE,                 true },   // SCITEM_SOLVEDATA
    { SID_SCUSERLISTS,           true },   // SCITEM_USERLIST
    { SID_PRINTER_NOTFOUND_WARN, true },   // SCITEM_PRINTWARN
    { 0,                         false }  // SCITEM_CONDFORMATDLGDATA
};

ScMessagePool::ScMessagePool()
    :   SfxItemPool         ( "ScMessagePool",
                              MSGPOOL_START, MSGPOOL_END,
                              aMsgItemInfos, nullptr ),

    aGlobalStringItem       ( SfxStringItem         ( SCITEM_STRING, OUString() ) ),
    aGlobalSearchItem       ( SvxSearchItem         ( SCITEM_SEARCHDATA ) ),
    aGlobalSortItem         ( ScSortItem            ( SCITEM_SORTDATA, nullptr ) ),
    aGlobalQueryItem        ( ScQueryItem           ( SCITEM_QUERYDATA, nullptr, nullptr ) ),
    aGlobalSubTotalItem     ( ScSubTotalItem        ( SCITEM_SUBTDATA, nullptr, nullptr ) ),
    aGlobalConsolidateItem  ( ScConsolidateItem     ( SCITEM_CONSOLIDATEDATA, nullptr ) ),
    aGlobalPivotItem        ( ScPivotItem           ( SCITEM_PIVOTDATA, nullptr, nullptr, false ) ),
    aGlobalSolveItem        ( ScSolveItem           ( SCITEM_SOLVEDATA, nullptr ) ),
    aGlobalUserListItem     ( ScUserListItem        ( SCITEM_USERLIST ) ),

    aPrintWarnItem          ( SfxBoolItem           ( SCITEM_PRINTWARN, false ) ),
    aCondFormatDlgItem      ( ScCondFormatDlgItem   ( nullptr, -1, false ) ),

    mpPoolDefaults(new std::vector<SfxPoolItem*>(MSGPOOL_END - MSGPOOL_START + 1)),
    pDocPool(new ScDocumentPool)
{
    std::vector<SfxPoolItem*>& rPoolDefaults = *mpPoolDefaults;
    rPoolDefaults[SCITEM_STRING            - MSGPOOL_START] = &aGlobalStringItem;
    rPoolDefaults[SCITEM_SEARCHDATA        - MSGPOOL_START] = &aGlobalSearchItem;
    rPoolDefaults[SCITEM_SORTDATA          - MSGPOOL_START] = &aGlobalSortItem;
    rPoolDefaults[SCITEM_QUERYDATA         - MSGPOOL_START] = &aGlobalQueryItem;
    rPoolDefaults[SCITEM_SUBTDATA          - MSGPOOL_START] = &aGlobalSubTotalItem;
    rPoolDefaults[SCITEM_CONSOLIDATEDATA   - MSGPOOL_START] = &aGlobalConsolidateItem;
    rPoolDefaults[SCITEM_PIVOTDATA         - MSGPOOL_START] = &aGlobalPivotItem;
    rPoolDefaults[SCITEM_SOLVEDATA         - MSGPOOL_START] = &aGlobalSolveItem;
    rPoolDefaults[SCITEM_USERLIST          - MSGPOOL_START] = &aGlobalUserListItem;
    rPoolDefaults[SCITEM_PRINTWARN         - MSGPOOL_START] = &aPrintWarnItem;
    rPoolDefaults[SCITEM_CONDFORMATDLGDATA - MSGPOOL_START] = &aCondFormatDlgItem;

    SetDefaults( mpPoolDefaults );

    SetSecondaryPool( pDocPool );
}

ScMessagePool::~ScMessagePool()
{
    Delete();
    SetSecondaryPool( nullptr ); // before deleting defaults (accesses defaults)

    for ( sal_uInt16 i=0; i <= MSGPOOL_END-MSGPOOL_START; i++ )
        ClearRefCount( *(*mpPoolDefaults)[i] );

    delete mpPoolDefaults;

    SfxItemPool::Free(pDocPool);
}

MapUnit ScMessagePool::GetMetric( sal_uInt16 nWhich ) const
{
    // Own attributes: Twips, everything else 1/100 mm
    if ( nWhich >= ATTR_STARTINDEX && nWhich <= ATTR_ENDINDEX )
        return MapUnit::MapTwip;
    else
        return MapUnit::Map100thMM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
