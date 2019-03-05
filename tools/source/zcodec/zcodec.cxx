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
#include <rtl/crc.h>
#include <osl/endian.h>

#define PZSTREAM static_cast<z_stream*>(mpsC_Stream)

/* gzip flag byte */
//      GZ_ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define GZ_HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define GZ_EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define GZ_ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define GZ_COMMENT      0x10 /* bit 4 set: file comment present */
#define GZ_RESERVED     0xE0 /* bits 5..7: reserved */

static const int gz_magic[2] = { 0x1f, 0x8b }; /* gzip magic header */

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
    , mnCRC(0)
    , mnCompressLevel(0)
    , mbUpdateCrc(false)
    , mbGzLib(false)
{
    mpsC_Stream = new z_stream;
}

ZCodec::~ZCodec()
{
    delete static_cast<z_stream*>(mpsC_Stream);
}

void ZCodec::BeginCompression( int nCompressLevel, bool updateCrc, bool gzLib )
{
    assert(meState == STATE_INIT);
    mbStatus = true;
    mbFinish = false;
    mpOStm = nullptr;
    mnInToRead = 0xffffffff;
    mpInBuf = mpOutBuf = nullptr;
    PZSTREAM->total_out = PZSTREAM->total_in = 0;
    mnCompressLevel = nCompressLevel;
    mbUpdateCrc = updateCrc;
    mbGzLib = gzLib;
    PZSTREAM->zalloc = nullptr;
    PZSTREAM->zfree = nullptr;
    PZSTREAM->opaque = nullptr;
    PZSTREAM->avail_out = PZSTREAM->avail_in = 0;
}

