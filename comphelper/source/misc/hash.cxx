/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/hash.hxx>
#include <config_oox.h>

#if USE_TLS_NSS
#include <nss.h>
#include <pk11pub.h>
#include <sechash.h>
#elif USE_TLS_OPENSSL
#include <openssl/evp.h>
#include <openssl/sha.h>
#endif // USE_TLS_OPENSSL

namespace comphelper {

struct HashImpl
{

#if USE_TLS_NSS
    HASHContext* mpContext;

    HASH_HashType getNSSType() const
    {
        switch (meType)
        {
            case HashType::MD5:
                return HASH_AlgMD5;
            case HashType::SHA1:
                return HASH_AlgSHA1;
            case HashType::SHA256:
                return HASH_AlgSHA256;
            case HashType::SHA512:
                return HASH_AlgSHA512;
        }

        return HASH_AlgNULL;
    }
#elif USE_TLS_OPENSSL
    EVP_MD_CTX* mpContext;

    const EVP_MD* getOpenSSLType() const
    {
        switch (meType)
        {
            case HashType::MD5:
                return EVP_md5();
            case HashType::SHA1:
                return EVP_sha1();
            case HashType::SHA256:
                return EVP_sha256();
            case HashType::SHA512:
                return EVP_sha512();
        }

        return nullptr;
    }
#endif

    HashType meType;

    HashImpl(HashType eType):
        meType(eType)
    {

#if USE_TLS_NSS
        NSS_NoDB_Init(nullptr);
        mpContext = HASH_Create(getNSSType());
        HASH_Begin(mpContext);
#elif USE_TLS_OPENSSL
        mpContext = EVP_MD_CTX_create();
        EVP_DigestInit_ex(mpContext, getOpenSSLType(), NULL);
#endif
    }

    ~HashImpl()
    {
#if USE_TLS_NSS
        HASH_Destroy(mpContext);
#elif USE_TLS_OPENSSL
        EVP_MD_CTX_destroy(mpContext);
#endif
    }
};

Hash::Hash(HashType eType):
    mpImpl(new HashImpl(eType))
{
}

Hash::~Hash()
{
}

void Hash::update(const unsigned char* pInput, size_t length)
{
#if USE_TLS_NSS
    HASH_Update(mpImpl->mpContext, pInput, length);
#elif USE_TLS_OPENSSL
    EVP_DigestUpdate(mpImpl->mpContext, pInput, length);
#else
    (void)pInput;
    (void)length;
#endif
}

std::vector<unsigned char> Hash::finalize()
{
    std::vector<unsigned char> hash(getLength(), 0);
    unsigned int digestWrittenLength;
#if USE_TLS_NSS
    HASH_End(mpImpl->mpContext, hash.data(), &digestWrittenLength, getLength());
#elif USE_TLS_OPENSSL
    EVP_DigestFinal_ex(mpImpl->mpContext, hash.data(), &digestWrittenLength);
#else
    (void)digestWrittenLength;
#endif

    return hash;
}

size_t Hash::getLength() const
{
    switch (mpImpl->meType)
    {
        case HashType::MD5:
            return 16;
        case HashType::SHA1:
            return 20;
        case HashType::SHA256:
            return 32;
        case HashType::SHA512:
            return 64;
    }

    return 0;
}

std::vector<unsigned char> Hash::calculateHash(const unsigned char* pInput, size_t length, HashType eType)
{
    Hash aHash(eType);
    aHash.update(pInput, length);
    return aHash.finalize();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
