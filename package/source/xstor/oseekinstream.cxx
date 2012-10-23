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

#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>

#include "oseekinstream.hxx"
#include "owriteablestream.hxx"

using namespace ::com::sun::star;

OInputSeekStream::OInputSeekStream( OWriteStream_Impl& pImpl,
                                    uno::Reference < io::XInputStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                    sal_Int32 nStorageType )
: OInputCompStream( pImpl, xStream, aProps, nStorageType )
{
    if ( m_xStream.is() )
    {
        m_xSeekable = uno::Reference< io::XSeekable >( m_xStream, uno::UNO_QUERY );
        OSL_ENSURE( m_xSeekable.is(), "No seeking support!\n" );
    }
}

OInputSeekStream::OInputSeekStream( uno::Reference < io::XInputStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                    sal_Int32 nStorageType )
: OInputCompStream( xStream, aProps, nStorageType )
{
    if ( m_xStream.is() )
    {
        m_xSeekable = uno::Reference< io::XSeekable >( m_xStream, uno::UNO_QUERY );
        OSL_ENSURE( m_xSeekable.is(), "No seeking support!\n" );
    }
}

OInputSeekStream::~OInputSeekStream()
{
}

uno::Sequence< uno::Type > SAL_CALL OInputSeekStream::getTypes()
        throw ( uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

        if ( pTypeCollection == NULL )
        {
            static ::cppu::OTypeCollection aTypeCollection(
                    ::getCppuType(( const uno::Reference< io::XSeekable >* )NULL ),
                    OInputCompStream::getTypes() );

            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

uno::Any SAL_CALL OInputSeekStream::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    uno::Any aReturn( ::cppu::queryInterface( rType,
                                           static_cast< io::XSeekable* >( this ) ) );

    if ( aReturn.hasValue() == sal_True )
    {
        return aReturn ;
    }

    return OInputCompStream::queryInterface( rType ) ;
}

void SAL_CALL OInputSeekStream::acquire()
        throw()
{
    OInputCompStream::acquire();
}

void SAL_CALL OInputSeekStream::release()
        throw()
{
    OInputCompStream::release();
}


void SAL_CALL OInputSeekStream::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( !m_xSeekable.is() )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "No seekable!" );
        throw uno::RuntimeException();
    }

    m_xSeekable->seek( location );
}

sal_Int64 SAL_CALL OInputSeekStream::getPosition()
        throw ( io::IOException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( !m_xSeekable.is() )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "No seekable!" );
        throw uno::RuntimeException();
    }

    return m_xSeekable->getPosition();
}

sal_Int64 SAL_CALL OInputSeekStream::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( !m_xSeekable.is() )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "No seekable!" );
        throw uno::RuntimeException();
    }

    return m_xSeekable->getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
