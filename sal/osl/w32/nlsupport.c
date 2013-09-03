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

#define UNICODE
#ifdef _MSC_VER
#pragma warning(push,1) /* disable warnings within system headers */
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <wchar.h>

#include <osl/mutex.h>
#include <osl/nlsupport.h>
#include <osl/diagnose.h>
#include <osl/process.h>
#include <rtl/tencinfo.h>


/* XXX NOTE:
 * http://msdn.microsoft.com/en-us/library/windows/desktop/dd373848.aspx
 * (retrieved 2013-02-13) has some weird description for the LOCALE_SISO*
 * constants: "The maximum number of characters allowed for this string is
 * nine, including a terminating null character." NINE?!? In ISO 639 and ISO
 * 3166?
 */
#define ELP_LANGUAGE_FIELD_LENGTH 4
#define ELP_COUNTRY_FIELD_LENGTH  3

/** Struct used in EnumLocalesProcA() called via EnumSystemLocalesA() to obtain
    available locales.
*/
struct EnumLocalesParams
{
    WCHAR Language[ELP_LANGUAGE_FIELD_LENGTH];
    WCHAR Country[ELP_COUNTRY_FIELD_LENGTH];
    LCID  Locale;
};

static DWORD g_dwTLSLocaleEncId = (DWORD) -1;

/*****************************************************************************
 * callback function test
 *
 * osl_getTextEncodingFromLocale calls EnumSystemLocalesA, so that we don't
 * need to provide a unicode wrapper for this function under Win9x
 * that means the callback function has an ansi prototype and receives
 * the locale strings as ansi strings
 *****************************************************************************/

BOOL CALLBACK EnumLocalesProcA( LPSTR lpLocaleStringA )
{
    struct EnumLocalesParams * params;

    LCID  localeId;
    LPSTR pszEndA;

    WCHAR langCode[ELP_LANGUAGE_FIELD_LENGTH];

    /* convert hex-string to LCID */
    localeId = strtol( lpLocaleStringA, &pszEndA, 16 );

    /* check params received via TLS */
    params = (struct EnumLocalesParams *) TlsGetValue( g_dwTLSLocaleEncId );
    if( NULL == params || '\0' == params->Language[0] )
        return FALSE;

    /*
        get the ISO language code for this locale

        remember: we call the GetLocaleInfoW function
        because the ansi version of this function returns
        an error under WinNT/2000 when called with an
        unicode only lcid
    */
    if( GetLocaleInfo( localeId, LOCALE_SISO639LANGNAME , langCode, ELP_LANGUAGE_FIELD_LENGTH ) )
    {
        WCHAR ctryCode[ELP_COUNTRY_FIELD_LENGTH];

        /* continue if language code does not match */
        if( 0 != wcscmp( langCode, params->Language ) )
            return TRUE;

        /* check if country code is set and equals the current locale */
        if( '\0' != params->Country[0] && GetLocaleInfo( localeId,
                    LOCALE_SISO3166CTRYNAME , ctryCode, ELP_COUNTRY_FIELD_LENGTH ) )
        {
            /* save return value in TLS and break if  found desired locale */
            if( 0 == wcscmp( ctryCode, params->Country ) )
            {
                params->Locale = localeId;
                return FALSE;
            }
        }
        else
        {
            /* fill with default values for that language */
            LANGID langId = LANGIDFROMLCID( localeId );

            /* exchange sublanguage with SUBLANG_NEUTRAL */
            langId = MAKELANGID( PRIMARYLANGID( langId ), SUBLANG_NEUTRAL );

            /* and use default sorting order */
            params->Locale = MAKELCID( langId, SORT_DEFAULT );

            return FALSE;
        }
    }

    /* retry by going on */
    return TRUE;
}


/*****************************************************************************
 * GetTextEncodingFromLCID
 *****************************************************************************/

rtl_TextEncoding GetTextEncodingFromLCID( LCID localeId )
{
    rtl_TextEncoding Encoding = RTL_TEXTENCODING_DONTKNOW;
    WCHAR ansiCP[6];

    /* query ansi codepage for given locale */
    if( localeId && GetLocaleInfo( localeId, LOCALE_IDEFAULTANSICODEPAGE, ansiCP, 6 ) )
    {
        /* if GetLocaleInfo returns "0", it is a UNICODE only locale */
        if( 0 != wcscmp( ansiCP, L"0" ) )
        {
            WCHAR *pwcEnd;
            UINT  codepage;

            /* values returned from GetLocaleInfo are decimal based */
            codepage = wcstol( ansiCP, &pwcEnd, 10 );

            /* find matching rtl encoding */
            Encoding = rtl_getTextEncodingFromWindowsCodePage( codepage );
        }
        else
            Encoding = RTL_TEXTENCODING_UNICODE;
    }

    return Encoding;
}


/*****************************************************************************
 * osl_getTextEncodingFromLocale
 *****************************************************************************/

rtl_TextEncoding SAL_CALL osl_getTextEncodingFromLocale( rtl_Locale * pLocale )
{
    struct EnumLocalesParams params = { L"", L"", 0 };

    /* initialise global TLS id */
    if( (DWORD) -1 == g_dwTLSLocaleEncId )
    {
        oslMutex globalMutex = * osl_getGlobalMutex();

        /* initializing must be thread save */
        osl_acquireMutex( globalMutex );

        if( (DWORD) -1 == g_dwTLSLocaleEncId )
            g_dwTLSLocaleEncId = TlsAlloc();

        osl_releaseMutex( globalMutex );
    }

    /* if pLocale is NULL, use process locale as default */
    if( NULL == pLocale )
        osl_getProcessLocale( &pLocale );

    /* copy in parameters to structure */
    if( pLocale && pLocale->Language && pLocale->Language->length < ELP_LANGUAGE_FIELD_LENGTH )
    {
        wcscpy( params.Language, pLocale->Language->buffer );

        if( pLocale->Country && pLocale->Country->length < ELP_COUNTRY_FIELD_LENGTH )
            wcscpy( params.Country, pLocale->Country->buffer );

        /* save pointer to local structure in TLS */
        TlsSetValue( g_dwTLSLocaleEncId, &params );

        /* enum all locales known to Windows */
        EnumSystemLocalesA( EnumLocalesProcA, LCID_SUPPORTED );

        /* use the LCID found in iteration */
        return GetTextEncodingFromLCID( params.Locale );
    }

    return RTL_TEXTENCODING_DONTKNOW;
}

/*****************************************************************************
 * imp_getProcessLocale
 *****************************************************************************/

void _imp_getProcessLocale( rtl_Locale ** ppLocale )
{
    WCHAR langCode[ELP_LANGUAGE_FIELD_LENGTH];
    WCHAR ctryCode[ELP_COUNTRY_FIELD_LENGTH];
    LCID  localeId;

    OSL_ASSERT( ppLocale );

    /* get the LCID to retrieve information from */
    localeId = GetUserDefaultLCID();

    /* call GetLocaleInfo to retrieve the iso codes */
    if( GetLocaleInfo( localeId, LOCALE_SISO639LANGNAME , langCode, ELP_LANGUAGE_FIELD_LENGTH )  &&
        GetLocaleInfo( localeId, LOCALE_SISO3166CTRYNAME , ctryCode, ELP_COUNTRY_FIELD_LENGTH ) )
    {
        *ppLocale = rtl_locale_register( langCode, ctryCode, L"" );
    }
    else
    {
        *ppLocale = rtl_locale_register( L"C", L"", L"" );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
