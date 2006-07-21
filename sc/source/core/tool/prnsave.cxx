/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prnsave.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:39:11 $
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

#include "prnsave.hxx"
#include "global.hxx"
#include "address.hxx"

// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------

//
//      Daten pro Tabelle
//

ScPrintSaverTab::ScPrintSaverTab() :
    mpRepeatCol(NULL),
    mpRepeatRow(NULL),
    mbEntireSheet(FALSE)
{
}

ScPrintSaverTab::~ScPrintSaverTab()
{
    delete mpRepeatCol;
    delete mpRepeatRow;
}

void ScPrintSaverTab::SetAreas( const ScRangeVec& rRanges, BOOL bEntireSheet )
{
    maPrintRanges = rRanges;
    mbEntireSheet = bEntireSheet;
}

void ScPrintSaverTab::SetRepeat( const ScRange* pCol, const ScRange* pRow )
{
    delete mpRepeatCol;
    mpRepeatCol = pCol ? new ScRange(*pCol) : NULL;
    delete mpRepeatRow;
    mpRepeatRow = pRow ? new ScRange(*pRow) : NULL;
}

inline BOOL PtrEqual( const ScRange* p1, const ScRange* p2 )
{
    return ( !p1 && !p2 ) || ( p1 && p2 && *p1 == *p2 );
}

BOOL ScPrintSaverTab::operator==( const ScPrintSaverTab& rCmp ) const
{
    return
        PtrEqual( mpRepeatCol, rCmp.mpRepeatCol ) &&
        PtrEqual( mpRepeatRow, rCmp.mpRepeatRow ) &&
        (mbEntireSheet == rCmp.mbEntireSheet) &&
        (maPrintRanges == rCmp.maPrintRanges);
}

//
//      Daten fuer das ganze Dokument
//

ScPrintRangeSaver::ScPrintRangeSaver( SCTAB nCount ) :
    nTabCount( nCount )
{
    if (nCount > 0)
        pData = new ScPrintSaverTab[nCount];
    else
        pData = NULL;
}

ScPrintRangeSaver::~ScPrintRangeSaver()
{
    delete[] pData;
}

ScPrintSaverTab& ScPrintRangeSaver::GetTabData(SCTAB nTab)
{
    DBG_ASSERT(nTab<nTabCount,"ScPrintRangeSaver Tab zu gross");
    return pData[nTab];
}

const ScPrintSaverTab& ScPrintRangeSaver::GetTabData(SCTAB nTab) const
{
    DBG_ASSERT(nTab<nTabCount,"ScPrintRangeSaver Tab zu gross");
    return pData[nTab];
}

BOOL ScPrintRangeSaver::operator==( const ScPrintRangeSaver& rCmp ) const
{
    BOOL bEqual = ( nTabCount == rCmp.nTabCount );
    if (bEqual)
        for (SCTAB i=0; i<nTabCount; i++)
            if (!(pData[i]==rCmp.pData[i]))
            {
                bEqual = FALSE;
                break;
            }
    return bEqual;
}




