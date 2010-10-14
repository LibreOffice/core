/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include "oinputstreamcontainer.hxx"
#include <cppuhelper/typeprovider.hxx>

using namespace ::com::sun::star;

//-----------------------------------------------
OFSInputStreamContainer::OFSInputStreamContainer( const uno::Reference< io::XInputStream >& xStream )
: m_xInputStream( xStream )
, m_xSeekable( xStream, uno::UNO_QUERY )
, m_bSeekable( sal_False )
, m_bDisposed( sal_False )
, m_pListenersContainer( NULL )
{
    m_bSeekable = m_xSeekable.is();
}

//-----------------------------------------------
OFSInputStreamContainer::~OFSInputStreamContainer()
{
    if ( m_pListenersContainer )
    {
        delete m_pListenersContainer;
        m_pListenersContainer = NULL;
    }
}

//-----------------------------------------------
uno::Sequence< uno::Type > SAL_CALL OFSInputStreamContainer::getTypes()
        throw ( uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( m_aMutex ) ;

        if ( pTypeCollection == NULL )
        {
            if ( m_bSeekable )
            {
                static ::cppu::OTypeCollection aTypeCollection(
                        ::getCppuType(( const uno::Reference< io::XStream >* )NULL ),
                        ::getCppuType(( const uno::Reference< io::XInputStream >* )NULL ),
                        ::getCppuType(( const uno::Reference< io::XSeekable >* )NULL ) );

                pTypeCollection = &aTypeCollection ;
            }
            else
            {
                static ::cppu::OTypeCollection aTypeCollection(
                        ::getCppuType(( const uno::Reference< io::XStream >* )NULL ),
                        ::getCppuType(( const uno::Reference< io::XInputStream >* )NULL ) );

                pTypeCollection = &aTypeCollection ;
            }
        }
    }

    return pTypeCollection->getTypes() ;

}

//-----------------------------------------------
uno::Any SAL_CALL OFSInputStreamContainer::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    uno::Any aReturn;
    if ( m_bSeekable )
        aReturn = uno::Any( ::cppu::queryInterface( rType,
                                           static_cast< io::XStream* >( this ),
                                           static_cast< io::XInputStream* >( this ),
                                           static_cast< io::XSeekable* >( this ) ) );
    else
        aReturn = uno::Any( ::cppu::queryInterface( rType,
                                           static_cast< io::XStream* >( this ),
                                           static_cast< io::XInputStream* >( this ) ) );

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    return ::cppu::OWeakObject::queryInterface( rType ) ;
}

//-----------------------------------------------
void SAL_CALL OFSInputStreamContainer::acquire()
        throw()
{
    ::cppu::OWeakObject::acquire();
}

//-----------------------------------------------
void SAL_CALL OFSInputStreamContainer::release()
        throw()
{
    ::cppu::OWeakObject::release();
}

//-----------------------------------------------
sal_Int32 SAL_CALL OFSInputStreamContainer::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    return m_xInputStream->readBytes( aData, nBytesToRead );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OFSInputStreamContainer::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    return m_xInputStream->readSomeBytes( aData, nMaxBytesToRead );
}

//-----------------------------------------------
void SAL_CALL OFSInputStreamContainer::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    m_xInputStream->skipBytes( nBytesToSkip );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OFSInputStreamContainer::available(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    return m_xInputStream->available();
}

//-----------------------------------------------
void SAL_CALL OFSInputStreamContainer::closeInput(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    dispose();
}

//-----------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OFSInputStreamContainer::getInputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xInputStream.is() )
        return uno::Reference< io::XInputStream >();

    return uno::Reference< io::XInputStream >( static_cast< io::XInputStream* >( this ), uno::UNO_QUERY );
}

//-----------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OFSInputStreamContainer::getOutputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    return uno::Reference< io::XOutputStream >();
}

//-----------------------------------------------
void SAL_CALL OFSInputStreamContainer::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xSeekable->seek( location );
}

//-----------------------------------------------
sal_Int64 SAL_CALL OFSInputStreamContainer::getPosition()
        throw ( io::IOException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getPosition();
}

//-----------------------------------------------
sal_Int64 SAL_CALL OFSInputStreamContainer::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getLength();
}

//-----------------------------------------------
void SAL_CALL OFSInputStreamContainer::dispose(  )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    m_xInputStream->closeInput();

    if ( m_pListenersContainer )
    {
        lang::EventObject aSource( static_cast< ::cppu::OWeakObject*>( this ) );
        m_pListenersContainer->disposeAndClear( aSource );
    }

    m_bDisposed = sal_True;
}

//-----------------------------------------------
void SAL_CALL OFSInputStreamContainer::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pListenersContainer )
        m_pListenersContainer = new ::cppu::OInterfaceContainerHelper( m_aMutex );

    m_pListenersContainer->addInterface( xListener );
}

//-----------------------------------------------
void SAL_CALL OFSInputStreamContainer::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_pListenersContainer )
        m_pListenersContainer->removeInterface( xListener );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
