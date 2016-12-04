/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#if defined SYSTEM_ZLIB || !defined ZLIB_PREFIX
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

#if defined SYSTEM_ZLIB || !defined ZLIB_PREFIX
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

