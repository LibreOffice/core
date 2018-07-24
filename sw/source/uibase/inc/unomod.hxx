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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UNOMOD_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UNOMOD_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/view/XPrintSettingsSupplier.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/ChainablePropertySet.hxx>
#include "usrpref.hxx"

class SwView;
class SwViewOption;
class SwPrintData;
class SwDoc;

css::uno::Reference< css::uno::XInterface >  SAL_CALL SwXModule_CreateInstance(const css::uno::Reference< css::lang::XMultiServiceFactory > & );

class SwXModule : public cppu::WeakImplHelper
<
    css::view::XViewSettingsSupplier,
    css::view::XPrintSettingsSupplier,
    css::lang::XServiceInfo
>
{

    css::uno::Reference< css::beans::XPropertySet >     mxViewSettings;
    css::uno::Reference< css::beans::XPropertySet >     mxPrintSettings;

protected:
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
        throw() override;
public:
    SwXPrintSettings( SwXPrintSettingsType eType, SwDoc * pDoc = nullptr );

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class SwXViewSettings final : public comphelper::ChainablePropertySet
{
    friend class SwXDocumentSettings;

    SwView*                     pView;
    std::unique_ptr<SwViewOption> mpViewOption;
    const SwViewOption*         mpConstViewOption;
    bool                    bObjectValid:1, mbApplyZoom;

    FieldUnit   eHRulerUnit;
    bool    mbApplyHRulerMetric;
    FieldUnit   eVRulerUnit;
    bool    mbApplyVRulerMetric;

    virtual void _preSetValues () override;
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const css::uno::Any &rValue ) override;
    virtual void _postSetValues() override;

    virtual void _preGetValues () override;
    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, css::uno::Any & rValue ) override;
    virtual void _postGetValues () override;

    virtual ~SwXViewSettings()
        throw() override;
public:
    SwXViewSettings(SwView*  pView);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    bool    IsValid() const {return bObjectValid;}
    void    Invalidate() {bObjectValid = false;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
