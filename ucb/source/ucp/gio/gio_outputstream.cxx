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

#include <com/sun/star/io/NotConnectedException.hpp>
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

OutputStream::~OutputStream()
{
    closeOutput();
}

void SAL_CALL OutputStream::writeBytes( const css::uno::Sequence< sal_Int8 >& rData )
{
    if (!mpStream)
        throw io::NotConnectedException();

    GError *pError=nullptr;
    if (!g_output_stream_write_all(G_OUTPUT_STREAM(mpStream), rData.getConstArray(), rData.getLength(), nullptr, nullptr, &pError))
        convertToIOException(pError, static_cast< cppu::OWeakObject * >(this));
}

void SAL_CALL OutputStream::flush()
{
    if (!mpStream)
        throw io::NotConnectedException();

    GError *pError=nullptr;
    if (!g_output_stream_flush(G_OUTPUT_STREAM(mpStream), nullptr, &pError))
        convertToIOException(pError, static_cast< cppu::OWeakObject * >(this));
}

void SAL_CALL OutputStream::closeOutput()
{
    if (mpStream)
        g_output_stream_close(G_OUTPUT_STREAM(mpStream), nullptr, nullptr);
}

uno::Any OutputStream::queryInterface( const uno::Type &type )
{
    uno::Any aRet = ::cppu::queryInterface ( type,
        static_cast< XOutputStream * >( this ) );

    return aRet.hasValue() ? aRet : Seekable::queryInterface( type );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
