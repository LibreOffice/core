/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    mbEntireSheet(sal_False)
{
}

ScPrintSaverTab::~ScPrintSaverTab()
{
    delete mpRepeatCol;
    delete mpRepeatRow;
}

void ScPrintSaverTab::SetAreas( const ScRangeVec& rRanges, sal_Bool bEntireSheet )
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

inline sal_Bool PtrEqual( const ScRange* p1, const ScRange* p2 )
{
    return ( !p1 && !p2 ) || ( p1 && p2 && *p1 == *p2 );
}

sal_Bool ScPrintSaverTab::operator==( const ScPrintSaverTab& rCmp ) const
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

sal_Bool ScPrintRangeSaver::operator==( const ScPrintRangeSaver& rCmp ) const
{
    sal_Bool bEqual = ( nTabCount == rCmp.nTabCount );
    if (bEqual)
        for (SCTAB i=0; i<nTabCount; i++)
            if (!(pData[i]==rCmp.pData[i]))
            {
                bEqual = sal_False;
                break;
            }
    return bEqual;
}




