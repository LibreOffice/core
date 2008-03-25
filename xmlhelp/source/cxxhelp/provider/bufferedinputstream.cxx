/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bufferedinputstream.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:19:52 $
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
#include "precompiled_xmlhelp.hxx"
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#include "bufferedinputstream.hxx"


using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace chelp;


Reference<XInputStream> chelp::turnToSeekable(const Reference<XInputStream>& xInputStream)
{
    if( ! xInputStream.is() )
        return xInputStream;

    Reference<XSeekable> xSeekable(xInputStream,UNO_QUERY);

    if( xSeekable.is() )
        return xInputStream;

    return new BufferedInputStream(xInputStream);
}



BufferedInputStream::BufferedInputStream(const Reference<XInputStream>& xInputStream)
    : m_nBufferLocation(0),
      m_nBufferSize(0),
      m_pBuffer(new sal_Int8[0])
{
    try
    {
        sal_Int32 num;
        sal_Int8  *tmp;
        Sequence< sal_Int8 > aData(4096);
        do{
            num = xInputStream->readBytes(aData,4096);
            if( num > 0 )
            {
                tmp = m_pBuffer;
                m_pBuffer = new sal_Int8[m_nBufferSize+num];
                rtl_copyMemory((void *)(m_pBuffer),
                               (void *)(tmp),
                               sal_uInt32(m_nBufferSize));
                rtl_copyMemory((void *)(m_pBuffer+m_nBufferSize),
                               (void *)(aData.getArray()),
                               sal_uInt32(num));
                m_nBufferSize += num;
                delete[] tmp;
            }
        } while( num == 4096 );
    }
    catch( const NotConnectedException&)
    {
    }
    catch( const BufferSizeExceededException&)
    {
    }
    catch( const IOException&)
    {
    }
    catch( const RuntimeException&)
    {
    }
    xInputStream->closeInput();
}


BufferedInputStream::~BufferedInputStream()
{
    delete[] m_pBuffer;
}


Any SAL_CALL BufferedInputStream::queryInterface( const Type& rType ) throw( RuntimeException )
{
    Any aRet = ::cppu::queryInterface( rType,
                                       SAL_STATIC_CAST( XInputStream*,this ),
                                       SAL_STATIC_CAST( XSeekable*,this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL BufferedInputStream::acquire( void ) throw()
{
    OWeakObject::acquire();
}


void SAL_CALL BufferedInputStream::release( void ) throw()
{
    OWeakObject::release();
}



sal_Int32 SAL_CALL BufferedInputStream::readBytes( Sequence< sal_Int8 >& aData,sal_Int32 nBytesToRead )
    throw( NotConnectedException,
           BufferSizeExceededException,
           IOException,
           RuntimeException)
{
    osl::MutexGuard aGuard( m_aMutex );

    if( 0 > nBytesToRead )
        throw BufferSizeExceededException();

    if( m_nBufferLocation + nBytesToRead > m_nBufferSize )
        nBytesToRead = m_nBufferSize - m_nBufferLocation;

    if( aData.getLength() < nBytesToRead )
        aData.realloc(nBytesToRead);

    rtl_copyMemory((void*)(aData.getArray()),
                   (void*)(m_pBuffer+m_nBufferLocation),
                   nBytesToRead);

    return nBytesToRead;
}


sal_Int32 SAL_CALL BufferedInputStream::readSomeBytes(
    Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead )
    throw( NotConnectedException,
           BufferSizeExceededException,
           IOException,
           RuntimeException)
{
    return readBytes(aData,nMaxBytesToRead);
}



void SAL_CALL BufferedInputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw( NotConnectedException,
           BufferSizeExceededException,
           IOException,
           RuntimeException )
{
    try
    {
        seek(m_nBufferLocation+nBytesToSkip);
    }
    catch( const IllegalArgumentException& )
    {
        throw BufferSizeExceededException();
    }
}



sal_Int32 SAL_CALL BufferedInputStream::available( void )
    throw( NotConnectedException,
           IOException,
           RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    return  m_nBufferSize-m_nBufferLocation;
}



void SAL_CALL BufferedInputStream::closeInput( void )
    throw( NotConnectedException,
           IOException,
           RuntimeException )
{
}


void SAL_CALL BufferedInputStream::seek( sal_Int64 location )
    throw( IllegalArgumentException,
           IOException,
           RuntimeException )
{
    if( 0 <= location && location < m_nBufferSize )
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_nBufferLocation = sal::static_int_cast<sal_Int32>( location );
    }
    else
        throw IllegalArgumentException();
}



sal_Int64 SAL_CALL BufferedInputStream::getPosition( void )
    throw( IOException,
           RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    return m_nBufferLocation;
}



sal_Int64 SAL_CALL BufferedInputStream::getLength( void ) throw( IOException,RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    return m_nBufferSize;
}
