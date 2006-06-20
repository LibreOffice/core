/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ZipPackageBuffer.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:14:28 $
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
#ifndef _ZIP_PACKAGE_BUFFER_HXX
#include <ZipPackageBuffer.hxx>
#endif
#include <string.h> // for memcpy


using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using com::sun::star::lang::IllegalArgumentException;

ZipPackageBuffer::ZipPackageBuffer(sal_Int64 nNewBufferSize )
: m_nBufferSize (nNewBufferSize)
, m_nEnd(0)
, m_nCurrent(0)
, m_bMustInitBuffer ( sal_True )
{
}
ZipPackageBuffer::ZipPackageBuffer(Sequence < sal_Int8 > &nNewBuffer )
: m_aBuffer ( nNewBuffer )
, m_nEnd ( nNewBuffer.getLength() )
, m_nCurrent( 0 )
, m_bMustInitBuffer ( sal_False )
{
}
ZipPackageBuffer::~ZipPackageBuffer(void)
{
}

sal_Int32 SAL_CALL ZipPackageBuffer::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if (nBytesToRead < 0)
        throw BufferSizeExceededException(::rtl::OUString(),*this);

    if (nBytesToRead + m_nCurrent > m_nEnd)
        nBytesToRead = static_cast < sal_Int32 > (m_nEnd - m_nCurrent);

    aData.realloc ( nBytesToRead );
    memcpy(aData.getArray(), m_aBuffer.getConstArray() + m_nCurrent, nBytesToRead);
    m_nCurrent +=nBytesToRead;
    return nBytesToRead;
}

sal_Int32 SAL_CALL ZipPackageBuffer::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return readBytes(aData, nMaxBytesToRead);
}
void SAL_CALL ZipPackageBuffer::skipBytes( sal_Int32 nBytesToSkip )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if (nBytesToSkip < 0)
        throw BufferSizeExceededException(::rtl::OUString(),*this);

    if (nBytesToSkip + m_nCurrent > m_nEnd)
        nBytesToSkip = static_cast < sal_Int32 > (m_nEnd - m_nCurrent);

    m_nCurrent+=nBytesToSkip;
}
sal_Int32 SAL_CALL ZipPackageBuffer::available(  )
        throw(NotConnectedException, IOException, RuntimeException)
{
    return static_cast < sal_Int32 > (m_nEnd - m_nCurrent);
}
void SAL_CALL ZipPackageBuffer::closeInput(  )
        throw(NotConnectedException, IOException, RuntimeException)
{
}
void SAL_CALL ZipPackageBuffer::writeBytes( const Sequence< sal_Int8 >& aData )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    sal_Int64 nDataLen = aData.getLength(), nCombined = m_nEnd + nDataLen;

    if ( nCombined > m_nBufferSize)
    {
        do
            m_nBufferSize *=2;
        while (nCombined > m_nBufferSize);
        m_aBuffer.realloc(static_cast < sal_Int32 > (m_nBufferSize));
        m_bMustInitBuffer = sal_False;
    }
    else if (m_bMustInitBuffer)
    {
         m_aBuffer.realloc ( static_cast < sal_Int32 > ( m_nBufferSize ) );
        m_bMustInitBuffer = sal_False;
    }
    memcpy( m_aBuffer.getArray() + m_nCurrent, aData.getConstArray(), static_cast < sal_Int32 > (nDataLen));
    m_nCurrent+=nDataLen;
    if (m_nCurrent>m_nEnd)
        m_nEnd = m_nCurrent;
}
void SAL_CALL ZipPackageBuffer::flush(  )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
}
void SAL_CALL ZipPackageBuffer::closeOutput(  )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
}
void SAL_CALL ZipPackageBuffer::seek( sal_Int64 location )
        throw( IllegalArgumentException, IOException, RuntimeException)
{
    if ( location > m_nEnd || location < 0 )
        throw IllegalArgumentException();
    m_nCurrent = location;
}
sal_Int64 SAL_CALL ZipPackageBuffer::getPosition(  )
        throw(IOException, RuntimeException)
{
    return m_nCurrent;
}
sal_Int64 SAL_CALL ZipPackageBuffer::getLength(  )
        throw(IOException, RuntimeException)
{
    return m_nEnd;
}
