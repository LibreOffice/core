/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Inflater.cxx,v $
 * $Revision: 1.15 $
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

/** Provides general purpose decompression using the ZLIB library */

Inflater::Inflater(sal_Bool bNoWrap)
: bFinish(sal_False),
  bFinished(sal_False),
  bSetParams(sal_False),
  bNeedDict(sal_False),
  nOffset(0),
  nLength(0),
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
void SAL_CALL Inflater::setInputSegment( const Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
{
    sInBuffer = rBuffer;
    nOffset = nNewOffset;
    nLength = nNewLength;
}

void SAL_CALL Inflater::setInput( const Sequence< sal_Int8 >& rBuffer )
{
    sInBuffer = rBuffer;
    nOffset = 0;
    nLength = rBuffer.getLength();
}

void SAL_CALL Inflater::setDictionarySegment( const Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
{
    if (pStream == NULL)
    {
        // do error handling
    }
    if (nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength())
    {
        // do error handling
    }
#ifdef SYSTEM_ZLIB
    inflateSetDictionary(pStream, (const unsigned char*)rBuffer.getConstArray() + nNewOffset,
                  nNewLength);
#else
    z_inflateSetDictionary(pStream, (const unsigned char*)rBuffer.getConstArray() + nNewOffset,
                  nNewLength);
#endif
}

void SAL_CALL Inflater::setDictionary( const Sequence< sal_Int8 >& rBuffer )
{
    if (pStream == NULL)
    {
        // do error handling
    }
#ifdef SYSTEM_ZLIB
    inflateSetDictionary(pStream, (const unsigned char*)rBuffer.getConstArray(),
                   rBuffer.getLength());
#else
    z_inflateSetDictionary(pStream, (const unsigned char*)rBuffer.getConstArray(),
                   rBuffer.getLength());
#endif
}

sal_Int32 SAL_CALL Inflater::getRemaining(  )
{
    return nLength;
}

sal_Bool SAL_CALL Inflater::needsInput(  )
{
    return nLength <=0;
}

sal_Bool SAL_CALL Inflater::needsDictionary(  )
{
    return bNeedDict;
}

void SAL_CALL Inflater::finish(  )
{
    bFinish = sal_True;
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

sal_Int32 SAL_CALL Inflater::doInflate( Sequence< sal_Int8 >& rBuffer )
{
    return doInflateBytes(rBuffer, 0, rBuffer.getLength());
}

sal_Int32 SAL_CALL Inflater::getAdler(  )
{
    return pStream->adler;
}

sal_Int32 SAL_CALL Inflater::getTotalIn(  )
{
    return pStream->total_in;
}

sal_Int32 SAL_CALL Inflater::getTotalOut(  )
{
    return pStream->total_out;
}

void SAL_CALL Inflater::reset(  )
{
#ifdef SYSTEM_ZLIB
    inflateReset(pStream);
#else
    z_inflateReset(pStream);
#endif
    bFinish = bNeedDict = bFinished = sal_False;
    nOffset = nLength = 0;
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
    sal_Int32 nResult;
    pStream->next_in   = ( unsigned char* ) ( sInBuffer.getConstArray() + nOffset );
    pStream->avail_in  = nLength;
    pStream->next_out  = reinterpret_cast < unsigned char* > ( rBuffer.getArray() + nNewOffset );
    pStream->avail_out = nNewLength;

#ifdef SYSTEM_ZLIB
    nResult = ::inflate(pStream, bFinish ? Z_SYNC_FLUSH : Z_PARTIAL_FLUSH);
#else
    nResult = ::z_inflate(pStream, bFinish ? Z_SYNC_FLUSH : Z_PARTIAL_FLUSH);
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
        case Z_BUF_ERROR:
            return 0;
        case Z_DATA_ERROR:
            return 0;
    }
    return 0;
}

