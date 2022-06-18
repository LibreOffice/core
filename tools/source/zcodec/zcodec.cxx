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
#include <tools/long.hxx>

/* gzip flag byte */
//                   GZ_ASCII_FLAG     = 0x01;   /* bit 0 set: file probably ascii text */
constexpr sal_uInt8  GZ_HEAD_CRC       = 0x02;   /* bit 1 set: header CRC present */
constexpr sal_uInt8  GZ_EXTRA_FIELD    = 0x04;   /* bit 2 set: extra field present */
constexpr sal_uInt8  GZ_ORIG_NAME      = 0x08;   /* bit 3 set: original file name present */
constexpr sal_uInt8  GZ_COMMENT        = 0x10;   /* bit 4 set: file comment present */
constexpr sal_uInt8  GZ_RESERVED       = 0xE0;   /* bits 5..7: reserved */
constexpr sal_uInt16 GZ_MAGIC_BYTES_LE = 0x8B1F; /* gzip magic bytes, little endian */
constexpr sal_uInt8  GZ_DEFLATE        = 0x08;
constexpr sal_uInt8  GZ_FS_UNKNOWN     = 0xFF;

ZCodec::ZCodec( size_t nInBufSize, size_t nOutBufSize )
    : meState(STATE_INIT)
    , mbStatus(false)
    , mbFinish(false)
    , mnInBufSize(nInBufSize)
    , mnInToRead(0)
    , mpOStm(nullptr)
    , mnOutBufSize(nOutBufSize)
    , mnUncompressedSize(0)
    , mnInBufCRC32(0)
    , mnLastModifiedTime(0)
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

bool ZCodec::IsZCompressed( SvStream& rIStm )
{
    sal_uInt64 nCurPos = rIStm.Tell();
    rIStm.Seek( 0 );
    sal_uInt16 nFirstTwoBytes = 0;
    rIStm.ReadUInt16( nFirstTwoBytes );
    rIStm.Seek( nCurPos );
    return nFirstTwoBytes == GZ_MAGIC_BYTES_LE;
}

void ZCodec::BeginCompression( int nCompressLevel, bool gzLib )
{
    assert(meState == STATE_INIT);
    mbStatus = true;
    mbFinish = false;
    mpOStm = nullptr;
    mnInToRead = 0xffffffff;
    mpInBuf.reset();
    mpOutBuf.reset();
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    pStream->total_out = pStream->total_in = 0;
    mnCompressLevel = nCompressLevel;
    mbGzLib = gzLib;
    pStream->zalloc = nullptr;
    pStream->zfree = nullptr;
    pStream->opaque = nullptr;
    pStream->avail_out = pStream->avail_in = 0;
}

tools::Long ZCodec::EndCompression()
{
    tools::Long retvalue = 0;
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
            if ( mbGzLib )
            {
                // metadata must be set to compress as gz format
                assert(!msFilename.isEmpty());
                // overwrite zlib checksum
                mpOStm->Seek(STREAM_SEEK_TO_END);
                mpOStm->SeekRel(-4);
                mpOStm->WriteUInt32( mnInBufCRC32 );       // Uncompressed buffer CRC32
                mpOStm->WriteUInt32( mnUncompressedSize ); // Uncompressed size mod 2^32
                mpOStm->Seek( 0 );
                mpOStm->WriteUInt16( GZ_MAGIC_BYTES_LE )   // Magic bytes
                        .WriteUInt8( GZ_DEFLATE )          // Compression algorithm
                        .WriteUInt8( GZ_ORIG_NAME )        // Filename
                        .WriteUInt32( mnLastModifiedTime ) // Modification time
                        .WriteUInt8( 0 )                   // Extra flags
                        .WriteUInt8( GZ_FS_UNKNOWN )       // Operating system
                        .WriteBytes( msFilename.pData->buffer, msFilename.pData->length );
                mpOStm->WriteUInt8( 0 ); // null terminate the filename string
            }
        }
        else
        {
            retvalue = pStream->total_out;
            inflateEnd( pStream );
        }
        mpOutBuf.reset();
        mpInBuf.reset();
        meState = STATE_INIT;
    }
    return mbStatus ? retvalue : -1;
}

