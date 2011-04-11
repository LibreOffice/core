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



// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>

#include "detdata.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"

//------------------------------------------------------------------------

SV_IMPL_PTRARR( ScDetOpArr_Impl, ScDetOpDataPtr );

//------------------------------------------------------------------------

ScDetOpList::ScDetOpList(const ScDetOpList& rList) :
    ScDetOpArr_Impl(),
    bHasAddError( false )
{
    sal_uInt16 nCount = rList.Count();

    for (sal_uInt16 i=0; i<nCount; i++)
        Append( new ScDetOpData(*rList[i]) );
}

void ScDetOpList::DeleteOnTab( SCTAB nTab )
{
    sal_uInt16 nPos = 0;
    while ( nPos < Count() )
    {
        // look for operations on the deleted sheet

        if ( (*this)[nPos]->GetPos().Tab() == nTab )
            Remove(nPos);
        else
            ++nPos;
    }
}

void ScDetOpList::UpdateReference( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    sal_uInt16 nCount = Count();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ScAddress aPos = (*this)[i]->GetPos();
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
            (*this)[i]->SetPos( ScAddress( nCol1, nRow1, nTab1 ) );
    }
}

void ScDetOpList::Append( ScDetOpData* pDetOpData )
{
    if ( pDetOpData->GetOperation() == SCDETOP_ADDERROR )
        bHasAddError = sal_True;

    Insert( pDetOpData, Count() );
}


sal_Bool ScDetOpList::operator==( const ScDetOpList& r ) const
{
    // fuer Ref-Undo

    sal_uInt16 nCount = Count();
    sal_Bool bEqual = ( nCount == r.Count() );
    for (sal_uInt16 i=0; i<nCount && bEqual; i++)       // Reihenfolge muss auch gleich sein
        if ( !(*(*this)[i] == *r[i]) )              // Eintraege unterschiedlich ?
            bEqual = false;

    return bEqual;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
