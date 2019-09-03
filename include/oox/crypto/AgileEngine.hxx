/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_OOX_CRYPTO_AGILEENGINE_HXX
#define INCLUDED_OOX_CRYPTO_AGILEENGINE_HXX

#include <vector>

#include <oox/dllapi.h>
#include <oox/crypto/CryptTools.hxx>
#include <oox/crypto/CryptoEngine.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace oox {
    class BinaryXInputStream;
    class BinaryXOutputStream;
}

namespace oox {
namespace crypto {

struct OOX_DLLPUBLIC AgileEncryptionInfo
{
    sal_Int32 spinCount;
    sal_Int32 saltSize;
    sal_Int32 keyBits;
    sal_Int32 hashSize;
    sal_Int32 blockSize;

    OUString cipherAlgorithm;
    OUString cipherChaining;
    OUString hashAlgorithm;

    std::vector<sal_uInt8> keyDataSalt;

    // Key Encryptor
    std::vector<sal_uInt8> saltValue;
    std::vector<sal_uInt8> encryptedVerifierHashInput;
    std::vector<sal_uInt8> encryptedVerifierHashValue;
    std::vector<sal_uInt8> encryptedKeyValue;

    // HMAC
    std::vector<sal_uInt8> hmacKey;
    std::vector<sal_uInt8> hmacHash;
    std::vector<sal_uInt8> hmacCalculatedHash;
    std::vector<sal_uInt8> hmacEncryptedKey; // encrypted Key
    std::vector<sal_uInt8> hmacEncryptedValue; // encrypted Hash
};

struct OOX_DLLPUBLIC AgileEncryptionParameters
{
    sal_Int32 spinCount;
    sal_Int32 saltSize;
    sal_Int32 keyBits;
    sal_Int32 hashSize;
    sal_Int32 blockSize;

    OUString cipherAlgorithm;
    OUString cipherChaining;
    OUString hashAlgorithm;
};

enum class AgileEncryptionPreset
{
    AES_128_SHA1,
    AES_256_SHA512,
};

class OOX_DLLPUBLIC AgileEngine final : public CryptoEngine
{
private:
    AgileEncryptionInfo mInfo;
    AgileEncryptionPreset meEncryptionPreset;

    void calculateHashFinal(const OUString& rPassword, std::vector<sal_uInt8>& aHashFinal);

    void calculateBlock(
            std::vector<sal_uInt8> const & rBlock,
            std::vector<sal_uInt8>& rHashFinal,
            std::vector<sal_uInt8>& rInput,
            std::vector<sal_uInt8>& rOutput);

    void encryptBlock(
            std::vector<sal_uInt8> const & rBlock,
            std::vector<sal_uInt8>& rHashFinal,
            std::vector<sal_uInt8>& rInput,
            std::vector<sal_uInt8>& rOutput);

    static Crypto::CryptoType cryptoType(const AgileEncryptionInfo& rInfo);

public:
    AgileEngine();

    AgileEncryptionInfo& getInfo() { return mInfo;}

    void setPreset(AgileEncryptionPreset ePreset)
    {
        meEncryptionPreset = ePreset;
    }

    // Decryption

    void decryptEncryptionKey(OUString const & rPassword);
    bool decryptAndCheckVerifierHash(OUString const & rPassword);

    bool generateEncryptionKey(OUString const & rPassword) override;
    bool readEncryptionInfo(css::uno::Reference<css::io::XInputStream> & rxInputStream) override;
    bool decrypt(BinaryXInputStream& aInputStream,
                 BinaryXOutputStream& aOutputStream) override;

    bool checkDataIntegrity() override;

    bool decryptHmacKey();
    bool decryptHmacValue();

    // Encryption

    void writeEncryptionInfo(BinaryXOutputStream& rStream) override;

    void encrypt(const css::uno::Reference<css::io::XInputStream>&  rxInputStream,
                 css::uno::Reference<css::io::XOutputStream>& rxOutputStream,
                 sal_uInt32 nSize) override;

    bool setupEncryption(OUString const & rPassword) override;

    bool generateAndEncryptVerifierHash(OUString const & rPassword);

    bool encryptHmacKey();
    bool encryptHmacValue();

    bool encryptEncryptionKey(OUString const & rPassword);
    void setupEncryptionParameters(AgileEncryptionParameters const & rAgileEncryptionParameters);
    bool setupEncryptionKey(OUString const & rPassword);
};

} // namespace crypto
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
