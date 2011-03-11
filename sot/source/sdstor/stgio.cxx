/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sot.hxx"

#include "sot/stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"
#include <rtl/instance.hxx>

///////////////////////////// class StgIo //////////////////////////////

// This class holds the storage header and all internal streams.

StgIo::StgIo() : StgCache()
{
    pTOC      = NULL;
    pDataFAT  = NULL;
    pDataStrm = NULL;
    pFAT      = NULL;
    bCopied   = sal_False;
}

StgIo::~StgIo()
{
    delete pTOC;
    delete pDataFAT;
    delete pDataStrm;
    delete pFAT;
}

// Load the header. Do not set an error code if the header is invalid.

sal_Bool StgIo::Load()
{
    if( pStrm )
    {
        if( aHdr.Load( *this ) )
        {
            if( aHdr.Check() )
                SetupStreams();
            else
                return sal_False;
        }
    }
    return Good();
}

// Set up an initial, empty storage

sal_Bool StgIo::Init()
{
    aHdr.Init();
    SetupStreams();
    return CommitAll();
}

void StgIo::SetupStreams()
{
    delete pTOC;
    delete pDataFAT;
    delete pDataStrm;
    delete pFAT;
    pTOC      = NULL;
    pDataFAT  = NULL;
    pDataStrm = NULL;
    pFAT      = NULL;
    ResetError();
    SetPhysPageSize( 1 << aHdr.GetPageSize() );
    pFAT = new StgFATStrm( *this );
    pTOC = new StgDirStrm( *this );
    if( !GetError() )
    {
        StgDirEntry* pRoot = pTOC->GetRoot();
        if( pRoot )
        {
            pDataFAT = new StgDataStrm( *this, aHdr.GetDataFATStart(), -1 );
            pDataStrm = new StgDataStrm( *this, pRoot );
            pDataFAT->SetIncrement( 1 << aHdr.GetPageSize() );
            pDataStrm->SetIncrement( GetDataPageSize() );
            pDataStrm->SetEntry( *pRoot );
        }
        else
            SetError( SVSTREAM_FILEFORMAT_ERROR );
    }
}

// get the logical data page size

short StgIo::GetDataPageSize()
{
    return 1 << aHdr.GetDataPageSize();
}

// Commit everything

sal_Bool StgIo::CommitAll()
{
    // Store the data (all streams and the TOC)
    if( pTOC->Store() )
    {
        if( Commit( NULL ) )
        {
            aHdr.SetDataFATStart( pDataFAT->GetStart() );
            aHdr.SetDataFATSize( pDataFAT->GetPages() );
            aHdr.SetTOCStart( pTOC->GetStart() );
            if( aHdr.Store( *this ) )
            {
                pStrm->Flush();
                sal_uLong n = pStrm->GetError();
                SetError( n );
#ifdef DBG_UTIL
                if( n==0 ) ValidateFATs();
#endif
                return sal_Bool( n == 0 );
            }
        }
    }
    SetError( SVSTREAM_WRITE_ERROR );
    return sal_False;
}


class EasyFat
{
    sal_Int32 *pFat;
    sal_Bool  *pFree;
    sal_Int32 nPages;
    sal_Int32 nPageSize;

public:
    EasyFat( StgIo & rIo, StgStrm *pFatStream, sal_Int32 nPSize );
    ~EasyFat() { delete[] pFat; delete[] pFree; }
    sal_Int32 GetPageSize() { return nPageSize; }
    sal_Int32 Count() { return nPages; }
    sal_Int32 operator[]( sal_Int32 nOffset ) { return pFat[ nOffset ]; }

    sal_uLong Mark( sal_Int32 nPage, sal_Int32 nCount, sal_Int32 nExpect );
    sal_Bool HasUnrefChains();
};

EasyFat::EasyFat( StgIo& rIo, StgStrm* pFatStream, sal_Int32 nPSize )
{
    nPages = pFatStream->GetSize() >> 2;
    nPageSize = nPSize;
    pFat = new sal_Int32[ nPages ];
    pFree = new sal_Bool[ nPages ];

    StgPage *pPage = NULL;
    sal_Int32 nFatPageSize = (1 << rIo.aHdr.GetPageSize()) - 2;

    for( sal_Int32 nPage = 0; nPage < nPages; nPage++ )
    {
        if( ! (nPage % nFatPageSize) )
        {
            pFatStream->Pos2Page( nPage << 2 );
            sal_Int32 nPhysPage = pFatStream->GetPage();
            pPage = rIo.Get( nPhysPage, sal_True );
        }

        pFat[ nPage ] = pPage->GetPage( short( nPage % nFatPageSize ) );
        pFree[ nPage ] = sal_True;
    }
}

