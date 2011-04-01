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
#ifndef SC_VBA_BUTTON_HXX
#define SC_VBA_BUTTON_HXX
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/msforms/XCommandButton.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper1< ScVbaControl, ov::msforms::XCommandButton > ButtonImpl_BASE;

class ScVbaButton : public ButtonImpl_BASE
{
public:
    ScVbaButton( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper  );
   // Attributes
    virtual rtl::OUString SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const rtl::OUString& _caption ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getAutoSize() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoSize( sal_Bool bAutoSize ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getCancel() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCancel( sal_Bool bCancel ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getDefault() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDefault( sal_Bool bDefault ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackColor() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBackColor( sal_Int32 nBackColor ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeColor() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setForeColor( sal_Int32 nForeColor ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XNewFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    //XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif //SC_VBA_BUTTON_HXX
