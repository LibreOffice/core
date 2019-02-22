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


#include <sot/stg.hxx>
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"
#include <o3tl/safeint.hxx>
#include <rtl/instance.hxx>
#include <sal/log.hxx>

#include <memory>

///////////////////////////// class StgIo

// This class holds the storage header and all internal streams.

StgIo::StgIo() : StgCache()
{
    m_pTOC      = nullptr;
    m_pDataFAT  = nullptr;
    m_pDataStrm = nullptr;
    m_pFAT      = nullptr;
    m_bCopied   = false;
}

StgIo::~StgIo()
{
    delete m_pTOC;
    delete m_pDataFAT;
    delete m_pDataStrm;
    delete m_pFAT;
}

// Load the header. Do not set an error code if the header is invalid.

bool StgIo::Load()
{
    if( GetStrm() )
    {
        if( m_aHdr.Load( *this ) )
        {
            if( m_aHdr.Check() )
                SetupStreams();
            else
                return false;
        }
        else
            return false;
    }
    return Good();
}

// Set up an initial, empty storage

bool StgIo::Init()
{
    m_aHdr.Init();
    SetupStreams();
    return CommitAll();
}

void StgIo::SetupStreams()
{
    delete m_pTOC;
    delete m_pDataFAT;
    delete m_pDataStrm;
    delete m_pFAT;
    m_pTOC      = nullptr;
    m_pDataFAT  = nullptr;
    m_pDataStrm = nullptr;
    m_pFAT      = nullptr;
    ResetError();

    short nPhysPageSize = 1 << m_aHdr.GetPageSize();
    SetPhysPageSize(nPhysPageSize);
    sal_Int32 nFatStrmSize;
    if (o3tl::checked_multiply<sal_Int32>(m_aHdr.GetFATSize(), nPhysPageSize, nFatStrmSize))
    {
        SAL_WARN("sot", "Error: " << m_aHdr.GetFATSize() << " * " << nPhysPageSize << " would overflow");
        SetError(SVSTREAM_FILEFORMAT_ERROR);
        m_pFAT = nullptr;
        m_pTOC = nullptr;
        return;
    }

    m_pFAT = new StgFATStrm(*this, nFatStrmSize);
    m_pTOC = new StgDirStrm(*this);
    if( GetError() )
        return;

    StgDirEntry* pRoot = m_pTOC->GetRoot();
    if( pRoot )
    {
        m_pDataFAT = new StgDataStrm( *this, m_aHdr.GetDataFATStart(), -1 );
        m_pDataStrm = new StgDataStrm( *this, *pRoot );
        m_pDataFAT->SetIncrement( 1 << m_aHdr.GetPageSize() );
        m_pDataStrm->SetIncrement( GetDataPageSize() );
        m_pDataStrm->SetEntry( *pRoot );
    }
    else
        SetError( SVSTREAM_FILEFORMAT_ERROR );
}

// get the logical data page size

short StgIo::GetDataPageSize() const
{
    return 1 << m_aHdr.GetDataPageSize();
}

// Commit everything

bool StgIo::CommitAll()
{
    // Store the data (all streams and the TOC)
    if( m_pTOC && m_pTOC->Store() && m_pDataFAT )
    {
        if( Commit() )
        {
            m_aHdr.SetDataFATStart( m_pDataFAT->GetStart() );
            m_aHdr.SetDataFATSize( m_pDataFAT->GetPages() );
            m_aHdr.SetTOCStart( m_pTOC->GetStart() );
            if( m_aHdr.Store( *this ) )
            {
                GetStrm()->Flush();
                const ErrCode n = GetStrm()->GetError();
                SetError( n );
#ifdef DBG_UTIL
                if( n==ERRCODE_NONE ) ValidateFATs();
#endif
                return n == ERRCODE_NONE;
            }
        }
    }
    SetError( SVSTREAM_WRITE_ERROR );
    return false;
}


