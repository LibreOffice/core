/*************************************************************************
 *
 *  $RCSfile: xlfd_extd.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:11:01 $
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef XLFD_ATTRIBUTE_HXX
#include "xlfd_attr.hxx"
#endif
#ifndef XLFD_SIMPLE_HXX
#include "xlfd_smpl.hxx"
#endif
#ifndef XLFD_EXTENDED_HXX
#include "xlfd_extd.hxx"
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#include <algorithm>

// --------------------------------------------------------------------------
//
//  classes for Xlfd handling that contain more than a single encoding.
//  Members that may vary through different encodings are stored in
//  a mpEncodingInfo member. There are three different classes:
//  true scalable fonts (truetype and type1) scalable bitmap fonts
//  (the ugly ones) and bitmap fonts. The ExtendedXlfd stores all the members
//  that are specific to a font outline
//  ( e.g. adobe-times-roman-medium-r-normal- X -p- X )
//  and specifies the interface.
//
// --------------------------------------------------------------------------

ExtendedXlfd::EncodingInfo&
ExtendedXlfd::EncodingInfo::operator= ( const Xlfd *pXlfd )
{
    mcSpacing     = pXlfd->mcSpacing;
    mnResolutionX = pXlfd->mnResolutionX;
    mnResolutionY = pXlfd->mnResolutionY;
    mnAddstyle    = pXlfd->mnAddstyle;
    mnCharset     = pXlfd->mnCharset;

    mnEncoding    = pXlfd->GetEncoding();

    return *this;
}

ExtendedXlfd::EncodingInfo&
ExtendedXlfd::EncodingInfo::operator= ( const ExtendedXlfd::EncodingInfo& rInfo )
{
    mcSpacing     = rInfo.mcSpacing;
    mnResolutionX = rInfo.mnResolutionX;
    mnResolutionY = rInfo.mnResolutionY;
    mnAddstyle    = rInfo.mnAddstyle;
    mnCharset     = rInfo.mnCharset;
    mnEncoding    = rInfo.mnEncoding;

    return *this;
}
// ------ base class --------------------------------------------------------

ExtendedXlfd::ExtendedXlfd()
{
    mpEncodingInfo = NULL;
    mnEncodings    = 0;
}

ExtendedXlfd::~ExtendedXlfd()
{
    if ( mnEncodings != 0 )
        rtl_freeMemory( mpEncodingInfo );
}

inline void*
Realloc( void *pPtr, sal_Size nSize )
{
    return rtl_reallocateMemory( pPtr, nSize );
}

int
ExtendedXlfd::GetEncodingIdx( rtl_TextEncoding nEncoding ) const
{
    for ( int i = 0; i < mnEncodings; i++ )
        if ( nEncoding == mpEncodingInfo[i].mnEncoding )
            return i;
    return -1;
}

Bool
ExtendedXlfd::HasEncoding( rtl_TextEncoding nEncoding ) const
{
    return !(GetEncodingIdx( nEncoding ) < 0) ;
}

rtl_TextEncoding
ExtendedXlfd::GetEncoding( int i ) const
{
    if ( i < mnEncodings && i >= 0 )
        return mpEncodingInfo[i].mnEncoding;

    return RTL_TEXTENCODING_DONTKNOW;
}

rtl_TextEncoding
ExtendedXlfd::GetEncoding() const
{
    return mnEncodings == 1 ? mpEncodingInfo[0].mnEncoding : RTL_TEXTENCODING_DONTKNOW;
}

// query the most unicode / Ascii compatible font: either one of the fonts
// is utf16 encoded or there's a single byte font which is unicode
// compatible for the first 256 chars (latin1) or for at least 128
// chars (most latin-X encodings, cyrillic encodings)
rtl_TextEncoding
ExtendedXlfd::GetAsciiEncoding( int *pAsciiRange ) const
{
    rtl_TextEncoding nBestEncoding = RTL_TEXTENCODING_DONTKNOW;
    int nLargestRange = 0x0000;

    for ( int i = 0; i < mnEncodings && nLargestRange < 0xffff; i++ )
    {
        rtl_TextEncoding nCurEncoding = mpEncodingInfo[i].mnEncoding;
        switch ( nCurEncoding )
        {
            case RTL_TEXTENCODING_UNICODE:
                nLargestRange = 0xffff;
                nBestEncoding = nCurEncoding;
                break;

            case RTL_TEXTENCODING_ISO_8859_1:
            case RTL_TEXTENCODING_MS_1252:
                nLargestRange = 0x00ff;
                nBestEncoding = nCurEncoding;
                break;

            case RTL_TEXTENCODING_ISO_8859_2:
            case RTL_TEXTENCODING_ISO_8859_4:
            case RTL_TEXTENCODING_ISO_8859_5:
            case RTL_TEXTENCODING_ISO_8859_6:
            case RTL_TEXTENCODING_ISO_8859_7:
            case RTL_TEXTENCODING_ISO_8859_8:
            case RTL_TEXTENCODING_ISO_8859_9:
            case RTL_TEXTENCODING_ISO_8859_13:
            case RTL_TEXTENCODING_ISO_8859_15:
            case RTL_TEXTENCODING_MS_1251:
            case RTL_TEXTENCODING_KOI8_R:
            case RTL_TEXTENCODING_JIS_X_0201:
                if ( nLargestRange < 0x0080 )
                {
                    nLargestRange = 0x0080;
                    nBestEncoding = nCurEncoding;
                }
                break;

            default:
                if ( nLargestRange == 0x0000 )
                {
                    nBestEncoding = nCurEncoding;
                }
                break;
        }
    }

    if ( pAsciiRange != NULL )
        *pAsciiRange = nLargestRange;

    return nBestEncoding;
}

Bool
ExtendedXlfd::AddEncoding( const Xlfd *pXlfd )
{
    rtl_TextEncoding nEncoding = pXlfd->GetEncoding();

    if ( HasEncoding(nEncoding) )
        return False;

    if ( mnEncodings == 0 )
    {
        // bootstrap
        mnFoundry  = pXlfd->mnFoundry;
        mnFamily   = pXlfd->mnFamily;
        mnWeight   = pXlfd->mnWeight;
        mnSlant    = pXlfd->mnSlant;
        mnSetwidth = pXlfd->mnSetwidth;
        mpFactory  = pXlfd->mpFactory;
    }

    mpEncodingInfo = (EncodingInfo*)Realloc( mpEncodingInfo,
            (mnEncodings + 1) * sizeof(EncodingInfo) );
    mpEncodingInfo[ mnEncodings ] = pXlfd;
    mnEncodings += 1;

    return True;
}

void
ExtendedXlfd::ToString( ByteString &rString,
        unsigned short nPixelSize, rtl_TextEncoding nEncoding ) const
{
    AppendAttribute( mpFactory->RetrieveFoundry(mnFoundry),   rString );
    AppendAttribute( mpFactory->RetrieveFamily(mnFamily),     rString );
    AppendAttribute( mpFactory->RetrieveWeight(mnWeight),     rString );
    AppendAttribute( mpFactory->RetrieveSlant(mnSlant),       rString );
    AppendAttribute( mpFactory->RetrieveSetwidth(mnSetwidth), rString );
}

void
ExtendedXlfd::ToString( ByteString &rString,
        unsigned short nPixelSize, char* pMatricsString, rtl_TextEncoding nEncoding ) const
{
    AppendAttribute( mpFactory->RetrieveFoundry(mnFoundry),   rString );
    AppendAttribute( mpFactory->RetrieveFamily(mnFamily),     rString );
    AppendAttribute( mpFactory->RetrieveWeight(mnWeight),     rString );
    AppendAttribute( mpFactory->RetrieveSlant(mnSlant),       rString );
    AppendAttribute( mpFactory->RetrieveSetwidth(mnSetwidth), rString );
}

// interface to the independent vcl class implfontdata
// this must not be called if mnEncodings is zero
void
ExtendedXlfd::ToImplFontData( ImplFontData *pFontData ) const
{
    pFontData->mpSysData = (void*)this;

    Attribute *pFamilyAttr = mpFactory->RetrieveFamily( mnFamily );
    Attribute *pWeightAttr = mpFactory->RetrieveWeight( mnWeight );
    Attribute *pWidthAttr  = mpFactory->RetrieveSetwidth( mnSetwidth );
    Attribute *pSlantAttr  = mpFactory->RetrieveSlant( mnSlant );

    pFontData->meFamily    = GetFamily();
    pFontData->meWeight    = GetWeight();
    pFontData->meItalic    = GetItalic();
    pFontData->meWidthType = GetWidth();

    // family name
    pFontData->maName      = pFamilyAttr->GetAnnotation();

    // the helvetica narrow hack
    if (  ! pFamilyAttr->HasFeature(XLFD_FEATURE_NARROW)
        &&  pWidthAttr->HasFeature(XLFD_FEATURE_NARROW) )
    {
        static const String aNarrow( RTL_CONSTASCII_USTRINGPARAM(" Narrow") );
        pFontData->maName += aNarrow;
    }

    // stylename = weight + slant + width
    // XXX Fix me: there may be a space missing between them
    String aStyleName;
    if ( pFontData->meWeight != WEIGHT_NORMAL )
        aStyleName += pWeightAttr->GetAnnotation();
    if ( pFontData->meItalic != ITALIC_NONE )
        aStyleName += pSlantAttr->GetAnnotation();
    if (   (  pFontData->meWidthType != WIDTH_NORMAL)
        && (! pWidthAttr->HasFeature(XLFD_FEATURE_NARROW)) )
        aStyleName += pWidthAttr->GetAnnotation();
    pFontData->maStyleName = aStyleName;

    pFontData->meCharSet     = GetEncoding();
    pFontData->mbOrientation = TRUE;

    // printer resident font / downloadable
    pFontData->mbDevice      = TRUE;

    // spacing
    pFontData->mePitch       = GetSpacing();

}

static FontPitch
GetPitch( const char cSpacing )
{
    switch ( cSpacing )
    {
        case 'c':
        case 'm': return PITCH_FIXED;
        case 'p': return PITCH_VARIABLE;
        default:  return PITCH_DONTKNOW;
    }
}

// you must not call any of the ExtendedXlfd::GetXXX() functions if the
// ExtendedXlfd is really empty (i.e. mnEncodings is zero)

FontPitch
ExtendedXlfd::GetSpacing() const
{
    if ( mnEncodings  > 1 )
        return PITCH_VARIABLE;
    if ( mnEncodings == 1 )
        return GetPitch( mpEncodingInfo[0].mcSpacing );

    return PITCH_DONTKNOW;
}

FontPitch
ExtendedXlfd::GetSpacing( rtl_TextEncoding nEncoding ) const
{
    for ( int nIdx = 0; nIdx < mnEncodings; nIdx++ )
    {
        if ( mpEncodingInfo[nIdx].mnEncoding == nEncoding )
            return GetPitch( mpEncodingInfo[nIdx].mcSpacing );
    }
    return PITCH_DONTKNOW;
}

FontFamily
ExtendedXlfd::GetFamily() const
{
    Attribute *pFamilyAttr= mpFactory->RetrieveFamily(mnFamily);
    return (FontFamily)pFamilyAttr->GetValue();
}

FontWeight
ExtendedXlfd::GetWeight() const
{
    Attribute *pWeightAttr = mpFactory->RetrieveWeight(mnWeight);
    return (FontWeight)pWeightAttr->GetValue();
}

FontItalic
ExtendedXlfd::GetItalic() const
{
    Attribute *pSlantAttr  = mpFactory->RetrieveSlant(mnSlant);
    return (FontItalic)pSlantAttr->GetValue();
}

FontWidth
ExtendedXlfd::GetWidth() const
{
    Attribute *pWidthAttr  = mpFactory->RetrieveSetwidth(mnSetwidth);
    return (FontWidth)pWidthAttr->GetValue();
}

struct CodeRange
{
    sal_uInt32 mnMin, mnEnd;
    bool operator<( const CodeRange& b ) const { return mnMin < b.mnMin; }
};

int ExtendedXlfd::GetFontCodeRanges( sal_uInt32* pCodePairs ) const
{
    int nRangeCount = 0;

    // approximate unicode ranges from encodings
    sal_uInt32* pRange = pCodePairs;
    for( unsigned short i = 0; i < mnEncodings; ++i )
    {
        sal_uInt32 pBuffer[32];
        if( !pCodePairs )
            pRange = pBuffer;
        sal_uInt32* pOldRange = pRange;

        // TODO: move encoding -> unicode range mapping to RTL
        // NOTE: for now only some are VERY roughly approximated
        switch( mpEncodingInfo[i].mnEncoding )
        {
            case RTL_TEXTENCODING_SYMBOL: // postscript symbol encoding
                *(pRange++) = 0xF020; *(pRange++) = 0xF100;
                break;
            case RTL_TEXTENCODING_ISO_8859_15:
                *(pRange++) = 0x20AC; *(pRange++) = 0x20AD; // Euro currency symbol
                // fall through
            case RTL_TEXTENCODING_APPLE_ROMAN:
            case RTL_TEXTENCODING_ISO_8859_1:
            case RTL_TEXTENCODING_MS_1252:
            case RTL_TEXTENCODING_IBM_437:
            case RTL_TEXTENCODING_IBM_852:
                *(pRange++) = 0x0020; *(pRange++) = 0x0080;
                *(pRange++) = 0x00A0; *(pRange++) = 0x0100;
                break;

            // Cyrillic
            case RTL_TEXTENCODING_APPLE_CYRILLIC:
            case RTL_TEXTENCODING_APPLE_UKRAINIAN:
            case RTL_TEXTENCODING_ISO_8859_5:
            case RTL_TEXTENCODING_KOI8_R:
            case RTL_TEXTENCODING_IBM_855:
            case RTL_TEXTENCODING_IBM_866:
                *(pRange++) = 0x0020; *(pRange++) = 0x0080;
                *(pRange++) = 0x0400; *(pRange++) = 0x04AF;
                *(pRange++) = 0x2116; *(pRange++) = 0x2117;
                break;

            // Greek
            case RTL_TEXTENCODING_APPLE_GREEK:
            case RTL_TEXTENCODING_ISO_8859_7:
            case RTL_TEXTENCODING_IBM_737:
            case RTL_TEXTENCODING_IBM_869:
            case RTL_TEXTENCODING_MS_1253:
                *(pRange++) = 0x0020; *(pRange++) = 0x0080;
                *(pRange++) = 0x00A0; *(pRange++) = 0x0100;
                *(pRange++) = 0x0370; *(pRange++) = 0x0400;
                *(pRange++) = 0x2015; *(pRange++) = 0x2020;
                break;

            // Turkish
            case RTL_TEXTENCODING_APPLE_TURKISH:
            case RTL_TEXTENCODING_IBM_857:
            case RTL_TEXTENCODING_ISO_8859_9:
            case RTL_TEXTENCODING_MS_1254:
                *(pRange++) = 0x0020; *(pRange++) = 0x0080;
                *(pRange++) = 0x00A0; *(pRange++) = 0x0160;
                break;

            // misc 8bit encodings
            case RTL_TEXTENCODING_APPLE_CENTEURO:
            case RTL_TEXTENCODING_APPLE_CROATIAN:
            case RTL_TEXTENCODING_APPLE_ICELAND:
            case RTL_TEXTENCODING_APPLE_ROMANIAN:
            case RTL_TEXTENCODING_ISO_8859_2:
            case RTL_TEXTENCODING_ISO_8859_3:
            case RTL_TEXTENCODING_ISO_8859_4:
            case RTL_TEXTENCODING_ISO_8859_10:
            case RTL_TEXTENCODING_ISO_8859_13:
            case RTL_TEXTENCODING_IBM_775:
            case RTL_TEXTENCODING_IBM_850:
            case RTL_TEXTENCODING_IBM_860:
            case RTL_TEXTENCODING_IBM_861:
            case RTL_TEXTENCODING_IBM_863:
            case RTL_TEXTENCODING_IBM_865:
            case RTL_TEXTENCODING_MS_1250:
            case RTL_TEXTENCODING_MS_1251:
            case RTL_TEXTENCODING_MS_1257:
            case RTL_TEXTENCODING_MS_1258:
                *(pRange++) = 0x0020; *(pRange++) = 0x0080;
                *(pRange++) = 0x00A0; *(pRange++) = 0x02EA;
                break;

            case RTL_TEXTENCODING_ISO_8859_14:
                *(pRange++) = 0x0020; *(pRange++) = 0x0080;
                *(pRange++) = 0x00A0; *(pRange++) = 0x0100;
                *(pRange++) = 0x1E00; *(pRange++) = 0x1F00;
                break;

            // Traditional, Simplified, Japanese
            case RTL_TEXTENCODING_APPLE_CHINSIMP:
            case RTL_TEXTENCODING_APPLE_CHINTRAD:
            case RTL_TEXTENCODING_APPLE_JAPANESE:
            case RTL_TEXTENCODING_SHIFT_JIS:
            case RTL_TEXTENCODING_GB_2312:
            case RTL_TEXTENCODING_GBT_12345:
            case RTL_TEXTENCODING_GBK:
            case RTL_TEXTENCODING_BIG5:
            case RTL_TEXTENCODING_EUC_JP:
            case RTL_TEXTENCODING_EUC_CN:
            case RTL_TEXTENCODING_EUC_TW:
            case RTL_TEXTENCODING_ISO_2022_JP:
            case RTL_TEXTENCODING_ISO_2022_CN:
            case RTL_TEXTENCODING_GB_18030:
            case RTL_TEXTENCODING_BIG5_HKSCS:
            case RTL_TEXTENCODING_JIS_X_0201:
            case RTL_TEXTENCODING_JIS_X_0208:
            case RTL_TEXTENCODING_JIS_X_0212:
            case RTL_TEXTENCODING_MS_932:
            case RTL_TEXTENCODING_MS_936:
            case RTL_TEXTENCODING_MS_950:
                *(pRange++) = 0x3000; *(pRange++) = 0xA000;
                *(pRange++) = 0xF900; *(pRange++) = 0xFB00;
                break;

            // Korean
            case RTL_TEXTENCODING_APPLE_KOREAN:
            case RTL_TEXTENCODING_MS_949:
            case RTL_TEXTENCODING_MS_1361:
            case RTL_TEXTENCODING_EUC_KR:
            case RTL_TEXTENCODING_ISO_2022_KR:
                *(pRange++) = 0x1100; *(pRange++) = 0x1200;
                *(pRange++) = 0x3130; *(pRange++) = 0x3190;
                *(pRange++) = 0xAC00; *(pRange++) = 0xD7A4;
                break;

            // Arabic
            case RTL_TEXTENCODING_APPLE_ARABIC:
            case RTL_TEXTENCODING_APPLE_FARSI:
            case RTL_TEXTENCODING_ISO_8859_6:
            case RTL_TEXTENCODING_IBM_864:
            case RTL_TEXTENCODING_MS_1256:
                *(pRange++) = 0x0600; *(pRange++) = 0x0700;
                *(pRange++) = 0xFB50; *(pRange++) = 0xFE00;
                *(pRange++) = 0xFE70; *(pRange++) = 0xFF00;
                break;

            // Hebrew
            case RTL_TEXTENCODING_APPLE_HEBREW:
            case RTL_TEXTENCODING_ISO_8859_8:
            case RTL_TEXTENCODING_IBM_862:
            case RTL_TEXTENCODING_MS_1255:
                *(pRange++) = 0x0590; *(pRange++) = 0x0600;
                *(pRange++) = 0xFB1D; *(pRange++) = 0xFB50;
                break;

            // Indic
            case RTL_TEXTENCODING_APPLE_GURMUKHI:
            case RTL_TEXTENCODING_APPLE_DEVANAGARI:
            case RTL_TEXTENCODING_APPLE_GUJARATI:
                *(pRange++) = 0x0900; *(pRange++) = 0x0B00;
                break;

            // Thai
            case RTL_TEXTENCODING_APPLE_THAI:
            case RTL_TEXTENCODING_TIS_620:
            case RTL_TEXTENCODING_MS_874:
                *(pRange++) = 0x0E00; *(pRange++) = 0x0E80;
                break;

            // Unicode
            case RTL_TEXTENCODING_UNICODE:
            case RTL_TEXTENCODING_UTF7:
            case RTL_TEXTENCODING_UTF8:
                *(pRange++) = 0x0020; *(pRange++) = 0xFFFF;
                break;

            case RTL_TEXTENCODING_DONTKNOW:
                *(pRange++) = 0x0020; *(pRange++) = 0xFFFF;
                break;
            default:
                // *(pRange++) = 0x0020; *(pRange++) = 0xFFFF;
                break;
        }

        nRangeCount += (pRange - pOldRange) / 2;
    }

    // sort and merge the code pairs
    if( pCodePairs && nRangeCount )
    {
        CodeRange* pSrc = (CodeRange*)pCodePairs;
        CodeRange* pEnd = pSrc + nRangeCount;
        ::std::sort( pSrc, pEnd );
        for( CodeRange* pDst = pSrc; ++pSrc < pEnd; )
        {
            if( pDst->mnEnd < pSrc->mnMin )
                ++pDst;
            else
            {
                // merge overlapping ranges
                if( pDst->mnEnd < pSrc->mnEnd )
                    pDst->mnEnd = pSrc->mnEnd;
                --nRangeCount;
            }
        }
    }

    return nRangeCount;
}

#if OSL_DEBUG_LEVEL > 1
void
ExtendedXlfd::Dump() const
{
    for ( int i = 0; i < mnEncodings; i++ )
    {
        ByteString aString;
        ToString( aString, 0, mpEncodingInfo[i].mnEncoding );
        fprintf( stderr, "%s\n", aString.GetBuffer() );
    }
}
#endif

// ------ class to handle scalable bitmap fonts ------------------------------

ScalableBitmapXlfd::ScalableBitmapXlfd()
{
}

ScalableBitmapXlfd::~ScalableBitmapXlfd()
{
}

void
ScalableBitmapXlfd::ToString( ByteString &rString,
        unsigned short nPixelSize, rtl_TextEncoding nEncoding ) const
{
    int nIdx = GetEncodingIdx( nEncoding );
    if ( nIdx < 0 )
        return;

    ExtendedXlfd::ToString( rString, nPixelSize, nEncoding );
    EncodingInfo& rInfo = mpEncodingInfo[ nIdx ];

    AppendAttribute( mpFactory->RetrieveAddstyle(rInfo.mnAddstyle), rString );

    rString += '-';
    rString += ByteString::CreateFromInt32( nPixelSize );
    rString += "-0-";
    rString += ByteString::CreateFromInt32( rInfo.mnResolutionX );
    rString += '-';
    rString += ByteString::CreateFromInt32( rInfo.mnResolutionY );
    rString += '-';
    rString += rInfo.mcSpacing;
    rString += "-0";

    AppendAttribute( mpFactory->RetrieveCharset(rInfo.mnCharset), rString );
}

void
ScalableBitmapXlfd::ToString( ByteString &rString,
        unsigned short nPixelSize, char *pMatricsString, rtl_TextEncoding nEncoding ) const
{
    int nIdx = GetEncodingIdx( nEncoding );
    if ( nIdx < 0 )
        return;

    ExtendedXlfd::ToString( rString, nPixelSize, nEncoding );
    EncodingInfo& rInfo = mpEncodingInfo[ nIdx ];

    AppendAttribute( mpFactory->RetrieveAddstyle(rInfo.mnAddstyle), rString );

    rString += "-*-";
    char pTmp[256];
    sprintf( pTmp, pMatricsString, nPixelSize, nPixelSize );
    rString += pTmp;
    rString += "-*-*-";
    rString += rInfo.mcSpacing;
    rString += "-*";

    AppendAttribute( mpFactory->RetrieveCharset(rInfo.mnCharset), rString );
}

void
BitmapXlfd::ToString( ByteString &rString,
        unsigned short nPixelSize, char *pMatricsString, rtl_TextEncoding nEncoding ) const
{
    int nIdx = GetEncodingIdx( nEncoding );
    if ( nIdx < 0 )
        return;

    ExtendedXlfd::ToString( rString, nPixelSize, nEncoding );
    EncodingInfo& rInfo = mpEncodingInfo[ nIdx ];

    AppendAttribute( mpFactory->RetrieveAddstyle(rInfo.mnAddstyle), rString );

    rString += "-*-";
    char pTmp[256];
    sprintf( pTmp, pMatricsString, nPixelSize, nPixelSize );
    rString += pTmp;
    rString += "-*-*-";
    rString += rInfo.mcSpacing;
    rString += "-*";

    AppendAttribute( mpFactory->RetrieveCharset(rInfo.mnCharset), rString );
}

void
ScalableBitmapXlfd::ToImplFontData( ImplFontData *pFontData ) const
{
    ExtendedXlfd::ToImplFontData( pFontData );

    pFontData->meType   = TYPE_SCALABLE;
    pFontData->mnWidth  = 0;
    pFontData->mnHeight = 0;
    pFontData->mnQuality= 0;
}

// ------ class to handle true bitmap fonts ----------------------------------

BitmapXlfd::BitmapXlfd( )
{
}

BitmapXlfd::~BitmapXlfd( )
{
}

Bool
BitmapXlfd::AddEncoding( const Xlfd *pXlfd )
{
    if ( mnEncodings == 0 )
    {
        mnPixelSize    = pXlfd->mnPixelSize;
        mnPointSize    = pXlfd->mnPointSize;
        mnAverageWidth = pXlfd->mnAverageWidth;
    }

    return ExtendedXlfd::AddEncoding( pXlfd );
}

Bool
BitmapXlfd::AddEncoding( const ScalableXlfd *pXlfd )
{
    if ( mnEncodings == 0 )
        return False;

    for (int i = 0; i < pXlfd->NumEncodings(); i++)
    {
        rtl_TextEncoding nEncoding = pXlfd->GetEncoding(i);
        if ( HasEncoding(nEncoding) )
            continue;

        mpEncodingInfo = (EncodingInfo*)Realloc( mpEncodingInfo,
                (mnEncodings + 1) * sizeof(EncodingInfo) );
        mpEncodingInfo[mnEncodings] = pXlfd->mpEncodingInfo[i];
        mnEncodings += 1;
    }

    return True;
}

void
BitmapXlfd::ToString( ByteString &rString,
        unsigned short nPixelSize, rtl_TextEncoding nEncoding ) const
{
    int nIdx = GetEncodingIdx( nEncoding );
    if ( nIdx < 0 )
        return;

    ExtendedXlfd::ToString( rString, nPixelSize, nEncoding );
    EncodingInfo& rInfo = mpEncodingInfo[ nIdx ];
    AppendAttribute( mpFactory->RetrieveAddstyle(rInfo.mnAddstyle), rString );
    rString += '-';
    rString += ByteString::CreateFromInt32( mnPixelSize );
    #ifdef __notdef__
        // since mnPointSize is not asked when comparing two bitmap font xlfd
        // it may differ (and it does) for two fonts with same pixelsize
        rString += "-";
        rString += ByteString::CreateFromInt32( mnPointSize );
        rString += "-";
        rString += ByteString::CreateFromInt32( rInfo.mnResolutionX );
        rString += '-';
        rString += ByteString::CreateFromInt32( rInfo.mnResolutionY );
        rString += '-';
    #else
        rString += "-*-*-*-";
    #endif /* __notdef__ */

    rString += rInfo.mcSpacing;

    #ifdef __notdef__
        rString += '-';
        rString += ByteString::CreateFromInt32( mnAverageWidth );
    #else
        rString += "-*";
    #endif

    AppendAttribute( mpFactory->RetrieveCharset(rInfo.mnCharset), rString );
}

