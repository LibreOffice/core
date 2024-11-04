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
#include <string_view>
#include <vcl/pdfwriter.hxx>

namespace vcl::pdf
{
class EncryptionHashTransporter;

constexpr sal_Int32 ENCRYPTED_PWD_SIZE = 32;

// the maximum password length
constexpr sal_Int32 MD5_DIGEST_SIZE = 16;

// security 128 bit
constexpr sal_Int32 SECUR_128BIT_KEY = 16;

// maximum length of MD5 digest input, in step 2 of algorithm 3.1
// PDF spec ver. 1.4: see there for details
constexpr sal_Int32 MAXIMUM_RC4_KEY_LENGTH = SECUR_128BIT_KEY + 3 + 2;

void padPassword(std::u16string_view i_rPassword, sal_uInt8* o_pPaddedPW);

/* algorithm 3.2: compute an encryption key */
bool computeEncryptionKey(vcl::pdf::EncryptionHashTransporter*,
                          vcl::PDFWriter::PDFEncryptionProperties& io_rProperties,
                          sal_Int32 i_nAccessPermissions);

/* algorithm 3.3: computing the encryption dictionary'ss owner password value ( /O ) */
bool computeODictionaryValue(const sal_uInt8* i_pPaddedOwnerPassword,
                             const sal_uInt8* i_pPaddedUserPassword,
                             std::vector<sal_uInt8>& io_rOValue, sal_Int32 i_nKeyLength);

/* algorithm 3.4 or 3.5: computing the encryption dictionary's user password value ( /U ) revision 2 or 3 of the standard security handler */
bool computeUDictionaryValue(vcl::pdf::EncryptionHashTransporter* i_pTransporter,
                             vcl::PDFWriter::PDFEncryptionProperties& io_rProperties,
                             sal_Int32 i_nKeyLength, sal_Int32 i_nAccessPermissions);

void computeDocumentIdentifier(std::vector<sal_uInt8>& o_rIdentifier,
                               const vcl::PDFWriter::PDFDocInfo& i_rDocInfo,
                               const OString& i_rCString1,
                               const css::util::DateTime& rCreationMetaDate, OString& o_rCString2);

sal_Int32 computeAccessPermissions(const vcl::PDFWriter::PDFEncryptionProperties& i_rProperties,
                                   sal_Int32& o_rKeyLength, sal_Int32& o_rRC4KeyLength);

class PDFEncryptor
{
public:
    /* used to cipher the stream data and for password management */
    rtlCipher m_aCipher = nullptr;

    /* pad string used for password in Standard security handler */
    static const sal_uInt8 s_nPadString[ENCRYPTED_PWD_SIZE];

    /* set to true if the following stream must be encrypted, used inside writeBuffer() */
    bool m_bEncryptThisStream = false;

    /* The encryption key, formed with the user password according to algorithm 3.2,
     * maximum length is 16 bytes + 3 + 2 for 128 bit security */
    sal_Int32 m_nKeyLength = 0; // key length, 16 or 5
    sal_Int32 m_nRC4KeyLength = 0; // key length, 16 or 10, to be input to the algorithm 3.1

    PDFEncryptor()
    {
        /* prepare the cypher engine */
        m_aCipher = rtl_cipher_createARCFOUR(rtl_Cipher_ModeStream);
    }

    ~PDFEncryptor() { rtl_cipher_destroyARCFOUR(m_aCipher); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
