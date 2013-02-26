/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_MSFILTER_RTFUTIL_HXX
#define INCLUDED_MSFILTER_RTFUTIL_HXX

#include "filter/msfilter/msfilterdllapi.h"
#include <rtl/string.hxx>
#include <rtl/textenc.h>
#include <tools/string.hxx>

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
MSFILTER_DLLPUBLIC OString OutString(const String &rStr, rtl_TextEncoding eDestEnc, bool bUnicode = true);

/**
 * Handles correct unicode and legacy export of a string, when a
 * '{' \upr '{' keyword ansi_text '}{\*' \ud '{' keyword Unicode_text '}}}'
 * construct should be used.
 *
 * @param pToken the keyword
 * @param rStr the text to export
 * @param eDestEnc the legacy encoding to use
 */
MSFILTER_DLLPUBLIC OString OutStringUpr(const sal_Char *pToken, const String &rStr, rtl_TextEncoding eDestEnc);

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
