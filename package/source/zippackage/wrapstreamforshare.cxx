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
#include "precompiled_package.hxx"
#include <osl/diagnose.h>

#include "wrapstreamforshare.hxx"

using namespace ::com::sun::star;


WrapStreamForShare::WrapStreamForShare( const uno::Reference< io::XInputStream >& xInStream,
                                        const SotMutexHolderRef& rMutexRef )
: m_rMutexRef( rMutexRef )
, m_xInStream( xInStream )
, m_nCurPos( 0 )
{
    m_xSeekable = uno::Reference< io::XSeekable >( m_xInStream, uno::UNO_QUERY );
    if ( !m_rMutexRef.Is() || !m_xInStream.is() || !m_xSeekable.is() )
    {
        OSL_FAIL( "Wrong initialization of wrapping stream!\n" );
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    }
}

WrapStreamForShare::~WrapStreamForShare()
{
}

// XInputStream
sal_Int32 SAL_CALL WrapStreamForShare::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    m_xSeekable->seek( m_nCurPos );

    sal_Int32 nRead = m_xInStream->readBytes( aData, nBytesToRead );
    m_nCurPos += nRead;

    return nRead;
}

sal_Int32 SAL_CALL WrapStreamForShare::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    m_xSeekable->seek( m_nCurPos );

    sal_Int32 nRead = m_xInStream->readSomeBytes( aData, nMaxBytesToRead );
    m_nCurPos += nRead;

    return nRead;
}

void SAL_CALL WrapStreamForShare::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    m_xSeekable->seek( m_nCurPos );

    m_xInStream->skipBytes( nBytesToSkip );
    m_nCurPos = m_xSeekable->getPosition();
}

sal_Int32 SAL_CALL WrapStreamForShare::available()
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    return m_xInStream->available();
}

void SAL_CALL WrapStreamForShare::closeInput()
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    // the package is the owner so it will close the stream
    // m_xInStream->closeInput();
    m_xInStream = uno::Reference< io::XInputStream >();
    m_xSeekable = uno::Reference< io::XSeekable >();
}

// XSeekable
void SAL_CALL WrapStreamForShare::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    // let stream implementation do all the checking
    m_xSeekable->seek( location );

    m_nCurPos = m_xSeekable->getPosition();
}

sal_Int64 SAL_CALL WrapStreamForShare::getPosition()
        throw ( io::IOException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    return m_nCurPos;
}

sal_Int64 SAL_CALL WrapStreamForShare::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    return m_xSeekable->getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