class EasyFat
{
    std::unique_ptr<sal_Int32[]> pFat;
    std::unique_ptr<bool[]> pFree;
    sal_Int32 nPages;
    sal_Int32 nPageSize;

public:
    EasyFat( StgIo & rIo, StgStrm *pFatStream, sal_Int32 nPSize );

    sal_Int32 GetPageSize() const { return nPageSize; }

    FatError Mark( sal_Int32 nPage, sal_Int32 nCount, sal_Int32 nExpect );
    bool HasUnrefChains() const;
};

EasyFat::EasyFat( StgIo& rIo, StgStrm* pFatStream, sal_Int32 nPSize )
{
    nPages = pFatStream->GetSize() >> 2;
    nPageSize = nPSize;
    pFat.reset( new sal_Int32[ nPages ] );
    pFree.reset( new bool[ nPages ] );

    rtl::Reference< StgPage > pPage;
    sal_Int32 nFatPageSize = (1 << rIo.m_aHdr.GetPageSize()) - 2;

    for( sal_Int32 nPage = 0; nPage < nPages; nPage++ )
    {
        if( ! (nPage % nFatPageSize) )
        {
            pFatStream->Pos2Page( nPage << 2 );
            sal_Int32 nPhysPage = pFatStream->GetPage();
            pPage = rIo.Get( nPhysPage, true );
        }

        pFat[ nPage ] = StgCache::GetFromPage( pPage, short( nPage % nFatPageSize ) );
        pFree[ nPage ] = true;
    }
}

bool EasyFat::HasUnrefChains() const
{
    for( sal_Int32 nPage = 0; nPage < nPages; nPage++ )
    {
        if( pFree[ nPage ] && pFat[ nPage ] != -1 )
            return true;
    }
    return false;
}

FatError EasyFat::Mark( sal_Int32 nPage, sal_Int32 nCount, sal_Int32 nExpect )
{
    if( nCount > 0 )
    {
        --nCount;
        nCount /= GetPageSize();
        ++nCount;
    }

    sal_Int32 nCurPage = nPage;
    while( nCount != 0 )
    {
        if( nCurPage < 0 || nCurPage >= nPages )
            return FatError::OutOfBounds;
        pFree[ nCurPage ] = false;
        nCurPage = pFat[ nCurPage ];
        // stream too long
        if( nCurPage != nExpect && nCount == 1 )
            return FatError::WrongLength;
        // stream too short
        if( nCurPage == nExpect && nCount != 1 && nCount != -1 )
            return FatError::WrongLength;
        // last block for stream without length
        if( nCurPage == nExpect && nCount == -1 )
            nCount = 1;
        if( nCount != -1 )
            nCount--;
    }
    return FatError::Ok;
}

class Validator
{
    FatError nError;

    EasyFat aSmallFat;
    EasyFat aFat;

    StgIo &rIo;

    FatError ValidateMasterFATs();
    FatError ValidateDirectoryEntries();
    FatError FindUnrefedChains() const;
    FatError MarkAll( StgDirEntry *pEntry );

public:
    explicit Validator( StgIo &rIo );
    bool IsError() const { return nError != FatError::Ok; }
};

Validator::Validator( StgIo &rIoP )
    : aSmallFat( rIoP, rIoP.m_pDataFAT, 1 << rIoP.m_aHdr.GetDataPageSize() ),
      aFat( rIoP, rIoP.m_pFAT, 1 << rIoP.m_aHdr.GetPageSize() ),
      rIo( rIoP )
{
    FatError nErr = nError = FatError::Ok;

    if( ( nErr = ValidateMasterFATs() ) != FatError::Ok )
        nError = nErr;
    else if(    ( nErr = ValidateDirectoryEntries() ) != FatError::Ok )
        nError = nErr;
    else if(    ( nErr = FindUnrefedChains()) != FatError::Ok )
        nError = nErr;
}

