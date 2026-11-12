/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "pptcrypt.hxx"

#include <rtl/alloc.h>
#include <rtl/cipher.h>
#include <rtl/digest.h>

#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <vector>

namespace
{
// by definition, SHA1 size is 20 bytes
constexpr sal_uInt32 SHA1_HASH_SIZE = 20;

// MS-OFFCRYPTO 2.3.3: An array of bytes that specifies the salt value used during password hash generation
constexpr sal_uInt32 CRYPTOAPI_SALT_SIZE = 16;

// MS-OFFCRYPTO 2.3.3: A value that MUST be the randomly generated Verifier value
constexpr sal_uInt32 ENCRYPTION_VERIFIER_SIZE = 16;

constexpr sal_uInt32 RC4_MAX_KEY_SIZE = 16;

// MS-OFFCRYPTO 2.3.5.1: If set to 0x00000000, it MUST be interpreted as 0x00000028 bits
constexpr sal_uInt32 RC4_KEY_BITS_UNSPECIFIED = 0;
constexpr sal_uInt32 RC4_DEFAULT_KEY_BITS = 40;

constexpr sal_uInt32 RC4_40BIT_EFFECTIVE_KEY_SIZE = 16;

// MS-OFFCRYPTO 2.3.5.2: Unless otherwise specified, the maximum password length MUST be 255 Unicode characters
constexpr sal_Int32 MAX_PASSWORD_LENGTH = 255;

using DigestHandle = std::unique_ptr<void, decltype(&rtl_digest_destroy)>;

using CipherHandle = std::unique_ptr<void, decltype(&rtl_cipher_destroyARCFOUR)>;

using SHA1Hash = std::array<sal_uInt8, SHA1_HASH_SIZE>;
using RC4Key = std::array<sal_uInt8, RC4_MAX_KEY_SIZE>;

// for RAII
struct SecureArrayCleaner
{
    sal_uInt8* pData;
    size_t nSize;

