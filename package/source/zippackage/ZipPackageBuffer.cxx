/*************************************************************************
 *
 *  $RCSfile: ZipPackageBuffer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mtg $ $Date: 2000-11-23 14:15:52 $
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
#ifndef _ZIP_PACKAGE_BUFFER_HXX
#include "ZipPackageBuffer.hxx"
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::io;

ZipPackageBuffer::ZipPackageBuffer(sal_Int64 nNewBufferSize)
: nBufferSize (nNewBufferSize)
, aBuffer (nNewBufferSize)
, nCurrent(0)
, nEnd(0)
{
}
ZipPackageBuffer::~ZipPackageBuffer(void)
{
}
Any SAL_CALL  ZipPackageBuffer::queryInterface( const Type& rType )
        throw(RuntimeException)
{
    Any aReturn( ::cppu::queryInterface
                (   rType, static_cast< com::sun::star::io::XInputStream*>  ( this ),
                           static_cast< com::sun::star::io::XSeekable*> ( this ),
                           static_cast< com::sun::star::io::XOutputStream*> ( this )));
    if ( aReturn.hasValue () == sal_True )
        return aReturn ;
    else
        return OWeakObject::queryInterface ( rType ) ;
}
void SAL_CALL  ZipPackageBuffer::acquire(void)
    throw()
{
    OWeakObject::acquire();
}
void SAL_CALL  ZipPackageBuffer::release(void)
    throw()
{
    OWeakObject::release();
}
sal_Int32 SAL_CALL ZipPackageBuffer::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if (nBytesToRead + nCurrent > nEnd)
        nBytesToRead = nEnd - nCurrent;
    sal_Int64 nEndRead = nBytesToRead+nCurrent;

    for (sal_Int64 i =0; nCurrent < nEndRead; nCurrent++, i++)
        aData[i] = aBuffer[nCurrent];
    return nBytesToRead;
}
sal_Int32 SAL_CALL ZipPackageBuffer::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    /*
    if (nMaxBytesToRead + nCurrent > nEnd)
        nMaxBytesToRead = nEnd - nCurrent;
    sal_Int64 nEndRead = nMaxBytesToRead+nCurrent;

    for (sal_Int64 i =0; nCurrent < nEndRead; nCurrent++, i++)
        aData[i] = aBuffer[nCurrent];
    return nMaxBytesToRead;
    */
    return readBytes(aData, nMaxBytesToRead);
}
void SAL_CALL ZipPackageBuffer::skipBytes( sal_Int32 nBytesToSkip )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    nCurrent+=nBytesToSkip;
}
sal_Int32 SAL_CALL ZipPackageBuffer::available(  )
        throw(NotConnectedException, IOException, RuntimeException)
{
    return nEnd - nCurrent;
}
void SAL_CALL ZipPackageBuffer::closeInput(  )
        throw(NotConnectedException, IOException, RuntimeException)
{
}
void SAL_CALL ZipPackageBuffer::writeBytes( const Sequence< sal_Int8 >& aData )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    sal_Int64 nDataLen = aData.getLength();
    if (nEnd + nDataLen > nBufferSize)
    {
        nBufferSize *=2;
        aBuffer.realloc(nBufferSize);
    }
    for (sal_Int64 i=0; i<nDataLen;i++,nCurrent++)
        aBuffer[nCurrent] = aData[i];
    if (nCurrent>nEnd)
        nEnd = nCurrent;
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
        throw(::com::sun::star::lang::IllegalArgumentException, IOException, RuntimeException)
{
    nCurrent = location;
}
sal_Int64 SAL_CALL ZipPackageBuffer::getPosition(  )
        throw(IOException, RuntimeException)
{
    sal_Int8 nP = aBuffer[nCurrent];
    sal_Int8 nR = aBuffer[nEnd];
    int i =0;
    return nCurrent;
}
sal_Int64 SAL_CALL ZipPackageBuffer::getLength(  )
        throw(IOException, RuntimeException)
{
    return nEnd;
}
