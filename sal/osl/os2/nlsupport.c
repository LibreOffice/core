/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nlsupport.c,v $
 * $Revision: 1.5 $
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

#define INCL_WIN
#include "svpm.h"

#include <osl/nlsupport.h>
#include <osl/diagnose.h>
#include <osl/process.h>
#include <rtl/memory.h>

#include <string.h>

/*****************************************************************************
 typedefs
 *****************************************************************************/


typedef struct {
    const char              *key;
    const rtl_TextEncoding   value;
} _pair;


/*****************************************************************************
 compare function for binary search
 *****************************************************************************/

static int
_pair_compare (const char *key, const _pair *pair)
{
    int result = rtl_str_compareIgnoreAsciiCase( key, pair->key );
    return result;
}

/*****************************************************************************
 binary search on encoding tables
 *****************************************************************************/

static const _pair*
_pair_search (const char *key, const _pair *base, unsigned int member )
{
    unsigned int lower = 0;
    unsigned int upper = member;
    unsigned int current;
    int comparison;

    /* check for validity of input */
    if ( (key == NULL) || (base == NULL) || (member == 0) )
        return NULL;

    /* binary search */
    while ( lower < upper )
    {
        current = (lower + upper) / 2;
        comparison = _pair_compare( key, base + current );
        if (comparison < 0)
            upper = current;
        else if (comparison > 0)
            lower = current + 1;
        else
            return base + current;
    }

    return NULL;
}


/*****************************************************************************
 convert rtl_Locale to locale string
 *****************************************************************************/

static char * _compose_locale( rtl_Locale * pLocale, char * buffer, size_t n )
{
    /* check if a valid locale is specified */
    if( pLocale && pLocale->Language && (pLocale->Language->length == 2) )
    {
        size_t offset = 0;

        /* convert language code to ascii */
        {
            rtl_String *pLanguage = NULL;

            rtl_uString2String( &pLanguage,
                pLocale->Language->buffer, pLocale->Language->length,
                RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS );

            if( pLanguage->length < n )
            {
                strcpy( buffer, pLanguage->buffer );
                offset = pLanguage->length;
            }

            rtl_string_release( pLanguage );
        }

        /* convert country code to ascii */
        if( pLocale->Country && (pLocale->Country->length == 2) )
        {
            rtl_String *pCountry = NULL;

            rtl_uString2String( &pCountry,
                pLocale->Country->buffer, pLocale->Country->length,
                RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS );

            if( offset + pCountry->length + 1 < n )
            {
                strcpy( buffer + offset++, "_" );
                strcpy( buffer + offset, pCountry->buffer );
                offset += pCountry->length;
            }

            rtl_string_release( pCountry );
        }

        /* convert variant to ascii - check if there is enough space for the variant string */
        if( pLocale->Variant && pLocale->Variant->length &&
            ( pLocale->Variant->length < n - 6 ) )
        {
            rtl_String *pVariant = NULL;

            rtl_uString2String( &pVariant,
                pLocale->Variant->buffer, pLocale->Variant->length,
                RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS );

            if( offset + pVariant->length + 1 < n )
            {
                strcpy( buffer + offset, pVariant->buffer );
                offset += pVariant->length;
            }

            rtl_string_release( pVariant );
        }

        return buffer;
    }

    return NULL;
}

/*****************************************************************************
 convert locale string to rtl_Locale
 *****************************************************************************/

