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

// no include "precompiled_tools.hxx" because this is included in other cxx files.

// -----------------------------------------------------------------------

void ByteString::ImplUpdateStringFromUniString(
    const sal_Unicode* pUniStr, sal_Size nUniLen,
    rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    ByteStringData* pNewStringData = NULL;
    rtl_uString2String( (rtl_String **)(&pNewStringData),
                        pUniStr, nUniLen,
                        eTextEncoding, nCvtFlags );
    STRING_RELEASE((STRING_TYPE *)mpData);
    mpData = pNewStringData;
}

// =======================================================================

ByteString::ByteString( const UniString& rUniStr, rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( ByteString, DbgCheckByteString );
    DBG_CHKOBJ( &rUniStr, UniString, DbgCheckUniString );

    mpData = NULL;
    rtl_uString2String( (rtl_String **)(&mpData),
                        rUniStr.mpData->maStr, rUniStr.mpData->mnLen,
                        eTextEncoding, nCvtFlags );
}

// -----------------------------------------------------------------------

ByteString::ByteString( const UniString& rUniStr, xub_StrLen nPos, xub_StrLen nLen,
                        rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( ByteString, DbgCheckByteString );
    DBG_CHKOBJ( &rUniStr, UniString, DbgCheckUniString );

    // Stringlaenge ermitteln
    if ( nPos > rUniStr.mpData->mnLen )
        nLen = 0;
    else
    {
        // Laenge korrigieren, wenn noetig
        sal_Int32 nMaxLen = rUniStr.mpData->mnLen-nPos;
        if ( nLen > nMaxLen )
            nLen = static_cast< xub_StrLen >(nMaxLen);
    }

    mpData = NULL;
    rtl_uString2String( (rtl_String **)(&mpData),
                        rUniStr.mpData->maStr+nPos, nLen,
                        eTextEncoding, nCvtFlags );
}

// -----------------------------------------------------------------------

ByteString::ByteString( const sal_Unicode* pUniStr,
                        rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( ByteString, DbgCheckByteString );
    DBG_ASSERT( pUniStr, "ByteString::ByteString() - pUniStr is NULL" );

    mpData = NULL;
    rtl_uString2String( (rtl_String **)(&mpData),
                        pUniStr, ImplStringLen( pUniStr ),
                        eTextEncoding, nCvtFlags );
}

// -----------------------------------------------------------------------

