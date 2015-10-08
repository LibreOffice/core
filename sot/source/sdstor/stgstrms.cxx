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

#include <algorithm>

#include <string.h>
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <unotools/tempfile.hxx>
#include <set>

#include "sot/stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"
#include <memory>

///////////////////////////// class StgFAT

// The FAT class performs FAT operations on an underlying storage stream.
// This stream is either the master FAT stream (m == true ) or a normal
// storage stream, which then holds the FAT for small data allocations.

StgFAT::StgFAT( StgStrm& r, bool m ) : m_rStrm( r )
{
    m_bPhys   = m;
    m_nPageSize = m_rStrm.GetIo().GetPhysPageSize();
    m_nEntries  = m_nPageSize >> 2;
    m_nOffset   = 0;
    m_nMaxPage  = 0;
    m_nLimit    = 0;
}

// Retrieve the physical page for a given byte offset.

rtl::Reference< StgPage > StgFAT::GetPhysPage( sal_Int32 nByteOff )
{
    rtl::Reference< StgPage > pPg;
    // Position within the underlying stream
    // use the Pos2Page() method of the stream
    if( m_rStrm.Pos2Page( nByteOff ) )
    {
        m_nOffset = m_rStrm.GetOffset();
        sal_Int32 nPhysPage = m_rStrm.GetPage();
        // get the physical page (must be present)
        pPg = m_rStrm.GetIo().Get( nPhysPage, true );
    }
    return pPg;
}

// Get the follow page for a certain FAT page.

sal_Int32 StgFAT::GetNextPage( sal_Int32 nPg )
{
    if( nPg >= 0 )
    {
      rtl::Reference< StgPage > pPg = GetPhysPage( nPg << 2 );
      nPg = pPg.is() ? StgCache::GetFromPage( pPg, m_nOffset >> 2 ) : STG_EOF;
    }
    return nPg;
}

// Find the best fit block for the given size. Return
// the starting block and its size or STG_EOF and 0.
// nLastPage is a stopper which tells the current
// underlying stream size. It is treated as a recommendation
// to abort the search to inhibit excessive file growth.

sal_Int32 StgFAT::FindBlock( sal_Int32& nPgs )
{
    sal_Int32 nMinStart = STG_EOF, nMinLen = 0;
    sal_Int32 nMaxStart = STG_EOF, nMaxLen = 0x7FFFFFFFL;
    sal_Int32 nTmpStart = STG_EOF, nTmpLen = 0;
    sal_Int32 nPages    = m_rStrm.GetSize() >> 2;
    bool bFound     = false;
    rtl::Reference< StgPage > pPg;
    short nEntry = 0;
    for( sal_Int32 i = 0; i < nPages; i++, nEntry++ )
    {
        if( !( nEntry % m_nEntries ) )
        {
            // load the next page for that stream
            nEntry = 0;
            pPg = GetPhysPage( i << 2 );
            if( !pPg.is() )
                return STG_EOF;
        }
        sal_Int32 nCur = StgCache::GetFromPage( pPg, nEntry );
        if( nCur == STG_FREE )
        {
            // count the size of this area
            if( nTmpLen )
                nTmpLen++;
            else
                nTmpStart = i,
                nTmpLen   = 1;
            if( nTmpLen == nPgs
             // If we already did find a block, stop when reaching the limit
             || ( bFound && ( nEntry >= m_nLimit ) ) )
                break;
        }
        else if( nTmpLen )
        {
            if( nTmpLen > nPgs && nTmpLen < nMaxLen )
                // block > requested size
                nMaxLen = nTmpLen, nMaxStart = nTmpStart, bFound = true;
            else if( nTmpLen >= nMinLen )
            {
                // block < requested size
                nMinLen = nTmpLen, nMinStart = nTmpStart;
                bFound = true;
                if( nTmpLen == nPgs )
                    break;
            }
            nTmpStart = STG_EOF;
            nTmpLen   = 0;
        }
    }
    // Determine which block to use.
    if( nTmpLen )
    {
        if( nTmpLen > nPgs  && nTmpLen < nMaxLen )
            // block > requested size
            nMaxLen = nTmpLen, nMaxStart = nTmpStart;
        else if( nTmpLen >= nMinLen )
            // block < requested size
            nMinLen = nTmpLen, nMinStart = nTmpStart;
    }
    if( nMinStart != STG_EOF && nMaxStart != STG_EOF )
    {
        // two areas found; return the best fit area
        sal_Int32 nMinDiff = nPgs - nMinLen;
        sal_Int32 nMaxDiff = nMaxLen - nPgs;
        if( nMinDiff > nMaxDiff )
            nMinStart = STG_EOF;
    }
    if( nMinStart != STG_EOF )
    {
        nPgs = nMinLen; return nMinStart;
    }
    else
    {
        return nMaxStart;
    }
}

// Set up the consecutive chain for a given block.

