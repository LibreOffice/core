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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wchar.h>

#include "nlsupport.hxx"

#include <osl/nlsupport.h>
#include <osl/diagnose.h>
#include <osl/process.h>
#include <rtl/tencinfo.h>
#include <rtl/ustrbuf.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

/* XXX NOTE:
 * http://msdn.microsoft.com/en-us/library/windows/desktop/dd373848.aspx
 * (retrieved 2013-02-13) has some weird description for the LOCALE_SISO*
 * constants: "The maximum number of characters allowed for this string is
 * nine, including a terminating null character." NINE?!? In ISO 639 and ISO
 * 3166?
 */
constexpr int ELP_LANGUAGE_FIELD_LENGTH = 4;
constexpr int ELP_COUNTRY_FIELD_LENGTH = 3;

static int GetLocaleInfoN(LPCWSTR l, LCTYPE t, DWORD& n)
{
    return GetLocaleInfoEx(l, t | LOCALE_RETURN_NUMBER, reinterpret_cast<LPWSTR>(&n),
                           sizeof(n) / sizeof(WCHAR));
}

rtl_TextEncoding SAL_CALL osl_getTextEncodingFromLocale( rtl_Locale * pLocale )
{
    /* if pLocale is NULL, use process locale as default */
    if( nullptr == pLocale )
        osl_getProcessLocale( &pLocale );

    if (!pLocale || !pLocale->Language || !pLocale->Language->length)
        return RTL_TEXTENCODING_DONTKNOW;

    /* Build a BCP47 tag */
    OUStringBuffer sLocale(OUString::unacquired(&pLocale->Language));
    if (pLocale->Country && pLocale->Country->length > 0)
        sLocale.append("-" + OUString::unacquired(&pLocale->Country));
    sLocale.append('\0');

    /* query ansi codepage for given locale */
    DWORD codepage;
    if (!GetLocaleInfoN(o3tl::toW(sLocale.getStr()), LOCALE_IDEFAULTANSICODEPAGE, codepage))
    {
        WCHAR resolved[LOCALE_NAME_MAX_LENGTH];
        if (!ResolveLocaleName(o3tl::toW(sLocale.getStr()), resolved, std::size(resolved)))
            return RTL_TEXTENCODING_DONTKNOW;
        if (!GetLocaleInfoN(resolved, LOCALE_IDEFAULTANSICODEPAGE, codepage))
            return RTL_TEXTENCODING_DONTKNOW;
    }

    /* if GetLocaleInfo returns 0, it is a UNICODE only locale */
    if (0 == codepage)
        return RTL_TEXTENCODING_UNICODE;

    /* find matching rtl encoding */
    return rtl_getTextEncodingFromWindowsCodePage(codepage);
}

void imp_getProcessLocale( rtl_Locale ** ppLocale )
{
    WCHAR locale[LOCALE_NAME_MAX_LENGTH];
    WCHAR langCode[ELP_LANGUAGE_FIELD_LENGTH];
    WCHAR ctryCode[ELP_COUNTRY_FIELD_LENGTH];

    assert(ppLocale);

    /* get the locale name to retrieve information from */
    /* and call GetLocaleInfo to retrieve the iso codes */
    if( GetUserDefaultLocaleName(locale, std::size(locale)) &&
        GetLocaleInfoEx( locale, LOCALE_SISO639LANGNAME , langCode, std::size(langCode) )  &&
        GetLocaleInfoEx( locale, LOCALE_SISO3166CTRYNAME , ctryCode, std::size(ctryCode) ) )
    {
        *ppLocale = rtl_locale_register( o3tl::toU(langCode), o3tl::toU(ctryCode), u"" );
    }
    else
    {
        *ppLocale = rtl_locale_register( u"C", u"", u"" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
