/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _PACKAGE_CONSTANTS_HXX_
#define _PACKAGE_CONSTANTS_HXX_

#include <sal/types.h>

const sal_Int32 n_ConstBufferSize = 32768;
const sal_Int32 n_ConstMaxMemoryStreamSize = 20480;
const sal_Int32 n_ConstDigestLength = 1024;

// the constants related to the manifest.xml entries
#define PKG_MNFST_MEDIATYPE 0
#define PKG_MNFST_VERSION   1
#define PKG_MNFST_FULLPATH  2

#define PKG_MNFST_INIVECTOR 3
#define PKG_MNFST_SALT      4
#define PKG_MNFST_ITERATION 5
#define PKG_MNFST_UCOMPSIZE 6
#define PKG_MNFST_DIGEST    7

#define PKG_SIZE_NOENCR_MNFST 3
#define PKG_SIZE_ENCR_MNFST   8

// the properties related constants
#define ENCRYPTION_KEY_PROPERTY "EncryptionKey"
#define STORAGE_ENCRYPTION_KEYS_PROPERTY "StorageEncryptionKeys"
#define ENCRYPTION_ALGORITHMS_PROPERTY "EncryptionAlgorithms"
#define HAS_ENCRYPTED_ENTRIES_PROPERTY "HasEncryptedEntries"
#define HAS_NONENCRYPTED_ENTRIES_PROPERTY "HasNonEncryptedEntries"
#define IS_INCONSISTENT_PROPERTY "IsInconsistent"
#define MEDIATYPE_FALLBACK_USED_PROPERTY "MediaTypeFallbackUsed"


#endif

