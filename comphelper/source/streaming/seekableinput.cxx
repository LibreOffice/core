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

#include <sal/config.h>

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XOutputStream.hpp>


#include <comphelper/processfactory.hxx>
#include <comphelper/seekableinput.hxx>

using namespace ::com::sun::star;

namespace comphelper
{

const sal_Int32 nConstBufferSize = 32000;


void copyInputToOutput_Impl( const uno::Reference< io::XInputStream >& xIn,
                            const uno::Reference< io::XOutputStream >& xOut )
{
    sal_Int32 nRead;
    uno::Sequence< sal_Int8 > aSequence( nConstBufferSize );

    do
    {
        nRead = xIn->readBytes( aSequence, nConstBufferSize );
        if ( nRead < nConstBufferSize )
        {
            uno::Sequence< sal_Int8 > aTempBuf( aSequence.getConstArray(), nRead );
            xOut->writeBytes( aTempBuf );
        }
        else
            xOut->writeBytes( aSequence );
    }
    while ( nRead == nConstBufferSize );
}


OSeekableInputWrapper::OSeekableInputWrapper(
            const uno::Reference< io::XInputStream >& xInStream,
            const uno::Reference< uno::XComponentContext >& rxContext )
: m_xContext( rxContext )
, m_xOriginalStream( xInStream )
{
    if ( !m_xContext.is() )
        throw uno::RuntimeException();
}


OSeekableInputWrapper::~OSeekableInputWrapper()
{
}


uno::Reference< io::XInputStream > OSeekableInputWrapper::CheckSeekableCanWrap(
                            const uno::Reference< io::XInputStream >& xInStream,
                            const uno::Reference< uno::XComponentContext >& rxContext )
{
    // check that the stream is seekable and just wrap it if it is not
    uno::Reference< io::XSeekable > xSeek( xInStream, uno::UNO_QUERY );
    if ( xSeek.is() )
        return xInStream;

    uno::Reference< io::XInputStream > xNewStream(
            static_cast< io::XInputStream* >(
                new OSeekableInputWrapper( xInStream, rxContext ) ) );
    return xNewStream;
}


void OSeekableInputWrapper::PrepareCopy_Impl()
{
    if ( !m_xCopyInput.is() )
    {
        if ( !m_xContext.is() )
            throw uno::RuntimeException();

        uno::Reference< io::XOutputStream > xTempOut(
                io::TempFile::create(m_xContext),
                uno::UNO_QUERY_THROW );

        copyInputToOutput_Impl( m_xOriginalStream, xTempOut );
        xTempOut->closeOutput();

        uno::Reference< io::XSeekable > xTempSeek( xTempOut, uno::UNO_QUERY );
        if ( xTempSeek.is() )
        {
            xTempSeek->seek( 0 );
            m_xCopyInput.set( xTempOut, uno::UNO_QUERY );
            if ( m_xCopyInput.is() )
                m_xCopySeek = xTempSeek;
        }
    }

    if ( !m_xCopyInput.is() )
        throw io::IOException();
}

// XInputStream

sal_Int32 SAL_CALL OSeekableInputWrapper::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xOriginalStream.is() )
        throw io::NotConnectedException();

    PrepareCopy_Impl();

    return m_xCopyInput->readBytes( aData, nBytesToRead );
}


sal_Int32 SAL_CALL OSeekableInputWrapper::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xOriginalStream.is() )
        throw io::NotConnectedException();

    PrepareCopy_Impl();

    return m_xCopyInput->readSomeBytes( aData, nMaxBytesToRead );
}


void SAL_CALL OSeekableInputWrapper::skipBytes( sal_Int32 nBytesToSkip )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xOriginalStream.is() )
        throw io::NotConnectedException();

    PrepareCopy_Impl();

    m_xCopyInput->skipBytes( nBytesToSkip );
}


sal_Int32 SAL_CALL OSeekableInputWrapper::available()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xOriginalStream.is() )
        throw io::NotConnectedException();

    PrepareCopy_Impl();

    return m_xCopyInput->available();
}


void SAL_CALL OSeekableInputWrapper::closeInput()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xOriginalStream.is() )
        throw io::NotConnectedException();

    m_xOriginalStream->closeInput();
    m_xOriginalStream.clear();

    if ( m_xCopyInput.is() )
    {
        m_xCopyInput->closeInput();
        m_xCopyInput.clear();
    }

    m_xCopySeek.clear();
}


// XSeekable

void SAL_CALL OSeekableInputWrapper::seek( sal_Int64 location )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xOriginalStream.is() )
        throw io::NotConnectedException();

    PrepareCopy_Impl();

    m_xCopySeek->seek( location );
}


sal_Int64 SAL_CALL OSeekableInputWrapper::getPosition()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xOriginalStream.is() )
        throw io::NotConnectedException();

    PrepareCopy_Impl();

    return m_xCopySeek->getPosition();
}


sal_Int64 SAL_CALL OSeekableInputWrapper::getLength()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xOriginalStream.is() )
        throw io::NotConnectedException();

    PrepareCopy_Impl();

    return m_xCopySeek->getLength();
}

}   // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
