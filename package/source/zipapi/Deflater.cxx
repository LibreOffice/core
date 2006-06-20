/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Deflater.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:12:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _DEFLATER_HXX_
#include <Deflater.hxx>
#endif
#ifndef _ZLIB_H
#ifdef SYSTEM_ZLIB
#include <zlib.h>
#else
#include <external/zlib/zlib.h>
#endif
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#endif
#include <string.h> // for memset

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star;

/** Provides general purpose compression using the ZLIB compression
 * library.
 */

Deflater::~Deflater(void)
{
    end();
}
void Deflater::init (sal_Int32 nLevelArg, sal_Int32 nStrategyArg, sal_Bool bNowrap)
{
    pStream = new z_stream;
    /* Memset it to 0...sets zalloc/zfree/opaque to NULL */
    memset (pStream, 0, sizeof(*pStream));

    switch (deflateInit2(pStream, nLevelArg, Z_DEFLATED, bNowrap? -MAX_WBITS : MAX_WBITS,
                DEF_MEM_LEVEL, nStrategyArg))
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

Deflater::Deflater()
: bFinish(sal_False)
, bFinished(sal_False)
, bSetParams(sal_False)
, nLevel(DEFAULT_COMPRESSION)
, nStrategy(DEFAULT_STRATEGY)
, nOffset(0)
, nLength(0)
{
    init(DEFAULT_COMPRESSION, DEFAULT_STRATEGY, sal_False);
}

Deflater::Deflater(sal_Int32 nSetLevel)
: bFinish(sal_False)
, bFinished(sal_False)
, bSetParams(sal_False)
, nLevel(nSetLevel)
, nStrategy(DEFAULT_STRATEGY)
, nOffset(0)
, nLength(0)
{
    init(nSetLevel, DEFAULT_STRATEGY, sal_False);
}

Deflater::Deflater(sal_Int32 nSetLevel, sal_Bool bNowrap)
: bFinish(sal_False)
, bFinished(sal_False)
, bSetParams(sal_False)
, nLevel(nSetLevel)
, nStrategy(DEFAULT_STRATEGY)
, nOffset(0)
, nLength(0)
{
    init(nSetLevel, DEFAULT_STRATEGY, bNowrap);
}

sal_Int32 Deflater::doDeflateBytes (uno::Sequence < sal_Int8 > &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    sal_Int32 nResult;
    if (bSetParams)
    {
        pStream->next_in   = (unsigned char*) sInBuffer.getConstArray() + nOffset;
        pStream->next_out  = (unsigned char*) rBuffer.getArray()+nNewOffset;
        pStream->avail_in  = nLength;
        pStream->avail_out = nNewLength;

#ifdef SYSTEM_ZLIB
        nResult = deflateParams(pStream, nLevel, nStrategy);
#else
        nResult = z_deflateParams(pStream, nLevel, nStrategy);
#endif
        switch (nResult)
        {
            case Z_OK:
                bSetParams = sal_False;
                nOffset += nLength - pStream->avail_in;
                nLength = pStream->avail_in;
                return nNewLength - pStream->avail_out;
            case Z_BUF_ERROR:
                bSetParams = sal_False;
                return 0;
            default:
                return 0;
        }
    }
    else
    {
        pStream->next_in   = (unsigned char*) sInBuffer.getConstArray() + nOffset;
        pStream->next_out  = (unsigned char*) rBuffer.getArray()+nNewOffset;
        pStream->avail_in  = nLength;
        pStream->avail_out = nNewLength;

#ifdef SYSTEM_ZLIB
        nResult = deflate(pStream, bFinish ? Z_FINISH : Z_NO_FLUSH);
#else
        nResult = z_deflate(pStream, bFinish ? Z_FINISH : Z_NO_FLUSH);
#endif
        switch (nResult)
        {
            case Z_STREAM_END:
                bFinished = sal_True;
            case Z_OK:
                nOffset += nLength - pStream->avail_in;
                nLength = pStream->avail_in;
                return nNewLength - pStream->avail_out;
            case Z_BUF_ERROR:
                bSetParams = sal_False;
                return 0;
            default:
                return 0;
        }
    }
}

void SAL_CALL Deflater::setInputSegment( const uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
{
    OSL_ASSERT( !(nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength()));

    sInBuffer = rBuffer;
    nOffset = nNewOffset;
    nLength = nNewLength;
}
void SAL_CALL Deflater::setInput( const uno::Sequence< sal_Int8 >& rBuffer )
{
    sInBuffer = rBuffer;
    nOffset = 0;
    nLength = rBuffer.getLength();
}
void SAL_CALL Deflater::setDictionarySegment( const uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
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
    deflateSetDictionary(pStream, (const unsigned char*)rBuffer.getConstArray()+nOffset, nLength);
#else
    z_deflateSetDictionary(pStream, (const unsigned char*)rBuffer.getConstArray()+nOffset, nLength);
#endif
}
void SAL_CALL Deflater::setDictionary( const uno::Sequence< sal_Int8 >& rBuffer )
{
    if (pStream == NULL)
    {
        // do error handling
    }
#ifdef SYSTEM_ZLIB
    deflateSetDictionary(pStream, (const unsigned char*)rBuffer.getConstArray(), rBuffer.getLength());
#else
    z_deflateSetDictionary(pStream, (const unsigned char*)rBuffer.getConstArray(), rBuffer.getLength());
#endif
}
void SAL_CALL Deflater::setStrategy( sal_Int32 nNewStrategy )
{
    if (nNewStrategy != DEFAULT_STRATEGY &&
        nNewStrategy != FILTERED &&
        nNewStrategy != HUFFMAN_ONLY)
    {
        // do error handling
    }
    if (nStrategy != nNewStrategy)
    {
        nStrategy = nNewStrategy;
        bSetParams = sal_True;
    }
}
void SAL_CALL Deflater::setLevel( sal_Int32 nNewLevel )
{
    if ((nNewLevel < 0 || nNewLevel > 9) && nNewLevel != DEFAULT_COMPRESSION)
    {
        // do error handling
    }
    if (nNewLevel != nLevel)
    {
        nLevel = nNewLevel;
        bSetParams = sal_True;
    }
}
sal_Bool SAL_CALL Deflater::needsInput(  )
{
    return nLength <=0;
}
void SAL_CALL Deflater::finish(  )
{
    bFinish = sal_True;
}
sal_Bool SAL_CALL Deflater::finished(  )
{
    return bFinished;
}
sal_Int32 SAL_CALL Deflater::doDeflateSegment( uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
{
    OSL_ASSERT( !(nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength()));
    return doDeflateBytes(rBuffer, nNewOffset, nNewLength);
}
sal_Int32 SAL_CALL Deflater::doDeflate( uno::Sequence< sal_Int8 >& rBuffer )
{
    return doDeflateBytes(rBuffer, 0, rBuffer.getLength());
}

sal_Int32 SAL_CALL Deflater::getAdler(  )
{
    return pStream->adler;
}
sal_Int32 SAL_CALL Deflater::getTotalIn(  )
{
    return pStream->total_in;
}
sal_Int32 SAL_CALL Deflater::getTotalOut(  )
{
    return pStream->total_out;
}
void SAL_CALL Deflater::reset(  )
{
#ifdef SYSTEM_ZLIB
    deflateReset(pStream);
#else
    z_deflateReset(pStream);
#endif
    bFinish = sal_False;
    bFinished = sal_False;
    nOffset = nLength = 0;
}
void SAL_CALL Deflater::end(  )
{
    if (pStream != NULL)
    {
#ifdef SYSTEM_ZLIB
        deflateEnd(pStream);
#else
        z_deflateEnd(pStream);
#endif
        delete pStream;
    }
    pStream = NULL;
}
