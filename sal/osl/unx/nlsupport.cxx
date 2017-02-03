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

#include <osl/nlsupport.h>
#include <osl/diagnose.h>
#include <osl/process.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include "nlsupport.hxx"

#if defined(LINUX) || defined(__sun) || defined(NETBSD) || \
    defined(FREEBSD) || defined(MACOSX)  || defined(IOS) || defined(OPENBSD) || \
    defined(DRAGONFLY)
#include <pthread.h>
#if !defined(MACOSX) && !defined(IOS)
#include <locale.h>
#include <langinfo.h>
#else
#include <osl/module.h>
#include <osl/thread.h>
#endif  /* !MACOSX && !IOS */
#endif  /* LINUX || __sun || NETBSD || MACOSX || IOS */

#if defined(MACOSX) || defined(IOS)
#include "system.hxx"
#endif

#include <string.h>

namespace {

struct Pair {
    const char              *key;
    const rtl_TextEncoding   value;
};

}

/*****************************************************************************
 compare function for binary search
 *****************************************************************************/

static int
pair_compare (const char *key, const Pair *pair)
{
    int result = rtl_str_compareIgnoreAsciiCase( key, pair->key );
    return result;
}

/*****************************************************************************
 binary search on encoding tables
 *****************************************************************************/

static const Pair*
pair_search (const char *key, const Pair *base, unsigned int member )
{
    unsigned int lower = 0;
    unsigned int upper = member;

    /* check for validity of input */
    if ( (key == nullptr) || (base == nullptr) || (member == 0) )
        return nullptr;

    /* binary search */
    while ( lower < upper )
    {
        const unsigned int current = (lower + upper) / 2;
        const int comparison = pair_compare( key, base + current );
        if (comparison < 0)
            upper = current;
        else if (comparison > 0)
            lower = current + 1;
        else
            return base + current;
    }

    return nullptr;
}

/*****************************************************************************
 convert rtl_Locale to locale string
 *****************************************************************************/

static char * compose_locale( rtl_Locale * pLocale, char * buffer, size_t n )
{
    /* check if a valid locale is specified */
    if( pLocale && pLocale->Language &&
            (pLocale->Language->length == 2 || pLocale->Language->length == 3) )
    {
        size_t offset = 0;

        /* convert language code to ascii */
        {
            rtl_String *pLanguage = nullptr;

            rtl_uString2String( &pLanguage,
                pLocale->Language->buffer, pLocale->Language->length,
                RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS );

            if( sal::static_int_cast<sal_uInt32>(pLanguage->length) < n )
            {
                strcpy( buffer, pLanguage->buffer );
                offset = pLanguage->length;
            }

            rtl_string_release( pLanguage );
        }

        /* convert country code to ascii */
        if( pLocale->Country && (pLocale->Country->length == 2) )
        {
            rtl_String *pCountry = nullptr;

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
            ( sal::static_int_cast<sal_uInt32>(pLocale->Variant->length) < n - 6 ) )
        {
            rtl_String *pVariant = nullptr;

            rtl_uString2String( &pVariant,
                pLocale->Variant->buffer, pLocale->Variant->length,
                RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS );

            if( offset + pVariant->length + 1 < n )
            {
                strcpy( buffer + offset, pVariant->buffer );
            }

            rtl_string_release( pVariant );
        }

        return buffer;
    }

    return nullptr;
}

/*****************************************************************************
 convert locale string to rtl_Locale
 *****************************************************************************/

static rtl_Locale * parse_locale( const char * locale )
{
    static sal_Unicode c_locale[2] = { (sal_Unicode) 'C', 0 };

    /* check if locale contains a valid string */
    if( locale )
    {
        size_t len = strlen( locale );

        if( len >= 2 )
        {
            rtl_uString * pLanguage = nullptr;
            rtl_uString * pCountry  = nullptr;
            rtl_uString * pVariant  = nullptr;

            size_t offset = 2;

            rtl_Locale * ret;

            /* language is a two or three letter code */
            if( (len > 3 && locale[3] == '_') || (len == 3 && locale[2] != '_') )
                offset = 3;

            /* convert language code to unicode */
            rtl_string2UString( &pLanguage, locale, offset, RTL_TEXTENCODING_ASCII_US, OSTRING_TO_OUSTRING_CVTFLAGS );
            OSL_ASSERT(pLanguage != nullptr);

            /* convert country code to unicode */
            if( len >= offset+3 && locale[offset] == '_' )
            {
                rtl_string2UString( &pCountry, locale + offset + 1, 2, RTL_TEXTENCODING_ASCII_US, OSTRING_TO_OUSTRING_CVTFLAGS );
                OSL_ASSERT(pCountry != nullptr);
                offset += 3;
            }

            /* convert variant code to unicode - do not rely on "." as delimiter */
            if( len > offset ) {
                rtl_string2UString( &pVariant, locale + offset, len - offset, RTL_TEXTENCODING_ASCII_US, OSTRING_TO_OUSTRING_CVTFLAGS );
                OSL_ASSERT(pVariant != nullptr);
            }

            ret =  rtl_locale_register( pLanguage->buffer, pCountry ? pCountry->buffer : c_locale + 1, pVariant ? pVariant->buffer : c_locale + 1 );

            if (pVariant) rtl_uString_release(pVariant);
            if (pCountry) rtl_uString_release(pCountry);
            if (pLanguage) rtl_uString_release(pLanguage);

            return ret;
        }
        else
            return rtl_locale_register( c_locale, c_locale + 1, c_locale + 1 );
    }

    return nullptr;
}