void ZCodec::SetCompressionMetadata( const OString& sFilename, sal_uInt32 nLastModifiedTime, sal_uInt32 nInBufCRC32 )
{
    assert( mbGzLib );
    msFilename = sFilename;
    mnLastModifiedTime = nLastModifiedTime;
    mnInBufCRC32 = nInBufCRC32;
}

void ZCodec::Compress( SvStream& rIStm, SvStream& rOStm )
{
    assert(meState == STATE_INIT);
    mpOStm = &rOStm;
    rIStm.Seek(0);
    mnUncompressedSize = rIStm.TellEnd();
    InitCompress();
    mpInBuf.reset(new sal_uInt8[ mnInBufSize ]);
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    for (;;)
    {
        pStream->next_in = mpInBuf.get();
        pStream->avail_in = rIStm.ReadBytes( mpInBuf.get(), mnInBufSize );
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

tools::Long ZCodec::Decompress( SvStream& rIStm, SvStream& rOStm )
{
    int err;
    size_t nInToRead;
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    tools::Long    nOldTotal_Out = pStream->total_out;

    assert(meState == STATE_INIT);
    mpOStm = &rOStm;
    InitDecompress(rIStm);
    pStream->avail_out = mnOutBufSize;
    mpOutBuf.reset(new sal_uInt8[ pStream->avail_out ]);
    pStream->next_out = mpOutBuf.get();
    do
    {
        if ( pStream->avail_out == 0 ) ImplWriteBack();
        if ( pStream->avail_in == 0 && mnInToRead )
        {
            nInToRead = std::min( mnInBufSize, mnInToRead );
            pStream->next_in = mpInBuf.get();
            pStream->avail_in = rIStm.ReadBytes(mpInBuf.get(), nInToRead);
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

    return mbStatus ? static_cast<tools::Long>(pStream->total_out - nOldTotal_Out) : -1;
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
    pStream->next_in = pData;

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

tools::Long ZCodec::Read( SvStream& rIStm, sal_uInt8* pData, sal_uInt32 nSize )
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
            pStream->next_in = mpInBuf.get();
            pStream->avail_in = rIStm.ReadBytes(mpInBuf.get(), nInToRead);
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

    return (mbStatus ? static_cast<tools::Long>(nSize - pStream->avail_out) : -1);
}

void ZCodec::ImplWriteBack()
{
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    size_t nAvail = mnOutBufSize - pStream->avail_out;

    if ( nAvail > 0 )
    {
        pStream->next_out = mpOutBuf.get();
        mpOStm->WriteBytes( mpOutBuf.get(), nAvail );
        pStream->avail_out = mnOutBufSize;
    }
}

void ZCodec::InitCompress()
{
    assert(meState == STATE_INIT);
    if (mbGzLib)
    {
        // Seek just enough so that the zlib header is overwritten after compression
        // with the gz header
        // 10 header bytes + filename length + null terminator - 2 bytes for
        // zlib header that gets overwritten
        mpOStm->Seek(10 + msFilename.getLength() + 1 - 2);
    }
    meState = STATE_COMPRESS;
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    mbStatus = deflateInit2_(
        pStream, mnCompressLevel, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL,
        Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof (z_stream)) >= 0;
    mpOutBuf.reset(new sal_uInt8[mnOutBufSize]);
    pStream->next_out = mpOutBuf.get();
    pStream->avail_out = mnOutBufSize;
}

void ZCodec::InitDecompress(SvStream & inStream)
{
    assert(meState == STATE_INIT);
    auto pStream = static_cast<z_stream*>(mpsC_Stream);
    if ( mbStatus &&  mbGzLib )
    {
        sal_uInt8 j, nMethod, nFlags;
        sal_uInt16 nFirstTwoBytes;
        inStream.Seek( 0 );
        inStream.ReadUInt16( nFirstTwoBytes );
        if ( nFirstTwoBytes != GZ_MAGIC_BYTES_LE )
            mbStatus = false;
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
    mpInBuf.reset(new sal_uInt8[ mnInBufSize ]);
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