static rtl_Locale * _parse_locale( const char * locale )
{
    static sal_Unicode c_locale[2] = { (sal_Unicode) 'C', 0 };

    /* check if locale contains a valid string */
    if( locale )
    {
        size_t len = strlen( locale );

        if( len >= 2 )
        {
            rtl_uString * pLanguage = NULL;
            rtl_uString * pCountry  = NULL;
            rtl_uString * pVariant  = NULL;

            size_t offset = 2;

            /* convert language code to unicode */
            rtl_string2UString( &pLanguage, locale, 2, RTL_TEXTENCODING_ASCII_US, OSTRING_TO_OUSTRING_CVTFLAGS );
            OSL_ASSERT(pLanguage != NULL);

            /* convert country code to unicode */
            if( len >= 5 && '_' == locale[2] )
            {
                rtl_string2UString( &pCountry, locale + 3, 2, RTL_TEXTENCODING_ASCII_US, OSTRING_TO_OUSTRING_CVTFLAGS );
                OSL_ASSERT(pCountry != NULL);
                offset = 5;
            }

            /* convert variant code to unicode - do not rely on "." as delimiter */
            if( len > offset ) {
                rtl_string2UString( &pVariant, locale + offset, len - offset, RTL_TEXTENCODING_ASCII_US, OSTRING_TO_OUSTRING_CVTFLAGS );
                OSL_ASSERT(pVariant != NULL);
            }

            rtl_Locale * ret =  rtl_locale_register( pLanguage->buffer, pCountry ? pCountry->buffer : c_locale + 1, pVariant ? pVariant->buffer : c_locale + 1 );

            if (pVariant) rtl_uString_release(pVariant);
            if (pCountry) rtl_uString_release(pCountry);
            if (pLanguage) rtl_uString_release(pLanguage);

            return ret;
        }
        else
            return rtl_locale_register( c_locale, c_locale + 1, c_locale + 1 );
    }

    return NULL;
}

/*
 * This implementation of osl_getTextEncodingFromLocale maps
 * from the ISO language codes.
 */

const _pair _full_locale_list[] = {
    { "ja_JP.eucJP",  RTL_TEXTENCODING_EUC_JP      },
    { "ja_JP.EUC",    RTL_TEXTENCODING_EUC_JP      },
    { "ko_KR.EUC",    RTL_TEXTENCODING_EUC_KR      },
    { "zh_CN.EUC",    RTL_TEXTENCODING_EUC_CN      },
    { "zh_TW.EUC",    RTL_TEXTENCODING_EUC_TW      }
};

const _pair _locale_extension_list[] = {
    { "big5",         RTL_TEXTENCODING_BIG5        },
    { "big5hk",       RTL_TEXTENCODING_BIG5_HKSCS  },
    { "gb18030",      RTL_TEXTENCODING_GB_18030    },
    { "euc",          RTL_TEXTENCODING_EUC_JP      },
    { "iso8859-1",    RTL_TEXTENCODING_ISO_8859_1  },
    { "iso8859-10",   RTL_TEXTENCODING_ISO_8859_10 },
    { "iso8859-13",   RTL_TEXTENCODING_ISO_8859_13 },
    { "iso8859-14",   RTL_TEXTENCODING_ISO_8859_14 },
    { "iso8859-15",   RTL_TEXTENCODING_ISO_8859_15 },
    { "iso8859-2",    RTL_TEXTENCODING_ISO_8859_2  },
    { "iso8859-3",    RTL_TEXTENCODING_ISO_8859_3  },
    { "iso8859-4",    RTL_TEXTENCODING_ISO_8859_4  },
    { "iso8859-5",    RTL_TEXTENCODING_ISO_8859_5  },
    { "iso8859-6",    RTL_TEXTENCODING_ISO_8859_6  },
    { "iso8859-7",    RTL_TEXTENCODING_ISO_8859_7  },
    { "iso8859-8",    RTL_TEXTENCODING_ISO_8859_8  },
    { "iso8859-9",    RTL_TEXTENCODING_ISO_8859_9  },
    { "koi8-r",       RTL_TEXTENCODING_KOI8_R      },
    { "koi8-u",       RTL_TEXTENCODING_KOI8_U      },
    { "pck",          RTL_TEXTENCODING_MS_932      },
#if (0)
    { "sun_eu_greek", RTL_TEXTENCODING_DONTKNOW    },
#endif
    { "utf-16",       RTL_TEXTENCODING_UNICODE     },
    { "utf-7",        RTL_TEXTENCODING_UTF7        },
    { "utf-8",        RTL_TEXTENCODING_UTF8        }
};

