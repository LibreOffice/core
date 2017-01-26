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

#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAHYPERLINK_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAHYPERLINK_HXX

#include <ooo/vba/excel/XHyperlink.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XRange.hpp>

#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XHyperlink > HyperlinkImpl_BASE;

class ScVbaHyperlink : public HyperlinkImpl_BASE
{
public:
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    ScVbaHyperlink(
        const css::uno::Sequence< css::uno::Any >& rArgs,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    /// @throws css::uno::RuntimeException
    ScVbaHyperlink(
        const css::uno::Reference< ov::XHelperInterface >& rxAnchor,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Any& rAddress, const css::uno::Any& rSubAddress,
        const css::uno::Any& rScreenTip, const css::uno::Any& rTextToDisplay );

    virtual ~ScVbaHyperlink() override;

    // Attributes
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName( const OUString& rName ) override;
    virtual OUString SAL_CALL getAddress() override;
    virtual void SAL_CALL setAddress( const OUString& rAddress ) override;
    virtual OUString SAL_CALL getSubAddress() override;
    virtual void SAL_CALL setSubAddress( const OUString& rSubAddress ) override;
    virtual OUString SAL_CALL getScreenTip() override;
    virtual void SAL_CALL setScreenTip( const OUString& rScreenTip ) override;
    virtual OUString SAL_CALL getTextToDisplay() override;
    virtual void SAL_CALL setTextToDisplay( const OUString& rTextToDisplay ) override;
    virtual sal_Int32 SAL_CALL getType() override;
    virtual css::uno::Reference< ov::excel::XRange > SAL_CALL getRange() override;
    virtual css::uno::Reference< ov::msforms::XShape > SAL_CALL getShape() override;

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    typedef ::std::pair< OUString, OUString > UrlComponents;

    /// @throws css::uno::RuntimeException
    void ensureTextField();
    /// @throws css::uno::RuntimeException
    UrlComponents getUrlComponents();
    /// @throws css::uno::RuntimeException
    void setUrlComponents( const UrlComponents& rUrlComp );

private:
    css::uno::Reference< css::table::XCell > mxCell;
    css::uno::Reference< css::beans::XPropertySet > mxTextField;
    OUString maScreenTip;
    long mnType;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAHYPERLINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
