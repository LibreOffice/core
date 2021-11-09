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

constexpr OUStringLiteral MANIFEST_NSPREFIX = u"manifest:";
constexpr OUStringLiteral ELEMENT_MANIFEST = u"manifest:manifest";
constexpr OUStringLiteral ATTRIBUTE_XMLNS = u"xmlns:manifest";
constexpr OUStringLiteral ATTRIBUTE_XMLNS_LOEXT = u"xmlns:loext";
constexpr OUStringLiteral MANIFEST_NAMESPACE = u"http://openoffice.org/2001/manifest";
constexpr OUStringLiteral MANIFEST_OASIS_NAMESPACE = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0";
constexpr OUStringLiteral MANIFEST_LOEXT_NAMESPACE = u"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0";
constexpr OUStringLiteral MANIFEST_DOCTYPE = u"<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">";
constexpr OUStringLiteral ATTRIBUTE_CDATA = u"CDATA";

constexpr OUStringLiteral ELEMENT_FILE_ENTRY = u"manifest:file-entry";
constexpr OUStringLiteral ATTRIBUTE_FULL_PATH  = u"manifest:full-path";
constexpr OUStringLiteral ATTRIBUTE_VERSION  = u"manifest:version";
constexpr OUStringLiteral ATTRIBUTE_MEDIA_TYPE = u"manifest:media-type";
constexpr OUStringLiteral ATTRIBUTE_SIZE  = u"manifest:size";
constexpr OUStringLiteral ELEMENT_MANIFEST_KEYINFO  = u"loext:keyinfo";
constexpr OUStringLiteral ELEMENT_ENCRYPTED_KEYINFO = u"loext:KeyInfo";
constexpr OUStringLiteral ELEMENT_ENCRYPTEDKEY  = u"loext:encrypted-key";
constexpr OUStringLiteral ELEMENT_ENCRYPTIONMETHOD = u"loext:encryption-method";
constexpr OUStringLiteral ELEMENT_PGPDATA  = u"loext:PGPData";
constexpr OUStringLiteral ELEMENT_PGPKEYID  = u"loext:PGPKeyID";
constexpr OUStringLiteral ELEMENT_PGPKEYPACKET = u"loext:PGPKeyPacket";
constexpr OUStringLiteral ATTRIBUTE_ALGORITHM = u"loext:PGPAlgorithm";
constexpr OUStringLiteral ELEMENT_CIPHERDATA = u"loext:CipherData";
constexpr OUStringLiteral ELEMENT_CIPHERVALUE = u"loext:CipherValue";
constexpr OUStringLiteral ELEMENT_MANIFEST13_KEYINFO = u"manifest:keyinfo";
constexpr OUStringLiteral ELEMENT_ENCRYPTEDKEY13 = u"manifest:encrypted-key";
constexpr OUStringLiteral ELEMENT_ENCRYPTIONMETHOD13 = u"manifest:encryption-method";
constexpr OUStringLiteral ELEMENT_PGPDATA13 = u"manifest:PGPData";
constexpr OUStringLiteral ELEMENT_PGPKEYID13 = u"manifest:PGPKeyID";
constexpr OUStringLiteral ELEMENT_PGPKEYPACKET13 = u"manifest:PGPKeyPacket";
constexpr OUStringLiteral ATTRIBUTE_ALGORITHM13 = u"manifest:PGPAlgorithm";
constexpr OUStringLiteral ELEMENT_CIPHERDATA13 = u"manifest:CipherData";
constexpr OUStringLiteral ELEMENT_CIPHERVALUE13 = u"manifest:CipherValue";

constexpr OUStringLiteral ELEMENT_ENCRYPTION_DATA = u"manifest:encryption-data";
constexpr OUStringLiteral ATTRIBUTE_CHECKSUM_TYPE = u"manifest:checksum-type";
constexpr OUStringLiteral ATTRIBUTE_CHECKSUM  = u"manifest:checksum";

constexpr OUStringLiteral ELEMENT_ALGORITHM = u"manifest:algorithm";
constexpr OUStringLiteral ATTRIBUTE_ALGORITHM_NAME = u"manifest:algorithm-name";
constexpr OUStringLiteral ATTRIBUTE_INITIALISATION_VECTOR = u"manifest:initialisation-vector";

constexpr OUStringLiteral ELEMENT_START_KEY_GENERATION  = u"manifest:start-key-generation";
constexpr OUStringLiteral ATTRIBUTE_START_KEY_GENERATION_NAME  = u"manifest:start-key-generation-name";
constexpr OUStringLiteral ATTRIBUTE_KEY_SIZE  = u"manifest:key-size";

constexpr OUStringLiteral ELEMENT_KEY_DERIVATION = u"manifest:key-derivation";
constexpr OUStringLiteral ATTRIBUTE_KEY_DERIVATION_NAME  = u"manifest:key-derivation-name";
constexpr OUStringLiteral ATTRIBUTE_SALT  = u"manifest:salt";
constexpr OUStringLiteral ATTRIBUTE_ITERATION_COUNT = u"manifest:iteration-count";

/// OFFICE-3708: wrong URL cited in ODF 1.2 and used since OOo 3.4 beta
constexpr OUStringLiteral SHA256_URL_ODF12 = u"http://www.w3.org/2000/09/xmldsig#sha256";
constexpr OUStringLiteral SHA256_URL = u"http://www.w3.org/2001/04/xmlenc#sha256";
constexpr OUStringLiteral SHA1_NAME = u"SHA1";
constexpr OUStringLiteral SHA1_URL = u"http://www.w3.org/2000/09/xmldsig#sha1";

constexpr OUStringLiteral SHA1_1K_NAME = u"SHA1/1K";
constexpr OUStringLiteral SHA1_1K_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha1-1k";
constexpr OUStringLiteral SHA256_1K_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#sha256-1k";

constexpr OUStringLiteral BLOWFISH_NAME = u"Blowfish CFB";
constexpr OUStringLiteral BLOWFISH_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#blowfish";
constexpr OUStringLiteral AES128_URL = u"http://www.w3.org/2001/04/xmlenc#aes128-cbc";
constexpr OUStringLiteral AES192_URL = u"http://www.w3.org/2001/04/xmlenc#aes192-cbc";
constexpr OUStringLiteral AES256_URL = u"http://www.w3.org/2001/04/xmlenc#aes256-cbc";

constexpr OUStringLiteral PBKDF2_NAME = u"PBKDF2";
constexpr OUStringLiteral PGP_NAME = u"PGP";
constexpr OUStringLiteral PBKDF2_URL = u"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0#pbkdf2";

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
