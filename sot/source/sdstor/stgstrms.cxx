/*************************************************************************
 *
 *  $RCSfile: stgstrms.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mm $ $Date: 2000-10-12 16:18:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string.h>     // memcpy()

#include <osl/file.hxx>
#include <tools/tempfile.hxx>
#include <tools/debug.hxx>

#include "stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"
#pragma hdrstop

#if defined(W31)
        #include <tools/svwin.h>
    #define memcpy hmemcpy
    #define __HUGE _huge
#else
    #define __HUGE
#endif

///////////////////////////// class StgFAT ///////////////////////////////

// The FAT class performs FAT operations on an underlying storage stream.
// This stream is either the master FAT stream (m == TRUE ) or a normal
// storage stream, which then holds the FAT for small data allocations.

StgFAT::StgFAT( StgStrm& r, BOOL m ) : rStrm( r )
{
    bPhys   = m;
    nPageSize = rStrm.GetIo().GetPhysPageSize();
    nEntries  = nPageSize >> 2;
    nOffset   = 0;
    nMaxPage  = 0;
    nLimit    = 0;
}

// Retrieve the physical page for a given byte offset.

StgPage* StgFAT::GetPhysPage( INT32 nByteOff )
{
    StgPage* pPg = NULL;
    // Position within the underlying stream
    // use the Pos2Page() method of the stream
    if( rStrm.Pos2Page( nByteOff ) )
    {
        nOffset = rStrm.GetOffset();
        INT32 nPhysPage = rStrm.GetPage();
        // get the physical page (must be present)
        pPg = rStrm.GetIo().Get( nPhysPage, TRUE );
    }
    return pPg;
}

// Get the follow page for a certain FAT page.

INT32 StgFAT::GetNextPage( INT32 nPg )
{
    if( nPg >= 0 )
    {
        StgPage* pPg = GetPhysPage( nPg << 2 );
        nPg = pPg ? pPg->GetPage( nOffset >> 2 ) : STG_EOF;
    }
    return nPg;
}

// Find the best fit block for the given size. Return
// the starting block and its size or STG_EOF and 0.
// nLastPage is a stopper which tells the current
// underlying stream size. It is treated as a recommendation
// to abort the search to inhibit excessive file growth.

INT32 StgFAT::FindBlock( INT32& nPgs )
{
    INT32 nMinStart = STG_EOF, nMinLen = 0;
    INT32 nMaxStart = STG_EOF, nMaxLen = 0x7FFFFFFFL;
    INT32 nTmpStart = STG_EOF, nTmpLen = 0;
    INT32 nPages    = rStrm.GetSize() >> 2;
    BOOL bFound     = FALSE;
    StgPage* pPg;
    short nEntry = 0;
    for( INT32 i = 0; i < nPages; i++, nEntry++ )
    {
        if( !( nEntry % nEntries ) )
        {
            // load the next page for that stream
            nEntry = 0;
            pPg = GetPhysPage( i << 2 );
            if( !pPg )
                return STG_EOF;
        }
        INT32 nCur = pPg->GetPage( nEntry );
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
             || ( bFound && ( nEntry >= nLimit ) ) )
                break;
        }
        else if( nTmpLen )
        {
            if( nTmpLen > nPgs && nTmpLen < nMaxLen )
                // block > requested size
                nMaxLen = nTmpLen, nMaxStart = nTmpStart, bFound = TRUE;
            else if( nTmpLen >= nMinLen )
            {
                // block < requested size
                nMinLen = nTmpLen, nMinStart = nTmpStart;
                bFound = TRUE;
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
        INT32 nMinDiff = nPgs - nMinLen;
        INT32 nMaxDiff = nMaxLen - nPgs;
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

BOOL StgFAT::MakeChain( INT32 nStart, INT32 nPgs )
{
    INT32 nPos = nStart << 2;
    StgPage* pPg = GetPhysPage( nPos );
    if( !pPg || !nPgs )
        return FALSE;
    while( --nPgs )
    {
        if( nOffset >= nPageSize )
        {
            pPg = GetPhysPage( nPos );
            if( !pPg )
                return FALSE;
        }
        pPg->SetPage( nOffset >> 2, ++nStart );
        nOffset += 4;
        nPos += 4;
    }
    if( nOffset >= nPageSize )
    {
        pPg = GetPhysPage( nPos );
        if( !pPg )
            return FALSE;
    }
    pPg->SetPage( nOffset >> 2, STG_EOF );
    return TRUE;
}

// Allocate a block of data from the given page number on.
// It the page number is != STG_EOF, chain the block.

INT32 StgFAT::AllocPages( INT32 nBgn, INT32 nPgs )
{
    INT32 nOrig = nBgn;
    INT32 nLast = nBgn;
    INT32 nBegin, nAlloc;
    INT32 nPages = rStrm.GetSize() >> 2;
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
            if( ( nBegin + nAlloc ) > nMaxPage )
                nMaxPage = nBegin + nAlloc;
            if( !MakeChain( nBegin, nAlloc ) )
                return STG_EOF;
            if( nOrig == STG_EOF )
                nOrig = nBegin;
            else
            {
                // Patch the chain
                StgPage* pPg = GetPhysPage( nLast << 2 );
                if( !pPg )
                    return STG_EOF;
                pPg->SetPage( nOffset >> 2, nBegin );
            }
            nLast = nBegin + nAlloc - 1;
            nPgs -= nAlloc;
        }
        if( nPgs && !nPasses )
        {
            // we need new, fresh space, so allocate and retry
            if( !rStrm.SetSize( ( nPages + nPgs ) << 2 ) )
                return STG_EOF;
            if( !bPhys && !InitNew( nPages ) )
                return FALSE;
            nPages = rStrm.GetSize() >> 2;
            nPasses++;
        }
        else
            break;
    }
    // now we should have a chain for the complete block
    if( nBegin == STG_EOF || nPgs )
    {
        rStrm.GetIo().SetError( SVSTREAM_FILEFORMAT_ERROR );
        return STG_EOF; // bad structure
    }
    return nOrig;
}

// Initialize newly allocated pages for a standard FAT stream
// It can be assumed that the stream size is always on
// a page boundary

BOOL StgFAT::InitNew( INT32 nPage1 )
{
    INT32 n = ( ( rStrm.GetSize() >> 2 ) - nPage1 ) / nEntries;
    while( n-- )
    {
        StgPage* pPg = NULL;
        // Position within the underlying stream
        // use the Pos2Page() method of the stream
        rStrm.Pos2Page( nPage1 << 2 );
        // Initialize the page
        pPg = rStrm.GetIo().Copy( rStrm.GetPage(), STG_FREE );
        for( short i = 0; i < nEntries; i++ )
            pPg->SetPage( i, STG_FREE );
        nPage1++;
    }
    return TRUE;
}

// Release a chain

BOOL StgFAT::FreePages( INT32 nStart, BOOL bAll )
{
    while( nStart >= 0 )
    {
        StgPage* pPg = GetPhysPage( nStart << 2 );
        if( !pPg )
            return FALSE;
        nStart = pPg->GetPage( nOffset >> 2 );
        // The first released page is either set to EOF or FREE
        pPg->SetPage( nOffset >> 2, bAll ? STG_FREE : STG_EOF );
        bAll = TRUE;
    }
    return TRUE;
}

///////////////////////////// class StgStrm ////////////////////////////////

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
    r.aEntry.SetLeaf( STG_DATA, nStart );
    r.aEntry.SetSize( nSize );
    pEntry = &r;
    r.SetDirty();
}

// Compute page number and offset for the given byte position.
// If the position is behind the size, set the stream right
// behind the EOF.

BOOL StgStrm::Pos2Page( INT32 nBytePos )
{
    INT32 nRel, nBgn;
    // Values < 0 seek to the end
    if( nBytePos < 0 || nBytePos >= nSize )
        nBytePos = nSize;
    // Adjust the position back to offset 0
    nPos -= nOffset;
    INT32 nMask = ~( nPageSize - 1 );
    INT32 nOld = nPos & nMask;
    INT32 nNew = nBytePos & nMask;
    nOffset = (short) ( nBytePos & ~nMask );
    nPos = nBytePos;
    if( nOld == nNew )
        return TRUE;
    if( nNew > nOld )
    {
        // the new position is behind the current, so an incremental
        // positioning is OK. Set the page relative position
        nRel = nNew - nOld;
        nBgn = nPage;
    }
    else
    {
        // the new position is before the current, so we have to scan
        // the entire chain.
        nRel = nNew;
        nBgn = nStart;
    }
    // now, traverse the FAT chain.
    nRel /= nPageSize;
    INT32 nLast = STG_EOF;
    while( nRel && nBgn >= 0 )
    {
        nLast = nBgn;
        nBgn = pFat->GetNextPage( nBgn );
        nRel--;
    }
    // special case: seek to 1st byte of new, unallocated page
    // (in case the file size is a multiple of the page size)
    if( nBytePos == nSize && nBgn == STG_EOF && !nRel && !nOffset )
        nBgn = nLast, nOffset = nPageSize;
    if( nBgn < 0 && nBgn != STG_EOF )
    {
        rIo.SetError( SVSTREAM_FILEFORMAT_ERROR );
        nBgn = STG_EOF;
        nOffset = nPageSize;
    }
    nPage = nBgn;
    return BOOL( nRel == 0 && nPage >= 0 );
}

// Retrieve the physical page for a given byte offset.

StgPage* StgStrm::GetPhysPage( INT32 nBytePos, BOOL bForce )
{
    if( !Pos2Page( nBytePos ) )
        return NULL;
    return rIo.Get( nPage, bForce );
}

// Copy an entire stream. Both streams are allocated in the FAT.
// The target stream is this stream.

BOOL StgStrm::Copy( INT32 nFrom, INT32 nBytes )
{
    INT32 nTo = nStart;
    INT32 nPgs = ( nBytes + nPageSize - 1 ) / nPageSize;
    while( nPgs-- )
    {
        if( nTo < 0 )
        {
            rIo.SetError( SVSTREAM_FILEFORMAT_ERROR );
            return FALSE;
        }
        rIo.Copy( nTo, nFrom );
        if( nFrom >= 0 )
        {
            nFrom = pFat->GetNextPage( nFrom );
            if( nFrom < 0 )
            {
                rIo.SetError( SVSTREAM_FILEFORMAT_ERROR );
                return FALSE;
            }
        }
        nTo = pFat->GetNextPage( nTo );
    }
    return TRUE;
}

BOOL StgStrm::SetSize( INT32 nBytes )
{
    // round up to page size
    INT32 nOld = ( ( nSize + nPageSize - 1 ) / nPageSize ) * nPageSize;
    INT32 nNew = ( ( nBytes + nPageSize - 1 ) / nPageSize ) * nPageSize;
    if( nNew > nOld )
    {
        if( !Pos2Page( nSize ) )
            return FALSE;
        INT32 nBgn = pFat->AllocPages( nPage, ( nNew - nOld ) / nPageSize );
        if( nBgn == STG_EOF )
            return FALSE;
        if( nStart == STG_EOF )
            nStart = nPage = nBgn;
    }
    else if( nNew < nOld )
    {
        BOOL bAll = BOOL( nBytes == 0 );
        if( !Pos2Page( nBytes ) || !pFat->FreePages( nPage, bAll ) )
            return FALSE;
        if( bAll )
            nStart = nPage = STG_EOF;
    }
    if( pEntry )
    {
        // change the dir entry?
        if( !nSize || !nBytes )
            pEntry->aEntry.SetLeaf( STG_DATA, nStart );
        pEntry->aEntry.SetSize( nBytes );
        pEntry->SetDirty();
    }
    nSize = nBytes;
    pFat->SetLimit( GetPages() );
    return TRUE;
}

// Return the # of allocated pages

INT32 StgStrm::GetPages()
{
    return ( nSize + nPageSize - 1 ) / nPageSize;
}

//////////////////////////// class StgFATStrm //////////////////////////////

// The FAT stream class provides physical access to the master FAT.
// Since this access is implemented as a StgStrm, we can use the
// FAT allocator.

StgFATStrm::StgFATStrm( StgIo& r ) : StgStrm( r )
{
    pFat = new StgFAT( *this, TRUE );
    nSize = rIo.aHdr.GetFATSize() * nPageSize;
}

BOOL StgFATStrm::Pos2Page( INT32 nBytePos )
{
    // Values < 0 seek to the end
    if( nBytePos < 0 || nBytePos >= nSize  )
        nBytePos = nSize ? nSize - 1 : 0;
    nPage   = nBytePos / nPageSize;
    nOffset = (short) ( nBytePos % nPageSize );
    nPos    = nBytePos;
    nPage   = GetPage( (short) nPage, FALSE );
    return BOOL( nPage >= 0 );
}

// Retrieve the physical page for a given byte offset.
// Since Pos2Page() already has computed the physical offset,
// use the byte offset directly.

StgPage* StgFATStrm::GetPhysPage( INT32 nBytePos, BOOL bForce )
{
    return rIo.Get( nBytePos / ( nPageSize >> 2 ), bForce );
}

// Get the page number entry for the given page offset.

INT32 StgFATStrm::GetPage( short nOff, BOOL bMake, USHORT *pnMasterAlloc )
{
    if( pnMasterAlloc ) *pnMasterAlloc = 0;
    if( nOff < rIo.aHdr.GetFAT1Size() )
        return rIo.aHdr.GetFATPage( nOff );
    INT32 nMaxPage = nSize >> 2;
    nOff -= rIo.aHdr.GetFAT1Size();
    // Anzahl der Masterpages, durch die wir iterieren muessen
    USHORT nMasterCount =  ( nPageSize >> 2 ) - 1;
    USHORT nBlocks = nOff / nMasterCount;
    // Offset in letzter Masterpage
    nOff = nOff % nMasterCount;

    StgPage* pOldPage = 0;
    StgPage* pMaster = 0;
    INT32 nFAT = rIo.aHdr.GetFATChain();
    for( USHORT nCount = 0; nCount <= nBlocks; nCount++ )
    {
        if( nFAT == STG_EOF || nFAT == STG_FREE )
        {
            if( bMake )
            {
                // create a new master page
                nFAT = nMaxPage++;
                pMaster = rIo.Copy( nFAT, STG_FREE );
                for( short k = 0; k < ( nPageSize >> 2 ); k++ )
                    pMaster->SetPage( k, STG_FREE );
                // Verkettung herstellen
                if( !pOldPage )
                    rIo.aHdr.SetFATChain( nFAT );
                else
                    pOldPage->SetPage( nMasterCount, nFAT );
                if( nMaxPage >= rIo.GetPhysPages() )
                    if( !rIo.SetSize( nMaxPage ) )
                        return STG_EOF;
                // mark the page as used
                // Platz fuer Masterpage schaffen
                if( !pnMasterAlloc ) // Selbst Platz schaffen
                {
                    if( !Pos2Page( nFAT << 2 ) )
                        return STG_EOF;
                    StgPage* pPg = rIo.Get( nPage, TRUE );
                    if( !pPg )
                        return STG_EOF;
                    pPg->SetPage( nOffset >> 2, STG_MASTER );
                }
                else
                    (*pnMasterAlloc)++;
                rIo.aHdr.SetMasters( nCount + 1 );
                pOldPage = pMaster;
            }
        }
        else
        {
            pMaster = rIo.Get( nFAT, TRUE );
            nFAT = pMaster->GetPage( nMasterCount );
            pOldPage = pMaster;
        }
    }
    if( pMaster )
        return pMaster->GetPage( nOff );
    rIo.SetError( SVSTREAM_GENERALERROR );
    return STG_EOF;
}


// Set the page number entry for the given page offset.

BOOL StgFATStrm::SetPage( short nOff, INT32 nNewPage )
{
    BOOL bRes = TRUE;
    if( nOff < rIo.aHdr.GetFAT1Size() )
        rIo.aHdr.SetFATPage( nOff, nNewPage );
    else
    {
        nOff -= rIo.aHdr.GetFAT1Size();
        // Anzahl der Masterpages, durch die wir iterieren muessen
        USHORT nMasterCount =  ( nPageSize >> 2 ) - 1;
        USHORT nBlocks = nOff / nMasterCount;
        // Offset in letzter Masterpage
        nOff = nOff % nMasterCount;

        StgPage* pMaster = 0;
        INT32 nFAT = rIo.aHdr.GetFATChain();
        for( USHORT nCount = 0; nCount <= nBlocks; nCount++ )
        {
            if( nFAT == STG_EOF || nFAT == STG_FREE )
            {
                pMaster = 0;
                break;
            }
            pMaster = rIo.Get( nFAT, TRUE );
            nFAT = pMaster->GetPage( nMasterCount );
        }
        if( pMaster )
            pMaster->SetPage( nOff, nNewPage );
        else
        {
            rIo.SetError( SVSTREAM_GENERALERROR );
            bRes = FALSE;
        }
    }

    // lock the page against access
    if( bRes )
    {
        Pos2Page( nNewPage << 2 );
        StgPage* pPg = rIo.Get( nPage, TRUE );
        if( pPg )
            pPg->SetPage( nOffset >> 2, STG_FAT );
        else
            bRes = FALSE;
    }
    return bRes;
}

BOOL StgFATStrm::SetSize( INT32 nBytes )
{
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
            INT32 nPg = 0;
            USHORT nMasterAlloc = 0;
            nPg = GetPage( nOld, TRUE, &nMasterAlloc );
            if( nPg == STG_EOF )
                return FALSE;
            // 4 Bytes have been used for Allocation of each MegaMasterPage
            nBytes += nMasterAlloc << 2;

            // find a free page using the FAT allocator
            INT32 n = 1;
            INT32 nNewPage = pFat->FindBlock( n );
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
                        return FALSE;
            }
            // Set up the page with empty entries
            StgPage* pPg = rIo.Copy( nNewPage, STG_FREE );
            for( short j = 0; j < ( nPageSize >> 2 ); j++ )
                pPg->SetPage( j, STG_FREE );

            // store the page number into the master FAT
            // Set the size before so the correct FAT can be found
            nSize = ( nOld + 1 ) * nPageSize;
            SetPage( nOld, nNewPage );

            // MegaMasterPages were created, mark it them as used

            UINT32 nMax = rIo.aHdr.GetMasters( );
            UINT32 nFAT = rIo.aHdr.GetFATChain();
            if( nMasterAlloc )
                for( USHORT nCount = 0; nCount < nMax; nCount++ )
                {
                    if( !Pos2Page( nFAT << 2 ) )
                        return FALSE;
                    if( nMax - nCount <= nMasterAlloc )
                    {
                        StgPage* pPg = rIo.Get( nPage, TRUE );
                        if( !pPg )
                            return FALSE;
                        pPg->SetPage( nOffset >> 2, STG_MASTER );
                    }
                    StgPage* pPage = rIo.Get( nFAT, TRUE );
                    if( !pPage ) return FALSE;
                    nFAT = pPage->GetPage( (nPageSize >> 2 ) - 1 );
                }

            nOld++;
            // We have used up 4 bytes for the STG_FAT entry
            nBytes += 4;
            nNew = (short) (
                ( nBytes + ( nPageSize - 1 ) ) / nPageSize );
        }
    }
    nSize = nNew * nPageSize;
    rIo.aHdr.SetFATSize( nNew );
    return TRUE;
}

/////////////////////////// class StgDataStrm //////////////////////////////

// This class is a normal physical stream which can be initialized
// either with an existing dir entry or an existing FAT chain.
// The stream has a size increment which normally is 1, but which can be
// set to any value is you want the size to be incremented by certain values.

StgDataStrm::StgDataStrm( StgIo& r, INT32 nBgn, INT32 nLen ) : StgStrm( r )
{
    Init( nBgn, nLen );
}

StgDataStrm::StgDataStrm( StgIo& r, StgDirEntry* p ) : StgStrm( r )
{
    pEntry = p;
    Init( p->aEntry.GetLeaf( STG_DATA ),
          p->aEntry.GetSize() );
}

void StgDataStrm::Init( INT32 nBgn, INT32 nLen )
{
    pFat = new StgFAT( *rIo.pFAT, TRUE );
    nStart = nPage = nBgn;
    nSize  = nLen;
    nIncr  = 1;
    nOffset = 0;
    if( nLen < 0 )
    {
        // determine the actual size of the stream by scanning
        // the FAT chain and counting the # of pages allocated
        nSize = 0;
        INT32 nOldBgn = -1;
        while( nBgn >= 0 && nBgn != nOldBgn )
        {
            nOldBgn = nBgn;
            nBgn = pFat->GetNextPage( nBgn );
            if( nBgn == nOldBgn )
                rIo.SetError( ERRCODE_IO_WRONGFORMAT );
            nSize += nPageSize;
        }
    }
}

// Set the size of a physical stream.

BOOL StgDataStrm::SetSize( INT32 nBytes )
{
    nBytes = ( ( nBytes + nIncr - 1 ) / nIncr ) * nIncr;
    INT32 nOldSz = nSize;
    if( ( nOldSz != nBytes ) )
    {
        if( !StgStrm::SetSize( nBytes ) )
            return FALSE;
        INT32 nMaxPage = pFat->GetMaxPage();
        if( nMaxPage > rIo.GetPhysPages() )
            if( !rIo.SetSize( nMaxPage ) )
                return FALSE;
        // If we only allocated one page or less, create this
        // page in the cache for faster throughput. The current
        // position is the former EOF point.
        if( ( nSize - 1 )  / nPageSize - ( nOldSz - 1 ) / nPageSize == 1 )
        {
            Pos2Page( nBytes );
            if( nPage >= 0 )
                rIo.Copy( nPage, STG_FREE );
        }
    }
    return TRUE;
}

// Get the address of the data byte at a specified offset.
// If bForce = TRUE, a read of non-existent data causes
// a read fault.

void* StgDataStrm::GetPtr( INT32 nPos, BOOL bForce, BOOL bDirty )
{
    if( Pos2Page( nPos ) )
    {
        StgPage* pPg = rIo.Get( nPage, bForce );
        if( pPg )
        {
            pPg->SetOwner( pEntry );
            if( bDirty )
                pPg->SetDirty();
            return ((BYTE *)pPg->GetData()) + nOffset;
        }
    }
    return NULL;
}

// This could easily be adapted to a better algorithm by determining
// the amount of consecutable blocks before doing a read. The result
// is the number of bytes read. No error is generated on EOF.

INT32 StgDataStrm::Read( void* pBuf, INT32 n )
{
    if( ( nPos + n ) > nSize )
        n = nSize - nPos;
    INT32 nDone = 0;
    while( n )
    {
        short nBytes = nPageSize - nOffset;
        short nRes;
        StgPage* pPg;
        if( (INT32) nBytes > n )
            nBytes = (short) n;
        if( nBytes )
        {
            void *p = (BYTE *) pBuf + nDone;
            if( nBytes == nPageSize )
            {
                pPg = rIo.Find( nPage );
                if( pPg )
                {
                    // data is present, so use the cached data
                    pPg->SetOwner( pEntry );
                    memcpy( p, pPg->GetData(), nBytes );
                    nRes = nBytes;
                }
                else
                    // do a direct (unbuffered) read
                    nRes = (short) rIo.Read( nPage, p, 1 ) * nPageSize;
            }
            else
            {
                // partial block read thru the cache.
                pPg = rIo.Get( nPage, FALSE );
                if( !pPg )
                    break;
                pPg->SetOwner( pEntry );
                memcpy( p, (BYTE*)pPg->GetData() + nOffset, nBytes );
                nRes = nBytes;
            }
            nDone += nRes;
            nPos += nRes;
            n -= nRes;
            nOffset += nRes;
            if( nRes != nBytes )
                break;  // read error or EOF
        }
        // Switch to next page if necessary
        if( nOffset >= nPageSize && !Pos2Page( nPos ) )
            break;
    }
    return nDone;
}

INT32 StgDataStrm::Write( const void* pBuf, INT32 n )
{
    INT32 nDone = 0;
    if( ( nPos + n ) > nSize )
    {
        INT32 nOld = nPos;
        if( !SetSize( nPos + n ) )
            return FALSE;
        Pos2Page( nOld );
    }
    while( n )
    {
        short nBytes = nPageSize - nOffset;
        short nRes;
        StgPage* pPg;
        if( (INT32) nBytes > n )
            nBytes = (short) n;
        if( nBytes )
        {
            const void *p = (const BYTE *) pBuf + nDone;
            if( nBytes == nPageSize )
            {
                pPg = rIo.Find( nPage );
                if( pPg )
                {
                    // data is present, so use the cached data
                    pPg->SetOwner( pEntry );
                    memcpy( pPg->GetData(), p, nBytes );
                    pPg->SetDirty();
                    nRes = nBytes;
                }
                else
                    // do a direct (unbuffered) write
                    nRes = (short) rIo.Write( nPage, (void*) p, 1 ) * nPageSize;
            }
            else
            {
                // partial block read thru the cache.
                pPg = rIo.Get( nPage, FALSE );
                if( !pPg )
                    break;
                pPg->SetOwner( pEntry );
                memcpy( (BYTE*)pPg->GetData() + nOffset, p, nBytes );
                pPg->SetDirty();
                nRes = nBytes;
            }
            nDone += nRes;
            nPos += nRes;
            n -= nRes;
            nOffset += nRes;
            if( nRes != nBytes )
                break;  // read error
        }
        // Switch to next page if necessary
        if( nOffset >= nPageSize && !Pos2Page( nPos ) )
            break;
    }
    return nDone;
}

//////////////////////////// class StgSmallStream ///////////////////////////

// The small stream class provides access to streams with a size < 4096 bytes.
// This stream is a StgStream containing small pages. The FAT for this stream
// is also a StgStream. The start of the FAT is in the header at DataRootPage,
// the stream itself is pointed to by the root entry (it holds start & size).

StgSmallStrm::StgSmallStrm( StgIo& r, INT32 nBgn, INT32 nLen ) : StgStrm( r )
{
    Init( nBgn, nLen );
}

StgSmallStrm::StgSmallStrm( StgIo& r, StgDirEntry* p ) : StgStrm( r )
{
    pEntry = p;
    Init( p->aEntry.GetLeaf( STG_DATA ),
          p->aEntry.GetSize() );
}

void StgSmallStrm::Init( INT32 nBgn, INT32 nLen )
{
    pFat = new StgFAT( *rIo.pDataFAT, FALSE );
    pData = rIo.pDataStrm;
    nPageSize = rIo.GetDataPageSize();
    nStart =
    nPage  = nBgn;
    nSize  = nLen;
}

// This could easily be adapted to a better algorithm by determining
// the amount of consecutable blocks before doing a read. The result
// is the number of bytes read. No error is generated on EOF.

INT32 StgSmallStrm::Read( void* pBuf, INT32 n )
{
    // We can safely assume that reads are not huge, since the
    // small stream is likely to be < 64 KBytes.
    if( ( nPos + n ) > nSize )
        n = nSize - nPos;
    short nDone = 0;
    while( n )
    {
        short nBytes = nPageSize - nOffset;
        if( (INT32) nBytes > n )
            nBytes = (short) n;
        if( nBytes )
        {
            if( !pData->Pos2Page( nPage * nPageSize + nOffset ) )
                break;
            // all reading thru the stream
            short nRes = (short) pData->Read( (BYTE*)pBuf + nDone, nBytes );
            nDone += nRes;
            nPos += nRes;
            n -= nRes;
            nOffset += nRes;
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

INT32 StgSmallStrm::Write( const void* pBuf, INT32 n )
{
    // you can safely assume that reads are not huge, since the
    // small stream is likely to be < 64 KBytes.
    short nDone = 0;
    if( ( nPos + n ) > nSize )
    {
        INT32 nOld = nPos;
        if( !SetSize( nPos + n ) )
            return FALSE;
        Pos2Page( nOld );
    }
    while( n )
    {
        short nBytes = nPageSize - nOffset;
        if( (INT32) nBytes > n )
            nBytes = (short) n;
        if( nBytes )
        {
            // all writing goes thru the stream
            INT32 nDataPos = nPage * nPageSize + nOffset;
            if( pData->GetSize() < ( nDataPos + nBytes ) )
                if( !pData->SetSize( nDataPos + nBytes ) )
                    break;
            if( !pData->Pos2Page( nDataPos ) )
                break;
            short nRes = (short) pData->Write( (BYTE*)pBuf + nDone, nBytes );
            nDone += nRes;
            nPos += nRes;
            n -= nRes;
            nOffset += nRes;
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

/////////////////////////// class StgTmpStrm /////////////////////////////

// The temporary stream uses a memory stream if < 32K, otherwise a
// temporary file.

#define THRESHOLD 32768L

StgTmpStrm::StgTmpStrm( ULONG nInitSize )
          : SvMemoryStream( nInitSize > THRESHOLD
                              ? 16
                            : ( nInitSize ? nInitSize : 16 ), 4096 )
{
    pStrm = NULL;
    // this calls FlushData, so all members should be set by this time
    SetBufferSize( 0 );
    if( nInitSize > THRESHOLD )
        SetSize( nInitSize );
}

BOOL StgTmpStrm::Copy( StgTmpStrm& rSrc )
{
    ULONG n    = rSrc.GetSize();
    ULONG nCur = rSrc.Tell();
    SetSize( n );
    if( GetError() == SVSTREAM_OK )
    {
        void* p = new BYTE[ 4096 ];
        rSrc.Seek( 0L );
        Seek( 0L );
        while( n )
        {
            ULONG nn = n;
            if( nn > 4096 )
                nn = 4096;
            if( rSrc.Read( p, nn ) != nn )
                break;
            if( Write( p, nn ) != nn )
                break;
            n -= nn;
        }
        delete p;
        rSrc.Seek( nCur );
        Seek( nCur );
        return BOOL( n == 0 );
    }
    else
        return FALSE;
}

StgTmpStrm::~StgTmpStrm()
{
    if( pStrm )
    {
        pStrm->Close();
        osl::File::remove( aName );
        delete pStrm;
    }
}

ULONG StgTmpStrm::GetSize()
{
    ULONG n;
    if( pStrm )
    {
        ULONG old = pStrm->Tell();
        n = pStrm->Seek( STREAM_SEEK_TO_END );
        pStrm->Seek( old );
    }
    else
        n = nEndOfData;
    return n;
}

void StgTmpStrm::SetSize( ULONG n )
{
    if( pStrm )
        pStrm->SetStreamSize( n );
    else
    {
        if( n > THRESHOLD )
        {
            aName = TempFile::CreateTempName();
            SvFileStream* s = new SvFileStream( aName, STREAM_READWRITE );
            ULONG nCur = Tell();
            ULONG i = nEndOfData;
            if( i )
            {
                void* p = new BYTE[ 4096 ];
                Seek( 0L );
                while( i )
                {
                    ULONG nb = ( i > 4096 ) ? 4096 : i;
                    if( Read( p, nb ) == nb
                     && s->Write( p, nb ) == nb )
                        i -= nb;
                    else
                        break;
                }
                delete p;
            }
            if( !i && n > nEndOfData )
            {
                // We have to write one byte at the end of the file
                // if the file is bigger than the memstream to see
                // if it fits on disk
                s->Seek( n - 1 );
                s->Write( &i, 1 );
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
            pStrm = s;
            // Shrink the memory to 16 bytes, which seems to be the minimum
            ReAllocateMemory( - ( (long) nEndOfData - 16 ) );
        }
        else
        {
            if( n > nEndOfData )
            {
                ULONG nCur = Tell();
                Seek( nEndOfData - 1 );
                *this << (BYTE) 0;
                Seek( nCur );
            }
            else
                nEndOfData = n;
        }
    }
}

ULONG StgTmpStrm::GetData( void* pData, ULONG n )
{
    if( pStrm )
    {
        n = pStrm->Read( pData, n );
        SetError( pStrm->GetError() );
        return n;
    }
    else
        return SvMemoryStream::GetData( (sal_Char *)pData, n );
}

ULONG StgTmpStrm::PutData( const void* pData, ULONG n )
{
    UINT32 nCur = Tell();
    UINT32 nNew = nCur + n;
    if( nNew > THRESHOLD && !pStrm )
    {
        SetSize( nNew );
        if( GetError() != SVSTREAM_OK )
            return 0;
    }
    if( pStrm )
    {
        nNew = pStrm->Write( pData, n );
        SetError( pStrm->GetError() );
    }
    else
        nNew = SvMemoryStream::PutData( (sal_Char*)pData, n );
    return nNew;
}

ULONG StgTmpStrm::SeekPos( ULONG n )
{
    if( n == STREAM_SEEK_TO_END )
        n = GetSize();
    if( n && n > THRESHOLD && !pStrm )
    {
        SetSize( n );
        if( GetError() != SVSTREAM_OK )
            return Tell();
        else
            return n;
    }
    else if( pStrm )
    {
        n = pStrm->Seek( n );
        SetError( pStrm->GetError() );
        return n;
    }
    else
        return SvMemoryStream::SeekPos( n );
}

void StgTmpStrm::FlushData()
{
    if( pStrm )
    {
        pStrm->Flush();
        SetError( pStrm->GetError() );
    }
    else
        SvMemoryStream::FlushData();
}

