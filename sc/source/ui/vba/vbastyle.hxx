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

#include <ooo/vba/excel/XStyle.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include "vbaformat.hxx"

typedef ScVbaFormat< ov::excel::XStyle > ScVbaStyle_BASE;

class ScVbaStyle final : public ScVbaStyle_BASE
{
    css::uno::Reference< css::style::XStyle > mxStyle;
    css::uno::Reference< css::container::XNameContainer > mxStyleFamilyNameContainer;
    /// @throws css::uno::RuntimeException
    /// @throws css::script::BasicErrorException
    void initialise();
public:
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    ScVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const OUString& sStyleName, const css::uno::Reference< css::frame::XModel >& _xModel );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    ScVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, const css::uno::Reference< css::frame::XModel >& _xModel );
    /// @throws css::uno::RuntimeException
    static css::uno::Reference< css::container::XNameAccess > getStylesNameContainer( const css::uno::Reference< css::frame::XModel >& xModel );
    virtual css::uno::Reference< ov::XHelperInterface > thisHelperIface() override { return this; };
    // XStyle Methods
    virtual sal_Bool SAL_CALL BuiltIn() override;
    virtual void SAL_CALL setName( const OUString& Name ) override;
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setNameLocal( const OUString& NameLocal ) override;
    virtual OUString SAL_CALL getNameLocal() override;
    virtual void SAL_CALL Delete() override;
    // XFormat
    virtual void SAL_CALL setMergeCells( const css::uno::Any& MergeCells ) override;
    virtual css::uno::Any SAL_CALL getMergeCells(  ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
