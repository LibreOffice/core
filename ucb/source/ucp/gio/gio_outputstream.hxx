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

#pragma once

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include "gio_seekable.hxx"

namespace gio
{

class OutputStream final :
    public css::io::XOutputStream,
    public Seekable
{
private:
    GFileOutputStream *mpStream;

public:
    explicit OutputStream ( GFileOutputStream *pStream );
    virtual ~OutputStream() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type & type ) override;
    virtual void SAL_CALL acquire() noexcept override { OWeakObject::acquire(); }
    virtual void SAL_CALL release() noexcept override { OWeakObject::release(); }

    // XOutputStream
   virtual void SAL_CALL writeBytes( const css::uno::Sequence< sal_Int8 >& aData ) override;

   virtual void SAL_CALL flush() override;


    virtual void SAL_CALL closeOutput() override;
};

} // namespace gio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
