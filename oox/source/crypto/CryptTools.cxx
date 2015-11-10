/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "oox/crypto/CryptTools.hxx"
#include <com/sun/star/uno/RuntimeException.hpp>

namespace oox {
namespace core {

using namespace std;

Crypto::Crypto(CryptoType type)
    : mType(type)
#if USE_TLS_NSS
    , mContext(nullptr)
    , mSecParam(nullptr)
    , mSymKey(nullptr)
#endif
{
#if USE_TLS_NSS
    // Initialize NSS, database functions are not needed
    NSS_NoDB_Init(nullptr);
#endif // USE_TLS_NSS
}

Crypto::~Crypto()
{
#if USE_TLS_OPENSSL
    EVP_CIPHER_CTX_cleanup( &mContext );
#endif
#if USE_TLS_NSS
    PK11_DestroyContext( mContext, PR_TRUE );
    PK11_FreeSymKey( mSymKey );
    SECITEM_FreeItem( mSecParam, PR_TRUE );
#endif
}

#if USE_TLS_OPENSSL
const EVP_CIPHER* Crypto::getCipher(CryptoType type)
{
    switch(type)
    {
        case AES_128_ECB:
            return EVP_aes_128_ecb();
        case AES_128_CBC:
            return EVP_aes_128_cbc();
        case AES_256_CBC:
            return EVP_aes_256_cbc();
        default:
            break;
    }
    return NULL;
}
#endif

#if USE_TLS_NSS
void Crypto::setupContext(vector<sal_uInt8>& key, vector<sal_uInt8>& iv, CryptoType type, CK_ATTRIBUTE_TYPE operation)
{
    CK_MECHANISM_TYPE mechanism = static_cast<CK_ULONG>(-1);

    SECItem ivItem;
    ivItem.type = siBuffer;
    if(iv.empty())
        ivItem.data = nullptr;
    else
        ivItem.data = &iv[0];
    ivItem.len = iv.size();

    SECItem* pIvItem = nullptr;

    switch(type)
    {
        case AES_128_ECB:
            mechanism = CKM_AES_ECB;
            break;
        case AES_128_CBC:
            mechanism = CKM_AES_CBC;
            pIvItem = &ivItem;
            break;
        case AES_256_CBC:
            mechanism = CKM_AES_CBC;
            pIvItem = &ivItem;
            break;
        default:
            break;
    }

    PK11SlotInfo* pSlot( PK11_GetBestSlot( mechanism, nullptr ) );

    if (!pSlot)
        throw css::uno::RuntimeException("NSS Slot failure", css::uno::Reference<css::uno::XInterface>());

    SECItem keyItem;
    keyItem.type = siBuffer;
    keyItem.data = &key[0];
    keyItem.len  = key.size();

    mSymKey = PK11_ImportSymKey( pSlot, mechanism, PK11_OriginUnwrap, CKA_ENCRYPT, &keyItem, nullptr );
    mSecParam = PK11_ParamFromIV( mechanism, pIvItem );
    mContext = PK11_CreateContextBySymKey( mechanism, operation, mSymKey, mSecParam );
}
#endif // USE_TLS_NSS

// DECRYPT

Decrypt::Decrypt(vector<sal_uInt8>& key, vector<sal_uInt8>& iv, CryptoType type) :
    Crypto(type)
{
#if USE_TLS_OPENSSL
    EVP_CIPHER_CTX_init( &mContext );

    const EVP_CIPHER* cipher = getCipher(type);

    if (iv.empty())
        EVP_DecryptInit_ex( &mContext, cipher, NULL, &key[0], 0 );
    else
        EVP_DecryptInit_ex( &mContext, cipher, NULL, &key[0], &iv[0] );
    EVP_CIPHER_CTX_set_padding( &mContext, 0 );
#endif

#if USE_TLS_NSS
    setupContext(key, iv, type, CKA_DECRYPT);
#endif // USE_TLS_NSS
}

sal_uInt32 Decrypt::update(vector<sal_uInt8>& output, vector<sal_uInt8>& input, sal_uInt32 inputLength)
{
    int outputLength = 0;

    sal_uInt32 actualInputLength = inputLength == 0 || inputLength > input.size() ? input.size() : inputLength;

#if USE_TLS_OPENSSL
    (void)EVP_DecryptUpdate( &mContext, &output[0], &outputLength, &input[0], actualInputLength );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    (void)PK11_CipherOp( mContext, &output[0], &outputLength, actualInputLength, &input[0], actualInputLength );
#endif // USE_TLS_NSS

    return static_cast<sal_uInt32>(outputLength);
}

sal_uInt32 Decrypt::aes128ecb(vector<sal_uInt8>& output, vector<sal_uInt8>& input, vector<sal_uInt8>& key)
{
    sal_uInt32 outputLength = 0;
    vector<sal_uInt8> iv;
    Decrypt crypto(key, iv, Crypto::AES_128_ECB);
    outputLength = crypto.update(output, input);
    return outputLength;
}

// ENCRYPT

Encrypt::Encrypt(vector<sal_uInt8>& key, vector<sal_uInt8>& iv, CryptoType type) :
    Crypto(type)
{
#if USE_TLS_OPENSSL
    EVP_CIPHER_CTX_init( &mContext );

    const EVP_CIPHER* cipher = getCipher(type);

    if (iv.empty())
        EVP_EncryptInit_ex( &mContext, cipher, NULL, &key[0], 0 );
    else
        EVP_EncryptInit_ex( &mContext, cipher, NULL, &key[0], &iv[0] );
    EVP_CIPHER_CTX_set_padding( &mContext, 0 );
#endif

#if USE_TLS_NSS
    setupContext(key, iv, type, CKA_ENCRYPT);
#endif // USE_TLS_NSS
}

sal_uInt32 Encrypt::update(vector<sal_uInt8>& output, vector<sal_uInt8>& input, sal_uInt32 inputLength)
{
    int outputLength = 0;

    sal_uInt32 actualInputLength = inputLength == 0 || inputLength > input.size() ? input.size() : inputLength;

#if USE_TLS_OPENSSL
    (void)EVP_EncryptUpdate( &mContext, &output[0], &outputLength, &input[0], actualInputLength );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    (void)PK11_CipherOp( mContext, &output[0], &outputLength, actualInputLength, &input[0], actualInputLength );
#endif // USE_TLS_NSS

    return static_cast<sal_uInt32>(outputLength);
}

// Digest

#if USE_TLS_OPENSSL
const sal_uInt32 Digest::DIGEST_LENGTH_SHA1 = SHA_DIGEST_LENGTH;
const sal_uInt32 Digest::DIGEST_LENGTH_SHA512 = SHA512_DIGEST_LENGTH;
#endif
#if USE_TLS_NSS
const sal_uInt32 Digest::DIGEST_LENGTH_SHA1 = SHA1_LENGTH;
const sal_uInt32 Digest::DIGEST_LENGTH_SHA512 = SHA512_LENGTH;
#endif

namespace
{

#if USE_TLS_OPENSSL
const EVP_MD* lclOpenSSLgetEngine(Digest::DigestType eType)
{
    switch(eType)
    {
        case Digest::SHA1:
            return EVP_sha1();
        case Digest::SHA512:
            return EVP_sha512();
        default:
            break;
    }
    return NULL;
}
#endif

#if USE_TLS_NSS
HASH_HashType lclNSSgetHashType(Digest::DigestType eType)
{
    switch(eType)
    {
        case Digest::SHA1:
            return HASH_AlgSHA1;
        case Digest::SHA512:
            return HASH_AlgSHA512;
        default:
            break;
    }
    return HASH_AlgNULL;
}
#endif

}

Digest::Digest(DigestType eType) :
    meType(eType)
{
    #if USE_TLS_OPENSSL
    mpContext = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mpContext, lclOpenSSLgetEngine(eType), NULL);
    #endif

