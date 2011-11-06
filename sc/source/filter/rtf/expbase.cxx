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




#include "expbase.hxx"
#include "document.hxx"
#include "editutil.hxx"


//------------------------------------------------------------------

#if defined(UNX)
const sal_Char __FAR_DATA ScExportBase::sNewLine = '\012';
#else
const sal_Char __FAR_DATA ScExportBase::sNewLine[] = "\015\012";
#endif


ScExportBase::ScExportBase( SvStream& rStrmP, ScDocument* pDocP,
                const ScRange& rRangeP )
            :
            rStrm( rStrmP ),
            aRange( rRangeP ),
            pDoc( pDocP ),
            pFormatter( pDocP->GetFormatTable() ),
            pEditEngine( NULL )
{
}


ScExportBase::~ScExportBase()
{
    delete pEditEngine;
}


sal_Bool ScExportBase::GetDataArea( SCTAB nTab, SCCOL& nStartCol,
            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const
{
    pDoc->GetDataStart( nTab, nStartCol, nStartRow );
    pDoc->GetPrintArea( nTab, nEndCol, nEndRow, sal_True );
    return TrimDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}


sal_Bool ScExportBase::TrimDataArea( SCTAB nTab, SCCOL& nStartCol,
            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const
{
    SCCOL nLastCol;
    while ( nStartCol <= nEndCol && pDoc->ColHidden(nStartCol, nTab, nLastCol))
        ++nStartCol;
    while ( nStartCol <= nEndCol && pDoc->ColHidden(nEndCol, nTab, nLastCol))
        --nEndCol;
    nStartRow = pDoc->FirstVisibleRow(nStartRow, nEndRow, nTab);
    nEndRow = pDoc->LastVisibleRow(nStartRow, nEndRow, nTab);
    return nStartCol <= nEndCol && nStartRow <= nEndRow && nEndRow !=
        ::std::numeric_limits<SCROW>::max();
}


sal_Bool ScExportBase::IsEmptyTable( SCTAB nTab ) const
{
    if ( !pDoc->HasTable( nTab ) || !pDoc->IsVisible( nTab ) )
        return sal_True;
    SCCOL nStartCol, nEndCol;
    SCROW nStartRow, nEndRow;
    return !GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}


ScFieldEditEngine& ScExportBase::GetEditEngine() const
{
    if ( !pEditEngine )
        ((ScExportBase*)this)->pEditEngine = new ScFieldEditEngine( pDoc->GetEditPool() );
    return *pEditEngine;
}


