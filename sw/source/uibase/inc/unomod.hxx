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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/view/XPrintSettingsSupplier.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/ChainablePropertySet.hxx>
#include <tools/fldunit.hxx>

class SwView;
class SwViewOption;
class SwPrintData;
class SwDoc;

css::uno::Reference< css::uno::XInterface >  SAL_CALL SwXModule_CreateInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & );

class SwXModule final : public cppu::WeakImplHelper
<
    css::view::XViewSettingsSupplier,
    css::view::XPrintSettingsSupplier,
    css::lang::XServiceInfo
>
{

    css::uno::Reference< css::beans::XPropertySet >     mxViewSettings;
    css::uno::Reference< css::beans::XPropertySet >     mxPrintSettings;

    virtual ~SwXModule() override;
public:
    SwXModule();

    //XViewSettings
    virtual css::uno::Reference< css::beans::XPropertySet >  SAL_CALL getViewSettings() override;

    //XPrintSettings
    virtual css::uno::Reference< css::beans::XPropertySet >  SAL_CALL getPrintSettings() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

enum class SwXPrintSettingsType
{
    Module,
    Document
};

class SwXPrintSettings final : public comphelper::ChainablePropertySet
{
    friend class SwXDocumentSettings;

    SwXPrintSettingsType meType;
    SwPrintData * mpPrtOpt;
    SwDoc *mpDoc;

    virtual void _preSetValues () override;
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const css::uno::Any &rValue ) override;
    virtual void _postSetValues () override;

    virtual void _preGetValues () override;

    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, css::uno::Any & rValue ) override;
    virtual void _postGetValues () override;

    virtual ~SwXPrintSettings()
        noexcept override;
public:
    SwXPrintSettings( SwXPrintSettingsType eType, SwDoc * pDoc = nullptr );

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

/**
 * Implements com.sun.star.text.ViewSettings: UNO access to per-view settings (e.g. if formatting
 * marks are visible or not.)
 */
class SwXViewSettings final : public comphelper::ChainablePropertySet
{
    friend class SwXDocumentSettings;

    SwView*                     m_pView;
    std::unique_ptr<SwViewOption> mpViewOption;
    const SwViewOption*         mpConstViewOption;
    bool                    m_bObjectValid:1, mbApplyZoom;

    FieldUnit   m_eHRulerUnit;
    bool    mbApplyHRulerMetric;
    FieldUnit   m_eVRulerUnit;
    bool    mbApplyVRulerMetric;

    virtual void _preSetValues () override;
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const css::uno::Any &rValue ) override;
    virtual void _postSetValues() override;

    virtual void _preGetValues () override;
    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, css::uno::Any & rValue ) override;
    virtual void _postGetValues () override;

    virtual ~SwXViewSettings()
        noexcept override;
public:
    SwXViewSettings(SwView*  pView);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    bool    IsValid() const {return m_bObjectValid;}
    void    Invalidate() {m_bObjectValid = false;}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
