/*************************************************************************
 *
 *  $RCSfile: nlsupport.c,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:21:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <osl/nlsupport.h>
#include <osl/diagnose.h>
#include <osl/process.h>

#if defined(LINUX) || defined(SOLARIS) || defined(IRIX) || defined(NETBSD) || defined(FREEBSD) || defined(MACOSX)
#include <pthread.h>
#ifndef MACOSX
 #include <locale.h>
 #include <langinfo.h>
#else
#include <osl/module.h>
#include <osl/thread.h>
#include <unxmacxp_protos.h>
#endif  /* !MACOSX */
#endif  /* LINUX || SOLARIS || IRIX || NETBSD || MACOSX */

/*****************************************************************************/
/* typedefs
/*****************************************************************************/


typedef struct {
    const char              *key;
    const rtl_TextEncoding   value;
} _pair;


/*****************************************************************************/
/* compare function for binary search
/*****************************************************************************/

static int
_pair_compare (const char *key, const _pair *pair)
{
    int result = rtl_str_compareIgnoreAsciiCase( key, pair->key );
    return result;
}

/*****************************************************************************/
/* binary search on encoding tables
/*****************************************************************************/

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


/*****************************************************************************/
/* convert rtl_Locale to locale string
/*****************************************************************************/

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

/*****************************************************************************/
/* convert locale string to rtl_Locale
/*****************************************************************************/

static rtl_Locale * _parse_locale( const char * locale )
{
    static sal_Unicode c_locale[2] = { (sal_Unicode) 'C', 0 };

    rtl_uString * pLanguage = NULL;
    rtl_uString * pCountry  = NULL;
    rtl_uString * pVariant  = NULL;

    /* check if locale contains a valid string */
    if( locale )
    {
        size_t len = strlen( locale );

        if( len >= 2 )
        {
            size_t offset = 2;

            /* convert language code to unicode */
            rtl_string2UString( &pLanguage, locale, 2, RTL_TEXTENCODING_ASCII_US, OSTRING_TO_OUSTRING_CVTFLAGS );

            /* convert country code to unicode */
            if( len >= 5 && '_' == locale[2] )
            {
                rtl_string2UString( &pCountry, locale + 3, 2, RTL_TEXTENCODING_ASCII_US, OSTRING_TO_OUSTRING_CVTFLAGS );
                offset = 5;
            }

            /* convert variant code to unicode - do not rely on "." as delimiter */
            if( len > offset )
                rtl_string2UString( &pVariant, locale + offset, len - offset, RTL_TEXTENCODING_ASCII_US, OSTRING_TO_OUSTRING_CVTFLAGS );

            return rtl_locale_register( pLanguage->buffer, pCountry ? pCountry->buffer : c_locale + 1, pVariant ? pVariant->buffer : c_locale + 1 );
        }
        else
            return rtl_locale_register( c_locale, c_locale + 1, c_locale + 1 );
    }

    return NULL;
}

#if defined(LINUX) || defined(SOLARIS) || defined(IRIX) || defined(NETBSD) || defined(FREEBSD) || defined(MACOSX)

/*
 * This implementation of osl_getTextEncodingFromLocale maps
 * from nl_langinfo(CODESET) to rtl_textencoding defines.
 * nl_langinfo() is supported only on Linux and Solaris.
 * nl_langinfo() is supported only on Linux, Solaris and IRIX,
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
 * to be completly upper- , or lowercase.
 */

#if defined(SOLARIS)

