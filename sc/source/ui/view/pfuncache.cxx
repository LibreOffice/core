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

#include <tools/multisel.hxx>

#include "pfuncache.hxx"
#include "printfun.hxx"
#include "docsh.hxx"
#include "markdata.hxx"
#include "prevloc.hxx"

ScPrintFuncCache::ScPrintFuncCache( ScDocShell* pD, const ScMarkData& rMark,
                                    const ScPrintSelectionStatus& rStatus ) :
    aSelection( rStatus ),
    pDocSh( pD ),
    nTotalPages( 0 ),
    nPages(),
    nFirstAttr(),
    bLocInitialized( false )
{
    //  page count uses the stored cell widths for the printer anyway,
    //  so ScPrintFunc with the document's printer can be used to count

    SfxPrinter* pPrinter = pDocSh->GetPrinter();

    ScRange aRange;
    const ScRange* pSelRange = nullptr;
    if ( rMark.IsMarked() )
    {
        rMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }

    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();

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
            nFirstAttr.push_back( aFunc.GetFirstPageNo() );         // from page style or previous sheet
        }
        else
            nFirstAttr.push_back( nAttrPage );

        nPages.push_back( nThisTab );
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
    const ScRange* pSelRange = nullptr;
    if ( rMark.IsMarked() )
    {
        rMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }

    long nRenderer = 0;     // 0-based physical page number across sheets
    long nTabStart = 0;

    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && (*itr) < nTabCount; ++itr)
    {
        SCTAB nTab = *itr;
        ScPrintFunc aFunc( pDev, pDocSh, nTab, nFirstAttr[nTab], nTotalPages, pSelRange, &aSelection.GetOptions() );
        aFunc.SetRenderFlag( true );

        long nDisplayStart = GetDisplayStart( nTab );

        for ( long nPage=0; nPage<nPages[nTab]; nPage++ )
        {
            Range aPageRange( nRenderer+1, nRenderer+1 );
            MultiSelection aPage( aPageRange );
            aPage.SetTotalRange( Range(0,RANGE_MAX) );
            aPage.Select( aPageRange );

            ScPreviewLocationData aLocData( &rDoc, pDev );
            aFunc.DoPrint( aPage, nTabStart, nDisplayStart, false, &aLocData );

            ScRange aCellRange;
            Rectangle aPixRect;
            if ( aLocData.GetMainCellRange( aCellRange, aPixRect ) )
                aLocations.push_back( ScPrintPageLocation( nRenderer, aCellRange, aPixRect ) );

            ++nRenderer;
        }

        nTabStart += nPages[nTab];
    }

    bLocInitialized = true;
}

bool ScPrintFuncCache::FindLocation( const ScAddress& rCell, ScPrintPageLocation& rLocation ) const
{
    for ( std::vector<ScPrintPageLocation>::const_iterator aIter(aLocations.begin()), aEnd(aLocations.end());
          aIter != aEnd; ++aIter )
    {
        if ( aIter->aCellRange.In( rCell ) )
        {
            rLocation = *aIter;
            return true;
        }
    }
    return false;   // not found
}

bool ScPrintFuncCache::IsSameSelection( const ScPrintSelectionStatus& rStatus ) const
{
    return aSelection == rStatus;
}

SCTAB ScPrintFuncCache::GetTabForPage( long nPage ) const
{
    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTabCount = rDoc.GetTableCount();
    SCTAB nTab = 0;
    while ( nTab < nTabCount && nPage >= nPages[nTab] )
        nPage -= nPages[nTab++];
    if (nTab >= nTabCount)
        nTab = nTabCount - 1;
    return nTab;
}

long ScPrintFuncCache::GetTabStart( SCTAB nTab ) const
{
    long nRet = 0;
    for ( SCTAB i=0; i<nTab&& i < static_cast<SCTAB>(nPages.size()); i++ )
        nRet += nPages[i];
    return nRet;
}

long ScPrintFuncCache::GetDisplayStart( SCTAB nTab ) const
{
    //! merge with lcl_GetDisplayStart in preview?

    long nDisplayStart = 0;
    ScDocument& rDoc = pDocSh->GetDocument();
    for (SCTAB i=0; i<nTab; i++)
    {
        if ( rDoc.NeedPageResetAfterTab(i) )
            nDisplayStart = 0;
        else
        {
            if ( i < static_cast<SCTAB>(nPages.size()) )
                nDisplayStart += nPages[i];
            else
                OSL_FAIL("nPages out of bounds, FIX IT!");
        }
    }
    return nDisplayStart;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
