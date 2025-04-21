/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <comphelper/comphelperdllapi.h>
#include <sal/types.h>

#include <vector>
#include <memory>

namespace comphelper
{
/** Rounds up the input to the nearest multiple
 *
 *  For example:
 *  input  1, multiple 16 = 16
 *  input 16, multiple 16 = 16
 *  input 17, multiple 16 = 32
 *  input 31, multiple 16 = 32
 */
template <typename T> T roundUp(T input, T multiple)
{
    if (input % multiple == 0)
        return input;
    return ((input / multiple) * multiple) + multiple;
}

enum class CryptoHashType
{
    SHA1,
    SHA256,
    SHA384,
    SHA512
};

enum class CryptoType
{
    UNKNOWN,
    AES_128_ECB,
    AES_128_CBC,
    AES_192_ECB,
    AES_192_CBC,
    AES_256_ECB,
    AES_256_CBC,
};

class ICryptoImplementation
{
public:
    static std::shared_ptr<ICryptoImplementation> createInstance();

    virtual void setupDecryptContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv,
                                     CryptoType eType)
        = 0;
    virtual void setupEncryptContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv,
                                     CryptoType type)
        = 0;
    virtual void setupCryptoHashContext(std::vector<sal_uInt8>& rKey, CryptoHashType eType) = 0;

    virtual sal_uInt32 decryptUpdate(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                                     sal_uInt32 inputLength)
        = 0;
    virtual sal_uInt32 encryptUpdate(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                                     sal_uInt32 inputLength)
        = 0;
    virtual bool cryptoHashUpdate(std::vector<sal_uInt8>& rInput, sal_uInt32 nInputLength) = 0;
    virtual bool cryptoHashFinalize(std::vector<sal_uInt8>& rHash) = 0;
};

class COMPHELPER_DLLPUBLIC Crypto
{
protected:
    std::shared_ptr<ICryptoImplementation> mpImpl;

    Crypto();

public:
    virtual ~Crypto();
};

/** Decrypt vector of bytes with AES encryption */
class COMPHELPER_DLLPUBLIC Decrypt final : public Crypto
{
public:
    Decrypt(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, CryptoType type);

    sal_uInt32 update(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                      sal_uInt32 inputLength = 0);

    static sal_uInt32 aes128ecb(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                                std::vector<sal_uInt8>& key);
};

/** Encrypt vector of bytes with AES encryption */
class COMPHELPER_DLLPUBLIC Encrypt final : public Crypto
{
public:
    /** Initialize encryption for key, init vector and encryption type.
     *
     * key - encryption key, key size should be the same as block size
     * iv - init vector: it can be empty - will not be used (init vector will be 0)
     */
    Encrypt(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, CryptoType type);

    /** Encrypt the input and write into output
     *
     * inputLength - size from the input to be encrypted (0 means to use the size of the vector)
     */
    sal_uInt32 update(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                      sal_uInt32 inputLength = 0);
};

class COMPHELPER_DLLPUBLIC CryptoHash final : public Crypto
{
    sal_Int32 mnHashSize;

public:
    CryptoHash(std::vector<sal_uInt8>& rKey, CryptoHashType eType);
    bool update(std::vector<sal_uInt8>& rInput, sal_uInt32 nInputLength = 0);
    std::vector<sal_uInt8> finalize();
};

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
