/*************************************************************************
 *
 *  $RCSfile: salgdi3.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: th $ $Date: 2001-04-06 12:49:39 $
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

#include <string.h>

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#define _SV_SALGDI3_CXX

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _SV_WINCOMP_HXX
#include <wincomp.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <font.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

// -----------
// - Defines -
// -----------

#ifdef WIN
#define GDI_ERROR   (0xFFFFFFFFUL)
#endif

#define GLYPH_INC               (512UL)
#define MAX_POLYCOUNT           (2048UL)
#define CHECKPOINTS( _def_nPnt )    \
    if( (_def_nPnt) >= nPtSize )    \
        nPtSize = ImplIncreaseArrays( nPtSize, &pPoints, &pFlags, GLYPH_INC )

// -----------
// - Inlines -
// -----------

inline FIXED FixedFromDouble( double d )
{
    const long l = (long) ( d * 65536. );
    return *(FIXED*) &l;
}

// -----------------------------------------------------------------------

inline int IntFromFixed(FIXED f)
{
    return( ( f.fract >= 0x8000 ) ? ( f.value + 1 ) : f.value );
}

// -----------------------------------------------------------------------

inline FIXED fxDiv2( FIXED fxVal1, FIXED fxVal2 )
{
    const long l = (*((long far *)&(fxVal1)) + *((long far *)&(fxVal2))) >> 1;
    return *(FIXED*) &l;
}

// =======================================================================

#define SAL_DRAWTEXT_STACKBUF           128

// =======================================================================

// Diese Variablen koennen static sein, da systemweite Einstellungen
// gemerkt werden
static BOOL bImplSalCourierScalable = FALSE;
static BOOL bImplSalCourierNew = FALSE;

// =======================================================================

struct ImplEnumInfo
{
    HDC                 mhDC;
    ImplDevFontList*    mpList;
    XubString*          mpName;
    LOGFONTA*           mpLogFontA;
    LOGFONTW*           mpLogFontW;
    BOOL                mbCourier;
    BOOL                mbImplSalCourierScalable;
    BOOL                mbImplSalCourierNew;
    BOOL                mbPrinter;
};

// =======================================================================

static CharSet ImplCharSetToSal( BYTE nCharSet )
{
    if ( nCharSet == OEM_CHARSET )
    {
        UINT nCP = (USHORT)GetOEMCP();
        return rtl_getTextEncodingFromPCCodePage( nCP );
    }
    else
        return rtl_getTextEncodingFromWindowsCharset( nCharSet );
}

// -----------------------------------------------------------------------

static BYTE ImplCharSetToWin( CharSet eCharSet )
{
    return rtl_getBestWindowsCharsetFromTextEncoding( eCharSet );
}

// -----------------------------------------------------------------------

static FontFamily ImplFamilyToSal( BYTE nFamily )
{
    switch ( nFamily & 0xF0 )
    {
        case FF_DECORATIVE:
            return FAMILY_DECORATIVE;

        case FF_MODERN:
            return FAMILY_MODERN;

        case FF_ROMAN:
            return FAMILY_ROMAN;

        case FF_SCRIPT:
            return FAMILY_SCRIPT;

        case FF_SWISS:
            return FAMILY_SWISS;
    }

    return FAMILY_DONTKNOW;
}

// -----------------------------------------------------------------------

static BYTE ImplFamilyToWin( FontFamily eFamily )
{
    switch ( eFamily )
    {
        case FAMILY_DECORATIVE:
            return FF_DECORATIVE;

        case FAMILY_MODERN:
            return FF_MODERN;

        case FAMILY_ROMAN:
            return FF_ROMAN;

        case FAMILY_SCRIPT:
            return FF_SCRIPT;

        case FAMILY_SWISS:
            return FF_SWISS;

        case FAMILY_SYSTEM:
            return FF_SWISS;
    }

    return FF_DONTCARE;
}

// -----------------------------------------------------------------------

static FontWeight ImplWeightToSal( WinWeight nWeight )
{
    if ( nWeight <= FW_THIN )
        return WEIGHT_THIN;
    else if ( nWeight <= FW_ULTRALIGHT )
        return WEIGHT_ULTRALIGHT;
    else if ( nWeight <= FW_LIGHT )
        return WEIGHT_LIGHT;
    else if ( nWeight < FW_MEDIUM )
        return WEIGHT_NORMAL;
    else if ( nWeight == FW_MEDIUM )
        return WEIGHT_MEDIUM;
    else if ( nWeight <= FW_SEMIBOLD )
        return WEIGHT_SEMIBOLD;
    else if ( nWeight <= FW_BOLD )
        return WEIGHT_BOLD;
    else if ( nWeight <= FW_ULTRABOLD )
        return WEIGHT_ULTRABOLD;
    else
        return WEIGHT_BLACK;
}

// -----------------------------------------------------------------------

static WinWeight ImplWeightToWin( FontWeight eWeight )
{
    switch ( eWeight )
    {
        case WEIGHT_THIN:
            return FW_THIN;

        case WEIGHT_ULTRALIGHT:
            return FW_ULTRALIGHT;

        case WEIGHT_LIGHT:
            return FW_LIGHT;

        case WEIGHT_SEMILIGHT:
        case WEIGHT_NORMAL:
            return FW_NORMAL;

        case WEIGHT_MEDIUM:
            return FW_MEDIUM;

        case WEIGHT_SEMIBOLD:
            return FW_SEMIBOLD;

        case WEIGHT_BOLD:
            return FW_BOLD;

        case WEIGHT_ULTRABOLD:
            return FW_ULTRABOLD;

        case WEIGHT_BLACK:
            return FW_BLACK;
    }

    return 0;
}

// -----------------------------------------------------------------------

inline FontPitch ImplLogPitchToSal( BYTE nPitch )
{
    if ( nPitch & FIXED_PITCH )
        return PITCH_FIXED;
    else
        return PITCH_VARIABLE;
}

// -----------------------------------------------------------------------

inline FontPitch ImplMetricPitchToSal( BYTE nPitch )
{
    // Sausaecke bei MS !! siehe NT Hilfe
    if ( !(nPitch & TMPF_FIXED_PITCH) )
        return PITCH_FIXED;
    else
        return PITCH_VARIABLE;
}

// -----------------------------------------------------------------------

inline BYTE ImplPitchToWin( FontPitch ePitch )
{
    if ( ePitch == PITCH_FIXED )
        return FIXED_PITCH;
    else if ( ePitch == PITCH_VARIABLE )
        return VARIABLE_PITCH;
    else
        return DEFAULT_PITCH;
}

// -----------------------------------------------------------------------

static void ImplLogMetricToDevFontDataA( const LOGFONTA* pLogFont,
                                         const NEWTEXTMETRICA* pMetric,
                                         DWORD nFontType,
                                         ImplFontData* pData )
{
    if ( !(nFontType & RASTER_FONTTYPE) )
    {
        pData->mnWidth  = 0;
        pData->mnHeight = 0;
    }
    else
    {
        pData->mnWidth  = 0;
        pData->mnHeight = pMetric->tmHeight-pMetric->tmInternalLeading;
    }
    pData->meFamily         = ImplFamilyToSal( pLogFont->lfPitchAndFamily );
    pData->meCharSet        = ImplCharSetToSal( pLogFont->lfCharSet );
    pData->meWidthType      = WIDTH_DONTKNOW;
    pData->meWeight         = ImplWeightToSal( pLogFont->lfWeight );
    pData->meItalic         = (pLogFont->lfItalic) ? ITALIC_NORMAL : ITALIC_NONE;
    pData->mePitch          = ImplLogPitchToSal( pLogFont->lfPitchAndFamily );
    if ( pMetric->tmPitchAndFamily & (TMPF_VECTOR | TMPF_TRUETYPE) )
        pData->meType = TYPE_SCALABLE;
    else
        pData->meType = TYPE_RASTER;
    pData->mbOrientation    = (nFontType & RASTER_FONTTYPE) == 0;
    pData->mbDevice         = (pMetric->tmPitchAndFamily & TMPF_DEVICE) != 0;
    pData->mnQuality        = 0;
}

// -----------------------------------------------------------------------

static void ImplLogMetricToDevFontDataW( const LOGFONTW* pLogFont,
                                         const NEWTEXTMETRICW* pMetric,
                                         DWORD nFontType,
                                         ImplFontData* pData )
{
    if ( !(nFontType & RASTER_FONTTYPE) )
    {
        pData->mnWidth  = 0;
        pData->mnHeight = 0;
    }
    else
    {
        pData->mnWidth  = 0;
        pData->mnHeight = pMetric->tmHeight-pMetric->tmInternalLeading;
    }
    pData->meFamily         = ImplFamilyToSal( pLogFont->lfPitchAndFamily );
    pData->meCharSet        = ImplCharSetToSal( pLogFont->lfCharSet );
    pData->meWidthType      = WIDTH_DONTKNOW;
    pData->meWeight         = ImplWeightToSal( pLogFont->lfWeight );
    pData->meItalic         = (pLogFont->lfItalic) ? ITALIC_NORMAL : ITALIC_NONE;
    pData->mePitch          = ImplLogPitchToSal( pLogFont->lfPitchAndFamily );
    if ( pMetric->tmPitchAndFamily & (TMPF_VECTOR | TMPF_TRUETYPE) )
        pData->meType = TYPE_SCALABLE;
    else
        pData->meType = TYPE_RASTER;
    pData->mbOrientation    = (nFontType & RASTER_FONTTYPE) == 0;
    pData->mbDevice         = (pMetric->tmPitchAndFamily & TMPF_DEVICE) != 0;
    pData->mnQuality        = 0;
}

// -----------------------------------------------------------------------

void ImplSalLogFontToFontA( const LOGFONTA& rLogFont, Font& rFont )
{
    XubString aFontName( ImplSalGetUniString( rLogFont.lfFaceName ) );
    if ( aFontName.Len() )
    {
        rFont.SetName( aFontName );
        long nFontHeight = rLogFont.lfHeight;
        if ( nFontHeight < 0 )
            nFontHeight = -nFontHeight;
        HDC hDC = GetDC( 0 );
        long nDPIY = GetDeviceCaps( hDC, LOGPIXELSY );
        ReleaseDC( 0, hDC );
        nFontHeight *= 72;
        nFontHeight += nDPIY/2;
        nFontHeight /= nDPIY;
        rFont.SetSize( Size( 0, nFontHeight ) );
        rFont.SetOrientation( (short)rLogFont.lfEscapement );
        rFont.SetCharSet( ImplCharSetToSal( rLogFont.lfCharSet ) );
        rFont.SetFamily( ImplFamilyToSal( rLogFont.lfPitchAndFamily ) );
        rFont.SetPitch( ImplLogPitchToSal( rLogFont.lfPitchAndFamily ) );
        rFont.SetWeight( ImplWeightToSal( rLogFont.lfWeight ) );
        if ( rLogFont.lfItalic )
            rFont.SetItalic( ITALIC_NORMAL );
        else
            rFont.SetItalic( ITALIC_NONE );
        if ( rLogFont.lfUnderline )
            rFont.SetUnderline( UNDERLINE_SINGLE );
        else
            rFont.SetUnderline( UNDERLINE_NONE );
        if ( rLogFont.lfStrikeOut )
            rFont.SetStrikeout( STRIKEOUT_SINGLE );
        else
            rFont.SetStrikeout( STRIKEOUT_NONE );
    }
}

// -----------------------------------------------------------------------

void ImplSalLogFontToFontW( const LOGFONTW& rLogFont, Font& rFont )
{
    XubString aFontName( rLogFont.lfFaceName );
    if ( aFontName.Len() )
    {
        rFont.SetName( aFontName );
        long nFontHeight = rLogFont.lfHeight;
        if ( nFontHeight < 0 )
            nFontHeight = -nFontHeight;
        HDC hDC = GetDC( 0 );
        long nDPIY = GetDeviceCaps( hDC, LOGPIXELSY );
        ReleaseDC( 0, hDC );
        nFontHeight *= 72;
        nFontHeight += nDPIY/2;
        nFontHeight /= nDPIY;
        rFont.SetSize( Size( 0, nFontHeight ) );
        rFont.SetOrientation( (short)rLogFont.lfEscapement );
        rFont.SetCharSet( ImplCharSetToSal( rLogFont.lfCharSet ) );
        rFont.SetFamily( ImplFamilyToSal( rLogFont.lfPitchAndFamily ) );
        rFont.SetPitch( ImplLogPitchToSal( rLogFont.lfPitchAndFamily ) );
        rFont.SetWeight( ImplWeightToSal( rLogFont.lfWeight ) );
        if ( rLogFont.lfItalic )
            rFont.SetItalic( ITALIC_NORMAL );
        else
            rFont.SetItalic( ITALIC_NONE );
        if ( rLogFont.lfUnderline )
            rFont.SetUnderline( UNDERLINE_SINGLE );
        else
            rFont.SetUnderline( UNDERLINE_NONE );
        if ( rLogFont.lfStrikeOut )
            rFont.SetStrikeout( STRIKEOUT_SINGLE );
        else
            rFont.SetStrikeout( STRIKEOUT_NONE );
    }
}

// =======================================================================

void SalGraphics::SetTextColor( SalColor nSalColor )
{
    COLORREF aCol = PALETTERGB( SALCOLOR_RED( nSalColor ),
                                SALCOLOR_GREEN( nSalColor ),
                                SALCOLOR_BLUE( nSalColor ) );

    if( !maGraphicsData.mbPrinter &&
        GetSalData()->mhDitherPal &&
        ImplIsSysColorEntry( nSalColor ) )
    {
        aCol = PALRGB_TO_RGB( aCol );
    }

    ::SetTextColor( maGraphicsData.mhDC, aCol );
}

// -----------------------------------------------------------------------

int CALLBACK SalEnumQueryFontProcExW( const ENUMLOGFONTEXW*,
                                      const NEWTEXTMETRICEXW*,
                                      DWORD, LPARAM lParam )
{
    *((BOOL*)(void*)lParam) = TRUE;
    return 0;
}

// -----------------------------------------------------------------------

static void ImplSalGetVerticalFontNameW( HDC hDC, UniString& rName )
{
    if ( !rName.Len() )
        return;

    // Vertical fonts starts with a @
    UniString aTemp = rName;
    aTemp.Insert( (sal_Unicode)'@', 0 );

    // Test, if vertical Font available
    LOGFONTW aLogFont;
    memset( &aLogFont, 0, sizeof( aLogFont ) );

    UINT nNameLen = aTemp.Len();
    if ( nNameLen > (sizeof( aLogFont.lfFaceName )/sizeof( wchar_t ))-1 )
        nNameLen = (sizeof( aLogFont.lfFaceName )/sizeof( wchar_t ))-1;
    memcpy( aLogFont.lfFaceName, aTemp.GetBuffer(), nNameLen*sizeof( wchar_t ) );
    aLogFont.lfFaceName[nNameLen] = 0;

    BOOL bAvailable = FALSE;
    EnumFontFamiliesExW( hDC, &aLogFont, (FONTENUMPROCW)SalEnumQueryFontProcExW,
                         (LPARAM)(void*)&bAvailable, 0 );
    if ( bAvailable )
        rName = aTemp;
}

// -----------------------------------------------------------------------

int CALLBACK SalEnumQueryFontProcExA( const ENUMLOGFONTEXA*,
                                      const NEWTEXTMETRICEXA*,
                                      DWORD, LPARAM lParam )
{
    *((BOOL*)(void*)lParam) = TRUE;
    return 0;
}

// -----------------------------------------------------------------------

static void ImplSalGetVerticalFontNameA( HDC hDC, ByteString& rName )
{
    if ( !rName.Len() )
        return;

    // Vertical fonts starts with a @
    ByteString aTemp = rName;
    aTemp.Insert( '@', 0 );

    // Test, if vertical Font available
    LOGFONTA aLogFont;
    memset( &aLogFont, 0, sizeof( aLogFont ) );

    UINT nNameLen = aTemp.Len();
    if ( nNameLen > sizeof( aLogFont.lfFaceName )-1 )
        nNameLen = sizeof( aLogFont.lfFaceName )-1;
    memcpy( aLogFont.lfFaceName, aTemp.GetBuffer(), nNameLen );
    aLogFont.lfFaceName[nNameLen] = 0;

    BOOL bAvailable = FALSE;
    EnumFontFamiliesExA( hDC, &aLogFont, (FONTENUMPROCA)SalEnumQueryFontProcExA,
                         (LPARAM)(void*)&bAvailable, 0 );
    if ( bAvailable )
        rName = aTemp;
}

// -----------------------------------------------------------------------

void ImplGetLogFontFromFontSelect( HDC hDC,
                                   const ImplFontSelectData* pFont,
                                   LOGFONTW& rLogFont )
{
    UniString   aName;
    if ( pFont->mpFontData )
        aName = pFont->mpFontData->maName;
    else
        aName = pFont->maName.GetToken( 0 );

    // Test for vertical
    if ( pFont->mbVertical )
        ImplSalGetVerticalFontNameW( hDC, aName );

    UINT nNameLen = aName.Len();
    if ( nNameLen > (sizeof( rLogFont.lfFaceName )/sizeof( wchar_t ))-1 )
        nNameLen = (sizeof( rLogFont.lfFaceName )/sizeof( wchar_t ))-1;
    memcpy( rLogFont.lfFaceName, aName.GetBuffer(), nNameLen*sizeof( wchar_t ) );
    rLogFont.lfFaceName[nNameLen] = 0;

    if ( pFont->mpFontData )
        rLogFont.lfCharSet = (WIN_BYTE)(pFont->mpFontData->mpSysData);
    else
        rLogFont.lfCharSet = ImplCharSetToWin( pFont->meCharSet );

    rLogFont.lfPitchAndFamily  = ImplPitchToWin( pFont->mePitch );
    rLogFont.lfPitchAndFamily |= ImplFamilyToWin( pFont->meFamily );
    rLogFont.lfWeight          = ImplWeightToWin( pFont->meWeight );
    rLogFont.lfHeight          = (int)-pFont->mnHeight;
    rLogFont.lfWidth           = (int)pFont->mnWidth;
    rLogFont.lfUnderline       = 0;
    rLogFont.lfStrikeOut       = 0;
    rLogFont.lfItalic          = (pFont->meItalic) != ITALIC_NONE;
    rLogFont.lfEscapement      = pFont->mnOrientation;
    rLogFont.lfOrientation     = 0;
    rLogFont.lfClipPrecision   = CLIP_DEFAULT_PRECIS;
    rLogFont.lfQuality         = DEFAULT_QUALITY;
    if ( pFont->mnOrientation )
    {
        rLogFont.lfOutPrecision = OUT_TT_PRECIS;
        rLogFont.lfClipPrecision |= CLIP_LH_ANGLES;
    }
    else
        rLogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;

    // Disable Antialiasing
    if ( pFont->mbNonAntialiased )
        rLogFont.lfQuality = NONANTIALIASED_QUALITY;
}

// -----------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData* pFont )
{
    HFONT hNewFont;
    if ( aSalShlData.mbWNT )
    {
        LOGFONTW aLogFont;
        ImplGetLogFontFromFontSelect( maGraphicsData.mhDC, pFont, aLogFont );

        // Auf dem Bildschirm nehmen wir Courier New, wenn Courier nicht
        // skalierbar ist und wenn der Font skaliert oder rotiert werden
        // muss
        if ( maGraphicsData.mbScreen &&
             (pFont->mnWidth || pFont->mnOrientation ||
              !pFont->mpFontData || (pFont->mpFontData->mnHeight != pFont->mnHeight)) &&
             !bImplSalCourierScalable && bImplSalCourierNew &&
             (ImplSalWICompareAscii( aLogFont.lfFaceName, "Courier" ) == 0) )
            lstrcpyW( aLogFont.lfFaceName, L"Courier New" );

        hNewFont = CreateFontIndirectW( &aLogFont );
    }
    else
    {
        if ( !maGraphicsData.mpLogFont )
            maGraphicsData.mpLogFont = new LOGFONTA;

        ByteString aName;
        if ( pFont->mpFontData )
            aName = ImplSalGetWinAnsiString( pFont->mpFontData->maName );
        else
            aName = ImplSalGetWinAnsiString( pFont->maName.GetToken( 0 ) );

        // Test for vertical
        if ( pFont->mbVertical )
            ImplSalGetVerticalFontNameA( maGraphicsData.mhDC, aName );

        UINT nNameLen = aName.Len();
        if ( nNameLen > sizeof( maGraphicsData.mpLogFont->lfFaceName )-1 )
            nNameLen = sizeof( maGraphicsData.mpLogFont->lfFaceName )-1;
        memcpy( maGraphicsData.mpLogFont->lfFaceName, aName.GetBuffer(), nNameLen );
        maGraphicsData.mpLogFont->lfFaceName[nNameLen] = 0;

        if ( pFont->mpFontData )
            maGraphicsData.mpLogFont->lfCharSet = (WIN_BYTE)(pFont->mpFontData->mpSysData);
        else
            maGraphicsData.mpLogFont->lfCharSet = ImplCharSetToWin( pFont->meCharSet );

        maGraphicsData.mpLogFont->lfPitchAndFamily  = ImplPitchToWin( pFont->mePitch );
        maGraphicsData.mpLogFont->lfPitchAndFamily |= ImplFamilyToWin( pFont->meFamily );
        maGraphicsData.mpLogFont->lfWeight          = ImplWeightToWin( pFont->meWeight );
        maGraphicsData.mpLogFont->lfHeight          = (int)-pFont->mnHeight;
        maGraphicsData.mpLogFont->lfWidth           = (int)pFont->mnWidth;
        maGraphicsData.mpLogFont->lfUnderline       = 0;
        maGraphicsData.mpLogFont->lfStrikeOut       = 0;
        maGraphicsData.mpLogFont->lfItalic          = (pFont->meItalic) != ITALIC_NONE;
        maGraphicsData.mpLogFont->lfEscapement      = pFont->mnOrientation;
        maGraphicsData.mpLogFont->lfOrientation     = 0;
        maGraphicsData.mpLogFont->lfClipPrecision   = CLIP_DEFAULT_PRECIS;
        maGraphicsData.mpLogFont->lfQuality         = DEFAULT_QUALITY;
        if ( pFont->mnOrientation )
        {
            maGraphicsData.mpLogFont->lfOutPrecision = OUT_TT_PRECIS;
            maGraphicsData.mpLogFont->lfClipPrecision |= CLIP_LH_ANGLES;
        }
        else
            maGraphicsData.mpLogFont->lfOutPrecision = OUT_DEFAULT_PRECIS;

        // Disable Antialiasing
        if ( pFont->mbNonAntialiased )
            maGraphicsData.mpLogFont->lfQuality = NONANTIALIASED_QUALITY;

        // Auf dem Bildschirm nehmen wir Courier New, wenn Courier nicht
        // skalierbar ist und wenn der Font skaliert oder rotiert werden
        // muss
        if ( maGraphicsData.mbScreen &&
             (pFont->mnWidth || pFont->mnOrientation ||
              !pFont->mpFontData || (pFont->mpFontData->mnHeight != pFont->mnHeight)) &&
             !bImplSalCourierScalable && bImplSalCourierNew &&
             (stricmp( maGraphicsData.mpLogFont->lfFaceName, "Courier" ) == 0) )
            strcpy( maGraphicsData.mpLogFont->lfFaceName, "Courier New" );

        hNewFont = CreateFontIndirectA( maGraphicsData.mpLogFont );
    }

    HFONT hOldFont = SelectFont( maGraphicsData.mhDC, hNewFont );

    // destory or save old font
    if ( maGraphicsData.mhFont )
        DeleteFont( maGraphicsData.mhFont );
    else
        maGraphicsData.mhDefFont = hOldFont;

    // set new data
    maGraphicsData.mhFont = hNewFont;
    maGraphicsData.mbCalcOverhang = TRUE;

    maGraphicsData.mnFontCharSetCount = 0;
    maGraphicsData.mbFontKernInit = TRUE;
    if ( maGraphicsData.mpFontKernPairs )
    {
        delete maGraphicsData.mpFontKernPairs;
        maGraphicsData.mpFontKernPairs = 0;
    }
    maGraphicsData.mnFontKernPairCount = 0;

    // Auf dem Printer immer mit DrawTextArray arbeiten, da dort die
    // Zeichenbreiten genauer als Pixel sein koennen
    if ( maGraphicsData.mbPrinter )
        return SAL_SETFONT_USEDRAWTEXTARRAY;
    else
        return 0;
}

// -----------------------------------------------------------------------

long SalGraphics::GetCharWidth( sal_Unicode nChar1, sal_Unicode nChar2, long* pWidthAry )
{
    SIZE        aExtent;
    SIZE        aExtent2;
    sal_Unicode nCharCount = nChar2-nChar1+1;
    sal_Unicode i;
    int*        pWinWidthAry = (int*)pWidthAry;
    DBG_ASSERT( sizeof( int ) == sizeof( long ), "SalGraphics::GetCharWidth(): int != long" );

    // Da nicht bei allen Treibern diese Funktion funktioniert
    if ( !GetCharWidthW( maGraphicsData.mhDC, nChar1, nChar2, pWinWidthAry ) )
    {
        for ( i = 0; i < nCharCount; i++ )
        {
            WCHAR c =i+nChar1;
            if ( !GetTextExtentPointW( maGraphicsData.mhDC, &c, 1, &aExtent ) )
                pWinWidthAry[i] = 0;
            else
                pWinWidthAry[i] = aExtent.cx;
        }
    }

    // Ueberhang abziehen
    if ( maGraphicsData.mbCalcOverhang )
    {
        WCHAR aAA[2] = { 'A', 'A' };
        if ( GetTextExtentPointW( maGraphicsData.mhDC, aAA, 2, &aExtent ) &&
             GetTextExtentPointW( maGraphicsData.mhDC, aAA, 1, &aExtent2 ) )
        {
            maGraphicsData.mbCalcOverhang = FALSE;
            maGraphicsData.mnFontOverhang = (aExtent2.cx*2)-aExtent.cx;
        }

        int nOverhang = maGraphicsData.mnFontOverhang;
        if ( nOverhang )
        {
            for ( i = 0; i < nCharCount; i++ )
                pWinWidthAry[i] -= nOverhang;
        }
    }

    return 1;
}

// -----------------------------------------------------------------------

void SalGraphics::GetFontMetric( ImplFontMetricData* pMetric )
{
    if ( aSalShlData.mbWNT )
    {
        wchar_t aFaceName[LF_FACESIZE+60];
        GetTextFaceW( maGraphicsData.mhDC, sizeof( aFaceName ) / sizeof( wchar_t ), aFaceName );
        pMetric->maName = aFaceName;

        TEXTMETRICW aWinMetric;
        GetTextMetricsW( maGraphicsData.mhDC, &aWinMetric );

        pMetric->mnWidth            = aWinMetric.tmAveCharWidth;
        pMetric->meFamily           = ImplFamilyToSal( aWinMetric.tmPitchAndFamily );;
        pMetric->meCharSet          = ImplCharSetToSal( aWinMetric.tmCharSet );
        pMetric->meWeight           = ImplWeightToSal( aWinMetric.tmWeight );
        pMetric->mePitch            = ImplMetricPitchToSal( aWinMetric.tmPitchAndFamily );
        if ( aWinMetric.tmItalic )
            pMetric->meItalic = ITALIC_NORMAL;
        else
            pMetric->meItalic = ITALIC_NONE;
        if ( aWinMetric.tmPitchAndFamily & (TMPF_VECTOR | TMPF_TRUETYPE) )
            pMetric->meType = TYPE_SCALABLE;
        else
        {
            pMetric->meType = TYPE_RASTER;
            pMetric->mnOrientation = 0;
        }
        pMetric->mbDevice           = (aWinMetric.tmPitchAndFamily & TMPF_DEVICE) != 0;
        pMetric->mnAscent           = aWinMetric.tmAscent;
        pMetric->mnDescent          = aWinMetric.tmDescent;
        pMetric->mnLeading          = aWinMetric.tmInternalLeading;
        pMetric->mnSlant            = 0;
        pMetric->mnFirstChar        = 0;
        pMetric->mnLastChar         = 0xFF;
    }
    else
    {
        char aFaceName[LF_FACESIZE+60];
        GetTextFaceA( maGraphicsData.mhDC, sizeof( aFaceName ), aFaceName );
        pMetric->maName = ImplSalGetUniString( aFaceName );

        TEXTMETRICA aWinMetric;
        GetTextMetricsA( maGraphicsData.mhDC, &aWinMetric );

        pMetric->mnWidth            = aWinMetric.tmAveCharWidth;
        pMetric->meFamily           = ImplFamilyToSal( aWinMetric.tmPitchAndFamily );;
        pMetric->meCharSet          = ImplCharSetToSal( aWinMetric.tmCharSet );
        pMetric->meWeight           = ImplWeightToSal( aWinMetric.tmWeight );
        pMetric->mePitch            = ImplMetricPitchToSal( aWinMetric.tmPitchAndFamily );
        if ( aWinMetric.tmItalic )
            pMetric->meItalic = ITALIC_NORMAL;
        else
            pMetric->meItalic = ITALIC_NONE;
        if ( aWinMetric.tmPitchAndFamily & (TMPF_VECTOR | TMPF_TRUETYPE) )
            pMetric->meType = TYPE_SCALABLE;
        else
        {
            pMetric->meType = TYPE_RASTER;
            pMetric->mnOrientation = 0;
        }
        pMetric->mbDevice           = (aWinMetric.tmPitchAndFamily & TMPF_DEVICE) != 0;
        pMetric->mnAscent           = aWinMetric.tmAscent;
        pMetric->mnDescent          = aWinMetric.tmDescent;
        pMetric->mnLeading          = aWinMetric.tmInternalLeading;
        pMetric->mnSlant            = 0;
        pMetric->mnFirstChar        = 0;
        pMetric->mnLastChar         = 0xFF;
    }
}

// -----------------------------------------------------------------------

int CALLBACK SalEnumCharSetsProcExA( const ENUMLOGFONTEXA* pLogFont,
                                     const NEWTEXTMETRICEXA* pMetric,
                                     DWORD nFontType, LPARAM lParam )
{
    SalGraphicsData* pData = (SalGraphicsData*)lParam;
    // Charset already in the list?
    for ( BYTE i = 0; i < pData->mnFontCharSetCount; i++ )
    {
        if ( pData->mpFontCharSets[i] == pLogFont->elfLogFont.lfCharSet )
            return 1;
    }
    pData->mpFontCharSets[pData->mnFontCharSetCount] = pLogFont->elfLogFont.lfCharSet;
    pData->mnFontCharSetCount++;
    return 1;
}

// -----------------------------------------------------------------------

static void ImplGetAllFontCharSets( SalGraphicsData* pData )
{
    if ( !pData->mpFontCharSets )
        pData->mpFontCharSets = new BYTE[256];

    LOGFONTA aLogFont;
    memset( &aLogFont, 0, sizeof( aLogFont ) );
    aLogFont.lfCharSet = DEFAULT_CHARSET;
    GetTextFaceA( pData->mhDC, sizeof( aLogFont.lfFaceName ), aLogFont.lfFaceName );
    EnumFontFamiliesExA( pData->mhDC, &aLogFont, (FONTENUMPROCA)SalEnumCharSetsProcExA,
                         (LPARAM)(void*)pData, 0 );
}

// -----------------------------------------------------------------------

static void ImplAddKerningPairs( SalGraphicsData* pData )
{
    ULONG nPairs = ::GetKerningPairsA( pData->mhDC, 0, NULL );
    if ( !nPairs )
        return;

    CHARSETINFO aInfo;
    if ( !TranslateCharsetInfo( (DWORD*)(ULONG)GetTextCharset( pData->mhDC ), &aInfo, TCI_SRCCHARSET ) )
        return;

    if ( !pData->mpFontKernPairs )
        pData->mpFontKernPairs = new KERNINGPAIR[nPairs];
    else
    {
        KERNINGPAIR* pOldPairs = pData->mpFontKernPairs;
        pData->mpFontKernPairs = new KERNINGPAIR[nPairs+pData->mnFontKernPairCount];
        memcpy( pData->mpFontKernPairs, pOldPairs,
                pData->mnFontKernPairCount*sizeof( KERNINGPAIR ) );
        delete pOldPairs;
    }

    UINT            nCP = aInfo.ciACP;
    ULONG           nOldPairs = pData->mnFontKernPairCount;
    KERNINGPAIR*    pTempPair = pData->mpFontKernPairs+pData->mnFontKernPairCount;
    nPairs = ::GetKerningPairsA( pData->mhDC, nPairs, pTempPair );
    for ( ULONG i = 0; i < nPairs; i++ )
    {
        unsigned char   aBuf[2];
        wchar_t         nChar;
        int             nLen;
        BOOL            bAdd = TRUE;

        // None-ASCII?, then we must convert the char
        if ( (pTempPair->wFirst > 125) || (pTempPair->wFirst == 92) )
        {
            if ( pTempPair->wFirst < 256 )
            {
                aBuf[0] = (unsigned char)pTempPair->wFirst;
                nLen = 1;
            }
            else
            {
                aBuf[0] = (unsigned char)(pTempPair->wFirst >> 8);
                aBuf[1] = (unsigned char)(pTempPair->wFirst & 0xFF);
                nLen = 2;
            }
            if ( MultiByteToWideChar( nCP, MB_PRECOMPOSED | MB_USEGLYPHCHARS,
                                      (const char*)aBuf, nLen, &nChar, 1 ) )
                pTempPair->wFirst = nChar;
            else
                bAdd = FALSE;
        }
        if ( (pTempPair->wSecond > 125) || (pTempPair->wSecond == 92) )
        {
            if ( pTempPair->wSecond < 256 )
            {
                aBuf[0] = (unsigned char)pTempPair->wSecond;
                nLen = 1;
            }
            else
            {
                aBuf[0] = (unsigned char)(pTempPair->wSecond >> 8);
                aBuf[1] = (unsigned char)(pTempPair->wSecond & 0xFF);
                nLen = 2;
            }
            if ( MultiByteToWideChar( nCP, MB_PRECOMPOSED | MB_USEGLYPHCHARS,
                                      (const char*)aBuf, nLen, &nChar, 1 ) )
                pTempPair->wSecond = nChar;
            else
                bAdd = FALSE;
        }

        KERNINGPAIR* pTempPair2 = pData->mpFontKernPairs;
        for ( ULONG j = 0; j < nOldPairs; j++ )
        {
            if ( (pTempPair2->wFirst == pTempPair->wFirst) &&
                 (pTempPair2->wSecond == pTempPair->wSecond) )
            {
                bAdd = FALSE;
                break;
            }
            pTempPair2++;
        }

        if ( bAdd )
        {
            KERNINGPAIR* pDestPair = pData->mpFontKernPairs+pData->mnFontKernPairCount;
            if ( pDestPair != pTempPair )
                memcpy( pDestPair, pTempPair, sizeof( KERNINGPAIR ) );
            pData->mnFontKernPairCount++;
        }

        pTempPair++;
    }
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs )
{
    DBG_ASSERT( sizeof( KERNINGPAIR ) == sizeof( ImplKernPairData ),
                "SalGraphics::GetKernPairs(): KERNINGPAIR != ImplKernPairData" );

    if ( aSalShlData.mbWNT )
    {
        if ( !pKernPairs )
            return ::GetKerningPairsW( maGraphicsData.mhDC, 0, NULL );
        else
            return ::GetKerningPairsW( maGraphicsData.mhDC, nPairs, (KERNINGPAIR*)pKernPairs );
    }
    else
    {
        if ( maGraphicsData.mbFontKernInit )
        {
            if ( maGraphicsData.mpFontKernPairs )
            {
                delete maGraphicsData.mpFontKernPairs;
                maGraphicsData.mpFontKernPairs = 0;
            }
            maGraphicsData.mnFontKernPairCount = 0;

            if ( !maGraphicsData.mnFontCharSetCount )
                ImplGetAllFontCharSets( &maGraphicsData );

            if ( maGraphicsData.mnFontCharSetCount <= 1 )
                ImplAddKerningPairs( &maGraphicsData );
            else
            {
                // Query All Kerning Pairs from all possible CharSets
                for ( BYTE i = 0; i < maGraphicsData.mnFontCharSetCount; i++ )
                {
                    maGraphicsData.mpLogFont->lfCharSet = maGraphicsData.mpFontCharSets[i];
                    HFONT hNewFont = CreateFontIndirectA( maGraphicsData.mpLogFont );
                    HFONT hOldFont = SelectFont( maGraphicsData.mhDC, hNewFont );
                    ImplAddKerningPairs( &maGraphicsData );
                    SelectFont( maGraphicsData.mhDC, hOldFont );
                    DeleteFont( hNewFont );
                }
            }

            maGraphicsData.mbFontKernInit = FALSE;
        }

        if ( !pKernPairs )
            return maGraphicsData.mnFontKernPairCount;
        else
        {
            if ( nPairs > maGraphicsData.mnFontKernPairCount )
                nPairs = maGraphicsData.mnFontKernPairCount;
            memcpy( pKernPairs, maGraphicsData.mpFontKernPairs,
                    nPairs*sizeof( ImplKernPairData ) );
            return nPairs;
        }
    }
}

// -----------------------------------------------------------------------

int CALLBACK SalEnumFontsProcExA( const ENUMLOGFONTEXA* pLogFont,
                                  const NEWTEXTMETRICEXA* pMetric,
                                  DWORD nFontType, LPARAM lParam )
{
    ImplEnumInfo* pInfo = (ImplEnumInfo*)(void*)lParam;
    if ( !pInfo->mpName )
    {
        // Ignore vertical fonts
        if ( pLogFont->elfLogFont.lfFaceName[0] != '@' )
        {
            if ( !pInfo->mbImplSalCourierNew )
                pInfo->mbImplSalCourierNew = stricmp( pLogFont->elfLogFont.lfFaceName, "Courier New" ) == 0;
            if ( !pInfo->mbImplSalCourierScalable )
                pInfo->mbCourier = stricmp( pLogFont->elfLogFont.lfFaceName, "Courier" ) == 0;
            else
                pInfo->mbCourier = FALSE;
            XubString aName( ImplSalGetUniString( pLogFont->elfLogFont.lfFaceName ) );
            pInfo->mpName = &aName;
            strcpy( pInfo->mpLogFontA->lfFaceName, pLogFont->elfLogFont.lfFaceName );
            pInfo->mpLogFontA->lfCharSet = pLogFont->elfLogFont.lfCharSet;
            EnumFontFamiliesExA( pInfo->mhDC, pInfo->mpLogFontA, (FONTENUMPROCA)SalEnumFontsProcExA,
                                 (LPARAM)(void*)pInfo, 0 );
            pInfo->mpLogFontA->lfFaceName[0] = '\0';
            pInfo->mpLogFontA->lfCharSet = DEFAULT_CHARSET;
            pInfo->mpName = NULL;
            pInfo->mbCourier = FALSE;
        }
    }
    else
    {
        ImplFontData* pData = new ImplFontData;
        pData->maName = *(pInfo->mpName);

        ImplLogMetricToDevFontDataA( &(pLogFont->elfLogFont), &(pMetric->ntmTm), nFontType, pData );
        // Test if Stylename is correct
        const char* pStyleName = (const char*)pLogFont->elfStyle;
        const char* pTemp = pStyleName;
        const char* pEnd = pTemp + sizeof( pLogFont->elfStyle );
        while ( *pTemp && (pTemp < pEnd) )
        {
            if ( (*pTemp > 0) && (*pTemp < 0x20) )
            {
                pStyleName = NULL;
                break;
            }
            pTemp++;
        }
        if ( pStyleName && (pTemp < pEnd) )
            pData->maStyleName = ImplSalGetUniString( pStyleName );
        pData->mpSysData = (void*)(pLogFont->elfLogFont.lfCharSet);
        BOOL bAdd = TRUE;

        // Wenn es sich um einen nicht skalierbaren Bildschirm-Font
        // handelt, dann auf dem Drucker ignorieren
        if ( pData->meType != TYPE_SCALABLE )
        {
            if ( pInfo->mbPrinter )
                bAdd = pData->mbDevice;
        }
        else
        {
            // Feststellen, ob Courier skalierbar ist
            if ( pInfo->mbCourier )
                pInfo->mbImplSalCourierScalable = TRUE;
        }

        if ( bAdd )
            pInfo->mpList->Add( pData );
        else
            delete pData;
    }

    return 1;
}

// -----------------------------------------------------------------------

int CALLBACK SalEnumFontsProcExW( const ENUMLOGFONTEXW* pLogFont,
                                  const NEWTEXTMETRICEXW* pMetric,
                                  DWORD nFontType, LPARAM lParam )
{
    ImplEnumInfo* pInfo = (ImplEnumInfo*)(void*)lParam;
    if ( !pInfo->mpName )
    {
        // Ignore vertical fonts
        if ( pLogFont->elfLogFont.lfFaceName[0] != '@' )
        {
            if ( !pInfo->mbImplSalCourierNew )
                pInfo->mbImplSalCourierNew = ImplSalWICompareAscii( pLogFont->elfLogFont.lfFaceName, "Courier New" ) == 0;
            if ( !pInfo->mbImplSalCourierScalable )
                pInfo->mbCourier = ImplSalWICompareAscii( pLogFont->elfLogFont.lfFaceName, "Courier" ) == 0;
            else
                pInfo->mbCourier = FALSE;
            XubString aName( pLogFont->elfLogFont.lfFaceName );
            pInfo->mpName = &aName;
            memcpy( pInfo->mpLogFontW->lfFaceName, pLogFont->elfLogFont.lfFaceName, (aName.Len()+1)*sizeof( wchar_t ) );
            pInfo->mpLogFontW->lfCharSet = pLogFont->elfLogFont.lfCharSet;
            EnumFontFamiliesExW( pInfo->mhDC, pInfo->mpLogFontW, (FONTENUMPROCW)SalEnumFontsProcExW,
                                 (LPARAM)(void*)pInfo, 0 );
            pInfo->mpLogFontW->lfFaceName[0] = '\0';
            pInfo->mpLogFontW->lfCharSet = DEFAULT_CHARSET;
            pInfo->mpName = NULL;
            pInfo->mbCourier = FALSE;
        }
    }
    else
    {
        ImplFontData* pData = new ImplFontData;
        pData->maName = *(pInfo->mpName);

        ImplLogMetricToDevFontDataW( &(pLogFont->elfLogFont), &(pMetric->ntmTm), nFontType, pData );
        // Test if Stylename is correct
        const wchar_t* pStyleName = pLogFont->elfStyle;
        const wchar_t* pTemp = pStyleName;
        const wchar_t* pEnd = pTemp + sizeof( pLogFont->elfStyle )/sizeof( wchar_t );
        while ( *pTemp && (pTemp < pEnd) )
        {
            if ( *pTemp < 0x20 )
            {
                pStyleName = NULL;
                break;
            }
            pTemp++;
        }
        if ( pStyleName && (pTemp < pEnd) )
            pData->maStyleName = pStyleName;
        pData->mpSysData = (void*)(pLogFont->elfLogFont.lfCharSet);
        BOOL bAdd = TRUE;

        // Wenn es sich um einen nicht skalierbaren Bildschirm-Font
        // handelt, dann auf dem Drucker ignorieren
        if ( pData->meType != TYPE_SCALABLE )
        {
            if ( pInfo->mbPrinter )
                bAdd = pData->mbDevice;
        }
        else
        {
            // Feststellen, ob Courier skalierbar ist
            if ( pInfo->mbCourier )
                pInfo->mbImplSalCourierScalable = TRUE;
        }

        if ( bAdd )
            pInfo->mpList->Add( pData );
        else
            delete pData;
    }

    return 1;
}

// -----------------------------------------------------------------------

void SalGraphics::GetDevFontList( ImplDevFontList* pList )
{
    ImplEnumInfo aInfo;
    aInfo.mhDC          = maGraphicsData.mhDC;
    aInfo.mpList        = pList;
    aInfo.mpName        = NULL;
    aInfo.mpLogFontA    = NULL;
    aInfo.mpLogFontW    = NULL;
    aInfo.mbCourier     = FALSE;
    if ( !maGraphicsData.mbPrinter )
    {
        aInfo.mbImplSalCourierScalable  = FALSE;
        aInfo.mbImplSalCourierNew       = FALSE;
        aInfo.mbPrinter                 = FALSE;
    }
    else
    {
        aInfo.mbImplSalCourierScalable  = TRUE;
        aInfo.mbImplSalCourierNew       = TRUE;
        aInfo.mbPrinter                 = TRUE;
    }

    if ( aSalShlData.mbWNT )
    {
        LOGFONTW aLogFont;
        memset( &aLogFont, 0, sizeof( aLogFont ) );
        aLogFont.lfCharSet = DEFAULT_CHARSET;
        aInfo.mpLogFontW = &aLogFont;
        EnumFontFamiliesExW( maGraphicsData.mhDC, &aLogFont, (FONTENUMPROCW)SalEnumFontsProcExW,
                             (LPARAM)(void*)&aInfo, 0 );
    }
    else
    {
        LOGFONTA aLogFont;
        memset( &aLogFont, 0, sizeof( aLogFont ) );
        aLogFont.lfCharSet = DEFAULT_CHARSET;
        aInfo.mpLogFontA = &aLogFont;
        EnumFontFamiliesExA( maGraphicsData.mhDC, &aLogFont, (FONTENUMPROCA)SalEnumFontsProcExA,
                             (LPARAM)(void*)&aInfo, 0 );
    }

    // Feststellen, was es fuer Courier-Schriften auf dem Bildschirm gibt,
    // um in SetFont() evt. Courier auf Courier New zu mappen
    if ( !maGraphicsData.mbPrinter )
    {
        bImplSalCourierScalable = aInfo.mbImplSalCourierScalable;
        bImplSalCourierNew      = aInfo.mbImplSalCourierNew;
    }
}

// -----------------------------------------------------------------------

void SalGraphics::DrawText( long nX, long nY,
                            const xub_Unicode* pStr, xub_StrLen nLen )
{
    DBG_ASSERT( sizeof( WCHAR ) == sizeof( xub_Unicode ), "SalGraphics::DrawText(): WCHAR != sal_Unicode" );

    ::ExtTextOutW( maGraphicsData.mhDC, (int)nX, (int)nY,
                   0, NULL, pStr, nLen, NULL );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawTextArray( long nX, long nY,
                                 const xub_Unicode* pStr, xub_StrLen nLen,
                                 const long* pDXAry )
{
    DBG_ASSERT( sizeof( WCHAR ) == sizeof( xub_Unicode ), "SalGraphics::DrawText(): WCHAR != sal_Unicode" );

    if ( nLen < 2 )
        ::ExtTextOutW( maGraphicsData.mhDC, (int)nX, (int)nY, 0, NULL, pStr, nLen, NULL );
    else
    {
        int  aStackAry[SAL_DRAWTEXT_STACKBUF];
        int* pWinDXAry;

        if ( nLen <= SAL_DRAWTEXT_STACKBUF )
            pWinDXAry = aStackAry;
        else
            pWinDXAry = new int[nLen];

        pWinDXAry[0] = (int)pDXAry[0];
        for ( xub_StrLen i = 1; i < nLen-1; i++ )
            pWinDXAry[i] = (int)pDXAry[i]-pDXAry[i-1];

        // Breite vom letzten Zeichen ermitteln, da wir dieses auch
        // beim Windows-XArray in der richtigen Breite reingeben
        // muessen, um nicht auf Probleme bei einigen
        // Grafikkarten oder Druckertreibern zu stossen
        SIZE aExtent;
        if ( GetTextExtentPointW( maGraphicsData.mhDC, pStr+nLen-1, 1, &aExtent ) )
            pWinDXAry[nLen-1] = aExtent.cx;
        else
            pWinDXAry[nLen-1] = 4095;

        // Text ausgeben
        ::ExtTextOutW( maGraphicsData.mhDC, (int)nX, (int)nY, 0, NULL, pStr, nLen, pWinDXAry );

        if ( pWinDXAry != aStackAry )
            delete pWinDXAry;
    }
}

// -----------------------------------------------------------------------

static ULONG ImplIncreaseArrays( ULONG nSize, SalPoint** ppPoints, BYTE** ppFlags, ULONG nIncSize )
{
    const ULONG nOldSize = nSize;
    SalPoint*   pNewPoints = new SalPoint[ nSize += nIncSize ];
    BYTE*       pNewFlags = new BYTE[ nSize ];

    if( *ppPoints )
    {
        memcpy( pNewPoints, *ppPoints, nOldSize * sizeof( SalPoint ) );
        memset( pNewPoints + nOldSize, 0, nIncSize * sizeof( SalPoint ) );
        delete[] *ppPoints;
    }
    else
        memset( pNewPoints, 0, nSize * sizeof( SalPoint ) );

    if( *ppFlags )
    {
        memcpy( pNewFlags, *ppFlags, nOldSize );
        memset( pNewFlags + nOldSize, 0, nIncSize );
        delete[] *ppFlags;
    }
    else
        memset( pNewFlags, 0, nSize );

    *ppPoints = pNewPoints;
    *ppFlags = pNewFlags;

    return nSize;
}

// -----------------------------------------------------------------------

static void ImplGetFamilyAndAscents( HDC hDC, BYTE& rPitch, long& rAscent )
{
    rPitch = 0;
    rAscent = 0;

    if ( aSalShlData.mbWNT )
    {
        TEXTMETRICW aTextMetricW;
        if ( GetTextMetricsW( hDC, &aTextMetricW ) )
        {
            rPitch = aTextMetricW.tmPitchAndFamily;
            rAscent = aTextMetricW.tmAscent;
        }
    }
    else
    {
        TEXTMETRICA aTextMetricA;
        if ( GetTextMetricsA( hDC, &aTextMetricA ) )
        {
            rPitch = aTextMetricA.tmPitchAndFamily;
            rAscent = aTextMetricA.tmAscent;
        }
    }
}

// -----------------------------------------------------------------------

static BOOL ImplGetGlyphChar( SalGraphicsData* pData, sal_Unicode c,
                              WORD& rByteChar, HFONT& rOldFont )
{
    rOldFont = 0;

    if ( !pData->mnFontCharSetCount )
        ImplGetAllFontCharSets( pData );

    // Try at first the current charset
    CHARSETINFO aInfo;
    char        aDestBuf[2];
    int         nLen = 0;
    WIN_BOOL    bDefault;
    if ( TranslateCharsetInfo( (DWORD*)(ULONG)GetTextCharset( pData->mhDC ), &aInfo, TCI_SRCCHARSET ) )
    {
        bDefault = FALSE;
        nLen = WideCharToMultiByte( aInfo.ciACP,
                                    WC_COMPOSITECHECK | WC_DISCARDNS | WC_DEFAULTCHAR,
                                    &c, 1,
                                    aDestBuf, sizeof( aDestBuf ),
                                    NULL, &bDefault );
    }

    // Try all possible charsets
    if ( (nLen != 1) || bDefault )
    {
        // Query All Kerning Pairs from all possible CharSets
        for ( BYTE i = 0; i < pData->mnFontCharSetCount; i++ )
        {
            if ( TranslateCharsetInfo( (DWORD*)(ULONG)pData->mpFontCharSets[i], &aInfo, TCI_SRCCHARSET ) )
            {
                bDefault = FALSE;
                nLen = WideCharToMultiByte( aInfo.ciACP,
                                            WC_COMPOSITECHECK | WC_DISCARDNS | WC_DEFAULTCHAR,
                                            &c, 1,
                                            aDestBuf, sizeof( aDestBuf ),
                                            NULL, &bDefault );
                if ( (nLen == 1) && !bDefault )
                {
                    pData->mpLogFont->lfCharSet = pData->mpFontCharSets[i];
                    HFONT hNewFont = CreateFontIndirectA( pData->mpLogFont );
                    rOldFont = SelectFont( pData->mhDC, hNewFont );
                    break;
                }
            }
        }
    }

    // GetGlyphOutline() only works for characters < 256. For all characters
    // greater than 256 we use the default mechanismn in VCL to scan
    // the printed Glyph
    if ( (nLen == 1) && !bDefault )
    {
        rByteChar = (unsigned char)aDestBuf[0];
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( xub_Unicode cChar, long* pX, long* pY,
                                     long* pWidth, long* pHeight )
{
    HDC             hDC = maGraphicsData.mhDC;
    BYTE            nPitchAndFamily;
    long            nAscent;

    ImplGetFamilyAndAscents( hDC, nPitchAndFamily, nAscent );
    if ( !(nPitchAndFamily & TMPF_TRUETYPE) )
        return FALSE;

    GLYPHMETRICS    aGlyphMetrics;
    MAT2            aMat;
    DWORD           nSize;
    BOOL            bOK;
    HFONT           hOldFont = 0;

    // Einheitsmatrix erzeugen
    aMat.eM11 = FixedFromDouble(1.);
    aMat.eM12 = FixedFromDouble(0.);
    aMat.eM21 = FixedFromDouble(0.);
    aMat.eM22 = FixedFromDouble(1.);
    if ( aSalShlData.mbWNT )
        nSize = ::GetGlyphOutlineW( hDC, cChar, GGO_METRICS, &aGlyphMetrics, 0, NULL, &aMat );
    else
    {
        WORD nChar;
        if ( ImplGetGlyphChar( &maGraphicsData, cChar, nChar, hOldFont ) )
            nSize = ::GetGlyphOutlineA( hDC, nChar, GGO_METRICS, &aGlyphMetrics, 0, NULL, &aMat );
        else
            nSize = 0;
    }
    bOK = (nSize != GDI_ERROR) && nSize;
    if ( bOK )
    {
        *pX = aGlyphMetrics.gmptGlyphOrigin.x;
        *pY = nAscent - aGlyphMetrics.gmptGlyphOrigin.y;
        *pWidth = aGlyphMetrics.gmBlackBoxX;
        *pHeight = aGlyphMetrics.gmBlackBoxY;
    }

    if ( hOldFont )
    {
        HFONT hNewFont = SelectFont( maGraphicsData.mhDC, hOldFont );
        DeleteFont( hNewFont );
    }

    return bOK;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetGlyphOutline( xub_Unicode cChar, USHORT** ppPolySizes,
                                    SalPoint** ppPoints, BYTE** ppFlags )
{
    HDC             hDC = maGraphicsData.mhDC;
    BYTE            nPitchAndFamily;
    long            nAscent;

    ImplGetFamilyAndAscents( hDC, nPitchAndFamily, nAscent );
    if ( !(nPitchAndFamily & TMPF_TRUETYPE) )
        return 0;

    GLYPHMETRICS    aGlyphMetrics;
    MAT2            aMat;
    DWORD           nSize;
    USHORT          nChar = cChar;
    HFONT           hOldFont = 0;
    ULONG           nPolyCount = 0;

    // Einheitsmatrix erzeugen
    aMat.eM11 = FixedFromDouble(1.);
    aMat.eM12 = FixedFromDouble(0.);
    aMat.eM21 = FixedFromDouble(0.);
    aMat.eM22 = FixedFromDouble(1.);

    if ( aSalShlData.mbWNT )
        nSize = ::GetGlyphOutlineW( hDC, nChar, GGO_NATIVE, &aGlyphMetrics, 0, NULL, &aMat );
    else
    {
        if ( ImplGetGlyphChar( &maGraphicsData, cChar, nChar, hOldFont ) )
            nSize = ::GetGlyphOutlineA( hDC, nChar, GGO_NATIVE, &aGlyphMetrics, 0, NULL, &aMat );
        else
            nSize = 0;
    }

    if ( (nSize != GDI_ERROR) && nSize )
    {
        BYTE*   pData = new BYTE[ nSize ];
        ULONG   nTotalCount = 0;
        DWORD   nSize2;
        if ( aSalShlData.mbWNT )
            nSize2 = ::GetGlyphOutlineW( hDC, nChar, GGO_NATIVE, &aGlyphMetrics, nSize, pData, &aMat );
        else
            nSize2 = ::GetGlyphOutlineA( hDC, nChar, GGO_NATIVE, &aGlyphMetrics, nSize, pData, &aMat );
        if ( nSize == nSize2 )
        {
            ULONG               nPtSize = GLYPH_INC;
            SalPoint*           pPoints = new SalPoint[ GLYPH_INC ];
            SalPoint*           pTotalPoints = NULL;
            BYTE*               pFlags = new BYTE[ GLYPH_INC ];
            BYTE*               pTotalFlags = NULL;
            TTPOLYGONHEADER*    pHeader = (TTPOLYGONHEADER*)pData;
            TTPOLYCURVE*        pCurve;
            *ppPolySizes = new USHORT[ MAX_POLYCOUNT ];
            memset( *ppPolySizes, 0, MAX_POLYCOUNT * sizeof( USHORT ) );

            while ( ((BYTE*)pHeader < pData+nSize) && (nPolyCount < (MAX_POLYCOUNT - 1)) )
            {
                if ( pHeader->dwType == TT_POLYGON_TYPE )
                {
                    USHORT  nPnt = 0;
                    USHORT  i;

                    memset( pPoints, 0, nPtSize * sizeof( SalPoint ) );
                    memset( pFlags, 0, nPtSize );

                    // ersten Startpunkt holen; die folgenden Startpunkte sind
                    // die Endpunkte der vorhergehenden Kurven
                    pPoints[ nPnt ].mnX = IntFromFixed( pHeader->pfxStart.x );
                    pPoints[ nPnt++ ].mnY = IntFromFixed( pHeader->pfxStart.y );

                    pCurve = (TTPOLYCURVE*) ( pHeader + 1 );

                    while ( (BYTE*)pCurve < (BYTE*)pHeader+pHeader->cb )
                    {
                        if ( TT_PRIM_LINE == pCurve->wType )
                        {
                            for( i = 0; i < pCurve->cpfx; i++ )
                            {
                                CHECKPOINTS( nPnt );
                                pPoints[ nPnt ].mnX = IntFromFixed( pCurve->apfx[ i ].x );
                                pPoints[ nPnt++ ].mnY = IntFromFixed( pCurve->apfx[ i ].y );
                            }
                        }
                        else if ( pCurve->wType == TT_PRIM_QSPLINE )
                        {
                            for ( i = 0; i < pCurve->cpfx; )
                            {
                                // Punkt B, der Kontrollpunkt der Kurve
                                CHECKPOINTS( nPnt );
                                pPoints[ nPnt ].mnX = IntFromFixed( pCurve->apfx[ i ].x );
                                pPoints[ nPnt ].mnY = IntFromFixed( pCurve->apfx[ i++ ].y );

                                // Punkt verdoppeln fuer Bezier-Wandlung
                                CHECKPOINTS( nPnt + 1UL );
                                pPoints[ nPnt + 1 ] = pPoints[ nPnt ];
                                nPnt += 2;

                                // Endpunkt der Kurve bestimmen
                                if ( i == (pCurve->cpfx - 1) )
                                {
                                    // entweder letzter Punkt
                                    CHECKPOINTS( nPnt );
                                    pPoints[ nPnt ].mnX = IntFromFixed( pCurve->apfx[ i ].x );
                                    pPoints[ nPnt++].mnY = IntFromFixed( pCurve->apfx[ i++ ].y );
                                }
                                else
                                {
                                    // oder die Mitte zwischen den Kontrollpunkten
                                    // dieser und der naechsten Kurce
                                    CHECKPOINTS( nPnt );
                                    pPoints[ nPnt ].mnX = IntFromFixed( fxDiv2( pCurve->apfx[ i - 1 ].x,
                                                                                pCurve->apfx[ i ].x ) );
                                    pPoints[ nPnt++ ].mnY = IntFromFixed( fxDiv2( pCurve->apfx[ i - 1 ].y,
                                                                                  pCurve->apfx[ i ].y ) );
                                }

                                // Umrechnung in Bezier ( PQ = TrueType-Controlpunkt):
                                // P1 = 1/3 * (P0 + 2 * PQ) / P2 = 1/3 * (P3 + 2 * PQ)
                                pPoints[ nPnt - 3 ].mnX = ( pPoints[ nPnt - 4 ].mnX +
                                                          ( pPoints[ nPnt - 3 ].mnX << 1 ) ) / 3;
                                pPoints[ nPnt - 3 ].mnY = ( pPoints[ nPnt - 4 ].mnY +
                                                          ( pPoints[ nPnt - 3 ].mnY << 1 ) ) / 3;

                                pPoints[ nPnt - 2 ].mnX = ( pPoints[ nPnt - 1 ].mnX +
                                                          ( pPoints[ nPnt - 2 ].mnX << 1 ) ) / 3;
                                pPoints[ nPnt - 2 ].mnY = ( pPoints[ nPnt - 1 ].mnY +
                                                          ( pPoints[ nPnt - 2 ].mnY << 1 ) ) / 3;

                                pFlags[ nPnt - 3 ] = pFlags[ nPnt - 2 ] = 2;
                            }
                        }

                        // weiter mit naechstem Kurvensegment
                        pCurve = (TTPOLYCURVE*) &pCurve->apfx[ i ];
                    }

                    CHECKPOINTS( nPnt );
                    pPoints[nPnt++] = pPoints[0];

                    if ( nPnt )
                    {
                        (*ppPolySizes)[ nPolyCount++ ] = nPnt;
                        ImplIncreaseArrays( nTotalCount, &pTotalPoints, &pTotalFlags, nPnt );

                        // Polygon senkrecht kippen: TrueType-Y-Koordinaten verlaufen von unten nach oben
                        for ( i = 0; i < nPnt; i++ )
                        {
                            pTotalPoints[ nTotalCount ].mnX = pPoints[i].mnX;
                            pTotalPoints[ nTotalCount ].mnY = nAscent - pPoints[i].mnY;
                            pTotalFlags[ nTotalCount++ ] = pFlags[i];
                        }
                    }

                    // naechstes Polygon
                    pHeader = (TTPOLYGONHEADER*) ( (BYTE*) pHeader + pHeader->cb );
                }
            }

            delete[] pPoints;
            delete[] pFlags;

            if ( !nPolyCount )
            {
                delete[] pTotalPoints;
                *ppPoints = NULL;
                delete[] pTotalFlags;
                *ppFlags = NULL;
                delete[] *ppPolySizes;
                *ppPolySizes = NULL;
            }
            else
            {
                *ppPoints = pTotalPoints;
                *ppFlags = pTotalFlags;
            }
        }

        delete [] pData;
    }

    if ( hOldFont )
    {
        HFONT hNewFont = SelectFont( maGraphicsData.mhDC, hOldFont );
        DeleteFont( hNewFont );
    }

    return nPolyCount;
}
