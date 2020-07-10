/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FILTER_MSFILTER_RTFUTIL_HXX
#define INCLUDED_FILTER_MSFILTER_RTFUTIL_HXX

#include <filter/msfilter/msfilterdllapi.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/textenc.h>
#include <sal/types.h>
#include <tools/solar.h>

// RTF values are often multiplied by 2^16
#define RTF_MULTIPLIER 65536

class SvStream;

namespace msfilter::rtfutil
{
/// Outputs a single character in hex form.
MSFILTER_DLLPUBLIC OString OutHex(sal_uLong nHex, sal_uInt8 nLen);

/// Handles correct unicode and legacy export of a single character.
MSFILTER_DLLPUBLIC OString OutChar(sal_Unicode c, int* pUCMode, rtl_TextEncoding eDestEnc,
                                   bool* pSuccess, bool bUnicode = true);

/**
 * Handles correct unicode and legacy export of a string.
 *
 * @param rStr the string to export
 * @param eDestEnc the legacy encoding to use
 * @param bUnicode if unicode output is wanted as well, or just legacy
 */
MSFILTER_DLLPUBLIC OString OutString(const OUString& rStr, rtl_TextEncoding eDestEnc,
                                     bool bUnicode = true);

/**
 * Handles correct unicode and legacy export of a string, when a
 * '{' \upr '{' keyword ansi_text '}{\*' \ud '{' keyword Unicode_text '}}}'
 * construct should be used.
 *
 * @param pToken the keyword
 * @param rStr the text to export
 * @param eDestEnc the legacy encoding to use
 */
MSFILTER_DLLPUBLIC OString OutStringUpr(const char* pToken, const OUString& rStr,
                                        rtl_TextEncoding eDestEnc);

/**
 * Get the numeric value of a single character, representing a hex value.
 *
 * @return -1 on failure
 */
MSFILTER_DLLPUBLIC int AsHex(char ch);

/// Writes binary data as a hex dump.
MSFILTER_DLLPUBLIC OString WriteHex(const sal_uInt8* pData, sal_uInt32 nSize,
                                    SvStream* pStream = nullptr, sal_uInt32 nLimit = 64);

/**
 * Extract OLE2 data from an \objdata hex dump.
 */
MSFILTER_DLLPUBLIC bool ExtractOLE2FromObjdata(const OString& rObjdata, SvStream& rOle2);

/// Strips the header of a WMF file.
MSFILTER_DLLPUBLIC bool StripMetafileHeader(const sal_uInt8*& rpGraphicAry, sal_uInt64& rSize);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
