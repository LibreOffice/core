/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cryptocredentials.hxx"
#include <vector>

namespace sc
{
CredentialEncryption::CredentialEncryption()
{
    // Setup input
    aPassword = "Password";
}

bool CredentialEncryption::encrypt()
{
    //    std::vector<sal_uInt8> iv;
    //    std::vector<sal_uInt8> mKey;
    // Write content
    //     oox::core::Encrypt aEncryptor(mKey, iv, oox::core::Crypto::CryptoType::AES_128_ECB);
    return true;
}

CredentialDecryption::CredentialDecryption() { aPassword = "Password"; }

bool CredentialDecryption::decrypt() { return true; }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
