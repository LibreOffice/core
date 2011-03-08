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

#include "gio_outputstream.hxx"
#include "gio_content.hxx"

using namespace com::sun::star;

namespace gio
{

OutputStream::OutputStream(GFileOutputStream *pStream) : Seekable(G_SEEKABLE(pStream)), mpStream(pStream)
{
    if (!mpStream)
        throw io::NotConnectedException();
}

OutputStream::~OutputStream( void )
{
    closeOutput();
}

void SAL_CALL OutputStream::writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& rData )
    throw( io::NotConnectedException, io::BufferSizeExceededException,
           io::IOException, uno::RuntimeException)
{
    if (!mpStream)
        throw io::NotConnectedException();

    GError *pError=NULL;
    if (!g_output_stream_write_all(G_OUTPUT_STREAM(mpStream), rData.getConstArray(), rData.getLength(), NULL, NULL, &pError))
        convertToException(pError, static_cast< cppu::OWeakObject * >(this));
}

void SAL_CALL OutputStream::flush( void )
    throw( io::NotConnectedException, io::BufferSizeExceededException,
           io::IOException, uno::RuntimeException )
{
    if (!mpStream)
        throw io::NotConnectedException();

    GError *pError=NULL;
    if (!g_output_stream_flush(G_OUTPUT_STREAM(mpStream), NULL, &pError))
        convertToException(pError, static_cast< cppu::OWeakObject * >(this));
}

void SAL_CALL OutputStream::closeOutput( void )
    throw( io::NotConnectedException, io::IOException,
           uno::RuntimeException )
{
    if (mpStream)
        g_output_stream_close(G_OUTPUT_STREAM(mpStream), NULL, NULL);
}

uno::Any OutputStream::queryInterface( const uno::Type &type ) throw( uno::RuntimeException )
{
    uno::Any aRet = ::cppu::queryInterface ( type,
        static_cast< XOutputStream * >( this ) );

    return aRet.hasValue() ? aRet : Seekable::queryInterface( type );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
