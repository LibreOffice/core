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

#include <sal/config.h>

#include <algorithm>

#include <tools/stream.hxx>

#include <zlib.h>

#include <tools/zcodec.hxx>

/* gzip flag byte */
//      GZ_ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define GZ_HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define GZ_EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define GZ_ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define GZ_COMMENT      0x10 /* bit 4 set: file comment present */
#define GZ_RESERVED     0xE0 /* bits 5..7: reserved */

const int gz_magic[2] = { 0x1f, 0x8b }; /* gzip magic header */

ZCodec::ZCodec( size_t nInBufSize, size_t nOutBufSize )
    : meState(STATE_INIT)
    , mbStatus(false)
    , mbFinish(false)
    , mpInBuf(nullptr)
    , mnInBufSize(nInBufSize)
    , mnInToRead(0)
    , mpOStm(nullptr)
    , mpOutBuf(nullptr)
    , mnOutBufSize(nOutBufSize)
    , mnCompressLevel(0)
    , mbGzLib(false)
{
    mpsC_Stream = new z_stream;
}

ZCodec::~ZCodec()
{
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    delete pStream;
}

void ZCodec::BeginCompression( int nCompressLevel, bool gzLib )
{
    assert(meState == STATE_INIT);
    mbStatus = true;
    mbFinish = false;
    mpOStm = nullptr;
    mnInToRead = 0xffffffff;
    mpInBuf = mpOutBuf = nullptr;
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    pStream->total_out = pStream->total_in = 0;
    mnCompressLevel = nCompressLevel;
    mbGzLib = gzLib;
    pStream->zalloc = nullptr;
    pStream->zfree = nullptr;
    pStream->opaque = nullptr;
    pStream->avail_out = pStream->avail_in = 0;
}

long ZCodec::EndCompression()
{
    long retvalue = 0;
    auto pStream = static_cast<z_stream*>(mpsC_Stream);

    if (meState != STATE_INIT)
    {
        if (meState == STATE_COMPRESS)
        {
            if (mbStatus)
            {
                do
                {
                    ImplWriteBack();
                }
                while ( deflate( pStream, Z_FINISH ) != Z_STREAM_END );

                ImplWriteBack();
            }

            retvalue = pStream->total_in;
            deflateEnd( pStream );
        }
        else
        {
            retvalue = pStream->total_out;
            inflateEnd( pStream );
        }
        delete[] mpOutBuf;
        delete[] mpInBuf;
        meState = STATE_INIT;
    }
    return mbStatus ? retvalue : -1;
}

void ZCodec::Compress( SvStream& rIStm, SvStream& rOStm )
{
    assert(meState == STATE_INIT);
    mpOStm = &rOStm;
    InitCompress();
    mpInBuf = new sal_uInt8[ mnInBufSize ];
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    for (;;)
    {
        pStream->next_in = mpInBuf;
        pStream->avail_in = rIStm.ReadBytes( pStream->next_in, mnInBufSize );
        if (pStream->avail_in == 0)
            break;
        if ( pStream->avail_out == 0 )
            ImplWriteBack();
        if ( deflate( pStream, Z_NO_FLUSH ) < 0 )
        {
            mbStatus = false;
            break;
        }
    };
}

long ZCodec::Decompress( SvStream& rIStm, SvStream& rOStm )
{
    int err;
    size_t nInToRead;
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    long    nOldTotal_Out = pStream->total_out;

    assert(meState == STATE_INIT);
    mpOStm = &rOStm;
    InitDecompress(rIStm);
    pStream->avail_out = mnOutBufSize;
    pStream->next_out = mpOutBuf = new sal_uInt8[ pStream->avail_out ];
    do
    {
        if ( pStream->avail_out == 0 ) ImplWriteBack();
        if ( pStream->avail_in == 0 && mnInToRead )
        {
            nInToRead = std::min( mnInBufSize, mnInToRead );
            pStream->next_in = mpInBuf;
            pStream->avail_in = rIStm.ReadBytes(mpInBuf, nInToRead);
            mnInToRead -= nInToRead;
        }
        err = mbStatus ? inflate(pStream, Z_NO_FLUSH) : Z_ERRNO;
        if (err < 0 || err == Z_NEED_DICT)
        {
            mbStatus = false;
            break;
        }

    }
    while ( ( err != Z_STREAM_END)  && ( pStream->avail_in || mnInToRead ) );
    ImplWriteBack();

    return mbStatus ? static_cast<long>(pStream->total_out - nOldTotal_Out) : -1;
}

