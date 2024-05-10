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
#include <com/sun/star/uno/RuntimeException.hpp>
#include <sal/types.h>

#include <config_oox.h>

#if USE_TLS_OPENSSL
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
#include <nss.h>
#include <nspr.h>
#include <pk11pub.h>
#endif // USE_TLS_NSS

namespace oox::crypto {

#if USE_TLS_OPENSSL

#if (OPENSSL_VERSION_NUMBER < 0x10100000L)

static HMAC_CTX *HMAC_CTX_new(void)
{
   HMAC_CTX *pContext = new HMAC_CTX;
   HMAC_CTX_init(pContext);
   return pContext;
}

static void HMAC_CTX_free(HMAC_CTX *pContext)
{
    HMAC_CTX_cleanup(pContext);
    delete pContext;
}
#endif

namespace
{
    struct cipher_delete
    {
        void operator()(EVP_CIPHER_CTX* p) { EVP_CIPHER_CTX_free(p); }
    };

    struct hmac_delete
    {
SAL_WNODEPRECATED_DECLARATIONS_PUSH // 'HMAC_CTX_free' is deprecated
        void operator()(HMAC_CTX* p) { HMAC_CTX_free(p); }
SAL_WNODEPRECATED_DECLARATIONS_POP
    };
}

struct CryptoImpl
{
    std::unique_ptr<EVP_CIPHER_CTX, cipher_delete> mpContext;
    std::unique_ptr<HMAC_CTX, hmac_delete> mpHmacContext;

    CryptoImpl() = default;

    void setupEncryptContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, Crypto::CryptoType eType)
    {
        mpContext.reset(EVP_CIPHER_CTX_new());
        EVP_CIPHER_CTX_init(mpContext.get());

        const EVP_CIPHER* cipher = getCipher(eType);
        if (cipher == nullptr)
            return;

        if (iv.empty())
            EVP_EncryptInit_ex(mpContext.get(), cipher, nullptr, key.data(), nullptr);
        else
            EVP_EncryptInit_ex(mpContext.get(), cipher, nullptr, key.data(), iv.data());
        EVP_CIPHER_CTX_set_padding(mpContext.get(), 0);
    }

    void setupDecryptContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, Crypto::CryptoType eType)
    {
        mpContext.reset(EVP_CIPHER_CTX_new());
        EVP_CIPHER_CTX_init(mpContext.get());

        const EVP_CIPHER* pCipher = getCipher(eType);
        if (pCipher == nullptr)
            return;

        const size_t nMinKeySize = EVP_CIPHER_key_length(pCipher);
        if (key.size() < nMinKeySize)
            key.resize(nMinKeySize, 0);

        if (iv.empty())
            EVP_DecryptInit_ex(mpContext.get(), pCipher, nullptr, key.data(), nullptr);
        else
        {
            const size_t nMinIVSize = EVP_CIPHER_iv_length(pCipher);
            if (iv.size() < nMinIVSize)
                iv.resize(nMinIVSize, 0);

            EVP_DecryptInit_ex(mpContext.get(), pCipher, nullptr, key.data(), iv.data());
        }
        EVP_CIPHER_CTX_set_padding(mpContext.get(), 0);
    }

    void setupCryptoHashContext(std::vector<sal_uInt8>& rKey, CryptoHashType eType)
    {
SAL_WNODEPRECATED_DECLARATIONS_PUSH // 'HMAC_CTX_new' is deprecated
        mpHmacContext.reset(HMAC_CTX_new());
SAL_WNODEPRECATED_DECLARATIONS_POP
        const EVP_MD* aEvpMd = nullptr;
        switch (eType)
        {
            case CryptoHashType::SHA1:
                aEvpMd = EVP_sha1(); break;
            case CryptoHashType::SHA256:
                aEvpMd = EVP_sha256(); break;
            case CryptoHashType::SHA384:
                aEvpMd = EVP_sha384(); break;
            case CryptoHashType::SHA512:
                aEvpMd = EVP_sha512(); break;
        }
SAL_WNODEPRECATED_DECLARATIONS_PUSH // 'HMAC_Init_ex' is deprecated
        HMAC_Init_ex(mpHmacContext.get(), rKey.data(), rKey.size(), aEvpMd, nullptr);
SAL_WNODEPRECATED_DECLARATIONS_POP
    }

    ~CryptoImpl()
    {
        if (mpContext)
            EVP_CIPHER_CTX_cleanup(mpContext.get());
    }

    static const EVP_CIPHER* getCipher(Crypto::CryptoType type)
    {
        switch(type)
        {
            case Crypto::CryptoType::AES_128_ECB:
                return EVP_aes_128_ecb();
            case Crypto::CryptoType::AES_128_CBC:
                return EVP_aes_128_cbc();
            case Crypto::CryptoType::AES_256_CBC:
                return EVP_aes_256_cbc();
            default:
                break;
        }
        return nullptr;
    }
};

