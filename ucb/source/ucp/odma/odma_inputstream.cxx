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
#include "precompiled_ucb.hxx"
#include "odma_inputstream.hxx"
#include "com/sun/star/io/IOException.hpp"
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <ucbhelper/content.hxx>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <cppuhelper/implbase1.hxx>
#include "odma_contentprops.hxx"
#include "odma_provider.hxx"

using namespace odma;
using namespace com::sun::star;

class OActiveDataStreamer : public ::cppu::WeakImplHelper1< io::XActiveDataStreamer>
{
    uno::Reference< io::XStream > m_xStream;
public:
    OActiveDataStreamer(){}
    virtual void SAL_CALL setStream( const uno::Reference< io::XStream >& _rStream ) throw (uno::RuntimeException)
    {
        m_xStream = _rStream;
    }
    virtual uno::Reference< io::XStream > SAL_CALL getStream(  ) throw (uno::RuntimeException)
    {
        return m_xStream;
    }
};
// -----------------------------------------------------------------------------
OOdmaStream::OOdmaStream(::ucbhelper::Content* _pContent,
                         ContentProvider* _pProvider,
                         const ::rtl::Reference<ContentProperties>& _rProp)
 :m_pContent(_pContent)
 ,m_bInputStreamCalled(sal_False)
 ,m_bOutputStreamCalled(sal_False)
 ,m_bModified(sal_False)
 ,m_pProvider(_pProvider)
 ,m_aProp(_rProp)
{
}
// -----------------------------------------------------------------------------
OOdmaStream::~OOdmaStream()
{
    try
    {
        closeStream();
        delete m_pContent;
    }
    catch (io::IOException const &)
    {
        OSL_ENSURE(false, "unexpected situation");
    }
    catch (uno::RuntimeException const &)
    {
        OSL_ENSURE(false, "unexpected situation");
    }
}
// -----------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OOdmaStream::getInputStream(  ) throw( uno::RuntimeException)
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bInputStreamCalled = sal_True;
    }
    return uno::Reference< io::XInputStream >( this );
}
// -----------------------------------------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OOdmaStream::getOutputStream(  ) throw( uno::RuntimeException )
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bOutputStreamCalled = sal_True;
    }
    return uno::Reference< io::XOutputStream >( this );
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OOdmaStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    ensureInputStream();

    return m_xInput->readBytes(aData,nBytesToRead);
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OOdmaStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    return readBytes( aData,nMaxBytesToRead );
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::skipBytes( sal_Int32 nBytesToSkip )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    ensureInputStream();
    m_xInput->skipBytes(nBytesToSkip );
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OOdmaStream::available()
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException)
{
    ensureInputStream();
    return m_xInput->available();
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    ensureOutputStream();
    m_xOutput->writeBytes(aData);
    m_bModified = sal_True;
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::closeStream() throw( io::NotConnectedException,io::IOException,uno::RuntimeException )
{
    if( m_xInput.is() )
    {
        m_xInput->closeInput();
        m_xInput        = NULL;
        m_xInputSeek    = NULL;
    }
    if(m_xOutput.is())
    {
        m_xOutput->closeOutput();
        m_xOutput       = NULL;
        m_xTruncate     = NULL;
        if(m_bModified)
            m_pProvider->saveDocument(m_aProp->m_sDocumentId);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::closeInput()
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bInputStreamCalled = sal_False;

    if( ! m_bOutputStreamCalled )
        closeStream();
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::closeOutput()
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bOutputStreamCalled = sal_False;

    if( ! m_bInputStreamCalled )
        closeStream();
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::flush()
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    ensureOutputStream();
    m_xOutput->flush();
}
// -----------------------------------------------------------------------------
void OOdmaStream::ensureInputStream() throw( io::IOException )
{
    try
    {
        if(!m_xInput.is())
        {
            m_xInput = m_pContent->openStream();
            m_xInputSeek = uno::Reference< io::XSeekable>(m_xInput,uno::UNO_QUERY);
        }
    }
    catch(const uno::Exception&)
    {
    }
    if(!m_xInput.is())
        throw io::IOException();
}
// -----------------------------------------------------------------------------
void OOdmaStream::ensureOutputStream() throw( io::IOException )
{
    try
    {
        if(!m_xOutput.is())
        {
            ucb::OpenCommandArgument2 aCommand;
            aCommand.Mode = ucb::OpenMode::DOCUMENT;
            uno::Reference< io::XActiveDataStreamer > xActiveStreamer = new OActiveDataStreamer();
            aCommand.Sink = xActiveStreamer;
            m_pContent->executeCommand(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open")),uno::makeAny(aCommand));
            if(xActiveStreamer.is())
            {
                uno::Reference< io::XStream> xStream = xActiveStreamer->getStream();
                if(xStream.is())
                    m_xOutput = xStream->getOutputStream();
            }
        }
    }
    catch(const uno::Exception&)
    {
    }
    if(!m_xOutput.is())
        throw io::IOException();
    m_xTruncate = uno::Reference< io::XTruncate>(m_xOutput,uno::UNO_QUERY);
}
// -----------------------------------------------------------------------------
// XTruncate
void SAL_CALL OOdmaStream::truncate( void )
    throw( io::IOException,
           uno::RuntimeException )
{
    if(m_xTruncate.is())
        m_xTruncate->truncate();
}
// -----------------------------------------------------------------------------
// XSeekable
void SAL_CALL OOdmaStream::seek(sal_Int64 location )
    throw( lang::IllegalArgumentException,
           io::IOException,
           uno::RuntimeException )
{
    ensureInputStream();
    if(m_xInputSeek.is())
        m_xInputSeek->seek(location);
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL OOdmaStream::getPosition()
    throw( io::IOException,
           uno::RuntimeException )
{
    ensureInputStream();
    return m_xInputSeek.is() ? m_xInputSeek->getPosition() : sal_Int64(0);
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL OOdmaStream::getLength()
    throw( io::IOException,
           uno::RuntimeException )
{
    ensureInputStream();
    return m_xInputSeek.is() ? m_xInputSeek->getLength() : sal_Int64(0);
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