void
BitmapXlfd::ToImplFontData( ImplFontData *pFontData ) const
{
    ExtendedXlfd::ToImplFontData( pFontData );

    pFontData->meType   = TYPE_RASTER;
    pFontData->mnWidth  = 0;
    pFontData->mnHeight = mnPixelSize;
    pFontData->mnQuality= 256;
}


// ------ class to handle true scalable fonts --------------------------------

ScalableXlfd::ScalableXlfd()
{}

ScalableXlfd::~ScalableXlfd()
{}

void
ScalableXlfd::ToString( ByteString &rString,
        unsigned short nPixelSize, rtl_TextEncoding nEncoding ) const
{
    int nIdx = GetEncodingIdx( nEncoding );
    if ( nIdx < 0 )
        return;

    ExtendedXlfd::ToString( rString, nPixelSize, nEncoding);

    EncodingInfo& rInfo = mpEncodingInfo[ nIdx ];
    AppendAttribute( mpFactory->RetrieveAddstyle(rInfo.mnAddstyle), rString );

    rString += '-';
    rString += ByteString::CreateFromInt32( nPixelSize );
    rString += "-0-0-0-";
    rString += rInfo.mcSpacing;
    rString += "-0";

    AppendAttribute( mpFactory->RetrieveCharset(rInfo.mnCharset), rString );
}

