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
#include <unotools/streamwrap.hxx>

using namespace ::com::sun::star;

OSelfTerminateFileStream::OSelfTerminateFileStream( const uno::Reference< uno::XComponentContext >& xContext, utl::TempFileFast aTempFile )
: m_oTempFile( std::move(aTempFile) )
{
    uno::Reference< uno::XComponentContext > xOwnContext = xContext;
    if ( !xOwnContext.is() )
        xOwnContext.set( ::comphelper::getProcessComponentContext(), uno::UNO_SET_THROW );

    m_xStreamWrapper = new utl::OSeekableInputStreamWrapper( m_oTempFile->GetStream(StreamMode::READWRITE), /*bOwner*/false );
}

OSelfTerminateFileStream::~OSelfTerminateFileStream()
{
    CloseStreamDeleteFile();
}

void OSelfTerminateFileStream::CloseStreamDeleteFile()
{
    try
    {
        m_xStreamWrapper->closeInput();
    }
    catch( uno::Exception& )
    {}

    m_oTempFile.reset();
}

sal_Int32 SAL_CALL OSelfTerminateFileStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    return m_xStreamWrapper->readBytes( aData, nBytesToRead );
}

sal_Int32 SAL_CALL OSelfTerminateFileStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    return m_xStreamWrapper->readSomeBytes( aData, nMaxBytesToRead );
}

sal_Int32 OSelfTerminateFileStream::readSomeBytes( sal_Int8* aData, sal_Int32 nMaxBytesToRead )
{
    return m_xStreamWrapper->readSomeBytes( aData, nMaxBytesToRead );
}

void SAL_CALL OSelfTerminateFileStream::skipBytes( sal_Int32 nBytesToSkip )
{
    return m_xStreamWrapper->skipBytes( nBytesToSkip );
}

sal_Int32 SAL_CALL OSelfTerminateFileStream::available(  )
{
    return m_xStreamWrapper->available();
}

void SAL_CALL OSelfTerminateFileStream::closeInput(  )
{
    CloseStreamDeleteFile();
}

void SAL_CALL OSelfTerminateFileStream::seek( sal_Int64 location )
{
    m_xStreamWrapper->seek( location );
}

sal_Int64 SAL_CALL OSelfTerminateFileStream::getPosition()
{
    return m_xStreamWrapper->getPosition();
}

sal_Int64 SAL_CALL OSelfTerminateFileStream::getLength()
{
    return m_xStreamWrapper->getLength();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
