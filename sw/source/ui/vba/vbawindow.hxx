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
#ifndef SW_VBA_WINDOW_HXX
#define SW_VBA_WINDOW_HXX
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/word/XWindow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbawindowbase.hxx>

typedef cppu::ImplInheritanceHelper1< VbaWindowBase, ov::word::XWindow > WindowImpl_BASE;

class SwVbaWindow : public WindowImpl_BASE
{
public:
    SwVbaWindow(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XModel >& xModel,
        const css::uno::Reference< css::frame::XController >& xController )
        throw (css::uno::RuntimeException);

    // Attributes
    virtual css::uno::Any SAL_CALL getView() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setView( const css::uno::Any& _view ) throw (css::uno::RuntimeException);
    // Methods
    virtual void SAL_CALL Activate(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Close( const css::uno::Any& SaveChanges, const css::uno::Any& RouteDocument ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Panes( const css::uno::Any& aIndex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL ActivePane() throw (css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SW_VBA_WINDOW_HXX
