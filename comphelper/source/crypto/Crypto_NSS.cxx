/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/crypto/Crypto.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <sal/types.h>
#include <config_oox.h>

#include <nss.h>
#include <nspr.h>
#include <pk11pub.h>

namespace comphelper
{
namespace
{
#define MAX_WRAPPED_KEY_LEN 128

class CryptoImplementationNSS : public ICryptoImplementation
{
    PK11SlotInfo* mSlot;
    PK11Context* mContext;
    SECItem* mSecParam;
    PK11SymKey* mSymKey;
    PK11Context* mWrapKeyContext;
    PK11SymKey* mWrapKey;

public:
    CryptoImplementationNSS()
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
                throw css::uno::RuntimeException(
                    "NSS_NoDB_Init failed with "
                    + OUString(errorText, strlen(errorText), RTL_TEXTENCODING_UTF8) + " ("
                    + OUString::number(static_cast<int>(error)) + ")");
            }
        }
    }

    virtual ~CryptoImplementationNSS()
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
                throw css::uno::RuntimeException(u"PK11_KeyGen SymKey failure"_ustr,
                                                 css::uno::Reference<css::uno::XInterface>());

            /*
             * Encrypt authkey with wrapping key
             */

            /*
             * Initialization of IV is not needed because PK11_GetBestWrapMechanism should return ECB mode
             */
            SECItem tmp_sec_item = {};
            mWrapKeyContext
                = PK11_CreateContextBySymKey(wrap_mechanism, CKA_ENCRYPT, mWrapKey, &tmp_sec_item);
            if (!mWrapKeyContext)
                throw css::uno::RuntimeException(u"PK11_CreateContextBySymKey failure"_ustr,
                                                 css::uno::Reference<css::uno::XInterface>());

            unsigned char wrapped_key_data[MAX_WRAPPED_KEY_LEN];
            int wrapped_key_len = sizeof(wrapped_key_data);

            if (PK11_CipherOp(mWrapKeyContext, wrapped_key_data, &wrapped_key_len,
                              sizeof(wrapped_key_data), key->data, key->len)
                != SECSuccess)
            {
                throw css::uno::RuntimeException(u"PK11_CipherOp failure"_ustr,
                                                 css::uno::Reference<css::uno::XInterface>());
            }

            if (PK11_Finalize(mWrapKeyContext) != SECSuccess)
                throw css::uno::RuntimeException(u"PK11_Finalize failure"_ustr,
                                                 css::uno::Reference<css::uno::XInterface>());

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

    void setupEncryptContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv,
                             CryptoType type) override
    {
        setupCryptoContext(key, iv, type, CKA_ENCRYPT);
    }

    void setupDecryptContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv,
                             CryptoType type) override
    {
        setupCryptoContext(key, iv, type, CKA_DECRYPT);
    }

    void setupCryptoContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv,
                            CryptoType type, CK_ATTRIBUTE_TYPE operation)
    {
        CK_MECHANISM_TYPE mechanism = static_cast<CK_ULONG>(-1);

        SECItem ivItem;
        ivItem.type = siBuffer;
        if (iv.empty())
            ivItem.data = nullptr;
        else
            ivItem.data = iv.data();
        ivItem.len = iv.size();

        SECItem* pIvItem = nullptr;

        switch (type)
        {
            case CryptoType::AES_128_ECB:
            case CryptoType::AES_192_ECB:
            case CryptoType::AES_256_ECB:
                mechanism = CKM_AES_ECB;
                break;
            case CryptoType::AES_128_CBC:
            case CryptoType::AES_192_CBC:
            case CryptoType::AES_256_CBC:
                mechanism = CKM_AES_CBC;
                pIvItem = &ivItem;
                break;
            default:
                break;
        }

        mSlot = PK11_GetBestSlot(mechanism, nullptr);

        if (!mSlot)
            throw css::uno::RuntimeException(u"NSS Slot failure"_ustr,
                                             css::uno::Reference<css::uno::XInterface>());

        SECItem keyItem;
        keyItem.type = siBuffer;
        keyItem.data = key.data();
        keyItem.len = key.size();

        mSymKey = ImportSymKey(mechanism, CKA_ENCRYPT, &keyItem);
        if (!mSymKey)
            throw css::uno::RuntimeException(u"NSS SymKey failure"_ustr,
                                             css::uno::Reference<css::uno::XInterface>());

        mSecParam = PK11_ParamFromIV(mechanism, pIvItem);
        mContext = PK11_CreateContextBySymKey(mechanism, operation, mSymKey, mSecParam);
    }

    void setupCryptoHashContext(std::vector<sal_uInt8>& rKey, CryptoHashType eType) override
    {
        CK_MECHANISM_TYPE aMechanism = static_cast<CK_ULONG>(-1);

        switch (eType)
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
            throw css::uno::RuntimeException(u"NSS Slot failure"_ustr,
                                             css::uno::Reference<css::uno::XInterface>());

        SECItem aKeyItem;
        aKeyItem.data = rKey.data();
        aKeyItem.len = rKey.size();

        mSymKey = ImportSymKey(aMechanism, CKA_SIGN, &aKeyItem);
        if (!mSymKey)
            throw css::uno::RuntimeException(u"NSS SymKey failure"_ustr,
                                             css::uno::Reference<css::uno::XInterface>());

        SECItem param;
        param.data = nullptr;
        param.len = 0;
        mContext = PK11_CreateContextBySymKey(aMechanism, CKA_SIGN, mSymKey, &param);

        // Also call digest to begin
        PK11_DigestBegin(mContext);
    }

    sal_uInt32 decryptUpdate(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                             sal_uInt32 inputLength) override
    {
        if (!mContext)
            return 0;
        int outputLength = 0;
        (void)PK11_CipherOp(mContext, output.data(), &outputLength, inputLength, input.data(),
                            inputLength);
        return outputLength;
    }

    sal_uInt32 encryptUpdate(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                             sal_uInt32 inputLength) override
    {
        if (!mContext)
            return 0;
        int outputLength = 0;
        (void)PK11_CipherOp(mContext, output.data(), &outputLength, inputLength, input.data(),
                            inputLength);
        return outputLength;
    }

    bool cryptoHashUpdate(std::vector<sal_uInt8>& rInput, sal_uInt32 nInputLength) override
    {
        return PK11_DigestOp(mContext, rInput.data(), nInputLength) == SECSuccess;
    }

    bool cryptoHashFinalize(std::vector<sal_uInt8>& rHash) override
    {
        unsigned int nSizeWritten = 0;
        PK11_DigestFinal(mContext, rHash.data(), &nSizeWritten, rHash.size());
        return nSizeWritten == rHash.size();
    }
};

} // anonymous namespace

std::shared_ptr<ICryptoImplementation> ICryptoImplementation::createInstance()
{
    return std::shared_ptr<ICryptoImplementation>(new CryptoImplementationNSS);
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
