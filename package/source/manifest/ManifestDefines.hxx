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
#ifndef INCLUDED_PACKAGE_SOURCE_MANIFEST_MANIFESTDEFINES_HXX
#define INCLUDED_PACKAGE_SOURCE_MANIFEST_MANIFESTDEFINES_HXX

#include <PackageConstants.hxx>

#define MANIFEST_NSPREFIX "manifest:"
#define ELEMENT_MANIFEST "manifest:manifest"
#define ATTRIBUTE_XMLNS "xmlns:manifest"
#define ATTRIBUTE_XMLNS_LOEXT "xmlns:loext"
#define MANIFEST_NAMESPACE "http://openoffice.org/2001/manifest"
#define MANIFEST_OASIS_NAMESPACE "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"
#define MANIFEST_LOEXT_NAMESPACE "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"
#define MANIFEST_DOCTYPE "<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">"
#define ATTRIBUTE_CDATA "CDATA"

#define ELEMENT_FILE_ENTRY "manifest:file-entry"
#define ATTRIBUTE_FULL_PATH "manifest:full-path"
#define ATTRIBUTE_VERSION "manifest:version"
#define ATTRIBUTE_MEDIA_TYPE "manifest:media-type"
#define ATTRIBUTE_SIZE "manifest:size"
#define ELEMENT_KEYINFO "loext:KeyInfo"
#define ELEMENT_ENCRYPTEDKEY "loext:EncryptedKey"
#define ELEMENT_ENCRYPTIONMETHOD "loext:EncryptionMethod"
#define ELEMENT_PGPDATA "loext:PGPData"
#define ELEMENT_PGPKEYID "loext:PGPKeyID"
#define ELEMENT_PGPKEYPACKET "loext:PGPKeyPacket"
#define ATTRIBUTE_ALGORITHM "loext:Algorithm"
#define ELEMENT_CIPHERDATA "loext:CipherData"
#define ELEMENT_CIPHERVALUE "loext:CipherValue"

#define ELEMENT_ENCRYPTION_DATA "manifest:encryption-data"
#define ATTRIBUTE_CHECKSUM_TYPE "manifest:checksum-type"
#define ATTRIBUTE_CHECKSUM "manifest:checksum"

#define ELEMENT_ALGORITHM "manifest:algorithm"
#define ATTRIBUTE_ALGORITHM_NAME "manifest:algorithm-name"
#define ATTRIBUTE_INITIALISATION_VECTOR "manifest:initialisation-vector"

#define ELEMENT_START_KEY_GENERATION "manifest:start-key-generation"
#define ATTRIBUTE_START_KEY_GENERATION_NAME "manifest:start-key-generation-name"
#define ATTRIBUTE_KEY_SIZE "manifest:key-size"

#define ELEMENT_KEY_DERIVATION "manifest:key-derivation"
#define ATTRIBUTE_KEY_DERIVATION_NAME "manifest:key-derivation-name"
#define ATTRIBUTE_SALT "manifest:salt"
#define ATTRIBUTE_ITERATION_COUNT "manifest:iteration-count"

/// OFFICE-3708: wrong URL cited in ODF 1.2 and used since OOo 3.4 beta
#define SHA256_URL_ODF12 "http://www.w3.org/2000/09/xmldsig#sha256"
#define SHA256_URL "http://www.w3.org/2001/04/xmlenc#sha256"
#define SHA1_NAME "SHA1"
#define SHA1_URL "http://www.w3.org/2000/09/xmldsig#sha1"

#define SHA1_1K_NAME "SHA1/1K"
#define SHA1_1K_URL "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha1-1k"
#define SHA256_1K_URL "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha256-1k"

#define BLOWFISH_NAME "Blowfish CFB"
#define BLOWFISH_URL "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#blowfish"
#define AES128_URL "http://www.w3.org/2001/04/xmlenc#aes128-cbc"
#define AES192_URL "http://www.w3.org/2001/04/xmlenc#aes192-cbc"
#define AES256_URL "http://www.w3.org/2001/04/xmlenc#aes256-cbc"

#define PBKDF2_NAME "PBKDF2"
#define PBKDF2_URL "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#pbkdf2"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
