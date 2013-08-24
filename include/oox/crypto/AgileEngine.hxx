/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef AGILE_ENGINE_HXX
#define AGILE_ENGINE_HXX

#include "CryptTools.hxx"
#include "CryptoEngine.hxx"

namespace oox {
namespace core {

const sal_uInt32 SEGMENT_LENGTH = 4096;

struct AgileEncryptionInfo
{
    sal_Int32               spinCount;
    sal_Int32               saltSize;
    sal_Int32               keyBits;
    sal_Int32               hashSize;
    sal_Int32               blockSize;

    OUString cipherAlgorithm;
    OUString cipherChaining;
    OUString hashAlgorithm;

    std::vector<sal_uInt8>  keyDataSalt;
    std::vector<sal_uInt8>  saltValue;
    std::vector<sal_uInt8>  encryptedVerifierHashInput;
    std::vector<sal_uInt8>  encryptedVerifierHashValue;
    std::vector<sal_uInt8>  encryptedKeyValue;
};

class AgileEngine : public CryptoEngine
{
    AgileEncryptionInfo mInfo;

    bool calculateHashFinal(const OUString& rPassword, std::vector<sal_uInt8>& aHashFinal);

    bool calculateBlock(
            const sal_uInt8* rBlock,
            sal_uInt32 aBlockSize,
            std::vector<sal_uInt8>& rHashFinal,
            std::vector<sal_uInt8>& rInput,
            std::vector<sal_uInt8>& rOutput);

    Crypto::CryptoType cryptoType(const AgileEncryptionInfo& rInfo);

public:
    AgileEngine();
    virtual ~AgileEngine();

    AgileEncryptionInfo& getInfo();

    virtual bool writeEncryptionInfo(
                    const OUString& rPassword,
                    BinaryXOutputStream& rStream);

    virtual bool generateEncryptionKey(const OUString& rPassword);

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