void
ScalableXlfd::ToString( ByteString &rString,
        unsigned short nPixelSize, char* pMatricsString, rtl_TextEncoding nEncoding ) const
{
    int nIdx = GetEncodingIdx( nEncoding );
    if ( nIdx < 0 )
        return;

    ExtendedXlfd::ToString( rString, nPixelSize, nEncoding);

    EncodingInfo& rInfo = mpEncodingInfo[ nIdx ];
    AppendAttribute( mpFactory->RetrieveAddstyle(rInfo.mnAddstyle), rString );

    rString += "-*-";
    char pTmp[256];
    sprintf( pTmp, pMatricsString, nPixelSize, nPixelSize );
    rString += pTmp;
    rString += "-*-*-";
    rString += rInfo.mcSpacing;
    rString += "-*";

    AppendAttribute( mpFactory->RetrieveCharset(rInfo.mnCharset), rString );
}

void
ScalableXlfd::ToImplFontData( ImplFontData *pFontData ) const
{
    ExtendedXlfd::ToImplFontData( pFontData );

    pFontData->meType   = TYPE_SCALABLE;
    pFontData->mnWidth  = 0;
    pFontData->mnHeight = 0;
    pFontData->mnQuality= 1024;
}

/* ------- virtual fonts for user interface ------------------------------- */

