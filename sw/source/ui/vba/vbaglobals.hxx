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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAGLOBALS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAGLOBALS_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <ooo/vba/word/XGlobals.hpp>
#include <ooo/vba/word/XApplication.hpp>
#include <ooo/vba/word/XSystem.hpp>
#include <ooo/vba/word/XOptions.hpp>
#include <ooo/vba/word/XSelection.hpp>
#include <cppuhelper/implbase.hxx>
#include <vbahelper/vbaglobalbase.hxx>

typedef ::cppu::ImplInheritanceHelper<VbaGlobalsBase, ov::word::XGlobals> SwVbaGlobals_BASE;

class SwVbaGlobals : public SwVbaGlobals_BASE
{
private:
    css::uno::Reference<ooo::vba::word::XApplication> mxApplication;

    /// @throws css::uno::RuntimeException
    css::uno::Reference<ooo::vba::word::XApplication> const& getApplication();

public:
    SwVbaGlobals(css::uno::Sequence<css::uno::Any> const& aArgs,
                 css::uno::Reference<css::uno::XComponentContext> const& rxContext);
    virtual ~SwVbaGlobals() override;

    // XGlobals
    virtual OUString SAL_CALL getName() override;
    virtual css::uno::Reference<ooo::vba::word::XSystem> SAL_CALL getSystem() override;
    virtual css::uno::Reference<ov::word::XDocument> SAL_CALL getActiveDocument() override;
    virtual css::uno::Reference<ov::word::XWindow> SAL_CALL getActiveWindow() override;
    virtual css::uno::Reference<ooo::vba::word::XOptions> SAL_CALL getOptions() override;
    virtual css::uno::Reference<ooo::vba::word::XSelection> SAL_CALL getSelection() override;
    virtual css::uno::Any SAL_CALL CommandBars(const css::uno::Any& aIndex) override;
    virtual css::uno::Any SAL_CALL Documents(const css::uno::Any& aIndex) override;
    virtual css::uno::Any SAL_CALL Addins(const css::uno::Any& aIndex) override;
    virtual css::uno::Any SAL_CALL Dialogs(const css::uno::Any& aIndex) override;
    virtual css::uno::Any SAL_CALL ListGalleries(const css::uno::Any& aIndex) override;
    virtual float SAL_CALL CentimetersToPoints(float Centimeters) override;
    // XMultiServiceFactory
    virtual css::uno::Sequence<OUString> SAL_CALL getAvailableServiceNames() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAGLOBALS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
