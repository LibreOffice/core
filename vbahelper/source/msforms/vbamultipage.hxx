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
#ifndef SC_VBA_MULTIPAGE_HXX
#define SC_VBA_MULTIPAGE_HXX
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/msforms/XMultiPage.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>
//#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

typedef cppu::ImplInheritanceHelper1< ScVbaControl, ov::msforms::XMultiPage > MultiPageImpl_BASE;

class ScVbaMultiPage : public MultiPageImpl_BASE
{
    css::uno::Reference< css::container::XIndexAccess > getPages( sal_Int32 nPages );
    css::uno::Reference< css::beans::XPropertySet > mxDialogProps;
public:
    ScVbaMultiPage(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::uno::XInterface >& xControl,
        const css::uno::Reference< css::frame::XModel >& xModel,
        ov::AbstractGeometryAttributes* pGeomHelper,
        const css::uno::Reference< css::awt::XControl >& xDialog );

   // Attributes
    virtual sal_Int32 SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( sal_Int32 _value ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Pages( const css::uno::Any& index ) throw (css::uno::RuntimeException);

    //XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    // XDefaultProperty
    rtl::OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException) { return ::rtl::OUString::createFromAscii("Value"); }
};
#endif //SC_VBA_LABEL_HXX
