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

#include <package/Inflater.hxx>
#include <zlib.h>
#include <string.h>

using namespace com::sun::star::uno;
using namespace ZipUtils;

/** Provides general purpose decompression using the ZLIB library */

Inflater::Inflater(bool bNoWrap)
: bFinished(false),
  bNeedDict(false),
  nOffset(0),
  nLength(0),
  nLastInflateError(0)
{
    pStream.reset(new z_stream);
    /* memset to 0 to set zalloc/opaque etc */
    memset (pStream.get(), 0, sizeof(*pStream));
    sal_Int32 nRes;
    nRes = inflateInit2(pStream.get(), bNoWrap ? -MAX_WBITS : MAX_WBITS);
    switch (nRes)
    {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
            pStream.reset();
            break;
        case Z_STREAM_ERROR:
            pStream.reset();
            break;
        default:
            break;
    }
}

Inflater::~Inflater()
{
    end();
}

void Inflater::setInput( const Sequence< sal_Int8 >& rBuffer )
{
    sInBuffer = rBuffer;
    nOffset = 0;
    nLength = rBuffer.getLength();
}


sal_Int32 Inflater::doInflateSegment( Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
{
    if (nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength())
    {
        // do error handling
    }
    return doInflateBytes(rBuffer, nNewOffset, nNewLength);
}

void Inflater::end(  )
{
    if (pStream)
    {
#if !defined Z_PREFIX
        inflateEnd(pStream.get());
#else
        z_inflateEnd(pStream.get());
#endif
        pStream.reset();
    }
}

sal_Int32 Inflater::doInflateBytes (Sequence < sal_Int8 >  &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    if ( !pStream )
    {
        nLastInflateError = Z_STREAM_ERROR;
        return 0;
    }

    nLastInflateError = 0;

    pStream->next_in   = reinterpret_cast<const unsigned char*>( sInBuffer.getConstArray() + nOffset );
    pStream->avail_in  = nLength;
    pStream->next_out  = reinterpret_cast < unsigned char* > ( rBuffer.getArray() + nNewOffset );
    pStream->avail_out = nNewLength;

#if !defined Z_PREFIX
    sal_Int32 nResult = ::inflate(pStream.get(), Z_PARTIAL_FLUSH);
#else
    sal_Int32 nResult = ::z_inflate(pStream.get(), Z_PARTIAL_FLUSH);
#endif

    switch (nResult)
    {
        case Z_STREAM_END:
            bFinished = true;
            [[fallthrough]];
        case Z_OK:
            nOffset += nLength - pStream->avail_in;
            nLength = pStream->avail_in;
            return nNewLength - pStream->avail_out;

        case Z_NEED_DICT:
            bNeedDict = true;
            nOffset += nLength - pStream->avail_in;
            nLength = pStream->avail_in;
            return 0;

        default:
            // it is no error, if there is no input or no output
            if ( nLength && nNewLength )
                nLastInflateError = nResult;
    }

    return 0;
}

InflaterBytes::InflaterBytes()
: bFinished(false),
  nOffset(0),
  nLength(0),
  sInBuffer(nullptr)
{
    pStream.reset(new z_stream);
    /* memset to 0 to set zalloc/opaque etc */
    memset (pStream.get(), 0, sizeof(*pStream));
    sal_Int32 nRes;
    nRes = inflateInit2(pStream.get(), -MAX_WBITS);
    switch (nRes)
    {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
            pStream.reset();
            break;
        case Z_STREAM_ERROR:
            pStream.reset();
            break;
        default:
            break;
    }
}

InflaterBytes::~InflaterBytes()
{
    end();
}

void InflaterBytes::setInput( const sal_Int8* rBuffer, sal_Int32 nBufLen )
{
    sInBuffer = rBuffer;
    nOffset = 0;
    nLength = nBufLen;
}


sal_Int32 InflaterBytes::doInflateSegment( sal_Int8* pOutBuffer, sal_Int32 nBufLen, sal_Int32 nNewOffset, sal_Int32 nNewLength )
{
    if (nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > nBufLen)
    {
        // do error handling
    }
    return doInflateBytes(pOutBuffer, nNewOffset, nNewLength);
}

void InflaterBytes::end(  )
{
    if (pStream)
    {
#if !defined Z_PREFIX
        inflateEnd(pStream.get());
#else
        z_inflateEnd(pStream.get());
#endif
        pStream.reset();
    }
}

sal_Int32 InflaterBytes::doInflateBytes (sal_Int8* pOutBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    if ( !pStream )
    {
        return 0;
    }

    pStream->next_in   = reinterpret_cast<const unsigned char*>( sInBuffer + nOffset );
    pStream->avail_in  = nLength;
    pStream->next_out  = reinterpret_cast < unsigned char* > ( pOutBuffer + nNewOffset );
    pStream->avail_out = nNewLength;

#if !defined Z_PREFIX
    sal_Int32 nResult = ::inflate(pStream.get(), Z_PARTIAL_FLUSH);
#else
    sal_Int32 nResult = ::z_inflate(pStream.get(), Z_PARTIAL_FLUSH);
#endif

    switch (nResult)
    {
        case Z_STREAM_END:
            bFinished = true;
            [[fallthrough]];
        case Z_OK:
            nOffset += nLength - pStream->avail_in;
            nLength = pStream->avail_in;
            return nNewLength - pStream->avail_out;

        case Z_NEED_DICT:
            nOffset += nLength - pStream->avail_in;
            nLength = pStream->avail_in;
            return 0;

        default:
            // it is no error, if there is no input or no output
            break;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
