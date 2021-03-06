/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <cppuhelper/implbase.hxx>

class DigestContext : public cppu::WeakImplHelper<css::xml::crypto::XDigestContext>
{
public:
    // XDigestContext
    virtual void SAL_CALL updateDigest(const css::uno::Sequence<::sal_Int8>& aData) override;
    virtual css::uno::Sequence<::sal_Int8> SAL_CALL finalizeDigestAndDispose() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
