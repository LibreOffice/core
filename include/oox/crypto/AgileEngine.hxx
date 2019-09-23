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
namespace core {

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
    sal_Int32 const spinCount;
    sal_Int32 const saltSize;
    sal_Int32 const keyBits;
    sal_Int32 const hashSize;
    sal_Int32 const blockSize;

    OUString const cipherAlgorithm;
    OUString const cipherChaining;
    OUString const hashAlgorithm;
};

enum class AgileEncryptionPreset
{
    AES_128_SHA1,
    AES_256_SHA512,
};

class OOX_DLLPUBLIC AgileEngine final : public CryptoEngine
{
private:
    std::vector<sal_uInt8> mKey;
    AgileEncryptionInfo mInfo;
    AgileEncryptionPreset meEncryptionPreset;
    css::uno::Reference< css::uno::XComponentContext > mxContext;

    css::uno::Reference<css::io::XInputStream> getStream(css::uno::Sequence<css::beans::NamedValue> & rStreams, const OUString sStreamName);

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

    // Decryption

    bool decryptHmacKey();
    bool decryptHmacValue();

    AgileEncryptionInfo& getInfo() { return mInfo; }

    void setPreset(AgileEncryptionPreset ePreset)
    {
        meEncryptionPreset = ePreset;
    }

    void decryptEncryptionKey(OUString const & rPassword);
    bool decryptAndCheckVerifierHash(OUString const & rPassword);

    // Encryption

    bool encryptHmacKey();
    bool encryptHmacValue();

    bool generateAndEncryptVerifierHash(OUString const & rPassword);

    bool encryptEncryptionKey(OUString const & rPassword);
    void setupEncryptionParameters(AgileEncryptionParameters const & rAgileEncryptionParameters);
    bool setupEncryptionKey(OUString const & rPassword);

public:
    AgileEngine(const css::uno::Reference< css::uno::XComponentContext >& rxContext);

    // Decryption

    bool generateEncryptionKey(OUString const & rPassword) override;
    bool readEncryptionInfo(css::uno::Sequence<css::beans::NamedValue> aStreams) override;
    bool decrypt(BinaryXInputStream& aInputStream,
                 BinaryXOutputStream& aOutputStream) override;

    bool checkDataIntegrity() override;

    // Encryption

    void writeEncryptionInfo(oox::ole::OleStorage& rOleStorage) override;

    void encrypt(css::uno::Reference<css::io::XInputStream>&  rxInputStream,
                 css::uno::Reference<css::io::XOutputStream>& rxOutputStream,
                 sal_uInt32 nSize) override;

    bool setupEncryption(css::uno::Sequence<css::beans::NamedValue>& rMediaEncData) override;

    virtual void createEncryptionData(comphelper::SequenceAsHashMap & aEncryptionData, const OUString rPassword) override;
};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
