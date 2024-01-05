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
#pragma once

#include <rtl/ustring.hxx>

inline constexpr OUString MANIFEST_NSPREFIX = u"manifest:"_ustr;
inline constexpr OUString ELEMENT_MANIFEST = u"manifest:manifest"_ustr;
inline constexpr OUString ATTRIBUTE_XMLNS = u"xmlns:manifest"_ustr;
inline constexpr OUString ATTRIBUTE_XMLNS_LOEXT = u"xmlns:loext"_ustr;
inline constexpr OUString MANIFEST_NAMESPACE = u"http://openoffice.org/2001/manifest"_ustr;
inline constexpr OUString MANIFEST_OASIS_NAMESPACE = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"_ustr;
inline constexpr OUString MANIFEST_LOEXT_NAMESPACE = u"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"_ustr;
inline constexpr OUString MANIFEST_DOCTYPE = u"<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">"_ustr;

inline constexpr OUString ELEMENT_FILE_ENTRY = u"manifest:file-entry"_ustr;
inline constexpr OUString ATTRIBUTE_FULL_PATH  = u"manifest:full-path"_ustr;
inline constexpr OUString ATTRIBUTE_VERSION  = u"manifest:version"_ustr;
inline constexpr OUString ATTRIBUTE_MEDIA_TYPE = u"manifest:media-type"_ustr;
inline constexpr OUString ATTRIBUTE_SIZE  = u"manifest:size"_ustr;
inline constexpr OUString ELEMENT_MANIFEST_KEYINFO  = u"loext:keyinfo"_ustr;
inline constexpr OUString ELEMENT_ENCRYPTED_KEYINFO = u"loext:KeyInfo"_ustr;
inline constexpr OUString ELEMENT_ENCRYPTEDKEY  = u"loext:encrypted-key"_ustr;
inline constexpr OUString ELEMENT_ENCRYPTIONMETHOD = u"loext:encryption-method"_ustr;
inline constexpr OUString ELEMENT_PGPDATA  = u"loext:PGPData"_ustr;
inline constexpr OUString ELEMENT_PGPKEYID  = u"loext:PGPKeyID"_ustr;
inline constexpr OUString ELEMENT_PGPKEYPACKET = u"loext:PGPKeyPacket"_ustr;
inline constexpr OUString ATTRIBUTE_ALGORITHM = u"loext:PGPAlgorithm"_ustr;
inline constexpr OUString ELEMENT_CIPHERDATA = u"loext:CipherData"_ustr;
inline constexpr OUString ELEMENT_CIPHERVALUE = u"loext:CipherValue"_ustr;
inline constexpr OUString ELEMENT_MANIFEST13_KEYINFO = u"manifest:keyinfo"_ustr;
inline constexpr OUString ELEMENT_ENCRYPTEDKEY13 = u"manifest:encrypted-key"_ustr;
inline constexpr OUString ELEMENT_ENCRYPTIONMETHOD13 = u"manifest:encryption-method"_ustr;
inline constexpr OUString ELEMENT_PGPDATA13 = u"manifest:PGPData"_ustr;
inline constexpr OUString ELEMENT_PGPKEYID13 = u"manifest:PGPKeyID"_ustr;
inline constexpr OUString ELEMENT_PGPKEYPACKET13 = u"manifest:PGPKeyPacket"_ustr;
inline constexpr OUString ATTRIBUTE_ALGORITHM13 = u"manifest:PGPAlgorithm"_ustr;
inline constexpr OUString ELEMENT_CIPHERDATA13 = u"manifest:CipherData"_ustr;
inline constexpr OUString ELEMENT_CIPHERVALUE13 = u"manifest:CipherValue"_ustr;

inline constexpr OUString ELEMENT_ENCRYPTION_DATA = u"manifest:encryption-data"_ustr;
inline constexpr OUString ATTRIBUTE_CHECKSUM_TYPE = u"manifest:checksum-type"_ustr;
inline constexpr OUString ATTRIBUTE_CHECKSUM  = u"manifest:checksum"_ustr;

