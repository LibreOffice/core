/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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







StgFAT::StgFAT( StgStrm& r, bool m ) : rStrm( r )
{
    bPhys   = m;
    nPageSize = rStrm.GetIo().GetPhysPageSize();
    nEntries  = nPageSize >> 2;
    nOffset   = 0;
    nMaxPage  = 0;
    nLimit    = 0;
}



rtl::Reference< StgPage > StgFAT::GetPhysPage( sal_Int32 nByteOff )
{
    rtl::Reference< StgPage > pPg;
    
    
    if( rStrm.Pos2Page( nByteOff ) )
    {
        nOffset = rStrm.GetOffset();
        sal_Int32 nPhysPage = rStrm.GetPage();
        
        pPg = rStrm.GetIo().Get( nPhysPage, true );
    }
    return pPg;
}



sal_Int32 StgFAT::GetNextPage( sal_Int32 nPg )
{
    if( nPg >= 0 )
    {
      rtl::Reference< StgPage > pPg = GetPhysPage( nPg << 2 );
      nPg = pPg.is() ? rStrm.GetIo().GetFromPage( pPg, nOffset >> 2 ) : STG_EOF;
    }
    return nPg;
}







sal_Int32 StgFAT::FindBlock( sal_Int32& nPgs )
{
    sal_Int32 nMinStart = STG_EOF, nMinLen = 0;
    sal_Int32 nMaxStart = STG_EOF, nMaxLen = 0x7FFFFFFFL;
    sal_Int32 nTmpStart = STG_EOF, nTmpLen = 0;
    sal_Int32 nPages    = rStrm.GetSize() >> 2;
    bool bFound     = false;
    rtl::Reference< StgPage > pPg;
    short nEntry = 0;
    for( sal_Int32 i = 0; i < nPages; i++, nEntry++ )
    {
        if( !( nEntry % nEntries ) )
        {
            
            nEntry = 0;
            pPg = GetPhysPage( i << 2 );
            if( !pPg.is() )
                return STG_EOF;
        }
        sal_Int32 nCur = rStrm.GetIo().GetFromPage( pPg, nEntry );
        if( nCur == STG_FREE )
        {
            
            if( nTmpLen )
                nTmpLen++;
            else
                nTmpStart = i,
                nTmpLen   = 1;
            if( nTmpLen == nPgs
             
             || ( bFound && ( nEntry >= nLimit ) ) )
                break;
        }
        else if( nTmpLen )
        {
            if( nTmpLen > nPgs && nTmpLen < nMaxLen )
                
                nMaxLen = nTmpLen, nMaxStart = nTmpStart, bFound = true;
            else if( nTmpLen >= nMinLen )
            {
                
                nMinLen = nTmpLen, nMinStart = nTmpStart;
                bFound = true;
                if( nTmpLen == nPgs )
                    break;
            }
            nTmpStart = STG_EOF;
            nTmpLen   = 0;
        }
    }
    
    if( nTmpLen )
    {
        if( nTmpLen > nPgs  && nTmpLen < nMaxLen )
            
            nMaxLen = nTmpLen, nMaxStart = nTmpStart;
        else if( nTmpLen >= nMinLen )
            
            nMinLen = nTmpLen, nMinStart = nTmpStart;
    }
    if( nMinStart != STG_EOF && nMaxStart != STG_EOF )
    {
        
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



bool StgFAT::MakeChain( sal_Int32 nStart, sal_Int32 nPgs )
{
    sal_Int32 nPos = nStart << 2;
    rtl::Reference< StgPage > pPg = GetPhysPage( nPos );
    if( !pPg.is() || !nPgs )
        return false;
    while( --nPgs )
    {
        if( nOffset >= nPageSize )
        {
            pPg = GetPhysPage( nPos );
            if( !pPg.is() )
                return false;
        }
        rStrm.GetIo().SetToPage( pPg, nOffset >> 2, ++nStart );
        nOffset += 4;
        nPos += 4;
    }
    if( nOffset >= nPageSize )
    {
        pPg = GetPhysPage( nPos );
        if( !pPg.is() )
            return false;
    }
    rStrm.GetIo().SetToPage( pPg, nOffset >> 2, STG_EOF );
    return true;
}




sal_Int32 StgFAT::AllocPages( sal_Int32 nBgn, sal_Int32 nPgs )
{
    sal_Int32 nOrig = nBgn;
    sal_Int32 nLast = nBgn;
    sal_Int32 nBegin = STG_EOF;
    sal_Int32 nAlloc;
    sal_Int32 nPages = rStrm.GetSize() >> 2;
    short nPasses = 0;
    
    while( nPasses < 2 )
    {
        
        while( nPgs )
        {
            nAlloc = nPgs;
            nBegin = FindBlock( nAlloc );
            
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
                
                rtl::Reference< StgPage > pPg = GetPhysPage( nLast << 2 );
                if( !pPg.is() )
                    return STG_EOF;
                rStrm.GetIo().SetToPage( pPg, nOffset >> 2, nBegin );
            }
            nLast = nBegin + nAlloc - 1;
            nPgs -= nAlloc;
        }
        if( nPgs && !nPasses )
        {
            
            if( !rStrm.SetSize( ( nPages + nPgs ) << 2 ) )
                return STG_EOF;
            if( !bPhys && !InitNew( nPages ) )
                return 0;
                    
                    
            nPages = rStrm.GetSize() >> 2;
            nPasses++;
        }
        else
            break;
    }
    
    if( nBegin == STG_EOF || nPgs )
    {
        rStrm.GetIo().SetError( SVSTREAM_FILEFORMAT_ERROR );
        return STG_EOF; 
    }
    return nOrig;
}





bool StgFAT::InitNew( sal_Int32 nPage1 )
{
    sal_Int32 n = ( ( rStrm.GetSize() >> 2 ) - nPage1 ) / nEntries;
    if ( n > 0 )
    {
        while( n-- )
        {
            rtl::Reference< StgPage > pPg;
            
            
            rStrm.Pos2Page( nPage1 << 2 );
            
            pPg = rStrm.GetIo().Copy( rStrm.GetPage(), STG_FREE );
            if ( !pPg.is() )
                return false;
            for( short i = 0; i < nEntries; i++ )
                rStrm.GetIo().SetToPage( pPg, i, STG_FREE );
            nPage1++;
        }
    }
    return true;
}



bool StgFAT::FreePages( sal_Int32 nStart, bool bAll )
{
    while( nStart >= 0 )
    {
        rtl::Reference< StgPage > pPg = GetPhysPage( nStart << 2 );
        if( !pPg.is() )
            return false;
        nStart = rStrm.GetIo().GetFromPage( pPg, nOffset >> 2 );
        
        rStrm.GetIo().SetToPage( pPg, nOffset >> 2, bAll ? STG_FREE : STG_EOF );
        bAll = true;
    }
    return true;
}







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



void StgStrm::SetEntry( StgDirEntry& r )
{
    r.aEntry.SetLeaf( STG_DATA, nStart );
    r.aEntry.SetSize( nSize );
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

    
    
    std::set< sal_Int32 > nUsedPageNumbers;

    while( nBgn >= 0 && !bError )
    {
        if( nBgn >= 0 )
            m_aPagesCache.push_back(nBgn);
        nBgn = pFat->GetNextPage( nBgn );

        
        if (!nUsedPageNumbers.insert(nBgn).second)
            bError = true;

        nOptSize += nPageSize;
    }
    if (bError)
    {
        if (pOptionalCalcSize)
            rIo.SetError( ERRCODE_IO_WRONGFORMAT );
        m_aPagesCache.clear();
    }
    if (pOptionalCalcSize)
        *pOptionalCalcSize = nOptSize;
}




bool StgStrm::Pos2Page( sal_Int32 nBytePos )
{
    if ( !pFat )
        return false;

    
    if( nBytePos < 0 || nBytePos >= nSize )
        nBytePos = nSize;
    
    nPos -= nOffset;
    sal_Int32 nMask = ~( nPageSize - 1 );
    sal_Int32 nOld = nPos & nMask;
    sal_Int32 nNew = nBytePos & nMask;
    nOffset = (short) ( nBytePos & ~nMask );
    nPos = nBytePos;
    if( nOld == nNew )
        return true;

    
    
    //
    
    
    size_t nIdx = nNew / nPageSize;
    if( nIdx >= m_aPagesCache.size() )
    {
        
        size_t nToAdd = nIdx + 1;

        if (m_aPagesCache.empty())
            m_aPagesCache.push_back( nStart );

        nToAdd -= m_aPagesCache.size();

        sal_Int32 nBgn = m_aPagesCache.back();

        
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
        rIo.SetError( SVSTREAM_FILEFORMAT_ERROR );
        nPage = STG_EOF;
        nOffset = nPageSize;
        return false;
    }
    
    
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



rtl::Reference< StgPage > StgStrm::GetPhysPage( sal_Int32 nBytePos, bool bForce )
{
    if( !Pos2Page( nBytePos ) )
        return NULL;
    return rIo.Get( nPage, bForce );
}




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
        
        if( !nSize || !nBytes )
            pEntry->aEntry.SetLeaf( STG_DATA, nStart );
        pEntry->aEntry.SetSize( nBytes );
        pEntry->SetDirty();
    }
    nSize = nBytes;
    pFat->SetLimit( GetPages() );
    return true;
}