VirtualXlfd::ExtEncodingInfo&
VirtualXlfd::ExtEncodingInfo::operator= ( const Xlfd *pXlfd )
{
    mnFoundry  = pXlfd->mnFoundry;
    mnFamily   = pXlfd->mnFamily;
    mnWeight   = pXlfd->mnWeight;
    mnSlant    = pXlfd->mnSlant;
    mnSetwidth = pXlfd->mnSetwidth;

    return *this;
}

VirtualXlfd::ExtEncodingInfo&
VirtualXlfd::ExtEncodingInfo::operator= ( const VirtualXlfd::ExtEncodingInfo& rInfo )
{
    mnFoundry  = rInfo.mnFoundry;
    mnFamily   = rInfo.mnFamily;
    mnWeight   = rInfo.mnWeight;
    mnSlant    = rInfo.mnSlant;
    mnSetwidth = rInfo.mnSetwidth;

    return *this;
}

VirtualXlfd::VirtualXlfd() : mpExtEncodingInfo(NULL)
{
    mnFoundry  = 0;
    mnFamily   = 0;
    mnWeight   = 0;
    mnSlant    = 0;
    mnSetwidth = 0;
}

VirtualXlfd::~VirtualXlfd()
{
    if ( mpExtEncodingInfo != NULL )
        rtl_freeMemory( mpExtEncodingInfo );
}