void ZCodec::Write( SvStream& rOStm, const sal_uInt8* pData, sal_uInt32 nSize )
{
    if (meState == STATE_INIT)
    {
        mpOStm = &rOStm;
        InitCompress();
    }
    assert(&rOStm == mpOStm);

    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    pStream->avail_in = nSize;
    pStream->next_in = const_cast<unsigned char*>(pData);

    while ( pStream->avail_in || ( pStream->avail_out == 0 ) )
    {
        if ( pStream->avail_out == 0 )
            ImplWriteBack();

        if ( deflate( pStream, Z_NO_FLUSH ) < 0 )
        {
            mbStatus = false;
            break;
        }
    }
}

long ZCodec::Read( SvStream& rIStm, sal_uInt8* pData, sal_uInt32 nSize )
{
    int err;
    size_t nInToRead;

    if ( mbFinish )
        return 0;           // pStream->total_out;

    if (meState == STATE_INIT)
    {
        InitDecompress(rIStm);
    }
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    pStream->avail_out = nSize;
    pStream->next_out = pData;
    do
    {
        if ( pStream->avail_in == 0 && mnInToRead )
        {
            nInToRead = std::min(mnInBufSize, mnInToRead);
            pStream->next_in = mpInBuf;
            pStream->avail_in = rIStm.ReadBytes(mpInBuf, nInToRead);
            mnInToRead -= nInToRead;
        }
        err = mbStatus ? inflate(pStream, Z_NO_FLUSH) : Z_ERRNO;
        if (err < 0 || err == Z_NEED_DICT)
        {
            // Accept Z_BUF_ERROR as EAGAIN or EWOULDBLOCK.
            mbStatus = (err == Z_BUF_ERROR);
            break;
        }
    }
    while ( (err != Z_STREAM_END) &&
            (pStream->avail_out != 0) &&
            (pStream->avail_in || mnInToRead) );
    if ( err == Z_STREAM_END )
        mbFinish = true;

    return (mbStatus ? static_cast<long>(nSize - pStream->avail_out) : -1);
}

long ZCodec::ReadAsynchron( SvStream& rIStm, sal_uInt8* pData, sal_uInt32 nSize )
{
    int err = 0;
    size_t nInToRead;

    if ( mbFinish )
        return 0;           // pStream->total_out;

    if (meState == STATE_INIT)
    {
        InitDecompress(rIStm);
    }
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    pStream->avail_out = nSize;
    pStream->next_out = pData;
    do
    {
        if ( pStream->avail_in == 0 && mnInToRead )
        {
            nInToRead = std::min(mnInBufSize, mnInToRead);

            sal_uInt32 const nRemaining = rIStm.remainingSize();
            if (nRemaining < nInToRead)
            {
                rIStm.SetError( ERRCODE_IO_PENDING );
                err= int(!Z_STREAM_END); // TODO What is appropriate code for this?
                break;
            }

            pStream->next_in = mpInBuf;
            pStream->avail_in = rIStm.ReadBytes(mpInBuf, nInToRead);
            mnInToRead -= nInToRead;
        }
        err = mbStatus ? inflate(pStream, Z_NO_FLUSH) : Z_ERRNO;
        if ( err < 0 )
        {
            // Accept Z_BUF_ERROR as EAGAIN or EWOULDBLOCK.
            mbStatus = (err == Z_BUF_ERROR);
            break;
        }
    }
    while ( (err == Z_OK) &&
            (pStream->avail_out != 0) &&
            (pStream->avail_in || mnInToRead) );
    if ( err == Z_STREAM_END )
        mbFinish = true;

    return (mbStatus ? static_cast<long>(nSize - pStream->avail_out) : -1);
}

