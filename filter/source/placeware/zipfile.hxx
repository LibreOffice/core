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

#ifndef INCLUDED_FILTER_SOURCE_PLACEWARE_ZIPFILE_HXX
#define INCLUDED_FILTER_SOURCE_PLACEWARE_ZIPFILE_HXX

/* Local file head offsets.  The header is followed by the filename and
   possibly the extra field. */
#define zf_lfhSig       0
#define zf_lfhExVers        4
#define zf_lfhFlags     6
#define zf_lfhComp      8
#define zf_lfhModTime       10
#define zf_lfhModDate       12
#define zf_lfhCRC       14
#define zf_lfhCompSize      18
#define zf_lfhUncompSize    22
#define zf_lfhNameLen       26
#define zf_lfhExtraLen      28
#define zf_lfhSIZE      30 /* total size of header */

/* We don't write data descriptors. */

/* Central dir file header offsets.  Each entry is followed by the filename,
   optional extra field, and file comment. */
#define zf_cdhSig       0
#define zf_cdhVers      4
#define zf_cdhExVers        6
#define zf_cdhFlags     8
#define zf_cdhComp      10
#define zf_cdhTime      12
#define zf_cdhDate      14
#define zf_cdhCRC       16
#define zf_cdhCompSize      20
#define zf_cdhUncompSize    24
#define zf_cdhNameLen       28
#define zf_cdhExtraLen      30
#define zf_cdhCommentLen    32
#define zf_cdhDiskNum       34
#define zf_cdhIFAttrs       36
#define zf_cdhEFAttrs       38
#define zf_cdhLHOffset      42
#define zf_cdhSIZE      46 /* total size of header */

/* End of central dir record offsets.  It is followed by the zipfile
   comment. */
#define zf_ecdSig       0
#define zf_ecdDiskNum       4
#define zf_ecdDirDiskNum    6
#define zf_ecdNumEntries    8
#define zf_ecdTotalEntries  10
#define zf_ecdDirSize       12
#define zf_ecdDirOffset     16
#define zf_ecdCommentLen    20
#define zf_ecdSIZE      22 /* total size */

/* Magic constants to put in these structures. */
#define zf_LFHSIGValue      0x04034b50
#define zf_CDHSIGValue      0x02014b50
#define zf_ECDSIGValue      0x06054b50

/* OS values for upper byte of version field. */
#define zf_osUnix       3

/* Encode a major,minor version in a byte. */
#define zf_Vers(major,minor)    ((major) * 10 + (minor))

/* Compression values. */
#define zf_compNone     0

#endif // INCLUDED_FILTER_SOURCE_PLACEWARE_ZIPFILE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
