/*************************************************************************
 *
 *  $RCSfile: EntryInputStream.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: mtg $ $Date: 2001-04-19 14:13:40 $
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
#ifndef _ENTRY_INPUT_STREAM_HXX
#include "EntryInputStream.hxx"
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/ZipConstants.hpp>
#endif
#include <memory.h> // for memcpy

using namespace rtl;
using namespace com::sun::star;

/** Provides access to the compressed data in a zipfile.
 *
 * 04/12/00 - uncompresses the stream into memory and seeks on it 'in memory'
 * This is a "temporary" fix which will be changed ASAP (read 2001)
 *
 * Acts on the same underlying XInputStream as both the full Zip File and other
 * EntryInputStreams, and thus must maintain its current position in the stream and
 * seek to it before performing any reads.
 */

EntryInputStream::EntryInputStream( uno::Reference < io::XInputStream > xNewInput,
                                    sal_Int64 nNewBegin,
                                    sal_Int64 nNewEnd,
                                    sal_Int64 nNewUncompressedSize,
                                    sal_Bool bIsDeflated)
: xStream( xNewInput )
, xSeek( xNewInput, uno::UNO_QUERY )
, nBegin( nNewBegin )
, nEnd ( nNewEnd )
, nCurrent( 0 )
, nUncompressedSize (nNewUncompressedSize)
, aSequence ( 0 )
, bReachEOF ( sal_False )
, bHaveInMemory ( sal_False )
, aInflater( sal_True )
, aBuffer( 0 )
, bDeflated ( bIsDeflated )
{
}
void EntryInputStream::readIntoMemory()
{
    if (!bHaveInMemory)
    {
        aBuffer.realloc ( static_cast < sal_Int32 > ( nUncompressedSize ) );
        if (bDeflated)
        {
            sal_Int32 nSize = static_cast < sal_Int32 > (nEnd - nBegin );
            aSequence.realloc( nSize );
            xSeek->seek(nBegin);
            xStream->readBytes(aSequence, nSize );
            aInflater.setInputSegment(aSequence, 0, nSize );
            aInflater.doInflate(aBuffer);
            aInflater.end();
            aSequence.realloc( 0 );
        }
        else
        {
            xSeek->seek(nBegin);
            xStream->readBytes(aBuffer, static_cast < sal_Int32 > (nUncompressedSize));
        }
        bHaveInMemory = sal_True;
    }
}
EntryInputStream::~EntryInputStream( void )
{
}

sal_Int32 SAL_CALL EntryInputStream::readBytes( uno::Sequence< sal_Int8 >& aData,
                                        sal_Int32 nBytesToRead )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    if (nBytesToRead <0)
        throw io::BufferSizeExceededException(::rtl::OUString(), *this);
    if (!bHaveInMemory)
        readIntoMemory();
    if (nBytesToRead + nCurrent > nUncompressedSize)
        nBytesToRead = static_cast < sal_Int32> (nUncompressedSize - nCurrent);

    aData.realloc( nBytesToRead );
    memcpy(aData.getArray(), aBuffer.getConstArray() + nCurrent, nBytesToRead);
    nCurrent+=nBytesToRead;

    return nBytesToRead;
}
sal_Int32 SAL_CALL EntryInputStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData,
                                                sal_Int32 nMaxBytesToRead )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    return readBytes( aData, nMaxBytesToRead );
}
void SAL_CALL EntryInputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    if (nBytesToSkip < 0)
        throw io::BufferSizeExceededException(::rtl::OUString(), *this);

    if (nBytesToSkip + nCurrent > nUncompressedSize )
        nBytesToSkip = static_cast < sal_Int32 > (nUncompressedSize - nCurrent);

    nCurrent+=nBytesToSkip;
}
sal_Int32 SAL_CALL EntryInputStream::available(  )
    throw(io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    return static_cast < sal_Int32 > (nUncompressedSize - nCurrent);
}
void SAL_CALL EntryInputStream::closeInput(  )
    throw(io::NotConnectedException, io::IOException, uno::RuntimeException)
{
}

void SAL_CALL EntryInputStream::seek( sal_Int64 location )
    throw(lang::IllegalArgumentException, io::IOException, uno::RuntimeException)
{
    if (location > nUncompressedSize)
        location = nUncompressedSize;
    if (location <0)
        location = 0;
    nCurrent = location;
}
sal_Int64 SAL_CALL EntryInputStream::getPosition(  )
        throw(io::IOException, uno::RuntimeException)
{
    return nCurrent;
}
sal_Int64 SAL_CALL EntryInputStream::getLength(  )
        throw(io::IOException, uno::RuntimeException)
{
    return nUncompressedSize;
}
