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

#include <ooo/vba/excel/XStyle.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include "vbaformat.hxx"

class ScModelObj;

typedef ScVbaFormat< ov::excel::XStyle > ScVbaStyle_BASE;

class ScVbaStyle final : public ScVbaStyle_BASE
{
    cpo::uno::Reference< css::style::XStyle > mxStyle;
    cpo::uno::Reference< css::container::XNameContainer > mxStyleFamilyNameContainer;
    /// @throws cpo::uno::RuntimeException
    /// @throws css::script::BasicErrorException
    void initialise();
public:
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    ScVbaStyle( const cpo::uno::Reference< ov::XHelperInterface >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext > & xContext, const OUString& sStyleName, const rtl::Reference<ScModelObj>& _xModel );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    ScVbaStyle( const cpo::uno::Reference< ov::XHelperInterface >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext > & xContext, const cpo::uno::Reference< css::beans::XPropertySet >& _xPropertySet, const rtl::Reference<ScModelObj>& _cModel );
    /// @throws cpo::uno::RuntimeException
    static cpo::uno::Reference< css::container::XNameAccess > getStylesNameContainer( const rtl::Reference<ScModelObj>& xModel );
    virtual cpo::uno::Reference< ov::XHelperInterface > thisHelperIface() override { return this; };
    // XStyle Methods
    virtual bool BuiltIn() override;
    virtual void setName( const OUString& Name ) override;
    virtual OUString getName() override;
    virtual void setNameLocal( const OUString& NameLocal ) override;
    virtual OUString getNameLocal() override;
    virtual void Delete() override;
    // XFormat
    virtual void setMergeCells( const cpo::uno::Any& MergeCells ) override;
    virtual cpo::uno::Any getMergeCells(  ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
