/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/sheet/XCompatibilityNames.hpp>
#include <com/sun/star/sheet/XVolatileResult.hpp>
#include <org/collaboraoffice/sheet/addin/XPythonComputeFunctions.hpp>
#include <cppuhelper/implbase.hxx>

class ScaPythonComputeAddIn final
    : public cppu::WeakImplHelper<css::sheet::XAddIn, css::sheet::XCompatibilityNames,
                                  org::collaboraoffice::sheet::addin::XPythonComputeFunctions,
                                  css::lang::XServiceName, css::lang::XServiceInfo>
{
    css::lang::Locale m_aLocale;

public:
    ScaPythonComputeAddIn();
    virtual ~ScaPythonComputeAddIn() override;

    // XPythonComputeFunctions
    virtual cpo::uno::Any getPy(const OUString& aCode,
                                const cpo::uno::Sequence<cpo::uno::Any>& aData) override;
    virtual cpo::uno::Any getPython(const OUString& aCode,
                                    const cpo::uno::Sequence<cpo::uno::Any>& aData) override;

    // XServiceName / XServiceInfo
    virtual OUString getServiceName() override;
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    // XLocalizable (via XAddIn)
    virtual void setLocale(const css::lang::Locale& eLocale) override;
    virtual css::lang::Locale getLocale() override;

    // XAddIn
    virtual OUString getProgrammaticFuntionName(const OUString& aDisplayName) override;
    virtual OUString getDisplayFunctionName(const OUString& aProgrammaticName) override;
    virtual OUString getFunctionDescription(const OUString& aProgrammaticName) override;
    virtual OUString getDisplayArgumentName(const OUString& aProgrammaticName,
                                            sal_Int32 nArgument) override;
    virtual OUString getArgumentDescription(const OUString& aProgrammaticName,
                                            sal_Int32 nArgument) override;
    virtual OUString getProgrammaticCategoryName(const OUString& aProgrammaticName) override;
    virtual OUString getDisplayCategoryName(const OUString& aProgrammaticName) override;

    // XCompatibilityNames
    virtual cpo::uno::Sequence<css::sheet::LocalizedName>
    getCompatibilityNames(const OUString& aProgrammaticName) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
