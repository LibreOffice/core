/*************************************************************************
 *
 *  $RCSfile: xlfd_extd.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: cp $ $Date: 2001-03-19 08:31:46 $
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
        free( mpEncodingInfo );
}

inline void*
Realloc( void *pPtr, sal_Size nSize )
{
    return pPtr == NULL ? malloc( nSize ) : realloc( pPtr, nSize );
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
    if ( mnEncodings  > 0 )
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

#ifdef DEBUG
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
{
}

ScalableXlfd::~ScalableXlfd()
{
}

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

// ------ printer fonts ---------------------------------------------------

PrinterFontXlfd::PrinterFontXlfd( )
{
}

PrinterFontXlfd::~PrinterFontXlfd( )
{
}

Bool
PrinterFontXlfd::AddEncoding( const Xlfd *pXlfd )
{
    rtl_TextEncoding nEncoding = pXlfd->GetEncoding();
    int nEncodingIdx = GetEncodingIdx( nEncoding );

    if ( nEncodingIdx == -1 )
    {
        // the encoding is new, so add it anyway
        return ExtendedXlfd::AddEncoding( pXlfd );
    }
    else
    {
        // we already have this encoding, check if we can replace
        // a soft font with a builtin font
        EncodingInfo& rInfo = mpEncodingInfo[ nEncodingIdx ];
        XlfdFonttype nNewType, nOldType;

        nOldType = rInfo.mnResolutionX == 0 && rInfo.mnResolutionY == 0 ?
                            eTypePrinterDownload : eTypePrinterBuiltIn;
        nNewType = pXlfd->Fonttype();

        if (   nOldType == eTypePrinterDownload
            && nNewType == eTypePrinterBuiltIn )
        {
            mpEncodingInfo[ nEncodingIdx ] = pXlfd;
        }
        return True;
    }

    return False;
}

/* ------- virtual fonts for user interface ------------------------------- */

VirtualXlfd::ExtEncodingInfo&
VirtualXlfd::ExtEncodingInfo::operator= ( const ExtendedXlfd *pXlfd )
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
        free (mpExtEncodingInfo);
}

Bool
VirtualXlfd::AddEncoding( const Xlfd *pXlfd )
{
    return False;
}

int
VirtualXlfd::GetFontQuality (unsigned short nFamily)
{
    Attribute  *pFamily  = mpFactory->RetrieveFamily(nFamily);

    if (pFamily->HasFeature(XLFD_FEATURE_INTERFACE_FONT_HIGQ))
        return 32;
    if (pFamily->HasFeature(XLFD_FEATURE_INTERFACE_FONT_MEDQ))
        return 16;
    return 0;
}

Bool
VirtualXlfd::AddEncoding( const ExtendedXlfd *pXlfd)
{
    mpFactory  = pXlfd->mpFactory;

    int nFontQuality = GetFontQuality (pXlfd->mnFamily);

    for (int i = 0; i < pXlfd->NumEncodings(); i++)
    {
        rtl_TextEncoding nEncoding = pXlfd->GetEncoding(i);
        int nIdx = GetEncodingIdx( nEncoding );
        if ( nIdx < 0 /* !HasEncoding(nEncoding) */)
        {
            /* XXX should be obsolete since all info is in mpExtEncodingInfo */
            mnFoundry  = pXlfd->mnFoundry;
            mnFamily   = pXlfd->mnFamily;
            mnWeight   = pXlfd->mnWeight;
            mnSlant    = pXlfd->mnSlant;
            mnSetwidth = pXlfd->mnSetwidth;
            /* XXX end of obsolete */

            mpEncodingInfo = (EncodingInfo*)Realloc( mpEncodingInfo,
                                        (mnEncodings + 1) * sizeof(EncodingInfo) );
            mpEncodingInfo[ mnEncodings ] = pXlfd->mpEncodingInfo[i];
            mpExtEncodingInfo = (ExtEncodingInfo*)Realloc( mpExtEncodingInfo,
                                        (mnEncodings + 1) * sizeof(ExtEncodingInfo) );
            mpExtEncodingInfo[ mnEncodings ] = pXlfd;

            mnEncodings++;
        }
        else
        if (nFontQuality > GetFontQuality (mpExtEncodingInfo[nIdx].mnFamily))
        {
            mpEncodingInfo[ nIdx ]    = pXlfd->mpEncodingInfo[i];
            mpExtEncodingInfo[ nIdx ] = pXlfd;
        }
    }

    return mnEncodings > 0 ? True : False;
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
        delete mpList;

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
        delete mpList;
}

XlfdStorage::XlfdStorage() :
    mnCount( 0 ),
    mnSize( 0 ),
    mpList( NULL ),
    mpInterfaceFont( NULL )
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

#ifdef DEBUG
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

void
XlfdStorage::InterfaceFont (AttributeProvider* pFactory)
{
    VirtualXlfd* pVirtualFont = new VirtualXlfd();

    for ( int i = 0; i < mnCount; i++ )
    {
        FontType   eType   = mpList[i]->GetFontType();
        FontWeight eWeight = mpList[i]->GetWeight();
        FontItalic eItalic = mpList[i]->GetItalic();

        if (    (eType   == TYPE_SCALABLE)
            && ((eWeight == WEIGHT_NORMAL) || (eWeight == WEIGHT_MEDIUM))
            && ((eItalic == ITALIC_NONE))
           )
        {
            Attribute *pFamily   = pFactory->RetrieveFamily(mpList[i]->mnFamily);
            Attribute *pSetWidth = pFactory->RetrieveSetwidth(mpList[i]->mnSetwidth);

            if (   pFamily->HasFeature( XLFD_FEATURE_INTERFACE_FONT )
                && !pSetWidth->HasFeature( XLFD_FEATURE_NARROW ))
            {
                pVirtualFont->AddEncoding( mpList[i] );
            }
        }
    }

    if (pVirtualFont->NumEncodings() > 0)
    {
        mpInterfaceFont = pVirtualFont;
        Add (pVirtualFont);
    }
    else
    {
        delete pVirtualFont;
    }
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

void
BitmapXlfdStorage::AddScalableFont( const ScalableXlfd *pScaleFnt )
{
    if ( pScaleFnt == NULL )
        return;

    for ( int i = 0; i < mnCount; i++ )
        ((BitmapXlfd*)(mpList[i]))->AddEncoding( pScaleFnt );
}

