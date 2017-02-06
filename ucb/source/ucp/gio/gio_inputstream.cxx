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

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <string.h>

#include "gio_inputstream.hxx"
#include "gio_content.hxx"

using namespace com::sun::star;

namespace gio
{

InputStream::InputStream(GFileInputStream *pStream): mpStream(pStream)
{
    if (!mpStream)
        throw io::NotConnectedException();
}

InputStream::~InputStream()
{
    closeInput();
}

sal_Int32 SAL_CALL InputStream::available()
{
    return 0;
}

void SAL_CALL InputStream::closeInput()
{
    if (mpStream)
        g_input_stream_close(G_INPUT_STREAM(mpStream), nullptr, nullptr);
}

void SAL_CALL InputStream::skipBytes( sal_Int32 nBytesToSkip )
{
    // Conservatively call readBytes and discard the read data, but given this
    // InputStream will always be wrapped in comphelper::OSeekableInputWrapper,
    // this function will never be called anyway:
    css::uno::Sequence<sal_Int8> data;
    readBytes(data, nBytesToSkip);
}

sal_Int32 SAL_CALL InputStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    if (!mpStream)
        throw io::NotConnectedException();

    try
    {
        aData.realloc( nBytesToRead );
    }
    catch ( const uno::Exception & )
    {
        throw io::BufferSizeExceededException();
    }

    gsize nBytesRead = 0;
    GError *pError=nullptr;
    if (!g_input_stream_read_all(G_INPUT_STREAM(mpStream), aData.getArray(), nBytesToRead, &nBytesRead, nullptr, &pError))
        convertToIOException(pError, static_cast< cppu::OWeakObject * >(this));
    aData.realloc(nBytesRead);
    return nBytesRead;
}

sal_Int32 SAL_CALL InputStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    return readBytes(aData, nMaxBytesToRead);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
