/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <oox/crypto/CryptTools.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

namespace oox {
namespace core {

Crypto::Crypto()
#if USE_TLS_NSS
    : mContext(nullptr)
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
    if (mContext)
        PK11_DestroyContext(mContext, PR_TRUE);
    if (mSymKey)
        PK11_FreeSymKey(mSymKey);
    if (mSecParam)
        SECITEM_FreeItem(mSecParam, PR_TRUE);
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
void Crypto::setupContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, CryptoType type, CK_ATTRIBUTE_TYPE operation)
{
    CK_MECHANISM_TYPE mechanism = static_cast<CK_ULONG>(-1);

    SECItem ivItem;
    ivItem.type = siBuffer;
    if(iv.empty())
        ivItem.data = nullptr;
    else
        ivItem.data = iv.data();
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

    PK11SlotInfo* pSlot(PK11_GetBestSlot(mechanism, nullptr));

    if (!pSlot)
        throw css::uno::RuntimeException("NSS Slot failure", css::uno::Reference<css::uno::XInterface>());

    SECItem keyItem;
    keyItem.type = siBuffer;
    keyItem.data = key.data();
    keyItem.len  = key.size();

    mSymKey = PK11_ImportSymKey(pSlot, mechanism, PK11_OriginUnwrap, CKA_ENCRYPT, &keyItem, nullptr);
    if (!mSymKey)
        throw css::uno::RuntimeException("NSS SymKey failure", css::uno::Reference<css::uno::XInterface>());

    mSecParam = PK11_ParamFromIV(mechanism, pIvItem);
    mContext = PK11_CreateContextBySymKey(mechanism, operation, mSymKey, mSecParam);
}
#endif // USE_TLS_NSS

// DECRYPT

Decrypt::Decrypt(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, CryptoType type)
    : Crypto()
{
#if USE_TLS_OPENSSL + USE_TLS_NSS == 0
    (void)key;
    (void)iv;
    (void)type;
#endif

#if USE_TLS_OPENSSL
    EVP_CIPHER_CTX_init(&mContext);

    const EVP_CIPHER* cipher = getCipher(type);

    const size_t nMinKeySize = EVP_CIPHER_key_length(cipher);
    if (key.size() < nMinKeySize)
        key.resize(nMinKeySize, 0);

    if (iv.empty())
        EVP_DecryptInit_ex(&mContext, cipher, nullptr, key.data(), 0);
    else
    {
        const size_t nMinIVSize = EVP_CIPHER_iv_length(cipher);
        if (iv.size() < nMinIVSize)
            iv.resize(nMinIVSize, 0);

        EVP_DecryptInit_ex(&mContext, cipher, nullptr, key.data(), iv.data());
    }
    EVP_CIPHER_CTX_set_padding(&mContext, 0);
#endif

#if USE_TLS_NSS
    setupContext(key, iv, type, CKA_DECRYPT);
#endif // USE_TLS_NSS
}

sal_uInt32 Decrypt::update(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input, sal_uInt32 inputLength)
{
    int outputLength = 0;

#if USE_TLS_OPENSSL + USE_TLS_NSS > 0
    sal_uInt32 actualInputLength = inputLength == 0 || inputLength > input.size() ? input.size() : inputLength;
#else
    (void)output;
    (void)input;
    (void)inputLength;
#endif

#if USE_TLS_OPENSSL
    (void)EVP_DecryptUpdate(&mContext, output.data(), &outputLength, input.data(), actualInputLength);
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    (void)PK11_CipherOp( mContext, output.data(), &outputLength, actualInputLength, input.data(), actualInputLength );
#endif // USE_TLS_NSS

    return static_cast<sal_uInt32>(outputLength);
}

sal_uInt32 Decrypt::aes128ecb(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input, std::vector<sal_uInt8>& key)
{
    sal_uInt32 outputLength = 0;
    std::vector<sal_uInt8> iv;
    Decrypt crypto(key, iv, Crypto::AES_128_ECB);
    outputLength = crypto.update(output, input);
    return outputLength;
}

// ENCRYPT

Encrypt::Encrypt(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, CryptoType type)
    : Crypto()
{
#if USE_TLS_OPENSSL + USE_TLS_NSS == 0
    (void)key;
    (void)iv;
    (void)type;
#endif

#if USE_TLS_OPENSSL
    EVP_CIPHER_CTX_init(&mContext);

    const EVP_CIPHER* cipher = getCipher(type);

    if (iv.empty())
        EVP_EncryptInit_ex(&mContext, cipher, nullptr, key.data(), 0);
    else
        EVP_EncryptInit_ex(&mContext, cipher, nullptr, key.data(), iv.data());
    EVP_CIPHER_CTX_set_padding(&mContext, 0);
#endif

#if USE_TLS_NSS
    setupContext(key, iv, type, CKA_ENCRYPT);
#endif // USE_TLS_NSS
}

sal_uInt32 Encrypt::update(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input, sal_uInt32 inputLength)
{
    int outputLength = 0;

#if USE_TLS_OPENSSL + USE_TLS_NSS > 0
    sal_uInt32 actualInputLength = inputLength == 0 || inputLength > input.size() ? input.size() : inputLength;
#else
    (void)output;
    (void)input;
    (void)inputLength;
#endif

#if USE_TLS_OPENSSL
    (void)EVP_EncryptUpdate(&mContext, output.data(), &outputLength, input.data(), actualInputLength);
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    (void)PK11_CipherOp(mContext, output.data(), &outputLength, actualInputLength, input.data(), actualInputLength);
#endif // USE_TLS_NSS

    return static_cast<sal_uInt32>(outputLength);
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