#elif USE_TLS_NSS

#define MAX_WRAPPED_KEY_LEN 128

struct CryptoImpl
{
    PK11SlotInfo* mSlot;
    PK11Context* mContext;
    SECItem*     mSecParam;
    PK11SymKey*  mSymKey;
    PK11Context* mWrapKeyContext;
    PK11SymKey*  mWrapKey;

    CryptoImpl()
        : mSlot(nullptr)
        , mContext(nullptr)
        , mSecParam(nullptr)
        , mSymKey(nullptr)
        , mWrapKeyContext(nullptr)
        , mWrapKey(nullptr)
    {
        // Initialize NSS, database functions are not needed
        if (!NSS_IsInitialized())
        {
            auto const e = NSS_NoDB_Init(nullptr);
            if (e != SECSuccess)
            {
                PRErrorCode error = PR_GetError();
                const char* errorText = PR_ErrorToName(error);
                throw css::uno::RuntimeException("NSS_NoDB_Init failed with " + OUString(errorText, strlen(errorText), RTL_TEXTENCODING_UTF8) + " (" + OUString::number(static_cast<int>(error)) + ")");
            }
        }
    }

    ~CryptoImpl()
    {
        if (mContext)
            PK11_DestroyContext(mContext, PR_TRUE);
        if (mSecParam)
            SECITEM_FreeItem(mSecParam, PR_TRUE);
        if (mSymKey)
            PK11_FreeSymKey(mSymKey);
        if (mWrapKeyContext)
            PK11_DestroyContext(mWrapKeyContext, PR_TRUE);
        if (mWrapKey)
            PK11_FreeSymKey(mWrapKey);
        if (mSlot)
            PK11_FreeSlot(mSlot);
    }

    PK11SymKey* ImportSymKey(CK_MECHANISM_TYPE mechanism, CK_ATTRIBUTE_TYPE operation, SECItem* key)
    {
        mSymKey = PK11_ImportSymKey(mSlot, mechanism, PK11_OriginUnwrap, operation, key, nullptr);
        if (!mSymKey) //rhbz#1614419 maybe failed due to FIPS, use rhbz#1461450 style workaround
        {
            /*
             * Without FIPS it would be possible to just use
             *  mSymKey = PK11_ImportSymKey( mSlot, mechanism, PK11_OriginUnwrap, CKA_ENCRYPT, &keyItem, nullptr );
             * with FIPS NSS Level 2 certification has to be "workarounded" (so it becomes Level 1) by using
             * following method:
             * 1. Generate wrap key
             * 2. Encrypt authkey with wrap key
             * 3. Unwrap encrypted authkey using wrap key
             */

            /*
             * Generate wrapping key
             */
            CK_MECHANISM_TYPE wrap_mechanism = PK11_GetBestWrapMechanism(mSlot);
            int wrap_key_len = PK11_GetBestKeyLength(mSlot, wrap_mechanism);
            mWrapKey = PK11_KeyGen(mSlot, wrap_mechanism, nullptr, wrap_key_len, nullptr);
            if (!mWrapKey)
                throw css::uno::RuntimeException(u"PK11_KeyGen SymKey failure"_ustr, css::uno::Reference<css::uno::XInterface>());

            /*
             * Encrypt authkey with wrapping key
             */

            /*
             * Initialization of IV is not needed because PK11_GetBestWrapMechanism should return ECB mode
             */
            SECItem tmp_sec_item = {};
            mWrapKeyContext = PK11_CreateContextBySymKey(wrap_mechanism, CKA_ENCRYPT, mWrapKey, &tmp_sec_item);
            if (!mWrapKeyContext)
                throw css::uno::RuntimeException(u"PK11_CreateContextBySymKey failure"_ustr, css::uno::Reference<css::uno::XInterface>());

            unsigned char wrapped_key_data[MAX_WRAPPED_KEY_LEN];
            int wrapped_key_len = sizeof(wrapped_key_data);

            if (PK11_CipherOp(mWrapKeyContext, wrapped_key_data, &wrapped_key_len,
                sizeof(wrapped_key_data), key->data, key->len) != SECSuccess)
            {
                throw css::uno::RuntimeException(u"PK11_CipherOp failure"_ustr, css::uno::Reference<css::uno::XInterface>());
            }

            if (PK11_Finalize(mWrapKeyContext) != SECSuccess)
                throw css::uno::RuntimeException(u"PK11_Finalize failure"_ustr, css::uno::Reference<css::uno::XInterface>());

            /*
             * Finally unwrap sym key
             */
            SECItem wrapped_key = {};
            wrapped_key.data = wrapped_key_data;
            wrapped_key.len = wrapped_key_len;

            mSymKey = PK11_UnwrapSymKey(mWrapKey, wrap_mechanism, &tmp_sec_item, &wrapped_key,
                mechanism, operation, key->len);
        }
        return mSymKey;
    }