bool StgFAT::MakeChain( sal_Int32 nStart, sal_Int32 nPgs )
{
    sal_Int32 nPos = nStart << 2;
    rtl::Reference< StgPage > pPg = GetPhysPage( nPos );
    if( !pPg.is() || !nPgs )
        return false;
    while( --nPgs )
    {
        if( m_nOffset >= m_nPageSize )
        {
            pPg = GetPhysPage( nPos );
            if( !pPg.is() )
                return false;
        }
        m_rStrm.GetIo().SetToPage( pPg, m_nOffset >> 2, ++nStart );
        m_nOffset += 4;
        nPos += 4;
    }
    if( m_nOffset >= m_nPageSize )
    {
        pPg = GetPhysPage( nPos );
        if( !pPg.is() )
            return false;
    }
    m_rStrm.GetIo().SetToPage( pPg, m_nOffset >> 2, STG_EOF );
    return true;
}

// Allocate a block of data from the given page number on.
// It the page number is != STG_EOF, chain the block.

sal_Int32 StgFAT::AllocPages( sal_Int32 nBgn, sal_Int32 nPgs )
{
    sal_Int32 nOrig = nBgn;
    sal_Int32 nLast = nBgn;
    sal_Int32 nBegin = STG_EOF;
    sal_Int32 nAlloc;
    sal_Int32 nPages = m_rStrm.GetSize() >> 2;
    short nPasses = 0;
    // allow for two passes
    while( nPasses < 2 )
    {
        // try to satisfy the request from the pool of free pages
        while( nPgs )
        {
            nAlloc = nPgs;
            nBegin = FindBlock( nAlloc );
            // no more blocks left in present alloc chain
            if( nBegin == STG_EOF )
                break;
            if( ( nBegin + nAlloc ) > m_nMaxPage )
                m_nMaxPage = nBegin + nAlloc;
            if( !MakeChain( nBegin, nAlloc ) )
                return STG_EOF;
            if( nOrig == STG_EOF )
                nOrig = nBegin;
            else
            {
                // Patch the chain
                rtl::Reference< StgPage > pPg = GetPhysPage( nLast << 2 );
                if( !pPg.is() )
                    return STG_EOF;
                m_rStrm.GetIo().SetToPage( pPg, m_nOffset >> 2, nBegin );
            }
            nLast = nBegin + nAlloc - 1;
            nPgs -= nAlloc;
        }
        if( nPgs && !nPasses )
        {
            // we need new, fresh space, so allocate and retry
            if( !m_rStrm.SetSize( ( nPages + nPgs ) << 2 ) )
                return STG_EOF;
            if( !m_bPhys && !InitNew( nPages ) )
                return 0;
                    // FIXME: this was originally "FALSE", whether or not that
                    // makes sense (or should be STG_EOF instead, say?)
            nPages = m_rStrm.GetSize() >> 2;
            nPasses++;
        }
        else
            break;
    }
    // now we should have a chain for the complete block
    if( nBegin == STG_EOF || nPgs )
    {
        m_rStrm.GetIo().SetError( SVSTREAM_FILEFORMAT_ERROR );
        return STG_EOF; // bad structure
    }
    return nOrig;
}

// Initialize newly allocated pages for a standard FAT stream
// It can be assumed that the stream size is always on
// a page boundary

bool StgFAT::InitNew( sal_Int32 nPage1 )
{
    sal_Int32 n = ( ( m_rStrm.GetSize() >> 2 ) - nPage1 ) / m_nEntries;
    if ( n > 0 )
    {
        while( n-- )
        {
            rtl::Reference< StgPage > pPg;
            // Position within the underlying stream
            // use the Pos2Page() method of the stream
            m_rStrm.Pos2Page( nPage1 << 2 );
            // Initialize the page
            pPg = m_rStrm.GetIo().Copy( m_rStrm.GetPage() );
            if ( !pPg.is() )
                return false;
            for( short i = 0; i < m_nEntries; i++ )
                m_rStrm.GetIo().SetToPage( pPg, i, STG_FREE );
            nPage1++;
        }
    }
    return true;
}

// Release a chain

bool StgFAT::FreePages( sal_Int32 nStart, bool bAll )
{
    while( nStart >= 0 )
    {
        rtl::Reference< StgPage > pPg = GetPhysPage( nStart << 2 );
        if( !pPg.is() )
            return false;
        nStart = StgCache::GetFromPage( pPg, m_nOffset >> 2 );
        // The first released page is either set to EOF or FREE
        m_rStrm.GetIo().SetToPage( pPg, m_nOffset >> 2, bAll ? STG_FREE : STG_EOF );
        bAll = true;
    }
    return true;
}

///////////////////////////// class StgStrm

// The base stream class provides basic functionality for seeking
// and accessing the data on a physical basis. It uses the built-in
// FAT class for the page allocations.

StgStrm::StgStrm( StgIo& r ) : rIo( r )
{
    pFat    = NULL;
    nStart  = nPage = STG_EOF;
    nOffset = 0;
    pEntry  = NULL;
    nPos = nSize = 0;
    nPageSize = rIo.GetPhysPageSize();
}

StgStrm::~StgStrm()
{
    delete pFat;
}

// Attach the stream to the given entry.

void StgStrm::SetEntry( StgDirEntry& r )
{
    r.m_aEntry.SetLeaf( STG_DATA, nStart );
    r.m_aEntry.SetSize( nSize );
    pEntry = &r;
    r.SetDirty();
}

/*
 * The page chain, is basically a singly linked list of slots each
 * point to the next page. Instead of traversing the file structure
 * for this each time build a simple flat in-memory vector list
 * of pages.
 */
