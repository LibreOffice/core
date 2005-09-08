/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: convertsinglebytetobmpunicode.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:34:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