sal_Int32 StgStrm::GetPages() const
{
    return ( nSize + nPageSize - 1 ) / nPageSize;
}







StgFATStrm::StgFATStrm( StgIo& r ) : StgStrm( r )
{
    pFat = new StgFAT( *this, true );
    nSize = rIo.aHdr.GetFATSize() * nPageSize;
}

bool StgFATStrm::Pos2Page( sal_Int32 nBytePos )
{
    
    if( nBytePos < 0 || nBytePos >= nSize  )
        nBytePos = nSize ? nSize - 1 : 0;
    nPage   = nBytePos / nPageSize;
    nOffset = (short) ( nBytePos % nPageSize );
    nPos    = nBytePos;
    nPage   = GetPage( (short) nPage, false );
    return nPage >= 0;
}





rtl::Reference< StgPage > StgFATStrm::GetPhysPage( sal_Int32 nBytePos, bool bForce )
{
    OSL_ENSURE( nBytePos >= 0, "The value may not be negative!" );
    return rIo.Get( nBytePos / ( nPageSize >> 2 ), bForce );
}



sal_Int32 StgFATStrm::GetPage( short nOff, bool bMake, sal_uInt16 *pnMasterAlloc )
{
    OSL_ENSURE( nOff >= 0, "The offset may not be negative!" );
    if( pnMasterAlloc ) *pnMasterAlloc = 0;
    if( nOff < rIo.aHdr.GetFAT1Size() )
        return rIo.aHdr.GetFATPage( nOff );
    sal_Int32 nMaxPage = nSize >> 2;
    nOff = nOff - rIo.aHdr.GetFAT1Size();
    
    sal_uInt16 nMasterCount =  ( nPageSize >> 2 ) - 1;
    sal_uInt16 nBlocks = nOff / nMasterCount;
    
    nOff = nOff % nMasterCount;

    rtl::Reference< StgPage > pOldPage;
    rtl::Reference< StgPage > pMaster;
    sal_Int32 nFAT = rIo.aHdr.GetFATChain();
    for( sal_uInt16 nCount = 0; nCount <= nBlocks; nCount++ )
    {
        if( nFAT == STG_EOF || nFAT == STG_FREE )
        {
            if( bMake )
            {
                m_aPagesCache.clear();

                
                nFAT = nMaxPage++;
                pMaster = rIo.Copy( nFAT, STG_FREE );
                if ( pMaster.is() )
                {
                    for( short k = 0; k < ( nPageSize >> 2 ); k++ )
                        rIo.SetToPage( pMaster, k, STG_FREE );
                    
                    if( !pOldPage.is() )
                        rIo.aHdr.SetFATChain( nFAT );
                    else
                        rIo.SetToPage( pOldPage, nMasterCount, nFAT );
                    if( nMaxPage >= rIo.GetPhysPages() )
                        if( !rIo.SetSize( nMaxPage ) )
                            return STG_EOF;
                    
                    
                    if( !pnMasterAlloc ) 
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
                    rIo.aHdr.SetMasters( nCount + 1 );
                    pOldPage = pMaster;
                }
            }
        }
        else
        {
            pMaster = rIo.Get( nFAT, true );
            if ( pMaster.is() )
            {
                nFAT = rIo.GetFromPage( pMaster, nMasterCount );
                pOldPage = pMaster;
            }
        }
    }
    if( pMaster.is() )
        return rIo.GetFromPage( pMaster, nOff );
    rIo.SetError( SVSTREAM_GENERALERROR );
    return STG_EOF;
}




