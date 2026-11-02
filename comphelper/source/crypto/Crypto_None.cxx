/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/crypto/Crypto.hxx>
#include <sal/types.h>
#include <config_oox.h>

namespace comphelper
{
namespace
{
class CryptoImplementationNone : public ICryptoImplementation
{
public:
    CryptoImplementationNone() = default;

    virtual ~CryptoImplementationNone() {}

    void setupEncryptContext(std::vector<sal_uInt8>& /*key*/, std::vector<sal_uInt8>& /*iv*/,
                             CryptoType /*eType*/) override
    {
    }

    void setupDecryptContext(std::vector<sal_uInt8>& /*key*/, std::vector<sal_uInt8>& /*iv*/,
                             CryptoType /*eType*/) override
    {
    }

    void setupCryptoHashContext(std::vector<sal_uInt8>& /*rKey*/, CryptoHashType /*eType*/) override
    {
    }

    sal_uInt32 decryptUpdate(std::vector<sal_uInt8>& /*output*/, std::vector<sal_uInt8>& /*input*/,
                             sal_uInt32 /*inputLength*/) override
    {
        return 0;
    }

    sal_uInt32 encryptUpdate(std::vector<sal_uInt8>& /*output*/, std::vector<sal_uInt8>& /*input*/,
                             sal_uInt32 /*inputLength*/) override
    {
        return 0;
    }

    bool cryptoHashUpdate(std::vector<sal_uInt8>& /*rInput*/, sal_uInt32 /*nInputLength*/) override
    {
        return false;
    }

    bool cryptoHashFinalize(std::vector<sal_uInt8>& /*rHash*/) override { return false; }
};
} // anonymous namespace

std::shared_ptr<ICryptoImplementation> ICryptoImplementation::createInstance()
{
    return std::shared_ptr<ICryptoImplementation>(new CryptoImplementationNone);
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
