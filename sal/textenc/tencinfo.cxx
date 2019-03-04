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

#include <sal/config.h>

#include <cstddef>
#include <cstring>

#include <rtl/tencinfo.h>

#include "gettextencodingdata.hxx"
#include "tenchelp.hxx"
#include <memory>

sal_Bool SAL_CALL rtl_isOctetTextEncoding(rtl_TextEncoding nEncoding)
{
    return
        nEncoding > RTL_TEXTENCODING_DONTKNOW
        && nEncoding != 9 // RTL_TEXTENCODING_SYSTEM
        && nEncoding <= RTL_TEXTENCODING_ADOBE_DINGBATS; // always update this!
}

/* ======================================================================= */

static void Impl_toAsciiLower( const char* pName, char* pBuf )
{
    while ( *pName )
    {
        /* A-Z */
        if ( (*pName >= 0x41) && (*pName <= 0x5A) )
            *pBuf = (*pName)+0x20;  /* toAsciiLower */
        else
            *pBuf = *pName;

        pBuf++;
        pName++;
    }

    *pBuf = '\0';
}

/* ----------------------------------------------------------------------- */

static void Impl_toAsciiLowerAndRemoveNonAlphanumeric( const char* pName, char* pBuf )
{
    while ( *pName )
    {
        /* A-Z */
        if ( (*pName >= 0x41) && (*pName <= 0x5A) )
        {
            *pBuf = (*pName)+0x20;  /* toAsciiLower */
            pBuf++;
        }
        /* a-z, 0-9 */
        else if ( ((*pName >= 0x61) && (*pName <= 0x7A)) ||
                  ((*pName >= 0x30) && (*pName <= 0x39)) )
        {
            *pBuf = *pName;
            pBuf++;
        }

        pName++;
    }

    *pBuf = '\0';
}

/* ----------------------------------------------------------------------- */

/* pMatchStr must match with all characters in pCompStr */
static bool Impl_matchString( const char* pCompStr, const char* pMatchStr )
{
    /* We test only for end in MatchStr, because the last 0 character from */
    /* pCompStr is unequal a character in MatchStr, so the loop terminates */
    while ( *pMatchStr )
    {
        if ( *pCompStr != *pMatchStr )
            return false;

        pCompStr++;
        pMatchStr++;
    }

    return true;
}

/* ======================================================================= */

struct ImplStrCharsetDef
{
    const char*             mpCharsetStr;
    rtl_TextEncoding const  meTextEncoding;
};

struct ImplStrFirstPartCharsetDef
{
    const char*             mpCharsetStr;
    const ImplStrCharsetDef*    mpSecondPartTab;
};

/* ======================================================================= */

sal_Bool SAL_CALL rtl_getTextEncodingInfo( rtl_TextEncoding eTextEncoding, rtl_TextEncodingInfo* pEncInfo )
{
    const ImplTextEncodingData* pData;

    pData = Impl_getTextEncodingData( eTextEncoding );
    if ( !pData )
    {
        /* HACK: For not implemented encoding, because not all
           calls handle the errors */
        if ( pEncInfo->StructSize < 5 )
            return false;
        pEncInfo->MinimumCharSize = 1;

        if ( pEncInfo->StructSize < 6 )
            return true;
        pEncInfo->MaximumCharSize = 1;

        if ( pEncInfo->StructSize < 7 )
            return true;
        pEncInfo->AverageCharSize = 1;

        if ( pEncInfo->StructSize < 12 )
            return true;
        pEncInfo->Flags = 0;

        return false;
    }

    if ( pEncInfo->StructSize < 5 )
        return false;
    pEncInfo->MinimumCharSize = pData->mnMinCharSize;

    if ( pEncInfo->StructSize < 6 )
        return true;
    pEncInfo->MaximumCharSize = pData->mnMaxCharSize;

    if ( pEncInfo->StructSize < 7 )
        return true;
    pEncInfo->AverageCharSize = pData->mnAveCharSize;

    if ( pEncInfo->StructSize < 12 )
        return true;
    pEncInfo->Flags = pData->mnInfoFlags;

    return true;
}

/* ======================================================================= */

rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromWindowsCharset( sal_uInt8 nWinCharset )
{
    rtl_TextEncoding eTextEncoding;

    switch ( nWinCharset )
    {
        case 0:     eTextEncoding = RTL_TEXTENCODING_MS_1252; break;    /* ANSI_CHARSET */
        case 2:     eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;     /* SYMBOL_CHARSET */
        case 77:    eTextEncoding = RTL_TEXTENCODING_APPLE_ROMAN; break;/* MAC_CHARSET */
        case 128:   eTextEncoding = RTL_TEXTENCODING_MS_932; break;     /* SHIFTJIS_CHARSET */
        case 129:   eTextEncoding = RTL_TEXTENCODING_MS_949; break;     /* HANGEUL_CHARSET */
        case 130:   eTextEncoding = RTL_TEXTENCODING_MS_1361; break;    /* JOHAB_CHARSET */
        case 134:   eTextEncoding = RTL_TEXTENCODING_MS_936; break;     /* GB2312_CHARSET */
        case 136:   eTextEncoding = RTL_TEXTENCODING_MS_950; break;     /* CHINESEBIG5_CHARSET */
        case 161:   eTextEncoding = RTL_TEXTENCODING_MS_1253; break;    /* GREEK_CHARSET */
        case 162:   eTextEncoding = RTL_TEXTENCODING_MS_1254; break;    /* TURKISH_CHARSET */
        case 163:   eTextEncoding = RTL_TEXTENCODING_MS_1258; break;    /* VIETNAMESE_CHARSET !!! */
        case 177:   eTextEncoding = RTL_TEXTENCODING_MS_1255; break;    /* HEBREW_CHARSET */
        case 178:   eTextEncoding = RTL_TEXTENCODING_MS_1256; break;    /* ARABIC_CHARSET */
        case 186:   eTextEncoding = RTL_TEXTENCODING_MS_1257; break;    /* BALTIC_CHARSET */
        case 204:   eTextEncoding = RTL_TEXTENCODING_MS_1251; break;    /* RUSSIAN_CHARSET */
        case 222:   eTextEncoding = RTL_TEXTENCODING_MS_874; break;     /* THAI_CHARSET */
        case 238:   eTextEncoding = RTL_TEXTENCODING_MS_1250; break;    /* EASTEUROPE_CHARSET */
        case 255:   eTextEncoding = RTL_TEXTENCODING_IBM_850; break;    /* OEM_CHARSET */
        default:    eTextEncoding = RTL_TEXTENCODING_DONTKNOW; break;
    }

    return eTextEncoding;
}

