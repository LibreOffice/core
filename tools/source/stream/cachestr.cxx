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

#include <tools/stream.hxx>
#include <tools/cachestr.hxx>
#include <tools/tempfile.hxx>

SvCacheStream::SvCacheStream( sal_uIntPtr nMaxMemSize )
{
    if( !nMaxMemSize )
        nMaxMemSize = 20480;
    SvStream::bIsWritable = sal_True;
    nMaxSize        = nMaxMemSize;
    bPersistent     = sal_False;
    pSwapStream     = 0;
    pCurrentStream  = new SvMemoryStream( nMaxMemSize );
    pTempFile       = 0;
}

SvCacheStream::~SvCacheStream()
{
       if( pCurrentStream != pSwapStream )
        delete pSwapStream;
    delete pCurrentStream;

    if( pSwapStream && !bPersistent && pTempFile )
    {
        // temporaeres File loeschen
        pTempFile->EnableKillingFile( sal_True );
    }

    delete pTempFile;
}

void SvCacheStream::SwapOut()
{
    if( pCurrentStream != pSwapStream )
    {
        if( !pSwapStream && !aFileName.Len() )
        {
            pTempFile = new TempFile;
            aFileName = pTempFile->GetName();
        }

        sal_uIntPtr nPos = pCurrentStream->Tell();
        pCurrentStream->Seek( 0 );
        if( !pSwapStream )
            pSwapStream = new SvFileStream( aFileName, STREAM_READWRITE | STREAM_TRUNC );
        *pSwapStream << *pCurrentStream;
        pSwapStream->Flush();
        delete pCurrentStream;
        pCurrentStream = pSwapStream;
        pCurrentStream->Seek( nPos );
    }
}

sal_uIntPtr SvCacheStream::GetData( void* pData, sal_uIntPtr nSize )
{
    return pCurrentStream->Read( pData, nSize );
}

sal_uIntPtr SvCacheStream::PutData( const void* pData, sal_uIntPtr nSize )
{
    // prefer swapping data instead copying it again
    if( pCurrentStream != pSwapStream
        && pCurrentStream->Tell() + nSize > nMaxSize )
        SwapOut();
    return pCurrentStream->Write( pData, nSize );
}

sal_uIntPtr SvCacheStream::SeekPos( sal_uIntPtr nPos )
{
    return pCurrentStream->Seek( nPos );
}

void SvCacheStream::FlushData()
{
    pCurrentStream->Flush();
    if( pCurrentStream != pSwapStream
        && ((SvMemoryStream*)pCurrentStream)->GetSize() > nMaxSize )
        SwapOut();
}

const void* SvCacheStream::GetBuffer()
{
    Flush();
    if( pCurrentStream != pSwapStream )
        return ((SvMemoryStream*)pCurrentStream)->GetData();
    else
        return 0;
}

void SvCacheStream::SetSize( sal_uIntPtr nSize )
{
    pCurrentStream->SetStreamSize( nSize );
}

sal_uIntPtr SvCacheStream::GetSize()
{
    // CAUTION: SvMemoryStream::GetSize() returns size of the allocated buffer
    Flush();
    sal_uIntPtr nTemp = Tell();
    sal_uIntPtr nLength = Seek( STREAM_SEEK_TO_END );
    Seek( nTemp );
    return nLength;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
