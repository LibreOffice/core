/*************************************************************************
 *
 *  $RCSfile: strucvt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:09 $
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

// =======================================================================

static UniStringData* ImplGetUniStringDataFromString( const char* pStr, sal_Size nLen,
                                                      rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_ASSERT( (eTextEncoding != 9) &&
                (eTextEncoding != RTL_TEXTENCODING_DONTKNOW) &&
                (eTextEncoding != RTL_TEXTENCODING_UCS2) &&
                (eTextEncoding != RTL_TEXTENCODING_UCS4),
                "ByteString-->UniString: Wrong TextEncoding" );

    if ( !nLen )
    {
        ImplIncRefCount( &aImplEmptyStrData );
        return &aImplEmptyStrData;
    }

#ifndef NOOLDSTRING
    if ( eTextEncoding == CHARSET_SYSTEM )
        eTextEncoding = GetSystemCharSet();
#endif
    nCvtFlags |= RTL_TEXTTOUNICODE_FLAGS_FLUSH;

    UniStringData*              pData;
    rtl_TextEncodingInfo        aTextEncInfo;
    rtl_TextToUnicodeConverter  hConverter = rtl_createTextToUnicodeConverter( eTextEncoding );
    sal_uInt32                  nInfo;
    sal_Size                    nSrcBytes;
    sal_Size                    nDestChars;
    sal_Size                    nNewLen;

    // get TextEncodingInfo
    aTextEncInfo.StructSize = sizeof( aTextEncInfo );
    rtl_getTextEncodingInfo( eTextEncoding, &aTextEncInfo );

    // Zuerst konvertieren wir mit der wahrscheinlichen Anzahl
    // der zu konvertierenden Zeichen
    nNewLen = nLen/aTextEncInfo.AverageCharSize;
    if ( nNewLen > STRING_MAXLEN )
        nNewLen = STRING_MAXLEN;
    pData = ImplAllocData( (xub_StrLen)nNewLen );
    nDestChars = rtl_convertTextToUnicode( hConverter, 0,
                                           pStr, nLen,
                                           pData->maStr, nNewLen,
                                           nCvtFlags,
                                           &nInfo, &nSrcBytes );

    // Buffer hat nicht gereicht, dann mit maximaler Buffergroesse
    if ( (nInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL) &&
         (nNewLen != STRING_MAXLEN) )
    {
        rtl_freeMemory( pData );
        nNewLen = nLen;
        if ( nNewLen > STRING_MAXLEN )
            nNewLen = STRING_MAXLEN;
        pData = ImplAllocData( (xub_StrLen)nNewLen );
        nDestChars = rtl_convertTextToUnicode( hConverter, 0,
                                               pStr, nLen,
                                               pData->maStr, nNewLen,
                                               nCvtFlags,
                                               &nInfo, &nSrcBytes );
    }

    // String entsprechend der durch das Konvertieren tatsaechlich
    // entstehenden Bytes anpassen
    if ( !nDestChars )
    {
        rtl_freeMemory( pData );
        ImplIncRefCount( &aImplEmptyStrData );
        pData = &aImplEmptyStrData;
    }
    else if ( nNewLen > nDestChars+8 )
    {
        UniStringData* pTempData = ImplAllocData( (xub_StrLen)nDestChars );
        memcpy( pTempData->maStr, pData->maStr, nDestChars*sizeof( sal_Unicode ) );
        rtl_freeMemory( pData );
        pData = pTempData;
    }
    else
    {
        pData->mnLen = (xub_StrLen)nDestChars;
        pData->maStr[nDestChars] = '\0';
    }
    rtl_destroyTextToUnicodeConverter( hConverter );
    return pData;
}

// =======================================================================

void UniString::InitStringRes( const char* pUTF8Str, xub_StrLen nLen )
{
    DBG_CTOR( UniString, DbgCheckUniString );

    mpData = ImplGetUniStringDataFromString( pUTF8Str, nLen,
                                             RTL_TEXTENCODING_UTF8,
                                             RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |
                                             RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                             RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT );
}

// =======================================================================

UniString::UniString( const ByteString& rByteStr, rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( UniString, DbgCheckUniString );
    DBG_CHKOBJ( &rByteStr, ByteString, DbgCheckByteString );

    mpData = ImplGetUniStringDataFromString( rByteStr.mpData->maStr, rByteStr.mpData->mnLen,
                                             eTextEncoding, nCvtFlags );
}

// -----------------------------------------------------------------------

UniString::UniString( const ByteString& rByteStr, xub_StrLen nPos, xub_StrLen nLen,
                      rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( UniString, DbgCheckUniString );
    DBG_CHKOBJ( &rByteStr, ByteString, DbgCheckByteString );

    // Stringlaenge ermitteln
    if ( nPos > rByteStr.mpData->mnLen )
        nLen = 0;
    else
    {
        // Laenge korrigieren, wenn noetig
        xub_StrLen nMaxLen = rByteStr.mpData->mnLen-nPos;
        if ( nLen > nMaxLen )
            nLen = nMaxLen;
    }

    mpData = ImplGetUniStringDataFromString( rByteStr.mpData->maStr+nPos, nLen,
                                             eTextEncoding, nCvtFlags );
}

// -----------------------------------------------------------------------

UniString::UniString( const char* pByteStr,
                      rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( UniString, DbgCheckUniString );
    DBG_ASSERT( pByteStr, "UniString::UniString() - pByteStr is NULL" );

    mpData = ImplGetUniStringDataFromString( pByteStr, ImplStringLen( pByteStr ),
                                             eTextEncoding, nCvtFlags );
}

// -----------------------------------------------------------------------

UniString::UniString( const char* pByteStr, xub_StrLen nLen,
                      rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( UniString, DbgCheckUniString );
    DBG_ASSERT( pByteStr, "UniString::UniString() - pByteStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pByteStr );

    mpData = ImplGetUniStringDataFromString( pByteStr, nLen,
                                             eTextEncoding, nCvtFlags );
}

// =======================================================================

UniString::UniString( const NAMESPACE_RTL(OUString)& rStr )
{
    DBG_CTOR( UniString, DbgCheckUniString );

    mpData = (UniStringData*)rStr.pData;
    ImplIncRefCount( mpData );
}

// -----------------------------------------------------------------------

NAMESPACE_RTL(OUString)::OUString( const UniString& rStr )
{
    pData = (rtl_uString*)rStr.mpData;
    rtl_uString_acquire( pData );
}

// -----------------------------------------------------------------------

UniString& UniString::Assign( const NAMESPACE_RTL(OUString)& rStr )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    rtl_uString_acquire( rStr.pData );
    ImplDeleteData( mpData );
    mpData = (UniStringData*)rStr.pData;
    return *this;
}
