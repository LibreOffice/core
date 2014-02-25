/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <string.h>

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
      m_pBuffer(new sal_Int8[1]) // Initialize with one to avoid gcc compiler warnings
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
                memcpy((void *)(m_pBuffer),
                               (void *)(tmp),
                               sal_uInt32(m_nBufferSize));
                memcpy((void *)(m_pBuffer+m_nBufferSize),
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


Any SAL_CALL BufferedInputStream::queryInterface( const Type& rType ) throw( RuntimeException, std::exception )
{
    Any aRet = ::cppu::queryInterface( rType,
                                       (static_cast< XInputStream* >(this)),
                                       (static_cast< XSeekable* >(this)) );

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
           RuntimeException, std::exception)
{
    osl::MutexGuard aGuard( m_aMutex );

    if( 0 > nBytesToRead )
        throw BufferSizeExceededException();

    if( m_nBufferLocation + nBytesToRead > m_nBufferSize )
        nBytesToRead = m_nBufferSize - m_nBufferLocation;

    if( aData.getLength() < nBytesToRead )
        aData.realloc(nBytesToRead);

    memcpy((void*)(aData.getArray()),
                   (void*)(m_pBuffer+m_nBufferLocation),
                   nBytesToRead);

    return nBytesToRead;
}


sal_Int32 SAL_CALL BufferedInputStream::readSomeBytes(
    Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead )
    throw( NotConnectedException,
           BufferSizeExceededException,
           IOException,
           RuntimeException, std::exception)
{
    return readBytes(aData,nMaxBytesToRead);
}



void SAL_CALL BufferedInputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw( NotConnectedException,
           BufferSizeExceededException,
           IOException,
           RuntimeException, std::exception )
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
           RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );
    return  m_nBufferSize-m_nBufferLocation;
}



void SAL_CALL BufferedInputStream::closeInput( void )
    throw( NotConnectedException,
           IOException,
           RuntimeException, std::exception )
{
}


void SAL_CALL BufferedInputStream::seek( sal_Int64 location )
    throw( IllegalArgumentException,
           IOException,
           RuntimeException, std::exception )
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
           RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );
    return m_nBufferLocation;
}



sal_Int64 SAL_CALL BufferedInputStream::getLength( void ) throw( IOException,RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );
    return m_nBufferSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