#if defined(LINUX) || defined(__sun) || defined(NETBSD) || \
    defined(FREEBSD) || defined(OPENBSD) || defined(DRAGONFLY)

/*
 * This implementation of osl_getTextEncodingFromLocale maps
 * from nl_langinfo(CODESET) to rtl_textencoding defines.
 * nl_langinfo() is supported only on Linux, Solaris,
 * >= NetBSD 1.6 and >= FreeBSD 4.4
 *
 * This routine is SLOW because of the setlocale call, so
 * grab the result and cache it.
 *
 * XXX this code has the usual mt problems aligned with setlocale() XXX
 */

#ifdef LINUX
#if !defined(CODESET)
#define CODESET _NL_CTYPE_CODESET_NAME
#endif
#endif

/*
 * _nl_language_list[] is an array list of supported encodings. Because
 * we are using a binary search, the list has to be in ascending order.
 * We are comparing the encodings case insensitiv, so the list has
 * to be completely upper- , or lowercase.
 */

#if defined(__sun)

/* The values in the below list can be obtained with a script like
 *  #!/bin/sh
 *  for i in `locale -a`; do
 *    LC_ALL=$i locale -k code_set_name
 *  done
 */
static const Pair nl_language_list[] = {
    { "5601",           RTL_TEXTENCODING_EUC_KR         }, /* ko_KR.EUC */
    { "646",            RTL_TEXTENCODING_ISO_8859_1     }, /* fake: ASCII_US */
    { "ANSI-1251",      RTL_TEXTENCODING_MS_1251        }, /* ru_RU.ANSI1251 */
    { "BIG5",           RTL_TEXTENCODING_BIG5           }, /* zh_CN.BIG5 */
    { "BIG5-HKSCS",     RTL_TEXTENCODING_BIG5_HKSCS     }, /* zh_CN.BIG5HK */
    { "CNS11643",       RTL_TEXTENCODING_EUC_TW         }, /* zh_TW.EUC */
    { "EUCJP",          RTL_TEXTENCODING_EUC_JP         }, /* ja_JP.eucjp */
    { "GB18030",        RTL_TEXTENCODING_GB_18030       }, /* zh_CN.GB18030 */
    { "GB2312",         RTL_TEXTENCODING_GB_2312        }, /* zh_CN */
    { "GBK",            RTL_TEXTENCODING_GBK            }, /* zh_CN.GBK */
    { "ISO8859-1",      RTL_TEXTENCODING_ISO_8859_1     },
    { "ISO8859-10",     RTL_TEXTENCODING_ISO_8859_10    },
    { "ISO8859-13",     RTL_TEXTENCODING_ISO_8859_13    }, /* lt_LT lv_LV */
    { "ISO8859-14",     RTL_TEXTENCODING_ISO_8859_14    },
    { "ISO8859-15",     RTL_TEXTENCODING_ISO_8859_15    },
    { "ISO8859-2",      RTL_TEXTENCODING_ISO_8859_2     },
    { "ISO8859-3",      RTL_TEXTENCODING_ISO_8859_3     },
    { "ISO8859-4",      RTL_TEXTENCODING_ISO_8859_4     },
    { "ISO8859-5",      RTL_TEXTENCODING_ISO_8859_5     },
    { "ISO8859-6",      RTL_TEXTENCODING_ISO_8859_6     },
    { "ISO8859-7",      RTL_TEXTENCODING_ISO_8859_7     },
    { "ISO8859-8",      RTL_TEXTENCODING_ISO_8859_8     },
    { "ISO8859-9",      RTL_TEXTENCODING_ISO_8859_9     },
    { "KOI8-R",         RTL_TEXTENCODING_KOI8_R         },
    { "KOI8-U",         RTL_TEXTENCODING_KOI8_U         },
    { "PCK",            RTL_TEXTENCODING_MS_932         },
    { "SUN_EU_GREEK",   RTL_TEXTENCODING_ISO_8859_7     }, /* 8859-7 + Euro */
    { "TIS620.2533",    RTL_TEXTENCODING_MS_874         }, /* th_TH.TIS620 */
    { "UTF-8",          RTL_TEXTENCODING_UTF8           }
};

/* XXX MS-874 is an extension to tis620, so this is not
 * really equivalent */

#elif defined(LINUX)

