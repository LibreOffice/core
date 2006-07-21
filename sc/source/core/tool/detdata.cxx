/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: detdata.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:27:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    bHasAddError( FALSE )
{
    USHORT nCount = rList.Count();

    for (USHORT i=0; i<nCount; i++)
        Append( new ScDetOpData(*rList[i]) );
}

void ScDetOpList::DeleteOnTab( SCTAB nTab )
{
    USHORT nPos = 0;
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
    USHORT nCount = Count();
    for (USHORT i=0; i<nCount; i++)
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

void ScDetOpList::Append( ScDetOpData* pData )
{
    if ( pData->GetOperation() == SCDETOP_ADDERROR )
        bHasAddError = TRUE;

    Insert( pData, Count() );
}


BOOL ScDetOpList::operator==( const ScDetOpList& r ) const
{
    // fuer Ref-Undo

    USHORT nCount = Count();
    BOOL bEqual = ( nCount == r.Count() );
    for (USHORT i=0; i<nCount && bEqual; i++)       // Reihenfolge muss auch gleich sein
        if ( !(*(*this)[i] == *r[i]) )              // Eintraege unterschiedlich ?
            bEqual = FALSE;

    return bEqual;
}

void ScDetOpList::Load( SvStream& rStream )
{
    ScMultipleReadHeader aHdr( rStream );
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!

    USHORT nNewCount;
    rStream >> nNewCount;

    ScAddress aPos;
    USHORT nOper;

    for (USHORT i=0; i<nNewCount; i++)
    {
        //  1) Position (ScAddress)
        //  2) Operation (USHORT)

        aHdr.StartEntry();

        rStream >> aPos;
        rStream >> nOper;
        Append( new ScDetOpData( aPos, (ScDetOpType) nOper ) );

        aHdr.EndEntry();
    }
#endif // SC_ROWLIMIT_STREAM_ACCESS
}

void ScDetOpList::Store( SvStream& rStream ) const
{
    ScMultipleWriteHeader aHdr( rStream );
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!

    USHORT nCount = Count();
    rStream << nCount;

    for (USHORT i=0; i<nCount; i++)
    {
        //  1) Position (ScAddress)
        //  2) Operation (USHORT)

        aHdr.StartEntry();

        ScDetOpData* pData = (*this)[i];
        rStream << pData->GetPos();
        rStream << (USHORT) pData->GetOperation();

        aHdr.EndEntry();
    }
#endif // SC_ROWLIMIT_STREAM_ACCESS
}



