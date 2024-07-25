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
#ifndef INCLUDED_PACKAGE_INC_PACKAGECONSTANTS_HXX
#define INCLUDED_PACKAGE_INC_PACKAGECONSTANTS_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

inline constexpr sal_Int32 n_ConstBufferSize = 32768;

// by calculation of the digest we read 32 bytes more ( if available )
// it allows to ignore the padding if the stream is longer than n_ConstDigestDecrypt since we read at least two blocks more;
// if the stream is shorter or equal the padding will be done successfully
inline constexpr sal_Int32 n_ConstDigestLength = 1024;
inline constexpr sal_Int32 n_ConstDigestDecrypt = 1056; // 1024 + 32

// the constants related to the manifest.xml entries
// these primarily exist so that ManifestImport can directly write into Sequence
#define PKG_MNFST_FULLPATH 0 //FullPath (Put full-path property first for MBA)
#define PKG_MNFST_VERSION 1 //Version
#define PKG_MNFST_MEDIATYPE 2 //MediaType

#define PKG_MNFST_INIVECTOR 3 //InitialisationVector
#define PKG_MNFST_SALT 4 //Salt
#define PKG_MNFST_ITERATION 5 //IterationCount
#define PKG_MNFST_UCOMPSIZE 6 //Size
#define PKG_MNFST_DIGEST 7 //Digest
#define PKG_MNFST_ENCALG 8 //EncryptionAlgorithm
#define PKG_MNFST_STARTALG 9 //StartKeyAlgorithm
#define PKG_MNFST_DIGESTALG 10 //DigestAlgorithm
#define PKG_MNFST_DERKEYSIZE 11 //DerivedKeySize
#define PKG_MNFST_KDF 12 // KeyDerivationFunction
#define PKG_MNFST_ARGON2ARGS 13 // Argon2 arguments
#define PKG_MNFST_KEYINFO 14 // PGP KeyInfo

#define PKG_SIZE_NOENCR_MNFST 3
#define PKG_SIZE_ENCR_MNFST 15 // max size

// the properties related constants
inline constexpr OUString ENCRYPTION_KEY_PROPERTY = u"EncryptionKey"_ustr;
inline constexpr OUString STORAGE_ENCRYPTION_KEYS_PROPERTY = u"StorageEncryptionKeys"_ustr;
inline constexpr OUString ENCRYPTION_ALGORITHMS_PROPERTY = u"EncryptionAlgorithms"_ustr;
inline constexpr OUString ENCRYPTION_GPG_PROPERTIES = u"EncryptionGpGProperties"_ustr;
#define HAS_ENCRYPTED_ENTRIES_PROPERTY "HasEncryptedEntries"
#define HAS_NONENCRYPTED_ENTRIES_PROPERTY "HasNonEncryptedEntries"
#define IS_INCONSISTENT_PROPERTY "IsInconsistent"
inline constexpr OUString MEDIATYPE_FALLBACK_USED_PROPERTY = u"MediaTypeFallbackUsed"_ustr;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