int
VirtualXlfd::GetFontQuality (unsigned short nFamily)
{
    Attribute  *pFamily = mpFactory->RetrieveFamily(nFamily);
    int nQuality = 0;

    if (pFamily->HasFeature(XLFD_FEATURE_HQ))
        nQuality += 16;
    if (pFamily->HasFeature(XLFD_FEATURE_MQ))
        nQuality +=  8;
    if (pFamily->HasFeature(XLFD_FEATURE_LQ))
        nQuality +=  4;
    return nQuality;
}

Bool
VirtualXlfd::AddEncoding( const Xlfd *pXlfd )
{
    rtl_TextEncoding nEncoding = pXlfd->GetEncoding();
    int nIdx = GetEncodingIdx( nEncoding );

    if (mnEncodings == 0)
    {
        mnFoundry  = pXlfd->mnFoundry;
        mnFamily   = pXlfd->mnFamily;
        mnWeight   = pXlfd->mnWeight;
        mnSlant    = pXlfd->mnSlant;
        mnSetwidth = pXlfd->mnSetwidth;
        mpFactory  = pXlfd->mpFactory;
    }

    if ( nIdx < 0 /* !HasEncoding(nEncoding) */)
    {
        mpEncodingInfo = (EncodingInfo*)Realloc( mpEncodingInfo,
                (mnEncodings + 1) * sizeof(EncodingInfo) );
        mpEncodingInfo[ mnEncodings ] = pXlfd;
        mpExtEncodingInfo = (ExtEncodingInfo*)Realloc( mpExtEncodingInfo,
                (mnEncodings + 1) * sizeof(ExtEncodingInfo) );
        mpExtEncodingInfo[ mnEncodings ] = pXlfd;

        mnEncodings += 1;
    }
    else
    if (  GetFontQuality (pXlfd->mnFamily)
        > GetFontQuality (mpExtEncodingInfo[nIdx].mnFamily) )
    {
        mpEncodingInfo[ nIdx ]    = pXlfd;
        mpExtEncodingInfo[ nIdx ] = pXlfd;
    }

    return mnEncodings > 0 ? True : False;
}