    ~SecureArrayCleaner() { rtl_secureZeroMemory(pData, nSize); }
};

bool calculateSHA1(const std::vector<sal_uInt8>& rData, SHA1Hash& rHash)
{
    if (rData.size() > std::numeric_limits<sal_uInt32>::max())
        return false;

    DigestHandle xDigest(rtl_digest_create(rtl_Digest_AlgorithmSHA1), rtl_digest_destroy);

    if (!xDigest)
        return false;

    if (rtl_digest_update(xDigest.get(), rData.data(), static_cast<sal_uInt32>(rData.size()))
        != rtl_Digest_E_None)
    {
        return false;
    }

    return rtl_digest_get(xDigest.get(), rHash.data(), SHA1_HASH_SIZE) == rtl_Digest_E_None;
}

bool deriveKey(const OUString& rPassword, const std::vector<sal_uInt8>& rSalt, sal_uInt32 nBlock,
               sal_uInt32 nKeyBits, RC4Key& rKey)
{
    if (rSalt.size() != CRYPTOAPI_SALT_SIZE || rPassword.getLength() > MAX_PASSWORD_LENGTH)
        return false;

    // MS-OFFCRYPTO 2.3.2: 0x00000028 – 0x00000080 (inclusive) 8-bit increments
    if (nKeyBits < 40 || nKeyBits > 128 || nKeyBits % 8 != 0)
        return false;

    // Let's calculate H0 (ie SHA1(Salt + Password_UTF16LE) )
    std::vector<sal_uInt8> aInitialData;
    aInitialData.reserve(rSalt.size() + rPassword.getLength() * 2);

    aInitialData.insert(aInitialData.end(), rSalt.begin(), rSalt.end());

    // we fill initialData with the password that must be converted in UTF16LE
    for (sal_Int32 i = 0; i < rPassword.getLength(); ++i)
    {
        const sal_uInt16 nChar = rPassword[i];

        aInitialData.push_back(static_cast<sal_uInt8>(nChar));
        aInitialData.push_back(static_cast<sal_uInt8>(nChar >> 8));
    }

    SHA1Hash aH0{};

    if (!calculateSHA1(aInitialData, aH0))
        return false;

    // Hfinal = SHA1(H0 + BlockNumber in little-endian 32 bits)

    std::vector<sal_uInt8> aFinalData(aH0.begin(), aH0.end());

    aFinalData.push_back(static_cast<sal_uInt8>(nBlock));
    aFinalData.push_back(static_cast<sal_uInt8>(nBlock >> 8));
    aFinalData.push_back(static_cast<sal_uInt8>(nBlock >> 16));
    aFinalData.push_back(static_cast<sal_uInt8>(nBlock >> 24));

    SHA1Hash aHFinal{};

    if (!calculateSHA1(aFinalData, aHFinal))
        return false;

    // Copy the first nKeyBytes bytes of aHFinal into rKey
    rKey.fill(0);

    const sal_uInt32 nKeyBytes = nKeyBits / 8;

    std::copy_n(aHFinal.begin(), nKeyBytes, rKey.begin());

    // for security sake
    rtl_secureZeroMemory(aInitialData.data(), aInitialData.size());
    rtl_secureZeroMemory(aH0.data(), aH0.size());
    rtl_secureZeroMemory(aFinalData.data(), aFinalData.size());
    rtl_secureZeroMemory(aHFinal.data(), aHFinal.size());

    return true;
}

bool decryptRC4Data(const RC4Key& rKey, sal_uInt32 nKeyBits,
                    const std::vector<sal_uInt8>& rEncryptedData,
                    std::vector<sal_uInt8>& rDecryptedData)
{
    if (rEncryptedData.size() > std::numeric_limits<sal_uInt32>::max())
        return false;

    if (rEncryptedData.empty())
    {
        rDecryptedData.clear();
        return true;
    }

    CipherHandle xCipher(rtl_cipher_createARCFOUR(rtl_Cipher_ModeStream),
                         rtl_cipher_destroyARCFOUR);

    if (!xCipher)
        return false;

    // MS-OFFCRYPTO 2.3.2: a 40-bit key is extended to 128 bits
    // with the remaining 88 bits set to zero.
    const sal_uInt32 nRC4KeyBytes
        = nKeyBits == RC4_DEFAULT_KEY_BITS ? RC4_40BIT_EFFECTIVE_KEY_SIZE : nKeyBits / 8;

    if (rtl_cipher_initARCFOUR(xCipher.get(), rtl_Cipher_DirectionDecode, rKey.data(), nRC4KeyBytes,
                               nullptr, 0)
        != rtl_Cipher_E_None)
    {
        return false;
    }

    std::vector<sal_uInt8> aDecryptedData(rEncryptedData.size());

    if (rtl_cipher_decodeARCFOUR(
            xCipher.get(), rEncryptedData.data(), static_cast<sal_uInt32>(rEncryptedData.size()),
            aDecryptedData.data(), static_cast<sal_uInt32>(aDecryptedData.size()))
        != rtl_Cipher_E_None)
    {
        return false;
    }

    rDecryptedData.swap(aDecryptedData);
    return true;
}

bool decryptVerifier(const RC4Key& rKey, sal_uInt32 nKeyBits,
                     const std::vector<sal_uInt8>& rEncryptedVerifier,
                     const std::vector<sal_uInt8>& rEncryptedHash,
                     std::array<sal_uInt8, ENCRYPTION_VERIFIER_SIZE>& rVerifier,
                     SHA1Hash& rVerifierHash)
{
    // The verifier and its hash share one continuous RC4 stream.
    std::vector<sal_uInt8> aEncryptedData;
    aEncryptedData.reserve(ENCRYPTION_VERIFIER_SIZE + SHA1_HASH_SIZE);
    aEncryptedData.insert(aEncryptedData.end(), rEncryptedVerifier.begin(),
                          rEncryptedVerifier.end());
    aEncryptedData.insert(aEncryptedData.end(), rEncryptedHash.begin(), rEncryptedHash.end());

    std::vector<sal_uInt8> aDecryptedData;
    if (!decryptRC4Data(rKey, nKeyBits, aEncryptedData, aDecryptedData))
        return false;

    if (aDecryptedData.size() != ENCRYPTION_VERIFIER_SIZE + SHA1_HASH_SIZE)
        return false;

    std::copy_n(aDecryptedData.begin(), ENCRYPTION_VERIFIER_SIZE, rVerifier.begin());
    std::copy_n(aDecryptedData.begin() + ENCRYPTION_VERIFIER_SIZE, SHA1_HASH_SIZE,
                rVerifierHash.begin());

    return true;
}

} // namespace