sal_Bool EasyFat::HasUnrefChains()
{
    for( sal_Int32 nPage = 0; nPage < nPages; nPage++ )
    {
        if( pFree[ nPage ] && pFat[ nPage ] != -1 )
            return sal_True;
    }
    return sal_False;
}

sal_uLong EasyFat::Mark( sal_Int32 nPage, sal_Int32 nCount, sal_Int32 nExpect )
{
    if( nCount > 0 )
        --nCount /= GetPageSize(), nCount++;

    sal_Int32 nCurPage = nPage;
    while( nCount != 0 )
    {
        pFree[ nCurPage ] = sal_False;
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
        // Naechster Block nicht in der FAT
        if( nCount && ( nCurPage < 0 || nCurPage >= nPages ) )
            return FAT_OUTOFBOUNDS;
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

    Validator( StgIo &rIo );
    sal_Bool IsError() { return nError != 0; }
};

Validator::Validator( StgIo &rIoP )
    : aSmallFat( rIoP, rIoP.pDataFAT, 1 << rIoP.aHdr.GetDataPageSize() ),
      aFat( rIoP, rIoP.pFAT, 1 << rIoP.aHdr.GetPageSize() ),
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
    sal_Int32 nCount = rIo.aHdr.GetFATSize();
    sal_uLong nErr;
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        if( ( nErr = aFat.Mark(rIo.pFAT->GetPage( short(i), sal_False ), aFat.GetPageSize(), -3 )) != FAT_OK )
            return nErr;
    }
    if( rIo.aHdr.GetMasters() )
        if( ( nErr = aFat.Mark(rIo.aHdr.GetFATChain( ), aFat.GetPageSize(), -4 )) != FAT_OK )
            return nErr;
    return FAT_OK;
}

sal_uLong Validator::MarkAll( StgDirEntry *pEntry )
{
    StgIterator aIter( *pEntry );
    sal_uLong nErr = FAT_OK;
    for( StgDirEntry* p = aIter.First(); p ; p = aIter.Next() )
    {
        if( p->aEntry.GetType() == STG_STORAGE )
        {
            nErr = MarkAll( p );
            if( nErr != FAT_OK )
                return nErr;
        }
        else
        {
            sal_Int32 nSize = p->aEntry.GetSize();
            if( nSize < rIo.aHdr.GetThreshold()  )
                nErr = aSmallFat.Mark( p->aEntry.GetStartPage(),nSize, -2 );
            else
                nErr = aFat.Mark( p->aEntry.GetStartPage(),nSize, -2 );
            if( nErr != FAT_OK )
                return nErr;
        }
    }
    return FAT_OK;
}

sal_uLong Validator::ValidateDirectoryEntries()
{
    // Normale DirEntries
    sal_uLong nErr = MarkAll( rIo.pTOC->GetRoot() );
    if( nErr != FAT_OK )
        return nErr;
    // Small Data
    nErr = aFat.Mark( rIo.pTOC->GetRoot()->aEntry.GetStartPage(),
                 rIo.pTOC->GetRoot()->aEntry.GetSize(), -2 );
    if( nErr != FAT_OK )
        return nErr;
    // Small Data FAT
    nErr = aFat.Mark(
        rIo.aHdr.GetDataFATStart(),
        rIo.aHdr.GetDataFATSize() * aFat.GetPageSize(), -2 );
    if( nErr != FAT_OK )
        return nErr;
    // TOC
    nErr = aFat.Mark(
        rIo.aHdr.GetTOCStart(), -1, -2 );
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

namespace { struct ErrorLink : public rtl::Static<Link, ErrorLink > {}; }

void StgIo::SetErrorLink( const Link& rLink )
{
    ErrorLink::get() = rLink;
}

const Link& StgIo::GetErrorLink()
{
    return ErrorLink::get();
}

sal_uLong StgIo::ValidateFATs()
{
    if( bFile )
    {
        Validator *pV = new Validator( *this );
        sal_Bool bRet1 = !pV->IsError(), bRet2 = sal_True ;
        delete pV;
        SvFileStream *pFileStrm = ( SvFileStream *) GetStrm();
        StgIo aIo;
        if( aIo.Open( pFileStrm->GetFileName(),
                      STREAM_READ  | STREAM_SHARE_DENYNONE) &&
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
        if( nErr != FAT_OK && !bCopied )
        {
            StgLinkArg aArg;
            aArg.aFile = pFileStrm->GetFileName();
            aArg.nErr = nErr;
            ErrorLink::get().Call( &aArg );
            bCopied = sal_True;
        }
//      DBG_ASSERT( nErr == FAT_OK ,"Storage kaputt");
        return nErr;
    }
//  OSL_FAIL("Validiere nicht (kein FileStorage)");
    return FAT_OK;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
