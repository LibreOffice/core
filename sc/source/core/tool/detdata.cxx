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

#include <algorithm>
#include <memory>
#include <detdata.hxx>
#include <refupdat.hxx>
#include <rechead.hxx>

ScDetOpList::ScDetOpList(const ScDetOpList& rList) :
    bHasAddError( false )
{
    size_t nCount = rList.Count();

    for (size_t i=0; i<nCount; i++)
        Append( new ScDetOpData( (*rList.aDetOpDataVector[i].get()) ) );
}

void ScDetOpList::DeleteOnTab( SCTAB nTab )
{
    aDetOpDataVector.erase(std::remove_if(aDetOpDataVector.begin(), aDetOpDataVector.end(),
        [&nTab](const std::unique_ptr<ScDetOpData>& rxDetOpData) {
            return rxDetOpData->GetPos().Tab() == nTab; // look for operations on the deleted sheet
        }),
        aDetOpDataVector.end());
}

void ScDetOpList::UpdateReference( const ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCCOL nDx, SCROW nDy, SCTAB nDz )
{
    for (auto& rxDetOpData : aDetOpDataVector )
    {
        ScAddress aPos = rxDetOpData->GetPos();
        SCCOL nCol1 = aPos.Col();
        SCROW nRow1 = aPos.Row();
        SCTAB nTab1 = aPos.Tab();
        SCCOL nCol2 = nCol1;
        SCROW nRow2 = nRow1;
        SCTAB nTab2 = nTab1;

        ScRefUpdateRes eRes =
            ScRefUpdate::Update( pDoc, eUpdateRefMode,
                rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
                rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
                nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        if ( eRes != UR_NOTHING )
            rxDetOpData->SetPos( ScAddress( nCol1, nRow1, nTab1 ) );
    }
}

void ScDetOpList::Append( ScDetOpData* pDetOpData )
{
    if ( pDetOpData->GetOperation() == SCDETOP_ADDERROR )
        bHasAddError = true;

    aDetOpDataVector.push_back( std::unique_ptr<ScDetOpData>(pDetOpData) );
}

bool ScDetOpList::operator==( const ScDetOpList& r ) const
{
    // for Ref-Undo

    size_t nCount = Count();
    bool bEqual = ( nCount == r.Count() );
    for (size_t i=0; i<nCount && bEqual; i++)       // order has to be the same
        if ( !(*aDetOpDataVector[i] == *r.aDetOpDataVector[i]) )    // entries are different ?
            bEqual = false;

    return bEqual;
}

const ScDetOpData& ScDetOpList::GetObject( size_t nPos ) const
{
    return (*aDetOpDataVector[nPos].get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