bool PptCrypt::decryptData(const OUString& rPassword, const std::vector<sal_uInt8>& rSalt,
                           sal_uInt32 nBlock, sal_uInt32 nKeyBits,
                           const std::vector<sal_uInt8>& rEncryptedData,
                           std::vector<sal_uInt8>& rDecryptedData)
{
    // MS-OFFCRYPTO 2.3.5.1: an unspecified KeySize means 40 bits.
    if (nKeyBits == RC4_KEY_BITS_UNSPECIFIED)
        nKeyBits = RC4_DEFAULT_KEY_BITS;

    RC4Key aKey{};
    SecureArrayCleaner aKeyCleaner{ aKey.data(), aKey.size() };

    if (!deriveKey(rPassword, rSalt, nBlock, nKeyBits, aKey))
        return false;

    return decryptRC4Data(aKey, nKeyBits, rEncryptedData, rDecryptedData);
}

bool PptCrypt::verifyPassword(const OUString& rPassword, const std::vector<sal_uInt8>& rSalt,
                              const std::vector<sal_uInt8>& rEncryptedVerifier,
                              const std::vector<sal_uInt8>& rEncryptedVerifierHash,
                              sal_uInt32 nKeyBits)
{
    // MS-OFFCRYPTO 2.3.5.1, KeySize "If set to 0x00000000, it MUST be interpreted as 0x00000028 bits."
    // A KeySize of zero (unspecified) means 40 bits (the default).
    if (nKeyBits == RC4_KEY_BITS_UNSPECIFIED)
        nKeyBits = RC4_DEFAULT_KEY_BITS;

    if (rSalt.size() != CRYPTOAPI_SALT_SIZE || rEncryptedVerifier.size() != ENCRYPTION_VERIFIER_SIZE
        || rEncryptedVerifierHash.size() != SHA1_HASH_SIZE)
    {
        return false;
    }

    RC4Key aKey{};

    // use of custom RAII struct
    SecureArrayCleaner aKeyCleaner{ aKey.data(), aKey.size() };

    // we must use block 0 to derive the key from the password and check password
    if (!deriveKey(rPassword, rSalt, /* use of block 0 */ 0, nKeyBits, aKey))
    {
        return false;
    }

    std::array<sal_uInt8, ENCRYPTION_VERIFIER_SIZE> aVerifier{};
    SHA1Hash aVerifierHash{};

    // now we can decrypted the encrypted verifier and the encrypted verifier hash
    if (!decryptVerifier(aKey, nKeyBits, rEncryptedVerifier, rEncryptedVerifierHash, aVerifier,
                         aVerifierHash))
    {
        return false;
    }

    // initialize the vector by copying the data of the array aVerifier
    std::vector<sal_uInt8> aVerifierData(aVerifier.begin(), aVerifier.end());

    SHA1Hash aCalculatedHash{};

    if (!calculateSHA1(aVerifierData, aCalculatedHash))
        return false;

    sal_uInt8 nDifference = 0;
    // compare hash byte per byte
    for (sal_uInt32 i = 0; i < SHA1_HASH_SIZE; ++i)
    {
        nDifference |= aCalculatedHash[i] ^ aVerifierHash[i];
    }

    return nDifference == 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
