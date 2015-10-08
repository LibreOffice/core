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


#include "sot/stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"
#include <rtl/instance.hxx>

///////////////////////////// class StgIo

// This class holds the storage header and all internal streams.

StgIo::StgIo() : StgCache()
{
    m_pTOC      = NULL;
    m_pDataFAT  = NULL;
    m_pDataStrm = NULL;
    m_pFAT      = NULL;
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
    if( m_pStrm )
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
    m_pTOC      = NULL;
    m_pDataFAT  = NULL;
    m_pDataStrm = NULL;
    m_pFAT      = NULL;
    ResetError();
    SetPhysPageSize( 1 << m_aHdr.GetPageSize() );
    m_pFAT = new StgFATStrm( *this );
    m_pTOC = new StgDirStrm( *this );
    if( !GetError() )
    {
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
}

// get the logical data page size

short StgIo::GetDataPageSize()
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
                m_pStrm->Flush();
                sal_uLong n = m_pStrm->GetError();
                SetError( n );
#ifdef DBG_UTIL
                if( n==0 ) ValidateFATs();
#endif
                return n == 0;
            }
        }
    }
    SetError( SVSTREAM_WRITE_ERROR );
    return false;
}


class EasyFat
{
    sal_Int32 *pFat;
    bool  *pFree;
    sal_Int32 nPages;
    sal_Int32 nPageSize;

public:
    EasyFat( StgIo & rIo, StgStrm *pFatStream, sal_Int32 nPSize );
    ~EasyFat() { delete[] pFat; delete[] pFree; }

    sal_Int32 GetPageSize() { return nPageSize; }

    sal_uLong Mark( sal_Int32 nPage, sal_Int32 nCount, sal_Int32 nExpect );
    bool HasUnrefChains();
};

EasyFat::EasyFat( StgIo& rIo, StgStrm* pFatStream, sal_Int32 nPSize )
{
    nPages = pFatStream->GetSize() >> 2;
    nPageSize = nPSize;
    pFat = new sal_Int32[ nPages ];
    pFree = new bool[ nPages ];

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

bool EasyFat::HasUnrefChains()
{
    for( sal_Int32 nPage = 0; nPage < nPages; nPage++ )
    {
        if( pFree[ nPage ] && pFat[ nPage ] != -1 )
            return true;
    }
    return false;
}

sal_uLong EasyFat::Mark( sal_Int32 nPage, sal_Int32 nCount, sal_Int32 nExpect )
{
    if( nCount > 0 )
        --nCount /= GetPageSize(), nCount++;

    sal_Int32 nCurPage = nPage;
    while( nCount != 0 )
    {
        if( nCurPage < 0 || nCurPage >= nPages )
            return FAT_OUTOFBOUNDS;
        pFree[ nCurPage ] = false;
        nCurPage = pFat[ nCurPage ];
        //Stream zu lang
        if( nCurPage != nExpect && nCount == 1 )
            return FAT_WRONGLENGTH;
        //Stream zu kurz
        if( nCurPage == nExpect && nCount != 1 && nCount != -1 )
            return FAT_WRONGLENGTH;
        // letzter Block bei Stream ohne Laenge
        if( nCurPage == nExpect && nCount == -1 )
            nCount = 1;
        if( nCount != -1 )
            nCount--;
    }
    return FAT_OK;
}

class Validator
{
    sal_uLong nError;

    EasyFat aSmallFat;
    EasyFat aFat;

    StgIo &rIo;

    sal_uLong ValidateMasterFATs();
    sal_uLong ValidateDirectoryEntries();
    sal_uLong FindUnrefedChains();
    sal_uLong MarkAll( StgDirEntry *pEntry );

public:
    explicit Validator( StgIo &rIo );
    bool IsError() { return nError != 0; }
};

Validator::Validator( StgIo &rIoP )
    : aSmallFat( rIoP, rIoP.m_pDataFAT, 1 << rIoP.m_aHdr.GetDataPageSize() ),
      aFat( rIoP, rIoP.m_pFAT, 1 << rIoP.m_aHdr.GetPageSize() ),
      rIo( rIoP )
{
    sal_uLong nErr = nError = FAT_OK;

    if( ( nErr = ValidateMasterFATs() ) != FAT_OK )
        nError = nErr;
    else if(    ( nErr = ValidateDirectoryEntries() ) != FAT_OK )
        nError = nErr;
    else if(    ( nErr = FindUnrefedChains()) != FAT_OK )
        nError = nErr;
}

sal_uLong Validator::ValidateMasterFATs()
{
    sal_Int32 nCount = rIo.m_aHdr.GetFATSize();
    sal_uLong nErr;
    if ( !rIo.m_pFAT )
        return FAT_INMEMORYERROR;

    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        if( ( nErr = aFat.Mark(rIo.m_pFAT->GetPage( short(i), false ), aFat.GetPageSize(), -3 )) != FAT_OK )
            return nErr;
    }
    if( rIo.m_aHdr.GetMasters() )
        if( ( nErr = aFat.Mark(rIo.m_aHdr.GetFATChain( ), aFat.GetPageSize(), -4 )) != FAT_OK )
            return nErr;

    return FAT_OK;
}

