/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOTOOLS_WINCODEPAGE_HXX
#define INCLUDED_UNOTOOLS_WINCODEPAGE_HXX

#include <unotools/unotoolsdllapi.h>
#include <rtl/textenc.h>

/** Map from an ISO-639 language code (and optionally ISO-3166 country/region code)
to a text encoding of corresponding Windows ANSI or OEM codepage.

@param pLanguage
Any language-country string.  Must not be null.

@param bOEM
If true, OEM codepage is returned, otherwise ANSI.

@return
The corresponding rtl_TextEncoding value.
If no mapping is found, RTL_TEXTENCODING_IBM_850 is returned when bOEM is true,
RTL_TEXTENCODING_MS_1252 otherwise.
*/
UNOTOOLS_DLLPUBLIC rtl_TextEncoding utl_getWinTextEncodingFromLangStr(
    const char* pLanguage, bool bOEM = false);

#endif  // INCLUDED_UNOTOOLS_WINCODEPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