inline constexpr OUString ELEMENT_ALGORITHM = u"manifest:algorithm"_ustr;
inline constexpr OUString ATTRIBUTE_ALGORITHM_NAME = u"manifest:algorithm-name"_ustr;
inline constexpr OUString ATTRIBUTE_INITIALISATION_VECTOR = u"manifest:initialisation-vector"_ustr;

inline constexpr OUString ELEMENT_START_KEY_GENERATION  = u"manifest:start-key-generation"_ustr;
inline constexpr OUString ATTRIBUTE_START_KEY_GENERATION_NAME  = u"manifest:start-key-generation-name"_ustr;
inline constexpr OUString ATTRIBUTE_KEY_SIZE  = u"manifest:key-size"_ustr;

inline constexpr OUString ELEMENT_KEY_DERIVATION = u"manifest:key-derivation"_ustr;
inline constexpr OUString ATTRIBUTE_KEY_DERIVATION_NAME  = u"manifest:key-derivation-name"_ustr;
inline constexpr OUString ATTRIBUTE_SALT  = u"manifest:salt"_ustr;
inline constexpr OUString ATTRIBUTE_ITERATION_COUNT = u"manifest:iteration-count"_ustr;
inline constexpr OUString ATTRIBUTE_ARGON2_T = u"manifest:argon2-iterations"_ustr;
inline constexpr OUString ATTRIBUTE_ARGON2_M = u"manifest:argon2-memory"_ustr;
inline constexpr OUString ATTRIBUTE_ARGON2_P = u"manifest:argon2-lanes"_ustr;
inline constexpr OUString ATTRIBUTE_ARGON2_T_LO = u"loext:argon2-iterations"_ustr;
inline constexpr OUString ATTRIBUTE_ARGON2_M_LO = u"loext:argon2-memory"_ustr;
inline constexpr OUString ATTRIBUTE_ARGON2_P_LO = u"loext:argon2-lanes"_ustr;

/// OFFICE-3708: wrong URL cited in ODF 1.2 and used since OOo 3.4 beta
inline constexpr OUString SHA256_URL_ODF12 = u"http://www.w3.org/2000/09/xmldsig#sha256"_ustr;
inline constexpr OUString SHA256_URL = u"http://www.w3.org/2001/04/xmlenc#sha256"_ustr;
inline constexpr OUString SHA1_NAME = u"SHA1"_ustr;
inline constexpr OUString SHA1_URL = u"http://www.w3.org/2000/09/xmldsig#sha1"_ustr;

inline constexpr OUString SHA1_1K_NAME = u"SHA1/1K"_ustr;
inline constexpr OUString SHA1_1K_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha1-1k"_ustr;
inline constexpr OUString SHA256_1K_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha256-1k"_ustr;

inline constexpr OUString BLOWFISH_NAME = u"Blowfish CFB"_ustr;
inline constexpr OUString BLOWFISH_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#blowfish"_ustr;
inline constexpr OUString AES128_URL = u"http://www.w3.org/2001/04/xmlenc#aes128-cbc"_ustr;
inline constexpr OUString AES192_URL = u"http://www.w3.org/2001/04/xmlenc#aes192-cbc"_ustr;
inline constexpr OUString AES256_URL = u"http://www.w3.org/2001/04/xmlenc#aes256-cbc"_ustr;
inline constexpr OUString AESGCM128_URL = u"http://www.w3.org/2009/xmlenc11#aes128-gcm"_ustr;
inline constexpr OUString AESGCM192_URL = u"http://www.w3.org/2009/xmlenc11#aes192-gcm"_ustr;
inline constexpr OUString AESGCM256_URL = u"http://www.w3.org/2009/xmlenc11#aes256-gcm"_ustr;

inline constexpr OUString PBKDF2_NAME = u"PBKDF2"_ustr;
inline constexpr OUString PGP_NAME = u"PGP"_ustr;
inline constexpr OUString PBKDF2_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#pbkdf2"_ustr;
inline constexpr OUString ARGON2ID_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.5#argon2id"_ustr;
inline constexpr OUString ARGON2ID_URL_LO = u"urn:org:documentfoundation:names:experimental:office:manifest:argon2id"_ustr;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
