/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef DOCUMENTCRYPTO_HXX
#define DOCUMENTCRYPTO_HXX

#include "oox/dllapi.h"

#include "oox/ole/olestorage.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>


namespace oox {
namespace core {

const sal_uInt32 ENCRYPTINFO_CRYPTOAPI      = 0x00000004;
const sal_uInt32 ENCRYPTINFO_DOCPROPS       = 0x00000008;
const sal_uInt32 ENCRYPTINFO_EXTERNAL       = 0x00000010;
const sal_uInt32 ENCRYPTINFO_AES            = 0x00000020;

const sal_uInt32 ENCRYPT_ALGO_AES128        = 0x0000660E;
const sal_uInt32 ENCRYPT_ALGO_AES192        = 0x0000660F;
const sal_uInt32 ENCRYPT_ALGO_AES256        = 0x00006610;
const sal_uInt32 ENCRYPT_ALGO_RC4           = 0x00006801;

const sal_uInt32 ENCRYPT_HASH_SHA1          = 0x00008004;

const sal_uInt32 ENCRYPT_KEY_SIZE_AES_128   = 0x00000080;
const sal_uInt32 ENCRYPT_KEY_SIZE_AES_192   = 0x000000C0;
const sal_uInt32 ENCRYPT_KEY_SIZE_AES_256   = 0x00000100;

const sal_uInt32 ENCRYPT_PROVIDER_TYPE_AES  = 0x00000018;
const sal_uInt32 ENCRYPT_PROVIDER_TYPE_RC4  = 0x00000001;

// version of encryption info used in MS Office 2007 (major = 3, minor = 2)
const sal_uInt32 VERSION_INFO_2007_FORMAT   = 0x00030002;

const sal_Int32 SALT_LENGTH                    = 16;
const sal_Int32 ENCRYPTED_VERIFIER_LENGTH      = 16;
const sal_Int32 ENCRYPTED_VERIFIER_HASH_LENGTH = 32;

struct EncryptionStandardHeader
{
    sal_uInt32 flags;
    sal_uInt32 sizeExtra;       // 0
    sal_uInt32 algId;           // if flag AES && CRYPTOAPI this defaults to 128-bit AES
    sal_uInt32 algIdHash;       // 0: determine by flags - defaults to SHA-1 if not external
    sal_uInt32 keySize;         // key size in bits: 0 (determine by flags), 128, 192, 256
    sal_uInt32 providedType;    // AES or RC4
    sal_uInt32 reserved1;       // 0
    sal_uInt32 reserved2;       // 0

    EncryptionStandardHeader();
};


struct EncryptionVerifierAES
{
    sal_uInt32 saltSize;                                                // must be 0x00000010
    sal_uInt8  salt[SALT_LENGTH];                                       // random generated salt value
    sal_uInt8  encryptedVerifier[ENCRYPTED_VERIFIER_LENGTH];            // randomly generated verifier value
    sal_uInt32 encryptedVerifierHashSize;                               // actually written hash size - depends on algorithm
    sal_uInt8  encryptedVerifierHash[ENCRYPTED_VERIFIER_HASH_LENGTH];   // verifier value hash - itself also encrypted

    EncryptionVerifierAES();
};

struct PackageEncryptionInfo
{
    EncryptionStandardHeader header;
    EncryptionVerifierAES    verifier;
};

class OOX_DLLPUBLIC AesEncoder
{
private:
    com::sun::star::uno::Reference< com::sun::star::io::XStream > mxDocumentStream;
    oox::ole::OleStorage& mrOleStorage;
    OUString maPassword;

    PackageEncryptionInfo mEncryptionInfo;

    bool checkEncryptionInfo(std::vector<sal_uInt8>& aKey, sal_uInt32 aKeyLength);
    bool writeEncryptionInfo( BinaryOutputStream& rStream );

public:
    AesEncoder(
        com::sun::star::uno::Reference< com::sun::star::io::XStream > xDocumentStream,
        oox::ole::OleStorage& rOleStorage,
        OUString aPassword);

    bool encode();

};

class OOX_DLLPUBLIC AesDecoder
{
private:
    oox::ole::OleStorage&   mrOleStorage;
    PackageEncryptionInfo   mEncryptionInfo;
    std::vector<sal_uInt8>  mKey;
    sal_uInt32              mKeyLength;

    bool readEncryptionInfoFromStream( BinaryInputStream& rStream );

public:
    AesDecoder(oox::ole::OleStorage& rOleStorage);

    bool decode(com::sun::star::uno::Reference< com::sun::star::io::XStream > xDocumentStream);
    bool readEncryptionInfo();
    bool generateEncryptionKey(const OUString& rPassword);

    com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > createEncryptionData();

    bool checkCurrentEncryptionData();

    static bool checkEncryptionData( const com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue >& rEncryptionData );
};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
