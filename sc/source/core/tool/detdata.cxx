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

#include "detdata.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"

//------------------------------------------------------------------------

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
