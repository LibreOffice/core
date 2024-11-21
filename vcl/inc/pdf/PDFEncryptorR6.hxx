/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <string_view>
#include <vector>
#include <vcl/dllapi.h>
#include <pdf/IPDFEncryptor.hxx>

namespace vcl::pdf
{
class EncryptionHashTransporter;

/** Algorithm 2.B: Computing a hash (revision 6 and later)
 *
 * Described in ISO 32000-2:2020(E) - 7.6.4.3.4
 */
VCL_DLLPUBLIC std::vector<sal_uInt8>
computeHashR6(const sal_uInt8* pPassword, size_t nPasswordLength,
              std::vector<sal_uInt8> const& rValidationSalt,
              std::vector<sal_uInt8> const& rUserKey = std::vector<sal_uInt8>());

/** Algorithm 11: Authenticating the user password (Security handlers of revision 6)
 *
 * Described in ISO 32000-2:2020(E) - 7.6.4.4.10
 */
VCL_DLLPUBLIC bool validateUserPassword(const sal_uInt8* pUserPass, size_t nPasswordLength,
                                        std::vector<sal_uInt8>& U);

/** Algorithm 12: Authenticating the owner password (Security handlers of revision 6)
 *
 * Described in ISO 32000-2:2020(E) - 7.6.4.4.11
 */
VCL_DLLPUBLIC bool validateOwnerPassword(const sal_uInt8* pUserPass, size_t nPasswordLength,
                                         std::vector<sal_uInt8>& U, std::vector<sal_uInt8>& O);

/** Generates the encryption key - random data 32-byte */
VCL_DLLPUBLIC std::vector<sal_uInt8> generateKey();

/** Algorithm 8: U and UE
 *
 * Computing the encryption dictionary’s U (user password) and UE (user encryption) values
 * (Security handlers of revision 6)
 *
 * Described in ISO 32000-2:2020(E) - 7.6.4.4.7
 */
VCL_DLLPUBLIC void generateUandUE(const sal_uInt8* pUserPass, size_t nPasswordLength,
                                  std::vector<sal_uInt8>& rFileEncryptionKey,
                                  std::vector<sal_uInt8>& U, std::vector<sal_uInt8>& UE);

/** Algorithm 9: O and OE
 *
 * Computing the encryption dictionary’s O (owner password) and OE (owner encryption) values
 * (Security handlers of revision 6)
 *
 * Described in ISO 32000-2:2020(E) - 7.6.4.4.8
 */
VCL_DLLPUBLIC void generateOandOE(const sal_uInt8* pUserPass, size_t nPasswordLength,
                                  std::vector<sal_uInt8>& rFileEncryptionKey,
                                  std::vector<sal_uInt8>& U, std::vector<sal_uInt8>& O,
                                  std::vector<sal_uInt8>& OE);

/** Algorithm 8 step b) in reverse
 *
 * Described in ISO 32000-2:2020(E) - 7.6.4.4.7
 *
 * - compute the hash with password and user key salt
 * - decrypt with hash as key and zero IV
 */
VCL_DLLPUBLIC std::vector<sal_uInt8> decryptKey(const sal_uInt8* pUserPass, size_t nPasswordLength,
                                                std::vector<sal_uInt8>& U,
                                                std::vector<sal_uInt8>& UE);

/** Algorithm 13: Validating the permissions (Security handlers of revision 6)
 *
 * Described in ISO 32000-2:2020(E) - 7.6.4.4.12
 */
VCL_DLLPUBLIC std::vector<sal_uInt8> decryptPerms(std::vector<sal_uInt8>& rPermsEncrypted,
                                                  std::vector<sal_uInt8>& rFileEncryptionKey);

/** Algorithm 10 step f)
 *
 * Computing the encryption dictionary’s Perms (permissions) value (Security handlers of revision 6)
 *
 * Described in ISO 32000-2:2020(E) - 7.6.4.4.9
 */
VCL_DLLPUBLIC std::vector<sal_uInt8> encryptPerms(std::vector<sal_uInt8>& rPerms,
                                                  std::vector<sal_uInt8>& rFileEncryptionKey);

/** Algorithm 10 steps a) - e)
 *
 * Computing the encryption dictionary’s Perms (permissions) value (Security handlers of revision 6)
 *
 * Described in ISO 32000-2:2020(E) - 7.6.4.4.9
 */
VCL_DLLPUBLIC std::vector<sal_uInt8> createPerms(sal_Int32 nAccessPermissions,
                                                 bool bEncryptMetadata);

/** Padding as described in Internet RFC 8018
 *
 * Described in ISO 32000-2:2020(E) - 7.6.3.1
 */
VCL_DLLPUBLIC size_t addPaddingToVector(std::vector<sal_uInt8>& rVector, size_t nBlockSize);

class EncryptionContext;

/** IPDFEncryptor implementation of PDF encryption version 5 revision 6 added in PDF 2.0
 *
 * The complete algorithm is defined in PDF 2.0 specification ISO 32000-2:2020(E)
 */
class VCL_DLLPUBLIC PDFEncryptorR6 : public IPDFEncryptor
{
    std::unique_ptr<EncryptionContext> m_pEncryptionContext;
    sal_Int32 m_nAccessPermissions = 0;

public:
    PDFEncryptorR6();
    ~PDFEncryptorR6();

    sal_Int32 getVersion() override { return 5; }
    sal_Int32 getRevision() override { return 6; }
    sal_Int32 getAccessPermissions() override { return m_nAccessPermissions; }
    /** Key length - AES 256 bit */
    sal_Int32 getKeyLength() override { return 256 / 8; }

    std::vector<sal_uInt8> getEncryptedAccessPermissions(std::vector<sal_uInt8>& rKey) override;

    static void initEncryption(EncryptionHashTransporter& rEncryptionHashTransporter,
                               const OUString& i_rOwnerPassword, const OUString& i_rUserPassword);

    bool prepareEncryption(
        const css::uno::Reference<css::beans::XMaterialHolder>& xEncryptionMaterialHolder,
        PDFEncryptionProperties& rProperties) override;

    void setupKeysAndCheck(PDFEncryptionProperties& rProperties) override;

    sal_uInt64 calculateSizeIncludingHeader(sal_uInt64 nSize) override;

    void setupEncryption(std::vector<sal_uInt8>& rEncryptionKey, sal_Int32 nObject) override;

    void setupEncryptionWithIV(std::vector<sal_uInt8>& rInitvector, std::vector<sal_uInt8>& rIV);

    /** Encrypts using Algorithm 1.A: Encryption of data using the AES algorithms
     *
     * Described in ISO 32000-2:2020(E) - 7.6.3.3
     */
    void encrypt(const void* pInput, sal_uInt64 nInputSize, std::vector<sal_uInt8>& rOutput,
                 sal_uInt64 nOutputsSize) override;
};

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