void ZCodec::ImplWriteBack()
{
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    size_t nAvail = mnOutBufSize - pStream->avail_out;

    if ( nAvail > 0 )
    {
        pStream->next_out = mpOutBuf;
        mpOStm->WriteBytes( mpOutBuf, nAvail );
        pStream->avail_out = mnOutBufSize;
    }
}

void ZCodec::SetBreak( size_t nInToRead )
{
    mnInToRead = nInToRead;
}

size_t ZCodec::GetBreak() const
{
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    return ( mnInToRead + pStream->avail_in );
}

void ZCodec::InitCompress()
{
    assert(meState == STATE_INIT);
    meState = STATE_COMPRESS;
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    mbStatus = deflateInit2_(
        pStream, mnCompressLevel, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL,
        Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof (z_stream)) >= 0;
    mpOutBuf = new sal_uInt8[mnOutBufSize];
    pStream->next_out = mpOutBuf;
    pStream->avail_out = mnOutBufSize;
}

void ZCodec::InitDecompress(SvStream & inStream)
{
    assert(meState == STATE_INIT);
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    if ( mbStatus &&  mbGzLib )
    {
        sal_uInt8 j, nMethod, nFlags;
        for (int i : gz_magic)   // gz - magic number
        {
            inStream.ReadUChar( j );
            if ( j != i )
                mbStatus = false;
        }
        inStream.ReadUChar( nMethod );
        inStream.ReadUChar( nFlags );
        if ( nMethod != Z_DEFLATED )
            mbStatus = false;
        if ( ( nFlags & GZ_RESERVED ) != 0 )
            mbStatus = false;
        /* Discard time, xflags and OS code: */
        inStream.SeekRel( 6 );
        /* skip the extra field */
        if ( nFlags & GZ_EXTRA_FIELD )
        {
            sal_uInt8 n1, n2;
            inStream.ReadUChar( n1 ).ReadUChar( n2 );
            inStream.SeekRel( n1 + ( n2 << 8 ) );
        }
        /* skip the original file name */
        if ( nFlags & GZ_ORIG_NAME)
        {
            do
            {
                inStream.ReadUChar( j );
            }
            while ( j && !inStream.eof() );
        }
        /* skip the .gz file comment */
        if ( nFlags & GZ_COMMENT )
        {
            do
            {
                inStream.ReadUChar( j );
            }
            while ( j && !inStream.eof() );
        }
        /* skip the header crc */
        if ( nFlags & GZ_HEAD_CRC )
            inStream.SeekRel( 2 );
        if ( mbStatus )
            mbStatus = inflateInit2( pStream, -MAX_WBITS) == Z_OK;
    }
    else
    {
        mbStatus = ( inflateInit( pStream ) >= 0 );
    }
    if ( mbStatus )
        meState = STATE_DECOMPRESS;
    mpInBuf = new sal_uInt8[ mnInBufSize ];
}

bool ZCodec::AttemptDecompression(SvStream& rIStm, SvStream& rOStm)
{
    assert(meState == STATE_INIT);
    sal_uInt64 nStreamPos = rIStm.Tell();
    BeginCompression(ZCODEC_DEFAULT_COMPRESSION, true/*gzLib*/);
    InitDecompress(rIStm);
    EndCompression();
    if ( !mbStatus || rIStm.GetError() )
    {
        rIStm.Seek(nStreamPos);
        return false;
    }
    rIStm.Seek(nStreamPos);
    BeginCompression(ZCODEC_DEFAULT_COMPRESSION, true/*gzLib*/);
    Decompress(rIStm, rOStm);
    EndCompression();
    if( !mbStatus || rIStm.GetError() || rOStm.GetError() )
    {
        rIStm.Seek(nStreamPos);
        return false;
    }
    rIStm.Seek(nStreamPos);
    rOStm.Seek(0);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
