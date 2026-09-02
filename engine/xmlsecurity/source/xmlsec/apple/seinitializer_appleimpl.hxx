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

#include <nss/seinitializer_nssimpl.hxx>

/** SEInitializer whose security contexts use the Keychain-aware security environment.

    NSS is still initialized (it remains the XML-DSig engine and the fallback certificate
    store), but the environment on top of it also exposes Keychain identities.
*/
class SEInitializer_AppleImpl : public SEInitializer_NssImpl
{
public:
    explicit SEInitializer_AppleImpl(
        const css::uno::Reference<cpo::uno::XComponentContext>& rxContext);
    virtual ~SEInitializer_AppleImpl() override;

    virtual css::uno::Reference<css::xml::crypto::XXMLSecurityContext>
        SAL_CALL createSecurityContext(const OUString&) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