    #if USE_TLS_NSS
    NSS_NoDB_Init(nullptr);
    mpContext = HASH_Create(lclNSSgetHashType(eType));
    HASH_Begin(mpContext);
    #endif
}

Digest::~Digest()
{
    #if USE_TLS_OPENSSL
    if(mpContext)
        EVP_MD_CTX_destroy(mpContext);
    #endif

    #if USE_TLS_NSS
    if(mpContext)
        HASH_Destroy(mpContext);
    #endif
}

sal_uInt32 Digest::getLength()
{
    switch(meType)
    {
        case SHA1:
            return DIGEST_LENGTH_SHA1;
        case SHA512:
            return DIGEST_LENGTH_SHA512;
        default:
            break;
    }
    return 0;
}

bool Digest::update(std::vector<sal_uInt8>& input)
{
    #if USE_TLS_OPENSSL
    EVP_DigestUpdate(mpContext, &input[0], input.size());
    #endif
    #if USE_TLS_NSS
    HASH_Update(mpContext, &input[0], input.size());
    #endif
    return true;
}

bool Digest::finalize(std::vector<sal_uInt8>& digest)
{
    digest.clear();

    #if USE_TLS_OPENSSL
    unsigned int digestWrittenLength;
    digest.resize(getLength(), 0);
    EVP_DigestFinal_ex(mpContext, &digest[0], &digestWrittenLength);
    #endif

    #if USE_TLS_NSS
    unsigned int digestWrittenLength;
    unsigned int digestLength = static_cast<unsigned int>(getLength());
    digest.resize(digestLength, 0);
    HASH_End(mpContext, &digest[0], &digestWrittenLength, digestLength);
    #endif
    return true;
}

bool Digest::sha1(vector<sal_uInt8>& output, vector<sal_uInt8>& input)
{
    bool aResult = false;

    Digest aDigest(SHA1);
    aDigest.update(input);
    aDigest.finalize(output);
    aResult = true;
    return aResult;
}

bool Digest::sha512(vector<sal_uInt8>& output, vector<sal_uInt8>& input)
{
    bool aResult = false;

    Digest aDigest(SHA512);
    aDigest.update(input);
    aDigest.finalize(output);
    aResult = true;
    return aResult;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
