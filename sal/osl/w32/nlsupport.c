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

struct EnumLocalesParams
{
    WCHAR Language[3];
    WCHAR Country[3];
    LCID  Locale;
};

static DWORD g_dwTLSLocaleEncId = (DWORD) -1;

/*****************************************************************************/
/* callback function test
/*
/* osl_getTextEncodingFromLocale calls EnumSystemLocalesA, so that we don't
/* need to provide a unicode wrapper for this function under Win9x
/* that means the callback function has an ansi prototype and receives
/* the locale strings as ansi strings
/*****************************************************************************/

BOOL CALLBACK EnumLocalesProcA( LPSTR lpLocaleStringA )
{
    struct EnumLocalesParams * params;

    LCID  localeId;
    LPSTR pszEndA;

    WCHAR langCode[4];

    /* convert hex-string to LCID */
    localeId = strtol( lpLocaleStringA, &pszEndA, 16 );

    /* check params received via TLS */
    params = (struct EnumLocalesParams *) TlsGetValue( g_dwTLSLocaleEncId );
    if( NULL == params || '\0' == params->Language[0] )
        return FALSE;

    /*
        get the ISO language code for this locale

        remeber: we call the GetLocaleInfoW function
        because the ansi version of this function returns
        an error under WinNT/2000 when called with an
        unicode only lcid
    */
    if( GetLocaleInfo( localeId, LOCALE_SISO639LANGNAME , langCode, 4 ) )
    {
        WCHAR ctryCode[4];

        /* continue if language code does not match */
        if( 0 != wcscmp( langCode, params->Language ) )
            return TRUE;

        /* check if country code is set and equals the current locale */
        if( '\0' != params->Country[0] && GetLocaleInfo( localeId, LOCALE_SISO3166CTRYNAME , ctryCode, 4 ) )
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


/*****************************************************************************/
/* GetTextEncodingFromLCID
/*****************************************************************************/

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

            /* values returned from GetLocaleInfo are dezimal based */
            codepage = wcstol( ansiCP, &pwcEnd, 10 );

            /* find matching rtl encoding */
            Encoding = rtl_getTextEncodingFromWindowsCodePage( codepage );
        }
        else
            Encoding = RTL_TEXTENCODING_UNICODE;
    }

    return Encoding;
}


/*****************************************************************************/
/* osl_getTextEncodingFromLocale
/*****************************************************************************/

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
    if( pLocale && pLocale->Language )
    {
        wcscpy( params.Language, pLocale->Language->buffer );

        if( pLocale->Country )
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

/*****************************************************************************/
/* imp_getProcessLocale
/*****************************************************************************/

void _imp_getProcessLocale( rtl_Locale ** ppLocale )
{
    WCHAR langCode[4];
    WCHAR ctryCode[4];
    LCID  localeId;

    OSL_ASSERT( ppLocale );

    /* get the LCID to retrieve information from */
    localeId = GetUserDefaultLCID();

    /* call GetLocaleInfo to retrieve the iso codes */
    if( GetLocaleInfo( localeId, LOCALE_SISO639LANGNAME , langCode, 4 )  &&
        GetLocaleInfo( localeId, LOCALE_SISO3166CTRYNAME , ctryCode, 4 ) )
    {
        *ppLocale = rtl_locale_register( langCode, ctryCode, L"" );
    }
    else
    {
        *ppLocale = rtl_locale_register( L"C", L"", L"" );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