ByteString::ByteString( const sal_Unicode* pUniStr, xub_StrLen nLen,
                        rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( ByteString, DbgCheckByteString );
    DBG_ASSERT( pUniStr, "ByteString::ByteString() - pUniStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pUniStr );

    mpData = NULL;
    rtl_uString2String( (rtl_String **)(&mpData),
                        pUniStr, nLen,
                        eTextEncoding, nCvtFlags );
}

// =======================================================================

static sal_uChar aImplByteTab[256] =
{
    0,   1,   2,   3,   4,   5,   6,   7,
    8,   9,  10,  11,  12,  13,  14,  15,
   16,  17,  18,  19,  20,  21,  22,  23,
   24,  25,  26,  27,  28,  29,  30,  31,
   32,  33,  34,  35,  36,  37,  38,  39,
   40,  41,  42,  43,  44,  45,  46,  47,
   48,  49,  50,  51,  52,  53,  54,  55,
   56,  57,  58,  59,  60,  61,  62,  63,
   64,  65,  66,  67,  68,  69,  70,  71,
   72,  73,  74,  75,  76,  77,  78,  79,
   80,  81,  82,  83,  84,  85,  86,  87,
   88,  89,  90,  91,  92,  93,  94,  95,
   96,  97,  98,  99, 100, 101, 102, 103,
  104, 105, 106, 107, 108, 109, 110, 111,
  112, 113, 114, 115, 116, 117, 118, 119,
  120, 121, 122, 123, 124, 125, 126, 127,
  128, 129, 130, 131, 132, 133, 134, 135,
  136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151,
  152, 153, 154, 155, 156, 157, 158, 159,
  160, 161, 162, 163, 164, 165, 166, 167,
  168, 169, 170, 171, 172, 173, 174, 175,
  176, 177, 178, 179, 180, 181, 182, 183,
  184, 185, 186, 187, 188, 189, 190, 191,
  192, 193, 194, 195, 196, 197, 198, 199,
  200, 201, 202, 203, 204, 205, 206, 207,
  208, 209, 210, 211, 212, 213, 214, 215,
  216, 217, 218, 219, 220, 221, 222, 223,
  224, 225, 226, 227, 228, 229, 230, 231,
  232, 233, 234, 235, 236, 237, 238, 239,
  240, 241, 242, 243, 244, 245, 246, 247,
  248, 249, 250, 251, 252, 253, 254, 255
};

// =======================================================================

struct Impl1ByteUnicodeTabData
{
    rtl_TextEncoding            meTextEncoding;
    sal_Unicode                 maUniTab[256];
    Impl1ByteUnicodeTabData*    mpNext;
};

// -----------------------------------------------------------------------

struct Impl1ByteConvertTabData
{
    rtl_TextEncoding            meSrcTextEncoding;
    rtl_TextEncoding            meDestTextEncoding;
    sal_uChar                   maConvertTab[256];
    sal_uChar                   maRepConvertTab[256];
    Impl1ByteConvertTabData*    mpNext;
};

// =======================================================================

sal_Unicode* ImplGet1ByteUnicodeTab( rtl_TextEncoding eTextEncoding )
{
#ifndef BOOTSTRAP
    TOOLSINDATA*                pToolsData = ImplGetToolsInData();
#else
    TOOLSINDATA*                pToolsData = 0x0;
#endif
    Impl1ByteUnicodeTabData*    pTab = pToolsData->mpFirstUniTabData;

    while ( pTab )
    {
        if ( pTab->meTextEncoding == eTextEncoding )
            return pTab->maUniTab;
        pTab = pTab->mpNext;
    }

    // get TextEncodingInfo
    rtl_TextEncodingInfo aTextEncInfo;
    aTextEncInfo.StructSize = sizeof( aTextEncInfo );
    rtl_getTextEncodingInfo( eTextEncoding, &aTextEncInfo );

    if ( aTextEncInfo.MaximumCharSize == 1 )
    {
        pTab = new Impl1ByteUnicodeTabData;
        pTab->meTextEncoding = eTextEncoding;
        pTab->mpNext = pToolsData->mpFirstUniTabData;

        rtl_TextToUnicodeConverter  hConverter;
        sal_uInt32                  nInfo;
        sal_Size                    nSrcBytes;
        sal_Size                    nDestChars;
        hConverter = rtl_createTextToUnicodeConverter( eTextEncoding );
        nDestChars = rtl_convertTextToUnicode( hConverter, 0,
                                               (const sal_Char*)aImplByteTab, 256,
                                               pTab->maUniTab, 256,
                                               RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |
                                               RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                               RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT,
                                               &nInfo, &nSrcBytes );
        rtl_destroyTextToUnicodeConverter( hConverter );

        if ( (nSrcBytes != 256) || (nDestChars != 256) )
            delete pTab;
        else
        {
            pToolsData->mpFirstUniTabData = pTab;
            return pTab->maUniTab;
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------

static sal_uChar* ImplGet1ByteConvertTab( rtl_TextEncoding eSrcTextEncoding,
                                          rtl_TextEncoding eDestTextEncoding,
                                          sal_Bool bReplace )
{
#ifndef BOOTSTRAP
    TOOLSINDATA*                pToolsData = ImplGetToolsInData();
#else
    TOOLSINDATA*                pToolsData = 0x0;
#endif
    Impl1ByteConvertTabData*    pTab = pToolsData->mpFirstConvertTabData;

    while ( pTab )
    {
        if ( (pTab->meSrcTextEncoding == eSrcTextEncoding) &&
             (pTab->meDestTextEncoding == eDestTextEncoding) )
        {
            if ( bReplace )
                return pTab->maRepConvertTab;
            else
                return pTab->maConvertTab;
        }
        pTab = pTab->mpNext;
    }

    // get TextEncodingInfo
    rtl_TextEncodingInfo aTextEncInfo1;
    aTextEncInfo1.StructSize = sizeof( aTextEncInfo1 );
    rtl_getTextEncodingInfo( eSrcTextEncoding, &aTextEncInfo1 );
    rtl_TextEncodingInfo aTextEncInfo2;
    aTextEncInfo2.StructSize = sizeof( aTextEncInfo2 );
    rtl_getTextEncodingInfo( eDestTextEncoding, &aTextEncInfo2 );

    if ( (aTextEncInfo1.MaximumCharSize == 1) &&
         (aTextEncInfo2.MaximumCharSize == 1) )
    {
        pTab = new Impl1ByteConvertTabData;
        pTab->meSrcTextEncoding = eSrcTextEncoding;
        pTab->meDestTextEncoding = eDestTextEncoding;
        pTab->mpNext = pToolsData->mpFirstConvertTabData;

        rtl_TextToUnicodeConverter  hConverter;
        rtl_UnicodeToTextConverter  hConverter2;
        sal_uInt32                  nInfo;
        sal_Size                    nSrcBytes;
        sal_Size                    nDestChars;
        sal_Size                    nSrcChars;
        sal_Size                    nDestBytes;
        sal_Unicode                 aTempBuf[256];
        hConverter = rtl_createTextToUnicodeConverter( eSrcTextEncoding );
        nDestChars = rtl_convertTextToUnicode( hConverter, 0,
                                               (const sal_Char*)aImplByteTab, 256,
                                               aTempBuf, 256,
                                               RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT |
                                               RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                               RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT,
                                               &nInfo, &nSrcBytes );
        rtl_destroyTextToUnicodeConverter( hConverter );
        if ( (nSrcBytes != 256) || (nDestChars != 256) )
            delete pTab;
        else
        {
            hConverter2 = rtl_createUnicodeToTextConverter( eDestTextEncoding );
            nDestBytes = rtl_convertUnicodeToText( hConverter2, 0,
                                                   aTempBuf, 256,
                                                   (sal_Char*)pTab->maConvertTab, 256,
                                                   RTL_UNICODETOTEXT_FLAGS_UNDEFINED_0 |
                                                   RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT,
                                                   &nInfo, &nSrcChars );
            if ( (nDestBytes == 256) || (nSrcChars == 256) )
            {
                nDestBytes = rtl_convertUnicodeToText( hConverter2, 0,
                                                       aTempBuf, 256,
                                                       (sal_Char*)pTab->maRepConvertTab, 256,
                                                       RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |
                                                       RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT |
                                                       RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE,
                                                       &nInfo, &nSrcChars );
            }
            rtl_destroyUnicodeToTextConverter( hConverter2 );
            if ( (nDestBytes != 256) || (nSrcChars != 256) )
                delete pTab;
            else
            {
                pToolsData->mpFirstConvertTabData = pTab;
                if ( bReplace )
                    return pTab->maRepConvertTab;
                else
                    return pTab->maConvertTab;
            }
        }
    }

    return NULL;
}

// =======================================================================

void ImplDeleteCharTabData()
{
#ifndef BOOTSTRAP
    TOOLSINDATA*                pToolsData = ImplGetToolsInData();
#else
    TOOLSINDATA*                pToolsData = 0x0;
#endif
    Impl1ByteUnicodeTabData*    pTempUniTab;
    Impl1ByteUnicodeTabData*    pUniTab = pToolsData->mpFirstUniTabData;
    while ( pUniTab )
    {
        pTempUniTab = pUniTab->mpNext;
        delete pUniTab;
        pUniTab = pTempUniTab;
    }
    pToolsData->mpFirstUniTabData = NULL;

    Impl1ByteConvertTabData*    pTempConvertTab;
    Impl1ByteConvertTabData*    pConvertTab = pToolsData->mpFirstConvertTabData;
    while ( pConvertTab )
    {
        pTempConvertTab = pConvertTab->mpNext;
        delete pConvertTab;
        pConvertTab = pTempConvertTab;
    }
    pToolsData->mpFirstConvertTabData = NULL;
}

// =======================================================================

void ByteString::ImplStringConvert(
    rtl_TextEncoding eSource, rtl_TextEncoding eTarget, sal_Bool bReplace )
{
    sal_uChar* pConvertTab = ImplGet1ByteConvertTab( eSource, eTarget, bReplace );
    if ( pConvertTab )
    {
        char* pStr = mpData->maStr;
        while ( *pStr )
        {
            sal_uChar c = (sal_uChar)*pStr;
            sal_uChar cConv = pConvertTab[c];
            if ( c != cConv )
            {
                pStr = ImplCopyStringData( pStr );
                *pStr = (char)cConv;
            }

            pStr++;
        }
    }
    else
    {
        rtl_UnicodeToTextConverter  hSrcConverter = rtl_createTextToUnicodeConverter( eSource );
        sal_uInt32                  nInfo;
        sal_Size                    nSrcBytes;
        sal_Size                    nDestChars;
        sal_Size                    nTempLen;
        sal_Unicode*                pTempBuf;
        nTempLen = mpData->mnLen;
        pTempBuf = new sal_Unicode[nTempLen];
        nDestChars = rtl_convertTextToUnicode( hSrcConverter, 0,
                                               mpData->maStr, mpData->mnLen,
                                               pTempBuf, nTempLen,
                                               RTL_TEXTTOUNICODE_FLAGS_FLUSH |
                                               RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |
                                               RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                               RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT,
                                               &nInfo, &nSrcBytes );
        rtl_destroyTextToUnicodeConverter( hSrcConverter );
        // Hier werten wir bReplace nicht aus, da fuer MultiByte-Textencodings
        // sowieso keine Ersatzdarstellung moeglich ist. Da sich der String
        // sowieso in der Laenge aendern kann, nehmen wir auch sonst keine
        // Ruecksicht darauf, das die Laenge erhalten bleibt.
        ImplUpdateStringFromUniString( pTempBuf, nDestChars, eTarget,
                                       RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |
                                       RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT |
                                       RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE |
                                       RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR |
                                       RTL_UNICODETOTEXT_FLAGS_PRIVATE_MAPTO0 |
                                       RTL_UNICODETOTEXT_FLAGS_NONSPACING_IGNORE |
                                       RTL_UNICODETOTEXT_FLAGS_CONTROL_IGNORE );
        delete [] pTempBuf;
    }
}

// =======================================================================

ByteString& ByteString::Convert( rtl_TextEncoding eSource, rtl_TextEncoding eTarget, sal_Bool bReplace )
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    // rtl_TextEncoding Dontknow kann nicht konvertiert werden
    if ( (eSource == RTL_TEXTENCODING_DONTKNOW) || (eTarget == RTL_TEXTENCODING_DONTKNOW) )
        return *this;

    // Wenn Source und Target gleich sind, muss nicht konvertiert werden
    if ( eSource == eTarget )
        return *this;

    // rtl_TextEncoding Symbol nur nach Unicode oder von Unicode wandeln, ansonsten
    // wollen wir die Zeichencodes beibehalten
    if ( (eSource == RTL_TEXTENCODING_SYMBOL) &&
         (eTarget != RTL_TEXTENCODING_UTF7) &&
         (eTarget != RTL_TEXTENCODING_UTF8) )
        return *this;
    if ( (eTarget == RTL_TEXTENCODING_SYMBOL) &&
         (eSource != RTL_TEXTENCODING_UTF7) &&
         (eSource != RTL_TEXTENCODING_UTF8) )
        return *this;

    // Zeichensatz umwandeln
    ImplStringConvert( eSource, eTarget, bReplace );

    return *this;
}

// =======================================================================

char ByteString::Convert( char c,
                          rtl_TextEncoding eSource, rtl_TextEncoding eTarget,
                          sal_Bool bReplace )
{
    // TextEncoding Dontknow kann nicht konvertiert werden
    if ( (eSource == RTL_TEXTENCODING_DONTKNOW) || (eTarget == RTL_TEXTENCODING_DONTKNOW) )
        return '\0';

    // Wenn Source und Target gleich sind, muss nicht konvertiert werden
    if ( eSource == eTarget )
        return c;

    // TextEncoding Symbol nur nach Unicode oder von Unicode wandeln, ansonsten
    // wollen wir die Zeichencodes beibehalten
    if ( (eSource == RTL_TEXTENCODING_SYMBOL) &&
         (eTarget != RTL_TEXTENCODING_UTF7) &&
         (eTarget != RTL_TEXTENCODING_UTF8) )
        return '\0';
    if ( (eTarget == RTL_TEXTENCODING_SYMBOL) &&
         (eSource != RTL_TEXTENCODING_UTF7) &&
         (eSource != RTL_TEXTENCODING_UTF8) )
        return '\0';

    sal_uChar* pConvertTab = ImplGet1ByteConvertTab( eSource, eTarget, bReplace );
    if ( pConvertTab )
        return (char)pConvertTab[(sal_uChar)c];
    else
        return '\0';
}

// =======================================================================

sal_Unicode ByteString::ConvertToUnicode( char c, rtl_TextEncoding eTextEncoding )
{
    sal_Size nLen = 1;
    return ConvertToUnicode( &c, &nLen, eTextEncoding );
}

// -----------------------------------------------------------------------

char ByteString::ConvertFromUnicode( sal_Unicode c, rtl_TextEncoding eTextEncoding, sal_Bool bReplace )
{
    sal_Size    nLen;
    char        aBuf[30];
    nLen = ConvertFromUnicode( c, aBuf, sizeof( aBuf ), eTextEncoding, bReplace );
    if ( nLen == 1 )
        return aBuf[0];
    else
        return 0;
}

// -----------------------------------------------------------------------

sal_Unicode ByteString::ConvertToUnicode( const char* pChar, sal_Size* pLen, rtl_TextEncoding eTextEncoding )
{
    // TextEncoding Dontknow wird nicht konvertiert
    if ( eTextEncoding == RTL_TEXTENCODING_DONTKNOW )
        return 0;

    rtl_TextToUnicodeConverter  hConverter;
    sal_uInt32                  nInfo;
    sal_Size                    nSrcBytes;
    sal_Size                    nDestChars;
    sal_Unicode                 nConvChar;
    hConverter = rtl_createTextToUnicodeConverter( eTextEncoding );
    nDestChars = rtl_convertTextToUnicode( hConverter, 0,
                                           (const sal_Char*)pChar, *pLen,
                                           &nConvChar, 1,
                                           RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT |
                                           RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                           RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT |
                                           RTL_TEXTTOUNICODE_FLAGS_FLUSH,
                                           &nInfo, &nSrcBytes );
    rtl_destroyTextToUnicodeConverter( hConverter );

    if ( nDestChars == 1 )
    {
        *pLen = nSrcBytes;
        return nConvChar;
    }
    else
    {
        *pLen = 0;
        return 0;
    }
}

// -----------------------------------------------------------------------

sal_Size ByteString::ConvertFromUnicode( sal_Unicode c, char* pBuf, sal_Size nBufLen, rtl_TextEncoding eTextEncoding,
                                         sal_Bool bReplace )
{
    // TextEncoding Dontknow wird nicht konvertiert
    if ( eTextEncoding == RTL_TEXTENCODING_DONTKNOW )
        return '\0';

    rtl_UnicodeToTextConverter  hConverter;
    sal_uInt32                  nInfo;
    sal_Size                    nSrcChars;
    sal_Size                    nDestBytes;
    sal_Unicode                 cUni = c;
    sal_uInt32                  nFlags = RTL_UNICODETOTEXT_FLAGS_NONSPACING_IGNORE |
                                         RTL_UNICODETOTEXT_FLAGS_CONTROL_IGNORE |
                                         RTL_UNICODETOTEXT_FLAGS_FLUSH;
    if ( bReplace )
    {
        nFlags |= RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |
                  RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT;
        nFlags |= RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE;
        if ( nBufLen > 1 )
            nFlags |= RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR;
    }
    else
    {
        nFlags |= RTL_UNICODETOTEXT_FLAGS_UNDEFINED_0 |
                  RTL_UNICODETOTEXT_FLAGS_INVALID_0;
    }

    hConverter = rtl_createUnicodeToTextConverter( eTextEncoding );
    nDestBytes = rtl_convertUnicodeToText( hConverter, 0,
                                           &cUni, 1,
                                           (sal_Char*)pBuf, nBufLen,
                                           nFlags,
                                           &nInfo, &nSrcChars );
    rtl_destroyUnicodeToTextConverter( hConverter );
    return nDestBytes;
}

// =======================================================================

ByteString::ByteString( const rtl::OString& rStr )
    : mpData(NULL)
{
    DBG_CTOR( ByteString, DbgCheckByteString );

    OSL_ENSURE(rStr.pData->length < STRING_MAXLEN,
               "Overflowing rtl::OString -> ByteString cut to zero length");

    if (rStr.pData->length < STRING_MAXLEN)
    {
        mpData = reinterpret_cast< ByteStringData * >(const_cast< rtl::OString & >(rStr).pData);
        STRING_ACQUIRE((STRING_TYPE *)mpData);
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
}

// -----------------------------------------------------------------------

ByteString& ByteString::Assign( const rtl::OString& rStr )
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    OSL_ENSURE(rStr.pData->length < STRING_MAXLEN,
               "Overflowing rtl::OString -> ByteString cut to zero length");

    if (rStr.pData->length < STRING_MAXLEN)
    {
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = reinterpret_cast< ByteStringData * >(const_cast< rtl::OString & >(rStr).pData);
        STRING_ACQUIRE((STRING_TYPE *)mpData);
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }

    return *this;
}
