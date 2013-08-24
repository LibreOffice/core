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

namespace oox {
namespace core {

using namespace std;

Crypto::Crypto(CryptoType type) :
    mType(type)
{
#if USE_TLS_NSS
    // Initialize NSS, database functions are not needed
    NSS_NoDB_Init(NULL);
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
    CK_MECHANISM_TYPE mechanism = -1;

    SECItem ivItem;
    ivItem.type = siBuffer;
    ivItem.data = &iv[0];
    ivItem.len = iv.size();

    SECItem* pIvItem = NULL;

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

    PK11SlotInfo* aSlot( PK11_GetBestSlot( mechanism, NULL ) );

    SECItem keyItem;
    keyItem.type = siBuffer;
    keyItem.data = &key[0];
    keyItem.len  = key.size();

    mSymKey = PK11_ImportSymKey( aSlot, mechanism, PK11_OriginUnwrap, CKA_ENCRYPT, &keyItem, NULL );
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
    EVP_DecryptUpdate( &mContext, &output[0], &outputLength, &input[0], actualInputLength );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    PK11_CipherOp( mContext, &output[0], &outputLength, actualInputLength, &input[0], actualInputLength );
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

sal_uInt32 Decrypt::aes128cbc(vector<sal_uInt8>& output, vector<sal_uInt8>& input, vector<sal_uInt8>& key, vector<sal_uInt8>& iv)
{
    sal_uInt32 outputLength = 0;
    Decrypt crypto(key, iv, Crypto::AES_128_CBC);
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
    EVP_EncryptUpdate( &mContext, &output[0], &outputLength, &input[0], actualInputLength );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    PK11_CipherOp( mContext, &output[0], &outputLength, actualInputLength, &input[0], actualInputLength );
#endif // USE_TLS_NSS

    return static_cast<sal_uInt32>(outputLength);
}

bool sha1(vector<sal_uInt8>& output, vector<sal_uInt8>& input)
{
    output.clear();
    output.resize(RTL_DIGEST_LENGTH_SHA1, 0);

    rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, &input[0], input.size() );
    rtl_digest_get( aDigest, &output[0], RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    return true;
}

bool sha512(vector<sal_uInt8>& output, vector<sal_uInt8>& input)
{
    bool aResult = false;

#if USE_TLS_OPENSSL
    output.clear();
    output.resize(SHA512_DIGEST_LENGTH, 0);

    SHA512_CTX context;
    SHA512_Init(&context);
    SHA512_Update(&context, &input[0], input.size());
    SHA512_Final(&output[0], &context);
    aResult = true;
#endif

#if USE_TLS_NSS
    output.clear();
    output.resize(SHA512_LENGTH, 0);

    // Initialize NSS, database functions are not needed
    NSS_NoDB_Init(NULL);
    SECStatus status;

    PK11Context* mContext = PK11_CreateDigestContext(SEC_OID_SHA512);
    status = PK11_DigestBegin(mContext);
    if (status != SECSuccess)
        return false;

    status = PK11_DigestOp(mContext, &input[0], input.size());
    if (status != SECSuccess)
        return false;

    sal_uInt32 outputLength = 0;

    status = PK11_DigestFinal(mContext, &output[0], &outputLength, SHA512_LENGTH);
    if (status != SECSuccess || outputLength != SHA512_LENGTH)
        return false;

    PK11_DestroyContext(mContext, PR_TRUE);

    aResult = true;
#endif
    return aResult;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
