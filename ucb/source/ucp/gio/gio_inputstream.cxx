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

#include <rtl/memory.h>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <string.h>

#include "gio_inputstream.hxx"
#include "gio_content.hxx"

using namespace com::sun::star;

namespace gio
{

InputStream::InputStream(GFileInputStream *pStream) : Seekable(G_SEEKABLE(pStream)), mpStream(pStream)
{
    if (!mpStream)
        throw io::NotConnectedException();
}

InputStream::~InputStream( void )
{
    closeInput();
}

sal_Int32 SAL_CALL InputStream::available()
    throw( io::NotConnectedException, io::IOException, uno::RuntimeException )
{
    return 0;
}

void SAL_CALL InputStream::closeInput()
    throw( io::NotConnectedException, io::IOException, uno::RuntimeException )
{
    if (mpStream)
        g_input_stream_close(G_INPUT_STREAM(mpStream), NULL, NULL);
}

void SAL_CALL InputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw( io::NotConnectedException, io::BufferSizeExceededException,
      io::IOException, uno::RuntimeException )
{
    if (!mpStream)
        throw io::NotConnectedException();

    if (!g_seekable_can_seek(G_SEEKABLE(mpStream)))
        throw io::IOException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Seek unsupported")),
            static_cast< cppu::OWeakObject * >(this));

    GError *pError=NULL;
    if (!g_seekable_seek(G_SEEKABLE(mpStream), nBytesToSkip, G_SEEK_CUR, NULL, &pError))
        convertToException(pError, static_cast< cppu::OWeakObject * >(this));
}

sal_Int32 SAL_CALL InputStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
    throw( io::NotConnectedException, io::BufferSizeExceededException,
      io::IOException, uno::RuntimeException )
{
    if (!mpStream)
        throw io::NotConnectedException();

    try
    {
        aData.realloc( nBytesToRead );
    }
    catch ( const uno::Exception &e )
    {
        throw io::BufferSizeExceededException();
    }

    gsize nBytesRead = 0;
    GError *pError=NULL;
    if (!g_input_stream_read_all(G_INPUT_STREAM(mpStream), aData.getArray(), nBytesToRead, &nBytesRead, NULL, &pError))
        convertToException(pError, static_cast< cppu::OWeakObject * >(this));
    aData.realloc(nBytesRead);
    return nBytesRead;
}

sal_Int32 SAL_CALL InputStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
    throw( io::NotConnectedException, io::BufferSizeExceededException,
      io::IOException, uno::RuntimeException )
{
    return readBytes(aData, nMaxBytesToRead);
}

uno::Any InputStream::queryInterface( const uno::Type &type ) throw( uno::RuntimeException )
{
    uno::Any aRet = ::cppu::queryInterface ( type,
        static_cast< XInputStream * >( this ) );

    return aRet.hasValue() ? aRet : Seekable::queryInterface( type );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
