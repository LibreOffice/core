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

#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

#include "selfterminatefilestream.hxx"
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

//-----------------------------------------------
OSelfTerminateFileStream::OSelfTerminateFileStream( const uno::Reference< lang::XMultiServiceFactory > xFactory, const ::rtl::OUString& aURL )
: m_aURL( aURL )
{
    uno::Reference< lang::XMultiServiceFactory > xOwnFactory = xFactory;
    if ( !xOwnFactory.is() )
        xOwnFactory.set( ::comphelper::getProcessServiceFactory(), uno::UNO_SET_THROW );

    // IMPORTANT: The implementation is based on idea that m_xFileAccess, m_xInputStream and m_xSeekable are always set
    // otherwise an exception is thrown in constructor

    m_xFileAccess.set( xOwnFactory->createInstance (
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SimpleFileAccess") ) ),
                       uno::UNO_QUERY_THROW );

    m_xInputStream.set( m_xFileAccess->openFileRead( aURL ), uno::UNO_SET_THROW );
    m_xSeekable.set( m_xInputStream, uno::UNO_QUERY_THROW );
}

//-----------------------------------------------
OSelfTerminateFileStream::~OSelfTerminateFileStream()
{
    CloseStreamDeleteFile();
}

//-----------------------------------------------
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

//-----------------------------------------------
sal_Int32 SAL_CALL OSelfTerminateFileStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    return m_xInputStream->readBytes( aData, nBytesToRead );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OSelfTerminateFileStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    return m_xInputStream->readSomeBytes( aData, nMaxBytesToRead );
}

//-----------------------------------------------
void SAL_CALL OSelfTerminateFileStream::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    return m_xInputStream->skipBytes( nBytesToSkip );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OSelfTerminateFileStream::available(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    return m_xInputStream->available();
}

//-----------------------------------------------
void SAL_CALL OSelfTerminateFileStream::closeInput(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    CloseStreamDeleteFile();
}

//-----------------------------------------------
void SAL_CALL OSelfTerminateFileStream::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    m_xSeekable->seek( location );
}

//-----------------------------------------------
sal_Int64 SAL_CALL OSelfTerminateFileStream::getPosition()
        throw ( io::IOException,
                uno::RuntimeException)
{
    return m_xSeekable->getPosition();
}

//-----------------------------------------------
sal_Int64 SAL_CALL OSelfTerminateFileStream::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    return m_xSeekable->getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