const _pair _nl_language_list[] = {
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
#elif defined(IRIX)

const _pair _nl_language_list[] = {
   { "big5",        RTL_TEXTENCODING_BIG5       }, /* China - Traditional Chinese */
   { "eucCN",       RTL_TEXTENCODING_EUC_CN     }, /* China */
   { "eucgbk",      RTL_TEXTENCODING_DONTKNOW   }, /* China - Simplified Chinese */
   { "eucJP",       RTL_TEXTENCODING_EUC_JP     }, /* Japan */
   { "eucKR",       RTL_TEXTENCODING_EUC_KR     }, /* Korea */
   { "eucTW",       RTL_TEXTENCODING_EUC_TW     }, /* Taiwan - Traditional Chinese */
   { "gbk",     RTL_TEXTENCODING_GBK        }, /* China - Simplified Chinese */
   { "ISO8859-1",   RTL_TEXTENCODING_ISO_8859_1 }, /* Western */
   { "ISO8859-2",   RTL_TEXTENCODING_ISO_8859_2     }, /* Central European */
   { "ISO8859-5",   RTL_TEXTENCODING_ISO_8859_5     }, /* Cyrillic */
   { "ISO8859-7",   RTL_TEXTENCODING_ISO_8859_7     }, /* Greek */
   { "ISO8859-9",   RTL_TEXTENCODING_ISO_8859_9     }, /* Turkish */
   { "ISO8859-15",  RTL_TEXTENCODING_ISO_8859_15    }, /* Western Updated (w/Euro sign) */
   { "sjis",        RTL_TEXTENCODING_SHIFT_JIS  } /* Japan */
};

#elif defined(LINUX) || defined(NETBSD) || defined(MACOSX)

const _pair _nl_language_list[] = {
    { "ANSI_X3.110-1983",           RTL_TEXTENCODING_DONTKNOW   },  /* ISO-IR-99 NAPLPS */
    { "ANSI_X3.4-1968",             RTL_TEXTENCODING_ISO_8859_1 },  /* fake: ASCII_US */
    { "ASMO_449",                   RTL_TEXTENCODING_DONTKNOW },    /* ISO_9036 ARABIC7 */
    { "BALTIC",                     RTL_TEXTENCODING_DONTKNOW },    /* ISO-IR-179 */
    { "BIG5",                       RTL_TEXTENCODING_BIG5 },        /* locale: zh_TW */
    { "BIG5-HKSCS",                 RTL_TEXTENCODING_BIG5_HKSCS },  /* locale: zh_CN.BIG5HK */
    { "BIG5HKSCS",                  RTL_TEXTENCODING_BIG5_HKSCS },  /* depricated */
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

#elif defined(FREEBSD)

const _pair _nl_language_list[] = {
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

const _pair _nl_language_list[] = {
    { "ASCII",         RTL_TEXTENCODING_ASCII_US       }, /* US-ASCII */
    { "BIG5",          RTL_TEXTENCODING_BIG5           }, /* China - Traditional Chinese */
    { "CP1251",        RTL_TEXTENCODING_MS_1251        }, /* MS-CYRL */
    { "CP866",         RTL_TEXTENCODING_IBM_866        }, /* CP866 866 */
    { "CTEXT",         RTL_TEXTENCODING_ASCII_US       }, /* US-ASCII */
    { "EUCCN",         RTL_TEXTENCODING_EUC_CN         }, /* China - Simplified Chinese */
    { "EUCJP",         RTL_TEXTENCODING_EUC_JP         }, /* Japan */
    { "EUCKR",         RTL_TEXTENCODING_EUC_KR         }, /* Korea */
    { "EUCTW",         RTL_TEXTENCODING_EUC_TW         }, /* China - Traditional Chinese */
    { "ISO-2022-JP",   RTL_TEXTENCODING_DONTKNOW       }, /* */
    { "ISO-2022-JP-2", RTL_TEXTENCODING_DONTKNOW       }, /* */
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

#endif /* ifdef SOLARIS IRIX LINUX FREEBSD NETBSD MACOSX */

#ifdef MACOSX
/* MacOS X has some special names for some of its encodings.  These encoding names are
 * returned by CFStringConvertEncodingToIANACharSetName() when given a CFTextEncoding
 * argument from one of the ones in CFStringEncodingExt.h in the CoreFoundation framework.
 */
const _pair _macxp_language_list[] = {
    { "MACINTOSH",              RTL_TEXTENCODING_ISO_8859_1 },      /* kCFStringEncodingMacRoman */
    { "X-MAC-JAPANESE",         RTL_TEXTENCODING_EUC_JP },          /* kCFStringEncodingMacJapanese */
    { "X-MAC-TRAD-CHINESE",     RTL_TEXTENCODING_APPLE_CHINTRAD },      /* kCFStringEncodingMacChineseTrad */
    { "X-MAC-KOREAN",           RTL_TEXTENCODING_EUC_KR },          /* kCFStringEncodingMacKorean */
    { "X-MAC-ARABIC",           RTL_TEXTENCODING_APPLE_ARABIC },        /* kCFStringEncodingMacArabic */
    { "X-MAC-HEBREW",           RTL_TEXTENCODING_APPLE_HEBREW },        /* kCFStringEncodingMacHebrew */
    { "X-MAC-GREEK",            RTL_TEXTENCODING_APPLE_GREEK },     /* kCFStringEncodingMacGreek */
    { "X-MAC-CYRILLIC",         RTL_TEXTENCODING_APPLE_CYRILLIC },      /* kCFStringEncodingMacCyrillic */
    { "X-MAC-DEVANAGARI",       RTL_TEXTENCODING_APPLE_DEVANAGARI },    /* kCFStringEncodingMacDevanagari */
    { "X-MAC-GURMUKHI",         RTL_TEXTENCODING_APPLE_GURMUKHI },      /* kCFStringEncodingMacGurmukhi */
    { "X-MAC-GUJARATI",         RTL_TEXTENCODING_APPLE_GUJARATI },      /* kCFStringEncodingMacGujarati */
    { "X-MAC-ORIYA",            RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacOriya */
    { "X-MAC-BENGALI",          RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacBengali */
    { "X-MAC-TAMIL",            RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacTamil */
    { "X-MAC-TELUGU",           RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacTelugu */
    { "X-MAC-KANNADA",          RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacKannada */
    { "X-MAC-MALAYALAM",        RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacMalayalam */
    { "X-MAC-SINHALESE",        RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacSinhalese */
    { "X-MAC-BURMESE",          RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacBurmese */
    { "X-MAC-KHMER",            RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacKhmer */
    { "X-MAC-THAI",         RTL_TEXTENCODING_APPLE_THAI },      /* kCFStringEncodingMacThai */
    { "X-MAC-LAOTIAN",          RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacLaotian */
    { "X-MAC-GEORGIAN",         RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacGeorgian */
    { "X-MAC-ARMENIAN",         RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacArmenian */
    { "X-MAC-SIMP-CHINESE",     RTL_TEXTENCODING_APPLE_CHINSIMP },      /* kCFStringEncodingMacChineseSimp */
    { "X-MAC-TIBETAN",          RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacTibetan */
    { "X-MAC-MONGOLIAN",        RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacMongolian */
    { "X-MAC-ETHIOPIC",         RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacEthiopic */
    { "X-MAC-CENTRALEURROMAN",  RTL_TEXTENCODING_APPLE_CENTEURO },      /* kCFStringEncodingMacCentralEurRoman */
    { "X-MAC-VIETNAMESE",       RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacVietnamese */
    { "X-MAC-EXTARABIC",        RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacExtArabic */
    { "X-MAC-SYMBOL",           RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacSymbol */
    { "X-MAC-DINGBATS",         RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacDingbats */
    { "X-MAC-TURKISH",          RTL_TEXTENCODING_APPLE_TURKISH },       /* kCFStringEncodingMacTurkish */
    { "X-MAC-CROATIAN",         RTL_TEXTENCODING_APPLE_CROATIAN },      /* kCFStringEncodingMacCroatian */
    { "X-MAC-ICELANDIC",        RTL_TEXTENCODING_APPLE_ICELAND },       /* kCFStringEncodingMacIcelandic */
    { "X-MAC-ROMANIAN",         RTL_TEXTENCODING_APPLE_ROMANIAN },      /* kCFStringEncodingMacRomanian */
    { "UNICODE-2-0",            RTL_TEXTENCODING_DONTKNOW },            /*  */
    { "X-MAC-FARSI",            RTL_TEXTENCODING_APPLE_FARSI },     /* kCFStringEncodingMacFarsi */
    { "X-MAC-UKRAINIAN",        RTL_TEXTENCODING_APPLE_UKRAINIAN }, /* kCFStringEncodingMacUkrainian */
    { "X-MAC-VT100",            RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacVT100 */
    { "macintosh",              RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingMacHFS */
    { "UTF-16BE",               RTL_TEXTENCODING_UNICODE },         /* kCFStringEncodingUnicode */
    { "UNICODE-1-1",            RTL_TEXTENCODING_DONTKNOW },            /*  */
    { "UNICODE-1-1",            RTL_TEXTENCODING_DONTKNOW },            /*  */
    { "UNICODE-2-0",            RTL_TEXTENCODING_DONTKNOW },            /*  */
    { "csUnicode",              RTL_TEXTENCODING_DONTKNOW },            /*  */
    { "CP437",              RTL_TEXTENCODING_IBM_437 },         /* kCFStringEncodingDOSLatinUS */
    { "CP737",              RTL_TEXTENCODING_IBM_737 },         /* kCFStringEncodingDOSGreek */
    { "CP775",              RTL_TEXTENCODING_IBM_775 },         /* kCFStringEncodingDOSBalticRim */
    { "CP850",              RTL_TEXTENCODING_IBM_850 },         /* kCFStringEncodingDOSLatin1 */
    { "CP852",              RTL_TEXTENCODING_IBM_852 },         /* kCFStringEncodingDOSGreek1 */
    { "CP857",              RTL_TEXTENCODING_IBM_857 },         /* kCFStringEncodingDOSTurkish */
    { "CP861",              RTL_TEXTENCODING_IBM_861 },         /* kCFStringEncodingDOSIcelandic */
    { "cp864",              RTL_TEXTENCODING_IBM_864 },         /* kCFStringEncodingDOSArabic */
    { "CP866",              RTL_TEXTENCODING_IBM_866 },         /* kCFStringEncodingDOSRussian */
    { "CP874",              RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingDOSThai */
    { "CP932",              RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingDOSJapanese */
    { "CP936",              RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingDOSChineseSimplif */
    { "CP949",              RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingDOSKorean */
    { "CP950",              RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingDOSChineseTrad */
    { "WINDOWS-1252",           RTL_TEXTENCODING_MS_1252 },         /* kCFStringEncodingWindowsLatin1 */
    { "WINDOWS-1250",           RTL_TEXTENCODING_MS_1250 },         /* kCFStringEncodingWindowsLatin2 */
    { "WINDOWS-1251",           RTL_TEXTENCODING_MS_1251 },         /* kCFStringEncodingWindowsCyrillic */
    { "WINDOWS-1253",           RTL_TEXTENCODING_MS_1253 },         /* kCFStringEncodingWindowsGreek */
    { "WINDOWS-1254",           RTL_TEXTENCODING_MS_1254 },         /* kCFStringEncodingWindowsLatin5 */
    { "WINDOWS-1255",           RTL_TEXTENCODING_MS_1255 },         /* kCFStringEncodingWindowsHebrew */
    { "WINDOWS-1256",           RTL_TEXTENCODING_MS_1256 },         /* kCFStringEncodingWindowsArabic */
    { "WINDOWS-1257",           RTL_TEXTENCODING_MS_1257 },         /* kCFStringEncodingWindowsBalticRim */
    { "WINDOWS-1258",           RTL_TEXTENCODING_MS_1258 },         /* kCFStringEncodingWindowsVietnamese */
    { "US-ASCII",               RTL_TEXTENCODING_ASCII_US },            /* kCFStringEncodingASCII */
    { "JIS_C6226-1983",         RTL_TEXTENCODING_GB_2312 },         /* kCFStringEncodingJIS_X0208_90 */
    { "csISO58GB231280",        RTL_TEXTENCODING_GB_2312 },         /* kCFStringEncodingGB_2312_80 */
    { "X-GBK",              RTL_TEXTENCODING_GBK },             /* kCFStringEncodingGBK_95 */
    { "csKSC56011987",          RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingKSC_5601_87 */
    { "ISO-2022-JP",            RTL_TEXTENCODING_ISO_2022_JP },     /* kCFStringEncodingISO_2022_JP */
    { "ISO-2022-CN",            RTL_TEXTENCODING_ISO_2022_CN },     /* kCFStringEncodingISO_2022_CN */
    { "ISO-2022-CN-EXT",        RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingISO_2022_CN_EXT */
    { "ISO-2022-KR",            RTL_TEXTENCODING_ISO_2022_KR },     /* kCFStringEncodingISO_2022_KR */
    { "EUC-JP",             RTL_TEXTENCODING_EUC_JP },          /* kCFStringEncodingEUC_JP */
    { "EUC-CN",             RTL_TEXTENCODING_EUC_CN },          /* kCFStringEncodingEUC_CN */
    { "EUC-TW",             RTL_TEXTENCODING_EUC_TW },          /* kCFStringEncodingEUC_TW */
    { "EUC-KR",             RTL_TEXTENCODING_EUC_KR },          /* kCFStringEncodingEUC_KR */
    { "SHIFT_JIS",              RTL_TEXTENCODING_SHIFT_JIS },           /* kCFStringEncodingShiftJIS */
    { "KOI8-R",             RTL_TEXTENCODING_KOI8_R },          /* kCFStringEncodingKOI8_R */
    { "X-MAC-ROMAN-LATIN1",     RTL_TEXTENCODING_APPLE_ROMAN },     /* kCFStringEncodingMacRomanLatin1 */
    { "HZ-GB-2312",         RTL_TEXTENCODING_GB_2312 },         /* kCFStringEncodingHZ_GB_2312 */
    { "Big5-HKSCS",         RTL_TEXTENCODING_BIG5_HKSCS },      /* kCFStringEncodingBig5_HKSCS_1999 */
    { "X-NEXTSTEP",         RTL_TEXTENCODING_DONTKNOW },            /* kCFStringEncodingNextStepLatin */
    { "cp037",              RTL_TEXTENCODING_DONTKNOW }         /* kCFStringEncodingEBCDIC_CP037 */
/* ___________ Add more encodings here ___________ */
};
#endif  /* ifdef MACOSX */

static pthread_mutex_t aLocalMutex = PTHREAD_MUTEX_INITIALIZER;

#ifndef MACOSX

/*****************************************************************************/
/* return the text encoding corresponding to the given locale
/*****************************************************************************/

rtl_TextEncoding osl_getTextEncodingFromLocale( rtl_Locale * pLocale )
{
    const _pair *language=0;

    char  locale_buf[64] = "";
    char  codeset_buf[64];

    char *ctype_locale = 0;
    char *codeset      = 0;

    /* default to process locale if pLocale == NULL */
    if( NULL == pLocale )
        osl_getProcessLocale( &pLocale );

    /* convert rtl_Locale to locale string */
    _compose_locale( pLocale, locale_buf, 64 );

    /* basic thread safeness */
    pthread_mutex_lock( &aLocalMutex );

    /* remember the charset as indicated by the LC_CTYPE locale */
    ctype_locale = setlocale( LC_CTYPE, NULL );

    /* set the desired LC_CTYPE locale */
    if( NULL == setlocale( LC_CTYPE, locale_buf ) )
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

    if ( codeset != NULL )
    {
        /* get codeset into mt save memory */
        rtl_copyMemory( codeset_buf, codeset, sizeof(codeset_buf) );
        codeset = codeset_buf;
    }

    /* restore the original value of locale */
    if ( ctype_locale != NULL )
        setlocale( LC_CTYPE, ctype_locale );

    pthread_mutex_unlock( &aLocalMutex );

    /* search the codeset in our language list */
    if ( codeset != NULL )
    {
        const unsigned int members = sizeof(_nl_language_list) / sizeof(_pair);
        language = _pair_search (codeset, _nl_language_list, members);
    }

    OSL_ASSERT( language && ( RTL_TEXTENCODING_DONTKNOW != language->value ) );

    /* a matching item in our list provides a mapping from codeset to
     * rtl-codeset */
    if ( language != NULL )
        return language->value;

    return RTL_TEXTENCODING_DONTKNOW;
}

/*****************************************************************************/
/* return the current process locale
/*****************************************************************************/

void _imp_getProcessLocale( rtl_Locale ** ppLocale )
{
    char * locale;

    /* basic thread safeness */
    pthread_mutex_lock( &aLocalMutex );

    /* set the locale defined by the env vars */
    locale = setlocale( LC_CTYPE, "" );

    /* fallback to the current locale */
    if( NULL == locale )
        locale = setlocale( LC_CTYPE, NULL );

    /* return the LC_CTYPE locale */
    *ppLocale = _parse_locale( locale );

    pthread_mutex_unlock( &aLocalMutex );
}

/*****************************************************************************/
/* set the current process locale
/*****************************************************************************/

int _imp_setProcessLocale( rtl_Locale * pLocale )
{
    char  locale_buf[64] = "";
    int   ret = 0;

    /* convert rtl_Locale to locale string */
    _compose_locale( pLocale, locale_buf, 64 );

    /* basic thread safeness */
    pthread_mutex_lock( &aLocalMutex );

    /* try to set LC_ALL locale */
    if( NULL == setlocale( LC_ALL, locale_buf ) )
        ret = -1;

    pthread_mutex_unlock( &aLocalMutex );
    return ret;
}

#else   /* ifndef MACOSX */

/*
 * MacOS X specific Locale manipulation code
 */

#include <premac.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>

/* OS X locale discovery function from dylib */
int (*pGetOSXLocale)( char *, sal_uInt32 );
int (*pConvertTextEncodingToIANACharsetName)( char *, unsigned int, CFStringEncoding );

void macxp_ConvertCFEncodingToIANACharSetName( char *buffer, unsigned int bufferLen, CFStringEncoding cfEncoding );

/*****************************************************************************/
/* return the text encoding corresponding to the given locale
/*****************************************************************************/

rtl_TextEncoding osl_getTextEncodingFromLocale( rtl_Locale * pLocale )
{
    const _pair          *pLanguage = NULL;
    char                 sEncoding[ 64 ] = "";
    CFStringEncoding     sCFSysEncoding;
    const unsigned int   macxpListMembers = sizeof(_macxp_language_list) / sizeof(_pair);
    const unsigned int   otherListMembers = sizeof(_nl_language_list) / sizeof(_pair);
    unsigned int         isDarwin;
    unsigned int         majorVersion;
    unsigned int         minorVersion;
    unsigned int         minorMinorVersion;

    /* For retrieving the CoreFoundation text encoding, the process goes like
     * this:
     * 1) On MacOS X this will be the first language listed in the
     *     Language tab of the International preference pane.
     * 2) On Darwin 5 the value is currently hardwired to
     *     kCFStringEncodingMacRoman.
     */
    sCFSysEncoding = CFStringGetSystemEncoding();

    /* If running on OS X, attempt to convert text encoding to a charset name using CoreFoundation calls. */
    /* Must use osl_psz_xxxx variants of functions here because otherwise osl_loadModule() tries to get the
     * un-Unicode the string we pass to it, which results in this function being called again and again and
     * again, finally overflowing the stack.
     */
    macxp_getSystemVersion( &isDarwin, &majorVersion, &minorVersion, &minorMinorVersion );
    if( !isDarwin )
    {
        /* Load the text encoding conversion library if we are running on OS X */
        const sal_Char   *aTextEncConvertLibName          = "libsalextra_x11osx_mxp.dylib";
        const sal_Char   *aOSXTextEncConvertFunctionName  = "macxp_OSXConvertCFEncodingToIANACharSetName";
        oslModule         pTextEncCovertLib;
        void             *pFunc;
        int               err;

        pTextEncCovertLib = osl_psz_loadModule( aTextEncConvertLibName, SAL_LOADMODULE_DEFAULT );
        if( !pTextEncCovertLib )
        {
            fprintf( stderr, "Could not load the OS X text encoding coversion library! (%s)\n", aTextEncConvertLibName );
        }
        else
        {
            /* Grab a pointer to the text encoding conversion function and call it */
            pFunc = osl_psz_getSymbol( pTextEncCovertLib, aOSXTextEncConvertFunctionName );
            if( pFunc )
            {
                pConvertTextEncodingToIANACharsetName = ( int(*)(char *, unsigned int, CFStringEncoding) )( pFunc );
                err = (*pConvertTextEncodingToIANACharsetName)( sEncoding, 64, sCFSysEncoding );
            }
            else
                fprintf( stderr, "Could not grab pointer to the OS X text encoding coversion routine! (%s)\n", aOSXTextEncConvertFunctionName );
        }
        osl_unloadModule( pTextEncCovertLib );
    }

    if ( strlen(sEncoding) <= 0 )
    {
        /*
         * Darwin doesn't have a CFStringConvertEncodingToIANACharSetName(),
         * so we call our own replacement.  Also fall back to our replacement
         * if for some reason we can't load our OS X convert library.
         */
        macxp_ConvertCFEncodingToIANACharSetName( sEncoding, 64, sCFSysEncoding );
    }

    /* Try the Mac-specific list first, then non-Mac list (picks up MACINTOSH from non-Mac list) */
    pLanguage = _pair_search( sEncoding, _macxp_language_list, macxpListMembers );
    if ( pLanguage == NULL )
        pLanguage = _pair_search( sEncoding, _nl_language_list, otherListMembers );

    OSL_ASSERT( pLanguage && ( RTL_TEXTENCODING_DONTKNOW != pLanguage->value ) );

    /*
     * a matching item in our list provides a mapping from codeset to
     * rtl-codeset
      */
    if ( pLanguage != NULL  )
        return pLanguage->value;

    return RTL_TEXTENCODING_DONTKNOW;
}

/*****************************************************************************/
/* return the current process locale
/*****************************************************************************/

void _imp_getProcessLocale( rtl_Locale ** ppLocale )
{
    static const char *locale = NULL;

    /* basic thread safeness */
    pthread_mutex_lock( &aLocalMutex );

    /* Only fetch the locale once and cache it */
    if ( NULL == locale )
    {
        unsigned int    isDarwin;
        unsigned int    majorVersion;
        unsigned int    minorVersion;
        unsigned int    minorMinorVersion;

        /* If running on OS X, attempt to fetch the locale using CoreServices calls. */
        macxp_getSystemVersion( &isDarwin, &majorVersion, &minorVersion, &minorMinorVersion );
        if( !isDarwin )
        {
            /* Load the locale discovery library if we are running on OS X */
            const sal_Char   *aLocaleLibName             = "libsalextra_x11osx_mxp.dylib";
            const sal_Char   *aGetOSXLocaleFunctionName  = "macxp_getOSXLocale";
            oslModule         pLocaleLib;
            void             *pFunc;
            int               err;

            pLocaleLib = osl_psz_loadModule( aLocaleLibName, SAL_LOADMODULE_DEFAULT );
            if( pLocaleLib )
            {
                /* Grab a pointer to the locale function and call it */
                pFunc = osl_psz_getSymbol( pLocaleLib, aGetOSXLocaleFunctionName );
                if( pFunc )
                {
                    pGetOSXLocale = ( int(*)(char *, sal_uInt32) )( pFunc );
                    locale = (char *)malloc( 20 );
                    if ( locale )
                        err = (*pGetOSXLocale)( locale, 20 );
                    else
                        fprintf( stderr, "nlsupport.c:  locale allocation returned NULL!\n" );
                }
                else
                    fprintf( stderr, "Could not load the OS X locale discovery function! (%s)\n", aGetOSXLocaleFunctionName );
            }
            else
                fprintf( stderr, "Could not load the OS X locale discovery library! (%s)\n", aLocaleLibName );

            /* Let go of the module, we don't need it anymore */
            osl_unloadModule( pLocaleLib );
         }
    }

    /* handle the case where OS specific method of finding locale fails */
    if ( NULL == locale )
    {
        /* simulate behavior of setlocale */
        locale = getenv( "LC_ALL" );

        if( NULL == locale )
            locale = getenv( "LC_CTYPE" );

        if( NULL == locale )
            locale = getenv( "LANG" );

        if( NULL == locale )
            locale = "C";
    }

    /* return the locale */
    *ppLocale = _parse_locale( locale );
#ifdef DEBUG
    fprintf( stderr, "nlsupport.c:  _imp_getProcessLocale() returning %s as current locale.\n", locale );
#endif

    pthread_mutex_unlock( &aLocalMutex );

}

/*****************************************************************************/
/* set the current process locale
/*****************************************************************************/

int _imp_setProcessLocale( rtl_Locale * pLocale )
{
    /* There is not yet any way to set the locale of a process
     * on Darwin or MacOS X through BSD layer functions.
     *
     * On MacOS X locale and language are set by the user through
     * the International pane of the System Preferences application.
     *
     * FIXME when this can be done.
     */
    return 0;
}


/*****************************************************************************/
/* Get a charset name from a CoreFoundation text encoding number
/*****************************************************************************/
void macxp_ConvertCFEncodingToIANACharSetName( char *buffer, unsigned int bufferLen, CFStringEncoding cfEncoding )
{
    switch( cfEncoding )
    {
        case 0:     strncpy( buffer, "MACINTOSH", bufferLen-1 );                break;  /* kCFStringEncodingMacRoman */
        case 1:     strncpy( buffer, "X-MAC-JAPANESE", bufferLen-1 );           break;  /* kCFStringEncodingMacJapanese */
        case 2:     strncpy( buffer, "X-MAC-TRAD-CHINESE", bufferLen-1 );       break;  /* kCFStringEncodingMacChineseTrad */
        case 3:     strncpy( buffer, "X-MAC-KOREAN", bufferLen-1 );         break;  /* kCFStringEncodingMacKorean */
        case 4:     strncpy( buffer, "X-MAC-ARABIC", bufferLen-1 );         break;  /* kCFStringEncodingMacArabic */
        case 5:     strncpy( buffer, "X-MAC-HEBREW", bufferLen-1 );         break;  /* kCFStringEncodingMacHebrew */
        case 6:     strncpy( buffer, "X-MAC-GREEK", bufferLen-1 );          break;  /* kCFStringEncodingMacGreek */
        case 7:     strncpy( buffer, "X-MAC-CYRILLIC", bufferLen-1 );           break;  /* kCFStringEncodingMacCyrillic */
        case 9:     strncpy( buffer, "X-MAC-DEVANAGARI", bufferLen-1 );     break;  /* kCFStringEncodingMacDevanagari */
        case 10:        strncpy( buffer, "X-MAC-GURMUKHI", bufferLen-1 );           break;  /* kCFStringEncodingMacGurmukhi */
        case 11:        strncpy( buffer, "X-MAC-GUJARATI", bufferLen-1 );           break;  /* kCFStringEncodingMacGujarati */
        case 12:        strncpy( buffer, "X-MAC-ORIYA", bufferLen-1 );          break;  /* kCFStringEncodingMacOriya */
        case 13:        strncpy( buffer, "X-MAC-BENGALI", bufferLen-1 );            break;  /* kCFStringEncodingMacBengali */
        case 14:        strncpy( buffer, "X-MAC-TAMIL", bufferLen-1 );          break;  /* kCFStringEncodingMacTamil */
        case 15:        strncpy( buffer, "X-MAC-TELUGU", bufferLen-1 );         break;  /* kCFStringEncodingMacTamil */
        case 16:        strncpy( buffer, "X-MAC-KANNADA", bufferLen-1 );            break;  /* kCFStringEncodingMacKannada */
        case 17:        strncpy( buffer, "X-MAC-MALAYALAM", bufferLen-1 );      break;  /* kCFStringEncodingMacMalayalam */
        case 18:        strncpy( buffer, "X-MAC-SINHALESE", bufferLen-1 );      break;  /* kCFStringEncodingMacSinhalese */
        case 19:        strncpy( buffer, "X-MAC-BURMESE", bufferLen-1 );            break;  /* kCFStringEncodingMacBurmese */
        case 20:        strncpy( buffer, "X-MAC-KHMER", bufferLen-1 );          break;  /* kCFStringEncodingMacKhmer */
        case 21:        strncpy( buffer, "X-MAC-THAI", bufferLen-1 );           break;  /* kCFStringEncodingMacThai */
        case 22:        strncpy( buffer, "X-MAC-LAOTIAN", bufferLen-1 );            break;  /* kCFStringEncodingMacLaotian */
        case 23:        strncpy( buffer, "X-MAC-GEORGIAN", bufferLen-1 );           break;  /* kCFStringEncodingMacGeorgian */
        case 24:        strncpy( buffer, "X-MAC-ARMENIAN", bufferLen-1 );           break;  /* kCFStringEncodingMacArmenian */
        case 25:        strncpy( buffer, "X-MAC-SIMP-CHINESE", bufferLen-1 );       break;  /* kCFStringEncodingMacChineseSimp */
        case 26:        strncpy( buffer, "X-MAC-TIBETAN", bufferLen-1 );            break;  /* kCFStringEncodingMacTibetan */
        case 27:        strncpy( buffer, "X-MAC-MONGOLIAN", bufferLen-1 );      break;  /* kCFStringEncodingMacMongolian */
        case 28:        strncpy( buffer, "X-MAC-ETHIOPIC", bufferLen-1 );           break;  /* kCFStringEncodingMacEthiopic */
        case 29:        strncpy( buffer, "X-MAC-CENTRALEURROMAN", bufferLen-1 );    break;  /* kCFStringEncodingMacCentralEurRoman */
        case 30:        strncpy( buffer, "X-MAC-VIETNAMESE", bufferLen-1 );     break;  /* kCFStringEncodingMacVietnamese */
        case 31:        strncpy( buffer, "X-MAC-EXTARABIC", bufferLen-1 );      break;  /* kCFStringEncodingMacExtArabic */
        case 33:        strncpy( buffer, "X-MAC-SYMBOL", bufferLen-1 );         break;  /* kCFStringEncodingMacSymbol */
        case 34:        strncpy( buffer, "X-MAC-DINGBATS", bufferLen-1 );           break;  /* kCFStringEncodingMacDingbats */
        case 35:        strncpy( buffer, "X-MAC-TURKISH", bufferLen-1 );            break;  /* kCFStringEncodingMacTurkish */
        case 36:        strncpy( buffer, "X-MAC-CROATIAN", bufferLen-1 );           break;  /* kCFStringEncodingMacCroatian */
        case 37:        strncpy( buffer, "X-MAC-ICELANDIC", bufferLen-1 );      break;  /* kCFStringEncodingMacIcelandic */
        case 38:        strncpy( buffer, "X-MAC-ROMANIAN", bufferLen-1 );           break;  /* kCFStringEncodingMacRomanian */
        case 126:       strncpy( buffer, "UNICODE-2-0", bufferLen-1 );          break;  /*  */
        case 140:       strncpy( buffer, "X-MAC-FARSI", bufferLen-1 );          break;  /* kCFStringEncodingMacFarsi */
        case 152:       strncpy( buffer, "X-MAC-UKRAINIAN", bufferLen-1 );      break;  /* kCFStringEncodingMacUkrainian */
        case 252:       strncpy( buffer, "X-MAC-VT100", bufferLen-1 );          break;  /* kCFStringEncodingMacVT100 */
        case 255:       strncpy( buffer, "macintosh", bufferLen-1 );                break;  /* kCFStringEncodingMacHFS */
        case 256:       strncpy( buffer, "UTF-16BE", bufferLen-1 );             break;  /* kCFStringEncodingUnicode */
        case 257:       strncpy( buffer, "UNICODE-1-1", bufferLen-1 );          break;  /*  */
        case 258:       strncpy( buffer, "UNICODE-1-1", bufferLen-1 );          break;  /*  */
        case 259:       strncpy( buffer, "UNICODE-2-0", bufferLen-1 );          break;  /*  */
        case 260:       strncpy( buffer, "csUnicode", bufferLen-1 );                break;  /*  */
        case 513:       strncpy( buffer, "ISO-8859-1", bufferLen-1 );           break;  /* kCFStringEncodingISOLatin1 */
        case 514:       strncpy( buffer, "ISO-8859-2", bufferLen-1 );           break;  /* kCFStringEncodingISOLatin2 */
        case 515:       strncpy( buffer, "ISO-8859-3", bufferLen-1 );           break;  /* kCFStringEncodingISOLatin3 */
        case 516:       strncpy( buffer, "ISO-8859-4", bufferLen-1 );           break;  /* kCFStringEncodingISOLatin4 */
        case 517:       strncpy( buffer, "ISO-8859-5", bufferLen-1 );           break;  /* kCFStringEncodingISOLatinCyrillic */
        case 518:       strncpy( buffer, "ISO-8859-6", bufferLen-1 );           break;  /* kCFStringEncodingISOLatinArabic */
        case 519:       strncpy( buffer, "ISO-8859-7", bufferLen-1 );           break;  /* kCFStringEncodingISOLatinGreek */
        case 520:       strncpy( buffer, "ISO-8859-8", bufferLen-1 );           break;  /* kCFStringEncodingISOLatinHebrew */
        case 521:       strncpy( buffer, "ISO-8859-9", bufferLen-1 );           break;  /* kCFStringEncodingISOLatin5 */
        case 522:       strncpy( buffer, "ISO-8859-10", bufferLen-1 );          break;  /* kCFStringEncodingISOLatin6 */
        case 523:       strncpy( buffer, "ISO-8859-11", bufferLen-1 );          break;  /* kCFStringEncodingISOLatinThai */
        case 525:       strncpy( buffer, "ISO-8859-13", bufferLen-1 );          break;  /* kCFStringEncodingISOLatin7 */
        case 526:       strncpy( buffer, "ISO-8859-14", bufferLen-1 );          break;  /* kCFStringEncodingISOLatin8 */
        case 527:       strncpy( buffer, "ISO-8859-15", bufferLen-1 );          break;  /* kCFStringEncodingISOLatin9 */
        case 1024:  strncpy( buffer, "CP437", bufferLen-1 );                break;  /* kCFStringEncodingDOSLatinUS */
        case 1029:  strncpy( buffer, "CP737", bufferLen-1 );                break;  /* kCFStringEncodingDOSGreek */
        case 1030:  strncpy( buffer, "CP775", bufferLen-1 );                break;  /* kCFStringEncodingDOSBalticRim */
        case 1040:  strncpy( buffer, "CP850", bufferLen-1 );                break;  /* kCFStringEncodingDOSLatin1 */
        case 1042:  strncpy( buffer, "CP852", bufferLen-1 );                break;  /* kCFStringEncodingDOSGreek1 */
        case 1044:  strncpy( buffer, "CP857", bufferLen-1 );                break;  /* kCFStringEncodingDOSTurkish */
        case 1046:  strncpy( buffer, "CP861", bufferLen-1 );                break;  /* kCFStringEncodingDOSIcelandic */
        case 1049:  strncpy( buffer, "cp864", bufferLen-1 );                break;  /* kCFStringEncodingDOSArabic */
        case 1051:  strncpy( buffer, "CP866", bufferLen-1 );                break;  /* kCFStringEncodingDOSRussian */
        case 1053:  strncpy( buffer, "CP874", bufferLen-1 );                break;  /* kCFStringEncodingDOSThai */
        case 1056:  strncpy( buffer, "CP932", bufferLen-1 );                break;  /* kCFStringEncodingDOSJapanese */
        case 1057:  strncpy( buffer, "CP936", bufferLen-1 );                break;  /* kCFStringEncodingDOSChineseSimplif */
        case 1058:  strncpy( buffer, "CP949", bufferLen-1 );                break;  /* kCFStringEncodingDOSKorean */
        case 1059:  strncpy( buffer, "CP950", bufferLen-1 );                break;  /* kCFStringEncodingDOSChineseTrad */
        case 1280:  strncpy( buffer, "WINDOWS-1252", bufferLen-1 );         break;  /* kCFStringEncodingWindowsLatin1 */
        case 1281:  strncpy( buffer, "WINDOWS-1250", bufferLen-1 );         break;  /* kCFStringEncodingWindowsLatin2 */
        case 1282:  strncpy( buffer, "WINDOWS-1251", bufferLen-1 );         break;  /* kCFStringEncodingWindowsCyrillic */
        case 1283:  strncpy( buffer, "WINDOWS-1253", bufferLen-1 );         break;  /* kCFStringEncodingWindowsGreek */
        case 1284:  strncpy( buffer, "WINDOWS-1254", bufferLen-1 );         break;  /* kCFStringEncodingWindowsLatin5 */
        case 1285:  strncpy( buffer, "WINDOWS-1255", bufferLen-1 );         break;  /* kCFStringEncodingWindowsHebrew */
        case 1286:  strncpy( buffer, "WINDOWS-1256", bufferLen-1 );         break;  /* kCFStringEncodingWindowsArabic */
        case 1287:  strncpy( buffer, "WINDOWS-1257", bufferLen-1 );         break;  /* kCFStringEncodingWindowsBalticRim */
        case 1288:  strncpy( buffer, "WINDOWS-1258", bufferLen-1 );         break;  /* kCFStringEncodingWindowsVietnamese */
        case 1536:  strncpy( buffer, "US-ASCII", bufferLen-1 );             break;  /* kCFStringEncodingASCII */
        case 1570:  strncpy( buffer, "JIS_C6226-1983", bufferLen-1 );           break;  /* kCFStringEncodingJIS_X0208_90 */
        case 1584:  strncpy( buffer, "csISO58GB231280", bufferLen-1 );      break;  /* kCFStringEncodingGB_2312_80 */
        case 1585:  strncpy( buffer, "X-GBK", bufferLen-1 );                break;  /* kCFStringEncodingGBK_95 */
        case 1600:  strncpy( buffer, "csKSC56011987", bufferLen-1 );            break;  /* kCFStringEncodingKSC_5601_87 */
        case 2080:  strncpy( buffer, "ISO-2022-JP", bufferLen-1 );          break;  /* kCFStringEncodingISO_2022_JP */
        case 2096:  strncpy( buffer, "ISO-2022-CN", bufferLen-1 );          break;  /* kCFStringEncodingISO_2022_CN */
        case 2097:  strncpy( buffer, "ISO-2022-CN-EXT", bufferLen-1 );      break;  /* kCFStringEncodingISO_2022_CN_EXT */
        case 2112:  strncpy( buffer, "ISO-2022-KR", bufferLen-1 );          break;  /* kCFStringEncodingISO_2022_KR */
        case 2336:  strncpy( buffer, "EUC-JP", bufferLen-1 );               break;  /* kCFStringEncodingEUC_JP */
        case 2352:  strncpy( buffer, "EUC-CN", bufferLen-1 );               break;  /* kCFStringEncodingEUC_CN */
        case 2353:  strncpy( buffer, "EUC-TW", bufferLen-1 );               break;  /* kCFStringEncodingEUC_TW */
        case 2368:  strncpy( buffer, "EUC-KR", bufferLen-1 );               break;  /* kCFStringEncodingEUC_KR */
        case 2561:  strncpy( buffer, "SHIFT_JIS", bufferLen-1 );                break;  /* kCFStringEncodingShiftJIS */
        case 2562:  strncpy( buffer, "KOI8-R", bufferLen-1 );               break;  /* kCFStringEncodingKOI8_R */
        case 2563:  strncpy( buffer, "BIG5", bufferLen-1 );                 break;  /* kCFStringEncodingBig5 */
        case 2564:  strncpy( buffer, "X-MAC-ROMAN-LATIN1", bufferLen-1 );       break;  /* kCFStringEncodingMacRomanLatin1 */
        case 2565:  strncpy( buffer, "HZ-GB-2312", bufferLen-1 );           break;  /* kCFStringEncodingHZ_GB_2312 */
        case 2566:  strncpy( buffer, "Big5-HKSCS", bufferLen-1 );           break;  /* kCFStringEncodingBig5_HKSCS_1999 */
        case 2817:  strncpy( buffer, "X-NEXTSTEP", bufferLen-1 );           break;  /* kCFStringEncodingNextStepLatin */
        case 3074:  strncpy( buffer, "cp037", bufferLen-1 );                break;  /* kCFStringEncodingEBCDIC_CP037 */
        default:        strncpy( buffer, "", bufferLen-1 );                 break;  /* Not listed */
    }
}

#endif    /* ifndef MACOSX */

#else /* ifdef LINUX || SOLARIS || MACOSX || NETBSD */

/*
 * This implementation of osl_getTextEncodingFromLocale maps
 * from the ISO language codes.
 */

const _pair _full_locale_list[] = {
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

/*****************************************************************************/
/* return the text encoding corresponding to the given locale
/*****************************************************************************/

rtl_TextEncoding osl_getTextEncodingFromLocale( rtl_Locale * pLocale )
{
    const _pair *language = 0;
    char locale_buf[64] = "";
    char *cp;

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


/*****************************************************************************/
/* return the current process locale
/*****************************************************************************/

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

/*****************************************************************************/
/* set the current process locale
/*****************************************************************************/

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
#elif defined( FREEBSD ) || defined( NETBSD )
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
#elif defined( FREEBSD ) || defined( NETBSD )
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
#elif defined( FREEBSD ) || defined( NETBSD )
            setenv("LC_CTYPE", locale_buf, 1 );
#else
            setenv( "LANG", locale_buf );
#endif
        }
    }

    return 0;
}

#endif /* ifdef LINUX || SOLARIS || MACOSX || NETBSD */