long ZCodec::EndCompression()
{
    long retvalue = 0;

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
                while ( deflate( PZSTREAM, Z_FINISH ) != Z_STREAM_END );

                ImplWriteBack();
            }

            retvalue = PZSTREAM->total_in;
            deflateEnd( PZSTREAM );
        }
        else
        {
            retvalue = PZSTREAM->total_out;
            inflateEnd( PZSTREAM );
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
    while ((PZSTREAM->avail_in = rIStm.ReadBytes(
                    PZSTREAM->next_in = mpInBuf, mnInBufSize )) != 0)
    {
        if ( PZSTREAM->avail_out == 0 )
            ImplWriteBack();
        if ( deflate( PZSTREAM, Z_NO_FLUSH ) < 0 )
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
    long    nOldTotal_Out = PZSTREAM->total_out;

    assert(meState == STATE_INIT);
    mpOStm = &rOStm;
    InitDecompress(rIStm);
    PZSTREAM->next_out = mpOutBuf = new sal_uInt8[ PZSTREAM->avail_out = mnOutBufSize ];
    do
    {
        if ( PZSTREAM->avail_out == 0 ) ImplWriteBack();
        if ( PZSTREAM->avail_in == 0 && mnInToRead )
        {
            nInToRead = std::min( mnInBufSize, mnInToRead );
            PZSTREAM->next_in = mpInBuf;
            PZSTREAM->avail_in = rIStm.ReadBytes(mpInBuf, nInToRead);
            mnInToRead -= nInToRead;

            if ( mbUpdateCrc )
                UpdateCRC( mpInBuf, nInToRead );

        }
        err = mbStatus ? inflate(PZSTREAM, Z_NO_FLUSH) : Z_ERRNO;
        if ( err < 0 )
        {
            mbStatus = false;
            break;
        }

    }
    while ( ( err != Z_STREAM_END)  && ( PZSTREAM->avail_in || mnInToRead ) );
    ImplWriteBack();

    return mbStatus ? static_cast<long>(PZSTREAM->total_out - nOldTotal_Out) : -1;
}

void ZCodec::Write( SvStream& rOStm, const sal_uInt8* pData, sal_uInt32 nSize )
{
    if (meState == STATE_INIT)
    {
        mpOStm = &rOStm;
        InitCompress();
    }
    assert(&rOStm == mpOStm);

    PZSTREAM->avail_in = nSize;
    PZSTREAM->next_in = const_cast<unsigned char*>(pData);

    while ( PZSTREAM->avail_in || ( PZSTREAM->avail_out == 0 ) )
    {
        if ( PZSTREAM->avail_out == 0 )
            ImplWriteBack();

        if ( deflate( PZSTREAM, Z_NO_FLUSH ) < 0 )
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
        return 0;           // PZSTREAM->total_out;

    if (meState == STATE_INIT)
    {
        InitDecompress(rIStm);
    }
    PZSTREAM->avail_out = nSize;
    PZSTREAM->next_out = pData;
    do
    {
        if ( PZSTREAM->avail_in == 0 && mnInToRead )
        {
            nInToRead = std::min(mnInBufSize, mnInToRead);
            PZSTREAM->next_in = mpInBuf;
            PZSTREAM->avail_in = rIStm.ReadBytes(mpInBuf, nInToRead);
            mnInToRead -= nInToRead;

            if ( mbUpdateCrc )
                UpdateCRC( mpInBuf, nInToRead );

        }
        err = mbStatus ? inflate(PZSTREAM, Z_NO_FLUSH) : Z_ERRNO;
        if (err < 0 || err == Z_NEED_DICT)
        {
            // Accept Z_BUF_ERROR as EAGAIN or EWOULDBLOCK.
            mbStatus = (err == Z_BUF_ERROR);
            break;
        }
    }
    while ( (err != Z_STREAM_END) &&
            (PZSTREAM->avail_out != 0) &&
            (PZSTREAM->avail_in || mnInToRead) );
    if ( err == Z_STREAM_END )
        mbFinish = true;

    return (mbStatus ? static_cast<long>(nSize - PZSTREAM->avail_out) : -1);
}

long ZCodec::ReadAsynchron( SvStream& rIStm, sal_uInt8* pData, sal_uInt32 nSize )
{
    int err = 0;
    size_t nInToRead;

    if ( mbFinish )
        return 0;           // PZSTREAM->total_out;

    if (meState == STATE_INIT)
    {
        InitDecompress(rIStm);
    }
    PZSTREAM->avail_out = nSize;
    PZSTREAM->next_out = pData;
    do
    {
        if ( PZSTREAM->avail_in == 0 && mnInToRead )
        {
            nInToRead = std::min(mnInBufSize, mnInToRead);

            sal_uInt32 const nRemaining = rIStm.remainingSize();
            if (nRemaining < nInToRead)
            {
                rIStm.SetError( ERRCODE_IO_PENDING );
                err= int(!Z_STREAM_END); // TODO What is appropriate code for this?
                break;
            }

            PZSTREAM->next_in = mpInBuf;
            PZSTREAM->avail_in = rIStm.ReadBytes(mpInBuf, nInToRead);
            mnInToRead -= nInToRead;

            if ( mbUpdateCrc )
                UpdateCRC( mpInBuf, nInToRead );

        }
        err = mbStatus ? inflate(PZSTREAM, Z_NO_FLUSH) : Z_ERRNO;
        if ( err < 0 )
        {
            // Accept Z_BUF_ERROR as EAGAIN or EWOULDBLOCK.
            mbStatus = (err == Z_BUF_ERROR);
            break;
        }
    }
    while ( (err == Z_OK) &&
            (PZSTREAM->avail_out != 0) &&
            (PZSTREAM->avail_in || mnInToRead) );
    if ( err == Z_STREAM_END )
        mbFinish = true;

    return (mbStatus ? static_cast<long>(nSize - PZSTREAM->avail_out) : -1);
}

void ZCodec::ImplWriteBack()
{
    sal_uIntPtr nAvail = mnOutBufSize - PZSTREAM->avail_out;

    if ( nAvail )
    {
        if (meState == STATE_COMPRESS && mbUpdateCrc)
            UpdateCRC( mpOutBuf, nAvail );
        PZSTREAM->next_out = mpOutBuf;
        mpOStm->WriteBytes( mpOutBuf, nAvail );
        PZSTREAM->avail_out = mnOutBufSize;
    }
}

void ZCodec::SetBreak( size_t nInToRead )
{
    mnInToRead = nInToRead;
}

size_t ZCodec::GetBreak() const
{
    return ( mnInToRead + PZSTREAM->avail_in );
}

void ZCodec::SetCRC( sal_uInt32 nCRC )
{
    mnCRC = nCRC;
}


void ZCodec::InitCompress()
{
    assert(meState == STATE_INIT);
    meState = STATE_COMPRESS;
    mbStatus = deflateInit2_(
        PZSTREAM, mnCompressLevel, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL,
        Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof (z_stream)) >= 0;
    mpOutBuf = new sal_uInt8[mnOutBufSize];
    PZSTREAM->next_out = mpOutBuf;
    PZSTREAM->avail_out = mnOutBufSize;
}

void ZCodec::InitDecompress(SvStream & inStream)
{
    assert(meState == STATE_INIT);
    if ( mbStatus &&  mbGzLib )
    {
        sal_uInt8 n1, n2, j, nMethod, nFlags;
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
            mbStatus = inflateInit2( PZSTREAM, -MAX_WBITS) == Z_OK;
    }
    else
    {
        mbStatus = ( inflateInit( PZSTREAM ) >= 0 );
    }
    if ( mbStatus )
        meState = STATE_DECOMPRESS;
    mpInBuf = new sal_uInt8[ mnInBufSize ];
}

void ZCodec::UpdateCRC ( sal_uInt8 const * pSource, long nDatSize)
{
    mnCRC = rtl_crc32( mnCRC, pSource, nDatSize );
}

bool ZCodec::AttemptDecompression(SvStream& rIStm, SvStream& rOStm)
{
    assert(meState == STATE_INIT);
    sal_uInt64 nStreamPos = rIStm.Tell();
    BeginCompression(ZCODEC_DEFAULT_COMPRESSION, false/*updateCrc*/, true/*gzLib*/);
    InitDecompress(rIStm);
    EndCompression();
    if ( !mbStatus || rIStm.GetError() )
    {
        rIStm.Seek(nStreamPos);
        return false;
    }
    rIStm.Seek(nStreamPos);
    BeginCompression(ZCODEC_DEFAULT_COMPRESSION, false/*updateCrc*/, true/*gzLib*/);
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