    void setupCryptoContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, Crypto::CryptoType type, CK_ATTRIBUTE_TYPE operation)
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
            case Crypto::CryptoType::AES_128_ECB:
                mechanism = CKM_AES_ECB;
                break;
            case Crypto::CryptoType::AES_128_CBC:
                mechanism = CKM_AES_CBC;
                pIvItem = &ivItem;
                break;
            case Crypto::CryptoType::AES_256_CBC:
                mechanism = CKM_AES_CBC;
                pIvItem = &ivItem;
                break;
            default:
                break;
        }

        mSlot = PK11_GetBestSlot(mechanism, nullptr);

        if (!mSlot)
            throw css::uno::RuntimeException(u"NSS Slot failure"_ustr, css::uno::Reference<css::uno::XInterface>());

        SECItem keyItem;
        keyItem.type = siBuffer;
        keyItem.data = key.data();
        keyItem.len  = key.size();

        mSymKey = ImportSymKey(mechanism, CKA_ENCRYPT, &keyItem);
        if (!mSymKey)
            throw css::uno::RuntimeException(u"NSS SymKey failure"_ustr, css::uno::Reference<css::uno::XInterface>());

        mSecParam = PK11_ParamFromIV(mechanism, pIvItem);
        mContext = PK11_CreateContextBySymKey(mechanism, operation, mSymKey, mSecParam);
    }

    void setupCryptoHashContext(std::vector<sal_uInt8>& rKey, CryptoHashType eType)
    {
        CK_MECHANISM_TYPE aMechanism = static_cast<CK_ULONG>(-1);

        switch(eType)
        {
            case CryptoHashType::SHA1:
                aMechanism = CKM_SHA_1_HMAC;
                break;
            case CryptoHashType::SHA256:
                aMechanism = CKM_SHA256_HMAC;
                break;
            case CryptoHashType::SHA384:
                aMechanism = CKM_SHA384_HMAC;
                break;
            case CryptoHashType::SHA512:
                aMechanism = CKM_SHA512_HMAC;
                break;
        }

        mSlot = PK11_GetBestSlot(aMechanism, nullptr);

        if (!mSlot)
            throw css::uno::RuntimeException(u"NSS Slot failure"_ustr, css::uno::Reference<css::uno::XInterface>());

        SECItem aKeyItem;
        aKeyItem.data = rKey.data();
        aKeyItem.len  = rKey.size();

        mSymKey = ImportSymKey(aMechanism, CKA_SIGN, &aKeyItem);
        if (!mSymKey)
            throw css::uno::RuntimeException(u"NSS SymKey failure"_ustr, css::uno::Reference<css::uno::XInterface>());

        SECItem param;
        param.data = nullptr;
        param.len = 0;
        mContext = PK11_CreateContextBySymKey(aMechanism, CKA_SIGN, mSymKey, &param);
    }
};
#else
struct CryptoImpl
{};
#endif

