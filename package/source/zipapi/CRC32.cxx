/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CRC32.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:26:29 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"
#ifndef _CRC32_HXX
#include <CRC32.hxx>
#endif
#ifndef _ZLIB_H
#ifdef SYSTEM_ZLIB
#include <zlib.h>
#else
#include <external/zlib/zlib.h>
#endif
#endif
#ifndef _PACKAGE_CONSTANTS_HXX_
#include <PackageConstants.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;

/** A class to compute the CRC32 value of a data stream
 */

CRC32::CRC32()
: nCRC(0)
{
}
CRC32::~CRC32()
{
}
void SAL_CALL CRC32::reset()
    throw(RuntimeException)
{
    nCRC=0;
}
sal_Int32 SAL_CALL CRC32::getValue()
    throw(RuntimeException)
{
    return nCRC & 0xFFFFFFFFL;
}
/** Update CRC32 with specified byte
 */
void SAL_CALL CRC32::updateByte (sal_Int8 nByte)
        throw(RuntimeException)
{
    sal_uInt8 pBuf[1];
    pBuf[0] = (sal_uInt8)nByte;
    nCRC  = crc32(nCRC, pBuf, 1);
}
/** Update CRC32 with specified sequence of bytes
 */
void SAL_CALL CRC32::updateSegment(const Sequence< sal_Int8 > &b,
                                    sal_Int32 off,
                                    sal_Int32 len)
        throw(RuntimeException)
{
    nCRC = crc32(nCRC, (const unsigned char*)b.getConstArray()+off, len );
}
/** Update CRC32 with specified sequence of bytes
 */
void SAL_CALL CRC32::update(const Sequence< sal_Int8 > &b)
        throw(RuntimeException)
{
    nCRC = crc32(nCRC, (const unsigned char*)b.getConstArray(),b.getLength());
}

sal_Int32 SAL_CALL CRC32::updateStream( Reference < XInputStream > & xStream )
    throw ( RuntimeException )
{
    sal_Int32 nLength, nTotal = 0;
    Sequence < sal_Int8 > aSeq ( n_ConstBufferSize );
    do
    {
        nLength = xStream->readBytes ( aSeq, n_ConstBufferSize );
        updateSegment ( aSeq, 0, nLength );
        nTotal += nLength;
    }
    while ( nLength == n_ConstBufferSize );

    return nTotal;
}
