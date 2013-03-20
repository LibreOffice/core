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

#pragma once
#if 1

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
