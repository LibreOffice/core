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

#include <tools/multisel.hxx>

#include "pfuncache.hxx"
#include "printfun.hxx"
#include "docsh.hxx"
#include "markdata.hxx"
#include "prevloc.hxx"

//------------------------------------------------------------------------

ScPrintFuncCache::ScPrintFuncCache( ScDocShell* pD, const ScMarkData& rMark,
                                    const ScPrintSelectionStatus& rStatus ) :
    aSelection( rStatus ),
    pDocSh( pD ),
    nTotalPages( 0 ),
    bLocInitialized( false )
{
    //  page count uses the stored cell widths for the printer anyway,
    //  so ScPrintFunc with the document's printer can be used to count

    SfxPrinter* pPrinter = pDocSh->GetPrinter();

    ScRange aRange;
    const ScRange* pSelRange = NULL;
    if ( rMark.IsMarked() )
    {
        rMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }

    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();

    // avoid repeated progress bars if row heights for all sheets are needed
    if ( nTabCount > 1 && rMark.GetSelectCount() == nTabCount )
        pDocSh->UpdatePendingRowHeights( nTabCount-1, true );

    SCTAB nTab;
    for ( nTab=0; nTab<nTabCount; nTab++ )
    {
        long nAttrPage = nTab > 0 ? nFirstAttr[nTab-1] : 1;

        long nThisTab = 0;
        if ( rMark.GetTableSelect( nTab ) )
        {
            ScPrintFunc aFunc( pDocSh, pPrinter, nTab, nAttrPage, 0, pSelRange, &aSelection.GetOptions() );
            nThisTab = aFunc.GetTotalPages();
            nFirstAttr[nTab] = aFunc.GetFirstPageNo();          // from page style or previous sheet
        }
        else
            nFirstAttr[nTab] = nAttrPage;

        nPages[nTab] = nThisTab;
        nTotalPages += nThisTab;
    }
}

ScPrintFuncCache::~ScPrintFuncCache()
{
}

void ScPrintFuncCache::InitLocations( const ScMarkData& rMark, OutputDevice* pDev )
{
    if ( bLocInitialized )
        return;                 // initialize only once

    ScRange aRange;
    const ScRange* pSelRange = NULL;
    if ( rMark.IsMarked() )
    {
        rMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }

    long nRenderer = 0;     // 0-based physical page number across sheets
    long nTabStart = 0;

    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    for ( SCTAB nTab=0; nTab<nTabCount; nTab++ )
    {
        if ( rMark.GetTableSelect( nTab ) )
        {
            ScPrintFunc aFunc( pDev, pDocSh, nTab, nFirstAttr[nTab], nTotalPages, pSelRange, &aSelection.GetOptions() );
            aFunc.SetRenderFlag( sal_True );

            long nDisplayStart = GetDisplayStart( nTab );

            for ( long nPage=0; nPage<nPages[nTab]; nPage++ )
            {
                Range aPageRange( nRenderer+1, nRenderer+1 );
                MultiSelection aPage( aPageRange );
                aPage.SetTotalRange( Range(0,RANGE_MAX) );
                aPage.Select( aPageRange );

                ScPreviewLocationData aLocData( pDoc, pDev );
                aFunc.DoPrint( aPage, nTabStart, nDisplayStart, sal_False, &aLocData );

                ScRange aCellRange;
                Rectangle aPixRect;
                if ( aLocData.GetMainCellRange( aCellRange, aPixRect ) )
                    aLocations.push_back( ScPrintPageLocation( nRenderer, aCellRange, aPixRect ) );

                ++nRenderer;
            }

            nTabStart += nPages[nTab];
        }
    }

    bLocInitialized = true;
}

bool ScPrintFuncCache::FindLocation( const ScAddress& rCell, ScPrintPageLocation& rLocation ) const
{
    for ( std::vector<ScPrintPageLocation>::const_iterator aIter(aLocations.begin());
          aIter != aLocations.end(); aIter++ )
    {
        if ( aIter->aCellRange.In( rCell ) )
        {
            rLocation = *aIter;
            return true;
        }
    }
    return false;   // not found
}

sal_Bool ScPrintFuncCache::IsSameSelection( const ScPrintSelectionStatus& rStatus ) const
{
    return aSelection == rStatus;
}

SCTAB ScPrintFuncCache::GetTabForPage( long nPage ) const
{
    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    SCTAB nTab = 0;
    while ( nTab < nTabCount && nPage >= nPages[nTab] )
        nPage -= nPages[nTab++];
    return nTab;
}

long ScPrintFuncCache::GetTabStart( SCTAB nTab ) const
{
    long nRet = 0;
    for ( SCTAB i=0; i<nTab; i++ )
        nRet += nPages[i];
    return nRet;
}

long ScPrintFuncCache::GetDisplayStart( SCTAB nTab ) const
{
    //! merge with lcl_GetDisplayStart in preview?

    long nDisplayStart = 0;
    ScDocument* pDoc = pDocSh->GetDocument();
    for (SCTAB i=0; i<nTab; i++)
    {
        if ( pDoc->NeedPageResetAfterTab(i) )
            nDisplayStart = 0;
        else
            nDisplayStart += nPages[i];
    }
    return nDisplayStart;
}