const _pair _iso_language_list[] = {
    { "af",  RTL_TEXTENCODING_ISO_8859_1 },
    { "ar",  RTL_TEXTENCODING_ISO_8859_6 },
    { "az",  RTL_TEXTENCODING_ISO_8859_9 },
    { "be",  RTL_TEXTENCODING_ISO_8859_5 },
    { "bg",  RTL_TEXTENCODING_ISO_8859_5 },
    { "ca",  RTL_TEXTENCODING_ISO_8859_1 },
    { "cs",  RTL_TEXTENCODING_ISO_8859_2 },
    { "da",  RTL_TEXTENCODING_ISO_8859_1 },
    { "de",  RTL_TEXTENCODING_ISO_8859_1 },
    { "el",  RTL_TEXTENCODING_ISO_8859_7 },
    { "en",  RTL_TEXTENCODING_ISO_8859_1 },
    { "es",  RTL_TEXTENCODING_ISO_8859_1 },
    { "et",  RTL_TEXTENCODING_ISO_8859_4 },
    { "eu",  RTL_TEXTENCODING_ISO_8859_1 },
    { "fa",  RTL_TEXTENCODING_ISO_8859_6 },
    { "fi",  RTL_TEXTENCODING_ISO_8859_1 },
    { "fo",  RTL_TEXTENCODING_ISO_8859_1 },
    { "fr",  RTL_TEXTENCODING_ISO_8859_1 },
    { "gr",  RTL_TEXTENCODING_ISO_8859_7 },
    { "he",  RTL_TEXTENCODING_ISO_8859_8 },
    { "hi",  RTL_TEXTENCODING_DONTKNOW },
    { "hr",  RTL_TEXTENCODING_ISO_8859_2 },
    { "hu",  RTL_TEXTENCODING_ISO_8859_2 },
    { "hy",  RTL_TEXTENCODING_DONTKNOW },
    { "id",  RTL_TEXTENCODING_ISO_8859_1 },
    { "is",  RTL_TEXTENCODING_ISO_8859_1 },
    { "it",  RTL_TEXTENCODING_ISO_8859_1 },
    { "iw",  RTL_TEXTENCODING_ISO_8859_8 },
    { "ja",  RTL_TEXTENCODING_EUC_JP },
    { "ka",  RTL_TEXTENCODING_DONTKNOW },
    { "kk",  RTL_TEXTENCODING_ISO_8859_5 },
    { "ko",  RTL_TEXTENCODING_EUC_KR },
    { "lt",  RTL_TEXTENCODING_ISO_8859_4 },
    { "lv",  RTL_TEXTENCODING_ISO_8859_4 },
    { "mk",  RTL_TEXTENCODING_ISO_8859_5 },
    { "mr",  RTL_TEXTENCODING_DONTKNOW },
    { "ms",  RTL_TEXTENCODING_ISO_8859_1 },
    { "nl",  RTL_TEXTENCODING_ISO_8859_1 },
    { "no",  RTL_TEXTENCODING_ISO_8859_1 },
    { "pl",  RTL_TEXTENCODING_ISO_8859_2 },
    { "pt",  RTL_TEXTENCODING_ISO_8859_1 },
    { "ro",  RTL_TEXTENCODING_ISO_8859_2 },
    { "ru",  RTL_TEXTENCODING_ISO_8859_5 },
    { "sa",  RTL_TEXTENCODING_DONTKNOW },
    { "sk",  RTL_TEXTENCODING_ISO_8859_2 },
    { "sl",  RTL_TEXTENCODING_ISO_8859_2 },
    { "sq",  RTL_TEXTENCODING_ISO_8859_2 },
    { "sv",  RTL_TEXTENCODING_ISO_8859_1 },
    { "sw",  RTL_TEXTENCODING_ISO_8859_1 },
    { "ta",  RTL_TEXTENCODING_DONTKNOW },
    { "th",  RTL_TEXTENCODING_DONTKNOW },
    { "tr",  RTL_TEXTENCODING_ISO_8859_9 },
    { "tt",  RTL_TEXTENCODING_ISO_8859_5 },
    { "uk",  RTL_TEXTENCODING_ISO_8859_5 },
    { "ur",  RTL_TEXTENCODING_ISO_8859_6 },
    { "uz",  RTL_TEXTENCODING_ISO_8859_9 },
    { "vi",  RTL_TEXTENCODING_DONTKNOW },
    { "zh",  RTL_TEXTENCODING_BIG5 }
};

/*****************************************************************************
 return the text encoding corresponding to the given locale
 *****************************************************************************/

