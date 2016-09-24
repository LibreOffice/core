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

#ifndef INCLUDED_SAL_TEXTENC_CONVERTSINGLEBYTETOBMPUNICODE_HXX
#define INCLUDED_SAL_TEXTENC_CONVERTSINGLEBYTETOBMPUNICODE_HXX

#include "sal/config.h"

#include <cstddef>

#include "sal/types.h"


/// @HTML

namespace rtl { namespace textenc {

/**
   Maps a range of BMP Unicode code points to individual bytes.

   @see rtl::textenc::BmpUnicodeToSingleByteConverterData
 */
struct BmpUnicodeToSingleByteRange {
    /**
       The start of the range of BMP Unicode code points.
     */
    sal_Unicode unicode;

    /**
       The extend of the range of BMP Unicode code points.

       <p>The range covers <code>unicode</code> to <code>unicode + range</code>,
       inclusive.  It is an error if <code>unicode + range</code> is greater
       than <code>0xFFFF</code>.</p>
     */
    sal_uInt8 range;

    /**
       The start of the corresponding range of individual bytes.

       <p>It is an error if <code>byte + range</code> is greater than
       <code>0xFF</code>.</p>
     */
    sal_uInt8 byte;
};

/**
   Data to convert between BMP Unicode and a single-byte character set.

   <p>Only supports conversions where each legal unit from the single-byte
   character set has one or more mappings to individual BMP Unicode code points
   that are neither noncharacters nor surrogates.</p>

   @see rtl_textenc_convertSingleByteToBmpUnicode
   @see rtl_textenc_convertBmpUnicodeToSingleByte
 */
struct BmpUnicodeToSingleByteConverterData {
    /**
       Mapping from the single-byte character set to BMP Unicode code points.

       <p>Illegal units from the single-byte character set are mapped to
       <code>0xFFFF</code>.</p>
     */
    sal_Unicode byteToUnicode[256];

    /**
       The number of Unicode-to-byte conversion ranges.
     */
    std::size_t unicodeToByteEntries;

    /**
       The array of Unicode-to-byte conversion ranges, sorted by increasing
       <code>unicode</code> values.

       <p>The ranges may not overlap.</p>
     */
    BmpUnicodeToSingleByteRange const * unicodeToByte;
};

} }

/**
   Function to convert from a single-byte character set to BMP Unicode.

   @see ImplConvertToUnicodeProc
 */
sal_Size rtl_textenc_convertSingleByteToBmpUnicode(
    void const * data, void * context, sal_Char const * srcBuf,
    sal_Size srcBytes, sal_Unicode * destBuf, sal_Size destChars,
    sal_uInt32 flags, sal_uInt32 * info, sal_Size * srcCvtBytes);

/**
   Function to convert from BMP Unicode to a single-byte character set.

   @see ImplConvertToTextProc
 */
sal_Size rtl_textenc_convertBmpUnicodeToSingleByte(
    void const * data, void * context,
    sal_Unicode const * srcBuf, sal_Size srcChars, sal_Char * destBuf,
    sal_Size destBytes, sal_uInt32 flags, sal_uInt32 * info,
    sal_Size * srcCvtChars);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
