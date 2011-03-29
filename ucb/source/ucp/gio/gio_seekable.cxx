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

#include "gio_seekable.hxx"
#include "gio_content.hxx"

using namespace com::sun::star;

namespace gio
{

Seekable::Seekable(GSeekable *pStream) : mpStream(pStream)
{
    if (!mpStream)
        throw io::NotConnectedException();
}

Seekable::~Seekable( void )
{
}

void SAL_CALL Seekable::truncate( void )
    throw( io::IOException, uno::RuntimeException )
{
    if (!mpStream)
        throw io::NotConnectedException();

    if (!g_seekable_can_truncate(mpStream))
        throw io::IOException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Truncate unsupported")),
            static_cast< cppu::OWeakObject * >(this));

    GError *pError=NULL;
    if (!g_seekable_truncate(mpStream, 0, NULL, &pError))
        convertToException(pError, static_cast< cppu::OWeakObject * >(this));
}

void SAL_CALL Seekable::seek( sal_Int64 location )
    throw( lang::IllegalArgumentException, io::IOException, uno::RuntimeException )
{
    if (!mpStream)
        throw io::NotConnectedException();

    if (!g_seekable_can_seek(mpStream))
        throw io::IOException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Seek unsupported")),
            static_cast< cppu::OWeakObject * >(this));

    GError *pError=NULL;
    if (!g_seekable_seek(mpStream, location, G_SEEK_SET, NULL, &pError))
        convertToException(pError, static_cast< cppu::OWeakObject * >(this));
}

sal_Int64 SAL_CALL Seekable::getPosition() throw( io::IOException, uno::RuntimeException )
{
    if (!mpStream)
        throw io::NotConnectedException();

    return g_seekable_tell(mpStream);
}

sal_Int64 SAL_CALL Seekable::getLength() throw( io::IOException, uno::RuntimeException )
{
    if (!mpStream)
        throw io::NotConnectedException();

    bool bOk = false;
    sal_uInt64 nSize = 0;

    GFileInfo* pInfo = G_IS_FILE_INPUT_STREAM(mpStream)
        ? g_file_input_stream_query_info(G_FILE_INPUT_STREAM(mpStream), const_cast<char*>(G_FILE_ATTRIBUTE_STANDARD_SIZE), NULL, NULL)
        : g_file_output_stream_query_info(G_FILE_OUTPUT_STREAM(mpStream), const_cast<char*>(G_FILE_ATTRIBUTE_STANDARD_SIZE), NULL, NULL);

    if (pInfo)
    {
        if (g_file_info_has_attribute(pInfo, G_FILE_ATTRIBUTE_STANDARD_SIZE))
        {
            nSize = g_file_info_get_size(pInfo);
            bOk = true;
        }
        g_object_unref(pInfo);
    }

    if (!bOk)
    {
        GError *pError=NULL;
        sal_Int64 nCurr = getPosition();
        if (!g_seekable_seek(mpStream, 0, G_SEEK_END, NULL, &pError))
            convertToException(pError, static_cast< cppu::OWeakObject * >(this));
        nSize = getPosition();
        seek(nCurr);
        bOk = true;
    }

    if (!bOk)
        throw io::IOException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Getting size unsupported")),
            static_cast< cppu::OWeakObject * >(this));

    return nSize;
}

uno::Any Seekable::queryInterface( const uno::Type &type ) throw( uno::RuntimeException )
{
    uno::Any aRet = ::cppu::queryInterface ( type,
        static_cast< XSeekable * >( this ) );

    if (!aRet.hasValue() && g_seekable_can_truncate(mpStream))
        aRet = ::cppu::queryInterface ( type, static_cast< XTruncate * >( this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( type );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
