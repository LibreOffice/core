/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>

class XMLSignature_NssImpl
    : public ::cppu::WeakImplHelper<css::xml::crypto::XXMLSignature, css::lang::XServiceInfo>
{
public:
    explicit XMLSignature_NssImpl();

    //Methods from XXMLSignature
    virtual css::uno::Reference<css::xml::crypto::XXMLSignatureTemplate> SAL_CALL generate(
        const css::uno::Reference<css::xml::crypto::XXMLSignatureTemplate>& aTemplate,
        const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& aEnvironment) override;

    virtual css::uno::Reference<css::xml::crypto::XXMLSignatureTemplate> SAL_CALL
    validate(const css::uno::Reference<css::xml::crypto::XXMLSignatureTemplate>& aTemplate,
             const css::uno::Reference<css::xml::crypto::XXMLSecurityContext>& aContext) override;

    //Methods from XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual bool SAL_CALL supportsService(const OUString& ServiceName) override;

    virtual cpo::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
