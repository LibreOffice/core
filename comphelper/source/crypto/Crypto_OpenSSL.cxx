/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/crypto/Crypto.hxx>
#include <sal/types.h>
#include <config_oox.h>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>

namespace comphelper
{
namespace
{
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)

static HMAC_CTX* HMAC_CTX_new(void)
{
    HMAC_CTX* pContext = new HMAC_CTX;
    HMAC_CTX_init(pContext);
    return pContext;
}

static void HMAC_CTX_free(HMAC_CTX* pContext)
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
        void
        operator()(HMAC_CTX* p)
    {
        HMAC_CTX_free(p);
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP
};
}

class CryptoImplementationOpenSSL : public ICryptoImplementation
{
    std::unique_ptr<EVP_CIPHER_CTX, cipher_delete> mpContext;
    std::unique_ptr<HMAC_CTX, hmac_delete> mpHmacContext;

public:
    CryptoImplementationOpenSSL() = default;

    virtual ~CryptoImplementationOpenSSL()
    {
        if (mpContext)
            EVP_CIPHER_CTX_cleanup(mpContext.get());
    }

    void setupEncryptContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv,
                             CryptoType eType) override
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

    void setupDecryptContext(std::vector<sal_uInt8>& key, std::vector<sal_uInt8>& iv,
                             CryptoType eType) override
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

    void setupCryptoHashContext(std::vector<sal_uInt8>& rKey, CryptoHashType eType) override
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH // 'HMAC_CTX_new' is deprecated
            mpHmacContext.reset(HMAC_CTX_new());
        SAL_WNODEPRECATED_DECLARATIONS_POP
        const EVP_MD* aEvpMd = nullptr;
        switch (eType)
        {
            case CryptoHashType::SHA1:
                aEvpMd = EVP_sha1();
                break;
            case CryptoHashType::SHA256:
                aEvpMd = EVP_sha256();
                break;
            case CryptoHashType::SHA384:
                aEvpMd = EVP_sha384();
                break;
            case CryptoHashType::SHA512:
                aEvpMd = EVP_sha512();
                break;
        }
        SAL_WNODEPRECATED_DECLARATIONS_PUSH // 'HMAC_Init_ex' is deprecated
            HMAC_Init_ex(mpHmacContext.get(), rKey.data(), rKey.size(), aEvpMd, nullptr);
        SAL_WNODEPRECATED_DECLARATIONS_POP
    }

    static const EVP_CIPHER* getCipher(CryptoType type)
    {
        switch (type)
        {
            case CryptoType::AES_128_ECB:
                return EVP_aes_128_ecb();
            case CryptoType::AES_192_ECB:
                return EVP_aes_192_ecb();
            case CryptoType::AES_256_ECB:
                return EVP_aes_256_ecb();
            case CryptoType::AES_128_CBC:
                return EVP_aes_128_cbc();
            case CryptoType::AES_192_CBC:
                return EVP_aes_192_cbc();
            case CryptoType::AES_256_CBC:
                return EVP_aes_256_cbc();
            default:
                break;
        }
        return nullptr;
    }

    sal_uInt32 decryptUpdate(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                             sal_uInt32 inputLength) override
    {
        if (!mpContext)
            return 0;
        int outputLength = 0;
        (void)EVP_DecryptUpdate(mpContext.get(), output.data(), &outputLength, input.data(),
                                inputLength);
        return outputLength;
    }

    sal_uInt32 encryptUpdate(std::vector<sal_uInt8>& output, std::vector<sal_uInt8>& input,
                             sal_uInt32 inputLength) override
    {
        if (!mpContext)
            return 0;
        int outputLength = 0;
        (void)EVP_EncryptUpdate(mpContext.get(), output.data(), &outputLength, input.data(),
                                inputLength);
        return sal_uInt32(outputLength);
    }

    bool cryptoHashUpdate(std::vector<sal_uInt8>& rInput, sal_uInt32 nInputLength) override
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH // 'HMAC_Update' is deprecated
            return HMAC_Update(mpHmacContext.get(), rInput.data(), nInputLength)
            != 0;
        SAL_WNODEPRECATED_DECLARATIONS_POP
    }

    bool cryptoHashFinalize(std::vector<sal_uInt8>& rHash) override
    {
        unsigned int nSizeWritten = 0;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH // 'HMAC_Final' is deprecated
            (void) HMAC_Final(mpHmacContext.get(), rHash.data(), &nSizeWritten);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        return nSizeWritten == rHash.size();
    }
};

} // anonymous namespace

std::shared_ptr<ICryptoImplementation> ICryptoImplementation::createInstance()
{
    return std::shared_ptr<ICryptoImplementation>(new CryptoImplementationOpenSSL);
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
