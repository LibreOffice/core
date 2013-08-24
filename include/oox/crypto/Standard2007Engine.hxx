/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef STANDARD_2007_ENGINE_HXX
#define STANDARD_2007_ENGINE_HXX

#include "CryptTools.hxx"
#include "CryptoEngine.hxx"

namespace oox {
namespace core {

const sal_uInt32 ENCRYPTINFO_CRYPTOAPI      = 0x00000004;
const sal_uInt32 ENCRYPTINFO_DOCPROPS       = 0x00000008;
const sal_uInt32 ENCRYPTINFO_EXTERNAL       = 0x00000010;
const sal_uInt32 ENCRYPTINFO_AES            = 0x00000020;

const sal_uInt32 ENCRYPT_ALGO_AES128        = 0x0000660E;
const sal_uInt32 ENCRYPT_ALGO_AES192        = 0x0000660F;
const sal_uInt32 ENCRYPT_ALGO_AES256        = 0x00006610;
const sal_uInt32 ENCRYPT_ALGO_RC4           = 0x00006801;

const sal_uInt32 ENCRYPT_HASH_SHA1          = 0x00008004;

const sal_uInt32 ENCRYPT_KEY_SIZE_AES_128   = 0x00000080;
const sal_uInt32 ENCRYPT_KEY_SIZE_AES_192   = 0x000000C0;
const sal_uInt32 ENCRYPT_KEY_SIZE_AES_256   = 0x00000100;

const sal_uInt32 ENCRYPT_PROVIDER_TYPE_AES  = 0x00000018;
const sal_uInt32 ENCRYPT_PROVIDER_TYPE_RC4  = 0x00000001;

// version of encryption info used in MS Office 2007 (major = 3, minor = 2)
const sal_uInt32 VERSION_INFO_2007_FORMAT   = 0x00020003;
// version of encryption info - agile (major = 4, minor = 4)
const sal_uInt32 VERSION_INFO_AGILE         = 0x00040004;

const sal_uInt32 SALT_LENGTH                    = 16;
const sal_uInt32 ENCRYPTED_VERIFIER_LENGTH      = 16;
const sal_uInt32 ENCRYPTED_VERIFIER_HASH_LENGTH = 32;

struct EncryptionStandardHeader
{
    sal_uInt32 flags;
    sal_uInt32 sizeExtra;       // 0
    sal_uInt32 algId;           // if flag AES && CRYPTOAPI this defaults to 128-bit AES
    sal_uInt32 algIdHash;       // 0: determine by flags - defaults to SHA-1 if not external
    sal_uInt32 keyBits;         // key size in bits: 0 (determine by flags), 128, 192, 256
    sal_uInt32 providedType;    // AES or RC4
    sal_uInt32 reserved1;       // 0
    sal_uInt32 reserved2;       // 0

    EncryptionStandardHeader();
};

struct EncryptionVerifierAES
{
    sal_uInt32 saltSize;                                                // must be 0x00000010
    sal_uInt8  salt[SALT_LENGTH];                                       // random generated salt value
    sal_uInt8  encryptedVerifier[ENCRYPTED_VERIFIER_LENGTH];            // randomly generated verifier value
    sal_uInt32 encryptedVerifierHashSize;                               // actually written hash size - depends on algorithm
    sal_uInt8  encryptedVerifierHash[ENCRYPTED_VERIFIER_HASH_LENGTH];   // verifier value hash - itself also encrypted

    EncryptionVerifierAES();
};

struct StandardEncryptionInfo
{
    EncryptionStandardHeader header;
    EncryptionVerifierAES    verifier;
};

class Standard2007Engine : public CryptoEngine
{
    StandardEncryptionInfo mInfo;

    bool generateVerifier();
    bool calculateEncryptionKey(const OUString& rPassword);

public:
    Standard2007Engine();
    virtual ~Standard2007Engine();

    StandardEncryptionInfo& getInfo();

    static bool checkEncryptionData(
                    std::vector<sal_uInt8> key, sal_uInt32 keySize,
                    std::vector<sal_uInt8> encryptedVerifier, sal_uInt32 verifierSize,
                    std::vector<sal_uInt8> encryptedHash, sal_uInt32 hashSize );

    virtual bool generateEncryptionKey(const OUString& rPassword);

    virtual bool writeEncryptionInfo(
                    const OUString& rPassword,
                    BinaryXOutputStream& rStream);

    virtual bool decrypt(
                    BinaryXInputStream& aInputStream,
                    BinaryXOutputStream& aOutputStream);

    virtual bool encrypt(
                    BinaryXInputStream& aInputStream,
                    BinaryXOutputStream& aOutputStream);

};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
