/*************************************************************************
 *
 *  $RCSfile: Inflater.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mtg $ $Date: 2000-11-21 12:07:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _INFLATER_HXX_
#include "Inflater.hxx"
#endif

#include <iostream.h>
#include <string.h>

/** Provides general purpose decompression using the ZLIB library */
void Inflater::init (sal_Bool bNowrap)
{
    pStream = new z_stream;
    /* memset to 0 to set zalloc/opaque etc */
    memset (pStream, 0, sizeof(*pStream));
    sal_Int32 nRes;
    nRes = inflateInit2(pStream, bNowrap ? -MAX_WBITS : MAX_WBITS);
    switch (nRes)
    {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
            VOS_DEBUG_ONLY ( pStream->msg);
            delete pStream;
            break;
        case Z_STREAM_ERROR:
            VOS_DEBUG_ONLY ( pStream->msg);
            delete pStream;
            break;
        default:
            VOS_DEBUG_ONLY ( pStream->msg);
            break;
    }
}

Inflater::Inflater(sal_Bool bNoWrap)
: bFinish(sal_False),
  bFinished(sal_False),
  bSetParams(sal_False),
  bNeedDict(sal_False),
  nOffset(0),
  nLength(0),
  pStream(NULL)
{
    init(bNoWrap);
}

Inflater::Inflater()
: bFinish(sal_False),
  bFinished(sal_False),
  bSetParams(sal_False),
  bNeedDict(sal_False),
  nOffset(0),
  nLength(0),
  pStream(NULL)
{
    init(sal_False);
}
Inflater::~Inflater()
{
    if (pStream)
        delete pStream;
}
void SAL_CALL Inflater::setInputSegment( const com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(com::sun::star::uno::RuntimeException)
{
    sInBuffer = rBuffer;
    nOffset = nNewOffset;
    nLength = nNewLength;
}

void SAL_CALL Inflater::setInput( const com::sun::star::uno::Sequence< sal_Int8 >& rBuffer )
        throw(com::sun::star::uno::RuntimeException)
{
    sInBuffer = rBuffer;
    nOffset = 0;
    nLength = rBuffer.getLength();
}

void SAL_CALL Inflater::setDictionarySegment( const com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(com::sun::star::uno::RuntimeException)
{
    if (pStream == NULL)
    {
        // do error handling
    }
    if (nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength())
    {
        // do error handling
    }
    z_inflateSetDictionary(pStream, (const unsigned char*)rBuffer.getConstArray() + nNewOffset,
                  nNewLength);
}

void SAL_CALL Inflater::setDictionary( const com::sun::star::uno::Sequence< sal_Int8 >& rBuffer )
        throw(com::sun::star::uno::RuntimeException)
{
    if (pStream == NULL)
    {
        // do error handling
    }
    z_inflateSetDictionary(pStream, (const unsigned char*)rBuffer.getConstArray(),
                   rBuffer.getLength());
}

sal_Int32 SAL_CALL Inflater::getRemaining(  )
        throw(com::sun::star::uno::RuntimeException)
{
    return nLength;
}

sal_Bool SAL_CALL Inflater::needsInput(  )
        throw(com::sun::star::uno::RuntimeException)
{
    return nLength <=0;
}

sal_Bool SAL_CALL Inflater::needsDictionary(  )
        throw(com::sun::star::uno::RuntimeException)
{
    return bNeedDict;
}

sal_Bool SAL_CALL Inflater::finished(  )
        throw(com::sun::star::uno::RuntimeException)
{
    return bFinished;
}

sal_Int32 SAL_CALL Inflater::doInflateSegment( com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
        throw(com::sun::star::uno::RuntimeException)
{
    if (nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength())
    {
        // do error handling
    }
    return doInflateBytes(rBuffer, nNewOffset, nNewLength);
}

sal_Int32 SAL_CALL Inflater::doInflate( com::sun::star::uno::Sequence< sal_Int8 >& rBuffer )
        throw(com::sun::star::uno::RuntimeException)
{
    return doInflateBytes(rBuffer, 0, rBuffer.getLength());
}

sal_Int32 SAL_CALL Inflater::getAdler(  )
        throw(com::sun::star::uno::RuntimeException)
{
    return pStream->adler;
}

sal_Int32 SAL_CALL Inflater::getTotalIn(  )
        throw(com::sun::star::uno::RuntimeException)
{
    return pStream->total_in;
}

sal_Int32 SAL_CALL Inflater::getTotalOut(  )
        throw(com::sun::star::uno::RuntimeException)
{
    return pStream->total_out;
}

void SAL_CALL Inflater::reset(  )
        throw(com::sun::star::uno::RuntimeException)
{
    z_inflateReset(pStream);
    bNeedDict = sal_False;
    bFinished = sal_False;
    nOffset = nLength = 0;
}

void SAL_CALL Inflater::end(  )
        throw(com::sun::star::uno::RuntimeException)
{
    z_inflateEnd(pStream);
    pStream = NULL;
}

sal_Int32 Inflater::doInflateBytes (com::sun::star::uno::Sequence < sal_Int8 >  &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    sal_Int32 nResult;
    pStream->next_in   = (unsigned char*) sInBuffer.getConstArray()+ nOffset;
    pStream->avail_in  = nLength;
    pStream->next_out  = (unsigned char*) rBuffer.getArray() + nNewOffset;
    pStream->avail_out = nNewLength;

    nResult = ::z_inflate(pStream, Z_PARTIAL_FLUSH);

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
            VOS_DEBUG_ONLY(pStream->msg);
            return 0;
    }
    return 0;
}

