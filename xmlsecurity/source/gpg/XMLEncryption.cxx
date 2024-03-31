/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLEncryption.hxx"

using namespace css::uno;
using namespace css::lang;
using namespace css::xml::crypto;

XMLEncryptionGpg::XMLEncryptionGpg() {
}

XMLEncryptionGpg::~XMLEncryptionGpg() {
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate > SAL_CALL XMLEncryptionGpg::encrypt(const Reference< XXMLEncryptionTemplate >& /*aTemplate*/,
                                                                    const Reference< XSecurityEnvironment >& /*aEnvironment*/)
{
    return nullptr;
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate > SAL_CALL XMLEncryptionGpg::decrypt(const Reference< XXMLEncryptionTemplate >& /*aTemplate*/,
                                                                    const Reference< XXMLSecurityContext >& /*aSecurityCtx*/)
{
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
