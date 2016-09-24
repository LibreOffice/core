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

#include <osl/diagnose.h>

#include "wrapstreamforshare.hxx"

using namespace ::com::sun::star;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

WrapStreamForShare::WrapStreamForShare( const uno::Reference< io::XInputStream >& xInStream,
                                        const rtl::Reference<SotMutexHolder>& rMutexRef )
: m_rMutexRef( rMutexRef )
, m_xInStream( xInStream )
, m_nCurPos( 0 )
{
    m_xSeekable.set( m_xInStream, uno::UNO_QUERY );
    if ( !m_rMutexRef.is() || !m_xInStream.is() || !m_xSeekable.is() )
    {
        OSL_FAIL( "Wrong initialization of wrapping stream!\n" );
        throw uno::RuntimeException(THROW_WHERE );
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
                uno::RuntimeException, std::exception )
{
    if ( !m_xInStream.is() )
        throw io::IOException(THROW_WHERE );

    m_xSeekable->seek( m_nCurPos );

    sal_Int32 nRead = m_xInStream->readBytes( aData, nBytesToRead );
    m_nCurPos += nRead;

    return nRead;
}

sal_Int32 SAL_CALL WrapStreamForShare::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException, std::exception )
{
    if ( !m_xInStream.is() )
        throw io::IOException(THROW_WHERE );

    m_xSeekable->seek( m_nCurPos );

    sal_Int32 nRead = m_xInStream->readSomeBytes( aData, nMaxBytesToRead );
    m_nCurPos += nRead;

    return nRead;
}

void SAL_CALL WrapStreamForShare::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException(THROW_WHERE );

    m_xSeekable->seek( m_nCurPos );

    m_xInStream->skipBytes( nBytesToSkip );
    m_nCurPos = m_xSeekable->getPosition();
}

sal_Int32 SAL_CALL WrapStreamForShare::available()
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException(THROW_WHERE );

    return m_xInStream->available();
}

void SAL_CALL WrapStreamForShare::closeInput()
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException(THROW_WHERE );

    // the package is the owner so it will close the stream
    // m_xInStream->closeInput();
    m_xInStream.clear();
    m_xSeekable.clear();
}

// XSeekable
void SAL_CALL WrapStreamForShare::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException(THROW_WHERE );

    // let stream implementation do all the checking
    m_xSeekable->seek( location );

    m_nCurPos = m_xSeekable->getPosition();
}

sal_Int64 SAL_CALL WrapStreamForShare::getPosition()
        throw ( io::IOException,
                uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException(THROW_WHERE );

    return m_nCurPos;
}

sal_Int64 SAL_CALL WrapStreamForShare::getLength()
        throw ( io::IOException,
                uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( !m_xInStream.is() )
        throw io::IOException(THROW_WHERE );

    return m_xSeekable->getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
