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

#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include "selfterminatefilestream.hxx"
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

OSelfTerminateFileStream::OSelfTerminateFileStream( const uno::Reference< uno::XComponentContext > xContext, const OUString& aURL )
: m_aURL( aURL )
{
    uno::Reference< uno::XComponentContext > xOwnContext = xContext;
    if ( !xOwnContext.is() )
        xOwnContext.set( ::comphelper::getProcessComponentContext(), uno::UNO_SET_THROW );

    // IMPORTANT: The implementation is based on idea that m_xFileAccess, m_xInputStream and m_xSeekable are always set
    // otherwise an exception is thrown in constructor

    m_xFileAccess.set( ucb::SimpleFileAccess::create(xOwnContext) );

    m_xInputStream.set( m_xFileAccess->openFileRead( aURL ), uno::UNO_SET_THROW );
    m_xSeekable.set( m_xInputStream, uno::UNO_QUERY_THROW );
}

OSelfTerminateFileStream::~OSelfTerminateFileStream()
{
    CloseStreamDeleteFile();
}

void OSelfTerminateFileStream::CloseStreamDeleteFile()
{
    try
    {
        m_xInputStream->closeInput();
    }
    catch( uno::Exception& )
    {}

    try
    {
        m_xFileAccess->kill( m_aURL );
    }
    catch( uno::Exception& )
    {}
}

sal_Int32 SAL_CALL OSelfTerminateFileStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    return m_xInputStream->readBytes( aData, nBytesToRead );
}

sal_Int32 SAL_CALL OSelfTerminateFileStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    return m_xInputStream->readSomeBytes( aData, nMaxBytesToRead );
}

void SAL_CALL OSelfTerminateFileStream::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    return m_xInputStream->skipBytes( nBytesToSkip );
}

sal_Int32 SAL_CALL OSelfTerminateFileStream::available(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    return m_xInputStream->available();
}

void SAL_CALL OSelfTerminateFileStream::closeInput(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    CloseStreamDeleteFile();
}

void SAL_CALL OSelfTerminateFileStream::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    m_xSeekable->seek( location );
}

sal_Int64 SAL_CALL OSelfTerminateFileStream::getPosition()
        throw ( io::IOException,
                uno::RuntimeException)
{
    return m_xSeekable->getPosition();
}

sal_Int64 SAL_CALL OSelfTerminateFileStream::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    return m_xSeekable->getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