bool StgFATStrm::SetPage( short nOff, sal_Int32 nNewPage )
{
    OSL_ENSURE( nOff >= 0, "The offset may not be negative!" );
    m_aPagesCache.clear();

    bool bRes = true;
    if( nOff < rIo.aHdr.GetFAT1Size() )
        rIo.aHdr.SetFATPage( nOff, nNewPage );
    else
    {
        nOff = nOff - rIo.aHdr.GetFAT1Size();
        
        sal_uInt16 nMasterCount =  ( nPageSize >> 2 ) - 1;
        sal_uInt16 nBlocks = nOff / nMasterCount;
        
        nOff = nOff % nMasterCount;

        rtl::Reference< StgPage > pMaster;
        sal_Int32 nFAT = rIo.aHdr.GetFATChain();
        for( sal_uInt16 nCount = 0; nCount <= nBlocks; nCount++ )
        {
            if( nFAT == STG_EOF || nFAT == STG_FREE )
            {
                pMaster = 0;
                break;
            }
            pMaster = rIo.Get( nFAT, true );
            if ( pMaster.is() )
                nFAT = rIo.GetFromPage( pMaster, nMasterCount );
        }
        if( pMaster.is() )
            rIo.SetToPage( pMaster, nOff, nNewPage );
        else
        {
            rIo.SetError( SVSTREAM_GENERALERROR );
            bRes = false;
        }
    }

    
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

    
    short nOld = (short) ( ( nSize + ( nPageSize - 1 ) ) / nPageSize );
    short nNew = (short) (
        ( nBytes + ( nPageSize - 1 ) ) / nPageSize ) ;
    if( nNew < nOld )
    {
        
        for( short i = nNew; i < nOld; i++ )
            SetPage( i, STG_FREE );
    }
    else
    {
        while( nOld < nNew )
        {
            
            
            sal_Int32 nPg = 0;
            sal_uInt16 nMasterAlloc = 0;
            nPg = GetPage( nOld, true, &nMasterAlloc );
            if( nPg == STG_EOF )
                return false;
            
            nBytes += nMasterAlloc << 2;

            
            sal_Int32 n = 1;
            OSL_ENSURE( pFat, "The pointer is always initializer here!" );
            sal_Int32 nNewPage = pFat->FindBlock( n );
            if( nNewPage == STG_EOF )
            {
                
                
                
                nNewPage = nSize >> 2;
                
                
                nNewPage += nMasterAlloc;
                
                if( nNewPage >= rIo.GetPhysPages() )
                    if( !rIo.SetSize( nNewPage + 1 ) )
                        return false;
            }
            
            rtl::Reference< StgPage > pPg = rIo.Copy( nNewPage, STG_FREE );
            if ( !pPg.is() )
                return false;
            for( short j = 0; j < ( nPageSize >> 2 ); j++ )
                rIo.SetToPage( pPg, j, STG_FREE );

            
            
            nSize = ( nOld + 1 ) * nPageSize;
            SetPage( nOld, nNewPage );

            

            sal_uInt32 nMax = rIo.aHdr.GetMasters( );
            sal_uInt32 nFAT = rIo.aHdr.GetFATChain();
            if( nMasterAlloc )
                for( sal_uInt16 nCount = 0; nCount < nMax; nCount++ )
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
                    nFAT = rIo.GetFromPage( pPage, (nPageSize >> 2 ) - 1 );
                }

            nOld++;
            
            nBytes += 4;
            nNew = (short) (
                ( nBytes + ( nPageSize - 1 ) ) / nPageSize );
        }
    }
    nSize = nNew * nPageSize;
    rIo.aHdr.SetFATSize( nNew );
    return true;
}








