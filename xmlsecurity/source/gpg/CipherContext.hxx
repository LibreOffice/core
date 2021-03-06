/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/xml/crypto/XCipherContext.hpp>

#include <cppuhelper/implbase.hxx>

class CipherContext : public cppu::WeakImplHelper<css::xml::crypto::XCipherContext>
{
private:
public:
    // XCipherContext
    virtual css::uno::Sequence<::sal_Int8>
        SAL_CALL convertWithCipherContext(const css::uno::Sequence<::sal_Int8>& aData) override;
    virtual css::uno::Sequence<::sal_Int8> SAL_CALL finalizeCipherContextAndDispose() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