void
VirtualXlfd::FilterInterfaceFont (const Xlfd *pXlfd)
{
    Attribute *pAttr;
    AttributeProvider *pFactory = pXlfd->mpFactory;

    if (! pXlfd->Fonttype() == TYPE_SCALABLE)
        return;
    pAttr = pFactory->RetrieveFamily(pXlfd->mnFamily);
    if (! pAttr->HasFeature(XLFD_FEATURE_INTERFACE_FONT))
        return;
    pAttr = pFactory->RetrieveSlant(pXlfd->mnSlant);
    if (! (FontItalic)pAttr->GetValue() == ITALIC_NONE)
        return;
    pAttr = pFactory->RetrieveSetwidth(pXlfd->mnSetwidth);
    if (pAttr->HasFeature(XLFD_FEATURE_NARROW))
        return;
    pAttr = pFactory->RetrieveWeight(pXlfd->mnWeight);
    FontWeight eWeight = (FontWeight)pAttr->GetValue();
    if ((eWeight != WEIGHT_NORMAL) && (eWeight != WEIGHT_MEDIUM))
        return;

    AddEncoding (pXlfd);
}

void
VirtualXlfd::ToString( ByteString &rString, unsigned short nPixelSize,
           rtl_TextEncoding nEncoding ) const
{
    int nIdx = GetEncodingIdx( nEncoding );
    if ( nIdx < 0 )
        return;

    ExtEncodingInfo &rExtInfo = mpExtEncodingInfo[ nIdx ];

    AppendAttribute( mpFactory->RetrieveFoundry(rExtInfo.mnFoundry),   rString );
    AppendAttribute( mpFactory->RetrieveFamily(rExtInfo.mnFamily),     rString );
    AppendAttribute( mpFactory->RetrieveWeight(rExtInfo.mnWeight),     rString );
    AppendAttribute( mpFactory->RetrieveSlant(rExtInfo.mnSlant),       rString );
    AppendAttribute( mpFactory->RetrieveSetwidth(rExtInfo.mnSetwidth), rString );

    EncodingInfo& rInfo = mpEncodingInfo[ nIdx ];
    AppendAttribute( mpFactory->RetrieveAddstyle(rInfo.mnAddstyle), rString );

    rString += '-';
    rString += ByteString::CreateFromInt32( nPixelSize );
    rString += "-0-0-0-";
    rString += rInfo.mcSpacing;
    rString += "-0";

    AppendAttribute( mpFactory->RetrieveCharset(rInfo.mnCharset), rString );
}