StgDataStrm::StgDataStrm( StgIo& r, sal_Int32 nBgn, sal_Int32 nLen ) : StgStrm( r )
{
    Init( nBgn, nLen );
}

StgDataStrm::StgDataStrm( StgIo& r, StgDirEntry& p ) : StgStrm( r )
{
    pEntry = &p;
    Init( p.aEntry.GetLeaf( STG_DATA ),
          p.aEntry.GetSize() );
}

void StgDataStrm::Init( sal_Int32 nBgn, sal_Int32 nLen )
{
    if ( rIo.pFAT )
        pFat = new StgFAT( *rIo.pFAT, true );

    OSL_ENSURE( pFat, "The pointer should not be empty!" );

    nStart = nPage = nBgn;
    nSize  = nLen;
    nIncr  = 1;
    nOffset = 0;
    if( nLen < 0 && pFat )
    {
        
        
        scanBuildPageChainCache( &nSize );
    }
}



bool StgDataStrm::SetSize( sal_Int32 nBytes )
{
    if ( !pFat )
        return false;

    nBytes = ( ( nBytes + nIncr - 1 ) / nIncr ) * nIncr;
    sal_Int32 nOldSz = nSize;
    if( ( nOldSz != nBytes ) )
    {
        if( !StgStrm::SetSize( nBytes ) )
            return false;
        sal_Int32 nMaxPage = pFat->GetMaxPage();
        if( nMaxPage > rIo.GetPhysPages() )
            if( !rIo.SetSize( nMaxPage ) )
                return false;
        
        
        
        if( ( nSize - 1 )  / nPageSize - ( nOldSz - 1 ) / nPageSize == 1 )
        {
            Pos2Page( nBytes );
            if( nPage >= 0 )
                rIo.Copy( nPage, STG_FREE );
        }
    }
    return true;
}