void StgStrm::scanBuildPageChainCache(sal_Int32 *pOptionalCalcSize)
{
    if (nSize > 0)
        m_aPagesCache.reserve(nSize/nPageSize);

    bool bError = false;
    sal_Int32 nBgn = nStart;
    sal_Int32 nOptSize = 0;

    // Track already scanned PageNumbers here and use them to
    // see if an  already counted page is re-visited
    std::set< sal_Int32 > nUsedPageNumbers;

    while( nBgn >= 0 && !bError )
    {
        if( nBgn >= 0 )
            m_aPagesCache.push_back(nBgn);
        nBgn = pFat->GetNextPage( nBgn );

        //returned second is false if it already exists
        if (!nUsedPageNumbers.insert(nBgn).second)
        {
            SAL_WARN ("sot", "Error: page number " << nBgn << " already in chain for stream");
            bError = true;
        }

        nOptSize += nPageSize;
    }
    if (bError)
    {
        SAL_WARN("sot", "returning wrong format error");
        if (pOptionalCalcSize)
            rIo.SetError( ERRCODE_IO_WRONGFORMAT );
        m_aPagesCache.clear();
    }
    if (pOptionalCalcSize)
        *pOptionalCalcSize = nOptSize;
}

// Compute page number and offset for the given byte position.
// If the position is behind the size, set the stream right
// behind the EOF.
bool StgStrm::Pos2Page( sal_Int32 nBytePos )
{
    if ( !pFat )
        return false;

    // Values < 0 seek to the end
    if( nBytePos < 0 || nBytePos >= nSize )
        nBytePos = nSize;
    // Adjust the position back to offset 0
    nPos -= nOffset;
    sal_Int32 nMask = ~( nPageSize - 1 );
    sal_Int32 nOld = nPos & nMask;
    sal_Int32 nNew = nBytePos & nMask;
    nOffset = (short) ( nBytePos & ~nMask );
    nPos = nBytePos;
    if( nOld == nNew )
        return true;

    // See fdo#47644 for a .doc with a vast amount of pages where seeking around the
    // document takes a colossal amount of time

    // Please Note: we build the pagescache incrementally as we go if necessary,
    // so that a corrupted FAT doesn't poison the stream state for earlier reads
    size_t nIdx = nNew / nPageSize;
    if( nIdx >= m_aPagesCache.size() )
    {
        // Extend the FAT cache ! ...
        size_t nToAdd = nIdx + 1;

        if (m_aPagesCache.empty())
            m_aPagesCache.push_back( nStart );

        nToAdd -= m_aPagesCache.size();

        sal_Int32 nBgn = m_aPagesCache.back();

        // Start adding pages while we can
        while( nToAdd > 0 && nBgn >= 0 )
        {
            nBgn = pFat->GetNextPage( nBgn );
            if( nBgn >= 0 )
            {
                m_aPagesCache.push_back( nBgn );
                nToAdd--;
            }
        }
    }

    if ( nIdx > m_aPagesCache.size() )
    {
        SAL_WARN("sot", "seek to index " << nIdx <<
                 " beyond page cache size " << m_aPagesCache.size());
        // fdo#84229 - handle seek to end and back as eg. XclImpStream expects
        nPage = STG_EOF;
        nOffset = 0;
        // Intriguingly in the past we didn't reset nPos to match the real
        // length of the stream thus:
        //   nIdx = m_aPagesCache.size();
        //   nPos = nPageSize * nIdx;
        // so retain this behavior for now.
        return false;
    }

    // special case: seek to 1st byte of new, unallocated page
    // (in case the file size is a multiple of the page size)
    if( nBytePos == nSize && !nOffset && nIdx > 0 && nIdx == m_aPagesCache.size() )
    {
        nIdx--;
        nOffset = nPageSize;
    }
    else if ( nIdx == m_aPagesCache.size() )
    {
        nPage = STG_EOF;
        return false;
    }

    nPage = m_aPagesCache[ nIdx ];

    return nPage >= 0;
}

// Copy an entire stream. Both streams are allocated in the FAT.
// The target stream is this stream.

bool StgStrm::Copy( sal_Int32 nFrom, sal_Int32 nBytes )
{
    if ( !pFat )
        return false;

    m_aPagesCache.clear();

    sal_Int32 nTo = nStart;
    sal_Int32 nPgs = ( nBytes + nPageSize - 1 ) / nPageSize;
    while( nPgs-- )
    {
        if( nTo < 0 )
        {
            rIo.SetError( SVSTREAM_FILEFORMAT_ERROR );
            return false;
        }
        rIo.Copy( nTo, nFrom );
        if( nFrom >= 0 )
        {
            nFrom = pFat->GetNextPage( nFrom );
            if( nFrom < 0 )
            {
                rIo.SetError( SVSTREAM_FILEFORMAT_ERROR );
                return false;
            }
        }
        nTo = pFat->GetNextPage( nTo );
    }
    return true;
}

