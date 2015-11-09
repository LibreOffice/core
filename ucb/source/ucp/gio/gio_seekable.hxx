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

#ifndef INCLUDED_UCB_SOURCE_UCP_GIO_GIO_SEEKABLE_HXX
#define INCLUDED_UCB_SOURCE_UCP_GIO_GIO_SEEKABLE_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>

#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <gio/gio.h>

namespace gio
{

class Seekable : public css::io::XTruncate,
    public css::io::XSeekable,
    public ::cppu::OWeakObject
{
private:
    GSeekable *mpStream;
public:
    explicit Seekable( GSeekable *pStream );
    virtual ~Seekable();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type & type )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw () override { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw() override { OWeakObject::release(); }

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location )
            throw( css::lang::IllegalArgumentException,
                css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int64 SAL_CALL getPosition()
            throw( css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int64 SAL_CALL getLength()
            throw( css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;

    // XTruncate
    virtual void SAL_CALL truncate()
            throw( css::io::IOException,
                css::uno::RuntimeException, std::exception ) override;
};

} // namespace gio
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
