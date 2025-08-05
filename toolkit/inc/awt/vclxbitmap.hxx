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

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/XDisplayBitmap.hpp>
#include <com/sun/star/util/XAccounting.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <mutex>
#include <vcl/bitmap.hxx>




class VCLXBitmap final : public cppu::WeakImplHelper<
                            css::awt::XBitmap,
                            css::awt::XDisplayBitmap,
                            css::util::XAccounting>
{
    std::mutex    maMutex;
    Bitmap        maBitmap;

    std::mutex&   GetMutex() { return maMutex; }


public:
    // inline constructors
    VCLXBitmap() : maMutex(), maBitmap() {}
    VCLXBitmap(const Bitmap& rBitmap) : maMutex(), maBitmap(rBitmap) {}

    void          SetBitmap( const Bitmap& rBmp )   { maBitmap = rBmp; }
    const Bitmap& GetBitmap() const                 { return maBitmap; }

    // css::awt::XBitmap
    css::awt::Size                 SAL_CALL getSize() override;
    css::uno::Sequence< sal_Int8 > SAL_CALL getDIB() override;
    css::uno::Sequence< sal_Int8 > SAL_CALL getMaskDIB() override;

    // XAccounting
    sal_Int64 SAL_CALL estimateUsage() override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