sal_uLong Validator::MarkAll( StgDirEntry *pEntry )
{
    if ( !pEntry )
        return FAT_INMEMORYERROR;

    StgIterator aIter( *pEntry );
    sal_uLong nErr = FAT_OK;
    for( StgDirEntry* p = aIter.First(); p ; p = aIter.Next() )
    {
        if( p->m_aEntry.GetType() == STG_STORAGE )
        {
            nErr = MarkAll( p );
            if( nErr != FAT_OK )
                return nErr;
        }
        else
        {
            sal_Int32 nSize = p->m_aEntry.GetSize();
            if( nSize < rIo.m_aHdr.GetThreshold()  )
                nErr = aSmallFat.Mark( p->m_aEntry.GetStartPage(),nSize, -2 );
            else
                nErr = aFat.Mark( p->m_aEntry.GetStartPage(),nSize, -2 );
            if( nErr != FAT_OK )
                return nErr;
        }
    }
    return FAT_OK;
}

sal_uLong Validator::ValidateDirectoryEntries()
{
    if ( !rIo.m_pTOC )
        return FAT_INMEMORYERROR;

    // Normale DirEntries
    sal_uLong nErr = MarkAll( rIo.m_pTOC->GetRoot() );
    if( nErr != FAT_OK )
        return nErr;
    // Small Data
    nErr = aFat.Mark( rIo.m_pTOC->GetRoot()->m_aEntry.GetStartPage(),
                 rIo.m_pTOC->GetRoot()->m_aEntry.GetSize(), -2 );
    if( nErr != FAT_OK )
        return nErr;
    // Small Data FAT
    nErr = aFat.Mark(
        rIo.m_aHdr.GetDataFATStart(),
        rIo.m_aHdr.GetDataFATSize() * aFat.GetPageSize(), -2 );
    if( nErr != FAT_OK )
        return nErr;
    // TOC
    nErr = aFat.Mark(
        rIo.m_aHdr.GetTOCStart(), -1, -2 );
    return nErr;
}

sal_uLong Validator::FindUnrefedChains()
{
    if( aSmallFat.HasUnrefChains() ||
        aFat.HasUnrefChains() )
        return FAT_UNREFCHAIN;
    else
        return FAT_OK;
}

namespace { struct ErrorLink : public rtl::Static<Link<StgLinkArg&,void>, ErrorLink > {}; }

void StgIo::SetErrorLink( const Link<StgLinkArg&,void>& rLink )
{
    ErrorLink::get() = rLink;
}

const Link<StgLinkArg&,void>& StgIo::GetErrorLink()
{
    return ErrorLink::get();
}

sal_uLong StgIo::ValidateFATs()
{
    if( m_bFile )
    {
        Validator *pV = new Validator( *this );
        bool bRet1 = !pV->IsError(), bRet2 = true ;
        delete pV;

        SvFileStream *pFileStrm = static_cast<SvFileStream *>( GetStrm() );
        if ( !pFileStrm )
            return FAT_INMEMORYERROR;

        StgIo aIo;
        if( aIo.Open( pFileStrm->GetFileName(),
                      StreamMode::READ | StreamMode::SHARE_DENYNONE) &&
            aIo.Load() )
        {
            pV = new Validator( aIo );
            bRet2 = !pV->IsError();
            delete pV;
        }

        sal_uLong nErr;
        if( bRet1 != bRet2 )
            nErr = bRet1 ? FAT_ONFILEERROR : FAT_INMEMORYERROR;
        else nErr = bRet1 ? FAT_OK : FAT_BOTHERROR;
        if( nErr != FAT_OK && !m_bCopied )
        {
            StgLinkArg aArg;
            aArg.aFile = pFileStrm->GetFileName();
            aArg.nErr = nErr;
            ErrorLink::get().Call( aArg );
            m_bCopied = true;
        }
//      DBG_ASSERT( nErr == FAT_OK ,"Storage kaputt");
        return nErr;
    }
//  OSL_FAIL("Validiere nicht (kein FileStorage)");
    return FAT_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
