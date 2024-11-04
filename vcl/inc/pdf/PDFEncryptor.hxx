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

namespace vcl::pdf
{
constexpr sal_Int32 ENCRYPTED_PWD_SIZE = 32;

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
