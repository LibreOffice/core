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
#include "precompiled_package.hxx"
#include <Inflater.hxx>
#ifndef _ZLIB_H
#ifdef SYSTEM_ZLIB
#include <zlib.h>
#else
#include <external/zlib/zlib.h>
#endif
#endif
#include <string.h> // for memset

using namespace com::sun::star::uno;
using namespace ZipUtils;

/** Provides general purpose decompression using the ZLIB library */

Inflater::Inflater(sal_Bool bNoWrap)
: bFinished(sal_False),
  bSetParams(sal_False),
  bNeedDict(sal_False),
  nOffset(0),
  nLength(0),
  nLastInflateError(0),
  pStream(NULL)
{
    pStream = new z_stream;
    /* memset to 0 to set zalloc/opaque etc */
    memset (pStream, 0, sizeof(*pStream));
    sal_Int32 nRes;
    nRes = inflateInit2(pStream, bNoWrap ? -MAX_WBITS : MAX_WBITS);
    switch (nRes)
    {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
            delete pStream;
            break;
        case Z_STREAM_ERROR:
            delete pStream;
            break;
        default:
            break;
    }
}

Inflater::~Inflater()
{
    end();
}

void SAL_CALL Inflater::setInput( const Sequence< sal_Int8 >& rBuffer )
{
    sInBuffer = rBuffer;
    nOffset = 0;
    nLength = rBuffer.getLength();
}

sal_Bool SAL_CALL Inflater::needsDictionary(  )
{
    return bNeedDict;
}

sal_Bool SAL_CALL Inflater::finished(  )
{
    return bFinished;
}

sal_Int32 SAL_CALL Inflater::doInflateSegment( Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
{
    if (nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength())
    {
        // do error handling
    }
    return doInflateBytes(rBuffer, nNewOffset, nNewLength);
}

void SAL_CALL Inflater::end(  )
{
    if (pStream != NULL)
    {
#ifdef SYSTEM_ZLIB
        inflateEnd(pStream);
#else
        z_inflateEnd(pStream);
#endif
        delete pStream;
    }
    pStream = NULL;
}

sal_Int32 Inflater::doInflateBytes (Sequence < sal_Int8 >  &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    if ( !pStream )
    {
        nLastInflateError = Z_STREAM_ERROR;
        return 0;
    }

    nLastInflateError = 0;

    pStream->next_in   = ( unsigned char* ) ( sInBuffer.getConstArray() + nOffset );
    pStream->avail_in  = nLength;
    pStream->next_out  = reinterpret_cast < unsigned char* > ( rBuffer.getArray() + nNewOffset );
    pStream->avail_out = nNewLength;

#ifdef SYSTEM_ZLIB
    sal_Int32 nResult = ::inflate(pStream, Z_PARTIAL_FLUSH);
#else
    sal_Int32 nResult = ::z_inflate(pStream, Z_PARTIAL_FLUSH);
#endif

    switch (nResult)
    {
        case Z_STREAM_END:
            bFinished = sal_True;
        case Z_OK:
            nOffset += nLength - pStream->avail_in;
            nLength = pStream->avail_in;
            return nNewLength - pStream->avail_out;

        case Z_NEED_DICT:
            bNeedDict = sal_True;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