bool StgStrm::SetSize( sal_Int32 nBytes )
{
    if ( nBytes < 0 || !pFat )
        return false;

    m_aPagesCache.clear();

    // round up to page size
    sal_Int32 nOld = ( ( nSize + nPageSize - 1 ) / nPageSize ) * nPageSize;
    sal_Int32 nNew = ( ( nBytes + nPageSize - 1 ) / nPageSize ) * nPageSize;
    if( nNew > nOld )
    {
        if( !Pos2Page( nSize ) )
            return false;
        sal_Int32 nBgn = pFat->AllocPages( nPage, ( nNew - nOld ) / nPageSize );
        if( nBgn == STG_EOF )
            return false;
        if( nStart == STG_EOF )
            nStart = nPage = nBgn;
    }
    else if( nNew < nOld )
    {
        bool bAll = ( nBytes == 0 );
        if( !Pos2Page( nBytes ) || !pFat->FreePages( nPage, bAll ) )
            return false;
        if( bAll )
            nStart = nPage = STG_EOF;
    }
    if( pEntry )
    {
        // change the dir entry?
        if( !nSize || !nBytes )
            pEntry->m_aEntry.SetLeaf( STG_DATA, nStart );
        pEntry->m_aEntry.SetSize( nBytes );
        pEntry->SetDirty();
    }
    nSize = nBytes;
    pFat->SetLimit( GetPages() );
    return true;
}

// Return the # of allocated pages


//////////////////////////// class StgFATStrm

// The FAT stream class provides physical access to the master FAT.
// Since this access is implemented as a StgStrm, we can use the
// FAT allocator.

StgFATStrm::StgFATStrm( StgIo& r ) : StgStrm( r )
{
    pFat = new StgFAT( *this, true );
    nSize = rIo.m_aHdr.GetFATSize() * nPageSize;
}

bool StgFATStrm::Pos2Page( sal_Int32 nBytePos )
{
    // Values < 0 seek to the end
    if( nBytePos < 0 || nBytePos >= nSize  )
        nBytePos = nSize ? nSize - 1 : 0;
    nPage   = nBytePos / nPageSize;
    nOffset = (short) ( nBytePos % nPageSize );
    nPos    = nBytePos;
    nPage   = GetPage( (short) nPage, false );
    return nPage >= 0;
}

// Get the page number entry for the given page offset.

sal_Int32 StgFATStrm::GetPage( short nOff, bool bMake, sal_uInt16 *pnMasterAlloc )
{
    OSL_ENSURE( nOff >= 0, "The offset may not be negative!" );
    if( pnMasterAlloc ) *pnMasterAlloc = 0;
    if( nOff < StgHeader::GetFAT1Size() )
        return rIo.m_aHdr.GetFATPage( nOff );
    sal_Int32 nMaxPage = nSize >> 2;
    nOff = nOff - StgHeader::GetFAT1Size();
    // Anzahl der Masterpages, durch die wir iterieren muessen
    sal_uInt16 nMasterCount =  ( nPageSize >> 2 ) - 1;
    sal_uInt16 nBlocks = nOff / nMasterCount;
    // Offset in letzter Masterpage
    nOff = nOff % nMasterCount;

    rtl::Reference< StgPage > pOldPage;
    rtl::Reference< StgPage > pMaster;
    sal_Int32 nFAT = rIo.m_aHdr.GetFATChain();
    for( sal_uInt16 nCount = 0; nCount <= nBlocks; nCount++ )
    {
        if( nFAT == STG_EOF || nFAT == STG_FREE )
        {
            if( bMake )
            {
                m_aPagesCache.clear();

                // create a new master page
                nFAT = nMaxPage++;
                pMaster = rIo.Copy( nFAT );
                if ( pMaster.is() )
                {
                    for( short k = 0; k < (short)( nPageSize >> 2 ); k++ )
                        rIo.SetToPage( pMaster, k, STG_FREE );
                    // Verkettung herstellen
                    if( !pOldPage.is() )
                        rIo.m_aHdr.SetFATChain( nFAT );
                    else
                        rIo.SetToPage( pOldPage, nMasterCount, nFAT );
                    if( nMaxPage >= rIo.GetPhysPages() )
                        if( !rIo.SetSize( nMaxPage ) )
                            return STG_EOF;
                    // mark the page as used
                    // Platz fuer Masterpage schaffen
                    if( !pnMasterAlloc ) // Selbst Platz schaffen
                    {
                        if( !Pos2Page( nFAT << 2 ) )
                            return STG_EOF;
                        rtl::Reference< StgPage > pPg = rIo.Get( nPage, true );
                        if( !pPg.is() )
                            return STG_EOF;
                        rIo.SetToPage( pPg, nOffset >> 2, STG_MASTER );
                    }
                    else
                        (*pnMasterAlloc)++;
                    rIo.m_aHdr.SetMasters( nCount + 1 );
                    pOldPage = pMaster;
                }
            }
        }
        else
        {
            pMaster = rIo.Get( nFAT, true );
            if ( pMaster.is() )
            {
                nFAT = StgCache::GetFromPage( pMaster, nMasterCount );
                pOldPage = pMaster;
            }
        }
    }
    if( pMaster.is() )
        return StgCache::GetFromPage( pMaster, nOff );
    rIo.SetError( SVSTREAM_GENERALERROR );
    return STG_EOF;
}


// Set the page number entry for the given page offset.