void* StgDataStrm::GetPtr( sal_Int32 Pos, bool bForce, bool bDirty )
{
    if( Pos2Page( Pos ) )
    {
        rtl::Reference< StgPage > pPg = rIo.Get( nPage, bForce );
        if (pPg.is() && nOffset < pPg->GetSize())
        {
            if( bDirty )
                rIo.SetDirty( pPg );
            return ((sal_uInt8 *)pPg->GetData()) + nOffset;
        }
    }
    return NULL;
}





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
            void *p = (sal_uInt8 *) pBuf + nDone;
            if( nBytes == nPageSize )
            {
                pPg = rIo.Find( nPage );
                if( pPg.is() )
                {
                    
                    memcpy( p, pPg->GetData(), nBytes );
                    nRes = nBytes;
                }
                else
                    
                    nRes = (short) rIo.Read( nPage, p, 1 ) * nPageSize;
            }
            else
            {
                
                pPg = rIo.Get( nPage, false );
                if( !pPg.is() )
                    break;
                memcpy( p, (sal_uInt8*)pPg->GetData() + nOffset, nBytes );
                nRes = nBytes;
            }
            nDone += nRes;
            nPos += nRes;
            n -= nRes;
            nOffset = nOffset + nRes;
            if( nRes != nBytes )
                break;  
        }
        
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
            const void *p = (const sal_uInt8 *) pBuf + nDone;
            if( nBytes == nPageSize )
            {
                pPg = rIo.Find( nPage );
                if( pPg.is() )
                {
                    
                    memcpy( pPg->GetData(), p, nBytes );
                    rIo.SetDirty( pPg );
                    nRes = nBytes;
                }
                else
                    
                    nRes = (short) rIo.Write( nPage, (void*) p, 1 ) * nPageSize;
            }
            else
            {
                
                pPg = rIo.Get( nPage, false );
                if( !pPg.is() )
                    break;
                memcpy( (sal_uInt8*)pPg->GetData() + nOffset, p, nBytes );
                rIo.SetDirty( pPg );
                nRes = nBytes;
            }
            nDone += nRes;
            nPos += nRes;
            n -= nRes;
            nOffset = nOffset + nRes;
            if( nRes != nBytes )
                break;  
        }
        
        if( nOffset >= nPageSize && !Pos2Page( nPos ) )
            break;
    }
    return nDone;
}








StgSmallStrm::StgSmallStrm( StgIo& r, sal_Int32 nBgn, sal_Int32 nLen ) : StgStrm( r )
{
    Init( nBgn, nLen );
}

StgSmallStrm::StgSmallStrm( StgIo& r, StgDirEntry& p ) : StgStrm( r )
{
    pEntry = &p;
    Init( p.aEntry.GetLeaf( STG_DATA ),
          p.aEntry.GetSize() );
}

void StgSmallStrm::Init( sal_Int32 nBgn, sal_Int32 nLen )
{
    if ( rIo.pDataFAT )
        pFat = new StgFAT( *rIo.pDataFAT, false );
    pData = rIo.pDataStrm;
    OSL_ENSURE( pFat && pData, "The pointers should not be empty!" );

    nPageSize = rIo.GetDataPageSize();
    nStart =
    nPage  = nBgn;
    nSize  = nLen;
}