rtl_TextEncoding osl_getTextEncodingFromLocale( rtl_Locale * pLocale )
{
    const _pair *language = 0;
    char locale_buf[64] = "";
    char *cp;

    WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
        "Please contact technical support and report above informations.\n\n",
        "Critical error: osl_getTextEncodingFromLocale",
        0, MB_ERROR | MB_OK | MB_MOVEABLE);

    /* default to process locale if pLocale == NULL */
    if( NULL == pLocale )
        osl_getProcessLocale( &pLocale );

    /* convert rtl_Locale to locale string */
    if( _compose_locale( pLocale, locale_buf, 64 ) )
    {
        /* check special handling list (EUC) first */
        const unsigned int members = sizeof( _full_locale_list ) / sizeof( _pair );
        language = _pair_search( locale_buf, _full_locale_list, members);

        if( NULL == language )
        {
            /*
             *  check if there is a charset qualifier at the end of the given locale string
             *  e.g. de.ISO8859-15 or de.ISO8859-15@euro which strongly indicates what
             *  charset to use
             */
            cp = strrchr( locale_buf, '.' );

            if( NULL != cp )
            {
                const unsigned int members = sizeof( _locale_extension_list ) / sizeof( _pair );
                language = _pair_search( cp + 1, _locale_extension_list, members);
            }
        }

        /* use iso language code to determine the charset */
        if( NULL == language )
        {
            const unsigned int members = sizeof( _iso_language_list ) / sizeof( _pair );

            /* iso lang codes have 2 charaters */
            locale_buf[2] = '\0';

            language = _pair_search( locale_buf, _iso_language_list, members);
        }
    }

    /* a matching item in our list provides a mapping from codeset to
     * rtl-codeset */
    if ( language != NULL )
        return language->value;

    return RTL_TEXTENCODING_DONTKNOW;
}

/*****************************************************************************
 return the current process locale
 *****************************************************************************/

void _imp_getProcessLocale( rtl_Locale ** ppLocale )
{
    /* simulate behavior off setlocale */
    char * locale = getenv( "LC_ALL" );

    if( NULL == locale )
        locale = getenv( "LC_CTYPE" );

    if( NULL == locale )
        locale = getenv( "LANG" );

    if( NULL == locale )
        locale = "C";

    *ppLocale = _parse_locale( locale );
}

/*****************************************************************************
 set the current process locale
 *****************************************************************************/

int _imp_setProcessLocale( rtl_Locale * pLocale )
{
#ifdef IRIX
    char env_buf[80];
#endif
    char locale_buf[64];

    /* convert rtl_Locale to locale string */
    if( NULL != _compose_locale( pLocale, locale_buf, 64 ) )
    {
        /* only change env vars that exist already */
        if( getenv( "LC_ALL" ) ) {
#if defined( IRIX )
            snprintf(env_buf, sizeof(env_buf), "LC_ALL=%s", locale_buf);
            env_buf[sizeof(env_buf)] = '\0';
            putenv(env_buf);
#elif defined( FREEBSD ) || defined( NETBSD ) || defined( MACOSX ) || defined( __EMX__ )
            setenv( "LC_ALL", locale_buf, 1);
#else
            setenv( "LC_ALL", locale_buf );
#endif
        }

        if( getenv( "LC_CTYPE" ) ) {
#if defined( IRIX )
            snprintf(env_buf, sizeof(env_buf), "LC_CTYPE=%s", locale_buf);
            env_buf[sizeof(env_buf)] = '\0';
            putenv(env_buf);
#elif defined( FREEBSD ) || defined( NETBSD ) || defined( MACOSX ) || defined( __EMX__ )
            setenv("LC_CTYPE", locale_buf, 1 );
#else
            setenv( "LC_CTYPE", locale_buf );
#endif
        }

        if( getenv( "LANG" ) ) {
#if defined( IRIX )
            snprintf(env_buf, sizeof(env_buf), "LANG=%s", locale_buf);
            env_buf[sizeof(env_buf)] = '\0';
            putenv(env_buf);
#elif defined( FREEBSD ) || defined( NETBSD ) || defined( MACOSX ) || defined( __EMX__ )
            setenv("LC_CTYPE", locale_buf, 1 );
#else
            setenv( "LANG", locale_buf );
#endif
        }
    }

    return 0;
}