void
VirtualXlfd::ToString( ByteString &rString, unsigned short nPixelSize,
        char* pMatricsString, rtl_TextEncoding nEncoding ) const
{
    int nIdx = GetEncodingIdx( nEncoding );
    if ( nIdx < 0 )
        return;

    ExtEncodingInfo &rExtInfo = mpExtEncodingInfo[ nIdx ];

    AppendAttribute( mpFactory->RetrieveFoundry(rExtInfo.mnFoundry),   rString );
    AppendAttribute( mpFactory->RetrieveFamily(rExtInfo.mnFamily),     rString );
    AppendAttribute( mpFactory->RetrieveWeight(rExtInfo.mnWeight),     rString );
    AppendAttribute( mpFactory->RetrieveSlant(rExtInfo.mnSlant),       rString );
    AppendAttribute( mpFactory->RetrieveSetwidth(rExtInfo.mnSetwidth), rString );

    EncodingInfo& rInfo = mpEncodingInfo[ nIdx ];
    AppendAttribute( mpFactory->RetrieveAddstyle(rInfo.mnAddstyle), rString );

    rString += "-*-";
    char pTmp[256];
    sprintf( pTmp, pMatricsString, nPixelSize, nPixelSize );
    rString += pTmp;
    rString += "-*-*-";
    rString += rInfo.mcSpacing;
    rString += "-*";

    AppendAttribute( mpFactory->RetrieveCharset(rInfo.mnCharset), rString );
}