static const Pair nl_language_list[] = {
    { "ANSI_X3.110-1983",           RTL_TEXTENCODING_DONTKNOW   },  /* ISO-IR-99 NAPLPS */
    { "ANSI_X3.4-1968",             RTL_TEXTENCODING_ISO_8859_1 },  /* fake: ASCII_US */
    { "ASMO_449",                   RTL_TEXTENCODING_DONTKNOW },    /* ISO_9036 ARABIC7 */
    { "BALTIC",                     RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-179 */
    { "BIG5",                       RTL_TEXTENCODING_BIG5 },        /* locale: zh_TW */
    { "BIG5-HKSCS",                 RTL_TEXTENCODING_BIG5_HKSCS },  /* locale: zh_CN.BIG5HK */
    { "BIG5HKSCS",                  RTL_TEXTENCODING_BIG5_HKSCS },  /* deprecated */
    { "BS_4730",                    RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-4 ISO646-GB */
    { "BS_VIEWDATA",                RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-47 */
    { "CP1250",                     RTL_TEXTENCODING_MS_1250 },     /* MS-EE */
    { "CP1251",                     RTL_TEXTENCODING_MS_1251 },     /* MS-CYRL */
    { "CP1252",                     RTL_TEXTENCODING_MS_1252 },     /* MS-ANSI */
    { "CP1253",                     RTL_TEXTENCODING_MS_1253 },     /* MS-GREEK */
    { "CP1254",                     RTL_TEXTENCODING_MS_1254 },     /* MS-TURK */
    { "CP1255",                     RTL_TEXTENCODING_MS_1255 },     /* MS-HEBR */
    { "CP1256",                     RTL_TEXTENCODING_MS_1256 },     /* MS-ARAB */
    { "CP1257",                     RTL_TEXTENCODING_MS_1257 },     /* WINBALTRIM */
    { "CSA_Z243.4-1985-1",          RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-121 */
    { "CSA_Z243.4-1985-2",          RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-122 CSA7-2 */
    { "CSA_Z243.4-1985-GR",         RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-123 */
    { "CSN_369103",                 RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-139 */
    { "CWI",                        RTL_TEXTENCODING_DONTKNOW },    /* CWI-2 CP-HU */
    { "DEC-MCS",                    RTL_TEXTENCODING_DONTKNOW },    /* DEC */
    { "DIN_66003",                  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-21 */
    { "DS_2089",                    RTL_TEXTENCODING_DONTKNOW },    /* DS2089 ISO646-DK */
    { "EBCDIC-AT-DE",               RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-AT-DE-A",             RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-CA-FR",               RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-DK-NO",               RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-DK-NO-A",             RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-ES",                  RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-ES-A",                RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-ES-S",                RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-FI-SE",               RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-FI-SE-A",             RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-FR",                  RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-IS-FRISS",            RTL_TEXTENCODING_DONTKNOW },    /*  FRISS */
    { "EBCDIC-IT",                  RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-PT",                  RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-UK",                  RTL_TEXTENCODING_DONTKNOW },
    { "EBCDIC-US",                  RTL_TEXTENCODING_DONTKNOW },
    { "ECMA-CYRILLIC",              RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-111 */
    { "ES",                         RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-17 */
    { "ES2",                        RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-85 */
    { "EUC-JP",                     RTL_TEXTENCODING_EUC_JP },      /* locale: ja_JP.eucjp */
    { "EUC-KR",                     RTL_TEXTENCODING_EUC_KR },      /* locale: ko_KR.euckr */
    { "EUC-TW",                     RTL_TEXTENCODING_EUC_TW },      /* locale: zh_TW.euctw */
    { "GB18030",                    RTL_TEXTENCODING_GB_18030 },    /* locale: zh_CN.gb18030 */
    { "GB2312",                     RTL_TEXTENCODING_GB_2312 },     /* locale: zh_CN */
    { "GB_1988-80",                 RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-57 */
    { "GBK",                        RTL_TEXTENCODING_GBK },         /* locale: zh_CN.GBK */
    { "GOST_19768-74",              RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-153 */
    { "GREEK-CCITT",                RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-150 */
    { "GREEK7",                     RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-88 */
    { "GREEK7-OLD",                 RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-18 */
    { "HP-ROMAN8",                  RTL_TEXTENCODING_DONTKNOW },    /* ROMAN8 R8 */
    { "IBM037",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-[US|CA|WT] */
    { "IBM038",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-INT CP038 */
    { "IBM1004",                    RTL_TEXTENCODING_DONTKNOW },    /* CP1004 OS2LATIN1 */
    { "IBM1026",                    RTL_TEXTENCODING_DONTKNOW },    /* CP1026 1026 */
    { "IBM1047",                    RTL_TEXTENCODING_DONTKNOW },    /* CP1047 1047 */
    { "IBM256",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-INT1 */
    { "IBM273",                     RTL_TEXTENCODING_DONTKNOW },    /* CP273 */
    { "IBM274",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-BE CP274 */
    { "IBM275",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-BR CP275 */
    { "IBM277",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CP-[DK|NO] */
    { "IBM278",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CP-[FISE]*/
    { "IBM280",                     RTL_TEXTENCODING_DONTKNOW },    /* CP280 EBCDIC-CP-IT*/
    { "IBM281",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-JP-E CP281 */
    { "IBM284",                     RTL_TEXTENCODING_DONTKNOW },    /* CP284 EBCDIC-CP-ES */
    { "IBM285",                     RTL_TEXTENCODING_DONTKNOW },    /* CP285 EBCDIC-CP-GB */
    { "IBM290",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-JP-KANA */
    { "IBM297",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CP-FR */
    { "IBM420",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CP-AR1 */
    { "IBM423",                     RTL_TEXTENCODING_DONTKNOW },    /* CP423 EBCDIC-CP-GR */
    { "IBM424",                     RTL_TEXTENCODING_DONTKNOW },    /* CP424 EBCDIC-CP-HE */
    { "IBM437",                     RTL_TEXTENCODING_IBM_437 },     /* CP437 437 */
    { "IBM500",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CP-[BE|CH] */
    { "IBM850",                     RTL_TEXTENCODING_IBM_850 },     /* CP850 850 */
    { "IBM851",                     RTL_TEXTENCODING_DONTKNOW },    /* CP851 851 */
    { "IBM852",                     RTL_TEXTENCODING_IBM_852 },     /* CP852 852 */
    { "IBM855",                     RTL_TEXTENCODING_IBM_855 },     /* CP855 855 */
    { "IBM857",                     RTL_TEXTENCODING_IBM_857 },     /* CP857 857 */
    { "IBM860",                     RTL_TEXTENCODING_IBM_860 },     /* CP860 860 */
    { "IBM861",                     RTL_TEXTENCODING_IBM_861 },     /* CP861 861 CP-IS */
    { "IBM862",                     RTL_TEXTENCODING_IBM_862 },     /* CP862 862 */
    { "IBM863",                     RTL_TEXTENCODING_IBM_863 },     /* CP863 863 */
    { "IBM864",                     RTL_TEXTENCODING_IBM_864 },     /* CP864 */
    { "IBM865",                     RTL_TEXTENCODING_IBM_865 },     /* CP865 865 */
    { "IBM866",                     RTL_TEXTENCODING_IBM_866 },     /* CP866 866 */
    { "IBM868",                     RTL_TEXTENCODING_DONTKNOW },    /* CP868 CP-AR */
    { "IBM869",                     RTL_TEXTENCODING_IBM_869 },     /* CP869 869 CP-GR */
    { "IBM870",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-[ROECE|YU] */
    { "IBM871",                     RTL_TEXTENCODING_DONTKNOW },    /* CP871 EBCDIC-CP-IS */
    { "IBM875",                     RTL_TEXTENCODING_DONTKNOW },    /* CP875 EBCDIC-GREEK */
    { "IBM880",                     RTL_TEXTENCODING_DONTKNOW },    /* EBCDIC-CYRILLIC */
    { "IBM891",                     RTL_TEXTENCODING_DONTKNOW },    /* CP891 */
    { "IBM903",                     RTL_TEXTENCODING_DONTKNOW },    /* CP903 */
    { "IBM904",                     RTL_TEXTENCODING_DONTKNOW },    /* CP904 904 */
    { "IBM905",                     RTL_TEXTENCODING_DONTKNOW },    /* CP905 EBCDIC-CP-TR */
    { "IBM918",                     RTL_TEXTENCODING_DONTKNOW },    /* CP918 EBCDIC-AR2 */
    { "IEC_P27-1",                  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-143 */
    { "INIS",                       RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-49 */
    { "INIS-8",                     RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-50 */
    { "INIS-CYRILLIC",              RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-51 */
    { "INVARIANT",                  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-170 */
    { "ISO-8859-1",                 RTL_TEXTENCODING_ISO_8859_1 },  /* ISO-IR-100 CP819 */
    { "ISO-8859-10",                RTL_TEXTENCODING_ISO_8859_10 }, /* ISO-IR-157 LATIN6 */
    { "ISO-8859-13",                RTL_TEXTENCODING_ISO_8859_13 }, /* ISO-IR-179 LATIN7 */
    { "ISO-8859-14",                RTL_TEXTENCODING_ISO_8859_14 }, /* LATIN8 L8 */
    { "ISO-8859-15",                RTL_TEXTENCODING_ISO_8859_15 },
    { "ISO-8859-2",                 RTL_TEXTENCODING_ISO_8859_2 },  /* LATIN2 L2 */
    { "ISO-8859-3",                 RTL_TEXTENCODING_ISO_8859_3 },  /* LATIN3 L3 */
    { "ISO-8859-4",                 RTL_TEXTENCODING_ISO_8859_4 },  /* LATIN4 L4 */
    { "ISO-8859-5",                 RTL_TEXTENCODING_ISO_8859_5 },  /* CYRILLIC */
    { "ISO-8859-6",                 RTL_TEXTENCODING_ISO_8859_6 },  /* ECMA-114 ARABIC */
    { "ISO-8859-7",                 RTL_TEXTENCODING_ISO_8859_7 },  /* ECMA-118 GREEK8 */
    { "ISO-8859-8",                 RTL_TEXTENCODING_ISO_8859_8 },  /* ISO_8859-8 HEBREW */
    { "ISO-8859-9",                 RTL_TEXTENCODING_ISO_8859_9 },  /* ISO_8859-9 LATIN5 */
    { "ISO-IR-90",                  RTL_TEXTENCODING_DONTKNOW },    /* ISO_6937-2:1983 */
    { "ISO_10367-BOX",              RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-155 */
    { "ISO_2033-1983",              RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-98 E13B */
    { "ISO_5427",                   RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-37 KOI-7 */
    { "ISO_5427-EXT",               RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-54  */
    { "ISO_5428",                   RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-55 */
    { "ISO_646.BASIC",              RTL_TEXTENCODING_ASCII_US },    /* REF */
    { "ISO_646.IRV",                RTL_TEXTENCODING_ASCII_US },    /* ISO-IR-2 IRV */
    { "ISO_646.IRV:1983",           RTL_TEXTENCODING_ISO_8859_1 },  /* fake: ASCII_US, used for "C" locale*/
    { "ISO_6937",                   RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-156 ISO6937*/
    { "ISO_6937-2-25",              RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-152 */
    { "ISO_6937-2-ADD",             RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-142 */
    { "ISO_8859-SUPP",              RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-154 */
    { "IT",                         RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-15  */
    { "JIS_C6220-1969-JP",          RTL_TEXTENCODING_DONTKNOW },    /* KATAKANA X0201-7 */
    { "JIS_C6220-1969-RO",          RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-14 */
    { "JIS_C6229-1984-A",           RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-91 */
    { "JIS_C6229-1984-B",           RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-92 */
    { "JIS_C6229-1984-B-ADD",       RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-93 */
    { "JIS_C6229-1984-HAND",        RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-94 */
    { "JIS_C6229-1984-HAND-ADD",    RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-95 */
    { "JIS_C6229-1984-KANA",        RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-96 */
    { "JIS_X0201",                  RTL_TEXTENCODING_DONTKNOW },    /* X0201 */
    { "JUS_I.B1.002",               RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-141 */
    { "JUS_I.B1.003-MAC",           RTL_TEXTENCODING_DONTKNOW },    /* MACEDONIAN */
    { "JUS_I.B1.003-SERB",          RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-146 SERBIAN */
    { "KOI-8",                      RTL_TEXTENCODING_DONTKNOW },
    { "KOI8-R",                     RTL_TEXTENCODING_KOI8_R },
    { "KOI8-U",                     RTL_TEXTENCODING_KOI8_U },
    { "KSC5636",                    RTL_TEXTENCODING_DONTKNOW },    /* ISO646-KR */
    { "LATIN-GREEK",                RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-19 */
    { "LATIN-GREEK-1",              RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-27 */
    { "MAC-IS",                     RTL_TEXTENCODING_APPLE_ROMAN },
    { "MAC-UK",                     RTL_TEXTENCODING_APPLE_ROMAN },
    { "MACINTOSH",                  RTL_TEXTENCODING_APPLE_ROMAN }, /* MAC */
    { "MSZ_7795.3",                 RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-86 */
    { "NATS-DANO",                  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-9-1 */
    { "NATS-DANO-ADD",              RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-9-2 */
    { "NATS-SEFI",                  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-8-1 */
    { "NATS-SEFI-ADD",              RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-8-2 */
    { "NC_NC00-10",                 RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-151 */
    { "NEXTSTEP",                   RTL_TEXTENCODING_DONTKNOW },    /* NEXT */
    { "NF_Z_62-010",                RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-69 */
    { "NF_Z_62-010_(1973)",         RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-25 */
    { "NS_4551-1",                  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-60 */
    { "NS_4551-2",                  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-61 */
    { "PT",                         RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-16 */
    { "PT2",                        RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-84 */
    { "SAMI",                       RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-158 */
    { "SEN_850200_B",               RTL_TEXTENCODING_DONTKNOW },    /* ISO646-[FI|SE] */
    { "SEN_850200_C",               RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-11 */
    { "T.101-G2",                   RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-128 */
    { "T.61-7BIT",                  RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-102 */
    { "T.61-8BIT",                  RTL_TEXTENCODING_DONTKNOW },    /* T.61 ISO-IR-103 */
    { "TIS-620",                    RTL_TEXTENCODING_MS_874 },     /* locale: th_TH */
    { "UTF-8",                      RTL_TEXTENCODING_UTF8 },        /* ISO-10646/UTF-8 */
    { "VIDEOTEX-SUPPL",             RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-70 */
    { "WIN-SAMI-2",                 RTL_TEXTENCODING_DONTKNOW }     /* WS2 */
};

#elif defined(FREEBSD) || defined(DRAGONFLY)

static const Pair nl_language_list[] = {
    { "ASCII",         RTL_TEXTENCODING_ASCII_US       }, /* US-ASCII */
    { "BIG5",          RTL_TEXTENCODING_BIG5           }, /* China - Traditional Chinese */
    { "CP1251",        RTL_TEXTENCODING_MS_1251        }, /* MS-CYRL */
    { "CP866",         RTL_TEXTENCODING_IBM_866        }, /* CP866 866 */
    { "EUCCN",         RTL_TEXTENCODING_EUC_CN         }, /* China - Simplified Chinese */
    { "EUCJP",         RTL_TEXTENCODING_EUC_JP         }, /* Japan */
    { "EUCKR",         RTL_TEXTENCODING_EUC_KR         }, /* Korea */
    { "ISO8859-1",     RTL_TEXTENCODING_ISO_8859_1     }, /* Western */
    { "ISO8859-15",    RTL_TEXTENCODING_ISO_8859_15    }, /* Western Updated (w/Euro sign) */
    { "ISO8859-2",     RTL_TEXTENCODING_ISO_8859_2     }, /* Central European */
    { "ISO8859-4",     RTL_TEXTENCODING_ISO_8859_4     }, /* LATIN4 L4 */
    { "ISO8859-5",     RTL_TEXTENCODING_ISO_8859_5     }, /* Cyrillic */
    { "ISO8859-7",     RTL_TEXTENCODING_ISO_8859_7     }, /* Greek */
    { "ISO8859-9",     RTL_TEXTENCODING_ISO_8859_9     }, /* Turkish */
    { "KOI8-R",        RTL_TEXTENCODING_KOI8_R         }, /* KOI8-R */
    { "KOI8-U",        RTL_TEXTENCODING_KOI8_U         }, /* KOI8-U */
    { "SJIS",          RTL_TEXTENCODING_SHIFT_JIS      }, /* Japan */
    { "US-ASCII",      RTL_TEXTENCODING_ASCII_US       }, /* US-ASCII */
    { "UTF-8",         RTL_TEXTENCODING_UTF8           }  /* ISO-10646/UTF-8 */
};

#elif defined(NETBSD)

static const Pair nl_language_list[] = {
    { "ASCII",         RTL_TEXTENCODING_ASCII_US       }, /* US-ASCII */
    { "BIG5",          RTL_TEXTENCODING_BIG5           }, /* China - Traditional Chinese */
    { "Big5",          RTL_TEXTENCODING_BIG5           }, /* China - Traditional Chinese */
    { "Big5-HKSCS",    RTL_TEXTENCODING_BIG5_HKSCS     }, /* locale: zh_CN.BIG5HK */
    { "Big5HKSCS",     RTL_TEXTENCODING_BIG5_HKSCS     }, /* deprecated */
    { "CP1251",        RTL_TEXTENCODING_MS_1251        }, /* MS-CYRL */
    { "CP866",         RTL_TEXTENCODING_IBM_866        }, /* CP866 866 */
    { "CTEXT",         RTL_TEXTENCODING_ASCII_US       }, /* US-ASCII */
    { "eucCN",         RTL_TEXTENCODING_EUC_CN         }, /* China - Simplified Chinese */
    { "eucJP",         RTL_TEXTENCODING_EUC_JP         }, /* Japan */
    { "eucKR",         RTL_TEXTENCODING_EUC_KR         }, /* Korea */
    { "eucTW",         RTL_TEXTENCODING_EUC_TW         }, /* China - Traditional Chinese */
    { "GB18030",       RTL_TEXTENCODING_GB_18030       }, /* locale: zh_CN.gb18030 */
    { "GB2312",        RTL_TEXTENCODING_GB_2312        }, /* locale: zh_CN */
    { "ISO-2022-JP",   RTL_TEXTENCODING_DONTKNOW       }, /* */
    { "ISO-2022-JP-2", RTL_TEXTENCODING_DONTKNOW       }, /* */
    { "ISO8859-1",     RTL_TEXTENCODING_ISO_8859_1     }, /* Western */
    { "ISO8859-13",    RTL_TEXTENCODING_ISO_8859_13    }, /* ISO-IR-179 LATIN7 */
    { "ISO8859-15",    RTL_TEXTENCODING_ISO_8859_15    }, /* Western Updated (w/Euro sign) */
    { "ISO8859-2",     RTL_TEXTENCODING_ISO_8859_2     }, /* Central European */
    { "ISO8859-4",     RTL_TEXTENCODING_ISO_8859_4     }, /* LATIN4 L4 */
    { "ISO8859-5",     RTL_TEXTENCODING_ISO_8859_5     }, /* Cyrillic */
    { "ISO8859-7",     RTL_TEXTENCODING_ISO_8859_7     }, /* Greek */
    { "ISO8859-9",     RTL_TEXTENCODING_ISO_8859_9     }, /* Turkish */
    { "KOI8-R",        RTL_TEXTENCODING_KOI8_R         }, /* KOI8-R */
    { "KOI8-U",        RTL_TEXTENCODING_KOI8_U         }, /* KOI8-U */
    { "PT154",         RTL_TEXTENCODING_PT154          }, /* */
    { "SJIS",          RTL_TEXTENCODING_SHIFT_JIS      }, /* Japan */
    { "US-ASCII",      RTL_TEXTENCODING_ASCII_US       }, /* US-ASCII */
    { "UTF-8",         RTL_TEXTENCODING_UTF8           }  /* ISO-10646/UTF-8 */
};

#elif defined(OPENBSD)

static const Pair nl_language_list[] = {
    { "ASCII",         RTL_TEXTENCODING_ASCII_US       }, /* US-ASCII */
    { "BIG5",          RTL_TEXTENCODING_BIG5           }, /* China - Traditional Chinese */
    { "CP1251",        RTL_TEXTENCODING_MS_1251        }, /* MS-CYRL */
    { "CP866",         RTL_TEXTENCODING_IBM_866        }, /* CP866 866 */
    { "EUCCN",         RTL_TEXTENCODING_EUC_CN         }, /* China - Simplified Chinese */
    { "EUCJP",         RTL_TEXTENCODING_EUC_JP         }, /* Japan */
    { "EUCKR",         RTL_TEXTENCODING_EUC_KR         }, /* Korea */
    { "ISO8859-1",     RTL_TEXTENCODING_ISO_8859_1     }, /* Western */
    { "ISO8859-15",    RTL_TEXTENCODING_ISO_8859_15    }, /* Western Updated (w/Euro sign) */
    { "ISO8859-2",     RTL_TEXTENCODING_ISO_8859_2     }, /* Central European */
    { "ISO8859-4",     RTL_TEXTENCODING_ISO_8859_4     }, /* LATIN4 L4 */
    { "ISO8859-5",     RTL_TEXTENCODING_ISO_8859_5     }, /* Cyrillic */
    { "ISO8859-7",     RTL_TEXTENCODING_ISO_8859_7     }, /* Greek */
    { "ISO8859-9",     RTL_TEXTENCODING_ISO_8859_9     }, /* Turkish */
    { "KOI8-R",        RTL_TEXTENCODING_KOI8_R         }, /* KOI8-R */
    { "KOI8-U",        RTL_TEXTENCODING_KOI8_U         }, /* KOI8-U */
    { "SJIS",          RTL_TEXTENCODING_SHIFT_JIS      }, /* Japan */
    { "US-ASCII",      RTL_TEXTENCODING_ASCII_US       }, /* US-ASCII */
    { "UTF-8",         RTL_TEXTENCODING_UTF8           }  /* ISO-10646/UTF-8 */
};

#endif /* ifdef __sun LINUX FREEBSD NETBSD OPENBSD */

static pthread_mutex_t aLocalMutex = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************************************
 return the text encoding corresponding to the given locale
 *****************************************************************************/

rtl_TextEncoding osl_getTextEncodingFromLocale( rtl_Locale * pLocale )
{
    const Pair *language=nullptr;

    char  locale_buf[64] = "";
    char  codeset_buf[64];

    char *ctype_locale = nullptr;
    char *codeset      = nullptr;

    /* default to process locale if pLocale == NULL */
    if( nullptr == pLocale )
        osl_getProcessLocale( &pLocale );

    /* convert rtl_Locale to locale string */
    compose_locale( pLocale, locale_buf, 64 );

    /* basic thread safeness */
    pthread_mutex_lock( &aLocalMutex );

    /* remember the charset as indicated by the LC_CTYPE locale */
    ctype_locale = setlocale( LC_CTYPE, nullptr );

    /* set the desired LC_CTYPE locale */
    if( nullptr == setlocale( LC_CTYPE, locale_buf ) )
    {
        pthread_mutex_unlock(&aLocalMutex);
        return RTL_TEXTENCODING_DONTKNOW;
    }

    /* get the charset as indicated by the LC_CTYPE locale */
#if defined(NETBSD) && !defined(CODESET)
    codeset = NULL;
#else
    codeset = nl_langinfo( CODESET );
#endif

    if ( codeset != nullptr )
    {
        /* get codeset into mt save memory */
        strncpy( codeset_buf, codeset, sizeof(codeset_buf) );
        codeset_buf[sizeof(codeset_buf) - 1] = 0;
        codeset = codeset_buf;
    }

    /* restore the original value of locale */
    if ( ctype_locale != nullptr )
        setlocale( LC_CTYPE, ctype_locale );

    pthread_mutex_unlock( &aLocalMutex );

    /* search the codeset in our language list */
    if ( codeset != nullptr )
    {
        language = pair_search (codeset, nl_language_list, SAL_N_ELEMENTS( nl_language_list ) );
    }

    OSL_ASSERT( language && ( RTL_TEXTENCODING_DONTKNOW != language->value ) );

    /* a matching item in our list provides a mapping from codeset to
     * rtl-codeset */
    if ( language != nullptr )
        return language->value;

    return RTL_TEXTENCODING_DONTKNOW;
}

/*****************************************************************************
 return the current process locale
 *****************************************************************************/

void imp_getProcessLocale( rtl_Locale ** ppLocale )
{
    char * locale;

    /* basic thread safeness */
    pthread_mutex_lock( &aLocalMutex );

    /* set the locale defined by the env vars */
    locale = setlocale( LC_CTYPE, "" );

    /* fallback to the current locale */
    if( nullptr == locale )
        locale = setlocale( LC_CTYPE, nullptr );

    /* return the LC_CTYPE locale */
    *ppLocale = parse_locale( locale );

    pthread_mutex_unlock( &aLocalMutex );
}

/*****************************************************************************
 set the current process locale
 *****************************************************************************/

int imp_setProcessLocale( rtl_Locale * pLocale )
{
    char  locale_buf[64] = "";
    int   ret = 0;

    /* convert rtl_Locale to locale string */
    compose_locale( pLocale, locale_buf, 64 );

    /* basic thread safeness */
    pthread_mutex_lock( &aLocalMutex );

    /* try to set LC_ALL locale */
    if( nullptr == setlocale( LC_ALL, locale_buf ) )
        ret = -1;

    pthread_mutex_unlock( &aLocalMutex );
    return ret;
}

#else /* ifdef LINUX || __sun || MACOSX || NETBSD */

/*
 * This implementation of osl_getTextEncodingFromLocale maps
 * from the ISO language codes.
 */

static const Pair full_locale_list[] = {
    { "ja_JP.eucJP",  RTL_TEXTENCODING_EUC_JP      },
    { "ja_JP.EUC",    RTL_TEXTENCODING_EUC_JP      },
    { "ko_KR.EUC",    RTL_TEXTENCODING_EUC_KR      },
    { "zh_CN.EUC",    RTL_TEXTENCODING_EUC_CN      },
    { "zh_TW.EUC",    RTL_TEXTENCODING_EUC_TW      }
};

static const Pair locale_extension_list[] = {
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

static const Pair iso_language_list[] = {
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
    const Pair *language = nullptr;
    char locale_buf[64] = "";

    /* default to process locale if pLocale == NULL */
    if( nullptr == pLocale )
        osl_getProcessLocale( &pLocale );

    /* convert rtl_Locale to locale string */
    if( compose_locale( pLocale, locale_buf, 64 ) )
    {
        /* check special handling list (EUC) first */
        language = pair_search( locale_buf, full_locale_list, SAL_N_ELEMENTS( full_locale_list ) );

        if( nullptr == language )
        {
            /*
             *  check if there is a charset qualifier at the end of the given locale string
             *  e.g. de.ISO8859-15 or de.ISO8859-15@euro which strongly indicates what
             *  charset to use
             */
            char* cp = strrchr( locale_buf, '.' );

            if( nullptr != cp )
            {
                language = pair_search( cp + 1, locale_extension_list, SAL_N_ELEMENTS( locale_extension_list ) );
            }
        }

        /* use iso language code to determine the charset */
        if( nullptr == language )
        {
            /* iso lang codes have 2 characters */
            locale_buf[2] = '\0';

            language = pair_search( locale_buf, iso_language_list, SAL_N_ELEMENTS( iso_language_list ) );
        }
    }

    /* a matching item in our list provides a mapping from codeset to
     * rtl-codeset */
    if ( language != nullptr )
        return language->value;

    return RTL_TEXTENCODING_DONTKNOW;
}

#if defined(MACOSX) || defined(IOS)

/*****************************************************************************
 return the current process locale
 *****************************************************************************/

void imp_getProcessLocale( rtl_Locale ** ppLocale )
{
    rtl::OUString loc16(macosx_getLocale());
    rtl::OString locale;
    if (!loc16.convertToString(
            &locale, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        SAL_INFO("sal.osl", "Cannot convert \"" << loc16 << "\" to UTF-8");
    }

    /* handle the case where OS specific method of finding locale fails */
    if ( locale.isEmpty() )
    {
        /* simulate behavior of setlocale */
        locale = getenv( "LC_ALL" );

        if( locale.isEmpty() )
            locale = getenv( "LC_CTYPE" );

        if( locale.isEmpty() )
            locale = getenv( "LANG" );

        if( locale.isEmpty() )
            locale = "C";
    }

    /* return the locale */
    *ppLocale = parse_locale( locale.getStr() );

    setenv( "LC_ALL", locale.getStr(), 1);
    setenv("LC_CTYPE", locale.getStr(), 1 );
    setenv("LANG", locale.getStr(), 1 );
}
#else
/*****************************************************************************
 return the current process locale
 *****************************************************************************/

void imp_getProcessLocale( rtl_Locale ** ppLocale )
{
#ifdef ANDROID
    /* No locale environment variables on Android, so why even bother
     * with getenv().
     */
   char const * locale = "en-US.UTF-8";
#else
    /* simulate behavior off setlocale */
    char * locale = getenv( "LC_ALL" );

    if( NULL == locale )
        locale = getenv( "LC_CTYPE" );

    if( NULL == locale )
        locale = getenv( "LANG" );

    if( NULL == locale )
        locale = "C";

#endif
    *ppLocale = parse_locale( locale );
}
#endif

/*****************************************************************************
 set the current process locale
 *****************************************************************************/

static int
imp_setenv (const char* name, const char* value)
{
    return setenv (name, value, 1);
}

int imp_setProcessLocale( rtl_Locale * pLocale )
{
    char locale_buf[64];

    /* convert rtl_Locale to locale string */
    if( nullptr != compose_locale( pLocale, locale_buf, 64 ) )
    {
        /* only change env vars that exist already */
        if( getenv( "LC_ALL" ) )
            imp_setenv( "LC_ALL", locale_buf );

        if( getenv( "LC_CTYPE" ) )
            imp_setenv("LC_CTYPE", locale_buf );

        if( getenv( "LANG" ) )
            imp_setenv( "LANG", locale_buf );
    }

    return 0;
}

#endif /* ifdef LINUX || __sun || MACOSX || NETBSD || AIX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
