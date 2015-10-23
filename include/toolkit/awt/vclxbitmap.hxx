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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXBITMAP_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXBITMAP_HXX

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/XDisplayBitmap.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <vcl/bitmapex.hxx>



//  class VCLXBitmap


class VCLXBitmap :  public css::awt::XBitmap,
                    public css::awt::XDisplayBitmap,
                    public css::lang::XTypeProvider,
                    public css::lang::XUnoTunnel,
                    public ::cppu::OWeakObject
{
private:
    ::osl::Mutex    maMutex;
    BitmapEx        maBitmap;

protected:
    ::osl::Mutex&   GetMutex() { return maMutex; }


public:
    void            SetBitmap( const BitmapEx& rBmp )   { maBitmap = rBmp; }
    const BitmapEx& GetBitmap() const                   { return maBitmap; }


    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XUnoTunnel
    static const css::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXBitmap*                                          GetImplementation( const css::uno::Reference< css::uno::XInterface >& rxIFace );
    sal_Int64                                                   SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XBitmap
    css::awt::Size                 SAL_CALL getSize() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 > SAL_CALL getDIB() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 > SAL_CALL getMaskDIB() throw(css::uno::RuntimeException, std::exception) override;
};




#endif // INCLUDED_TOOLKIT_AWT_VCLXBITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