bool StgFATStrm::SetPage( short nOff, sal_Int32 nNewPage )
{
    OSL_ENSURE( nOff >= 0, "The offset may not be negative!" );
    m_aPagesCache.clear();

    bool bRes = true;
    if( nOff < StgHeader::GetFAT1Size() )
        rIo.m_aHdr.SetFATPage( nOff, nNewPage );
    else
    {
        nOff = nOff - StgHeader::GetFAT1Size();
        // Anzahl der Masterpages, durch die wir iterieren muessen
        sal_uInt16 nMasterCount =  ( nPageSize >> 2 ) - 1;
        sal_uInt16 nBlocks = nOff / nMasterCount;
        // Offset in letzter Masterpage
        nOff = nOff % nMasterCount;

        rtl::Reference< StgPage > pMaster;
        sal_Int32 nFAT = rIo.m_aHdr.GetFATChain();
        for( sal_uInt16 nCount = 0; nCount <= nBlocks; nCount++ )
        {
            if( nFAT == STG_EOF || nFAT == STG_FREE )
            {
                pMaster = 0;
                break;
            }
            pMaster = rIo.Get( nFAT, true );
            if ( pMaster.is() )
                nFAT = StgCache::GetFromPage( pMaster, nMasterCount );
        }
        if( pMaster.is() )
            rIo.SetToPage( pMaster, nOff, nNewPage );
        else
        {
            rIo.SetError( SVSTREAM_GENERALERROR );
            bRes = false;
        }
    }

    // lock the page against access
    if( bRes )
    {
        Pos2Page( nNewPage << 2 );
        rtl::Reference< StgPage > pPg = rIo.Get( nPage, true );
        if( pPg.is() )
            rIo.SetToPage( pPg, nOffset >> 2, STG_FAT );
        else
            bRes = false;
    }
    return bRes;
}

bool StgFATStrm::SetSize( sal_Int32 nBytes )
{
    if ( nBytes < 0 )
        return false;

    m_aPagesCache.clear();

    // Set the number of entries to a multiple of the page size
    short nOld = (short) ( ( nSize + ( nPageSize - 1 ) ) / nPageSize );
    short nNew = (short) (
        ( nBytes + ( nPageSize - 1 ) ) / nPageSize ) ;
    if( nNew < nOld )
    {
        // release master pages
        for( short i = nNew; i < nOld; i++ )
            SetPage( i, STG_FREE );
    }
    else
    {
        while( nOld < nNew )
        {
            // allocate master pages
            // find a free master page slot
            sal_Int32 nPg = 0;
            sal_uInt16 nMasterAlloc = 0;
            nPg = GetPage( nOld, true, &nMasterAlloc );
            if( nPg == STG_EOF )
                return false;
            // 4 Bytes have been used for Allocation of each MegaMasterPage
            nBytes += nMasterAlloc << 2;

            // find a free page using the FAT allocator
            sal_Int32 n = 1;
            OSL_ENSURE( pFat, "The pointer is always initializer here!" );
            sal_Int32 nNewPage = pFat->FindBlock( n );
            if( nNewPage == STG_EOF )
            {
                // no free pages found; create a new page
                // Since all pages are allocated, extend
                // the file size for the next page!
                nNewPage = nSize >> 2;
                // if a MegaMasterPage was created avoid taking
                // the same Page
                nNewPage += nMasterAlloc;
                // adjust the file size if necessary
                if( nNewPage >= rIo.GetPhysPages() )
                    if( !rIo.SetSize( nNewPage + 1 ) )
                        return false;
            }
            // Set up the page with empty entries
            rtl::Reference< StgPage > pPg = rIo.Copy( nNewPage );
            if ( !pPg.is() )
                return false;
            for( short j = 0; j < (short)( nPageSize >> 2 ); j++ )
                rIo.SetToPage( pPg, j, STG_FREE );

            // store the page number into the master FAT
            // Set the size before so the correct FAT can be found
            nSize = ( nOld + 1 ) * nPageSize;
            SetPage( nOld, nNewPage );

            // MegaMasterPages were created, mark it them as used

            sal_uInt32 nMax = rIo.m_aHdr.GetMasters( );
            sal_uInt32 nFAT = rIo.m_aHdr.GetFATChain();
            if( nMasterAlloc )
                for( sal_uInt32 nCount = 0; nCount < nMax; nCount++ )
                {
                    if( !Pos2Page( nFAT << 2 ) )
                        return false;
                    if( nMax - nCount <= nMasterAlloc )
                    {
                        rtl::Reference< StgPage > piPg = rIo.Get( nPage, true );
                        if( !piPg.is() )
                            return false;
                        rIo.SetToPage( piPg, nOffset >> 2, STG_MASTER );
                    }
                    rtl::Reference< StgPage > pPage = rIo.Get( nFAT, true );
                    if( !pPage.is() ) return false;
                    nFAT = StgCache::GetFromPage( pPage, (nPageSize >> 2 ) - 1 );
                }

            nOld++;
            // We have used up 4 bytes for the STG_FAT entry
            nBytes += 4;
            nNew = (short) (
                ( nBytes + ( nPageSize - 1 ) ) / nPageSize );
        }
    }
    nSize = nNew * nPageSize;
    rIo.m_aHdr.SetFATSize( nNew );
    return true;
}

/////////////////////////// class StgDataStrm

// This class is a normal physical stream which can be initialized
// either with an existing dir entry or an existing FAT chain.
// The stream has a size increment which normally is 1, but which can be
// set to any value is you want the size to be incremented by certain values.

