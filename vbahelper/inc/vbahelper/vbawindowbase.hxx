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

#include <ooo/vba/XWindowBase.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::XWindowBase > WindowBaseImpl_BASE;

class VBAHELPER_DLLPUBLIC VbaWindowBase : public WindowBaseImpl_BASE
{
public:
    VbaWindowBase(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XModel >& xModel,
        const css::uno::Reference< css::frame::XController >& xController )
        throw (css::uno::RuntimeException);
    VbaWindowBase(
        css::uno::Sequence< css::uno::Any > const& aArgs,
        css::uno::Reference< css::uno::XComponentContext > const& xContext )
        throw (css::uno::RuntimeException);

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

protected:
    css::uno::Reference< css::frame::XController > getController() throw (css::uno::RuntimeException);
    css::uno::Reference< css::awt::XWindow > getWindow() throw (css::uno::RuntimeException);
    css::uno::Reference< css::awt::XWindow2 > getWindow2() throw (css::uno::RuntimeException);

    css::uno::Reference< css::frame::XModel > m_xModel;

private:
    void construct( const css::uno::Reference< css::frame::XController >& xController ) throw (css::uno::RuntimeException);

    css::uno::WeakReference< css::frame::XController > m_xController;
    css::uno::WeakReference< css::awt::XWindow > m_xWindow;
};

#endif //VBA_WINDOWBASE_HXX
