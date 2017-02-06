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

Seekable::~Seekable()
{
}

void SAL_CALL Seekable::truncate()
{
    if (!mpStream)
        throw io::NotConnectedException();

    if (!g_seekable_can_truncate(mpStream))
        throw io::IOException("Truncate unsupported",
            static_cast< cppu::OWeakObject * >(this));

    GError *pError=nullptr;
    if (!g_seekable_truncate(mpStream, 0, nullptr, &pError))
        convertToIOException(pError, static_cast< cppu::OWeakObject * >(this));
}

void SAL_CALL Seekable::seek( sal_Int64 location )
{
    if (!mpStream)
        throw io::NotConnectedException();

    if (!g_seekable_can_seek(mpStream))
        throw io::IOException("Seek unsupported",
            static_cast< cppu::OWeakObject * >(this));

    GError *pError=nullptr;
    if (!g_seekable_seek(mpStream, location, G_SEEK_SET, nullptr, &pError))
        convertToIOException(pError, static_cast< cppu::OWeakObject * >(this));
}

sal_Int64 SAL_CALL Seekable::getPosition()
{
    if (!mpStream)
        throw io::NotConnectedException();

    return g_seekable_tell(mpStream);
}

sal_Int64 SAL_CALL Seekable::getLength()
{
    if (!mpStream)
        throw io::NotConnectedException();

    bool bOk = false;
    sal_uInt64 nSize = 0;

    GFileInfo* pInfo = G_IS_FILE_INPUT_STREAM(mpStream)
        ? g_file_input_stream_query_info(G_FILE_INPUT_STREAM(mpStream), G_FILE_ATTRIBUTE_STANDARD_SIZE, nullptr, nullptr)
        : g_file_output_stream_query_info(G_FILE_OUTPUT_STREAM(mpStream), G_FILE_ATTRIBUTE_STANDARD_SIZE, nullptr, nullptr);

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
        GError *pError=nullptr;
        sal_Int64 nCurr = getPosition();
        if (!g_seekable_seek(mpStream, 0, G_SEEK_END, nullptr, &pError))
            convertToIOException(pError, static_cast< cppu::OWeakObject * >(this));
        nSize = getPosition();
        seek(nCurr);
    }

    return nSize;
}

uno::Any Seekable::queryInterface( const uno::Type &type )
{
    uno::Any aRet = ::cppu::queryInterface ( type,
        static_cast< XSeekable * >( this ) );

    if (!aRet.hasValue() && g_seekable_can_truncate(mpStream))
        aRet = ::cppu::queryInterface ( type, static_cast< XTruncate * >( this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( type );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
