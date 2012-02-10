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




// INCLUDE ---------------------------------------------------------------

#include "detdata.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"

//------------------------------------------------------------------------

ScDetOpList::ScDetOpList(const ScDetOpList& rList) :
    bHasAddError( false )
{
    sal_uInt16 nCount = rList.Count();

    for (sal_uInt16 i=0; i<nCount; i++)
        Append( new ScDetOpData(rList.aDetOpDataVector[i]) );
}

void ScDetOpList::DeleteOnTab( SCTAB nTab )
{
    sal_uInt16 nPos = 0;
    while ( nPos < Count() )
    {
        // look for operations on the deleted sheet

        if ( GetObject(nPos)->GetPos().Tab() == nTab )
            DeleteAndDestroy(nPos);
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
        ScAddress aPos = GetObject(i)->GetPos();
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
            GetObject(i)->SetPos( ScAddress( nCol1, nRow1, nTab1 ) );
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
    // fuer Ref-Undo

    sal_uInt16 nCount = Count();
    sal_Bool bEqual = ( nCount == r.Count() );
    for (sal_uInt16 i=0; i<nCount && bEqual; i++)       // Reihenfolge muss auch gleich sein
        if ( !(aDetOpDataVector[i] == r.aDetOpDataVector[i]) )    // Eintraege unterschiedlich ?
            bEqual = false;

    return bEqual;
}

ScDetOpData* ScDetOpList::GetObject(int i)
{
	return &aDetOpDataVector[i];
}

void ScDetOpList::DeleteAndDestroy(int i)
{
	const ScDetOpData* p = &aDetOpDataVector[i];
	if (p != NULL)
	{
		delete p;
		aDetOpDataVector.erase(aDetOpDataVector.begin() + i);
	}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
