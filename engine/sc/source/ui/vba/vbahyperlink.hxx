/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <ooo/vba/excel/XHyperlink.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include <tools/long.hxx>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::table { class XCell; }

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XHyperlink > HyperlinkImpl_BASE;

class ScVbaHyperlink : public HyperlinkImpl_BASE
{
public:
    /// @throws css::lang::IllegalArgumentException
    /// @throws cpo::uno::RuntimeException
    ScVbaHyperlink(
        const cpo::uno::Sequence< cpo::uno::Any >& rArgs,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext );

    /// @throws cpo::uno::RuntimeException
    ScVbaHyperlink(
        const cpo::uno::Reference< ov::XHelperInterface >& rxAnchor,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext,
        const cpo::uno::Any& rAddress, const cpo::uno::Any& rSubAddress,
        const cpo::uno::Any& rScreenTip, const cpo::uno::Any& rTextToDisplay );

    virtual ~ScVbaHyperlink() override;

    // Attributes
    virtual OUString getName() override;
    virtual void setName( const OUString& rName ) override;
    virtual OUString getAddress() override;
    virtual void setAddress( const OUString& rAddress ) override;
    virtual OUString getSubAddress() override;
    virtual void setSubAddress( const OUString& rSubAddress ) override;
    virtual OUString getScreenTip() override;
    virtual void setScreenTip( const OUString& rScreenTip ) override;
    virtual OUString getTextToDisplay() override;
    virtual void setTextToDisplay( const OUString& rTextToDisplay ) override;
    virtual sal_Int32 getType() override;
    virtual cpo::uno::Reference< ov::excel::XRange > getRange() override;
    virtual cpo::uno::Reference< ov::msforms::XShape > getShape() override;

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    typedef ::std::pair< OUString, OUString > UrlComponents;

    /// @throws cpo::uno::RuntimeException
    void ensureTextField();
    /// @throws cpo::uno::RuntimeException
    UrlComponents getUrlComponents();
    /// @throws cpo::uno::RuntimeException
    void setUrlComponents( const UrlComponents& rUrlComp );

private:
    cpo::uno::Reference< css::table::XCell > mxCell;
    cpo::uno::Reference< css::beans::XPropertySet > mxTextField;
    OUString maScreenTip;
    tools::Long mnType;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
