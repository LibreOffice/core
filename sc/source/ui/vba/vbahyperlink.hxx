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
#ifndef SC_VBA_HYPERLINK_HXX
#define SC_VBA_HYPERLINK_HXX

#include <ooo/vba/excel/XHyperlink.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XRange.hpp>

#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::excel::XHyperlink > HyperlinkImpl_BASE;

class ScVbaHyperlink : public HyperlinkImpl_BASE
{
    css::uno::Reference< css::table::XCell > mxCell;
    css::uno::Reference< css::beans::XPropertySet > mxTextField;

public:
    ScVbaHyperlink( css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext > const& xContext ) throw ( css::lang::IllegalArgumentException );
    virtual ~ScVbaHyperlink();

    // Attributes
    virtual ::rtl::OUString SAL_CALL getAddress() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAddress( const ::rtl::OUString &rAddress ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString &rName ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTextToDisplay() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTextToDisplay( const ::rtl::OUString &rTextToDisplay ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL Range() throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SC_VBA_HYPERLINK_HXX */

