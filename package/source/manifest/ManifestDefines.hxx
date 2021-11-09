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

#include <PackageConstants.hxx>

inline constexpr OUStringLiteral MANIFEST_NSPREFIX = u"manifest:";
inline constexpr OUStringLiteral ELEMENT_MANIFEST = u"manifest:manifest";
inline constexpr OUStringLiteral ATTRIBUTE_XMLNS = u"xmlns:manifest";
inline constexpr OUStringLiteral ATTRIBUTE_XMLNS_LOEXT = u"xmlns:loext";
inline constexpr OUStringLiteral MANIFEST_NAMESPACE = u"http://openoffice.org/2001/manifest";
inline constexpr OUStringLiteral MANIFEST_OASIS_NAMESPACE = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0";
inline constexpr OUStringLiteral MANIFEST_LOEXT_NAMESPACE = u"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0";
inline constexpr OUStringLiteral MANIFEST_DOCTYPE = u"<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">";
inline constexpr OUStringLiteral ATTRIBUTE_CDATA = u"CDATA";

inline constexpr OUStringLiteral ELEMENT_FILE_ENTRY = u"manifest:file-entry";
inline constexpr OUStringLiteral ATTRIBUTE_FULL_PATH  = u"manifest:full-path";
inline constexpr OUStringLiteral ATTRIBUTE_VERSION  = u"manifest:version";
inline constexpr OUStringLiteral ATTRIBUTE_MEDIA_TYPE = u"manifest:media-type";
inline constexpr OUStringLiteral ATTRIBUTE_SIZE  = u"manifest:size";
inline constexpr OUStringLiteral ELEMENT_MANIFEST_KEYINFO  = u"loext:keyinfo";
inline constexpr OUStringLiteral ELEMENT_ENCRYPTED_KEYINFO = u"loext:KeyInfo";
inline constexpr OUStringLiteral ELEMENT_ENCRYPTEDKEY  = u"loext:encrypted-key";
inline constexpr OUStringLiteral ELEMENT_ENCRYPTIONMETHOD = u"loext:encryption-method";
inline constexpr OUStringLiteral ELEMENT_PGPDATA  = u"loext:PGPData";
inline constexpr OUStringLiteral ELEMENT_PGPKEYID  = u"loext:PGPKeyID";
inline constexpr OUStringLiteral ELEMENT_PGPKEYPACKET = u"loext:PGPKeyPacket";
inline constexpr OUStringLiteral ATTRIBUTE_ALGORITHM = u"loext:PGPAlgorithm";
inline constexpr OUStringLiteral ELEMENT_CIPHERDATA = u"loext:CipherData";
inline constexpr OUStringLiteral ELEMENT_CIPHERVALUE = u"loext:CipherValue";
inline constexpr OUStringLiteral ELEMENT_MANIFEST13_KEYINFO = u"manifest:keyinfo";
inline constexpr OUStringLiteral ELEMENT_ENCRYPTEDKEY13 = u"manifest:encrypted-key";
inline constexpr OUStringLiteral ELEMENT_ENCRYPTIONMETHOD13 = u"manifest:encryption-method";
inline constexpr OUStringLiteral ELEMENT_PGPDATA13 = u"manifest:PGPData";
inline constexpr OUStringLiteral ELEMENT_PGPKEYID13 = u"manifest:PGPKeyID";
inline constexpr OUStringLiteral ELEMENT_PGPKEYPACKET13 = u"manifest:PGPKeyPacket";
inline constexpr OUStringLiteral ATTRIBUTE_ALGORITHM13 = u"manifest:PGPAlgorithm";
inline constexpr OUStringLiteral ELEMENT_CIPHERDATA13 = u"manifest:CipherData";
inline constexpr OUStringLiteral ELEMENT_CIPHERVALUE13 = u"manifest:CipherValue";

inline constexpr OUStringLiteral ELEMENT_ENCRYPTION_DATA = u"manifest:encryption-data";
inline constexpr OUStringLiteral ATTRIBUTE_CHECKSUM_TYPE = u"manifest:checksum-type";
inline constexpr OUStringLiteral ATTRIBUTE_CHECKSUM  = u"manifest:checksum";

inline constexpr OUStringLiteral ELEMENT_ALGORITHM = u"manifest:algorithm";
inline constexpr OUStringLiteral ATTRIBUTE_ALGORITHM_NAME = u"manifest:algorithm-name";
inline constexpr OUStringLiteral ATTRIBUTE_INITIALISATION_VECTOR = u"manifest:initialisation-vector";

inline constexpr OUStringLiteral ELEMENT_START_KEY_GENERATION  = u"manifest:start-key-generation";
inline constexpr OUStringLiteral ATTRIBUTE_START_KEY_GENERATION_NAME  = u"manifest:start-key-generation-name";
inline constexpr OUStringLiteral ATTRIBUTE_KEY_SIZE  = u"manifest:key-size";

inline constexpr OUStringLiteral ELEMENT_KEY_DERIVATION = u"manifest:key-derivation";
inline constexpr OUStringLiteral ATTRIBUTE_KEY_DERIVATION_NAME  = u"manifest:key-derivation-name";
inline constexpr OUStringLiteral ATTRIBUTE_SALT  = u"manifest:salt";
inline constexpr OUStringLiteral ATTRIBUTE_ITERATION_COUNT = u"manifest:iteration-count";

/// OFFICE-3708: wrong URL cited in ODF 1.2 and used since OOo 3.4 beta
inline constexpr OUStringLiteral SHA256_URL_ODF12 = u"http://www.w3.org/2000/09/xmldsig#sha256";
inline constexpr OUStringLiteral SHA256_URL = u"http://www.w3.org/2001/04/xmlenc#sha256";
inline constexpr OUStringLiteral SHA1_NAME = u"SHA1";
inline constexpr OUStringLiteral SHA1_URL = u"http://www.w3.org/2000/09/xmldsig#sha1";

inline constexpr OUStringLiteral SHA1_1K_NAME = u"SHA1/1K";
inline constexpr OUStringLiteral SHA1_1K_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha1-1k";
inline constexpr OUStringLiteral SHA256_1K_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha256-1k";

inline constexpr OUStringLiteral BLOWFISH_NAME = u"Blowfish CFB";
inline constexpr OUStringLiteral BLOWFISH_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#blowfish";
inline constexpr OUStringLiteral AES128_URL = u"http://www.w3.org/2001/04/xmlenc#aes128-cbc";
inline constexpr OUStringLiteral AES192_URL = u"http://www.w3.org/2001/04/xmlenc#aes192-cbc";
inline constexpr OUStringLiteral AES256_URL = u"http://www.w3.org/2001/04/xmlenc#aes256-cbc";

inline constexpr OUStringLiteral PBKDF2_NAME = u"PBKDF2";
inline constexpr OUStringLiteral PGP_NAME = u"PGP";
inline constexpr OUStringLiteral PBKDF2_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#pbkdf2";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