void
VirtualXlfd::ToImplFontData( ImplFontData *pFontData ) const
{
    pFontData->mpSysData = (void*)this;

    pFontData->meFamily    = FAMILY_SWISS;
    pFontData->meWeight    = WEIGHT_NORMAL;
    pFontData->meItalic    = ITALIC_NONE;
    pFontData->meWidthType = WIDTH_NORMAL;

    // family name
    static const String aFontName( RTL_CONSTASCII_USTRINGPARAM("Interface User") );
    pFontData->maName = aFontName;
    // pFontData->maStyleName = aStyleName;
    pFontData->meCharSet     =
    pFontData->mbOrientation = TRUE;
    pFontData->mbDevice      = TRUE;
    pFontData->mePitch       = PITCH_VARIABLE;
    pFontData->meType        = TYPE_SCALABLE;
    pFontData->mnWidth       = 0;
    pFontData->mnHeight      = 0;
    pFontData->mnQuality     = 1024;
}

// ------ font list -------------------------------------------------------

void
XlfdStorage::Dispose()
{
    for ( int i = 0; i < mnCount; i++ )
        delete mpList[i];
    if ( mnSize != 0 )
        rtl_freeMemory( mpList );

    mnCount = 0;
    mnSize  = 0;
    mpList  = NULL;
}

void
XlfdStorage::Reset()
{
    mnCount = 0;
}

XlfdStorage::~XlfdStorage()
{
    if ( mnSize != 0 )
        rtl_freeMemory( mpList );
}

XlfdStorage::XlfdStorage() :
    mnCount( 0 ),
    mnSize( 0 ),
    mpList( NULL )
{
}

void
XlfdStorage::Enlarge()
{
    if ( mnSize == 0 )
        mnSize  = 8;
    else
        mnSize = mnSize < 32768 ? (mnSize * 2) : 65535;

    mpList = (const ExtendedXlfd**)Realloc(mpList, mnSize * sizeof(ExtendedXlfd*));
}

void
XlfdStorage::Add( const ExtendedXlfd* pXlfd )
{
    if ( pXlfd == NULL )
        return;

    if ( mnCount >= mnSize )
        Enlarge();
    mpList[ mnCount++ ] = pXlfd;
}

void
XlfdStorage::Add( const XlfdStorage* pXlfd )
{
    if ( pXlfd == NULL )
        return;

    if ( pXlfd->mnCount != 0 )
    {
        unsigned short nNeeded = mnCount + pXlfd->mnCount;
        if ( mnSize <= nNeeded )
        {
            if ( mnSize == 0 )
                mnSize = pXlfd->mnSize;
            while ( mnSize <= nNeeded )
                mnSize = mnSize < 32768 ? (mnSize * 2) : 65535;
            mpList = (const ExtendedXlfd**)Realloc( mpList,
                    mnSize * sizeof(ExtendedXlfd*) );
        }
        memcpy( mpList + mnCount, pXlfd->mpList,
                pXlfd->mnCount * sizeof(ExtendedXlfd*) );
        mnCount += pXlfd->mnCount;
    }
}

#if OSL_DEBUG_LEVEL > 1
void
XlfdStorage::Dump() const
{
    for ( int i = 0; i < mnCount; i++ )
    {
        mpList[i]->Dump();
        fprintf(stderr, "\n" );
    }
}
#endif

const ExtendedXlfd*
XlfdStorage::Get( int nIdx ) const
{
    return nIdx >= 0 && nIdx < mnCount ? mpList[nIdx] : NULL ;
}

// ------ bitmap font list --------------------------------------------------

void
BitmapXlfdStorage::AddBitmapFont( const Xlfd *pXlfd )
{
    if ( pXlfd == NULL )
        return;

    unsigned short nSize = pXlfd->mnPixelSize;

    for ( int i = 0; i < mnCount; i++ )
        if ( nSize == ((BitmapXlfd*)mpList[i])->GetPixelSize() )
        {
            const_cast<ExtendedXlfd*>(mpList[i])->AddEncoding( pXlfd );
            return;
        }
    if ( mnCount >= mnSize )
        Enlarge();
    mpList[ mnCount ] = new BitmapXlfd();
    const_cast<ExtendedXlfd*>(mpList[ mnCount ])->AddEncoding( pXlfd );
    ++mnCount;
}
