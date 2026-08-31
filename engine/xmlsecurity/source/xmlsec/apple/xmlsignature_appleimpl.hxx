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

#include <nss/xmlsignature_nssimpl.hxx>

/** XML-DSig implementation that can sign with Keychain-backed private keys.

    When the signing certificate selected on the security environment is a Keychain identity,
    the references and the canonicalization of SignedInfo are still processed with xmlsec-nss,
    but the signature value itself is produced with the Security framework. Everything else
    (NSS-database keys, verification) is delegated to the NSS implementation.
*/
class XMLSignature_AppleImpl : public XMLSignature_NssImpl
{
public:
    XMLSignature_AppleImpl();
    virtual ~XMLSignature_AppleImpl() override;

    virtual css::uno::Reference<css::xml::crypto::XXMLSignatureTemplate> SAL_CALL generate(
        const css::uno::Reference<css::xml::crypto::XXMLSignatureTemplate>& aTemplate,
        const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& aEnvironment) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
