/*************************************************************************
 *
 *  $RCSfile: CRC32.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mtg $ $Date: 2001-05-31 09:47:40 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _CRC32_HXX
#include "CRC32.hxx"
#endif
#ifndef _ZLIB_H
#include <external/zlib/zlib.h>
#endif
#ifndef _PACKAGE_CONSTANTS_HXX_
#include <PackageConstants.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
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

void SAL_CALL CRC32::updateStream( Reference < XInputStream > & xStream )
{
    Reference < XSeekable > xSeek ( xStream, UNO_QUERY );
    if ( xSeek.is() )
    {
        sal_Int32 nLength = n_ConstBufferSize;
        sal_Int64 nCurrentPos = xSeek->getPosition();
        Sequence < sal_Int8 > aSeq ( n_ConstBufferSize );
        while ( nLength >= n_ConstBufferSize )
        {
            nLength = xStream->readBytes ( aSeq, n_ConstBufferSize );
            updateSegment ( aSeq, 0, nLength );
        }
        xSeek->seek ( nCurrentPos );
    }
}
