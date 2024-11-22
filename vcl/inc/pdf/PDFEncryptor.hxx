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

#include <rtl/cipher.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <pdf/IPDFEncryptor.hxx>

namespace com::sun::star::uno
{
template <typename> class Reference;
}

namespace vcl::pdf
{
class EncryptionHashTransporter;

class PDFEncryptor : public IPDFEncryptor
{
private:
    /* the numerical value of the access permissions, according to PDF spec, must be signed */
    sal_Int32 m_nAccessPermissions = 0;

    /* The encryption key, formed with the user password according to algorithm 3.2,
     * maximum length is 16 bytes + 3 + 2 for 128 bit security */

    sal_Int32 m_nKeyLength = 0; // key length, 16 or 5
    sal_Int32 m_nRC4KeyLength = 0; // key length, 16 or 10, to be input to the algorithm 3.1

    /* used to cipher the stream data and for password management */
    rtlCipher m_aCipher = nullptr;

public:
    PDFEncryptor();
    ~PDFEncryptor();

    sal_Int32 getVersion() override { return 2; };
    sal_Int32 getRevision() override { return 3; };

    sal_Int32 getAccessPermissions() override { return m_nAccessPermissions; }
    bool isMetadataEncrypted() override { return false; }
    sal_Int32 getKeyLength() override { return m_nKeyLength; }
    sal_Int32 getRC4KeyLength() { return m_nRC4KeyLength; }

    static void initEncryption(EncryptionHashTransporter& rEncryptionHashTransporter,
                               const OUString& i_rOwnerPassword, const OUString& i_rUserPassword);

    bool prepareEncryption(
        const css::uno::Reference<css::beans::XMaterialHolder>& xEncryptionMaterialHolder,
        PDFEncryptionProperties& rProperties) override;

    void setupKeysAndCheck(PDFEncryptionProperties& rProperties) override;

    void setupEncryption(std::vector<sal_uInt8>& rEncryptionKey, sal_Int32 nObject) override;

    void encrypt(const void* pInput, sal_uInt64 nInputSize, std::vector<sal_uInt8>& rOutput,
                 sal_uInt64 nOutputSize) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
