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
public:
    ScVbaHyperlink(
        const css::uno::Sequence< css::uno::Any >& rArgs,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext ) throw (css::lang::IllegalArgumentException);

    ScVbaHyperlink(
        const css::uno::Reference< ov::XHelperInterface >& rxAnchor,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Any& rAddress, const css::uno::Any& rSubAddress,
        const css::uno::Any& rScreenTip, const css::uno::Any& rTextToDisplay ) throw (css::uno::RuntimeException);

    virtual ~ScVbaHyperlink();

    // Attributes
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& rName ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAddress() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAddress( const ::rtl::OUString& rAddress ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSubAddress() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSubAddress( const ::rtl::OUString& rSubAddress ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getScreenTip() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setScreenTip( const ::rtl::OUString& rScreenTip ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTextToDisplay() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTextToDisplay( const ::rtl::OUString& rTextToDisplay ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getRange() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::msforms::XShape > SAL_CALL getShape() throw (css::uno::RuntimeException);

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    typedef ::std::pair< ::rtl::OUString, ::rtl::OUString > UrlComponents;

    void ensureTextField() throw (css::uno::RuntimeException);
    UrlComponents getUrlComponents() throw (css::uno::RuntimeException);
    void setUrlComponents( const UrlComponents& rUrlComp ) throw (css::uno::RuntimeException);

private:
    css::uno::Reference< css::table::XCell > mxCell;
    css::uno::Reference< css::beans::XPropertySet > mxTextField;
    ::rtl::OUString maScreenTip;
    long mnType;
};

#endif /* SC_VBA_HYPERLINK_HXX */