sal_Int32 StgSmallStrm::Read( void* pBuf, sal_Int32 n )
{
    
    
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
            if( !pData || !pData->Pos2Page( nPage * nPageSize + nOffset ) )
                break;
            
            short nRes = (short) pData->Read( (sal_uInt8*)pBuf + nDone, nBytes );
            nDone = nDone + nRes;
            nPos += nRes;
            n -= nRes;
            nOffset = nOffset + nRes;
            
            if( nRes != nBytes )
                break;
        }
        
        if( nOffset >= nPageSize && !Pos2Page( nPos ) )
            break;
    }
    return nDone;
}

sal_Int32 StgSmallStrm::Write( const void* pBuf, sal_Int32 n )
{
    
    
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
            
            sal_Int32 nDataPos = nPage * nPageSize + nOffset;
            if ( !pData
              || ( pData->GetSize() < ( nDataPos + nBytes )
                && !pData->SetSize( nDataPos + nBytes ) ) )
                break;
            if( !pData->Pos2Page( nDataPos ) )
                break;
            short nRes = (short) pData->Write( (sal_uInt8*)pBuf + nDone, nBytes );
            nDone = nDone + nRes;
            nPos += nRes;
            n -= nRes;
            nOffset = nOffset + nRes;
            
            if( nRes != nBytes )
                break;
        }
        
        if( nOffset >= nPageSize && !Pos2Page( nPos ) )
            break;
    }
    return nDone;
}






#define THRESHOLD 32768L

StgTmpStrm::StgTmpStrm( sal_uLong nInitSize )
          : SvMemoryStream( nInitSize > THRESHOLD
                              ? 16
                            : ( nInitSize ? nInitSize : 16 ), 4096 )
{
    pStrm = NULL;
    
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
        sal_uInt8* p = new sal_uInt8[ 4096 ];
        rSrc.Seek( 0L );
        Seek( 0L );
        while( n )
        {
            sal_uLong nn = n;
            if( nn > 4096 )
                nn = 4096;
            if( rSrc.Read( p, nn ) != nn )
                break;
            if( Write( p, nn ) != nn )
                break;
            n -= nn;
        }
        delete [] p;
        rSrc.Seek( nCur );
        Seek( nCur );
        return n == 0;
    }
    else
        return false;
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

sal_uLong StgTmpStrm::GetSize() const
{
    sal_uLong n;
    if( pStrm )
    {
        sal_uLong old = pStrm->Tell();
        n = pStrm->Seek( STREAM_SEEK_TO_END );
        pStrm->Seek( old );
    }
    else
        n = nEndOfData;
    return n;
}

void StgTmpStrm::SetSize( sal_uLong n )
{
    if( pStrm )
        pStrm->SetStreamSize( n );
    else
    {
        if( n > THRESHOLD )
        {
            aName = utl::TempFile::CreateTempName();
            SvFileStream* s = new SvFileStream( aName, STREAM_READWRITE );
            sal_uLong nCur = Tell();
            sal_uLong i = nEndOfData;
            if( i )
            {
                sal_uInt8* p = new sal_uInt8[ 4096 ];
                Seek( 0L );
                while( i )
                {
                    sal_uLong nb = ( i > 4096 ) ? 4096 : i;
                    if( Read( p, nb ) == nb
                     && s->Write( p, nb ) == nb )
                        i -= nb;
                    else
                        break;
                }
                delete [] p;
            }
            if( !i && n > nEndOfData )
            {
                
                
                
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
            
            ReAllocateMemory( - ( (long) nEndOfData - 16 ) );
        }
        else
        {
            if( n > nEndOfData )
            {
                sal_uLong nCur = Tell();
                Seek( nEndOfData - 1 );
                WriteUChar( (sal_uInt8) 0 );
                Seek( nCur );
            }
            else
                nEndOfData = n;
        }
    }
}

sal_uLong StgTmpStrm::GetData( void* pData, sal_uLong n )
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

sal_uLong StgTmpStrm::PutData( const void* pData, sal_uLong n )
{
    sal_uInt32 nCur = Tell();
    sal_uInt32 nNew = nCur + n;
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

sal_uLong StgTmpStrm::SeekPos( sal_uLong n )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
