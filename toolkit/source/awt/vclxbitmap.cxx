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
#include <tools/stream.hxx>
#include <rtl/uuid.h>

//  ----------------------------------------------------
//  class VCLXBitmap
//  ----------------------------------------------------

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXBitmap::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XBitmap* >(this)),
                                        (static_cast< ::com::sun::star::awt::XDisplayBitmap* >(this)),
                                        (static_cast< ::com::sun::star::lang::XUnoTunnel* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXBitmap )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXBitmap )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap>* ) NULL )
IMPL_XTYPEPROVIDER_END


// ::com::sun::star::awt::XBitmap
::com::sun::star::awt::Size VCLXBitmap::getSize() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSize( maBitmap.GetSizePixel().Width(), maBitmap.GetSizePixel().Height() );
    return aSize;
}

::com::sun::star::uno::Sequence< sal_Int8 > VCLXBitmap::getDIB() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    SvMemoryStream aMem;
    aMem << maBitmap.GetBitmap();
    return ::com::sun::star::uno::Sequence<sal_Int8>( (sal_Int8*) aMem.GetData(), aMem.Tell() );
}

::com::sun::star::uno::Sequence< sal_Int8 > VCLXBitmap::getMaskDIB() throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    SvMemoryStream aMem;
    aMem << maBitmap.GetMask();
    return ::com::sun::star::uno::Sequence<sal_Int8>( (sal_Int8*) aMem.GetData(), aMem.Tell() );
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
