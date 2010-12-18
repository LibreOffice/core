/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef VBA_WINDOWBASE_HXX
#define VBA_WINDOWBASE_HXX
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/XWindowBase.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/awt/XDevice.hpp>

#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1<ov::XWindowBase > WindowBaseImpl_BASE;

class VBAHELPER_DLLPUBLIC VbaWindowBase : public WindowBaseImpl_BASE
{
protected:
    css::uno::Reference< css::frame::XModel > m_xModel;
public:
    VbaWindowBase( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::frame::XModel >& xModel );
    VbaWindowBase( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext > const& xContext );

    // XWindowBase
    virtual sal_Int32 SAL_CALL getHeight() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setHeight( sal_Int32 _height ) throw (css::uno::RuntimeException) ;
    virtual sal_Int32 SAL_CALL getLeft() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setLeft( sal_Int32 _left ) throw (css::uno::RuntimeException) ;
    virtual sal_Int32 SAL_CALL getTop() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setTop( sal_Int32 _top ) throw (css::uno::RuntimeException) ;
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getWidth() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setWidth( sal_Int32 _width ) throw (css::uno::RuntimeException) ;

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //VBA_WINDOWBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
