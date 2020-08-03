/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/crypto/XXMLEncryption.hpp>

namespace com::sun::star::xml::crypto { class XXMLEncryptionTemplate; }
namespace com::sun::star::xml::crypto { class XXMLSecurityContext; }

class XMLEncryptionGpg : public cppu::WeakImplHelper< css::xml::crypto::XXMLEncryption >
{
public:
    explicit XMLEncryptionGpg();
    virtual ~XMLEncryptionGpg() override;

    // XXMLEncryption
    virtual css::uno::Reference< css::xml::crypto::XXMLEncryptionTemplate > SAL_CALL encrypt(
        const css::uno::Reference< css::xml::crypto::XXMLEncryptionTemplate >& aTemplate,
        const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& aEnvironment) override;

    virtual css::uno::Reference< css::xml::crypto::XXMLEncryptionTemplate > SAL_CALL decrypt(
        const css::uno::Reference< css::xml::crypto::XXMLEncryptionTemplate >& aTemplate,
        const css::uno::Reference< css::xml::crypto::XXMLSecurityContext >& aContext) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