StgDataStrm::StgDataStrm( StgIo& r, sal_Int32 nBgn, sal_Int32 nLen ) : StgStrm( r )
{
    Init( nBgn, nLen );
}

StgDataStrm::StgDataStrm( StgIo& r, StgDirEntry& p ) : StgStrm( r )
{
    pEntry = &p;
    Init( p.m_aEntry.GetLeaf( STG_DATA ),
          p.m_aEntry.GetSize() );
}

void StgDataStrm::Init( sal_Int32 nBgn, sal_Int32 nLen )
{
    if ( rIo.m_pFAT )
        pFat = new StgFAT( *rIo.m_pFAT, true );

    OSL_ENSURE( pFat, "The pointer should not be empty!" );

    nStart = nPage = nBgn;
    nSize  = nLen;
    m_nIncr  = 1;
    nOffset = 0;
    if( nLen < 0 && pFat )
    {
        // determine the actual size of the stream by scanning
        // the FAT chain and counting the # of pages allocated
        scanBuildPageChainCache( &nSize );
    }
}

// Set the size of a physical stream.

bool StgDataStrm::SetSize( sal_Int32 nBytes )
{
    if ( !pFat )
        return false;

    nBytes = ( ( nBytes + m_nIncr - 1 ) / m_nIncr ) * m_nIncr;
    sal_Int32 nOldSz = nSize;
    if( ( nOldSz != nBytes ) )
    {
        if( !StgStrm::SetSize( nBytes ) )
            return false;
        sal_Int32 nMaxPage = pFat->GetMaxPage();
        if( nMaxPage > rIo.GetPhysPages() )
            if( !rIo.SetSize( nMaxPage ) )
                return false;
        // If we only allocated one page or less, create this
        // page in the cache for faster throughput. The current
        // position is the former EOF point.
        if( ( nSize - 1 )  / nPageSize - ( nOldSz - 1 ) / nPageSize == 1 )
        {
            Pos2Page( nBytes );
            if( nPage >= 0 )
                rIo.Copy( nPage );
        }
    }
    return true;
}

// Get the address of the data byte at a specified offset.
// If bForce = true, a read of non-existent data causes
// a read fault.

void* StgDataStrm::GetPtr( sal_Int32 Pos, bool bForce, bool bDirty )
{
    if( Pos2Page( Pos ) )
    {
        rtl::Reference< StgPage > pPg = rIo.Get( nPage, bForce );
        if (pPg.is() && nOffset < pPg->GetSize())
        {
            if( bDirty )
                rIo.SetDirty( pPg );
            return static_cast<sal_uInt8 *>(pPg->GetData()) + nOffset;
        }
    }
    return NULL;
}

// This could easily be adapted to a better algorithm by determining
// the amount of consecutable blocks before doing a read. The result
// is the number of bytes read. No error is generated on EOF.

sal_Int32 StgDataStrm::Read( void* pBuf, sal_Int32 n )
{
    if ( n < 0 )
        return 0;

    if( ( nPos + n ) > nSize )
        n = nSize - nPos;
    sal_Int32 nDone = 0;
    while( n )
    {
        short nBytes = nPageSize - nOffset;
        rtl::Reference< StgPage > pPg;
        if( (sal_Int32) nBytes > n )
            nBytes = (short) n;
        if( nBytes )
        {
            short nRes;
            void *p = static_cast<sal_uInt8 *>(pBuf) + nDone;
            if( nBytes == nPageSize )
            {
                pPg = rIo.Find( nPage );
                if( pPg.is() )
                {
                    // data is present, so use the cached data
                    memcpy( p, pPg->GetData(), nBytes );
                    nRes = nBytes;
                }
                else
                    // do a direct (unbuffered) read
                    nRes = (short) rIo.Read( nPage, p, 1 ) * nPageSize;
            }
            else
            {
                // partial block read through the cache.
                pPg = rIo.Get( nPage, false );
                if( !pPg.is() )
                    break;
                memcpy( p, static_cast<sal_uInt8*>(pPg->GetData()) + nOffset, nBytes );
                nRes = nBytes;
            }
            nDone += nRes;
            nPos += nRes;
            n -= nRes;
            nOffset = nOffset + nRes;
            if( nRes != nBytes )
                break;  // read error or EOF
        }
        // Switch to next page if necessary
        if( nOffset >= nPageSize && !Pos2Page( nPos ) )
            break;
    }
    return nDone;
}

