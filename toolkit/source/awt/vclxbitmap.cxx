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

#include <toolkit/awt/vclxbitmap.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <tools/stream.hxx>
#include <rtl/uuid.h>
#include <vcl/dibtools.hxx>
#include <vcl/BitmapTools.hxx>


//  class VCLXBitmap


// css::uno::XInterface
css::uno::Any VCLXBitmap::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XBitmap* >(this),
                                        static_cast< css::awt::XDisplayBitmap* >(this),
                                        static_cast< css::lang::XUnoTunnel* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// css::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXBitmap )

IMPL_IMPLEMENTATION_ID( VCLXBitmap )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXBitmap::getTypes()
{
    static const css::uno::Sequence< css::uno::Type > aTypeList {
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XBitmap>::get(),
        cppu::UnoType<css::awt::XDisplayBitmap>::get()
    };
    return aTypeList;
}


// css::awt::XBitmap
css::awt::Size VCLXBitmap::getSize()
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::awt::Size aSize( maBitmap.GetSizePixel().Width(), maBitmap.GetSizePixel().Height() );
    return aSize;
}

css::uno::Sequence< sal_Int8 > VCLXBitmap::getDIB()
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    SvMemoryStream aMem;
    WriteDIB(maBitmap.GetBitmap(), aMem, false, true);
    return css::uno::Sequence<sal_Int8>( static_cast<sal_Int8 const *>(aMem.GetData()), aMem.Tell() );
}

css::uno::Sequence< sal_Int8 > VCLXBitmap::getMaskDIB()
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return vcl::bitmap::GetMaskDIB(maBitmap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
