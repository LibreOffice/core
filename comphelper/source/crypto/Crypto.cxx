/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/crypto/Crypto.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <sal/types.h>
#include <config_oox.h>

namespace comphelper
{
Crypto::Crypto()
    : mpImpl(ICryptoImplementation::createInstance())
{
}
Crypto::~Crypto() = default;

// DECRYPT

Decrypt::Decrypt(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, CryptoType type)
{
    mpImpl->setupDecryptContext(key, iv, type);
}

sal_uInt32 Decrypt::update(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                           sal_uInt32 inputLength)
{
    sal_uInt32 actualInputLength
        = inputLength == 0 || inputLength > input.size() ? input.size() : inputLength;
    return mpImpl->decryptUpdate(output, input, actualInputLength);
}

sal_uInt32 Decrypt::aes128ecb(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                              std::vector<sal_uInt8>& key)
{
    sal_uInt32 outputLength = 0;
    std::vector<sal_uInt8> iv;
    Decrypt crypto(key, iv, CryptoType::AES_128_ECB);
    outputLength = crypto.update(output, input);
    return outputLength;
}

// ENCRYPT

Encrypt::Encrypt(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, CryptoType type)
{
    mpImpl->setupEncryptContext(key, iv, type);
}

sal_uInt32 Encrypt::update(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                           sal_uInt32 inputLength)
{
    sal_uInt32 actualInputLength
        = inputLength == 0 || inputLength > input.size() ? input.size() : inputLength;
    return mpImpl->encryptUpdate(output, input, actualInputLength);
}

// CryptoHash - HMAC

namespace
{
sal_Int32 getSizeForHashType(CryptoHashType eType)
{
    switch (eType)
    {
        case CryptoHashType::SHA1:
            return 20;
        case CryptoHashType::SHA256:
            return 32;
        case CryptoHashType::SHA384:
            return 48;
        case CryptoHashType::SHA512:
            return 64;
    }
    return 0;
}

} // end anonymous namespace

CryptoHash::CryptoHash(std::vector<sal_uInt8>& rKey, CryptoHashType eType)
    : mnHashSize(getSizeForHashType(eType))
{
    mpImpl->setupCryptoHashContext(rKey, eType);
}

bool CryptoHash::update(std::vector<sal_uInt8>& rInput, sal_uInt32 nInputLength)
{
    sal_uInt32 nActualInputLength
        = (nInputLength == 0 || nInputLength > rInput.size()) ? rInput.size() : nInputLength;
    return mpImpl->cryptoHashUpdate(rInput, nActualInputLength);
}

std::vector<sal_uInt8> CryptoHash::finalize()
{
    std::vector<sal_uInt8> aHash(mnHashSize, 0);
    mpImpl->cryptoHashFinalize(aHash);
    return aHash;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