sal_Int32 StgDataStrm::Write( const void* pBuf, sal_Int32 n )
{
    if ( n < 0 )
        return 0;

    sal_Int32 nDone = 0;
    if( ( nPos + n ) > nSize )
    {
        sal_Int32 nOld = nPos;
        if( !SetSize( nPos + n ) )
            return 0;
        Pos2Page( nOld );
    }
    while( n )
    {
        short nBytes = nPageSize - nOffset;
        rtl::Reference< StgPage > pPg;
        if( (sal_Int32) nBytes > n )
            nBytes = (short) n;
        if( nBytes )
        {
            short nRes;
            const void *p = static_cast<const sal_uInt8 *>(pBuf) + nDone;
            if( nBytes == nPageSize )
            {
                pPg = rIo.Find( nPage );
                if( pPg.is() )
                {
                    // data is present, so use the cached data
                    memcpy( pPg->GetData(), p, nBytes );
                    rIo.SetDirty( pPg );
                    nRes = nBytes;
                }
                else
                    // do a direct (unbuffered) write
                    nRes = (short) rIo.Write( nPage, const_cast<void*>(p), 1 ) * nPageSize;
            }
            else
            {
                // partial block read through the cache.
                pPg = rIo.Get( nPage, false );
                if( !pPg.is() )
                    break;
                memcpy( static_cast<sal_uInt8*>(pPg->GetData()) + nOffset, p, nBytes );
                rIo.SetDirty( pPg );
                nRes = nBytes;
            }
            nDone += nRes;
            nPos += nRes;
            n -= nRes;
            nOffset = nOffset + nRes;
            if( nRes != nBytes )
                break;  // read error
        }
        // Switch to next page if necessary
        if( nOffset >= nPageSize && !Pos2Page( nPos ) )
            break;
    }
    return nDone;
}

//////////////////////////// class StgSmallStream

// The small stream class provides access to streams with a size < 4096 bytes.
// This stream is a StgStream containing small pages. The FAT for this stream
// is also a StgStream. The start of the FAT is in the header at DataRootPage,
// the stream itself is pointed to by the root entry (it holds start & size).

StgSmallStrm::StgSmallStrm( StgIo& r, sal_Int32 nBgn, sal_Int32 nLen ) : StgStrm( r )
{
    Init( nBgn, nLen );
}

StgSmallStrm::StgSmallStrm( StgIo& r, StgDirEntry& p ) : StgStrm( r )
{
    pEntry = &p;
    Init( p.m_aEntry.GetLeaf( STG_DATA ),
          p.m_aEntry.GetSize() );
}

void StgSmallStrm::Init( sal_Int32 nBgn, sal_Int32 nLen )
{
    if ( rIo.m_pDataFAT )
        pFat = new StgFAT( *rIo.m_pDataFAT, false );
    m_pData = rIo.m_pDataStrm;
    OSL_ENSURE( pFat && m_pData, "The pointers should not be empty!" );

    nPageSize = rIo.GetDataPageSize();
    nStart =
    nPage  = nBgn;
    nSize  = nLen;
}

// This could easily be adapted to a better algorithm by determining
// the amount of consecutable blocks before doing a read. The result
// is the number of bytes read. No error is generated on EOF.

sal_Int32 StgSmallStrm::Read( void* pBuf, sal_Int32 n )
{
    // We can safely assume that reads are not huge, since the
    // small stream is likely to be < 64 KBytes.
    if( ( nPos + n ) > nSize )
        n = nSize - nPos;
    short nDone = 0;
    while( n )
    {
        short nBytes = nPageSize - nOffset;
        if( (sal_Int32) nBytes > n )
            nBytes = (short) n;
        if( nBytes )
        {
            if( !m_pData || !m_pData->Pos2Page( nPage * nPageSize + nOffset ) )
                break;
            // all reading through the stream
            short nRes = (short) m_pData->Read( static_cast<sal_uInt8*>(pBuf) + nDone, nBytes );
            nDone = nDone + nRes;
            nPos += nRes;
            n -= nRes;
            nOffset = nOffset + nRes;
            // read problem?
            if( nRes != nBytes )
                break;
        }
        // Switch to next page if necessary
        if( nOffset >= nPageSize && !Pos2Page( nPos ) )
            break;
    }
    return nDone;
}

sal_Int32 StgSmallStrm::Write( const void* pBuf, sal_Int32 n )
{
    // you can safely assume that reads are not huge, since the
    // small stream is likely to be < 64 KBytes.
    short nDone = 0;
    if( ( nPos + n ) > nSize )
    {
        sal_Int32 nOld = nPos;
        if( !SetSize( nPos + n ) )
            return 0;
        Pos2Page( nOld );
    }
    while( n )
    {
        short nBytes = nPageSize - nOffset;
        if( (sal_Int32) nBytes > n )
            nBytes = (short) n;
        if( nBytes )
        {
            // all writing goes through the stream
            sal_Int32 nDataPos = nPage * nPageSize + nOffset;
            if ( !m_pData
              || ( m_pData->GetSize() < ( nDataPos + nBytes )
                && !m_pData->SetSize( nDataPos + nBytes ) ) )
                break;
            if( !m_pData->Pos2Page( nDataPos ) )
                break;
            short nRes = (short) m_pData->Write( static_cast<sal_uInt8 const *>(pBuf) + nDone, nBytes );
            nDone = nDone + nRes;
            nPos += nRes;
            n -= nRes;
            nOffset = nOffset + nRes;
            // write problem?
            if( nRes != nBytes )
                break;
        }
        // Switch to next page if necessary
        if( nOffset >= nPageSize && !Pos2Page( nPos ) )
            break;
    }
    return nDone;
}

/////////////////////////// class StgTmpStrm

// The temporary stream uses a memory stream if < 32K, otherwise a
// temporary file.

#define THRESHOLD 32768L

StgTmpStrm::StgTmpStrm( sal_uLong nInitSize )
          : SvMemoryStream( nInitSize > THRESHOLD
                              ? 16
                            : ( nInitSize ? nInitSize : 16 ), 4096 )
{
    m_pStrm = NULL;
    // this calls FlushData, so all members should be set by this time
    SetBufferSize( 0 );
    if( nInitSize > THRESHOLD )
        SetSize( nInitSize );
}

