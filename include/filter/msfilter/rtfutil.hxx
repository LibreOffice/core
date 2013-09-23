/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_MSFILTER_RTFUTIL_HXX
#define INCLUDED_MSFILTER_RTFUTIL_HXX

#include <filter/msfilter/msfilterdllapi.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/textenc.h>
#include <tools/solar.h>

namespace msfilter {
namespace rtfutil {

/// Outputs a single character in hex form.
MSFILTER_DLLPUBLIC OString OutHex(sal_uLong nHex, sal_uInt8 nLen);

/// Handles correct unicode and legacy export of a single character.
MSFILTER_DLLPUBLIC OString OutChar(sal_Unicode c, int *pUCMode, rtl_TextEncoding eDestEnc, bool* pSuccess = 0, bool bUnicode = true);

/**
 * Handles correct unicode and legacy export of a string.
 *
 * @param rStr the string to export
 * @param eDestEnc the legacy encoding to use
 * @param bUnicode if unicode output is wanted as well, or just legacy
 */
MSFILTER_DLLPUBLIC OString OutString(const OUString &rStr, rtl_TextEncoding eDestEnc, bool bUnicode = true);

/**
 * Handles correct unicode and legacy export of a string, when a
 * '{' \upr '{' keyword ansi_text '}{\*' \ud '{' keyword Unicode_text '}}}'
 * construct should be used.
 *
 * @param pToken the keyword
 * @param rStr the text to export
 * @param eDestEnc the legacy encoding to use
 */
MSFILTER_DLLPUBLIC OString OutStringUpr(const sal_Char *pToken, const OUString &rStr, rtl_TextEncoding eDestEnc);

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
