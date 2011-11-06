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

#include "detdata.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"

//------------------------------------------------------------------------

SV_IMPL_PTRARR( ScDetOpArr_Impl, ScDetOpDataPtr );

//------------------------------------------------------------------------

ScDetOpList::ScDetOpList(const ScDetOpList& rList) :
    ScDetOpArr_Impl(),
    bHasAddError( sal_False )
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
            bEqual = sal_False;

    return bEqual;
}