bool StgTmpStrm::Copy( StgTmpStrm& rSrc )
{
    sal_uLong n    = rSrc.GetSize();
    sal_uLong nCur = rSrc.Tell();
    SetSize( n );
    if( GetError() == SVSTREAM_OK )
    {
        std::unique_ptr<sal_uInt8[]> p(new sal_uInt8[ 4096 ]);
        rSrc.Seek( 0L );
        Seek( 0L );
        while( n )
        {
            sal_uLong nn = n;
            if( nn > 4096 )
                nn = 4096;
            if( rSrc.Read( p.get(), nn ) != nn )
                break;
            if( Write( p.get(), nn ) != nn )
                break;
            n -= nn;
        }
        p.reset();
        rSrc.Seek( nCur );
        Seek( nCur );
        return n == 0;
    }
    else
        return false;
}

StgTmpStrm::~StgTmpStrm()
{
    if( m_pStrm )
    {
        m_pStrm->Close();
        osl::File::remove( m_aName );
        delete m_pStrm;
    }
}

sal_uLong StgTmpStrm::GetSize() const
{
    sal_uLong n;
    if( m_pStrm )
    {
        sal_uLong old = m_pStrm->Tell();
        n = m_pStrm->Seek( STREAM_SEEK_TO_END );
        m_pStrm->Seek( old );
    }
    else
        n = nEndOfData;
    return n;
}

void StgTmpStrm::SetSize(sal_uInt64 n)
{
    if( m_pStrm )
        m_pStrm->SetStreamSize( n );
    else
    {
        if( n > THRESHOLD )
        {
            m_aName = utl::TempFile(0, false).GetURL();
            SvFileStream* s = new SvFileStream( m_aName, STREAM_READWRITE );
            sal_uLong nCur = Tell();
            sal_uLong i = nEndOfData;
            std::unique_ptr<sal_uInt8[]> p(new sal_uInt8[ 4096 ]);
            if( i )
            {
                Seek( 0L );
                while( i )
                {
                    sal_uLong nb = ( i > 4096 ) ? 4096 : i;
                    if( Read( p.get(), nb ) == nb
                        && s->Write( p.get(), nb ) == nb )
                        i -= nb;
                    else
                        break;
                }
            }
            if( !i && n > nEndOfData )
            {
                // We have to write one byte at the end of the file
                // if the file is bigger than the memstream to see
                // if it fits on disk
                s->Seek(nEndOfData);
                memset(p.get(), 0x00, 4096);
                i = n - nEndOfData;
                while (i)
                {
                    sal_uLong const nb = (i > 4096) ? 4096 : i;
                    if (s->Write(p.get(), nb) == nb)
                        i -= nb;
                    else
                        break; // error
                }
                s->Flush();
                if( s->GetError() != SVSTREAM_OK )
                    i = 1;
            }
            Seek( nCur );
            s->Seek( nCur );
            if( i )
            {
                SetError( s->GetError() );
                delete s;
                return;
            }
            m_pStrm = s;
            // Shrink the memory to 16 bytes, which seems to be the minimum
            ReAllocateMemory( - ( (long) nEndOfData - 16 ) );
        }
        else
        {
            if( n > nEndOfData )
            {
                SvMemoryStream::SetSize(n);
            }
            else
                nEndOfData = n;
        }
    }
}

sal_uLong StgTmpStrm::GetData( void* pData, sal_uLong n )
{
    if( m_pStrm )
    {
        n = m_pStrm->Read( pData, n );
        SetError( m_pStrm->GetError() );
        return n;
    }
    else
        return SvMemoryStream::GetData( pData, n );
}

sal_uLong StgTmpStrm::PutData( const void* pData, sal_uLong n )
{
    sal_uInt32 nCur = Tell();
    sal_uInt32 nNew = nCur + n;
    if( nNew > THRESHOLD && !m_pStrm )
    {
        SetSize( nNew );
        if( GetError() != SVSTREAM_OK )
            return 0;
    }
    if( m_pStrm )
    {
        nNew = m_pStrm->Write( pData, n );
        SetError( m_pStrm->GetError() );
    }
    else
        nNew = SvMemoryStream::PutData( pData, n );
    return nNew;
}

sal_uInt64 StgTmpStrm::SeekPos(sal_uInt64 n)
{
    // check if a truncated STREAM_SEEK_TO_END was passed
    assert(n != SAL_MAX_UINT32);
    if( n == STREAM_SEEK_TO_END )
        n = GetSize();
    if( n && n > THRESHOLD && !m_pStrm )
    {
        SetSize( n );
        if( GetError() != SVSTREAM_OK )
            return Tell();
        else
            return n;
    }
    else if( m_pStrm )
    {
        n = m_pStrm->Seek( n );
        SetError( m_pStrm->GetError() );
        return n;
    }
    else
        return SvMemoryStream::SeekPos( n );
}

void StgTmpStrm::FlushData()
{
    if( m_pStrm )
    {
        m_pStrm->Flush();
        SetError( m_pStrm->GetError() );
    }
    else
        SvMemoryStream::FlushData();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