FatError Validator::ValidateMasterFATs()
{
    sal_Int32 nCount = rIo.m_aHdr.GetFATSize();
    FatError nErr;
    if ( !rIo.m_pFAT )
        return FatError::InMemoryError;

    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        if( ( nErr = aFat.Mark(rIo.m_pFAT->GetPage(i, false), aFat.GetPageSize(), -3 )) != FatError::Ok)
            return nErr;
    }
    if( rIo.m_aHdr.GetMasters() )
        if( ( nErr = aFat.Mark(rIo.m_aHdr.GetFATChain( ), aFat.GetPageSize(), -4 )) != FatError::Ok )
            return nErr;

    return FatError::Ok;
}

FatError Validator::MarkAll( StgDirEntry *pEntry )
{
    if ( !pEntry )
        return FatError::InMemoryError;

    StgIterator aIter( *pEntry );
    FatError nErr = FatError::Ok;
    for( StgDirEntry* p = aIter.First(); p ; p = aIter.Next() )
    {
        if( p->m_aEntry.GetType() == STG_STORAGE )
        {
            nErr = MarkAll( p );
            if( nErr != FatError::Ok )
                return nErr;
        }
        else
        {
            sal_Int32 nSize = p->m_aEntry.GetSize();
            if( nSize < rIo.m_aHdr.GetThreshold()  )
                nErr = aSmallFat.Mark( p->m_aEntry.GetStartPage(),nSize, -2 );
            else
                nErr = aFat.Mark( p->m_aEntry.GetStartPage(),nSize, -2 );
            if( nErr != FatError::Ok )
                return nErr;
        }
    }
    return FatError::Ok;
}

FatError Validator::ValidateDirectoryEntries()
{
    if ( !rIo.m_pTOC )
        return FatError::InMemoryError;

    // Normal DirEntries
    FatError nErr = MarkAll( rIo.m_pTOC->GetRoot() );
    if( nErr != FatError::Ok )
        return nErr;
    // Small Data
    nErr = aFat.Mark( rIo.m_pTOC->GetRoot()->m_aEntry.GetStartPage(),
                 rIo.m_pTOC->GetRoot()->m_aEntry.GetSize(), -2 );
    if( nErr != FatError::Ok )
        return nErr;
    // Small Data FAT
    nErr = aFat.Mark(
        rIo.m_aHdr.GetDataFATStart(),
        rIo.m_aHdr.GetDataFATSize() * aFat.GetPageSize(), -2 );
    if( nErr != FatError::Ok )
        return nErr;
    // TOC
    nErr = aFat.Mark(
        rIo.m_aHdr.GetTOCStart(), -1, -2 );
    return nErr;
}

FatError Validator::FindUnrefedChains() const
{
    if( aSmallFat.HasUnrefChains() ||
        aFat.HasUnrefChains() )
        return FatError::UnrefChain;
    else
        return FatError::Ok;
}

FatError StgIo::ValidateFATs()
{
    if( m_bFile )
    {
        std::unique_ptr<Validator> pV(new Validator( *this ));
        bool bRet1 = !pV->IsError(), bRet2 = true ;
        pV.reset();

        SvFileStream *pFileStrm = static_cast<SvFileStream *>( GetStrm() );
        if ( !pFileStrm )
            return FatError::InMemoryError;

        StgIo aIo;
        if( aIo.Open( pFileStrm->GetFileName(),
                      StreamMode::READ | StreamMode::SHARE_DENYNONE) &&
            aIo.Load() )
        {
            pV.reset(new Validator( aIo ));
            bRet2 = !pV->IsError();
            pV.reset();
        }

        FatError nErr;
        if( bRet1 != bRet2 )
            nErr = bRet1 ? FatError::OnFileError : FatError::InMemoryError;
        else nErr = bRet1 ? FatError::Ok : FatError::BothError;
        if( nErr != FatError::Ok && !m_bCopied )
        {
            m_bCopied = true;
        }
//      DBG_ASSERT( nErr == FatError::Ok ,"Storage broken");
        return nErr;
    }
//  OSL_FAIL("Do not validate (no FileStorage)");
    return FatError::Ok;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