Crypto::Crypto()
    : mpImpl(std::make_unique<CryptoImpl>())
{
}

Crypto::~Crypto()
{
}

// DECRYPT

Decrypt::Decrypt(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv, CryptoType type)
{
#if USE_TLS_OPENSSL + USE_TLS_NSS == 0
    (void)key;
    (void)iv;
    (void)type;
#endif

#if USE_TLS_OPENSSL
    mpImpl->setupDecryptContext(key, iv, type);
#endif

#if USE_TLS_NSS
    mpImpl->setupCryptoContext(key, iv, type, CKA_DECRYPT);
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
    (void)EVP_DecryptUpdate(mpImpl->mpContext.get(), output.data(), &outputLength, input.data(), actualInputLength);
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    if (!mpImpl->mContext)
        return 0;
    (void)PK11_CipherOp(mpImpl->mContext, output.data(), &outputLength, actualInputLength, input.data(), actualInputLength);
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
{
#if USE_TLS_OPENSSL + USE_TLS_NSS == 0
    (void)key;
    (void)iv;
    (void)type;
#endif

#if USE_TLS_OPENSSL
    mpImpl->setupEncryptContext(key, iv, type);
#elif USE_TLS_NSS
    mpImpl->setupCryptoContext(key, iv, type, CKA_ENCRYPT);
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
    (void)EVP_EncryptUpdate(mpImpl->mpContext.get(), output.data(), &outputLength, input.data(), actualInputLength);
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
    (void)PK11_CipherOp(mpImpl->mContext, output.data(), &outputLength, actualInputLength, input.data(), actualInputLength);
#endif // USE_TLS_NSS

    return static_cast<sal_uInt32>(outputLength);
}

// CryptoHash - HMAC

namespace
{

sal_Int32 getSizeForHashType(CryptoHashType eType)
{
    switch (eType)
    {
        case CryptoHashType::SHA1: return 20;
        case CryptoHashType::SHA256: return 32;
        case CryptoHashType::SHA384: return 48;
        case CryptoHashType::SHA512: return 64;
    }
    return 0;
}

} // end anonymous namespace

CryptoHash::CryptoHash(std::vector<sal_uInt8>& rKey, CryptoHashType eType)
    : mnHashSize(getSizeForHashType(eType))
{
#if USE_TLS_OPENSSL
    mpImpl->setupCryptoHashContext(rKey, eType);
#elif USE_TLS_NSS
    mpImpl->setupCryptoHashContext(rKey, eType);
    PK11_DigestBegin(mpImpl->mContext);
#else
    (void)rKey;
#endif
}

bool CryptoHash::update(std::vector<sal_uInt8>& rInput, sal_uInt32 nInputLength)
{
#if USE_TLS_OPENSSL + USE_TLS_NSS > 0
    sal_uInt32 nActualInputLength = (nInputLength == 0 || nInputLength > rInput.size()) ? rInput.size() : nInputLength;
#else
    (void)rInput;
    (void)nInputLength;
#endif

#if USE_TLS_OPENSSL
SAL_WNODEPRECATED_DECLARATIONS_PUSH // 'HMAC_Update' is deprecated
    return HMAC_Update(mpImpl->mpHmacContext.get(), rInput.data(), nActualInputLength) != 0;
SAL_WNODEPRECATED_DECLARATIONS_POP
#elif USE_TLS_NSS
    return PK11_DigestOp(mpImpl->mContext, rInput.data(), nActualInputLength) == SECSuccess;
#else
    return false; // ???
#endif
}

std::vector<sal_uInt8> CryptoHash::finalize()
{
    std::vector<sal_uInt8> aHash(mnHashSize, 0);
    unsigned int nSizeWritten;
#if USE_TLS_OPENSSL
SAL_WNODEPRECATED_DECLARATIONS_PUSH // 'HMAC_Final' is deprecated
    (void) HMAC_Final(mpImpl->mpHmacContext.get(), aHash.data(), &nSizeWritten);
SAL_WNODEPRECATED_DECLARATIONS_POP
#elif USE_TLS_NSS
    PK11_DigestFinal(mpImpl->mContext, aHash.data(), &nSizeWritten, aHash.size());
#endif
    (void)nSizeWritten;

    return aHash;
}

} // namespace oox::crypto

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
