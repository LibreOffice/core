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

#include <string_view>
#include <rtl/cipher.h>
#include <com/sun/star/uno/Reference.hxx>

namespace vcl
{
struct PDFEncryptionProperties;
}
namespace com::sun::star::beans
{
class XMaterialHolder;
}

namespace vcl::pdf
{
class EncryptionHashTransporter;

class PDFEncryptor
{
private:
    /* the numerical value of the access permissions, according to PDF spec, must be signed */
    sal_Int32 m_nAccessPermissions = 0;

    /* The encryption key, formed with the user password according to algorithm 3.2,
     * maximum length is 16 bytes + 3 + 2 for 128 bit security */

    sal_Int32 m_nKeyLength = 0; // key length, 16 or 5
    sal_Int32 m_nRC4KeyLength = 0; // key length, 16 or 10, to be input to the algorithm 3.1
public:
    PDFEncryptor();
    ~PDFEncryptor();

    /* used to cipher the stream data and for password management */
    rtlCipher m_aCipher = nullptr;

    /* set to true if the following stream must be encrypted, used inside writeBuffer() */
    bool m_bEncryptThisStream = false;

    sal_Int32 getAccessPermissions() { return m_nAccessPermissions; }
    sal_Int32 getKeyLength() { return m_nKeyLength; }
    sal_Int32 getRC4KeyLength() { return m_nRC4KeyLength; }

    static css::uno::Reference<css::beans::XMaterialHolder>
    initEncryption(const OUString& i_rOwnerPassword, const OUString& i_rUserPassword);
    virtual bool prepareEncryption(
        const css::uno::Reference<css::beans::XMaterialHolder>& xEncryptionMaterialHolder,
        PDFEncryptionProperties& rProperties);
    void setupKeysAndCheck(PDFEncryptionProperties& rProperties);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
