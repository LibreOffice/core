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

#include <config_xzlib.h>
#include <tools/stream.hxx>
#include <tools/xzcodec.hxx>

#if ENABLE_XZLIB

#include <lzma.h>

#define PXZSTREAM static_cast<lzma_stream*>(mpLzma_Stream)

static const int xz_magic[6] = { 0xfd, 0x37, 0x7A, 0x58, 0x5A, 0x00  }; /* xz magic header */

XZCodec::XZCodec( sal_uIntPtr nInBufSize, sal_uIntPtr nOutBufSize )
    : meState(STATE_INIT)
    , mbStatus(false)
    , mpInBuf(NULL)
    , mnInBufSize(nInBufSize)
    , mpOutBuf(NULL)
    , mnOutBufSize(nOutBufSize)
{
    lzma_stream tmp = LZMA_STREAM_INIT;
    mpLzma_Stream = new lzma_stream;
    *static_cast<lzma_stream*>(mpLzma_Stream) = tmp;
}

XZCodec::~XZCodec()
{
    lzma_end(static_cast<lzma_stream*>(mpLzma_Stream));
    delete static_cast<lzma_stream*>(mpLzma_Stream);
}

void XZCodec::BeginCompression( void )
{
    assert(meState == STATE_INIT);
    mbStatus = true;
    mpInBuf = mpOutBuf = NULL;
    PXZSTREAM->total_out = PXZSTREAM->total_in = 0;
    PXZSTREAM->allocator = NULL;
    PXZSTREAM->avail_out = PXZSTREAM->avail_in = 0;
}

long XZCodec::EndCompression( void )
{
    if (meState != STATE_INIT)
    {
        delete[] mpOutBuf;
        delete[] mpInBuf;
        meState = STATE_INIT;
    }
    return 0;
}

long XZCodec::Compress( SvStream& rIStm, SvStream& rOStm )
{
    long nOldTotal_In = PXZSTREAM->total_in;

    assert(meState == STATE_INIT);
    meState = STATE_COMPRESS;
    lzma_ret ret = lzma_easy_encoder(PXZSTREAM, LZMA_PRESET_DEFAULT, LZMA_CHECK_CRC64);
    mbStatus = false;
    if (ret != LZMA_OK)
        return -1;

    lzma_action action = LZMA_RUN;
    mpInBuf  = new sal_uInt8[mnInBufSize];
    mpOutBuf = new sal_uInt8[mnOutBufSize];
    PXZSTREAM->next_in = NULL;
    PXZSTREAM->avail_in = 0;
    PXZSTREAM->next_out = mpOutBuf;
    PXZSTREAM->avail_out = mnOutBufSize;

    while (true)
    {
        if (PXZSTREAM->avail_in == 0 && !rIStm.IsEof())
        {
            PXZSTREAM->next_in = mpInBuf;
            PXZSTREAM->avail_in = rIStm.Read(mpInBuf, mnInBufSize);

            if (rIStm.GetError())
                return -1;

            if (rIStm.IsEof())
                action = LZMA_FINISH;
        }

        ret = lzma_code(PXZSTREAM, action);

        if (PXZSTREAM->avail_out == 0 || ret == LZMA_STREAM_END)
        {
            sal_Size write_size = mnOutBufSize - PXZSTREAM->avail_out;

            if (rOStm.Write(mpOutBuf, write_size) != write_size)
                return -1;

            PXZSTREAM->next_out = mpOutBuf;
            PXZSTREAM->avail_out = mnOutBufSize;
        }

        if (ret != LZMA_OK) {

            if (ret == LZMA_STREAM_END)
            {
                mbStatus = true;
                return (long)(PXZSTREAM->total_in - nOldTotal_In);
            }
            return -1;
        }
    }
    return 0;
}

long XZCodec::Decompress( SvStream& rIStm, SvStream& rOStm )
{
    assert(meState == STATE_INIT);
    meState = STATE_DECOMPRESS;
    mbStatus = false;
    lzma_ret ret = lzma_stream_decoder(PXZSTREAM, UINT64_MAX, LZMA_CONCATENATED);

    if (ret != LZMA_OK)
        return -1;

    long nOldTotal_Out = PXZSTREAM->total_out;
    lzma_action action = LZMA_RUN;
    mpInBuf  = new sal_uInt8[mnInBufSize];
    mpOutBuf = new sal_uInt8[mnOutBufSize];

    PXZSTREAM->next_in = NULL;
    PXZSTREAM->avail_in = 0;
    PXZSTREAM->next_out = mpOutBuf;
    PXZSTREAM->avail_out = mnOutBufSize;
    while (true)
    {
        if (PXZSTREAM->avail_in == 0 && !rIStm.IsEof())
        {
            PXZSTREAM->next_in = mpInBuf;
            PXZSTREAM->avail_in = rIStm.Read(mpInBuf, mnInBufSize);

            if (rIStm.GetError())
                return -1;

            if (rIStm.IsEof())
                action = LZMA_FINISH;
        }

        ret = lzma_code(PXZSTREAM, action);

        if (PXZSTREAM->avail_out == 0 || ret == LZMA_STREAM_END)
        {
            sal_Size write_size = mnOutBufSize - PXZSTREAM->avail_out;
            if (rOStm.Write(mpOutBuf, write_size) != write_size)
                return -1;

            PXZSTREAM->next_out = mpOutBuf;
            PXZSTREAM->avail_out = mnOutBufSize;
        }

        if (ret != LZMA_OK)
        {
            if (ret == LZMA_STREAM_END)
            {
                mbStatus = true;
                return (long)(PXZSTREAM->total_out - nOldTotal_Out);
            }
            return -1;
        }
    }
    return 0;
}

void XZCodec::InitDecompress(SvStream& inStream)
{
    sal_uInt8 chr;
    mbStatus = true;
    for ( int i = 0 ; i < 6; i++ )
    {
        inStream.ReadUChar( chr );
        if ( chr != xz_magic[ i ] )
            mbStatus = false;
    }

}


bool XZCodec::AttemptDecompression( SvStream& rIStm, SvStream& rOStm )
{
    assert(meState == STATE_INIT);
    sal_uLong nStreamPos = rIStm.Tell();
    BeginCompression();
    InitDecompress(rIStm);
    if ( !mbStatus || rIStm.GetError() )
    {
        rIStm.Seek(nStreamPos);
        return false;
    }
    rIStm.Seek(nStreamPos);
    BeginCompression();
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

#else

XZCodec::XZCodec( sal_uIntPtr /*nInBufSize*/, sal_uIntPtr /*nOutBufSize*/ )
{}

XZCodec::~XZCodec()
{}

void XZCodec::BeginCompression( void )
{}

long XZCodec::EndCompression( void )
{
    return -1;
}

long XZCodec::Compress( SvStream& /*rIStm*/, SvStream& /*rOStm*/ )
{
    return -1;
}

long XZCodec::Decompress( SvStream& /*rIStm*/, SvStream& /*rOStm*/ )
{
    return -1;
}

void XZCodec::InitDecompress(SvStream& /*inStream*/)
{}


bool XZCodec::AttemptDecompression( SvStream& /*rIStm*/, SvStream& /*rOStm*/ )
{
    return false;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
