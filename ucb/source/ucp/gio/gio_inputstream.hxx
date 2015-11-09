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

#ifndef INCLUDED_UCB_SOURCE_UCP_GIO_GIO_INPUTSTREAM_HXX
#define INCLUDED_UCB_SOURCE_UCP_GIO_GIO_INPUTSTREAM_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/io/XInputStream.hpp>

#include <gio/gio.h>

namespace gio
{

class InputStream: public cppu::WeakImplHelper<css::io::XInputStream>
{
private:
    GFileInputStream *mpStream;

public:
    explicit InputStream ( GFileInputStream *pStream );
    virtual ~InputStream();

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 > & aData,
        sal_Int32 nBytesToRead )
            throw( css::io::NotConnectedException,
                css::io::BufferSizeExceededException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 > & aData,
        sal_Int32 nMaxBytesToRead )
            throw( css::io::NotConnectedException,
                css::io::BufferSizeExceededException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
            throw( css::io::NotConnectedException,
                css::io::BufferSizeExceededException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL available()
            throw( css::io::NotConnectedException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL closeInput()
            throw( css::io::NotConnectedException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;
};

} // namespace gio
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
