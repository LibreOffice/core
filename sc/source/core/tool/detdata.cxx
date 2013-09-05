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

#include "detdata.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"

ScDetOpList::ScDetOpList(const ScDetOpList& rList) :
    bHasAddError( false )
{
    size_t nCount = rList.Count();

    for (size_t i=0; i<nCount; i++)
        Append( new ScDetOpData(rList.aDetOpDataVector[i]) );
}

void ScDetOpList::DeleteOnTab( SCTAB nTab )
{
    for (ScDetOpDataVector::iterator it = aDetOpDataVector.begin(); it != aDetOpDataVector.end(); /*noop*/ )
    {
        // look for operations on the deleted sheet
        if (it->GetPos().Tab() == nTab)
            it = aDetOpDataVector.erase( it);
        else
            ++it;
    }
}

void ScDetOpList::UpdateReference( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    for (ScDetOpDataVector::iterator it = aDetOpDataVector.begin(); it != aDetOpDataVector.end(); ++it )
    {
        ScAddress aPos = it->GetPos();
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
            it->SetPos( ScAddress( nCol1, nRow1, nTab1 ) );
    }
}

void ScDetOpList::Append( ScDetOpData* pDetOpData )
{
    if ( pDetOpData->GetOperation() == SCDETOP_ADDERROR )
        bHasAddError = sal_True;

    aDetOpDataVector.push_back( pDetOpData );
}

sal_Bool ScDetOpList::operator==( const ScDetOpList& r ) const
{
    // for Ref-Undo

    size_t nCount = Count();
    sal_Bool bEqual = ( nCount == r.Count() );
    for (size_t i=0; i<nCount && bEqual; i++)       // order has to be the same
        if ( !(aDetOpDataVector[i] == r.aDetOpDataVector[i]) )    // entries are different ?
            bEqual = false;

    return bEqual;
}

const ScDetOpData* ScDetOpList::GetObject( size_t nPos ) const
{
    return &aDetOpDataVector[nPos];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