/* ----------------------------------------------------------------------- */

rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromUnixCharset( const sal_Char* pUnixCharset )
{
    /* See <ftp://ftp.x.org/pub/DOCS/registry>, section 14 ("Font Charset
     * (Registry and Encoding) Names").
     */

    /* All Identifiers in the tables are lower case The function search */
    /* for the first matching string in the tables. */
    /* Sort order: unique (first 14, than 1), important */

    static ImplStrCharsetDef const aUnixCharsetISOTab[] =
    {
        { "15", RTL_TEXTENCODING_ISO_8859_15 },
        { "14", RTL_TEXTENCODING_ISO_8859_14 },
        { "13", RTL_TEXTENCODING_ISO_8859_13 },
        { "11", RTL_TEXTENCODING_TIS_620 },
        { "10", RTL_TEXTENCODING_ISO_8859_10 },
        { "1", RTL_TEXTENCODING_ISO_8859_1 },
        { "2", RTL_TEXTENCODING_ISO_8859_2 },
        { "3", RTL_TEXTENCODING_ISO_8859_3 },
        { "4", RTL_TEXTENCODING_ISO_8859_4 },
        { "5", RTL_TEXTENCODING_ISO_8859_5 },
        { "6", RTL_TEXTENCODING_ISO_8859_6 },
        { "7", RTL_TEXTENCODING_ISO_8859_7 },
        { "8", RTL_TEXTENCODING_ISO_8859_8 },
        { "9", RTL_TEXTENCODING_ISO_8859_9 },
        { nullptr, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrCharsetDef const aUnixCharsetADOBETab[] =
    {
        { "fontspecific", RTL_TEXTENCODING_SYMBOL },
        { nullptr, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrCharsetDef const aUnixCharsetMSTab[] =
    {
        { "1252", RTL_TEXTENCODING_MS_1252 },
        { "1250", RTL_TEXTENCODING_MS_1250 },
        { "1251", RTL_TEXTENCODING_MS_1251 },
        { "1253", RTL_TEXTENCODING_MS_1253 },
        { "1254", RTL_TEXTENCODING_MS_1254 },
        { "1255", RTL_TEXTENCODING_MS_1255 },
        { "1256", RTL_TEXTENCODING_MS_1256 },
        { "1257", RTL_TEXTENCODING_MS_1257 },
        { "1258", RTL_TEXTENCODING_MS_1258 },
        { "932", RTL_TEXTENCODING_MS_932 },
        { "936", RTL_TEXTENCODING_MS_936 },
        { "949", RTL_TEXTENCODING_MS_949 },
        { "950", RTL_TEXTENCODING_MS_950 },
        { "1361", RTL_TEXTENCODING_MS_1361 },
        { "cp1252", RTL_TEXTENCODING_MS_1252 },
        { "cp1250", RTL_TEXTENCODING_MS_1250 },
        { "cp1251", RTL_TEXTENCODING_MS_1251 },
        { "cp1253", RTL_TEXTENCODING_MS_1253 },
        { "cp1254", RTL_TEXTENCODING_MS_1254 },
        { "cp1255", RTL_TEXTENCODING_MS_1255 },
        { "cp1256", RTL_TEXTENCODING_MS_1256 },
        { "cp1257", RTL_TEXTENCODING_MS_1257 },
        { "cp1258", RTL_TEXTENCODING_MS_1258 },
        { "cp932", RTL_TEXTENCODING_MS_932 },
        { "cp936", RTL_TEXTENCODING_MS_936 },
        { "cp949", RTL_TEXTENCODING_MS_949 },
        { "cp950", RTL_TEXTENCODING_MS_950 },
        { "cp1361", RTL_TEXTENCODING_MS_1361 },
        { nullptr, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrCharsetDef const aUnixCharsetIBMTab[] =
    {
        { "437", RTL_TEXTENCODING_IBM_437 },
        { "850", RTL_TEXTENCODING_IBM_850 },
        { "860", RTL_TEXTENCODING_IBM_860 },
        { "861", RTL_TEXTENCODING_IBM_861 },
        { "863", RTL_TEXTENCODING_IBM_863 },
        { "865", RTL_TEXTENCODING_IBM_865 },
        { "737", RTL_TEXTENCODING_IBM_737 },
        { "775", RTL_TEXTENCODING_IBM_775 },
        { "852", RTL_TEXTENCODING_IBM_852 },
        { "855", RTL_TEXTENCODING_IBM_855 },
        { "857", RTL_TEXTENCODING_IBM_857 },
        { "862", RTL_TEXTENCODING_IBM_862 },
        { "864", RTL_TEXTENCODING_IBM_864 },
        { "866", RTL_TEXTENCODING_IBM_866 },
        { "869", RTL_TEXTENCODING_IBM_869 },
        { "874", RTL_TEXTENCODING_MS_874 },
        { "1004", RTL_TEXTENCODING_MS_1252 },
        { "65400", RTL_TEXTENCODING_SYMBOL },
        { nullptr, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrCharsetDef const aUnixCharsetKOI8Tab[] =
    {
        { "r", RTL_TEXTENCODING_KOI8_R },
        { "u", RTL_TEXTENCODING_KOI8_U },
        { nullptr, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrCharsetDef const aUnixCharsetJISX0208Tab[] =
    {
        { nullptr, RTL_TEXTENCODING_JIS_X_0208 }
    };

    static ImplStrCharsetDef const aUnixCharsetJISX0201Tab[] =
    {
        { nullptr, RTL_TEXTENCODING_JIS_X_0201 }
    };

    static ImplStrCharsetDef const aUnixCharsetJISX0212Tab[] =
    {
        { nullptr, RTL_TEXTENCODING_JIS_X_0212 }
    };

    static ImplStrCharsetDef const aUnixCharsetGBTab[] =
    {
        { nullptr, RTL_TEXTENCODING_GB_2312 }
    };

    static ImplStrCharsetDef const aUnixCharsetGBKTab[] =
    {
        { nullptr, RTL_TEXTENCODING_GBK }
    };

    static ImplStrCharsetDef const aUnixCharsetBIG5Tab[] =
    {
        { nullptr, RTL_TEXTENCODING_BIG5 }
    };

    static ImplStrCharsetDef const aUnixCharsetKSC56011987Tab[] =
    {
        { nullptr, RTL_TEXTENCODING_EUC_KR }
    };

    static ImplStrCharsetDef const aUnixCharsetKSC56011992Tab[] =
    {
        { nullptr, RTL_TEXTENCODING_MS_1361 }
    };

    static ImplStrCharsetDef const aUnixCharsetISO10646Tab[] =
    {
        { nullptr, RTL_TEXTENCODING_UNICODE }
    };

    static ImplStrCharsetDef const aUnixCharsetUNICODETab[] =
    {
/* Currently every Unicode Encoding is for us Unicode */
/*        { "fontspecific", RTL_TEXTENCODING_UNICODE }, */
        { nullptr, RTL_TEXTENCODING_UNICODE }
    };

    static ImplStrCharsetDef const aUnixCharsetSymbolTab[] =
    {
        { nullptr, RTL_TEXTENCODING_SYMBOL }
    };

    /* See <http://cvs.freedesktop.org/xorg/xc/fonts/encodings/iso8859-11.enc?
       rev=1.1.1.1>: */
    static ImplStrCharsetDef const aUnixCharsetTIS620Tab[] =
    {
        { "0", RTL_TEXTENCODING_TIS_620 },
        { "2529", RTL_TEXTENCODING_TIS_620 },
        { "2533", RTL_TEXTENCODING_TIS_620 },
        { nullptr, RTL_TEXTENCODING_DONTKNOW }
    };
    static ImplStrCharsetDef const aUnixCharsetTIS6202529Tab[] =
    {
        { "1", RTL_TEXTENCODING_TIS_620 },
        { nullptr, RTL_TEXTENCODING_DONTKNOW }
    };
    static ImplStrCharsetDef const aUnixCharsetTIS6202533Tab[] =
    {
        { "0", RTL_TEXTENCODING_TIS_620 },
        { "1", RTL_TEXTENCODING_TIS_620 },
        { nullptr, RTL_TEXTENCODING_DONTKNOW }
    };

    static ImplStrFirstPartCharsetDef const aUnixCharsetFirstPartTab[] =
    {
        { "iso8859", aUnixCharsetISOTab },
        { "adobe", aUnixCharsetADOBETab },
        { "ansi", aUnixCharsetMSTab },
        { "microsoft", aUnixCharsetMSTab },
        { "ibm", aUnixCharsetIBMTab },
        { "koi8", aUnixCharsetKOI8Tab },
        { "jisx0208", aUnixCharsetJISX0208Tab },
        { "jisx0208.1983", aUnixCharsetJISX0208Tab },
        { "jisx0201", aUnixCharsetJISX0201Tab },
        { "jisx0201.1976", aUnixCharsetJISX0201Tab },
        { "jisx0212", aUnixCharsetJISX0212Tab },
        { "jisx0212.1990", aUnixCharsetJISX0212Tab },
        { "gb2312", aUnixCharsetGBTab },
        { "gbk", aUnixCharsetGBKTab },
        { "big5", aUnixCharsetBIG5Tab },
        { "iso10646", aUnixCharsetISO10646Tab },
/*      { "unicode", aUnixCharsetUNICODETab }, */ /* fonts contain only default chars */
        { "sunolcursor", aUnixCharsetSymbolTab },
        { "sunolglyph", aUnixCharsetSymbolTab },
        { "iso10646", aUnixCharsetUNICODETab },
        { "ksc5601.1987", aUnixCharsetKSC56011987Tab },
        { "ksc5601.1992", aUnixCharsetKSC56011992Tab },
        { "tis620.2529", aUnixCharsetTIS6202529Tab },
        { "tis620.2533", aUnixCharsetTIS6202533Tab },
        { "tis620", aUnixCharsetTIS620Tab },
/*        { "sunudcja.1997",  },        */
/*        { "sunudcko.1997",  },        */
/*        { "sunudczh.1997",  },        */
/*        { "sunudczhtw.1997",  },      */
        { nullptr, nullptr }
    };

    rtl_TextEncoding    eEncoding = RTL_TEXTENCODING_DONTKNOW;
    char*           pTempBuf;
    sal_uInt32          nBufLen = strlen( pUnixCharset )+1;
    const char*     pFirstPart;
    const char*     pSecondPart;

    /* Alloc Buffer and map to lower case */
    std::unique_ptr<char[]> pBuf(new char[nBufLen]);
    Impl_toAsciiLower( pUnixCharset, pBuf.get() );

    /* Search FirstPart */
    pFirstPart = pBuf.get();
    pSecondPart = nullptr;
    pTempBuf = pBuf.get();
    while ( *pTempBuf )
    {
        if ( *pTempBuf == '-' )
        {
            *pTempBuf = '\0';
            pSecondPart = pTempBuf+1;
            break;
        }

        pTempBuf++;
    }

    /* found part separator */
    if ( pSecondPart )
    {
        /* Search for the part tab */
        const ImplStrFirstPartCharsetDef* pFirstPartData = aUnixCharsetFirstPartTab;
        while ( pFirstPartData->mpCharsetStr )
        {
            if ( Impl_matchString( pFirstPart, pFirstPartData->mpCharsetStr ) )
            {
                /* Search for the charset in the second part tab */
                const ImplStrCharsetDef* pData = pFirstPartData->mpSecondPartTab;
                while ( pData->mpCharsetStr )
                {
                    if ( Impl_matchString( pSecondPart, pData->mpCharsetStr ) )
                    {
                        break;
                    }

                    pData++;
                }

                /* use default encoding for first part */
                eEncoding = pData->meTextEncoding;
                break;
            }

            pFirstPartData++;
        }
    }

    return eEncoding;
}

/* ----------------------------------------------------------------------- */

rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromMimeCharset( const sal_Char* pMimeCharset )
{
    /* All Identifiers are in lower case and contain only alphanumeric */
    /* characters. The function search for the first equal string in */
    /* the table. In this table are only the most used mime types. */
    /* Sort order: important */
    static ImplStrCharsetDef const aVIPMimeCharsetTab[] =
    {
        { "usascii", RTL_TEXTENCODING_ASCII_US },
        { "utf8", RTL_TEXTENCODING_UTF8 },
        { "utf7", RTL_TEXTENCODING_UTF7 },
        { "iso88591", RTL_TEXTENCODING_ISO_8859_1 },
        { "iso88592", RTL_TEXTENCODING_ISO_8859_2 },
        { "iso88593", RTL_TEXTENCODING_ISO_8859_3 },
        { "iso88594", RTL_TEXTENCODING_ISO_8859_4 },
        { "iso88595", RTL_TEXTENCODING_ISO_8859_5 },
        { "iso88596", RTL_TEXTENCODING_ISO_8859_6 },
        { "iso88597", RTL_TEXTENCODING_ISO_8859_7 },
        { "iso88598", RTL_TEXTENCODING_ISO_8859_8 },
        { "iso88599", RTL_TEXTENCODING_ISO_8859_9 },
        { "iso885910", RTL_TEXTENCODING_ISO_8859_10 },
        { "iso885913", RTL_TEXTENCODING_ISO_8859_13 },
        { "iso885914", RTL_TEXTENCODING_ISO_8859_14 },
        { "iso885915", RTL_TEXTENCODING_ISO_8859_15 },
        { "iso2022jp", RTL_TEXTENCODING_ISO_2022_JP },
        { "iso2022jp2", RTL_TEXTENCODING_ISO_2022_JP },
        { "iso2022cn", RTL_TEXTENCODING_ISO_2022_CN },
        { "iso2022cnext", RTL_TEXTENCODING_ISO_2022_CN },
        { "iso2022kr", RTL_TEXTENCODING_ISO_2022_KR },
        { "eucjp", RTL_TEXTENCODING_EUC_JP },
        { "shiftjis", RTL_TEXTENCODING_SHIFT_JIS },
        { "mskanji", RTL_TEXTENCODING_MS_932 },
        { "gb2312", RTL_TEXTENCODING_GB_2312 },
        { "cngb", RTL_TEXTENCODING_GB_2312 },
        { "big5", RTL_TEXTENCODING_BIG5 },
        { "cnbig5", RTL_TEXTENCODING_BIG5 },
        { "cngb12345", RTL_TEXTENCODING_GBT_12345 },
        { "euckr", RTL_TEXTENCODING_EUC_KR },
        { "koi8r", RTL_TEXTENCODING_KOI8_R },
        { "windows1252", RTL_TEXTENCODING_MS_1252 },
        { "windows1250", RTL_TEXTENCODING_MS_1250 },
        { "windows1251", RTL_TEXTENCODING_MS_1251 },
        { "windows1253", RTL_TEXTENCODING_MS_1253 },
        { "windows1254", RTL_TEXTENCODING_MS_1254 },
        { "windows1255", RTL_TEXTENCODING_MS_1255 },
        { "windows1256", RTL_TEXTENCODING_MS_1256 },
        { "windows1257", RTL_TEXTENCODING_MS_1257 },
        { "windows1258", RTL_TEXTENCODING_MS_1258 },
        { nullptr, RTL_TEXTENCODING_DONTKNOW }
    };

    /* All Identifiers are in lower case and contain only alphanumeric */
    /* characters. The function search for the first matching string in */
    /* the table. */
    /* Sort order: unique (first iso885914, than iso88591), important */
    static ImplStrCharsetDef const aMimeCharsetTab[] =
    {
        { "unicode11utf7", RTL_TEXTENCODING_UTF7 },
        { "caunicode11utf7", RTL_TEXTENCODING_UTF7 },
        { "iso88591windows30", RTL_TEXTENCODING_ISO_8859_1 },
        { "iso88591win", RTL_TEXTENCODING_MS_1252 },
        { "iso88592win", RTL_TEXTENCODING_MS_1250 },
        { "iso88599win", RTL_TEXTENCODING_MS_1254 },
        { "iso885915", RTL_TEXTENCODING_ISO_8859_15 },
        { "iso885914", RTL_TEXTENCODING_ISO_8859_14 },
        { "iso885913", RTL_TEXTENCODING_ISO_8859_13 },
        { "iso885911", RTL_TEXTENCODING_TIS_620 },
            /* This is no official MIME character set name, but it might be in
               use in Thailand. */
        { "iso885910", RTL_TEXTENCODING_ISO_8859_10 },
        { "iso88591", RTL_TEXTENCODING_ISO_8859_1 },
        { "iso88592", RTL_TEXTENCODING_ISO_8859_2 },
        { "iso88593", RTL_TEXTENCODING_ISO_8859_3 },
        { "iso88594", RTL_TEXTENCODING_ISO_8859_4 },
        { "iso88595", RTL_TEXTENCODING_ISO_8859_5 },
        { "iso88596", RTL_TEXTENCODING_ISO_8859_6 },
        { "iso88597", RTL_TEXTENCODING_ISO_8859_7 },
        { "iso88598", RTL_TEXTENCODING_ISO_8859_8 },
        { "iso88599", RTL_TEXTENCODING_ISO_8859_9 },
        { "isoir100", RTL_TEXTENCODING_ISO_8859_1 },
        { "latin1", RTL_TEXTENCODING_ISO_8859_1 },
        { "l1", RTL_TEXTENCODING_ISO_8859_1 },
        { "cp819", RTL_TEXTENCODING_ISO_8859_1 },
        { "ibm819", RTL_TEXTENCODING_ISO_8859_1 },
        { "csisolatin1", RTL_TEXTENCODING_ISO_8859_1 },
        { "isoir101", RTL_TEXTENCODING_ISO_8859_2 },
        { "latin2", RTL_TEXTENCODING_ISO_8859_2 },
        { "l2", RTL_TEXTENCODING_ISO_8859_2 },
        { "csisolatin2", RTL_TEXTENCODING_ISO_8859_2 },
        { "isoir109", RTL_TEXTENCODING_ISO_8859_3 },
        { "latin3", RTL_TEXTENCODING_ISO_8859_3 },
        { "l3", RTL_TEXTENCODING_ISO_8859_3 },
        { "csisolatin3", RTL_TEXTENCODING_ISO_8859_3 },
        { "isoir110", RTL_TEXTENCODING_ISO_8859_4 },
        { "latin4", RTL_TEXTENCODING_ISO_8859_4 },
        { "l4", RTL_TEXTENCODING_ISO_8859_4 },
        { "csisolatin4", RTL_TEXTENCODING_ISO_8859_4 },
        { "isoir144", RTL_TEXTENCODING_ISO_8859_5 },
        { "cyrillicasian", RTL_TEXTENCODING_PT154 },
        { "cyrillic", RTL_TEXTENCODING_ISO_8859_5 },
        { "csisolatincyrillic", RTL_TEXTENCODING_ISO_8859_5 },
        { "isoir127", RTL_TEXTENCODING_ISO_8859_6 },
        { "arabic", RTL_TEXTENCODING_ISO_8859_6 },
        { "csisolatinarabic", RTL_TEXTENCODING_ISO_8859_6 },
        { "ecma114", RTL_TEXTENCODING_ISO_8859_6 },
        { "asmo708", RTL_TEXTENCODING_ISO_8859_6 },
        { "isoir126", RTL_TEXTENCODING_ISO_8859_7 },
        { "greek", RTL_TEXTENCODING_ISO_8859_7 },
        { "csisolatingreek", RTL_TEXTENCODING_ISO_8859_7 },
        { "elot928", RTL_TEXTENCODING_ISO_8859_7 },
        { "ecma118", RTL_TEXTENCODING_ISO_8859_7 },
        { "isoir138", RTL_TEXTENCODING_ISO_8859_8 },
        { "hebrew", RTL_TEXTENCODING_ISO_8859_8 },
        { "csisolatinhebrew", RTL_TEXTENCODING_ISO_8859_8 },
        { "isoir148", RTL_TEXTENCODING_ISO_8859_9 },
        { "latin5", RTL_TEXTENCODING_ISO_8859_9 },
        { "l5", RTL_TEXTENCODING_ISO_8859_9 },
        { "csisolatin5", RTL_TEXTENCODING_ISO_8859_9 },
        { "cswindows30latin1", RTL_TEXTENCODING_ISO_8859_1 },
        { "cswindows30latin1", RTL_TEXTENCODING_ISO_8859_1 },
        { "cswindows31latin1", RTL_TEXTENCODING_MS_1252 },
        { "cswindows31latin2", RTL_TEXTENCODING_MS_1250 },
        { "cswindows31latin5", RTL_TEXTENCODING_MS_1254 },
        { "iso10646us", RTL_TEXTENCODING_ASCII_US },
        { "iso646irv", RTL_TEXTENCODING_ASCII_US },
        { "cskoi8r", RTL_TEXTENCODING_KOI8_R },
        { "ibm437", RTL_TEXTENCODING_IBM_437 },
        { "cp437", RTL_TEXTENCODING_IBM_437 },
        { "437", RTL_TEXTENCODING_IBM_437 },
        { "cspc8codepage437", RTL_TEXTENCODING_IBM_437 },
        { "ansix34", RTL_TEXTENCODING_ASCII_US },
        { "ibm367", RTL_TEXTENCODING_ASCII_US },
        { "cp367", RTL_TEXTENCODING_ASCII_US },
        { "csascii", RTL_TEXTENCODING_ASCII_US },
        { "ibm775", RTL_TEXTENCODING_IBM_775 },
        { "cp775", RTL_TEXTENCODING_IBM_775 },
        { "cspc775baltic", RTL_TEXTENCODING_IBM_775 },
        { "ibm850", RTL_TEXTENCODING_IBM_850 },
        { "cp850", RTL_TEXTENCODING_IBM_850 },
        { "850", RTL_TEXTENCODING_IBM_850 },
        { "cspc850multilingual", RTL_TEXTENCODING_IBM_850 },
/*        { "ibm851", RTL_TEXTENCODING_IBM_851 }, */
/*        { "cp851", RTL_TEXTENCODING_IBM_851 }, */
/*        { "851", RTL_TEXTENCODING_IBM_851 }, */
/*        { "csibm851", RTL_TEXTENCODING_IBM_851 }, */
        { "ibm852", RTL_TEXTENCODING_IBM_852 },
        { "cp852", RTL_TEXTENCODING_IBM_852 },
        { "852", RTL_TEXTENCODING_IBM_852 },
        { "cspcp852", RTL_TEXTENCODING_IBM_852 },
        { "ibm855", RTL_TEXTENCODING_IBM_855 },
        { "cp855", RTL_TEXTENCODING_IBM_855 },
        { "855", RTL_TEXTENCODING_IBM_855 },
        { "csibm855", RTL_TEXTENCODING_IBM_855 },
        { "ibm857", RTL_TEXTENCODING_IBM_857 },
        { "cp857", RTL_TEXTENCODING_IBM_857 },
        { "857", RTL_TEXTENCODING_IBM_857 },
        { "csibm857", RTL_TEXTENCODING_IBM_857 },
        { "ibm860", RTL_TEXTENCODING_IBM_860 },
        { "cp860", RTL_TEXTENCODING_IBM_860 },
        { "860", RTL_TEXTENCODING_IBM_860 },
        { "csibm860", RTL_TEXTENCODING_IBM_860 },
        { "ibm861", RTL_TEXTENCODING_IBM_861 },
        { "cp861", RTL_TEXTENCODING_IBM_861 },
        { "861", RTL_TEXTENCODING_IBM_861 },
        { "csis", RTL_TEXTENCODING_IBM_861 },
        { "csibm861", RTL_TEXTENCODING_IBM_861 },
        { "ibm862", RTL_TEXTENCODING_IBM_862 },
        { "cp862", RTL_TEXTENCODING_IBM_862 },
        { "862", RTL_TEXTENCODING_IBM_862 },
        { "cspc862latinhebrew", RTL_TEXTENCODING_IBM_862 },
        { "ibm863", RTL_TEXTENCODING_IBM_863 },
        { "cp863", RTL_TEXTENCODING_IBM_863 },
        { "863", RTL_TEXTENCODING_IBM_863 },
        { "csibm863", RTL_TEXTENCODING_IBM_863 },
        { "ibm864", RTL_TEXTENCODING_IBM_864 },
        { "cp864", RTL_TEXTENCODING_IBM_864 },
        { "864", RTL_TEXTENCODING_IBM_864 },
        { "csibm864", RTL_TEXTENCODING_IBM_864 },
        { "ibm865", RTL_TEXTENCODING_IBM_865 },
        { "cp865", RTL_TEXTENCODING_IBM_865 },
        { "865", RTL_TEXTENCODING_IBM_865 },
        { "csibm865", RTL_TEXTENCODING_IBM_865 },
        { "ibm866", RTL_TEXTENCODING_IBM_866 },
        { "cp866", RTL_TEXTENCODING_IBM_866 },
        { "866", RTL_TEXTENCODING_IBM_866 },
        { "csibm866", RTL_TEXTENCODING_IBM_866 },
/*        { "ibm868", RTL_TEXTENCODING_IBM_868 }, */
/*        { "cp868", RTL_TEXTENCODING_IBM_868 }, */
/*        { "cpar", RTL_TEXTENCODING_IBM_868 }, */
/*        { "csibm868", RTL_TEXTENCODING_IBM_868 }, */
        { "ibm869", RTL_TEXTENCODING_IBM_869 },
        { "cp869", RTL_TEXTENCODING_IBM_869 },
        { "869", RTL_TEXTENCODING_IBM_869 },
        { "cpgr", RTL_TEXTENCODING_IBM_869 },
        { "csibm869", RTL_TEXTENCODING_IBM_869 },
        { "ibm869", RTL_TEXTENCODING_IBM_869 },
        { "cp869", RTL_TEXTENCODING_IBM_869 },
        { "869", RTL_TEXTENCODING_IBM_869 },
        { "cpgr", RTL_TEXTENCODING_IBM_869 },
        { "csibm869", RTL_TEXTENCODING_IBM_869 },
        { "mac", RTL_TEXTENCODING_APPLE_ROMAN },
        { "csmacintosh", RTL_TEXTENCODING_APPLE_ROMAN },
        { "shiftjis", RTL_TEXTENCODING_SHIFT_JIS },
        { "mskanji", RTL_TEXTENCODING_MS_932 },
        { "csshiftjis", RTL_TEXTENCODING_SHIFT_JIS },
        { "jisx0208", RTL_TEXTENCODING_JIS_X_0208 },
        { "jisc62261983", RTL_TEXTENCODING_JIS_X_0208 },
        { "csiso87jisx0208", RTL_TEXTENCODING_JIS_X_0208 },
        { "isoir86", RTL_TEXTENCODING_JIS_X_0208 },
        { "x0208", RTL_TEXTENCODING_JIS_X_0208 },
        { "jisx0201", RTL_TEXTENCODING_JIS_X_0201 },
        { "cshalfwidthkatakana", RTL_TEXTENCODING_JIS_X_0201 },
        { "x0201", RTL_TEXTENCODING_JIS_X_0201 },
        { "jisx0212", RTL_TEXTENCODING_JIS_X_0212 },
        { "csiso159jisx0212", RTL_TEXTENCODING_JIS_X_0212 },
        { "isoir159", RTL_TEXTENCODING_JIS_X_0208 },
        { "x0212", RTL_TEXTENCODING_JIS_X_0212 },
        { "isoir6", RTL_TEXTENCODING_ASCII_US },
        { "xsjis", RTL_TEXTENCODING_SHIFT_JIS },
        { "sjis", RTL_TEXTENCODING_SHIFT_JIS },
        { "ascii", RTL_TEXTENCODING_ASCII_US },
        { "us", RTL_TEXTENCODING_ASCII_US },
        { "gb180302000", RTL_TEXTENCODING_GB_18030 },
            /* This is no actual MIME character set name, it is only in here
               for backwards compatibility (before "GB18030" was officially
               registered with IANA, this code contained some guesses of what
               would become official names for GB18030). */
        { "gb18030", RTL_TEXTENCODING_GB_18030 },
        { "big5hkscs", RTL_TEXTENCODING_BIG5_HKSCS },
        { "tis620", RTL_TEXTENCODING_TIS_620 },
        { "gbk", RTL_TEXTENCODING_GBK },
        { "cp936", RTL_TEXTENCODING_GBK },
        { "ms936", RTL_TEXTENCODING_GBK },
        { "windows936", RTL_TEXTENCODING_GBK },
        { "cp874", RTL_TEXTENCODING_MS_874 },
            /* This is no official MIME character set name, but it might be in
               use in Thailand. */
        { "ms874", RTL_TEXTENCODING_MS_874 },
            /* This is no official MIME character set name, but it might be in
               use in Thailand. */
        { "windows874", RTL_TEXTENCODING_MS_874 },
            /* This is no official MIME character set name, but it might be in
               use in Thailand. */
        { "koi8u", RTL_TEXTENCODING_KOI8_U },
        { "cpis", RTL_TEXTENCODING_IBM_861 },
        { "ksc56011987", RTL_TEXTENCODING_MS_949 },
        { "isoir149", RTL_TEXTENCODING_MS_949 },
        { "ksc56011989", RTL_TEXTENCODING_MS_949 },
        { "ksc5601", RTL_TEXTENCODING_MS_949 },
        { "korean", RTL_TEXTENCODING_MS_949 },
        { "csksc56011987", RTL_TEXTENCODING_MS_949 },
            /* Map KS_C_5601-1987 and aliases to MS-949 instead of EUC-KR, as
               this character set identifier seems to be prominently used by MS
               to stand for KS C 5601 plus MS-949 extensions */
        { "latin9", RTL_TEXTENCODING_ISO_8859_15 },
        { "adobestandardencoding", RTL_TEXTENCODING_ADOBE_STANDARD },
        { "csadobestandardencoding", RTL_TEXTENCODING_ADOBE_STANDARD },
        { "adobesymbolencoding", RTL_TEXTENCODING_ADOBE_SYMBOL },
        { "cshppsmath", RTL_TEXTENCODING_ADOBE_SYMBOL },
        { "ptcp154", RTL_TEXTENCODING_PT154 },
        { "csptcp154", RTL_TEXTENCODING_PT154 },
        { "pt154", RTL_TEXTENCODING_PT154 },
        { "cp154", RTL_TEXTENCODING_PT154 },
        { "xisciide", RTL_TEXTENCODING_ISCII_DEVANAGARI },
            /* This is not an official MIME character set name, but is in use by
               various windows APIs. */
        { nullptr, RTL_TEXTENCODING_DONTKNOW }
    };

    rtl_TextEncoding            eEncoding = RTL_TEXTENCODING_DONTKNOW;
    const ImplStrCharsetDef*    pData = aVIPMimeCharsetTab;
    sal_uInt32                  nBufLen = strlen( pMimeCharset )+1;

    /* Alloc Buffer and map to lower case and remove non alphanumeric chars */
    std::unique_ptr<char[]> pBuf(new char[nBufLen]);
    Impl_toAsciiLowerAndRemoveNonAlphanumeric( pMimeCharset, pBuf.get() );

    /* Search for equal in the VIP table */
    while ( pData->mpCharsetStr )
    {
        if ( strcmp( pBuf.get(), pData->mpCharsetStr ) == 0 )
        {
            eEncoding = pData->meTextEncoding;
            break;
        }

        pData++;
    }

    /* Search for matching in the mime table */
    if ( eEncoding == RTL_TEXTENCODING_DONTKNOW )
    {
        pData = aMimeCharsetTab;
        while ( pData->mpCharsetStr )
        {
            if ( Impl_matchString( pBuf.get(), pData->mpCharsetStr ) )
            {
                eEncoding = pData->meTextEncoding;
                break;
            }

            pData++;
        }
    }

    return eEncoding;
}

/* ======================================================================= */

sal_uInt8 SAL_CALL rtl_getBestWindowsCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding )
{
    const ImplTextEncodingData* pData = Impl_getTextEncodingData( eTextEncoding );
    if ( pData )
        return pData->mnBestWindowsCharset;
    return 1;
}

/* ----------------------------------------------------------------------- */

const sal_Char* SAL_CALL rtl_getBestUnixCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding  )
{
    const ImplTextEncodingData* pData = Impl_getTextEncodingData( eTextEncoding );
    if ( pData )
        return pData->mpBestUnixCharset;
    if( eTextEncoding == RTL_TEXTENCODING_UNICODE )
        return "iso10646-1";
    return nullptr;
}

/* ----------------------------------------------------------------------- */

char const * SAL_CALL rtl_getMimeCharsetFromTextEncoding(rtl_TextEncoding
                                                             nEncoding)
{
    ImplTextEncodingData const * p = Impl_getTextEncodingData(nEncoding);
    return p && (p->mnInfoFlags & RTL_TEXTENCODING_INFO_MIME) != 0 ?
               p->mpBestMimeCharset : nullptr;
}

const sal_Char* SAL_CALL rtl_getBestMimeCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding )
{
    const ImplTextEncodingData* pData = Impl_getTextEncodingData( eTextEncoding );
    if ( pData )
        return pData->mpBestMimeCharset;
    return nullptr;
}

/* The following two functions are based on <http://www.sharmahd.com/tm/
   codepages.html>, <http://msdn.microsoft.com/workshop/author/dhtml/reference/
   charsets/charset4.asp>, and <http://www.iana.org/assignments/character-sets>.
 */

rtl_TextEncoding SAL_CALL
rtl_getTextEncodingFromWindowsCodePage(sal_uInt32 nCodePage)
{
    switch (nCodePage)
    {
    case 42: return RTL_TEXTENCODING_SYMBOL;
    case 437: return RTL_TEXTENCODING_IBM_437;
    case 708: return RTL_TEXTENCODING_ISO_8859_6;
    case 737: return RTL_TEXTENCODING_IBM_737;
    case 775: return RTL_TEXTENCODING_IBM_775;
    case 850: return RTL_TEXTENCODING_IBM_850;
    case 852: return RTL_TEXTENCODING_IBM_852;
    case 855: return RTL_TEXTENCODING_IBM_855;
    case 857: return RTL_TEXTENCODING_IBM_857;
    case 860: return RTL_TEXTENCODING_IBM_860;
    case 861: return RTL_TEXTENCODING_IBM_861;
    case 862: return RTL_TEXTENCODING_IBM_862;
    case 863: return RTL_TEXTENCODING_IBM_863;
    case 864: return RTL_TEXTENCODING_IBM_864;
    case 865: return RTL_TEXTENCODING_IBM_865;
    case 866: return RTL_TEXTENCODING_IBM_866;
    case 869: return RTL_TEXTENCODING_IBM_869;
    case 874: return RTL_TEXTENCODING_MS_874;
    case 932: return RTL_TEXTENCODING_MS_932;
    case 936: return RTL_TEXTENCODING_MS_936;
    case 949: return RTL_TEXTENCODING_MS_949;
    case 950: return RTL_TEXTENCODING_MS_950;
    case 1250: return RTL_TEXTENCODING_MS_1250;
    case 1251: return RTL_TEXTENCODING_MS_1251;
    case 1252: return RTL_TEXTENCODING_MS_1252;
    case 1253: return RTL_TEXTENCODING_MS_1253;
    case 1254: return RTL_TEXTENCODING_MS_1254;
    case 1255: return RTL_TEXTENCODING_MS_1255;
    case 1256: return RTL_TEXTENCODING_MS_1256;
    case 1257: return RTL_TEXTENCODING_MS_1257;
    case 1258: return RTL_TEXTENCODING_MS_1258;
    case 1361: return RTL_TEXTENCODING_MS_1361;
    case 10000: return RTL_TEXTENCODING_APPLE_ROMAN;
    case 10001: return RTL_TEXTENCODING_APPLE_JAPANESE;
    case 10002: return RTL_TEXTENCODING_APPLE_CHINTRAD;
    case 10003: return RTL_TEXTENCODING_APPLE_KOREAN;
    case 10004: return RTL_TEXTENCODING_APPLE_ARABIC;
    case 10005: return RTL_TEXTENCODING_APPLE_HEBREW;
    case 10006: return RTL_TEXTENCODING_APPLE_GREEK;
    case 10007: return RTL_TEXTENCODING_APPLE_CYRILLIC;
    case 10008: return RTL_TEXTENCODING_APPLE_CHINSIMP;
    case 10010: return RTL_TEXTENCODING_APPLE_ROMANIAN;
    case 10017: return RTL_TEXTENCODING_APPLE_UKRAINIAN;
    case 10029: return RTL_TEXTENCODING_APPLE_CENTEURO;
    case 10079: return RTL_TEXTENCODING_APPLE_ICELAND;
    case 10081: return RTL_TEXTENCODING_APPLE_TURKISH;
    case 10082: return RTL_TEXTENCODING_APPLE_CROATIAN;
    case 20127: return RTL_TEXTENCODING_ASCII_US;
    case 20866: return RTL_TEXTENCODING_KOI8_R;
    case 21866: return RTL_TEXTENCODING_KOI8_U;
    case 28591: return RTL_TEXTENCODING_ISO_8859_1;
    case 28592: return RTL_TEXTENCODING_ISO_8859_2;
    case 28593: return RTL_TEXTENCODING_ISO_8859_3;
    case 28594: return RTL_TEXTENCODING_ISO_8859_4;
    case 28595: return RTL_TEXTENCODING_ISO_8859_5;
    case 28596: return RTL_TEXTENCODING_ISO_8859_6;
    case 28597: return RTL_TEXTENCODING_ISO_8859_7;
    case 28598: return RTL_TEXTENCODING_ISO_8859_8;
    case 28599: return RTL_TEXTENCODING_ISO_8859_9;
    case 28605: return RTL_TEXTENCODING_ISO_8859_15;
    case 50220: return RTL_TEXTENCODING_ISO_2022_JP;
    case 50225: return RTL_TEXTENCODING_ISO_2022_KR;
    case 51932: return RTL_TEXTENCODING_EUC_JP;
    case 51936: return RTL_TEXTENCODING_EUC_CN;
    case 51949: return RTL_TEXTENCODING_EUC_KR;
    case 57002: return RTL_TEXTENCODING_ISCII_DEVANAGARI;
    case 65000: return RTL_TEXTENCODING_UTF7;
    case 65001: return RTL_TEXTENCODING_UTF8;
    default: return RTL_TEXTENCODING_DONTKNOW;
    }
}

sal_uInt32 SAL_CALL
rtl_getWindowsCodePageFromTextEncoding(rtl_TextEncoding nEncoding)
{
    switch (nEncoding)
    {
    case RTL_TEXTENCODING_SYMBOL: return 42;
    case RTL_TEXTENCODING_IBM_437: return 437;
 /* case RTL_TEXTENCODING_ISO_8859_6: return 708; */
    case RTL_TEXTENCODING_IBM_737: return 737;
    case RTL_TEXTENCODING_IBM_775: return 775;
    case RTL_TEXTENCODING_IBM_850: return 850;
    case RTL_TEXTENCODING_IBM_852: return 852;
    case RTL_TEXTENCODING_IBM_855: return 855;
    case RTL_TEXTENCODING_IBM_857: return 857;
    case RTL_TEXTENCODING_IBM_860: return 860;
    case RTL_TEXTENCODING_IBM_861: return 861;
    case RTL_TEXTENCODING_IBM_862: return 862;
    case RTL_TEXTENCODING_IBM_863: return 863;
    case RTL_TEXTENCODING_IBM_864: return 864;
    case RTL_TEXTENCODING_IBM_865: return 865;
    case RTL_TEXTENCODING_IBM_866: return 866;
    case RTL_TEXTENCODING_IBM_869: return 869;
    case RTL_TEXTENCODING_MS_874: return 874;
    case RTL_TEXTENCODING_MS_932: return 932;
    case RTL_TEXTENCODING_MS_936: return 936;
    case RTL_TEXTENCODING_MS_949: return 949;
    case RTL_TEXTENCODING_MS_950: return 950;
    case RTL_TEXTENCODING_MS_1250: return 1250;
    case RTL_TEXTENCODING_MS_1251: return 1251;
    case RTL_TEXTENCODING_MS_1252: return 1252;
    case RTL_TEXTENCODING_MS_1253: return 1253;
    case RTL_TEXTENCODING_MS_1254: return 1254;
    case RTL_TEXTENCODING_MS_1255: return 1255;
    case RTL_TEXTENCODING_MS_1256: return 1256;
    case RTL_TEXTENCODING_MS_1257: return 1257;
    case RTL_TEXTENCODING_MS_1258: return 1258;
    case RTL_TEXTENCODING_MS_1361: return 1361;
    case RTL_TEXTENCODING_APPLE_ROMAN: return 10000;
    case RTL_TEXTENCODING_APPLE_JAPANESE: return 10001;
    case RTL_TEXTENCODING_APPLE_CHINTRAD: return 10002;
    case RTL_TEXTENCODING_APPLE_KOREAN: return 10003;
    case RTL_TEXTENCODING_APPLE_ARABIC: return 10004;
    case RTL_TEXTENCODING_APPLE_HEBREW: return 10005;
    case RTL_TEXTENCODING_APPLE_GREEK: return 10006;
    case RTL_TEXTENCODING_APPLE_CYRILLIC: return 10007;
    case RTL_TEXTENCODING_APPLE_CHINSIMP: return 10008;
    case RTL_TEXTENCODING_APPLE_ROMANIAN: return 10010;
    case RTL_TEXTENCODING_APPLE_UKRAINIAN: return 10017;
    case RTL_TEXTENCODING_APPLE_CENTEURO: return 10029;
    case RTL_TEXTENCODING_APPLE_ICELAND: return 10079;
    case RTL_TEXTENCODING_APPLE_TURKISH: return 10081;
    case RTL_TEXTENCODING_APPLE_CROATIAN: return 10082;
    case RTL_TEXTENCODING_ASCII_US: return 20127;
    case RTL_TEXTENCODING_KOI8_R: return 20866;
    case RTL_TEXTENCODING_KOI8_U: return 21866;
    case RTL_TEXTENCODING_ISO_8859_1: return 28591;
    case RTL_TEXTENCODING_ISO_8859_2: return 28592;
    case RTL_TEXTENCODING_ISO_8859_3: return 28593;
    case RTL_TEXTENCODING_ISO_8859_4: return 28594;
    case RTL_TEXTENCODING_ISO_8859_5: return 28595;
    case RTL_TEXTENCODING_ISO_8859_6: return 28596;
    case RTL_TEXTENCODING_ISO_8859_7: return 28597;
    case RTL_TEXTENCODING_ISO_8859_8: return 28598;
    case RTL_TEXTENCODING_ISO_8859_9: return 28599;
    case RTL_TEXTENCODING_ISO_8859_15: return 28605;
    case RTL_TEXTENCODING_ISO_2022_JP: return 50220;
    case RTL_TEXTENCODING_ISO_2022_KR: return 50225;
    case RTL_TEXTENCODING_EUC_JP: return 51932;
    case RTL_TEXTENCODING_EUC_CN: return 51936;
    case RTL_TEXTENCODING_EUC_KR: return 51949;
    case RTL_TEXTENCODING_ISCII_DEVANAGARI: return 57002;
    case RTL_TEXTENCODING_UTF7: return 65000;
    case RTL_TEXTENCODING_UTF8: return 65001;
    default: return 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
