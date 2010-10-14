/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_sal_textenc_convertsinglebytetobmpunicode_hxx
#define INCLUDED_sal_textenc_convertsinglebytetobmpunicode_hxx

#include "tenchelp.h"

#include "sal/types.h"

#include <cstddef>

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
extern "C" sal_Size rtl_textenc_convertSingleByteToBmpUnicode(
    ImplTextConverterData const * data, void * context, sal_Char const * srcBuf,
    sal_Size srcBytes, sal_Unicode * destBuf, sal_Size destChars,
    sal_uInt32 flags, sal_uInt32 * info, sal_Size * srcCvtBytes);

/**
   Function to convert from BMP Unicode to a single-byte character set.

   @see ImplConvertToTextProc
 */
extern "C" sal_Size rtl_textenc_convertBmpUnicodeToSingleByte(
    ImplTextConverterData const * data, void * context,
    sal_Unicode const * srcBuf, sal_Size srcChars, sal_Char * destBuf,
    sal_Size destBytes, sal_uInt32 flags, sal_uInt32 * info,
    sal_Size * srcCvtChars);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
