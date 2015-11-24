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

#ifndef INCLUDED_OOX_CRYPTO_CRYPTTOOLS_HXX
#define INCLUDED_OOX_CRYPTO_CRYPTTOOLS_HXX

#include <config_oox.h>

#include <rtl/ustring.hxx>

#if USE_TLS_OPENSSL
#include <openssl/evp.h>
#include <openssl/sha.h>
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
#include <nss.h>
#include <pk11pub.h>
#include <sechash.h>
#endif // USE_TLS_NSS

#include <rtl/digest.h>
#include <vector>

namespace oox {
namespace core {

class Crypto
{
public:
    enum CryptoType
    {
        UNKNOWN,
        AES_128_ECB,
        AES_128_CBC,
        AES_256_CBC,
    };

protected:
#if USE_TLS_OPENSSL
    EVP_CIPHER_CTX mContext;
#endif
#if USE_TLS_NSS
    PK11Context* mContext;
    SECItem*     mSecParam;
    PK11SymKey*  mSymKey;
#endif

#if USE_TLS_OPENSSL
    const EVP_CIPHER* getCipher(CryptoType type);
#endif
#if USE_TLS_NSS
    void setupContext(
            std::vector<sal_uInt8>& key,
            std::vector<sal_uInt8>& iv,
            CryptoType type,
            CK_ATTRIBUTE_TYPE operation);
#endif

protected:
    Crypto();

public:
    virtual ~Crypto();

    virtual sal_uInt32 update(
                    std::vector<sal_uInt8>& output,
                    std::vector<sal_uInt8>& input,
                    sal_uInt32 inputLength = 0) = 0;
};

class Decrypt : public Crypto
{
public:
    Decrypt(std::vector<sal_uInt8>& key, CryptoType type);
    Decrypt(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, CryptoType type);

    virtual sal_uInt32 update(
                    std::vector<sal_uInt8>& output,
                    std::vector<sal_uInt8>& input,
                    sal_uInt32 inputLength = 0) override;


    static sal_uInt32 aes128ecb(
                    std::vector<sal_uInt8>& output,
                    std::vector<sal_uInt8>& input,
                    std::vector<sal_uInt8>& key );

};

class Encrypt : public Crypto
{
public:
    Encrypt(std::vector<sal_uInt8>& key, CryptoType type);
    Encrypt(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, CryptoType type);

    virtual sal_uInt32 update(
                    std::vector<sal_uInt8>& output,
                    std::vector<sal_uInt8>& input,
                    sal_uInt32 inputLength = 0) override;
};

class Digest
{
public:
    enum DigestType
    {
        UNKNOWN,
        SHA1,
        SHA512
    };

    static const sal_uInt32 DIGEST_LENGTH_SHA1;
    static const sal_uInt32 DIGEST_LENGTH_SHA512;

private:
    DigestType meType;

#if USE_TLS_OPENSSL
    EVP_MD_CTX* mpContext;
#endif

#if USE_TLS_NSS
    HASHContext* mpContext;
#endif

public:
    Digest(DigestType eType);
    virtual ~Digest();

    bool update(std::vector<sal_uInt8>& input);
    bool finalize(std::vector<sal_uInt8>& digest);

    sal_uInt32 getLength();

    static bool sha1(  std::vector<sal_uInt8>& digest, std::vector<sal_uInt8>& input);
    static bool sha512(std::vector<sal_uInt8>& digest, std::vector<sal_uInt8>& input);
};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
