/*************************************************************************
 *
 *  $RCSfile: salgdi3.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hdu $ $Date: 2002-08-09 11:38:24 $
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
#ifndef _OSL_FILE_HXX
#include <osl/file.hxx>
#endif
#ifndef _OSL_THREAD_HXX
#include <osl/thread.hxx>
#endif
#ifndef _OSL_PROCESS_HXX
#include <osl/process.h>
#endif

#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
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

#ifdef ENABLE_CTL
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif // _SV_SALLAYOUT_HXX
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif // _SV_POLY_HXX
#endif // ENABLE_CTL

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef __SUBFONT_H
#include <psprint/list.h>
#include <psprint/sft.h>
#endif

// -----------
// - Defines -
// -----------

#ifdef WIN
#define GDI_ERROR   (0xFFFFFFFFUL)
#endif

#define GLYPH_INC               (512UL)
#define MAX_POLYCOUNT           (2048UL)

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
    UINT                mnPreferedCharSet;
    BOOL                mbCourier;
    BOOL                mbImplSalCourierScalable;
    BOOL                mbImplSalCourierNew;
    BOOL                mbPrinter;
};

// =======================================================================

static CharSet ImplCharSetToSal( BYTE nCharSet )
{
    rtl_TextEncoding eTextEncoding;

    if ( nCharSet == OEM_CHARSET )
    {
        UINT nCP = (USHORT)GetOEMCP();
        switch ( nCP )
        {
            case 437:   eTextEncoding = RTL_TEXTENCODING_IBM_437; break;
            case 850:   eTextEncoding = RTL_TEXTENCODING_IBM_850; break;
            case 860:   eTextEncoding = RTL_TEXTENCODING_IBM_860; break;
            case 861:   eTextEncoding = RTL_TEXTENCODING_IBM_861; break;
            case 863:   eTextEncoding = RTL_TEXTENCODING_IBM_863; break;
            case 865:   eTextEncoding = RTL_TEXTENCODING_IBM_865; break;
            case 737:   eTextEncoding = RTL_TEXTENCODING_IBM_737; break;
            case 775:   eTextEncoding = RTL_TEXTENCODING_IBM_775; break;
            case 852:   eTextEncoding = RTL_TEXTENCODING_IBM_852; break;
            case 855:   eTextEncoding = RTL_TEXTENCODING_IBM_855; break;
            case 857:   eTextEncoding = RTL_TEXTENCODING_IBM_857; break;
            case 862:   eTextEncoding = RTL_TEXTENCODING_IBM_862; break;
            case 864:   eTextEncoding = RTL_TEXTENCODING_IBM_864; break;
            case 866:   eTextEncoding = RTL_TEXTENCODING_IBM_866; break;
            case 869:   eTextEncoding = RTL_TEXTENCODING_IBM_869; break;
            case 874:   eTextEncoding = RTL_TEXTENCODING_MS_874; break;
            case 932:   eTextEncoding = RTL_TEXTENCODING_MS_932; break;
            case 949:   eTextEncoding = RTL_TEXTENCODING_MS_949; break;
            case 950:   eTextEncoding = RTL_TEXTENCODING_MS_950; break;
            case 1004:  eTextEncoding = RTL_TEXTENCODING_MS_1252; break;
            case 65400: eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;
            default:    eTextEncoding = RTL_TEXTENCODING_DONTKNOW; break;
        };
    }
    else
    {
        if( nCharSet )
            eTextEncoding = rtl_getTextEncodingFromWindowsCharset( nCharSet );
        else
            eTextEncoding = RTL_TEXTENCODING_UNICODE;
    }

    return eTextEncoding;
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
    if( pMetric->tmPitchAndFamily & TMPF_TRUETYPE ) // prefer truetype
        pData->mnQuality    += 100;
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
    if( pMetric->tmPitchAndFamily & TMPF_TRUETYPE ) // prefer truetype
        pData->mnQuality    += 100;
}

// -----------------------------------------------------------------------

void ImplSalLogFontToFontA( HDC hDC, const LOGFONTA& rLogFont, Font& rFont,
                            BOOL bReplaceFont )
{
    XubString aFontName( ImplSalGetUniString( rLogFont.lfFaceName ) );
    if ( aFontName.Len() )
    {
        if ( bReplaceFont )
        {
            rFont.SetName( aFontName );
            rFont.SetCharSet( ImplCharSetToSal( rLogFont.lfCharSet ) );
            rFont.SetFamily( ImplFamilyToSal( rLogFont.lfPitchAndFamily ) );
            rFont.SetPitch( ImplLogPitchToSal( rLogFont.lfPitchAndFamily ) );
            rFont.SetWeight( ImplWeightToSal( rLogFont.lfWeight ) );
        }

        long nFontHeight = rLogFont.lfHeight;
        if ( nFontHeight < 0 )
            nFontHeight = -nFontHeight;
        long nDPIY = GetDeviceCaps( hDC, LOGPIXELSY );
        nFontHeight *= 72;
        nFontHeight += nDPIY/2;
        nFontHeight /= nDPIY;
        rFont.SetSize( Size( 0, nFontHeight ) );
        rFont.SetOrientation( (short)rLogFont.lfEscapement );
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

void ImplSalLogFontToFontW( HDC hDC, const LOGFONTW& rLogFont, Font& rFont,
                            BOOL bReplaceFont )
{
    XubString aFontName( rLogFont.lfFaceName );
    if ( aFontName.Len() )
    {
        if ( bReplaceFont )
        {
            rFont.SetName( aFontName );
            rFont.SetCharSet( ImplCharSetToSal( rLogFont.lfCharSet ) );
            rFont.SetFamily( ImplFamilyToSal( rLogFont.lfPitchAndFamily ) );
            rFont.SetPitch( ImplLogPitchToSal( rLogFont.lfPitchAndFamily ) );
            rFont.SetWeight( ImplWeightToSal( rLogFont.lfWeight ) );
        }

        long nFontHeight = rLogFont.lfHeight;
        if ( nFontHeight < 0 )
            nFontHeight = -nFontHeight;
        long nDPIY = GetDeviceCaps( hDC, LOGPIXELSY );
        nFontHeight *= 72;
        nFontHeight += nDPIY/2;
        nFontHeight /= nDPIY;
        rFont.SetSize( Size( 0, nFontHeight ) );
        rFont.SetOrientation( (short)rLogFont.lfEscapement );
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
    aLogFont.lfCharSet = DEFAULT_CHARSET;

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
    aLogFont.lfCharSet = DEFAULT_CHARSET;

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

BOOL ImplIsFontAvailable( HDC hDC, const UniString& rName )
{
    BOOL bAvailable = FALSE;

    if ( aSalShlData.mbWNT )
    {
        // Test, if Font available
        LOGFONTW aLogFont;
        memset( &aLogFont, 0, sizeof( aLogFont ) );
        aLogFont.lfCharSet = DEFAULT_CHARSET;

        UINT nNameLen = rName.Len();
        if ( nNameLen > (sizeof( aLogFont.lfFaceName )/sizeof( wchar_t ))-1 )
            nNameLen = (sizeof( aLogFont.lfFaceName )/sizeof( wchar_t ))-1;
        memcpy( aLogFont.lfFaceName, rName.GetBuffer(), nNameLen*sizeof( wchar_t ) );
        aLogFont.lfFaceName[nNameLen] = 0;

        EnumFontFamiliesExW( hDC, &aLogFont, (FONTENUMPROCW)SalEnumQueryFontProcExW,
                             (LPARAM)(void*)&bAvailable, 0 );
    }
    else
    {
        ByteString aTemp = ImplSalGetWinAnsiString( rName );

        // Test, if Font available
        LOGFONTA aLogFont;
        memset( &aLogFont, 0, sizeof( aLogFont ) );
        aLogFont.lfCharSet = DEFAULT_CHARSET;

        UINT nNameLen = aTemp.Len();
        if ( nNameLen > sizeof( aLogFont.lfFaceName )-1 )
            nNameLen = sizeof( aLogFont.lfFaceName )-1;
        memcpy( aLogFont.lfFaceName, aTemp.GetBuffer(), nNameLen );
        aLogFont.lfFaceName[nNameLen] = 0;

        EnumFontFamiliesExA( hDC, &aLogFont, (FONTENUMPROCA)SalEnumQueryFontProcExA,
                             (LPARAM)(void*)&bAvailable, 0 );
    }

    return bAvailable;
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
    rLogFont.lfOutPrecision    = OUT_TT_PRECIS;
    if ( pFont->mnOrientation )
        rLogFont.lfClipPrecision |= CLIP_LH_ANGLES;

    // disable antialiasing if requested
    if ( pFont->mbNonAntialiased )
        rLogFont.lfQuality = NONANTIALIASED_QUALITY;
}

// -----------------------------------------------------------------------

static HFONT ImplSelectFontW( HDC hDC, LOGFONTW& rLogFont, HFONT* pNewFont )
{
    HFONT hNewFont = CreateFontIndirectW( &rLogFont );
    HFONT hOldFont = SelectFont( hDC, hNewFont );

    TEXTMETRICW aWinMetric;
    // Font doesn't work, try a replacement
    if ( !GetTextMetricsW( hDC, &aWinMetric ) )
    {
        lstrcpyW( rLogFont.lfFaceName, L"Courier New" );
        rLogFont.lfPitchAndFamily = FIXED_PITCH;
        HFONT hNewFont2 = CreateFontIndirectW( &rLogFont );
        SelectFont( hDC, hNewFont2 );
        DeleteFont( hNewFont );
        hNewFont = hNewFont2;
    }

    *pNewFont = hNewFont;
    return hOldFont;
}

// -----------------------------------------------------------------------

static HFONT ImplSelectFontA( HDC hDC, LOGFONTA& rLogFont, HFONT* pNewFont )
{
    HFONT hNewFont = CreateFontIndirectA( &rLogFont );
    HFONT hOldFont = SelectFont( hDC, hNewFont );

    TEXTMETRICA aWinMetric;
    // Font doesn't work, try a replacement
    if ( !GetTextMetricsA( hDC, &aWinMetric ) )
    {
        strcpy( rLogFont.lfFaceName, "Courier New" );
        rLogFont.lfPitchAndFamily = FIXED_PITCH;
        HFONT hNewFont2 = CreateFontIndirectA( &rLogFont );
        SelectFont( hDC, hNewFont2 );
        DeleteFont( hNewFont );
        hNewFont = hNewFont2;
    }

    *pNewFont = hNewFont;
    return hOldFont;
}

// -----------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData* pFont )
{
    HFONT hNewFont = 0;
    HFONT hOldFont;

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

        hOldFont = ImplSelectFontW( maGraphicsData.mhDC, aLogFont, &hNewFont );
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
        maGraphicsData.mpLogFont->lfOutPrecision    = OUT_TT_PRECIS;
        if ( pFont->mnOrientation )
            maGraphicsData.mpLogFont->lfClipPrecision |= CLIP_LH_ANGLES;

        // disable antialiasing if requested
        if ( pFont->mbNonAntialiased )
            maGraphicsData.mpLogFont->lfQuality = NONANTIALIASED_QUALITY;

        // on the display we prefer Courier New when Courier is a
        // bitmap only font and we need to stretch or rotate it
        if ( maGraphicsData.mbScreen &&
             (pFont->mnWidth || pFont->mnOrientation ||
              !pFont->mpFontData || (pFont->mpFontData->mnHeight != pFont->mnHeight)) &&
             !bImplSalCourierScalable && bImplSalCourierNew &&
             (stricmp( maGraphicsData.mpLogFont->lfFaceName, "Courier" ) == 0) )
            strcpy( maGraphicsData.mpLogFont->lfFaceName, "Courier New" );

        hOldFont = ImplSelectFontA( maGraphicsData.mhDC, *maGraphicsData.mpLogFont, &hNewFont );
    }

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

    // some printers have higher internal resolution, so their
    // text output would be different from what we calculated
    // => suggest DrawTextArray to workaround this problem
    if ( maGraphicsData.mbPrinter )
        return SAL_SETFONT_USEDRAWTEXTARRAY;
    else
        return 0;
}

// -----------------------------------------------------------------------

//TODO: replace for ENABLE_CTL, because only needed to get font width factor
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
        if ( GetTextFaceW( maGraphicsData.mhDC, sizeof( aFaceName ) / sizeof( wchar_t ), aFaceName ) )
            pMetric->maName = aFaceName;

        TEXTMETRICW aWinMetric;
        if ( GetTextMetricsW( maGraphicsData.mhDC, &aWinMetric ) )
        {
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
            pMetric->mnFirstChar        = aWinMetric.tmFirstChar;
            pMetric->mnLastChar         = aWinMetric.tmLastChar;
        }
    }
    else
    {
        char aFaceName[LF_FACESIZE+60];
        if ( GetTextFaceA( maGraphicsData.mhDC, sizeof( aFaceName ), aFaceName ) )
            pMetric->maName = ImplSalGetUniString( aFaceName );

        TEXTMETRICA aWinMetric;
        if ( GetTextMetricsA( maGraphicsData.mhDC, &aWinMetric ) )
        {
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
            pMetric->mnFirstChar        = aWinMetric.tmFirstChar;
            pMetric->mnLastChar         = aWinMetric.tmLastChar;
        }
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

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static unsigned GetUShort( const unsigned char* p ){ return((p[0]<<8)+p[1]);}
static signed GetSShort( const unsigned char* p ){ return((short)((p[0]<<8)+p[1]));}

ULONG SalGraphics::GetFontCodeRanges( sal_uInt32* pCodePairs ) const
{
    int nRangeCount = 0;

    DWORD CmapTag = 'c' + ('m'<<8) + ('a'<<16)  + ('p'<<24);
    DWORD rc = GetFontData( maGraphicsData.mhDC, CmapTag, 0, NULL, 0 );
    if( rc != GDI_ERROR )
    {
        // we have a truetype font
        int nLength = rc;
        unsigned char* pCmap = new unsigned char[ nLength ];
        rc = GetFontData( maGraphicsData.mhDC, CmapTag, 0, pCmap, nLength );

        if( (rc != GDI_ERROR) && GetUShort( pCmap )==0 )
        {
            int nSubTables  = GetUShort( pCmap + 2 );
            const unsigned char* p = pCmap + 4;
            int nOffset = 0;
            int nFormat = -1;
            for( ; --nSubTables>=0; p+=8 )
            {
                int nPlatform = GetUShort( p );
                int nEncoding = GetUShort( p+2 );
                if( nEncoding!=0 && nEncoding!=1 )  // unicode encodings?
                    continue;
                nOffset       = GetUInt( p+4 );
                nFormat       = GetUShort( pCmap + nOffset );
                if( nFormat==4 )
                    break;
            }

            if( nFormat==4 && (nOffset+16)<nLength )
            {
                // analyze most common unicode mapping table
                int nSegCount = GetUShort( pCmap + nOffset + 6 );
                nRangeCount = nSegCount/2 - 1;
                if( pCodePairs )
                {
                    const unsigned char* pLimit = pCmap + nOffset + 14;
                    const unsigned char* pBegin = pLimit + 2 + nSegCount;
                    for( int i = 0; i < nRangeCount; ++i )
                    {
                        *(pCodePairs++) = GetUShort( pBegin + 2*i );
                        *(pCodePairs++) = GetUShort( pLimit + 2*i ) + 1;
                    }
                }
            }
         }

         delete[] pCmap;
    }

    return nRangeCount;
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

        pData->mbSubsettable = FALSE;
        pData->mbEmbeddable = FALSE;
        if( !(pMetric->ntmTm.tmPitchAndFamily & TMPF_DEVICE)
          && (pMetric->ntmTm.tmPitchAndFamily & TMPF_TRUETYPE) )
                pData->mbSubsettable = TRUE;

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

        // prefer the system character set, so that we get as much as
        // possible important characters. In the other case we could only
        // display a limited set of characters (#87309#)
        if ( pInfo->mnPreferedCharSet == pLogFont->elfLogFont.lfCharSet )
            pData->mnQuality += 100;

        // ignore non-scalable display font on printer
        if ( pData->meType != TYPE_SCALABLE )
        {
            if ( pInfo->mbPrinter )
                bAdd = pData->mbDevice;
        }
        else
        {
            pData->mnQuality += 150;

            // knowing Courier to be scalable is nice
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

        pData->mbSubsettable = FALSE;
        pData->mbEmbeddable = FALSE;
        if( !(pMetric->ntmTm.tmPitchAndFamily & TMPF_DEVICE)
         &&  (pMetric->ntmTm.tmPitchAndFamily & TMPF_TRUETYPE) )
                pData->mbSubsettable = TRUE;

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

        // prefer the system character set, so that we get as much as
        // possible important characters. In the other case we could only
        // display a limited set of characters (#87309#)
        if ( pInfo->mnPreferedCharSet == pLogFont->elfLogFont.lfCharSet )
            pData->mnQuality += 100;

        // ignore non-scalable display font on printer
        if ( pData->meType != TYPE_SCALABLE )
        {
            if ( pInfo->mbPrinter )
                bAdd = pData->mbDevice;
        }
        else
        {
            pData->mnQuality += 150;

            // knowing Courier to be scalable is nice
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
    // make sure all fonts are registered at least temporarily
    static bool bOnce = true;
    if( bOnce )
    {
        bOnce = false;

        // determine font path
        // since we are only interested in fonts that could not be
        // registered before because of missing administration rights
        // only the font path of the user installation is needed
        ::rtl::OUString aPath;
        osl_getExecutableFile( &aPath.pData );
        aPath = aPath.copy( 0, aPath.lastIndexOf('/') );
        String aFontDirUrl = aPath.copy( 0, aPath.lastIndexOf('/') );
        aFontDirUrl += String( RTL_CONSTASCII_USTRINGPARAM("/share/fonts/truetype") );

        // collect fonts in font path that could not be registered
        osl::Directory aFontDir( aFontDirUrl );
        osl::FileBase::RC rcOSL = aFontDir.open();
        if( rcOSL == osl::FileBase::E_None )
        {
            osl::DirectoryItem aDirItem;
            while( aFontDir.getNextItem( aDirItem, 10 ) == osl::FileBase::E_None )
            {
                osl::FileStatus aFileStatus( FileStatusMask_FileURL );
                rcOSL = aDirItem.getFileStatus( aFileStatus );

                ::rtl::OUString aUSytemPath;
                OSL_VERIFY( osl_File_E_None
                    == ::osl::FileBase::getSystemPathFromFileURL( aFileStatus.getFileURL(), aUSytemPath ));
                if( aSalShlData.mbWNT )
                    ::AddFontResourceW( aUSytemPath.getStr() );
                else
                {
                    rtl_TextEncoding theEncoding = osl_getThreadTextEncoding();
                    ::rtl::OString aCFileName = rtl::OUStringToOString( aUSytemPath, theEncoding );
                    ::AddFontResourceA( aCFileName.getStr() );
                }
            }
        }
    }

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

    aInfo.mnPreferedCharSet = DEFAULT_CHARSET;
    DWORD nCP = GetACP();
    CHARSETINFO aCharSetInfo;
    if ( TranslateCharsetInfo( (DWORD*)nCP, &aCharSetInfo, TCI_SRCCODEPAGE ) )
        aInfo.mnPreferedCharSet = aCharSetInfo.ciCharset;

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

#ifndef ENABLE_CTL
void SalGraphics::DrawText( long nX, long nY,
                            const xub_Unicode* pStr, xub_StrLen nLen )
{
    DBG_ASSERT( sizeof( WCHAR ) == sizeof( xub_Unicode ), "SalGraphics::DrawText(): WCHAR != sal_Unicode" );

    ::ExtTextOutW( maGraphicsData.mhDC, (int)nX, (int)nY,
                   0, NULL, pStr, nLen, NULL );
}
#endif // ENABLE_CTL

// -----------------------------------------------------------------------

#ifndef ENABLE_CTL
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
        // Grafikkarten oder Druckertreibern zu stossen ###
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
#endif // ENABLE_CTL

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( long nIndex, bool bIsGlyphIndex, Rectangle& rRect )
{
    HDC hDC = maGraphicsData.mhDC;

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = FixedFromDouble( 1.0 );
    aMat.eM12 = FixedFromDouble( 0.0 );
    aMat.eM21 = FixedFromDouble( 0.0 );
    aMat.eM22 = FixedFromDouble( 1.0 );

    UINT nGGOFlags = GGO_METRICS;
    if( bIsGlyphIndex )
        nGGOFlags |= GGO_GLYPH_INDEX;

    GLYPHMETRICS aGM;
    DWORD nSize = GDI_ERROR;
    if ( aSalShlData.mbWNT )
        nSize = ::GetGlyphOutlineW( hDC, nIndex, nGGOFlags, &aGM, 0, NULL, &aMat );
    else if( bIsGlyphIndex || (nIndex <= 255) )
        nSize = ::GetGlyphOutlineA( hDC, nIndex, nGGOFlags, &aGM, 0, NULL, &aMat );

    if( !nSize )    // blank glyphs are ok
        rRect.SetEmpty();
    else if( nSize != GDI_ERROR )
        rRect = Rectangle( Point( aGM.gmptGlyphOrigin.x, -aGM.gmptGlyphOrigin.y ),
            Size( aGM.gmBlackBoxX, aGM.gmBlackBoxY ) );
    else
        return false;

    return true;
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphOutline( long nIndex, bool bIsGlyphIndex, PolyPolygon& rPolyPoly )
{
    BOOL bRet = FALSE;
    HDC  hDC = maGraphicsData.mhDC;

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = FixedFromDouble( 1.0 );
    aMat.eM12 = FixedFromDouble( 0.0 );
    aMat.eM21 = FixedFromDouble( 0.0 );
    aMat.eM22 = FixedFromDouble( 1.0 );

    UINT nGGOFlags = GGO_NATIVE;
    if( bIsGlyphIndex )
        nGGOFlags |= GGO_GLYPH_INDEX;

    GLYPHMETRICS aGlyphMetrics;
    DWORD nSize1 = GDI_ERROR;
    if ( aSalShlData.mbWNT )
        nSize1 = ::GetGlyphOutlineW( hDC, nIndex, nGGOFlags, &aGlyphMetrics, 0, NULL, &aMat );
    else if( bIsGlyphIndex || (nIndex <= 255) )
        nSize1 = ::GetGlyphOutlineA( hDC, nIndex, nGGOFlags, &aGlyphMetrics, 0, NULL, &aMat );

    if( !nSize1 )       // blank glyphs are ok
        bRet = TRUE;
    else if( nSize1 != GDI_ERROR )
    {
        BYTE*   pData = new BYTE[ nSize1 ];
        ULONG   nTotalCount = 0;
        DWORD   nSize2;
        if ( aSalShlData.mbWNT )
            nSize2 = ::GetGlyphOutlineW( hDC, nIndex, nGGOFlags,
                &aGlyphMetrics, nSize1, pData, &aMat );
        else
            nSize2 = ::GetGlyphOutlineA( hDC, nIndex, nGGOFlags,
                &aGlyphMetrics, nSize1, pData, &aMat );

        if( nSize1 == nSize2 )
        {
            bRet = TRUE;

            int     nPtSize = GLYPH_INC;
            Point*  pPoints = new Point[ nPtSize ];
            BYTE*   pFlags = new BYTE[ nPtSize ];

            TTPOLYGONHEADER* pHeader = (TTPOLYGONHEADER*)pData;
            while( (BYTE*)pHeader < pData+nSize2 )
            {
                // only outline data is interesting
                if( pHeader->dwType != TT_POLYGON_TYPE )
                    break;

                // get start point; next start points are end points
                // of previous segment
                int nPnt = 0;

                long nX = IntFromFixed( pHeader->pfxStart.x );
                long nY = IntFromFixed( pHeader->pfxStart.y );
                pPoints[ nPnt ] = Point( nX, nY );
                pFlags[ nPnt++ ] = POLY_NORMAL;

                bool bHasOfflinePoints = false;
                TTPOLYCURVE* pCurve = (TTPOLYCURVE*)( pHeader + 1 );
                pHeader = (TTPOLYGONHEADER*)( (BYTE*)pHeader + pHeader->cb );
                while( (BYTE*)pCurve < (BYTE*)pHeader )
                {
                    int nNeededSize = nPnt + 16 + 3 * pCurve->cpfx;
                    if( nPtSize < nNeededSize )
                    {
                        Point* pOldPoints = pPoints;
                        BYTE* pOldFlags = pFlags;
                        nPtSize = nNeededSize;
                        pPoints = new Point[ nPtSize ];
                        pFlags = new BYTE[ nPtSize ];
                        for( int i = 0; i < nPnt; ++i )
                        {
                            pPoints[ i ] = pOldPoints[ i ];
                            pFlags[ i ] = pOldFlags[ i ];
                        }
                        delete[] pOldPoints;
                        delete[] pOldFlags;
                    }

                    int i = 0;
                    if( TT_PRIM_LINE == pCurve->wType )
                    {
                        while( i < pCurve->cpfx )
                        {
                            nX = IntFromFixed( pCurve->apfx[ i ].x );
                            nY = IntFromFixed( pCurve->apfx[ i ].y );
                            ++i;
                            pPoints[ nPnt ] = Point( nX, nY );
                            pFlags[ nPnt ] = POLY_NORMAL;
                            ++nPnt;
                        }
                    }
                    else if( TT_PRIM_QSPLINE == pCurve->wType )
                    {
                        bHasOfflinePoints = true;
                        while( i < pCurve->cpfx )
                        {
                            // get control point of quadratic bezier spline
                            nX = IntFromFixed( pCurve->apfx[ i ].x );
                            nY = IntFromFixed( pCurve->apfx[ i ].y );
                            ++i;
                            Point aControlP( nX, nY );

                            // calculate first cubic control point
                            // P0 = 1/3 * (PBeg + 2 * PQControl)
                            nX = pPoints[ nPnt-1 ].X() + 2 * aControlP.X();
                            nY = pPoints[ nPnt-1 ].Y() + 2 * aControlP.Y();
                            pPoints[ nPnt+0 ] = Point( (2*nX+3)/6, (2*nY+3)/6 );
                            pFlags[ nPnt+0 ] = POLY_CONTROL;

                            // calculate endpoint of segment
                            nX = IntFromFixed( pCurve->apfx[ i ].x );
                            nY = IntFromFixed( pCurve->apfx[ i ].y );

                            if ( i+1 >= pCurve->cpfx )
                            {
                                // endpoint is either last point in segment => advance
                                ++i;
                            }
                            else
                            {
                                // or is in the mid of two control points
                                nX += IntFromFixed( pCurve->apfx[ i-1 ].x );
                                nY += IntFromFixed( pCurve->apfx[ i-1 ].y );
                                nX = (nX + 1) / 2;
                                nY = (nY + 1) / 2;
                                // no need to advance, because the current point
                                // is the control point in next bezier spline
                            }

                            pPoints[ nPnt+2 ] = Point( nX, nY );
                            pFlags[ nPnt+2 ] = POLY_NORMAL;

                            // calculate second cubic control point
                            // P1 = 1/3 * (PEnd + 2 * PQControl)
                            nX = pPoints[ nPnt+2 ].X() + 2 * aControlP.X();
                            nY = pPoints[ nPnt+2 ].Y() + 2 * aControlP.Y();
                            pPoints[ nPnt+1 ] = Point( (2*nX+3)/6, (2*nY+3)/6 );
                            pFlags[ nPnt+1 ] = POLY_CONTROL;

                            nPnt += 3;
                        }
                    }

                    // next curve segment
                    pCurve = (TTPOLYCURVE*)&pCurve->apfx[ i ];
                }

                // end point is start point for closed contour
                // disabled, because Polygon class closes the contour itself
                // pPoints[nPnt++] = pPoints[0];

                // convert y-coordinates W32 -> VCL
                for( int i = 0; i < nPnt; ++i )
                    pPoints[i].Y() = -pPoints[i].Y();

                // insert into polypolygon
                Polygon aPoly( nPnt, pPoints, (bHasOfflinePoints ? pFlags : NULL) );
                rPolyPoly.Insert( aPoly );
            }

            delete[] pPoints;
            delete[] pFlags;
        }

        delete [] pData;
    }

    return bRet;
}

// -----------------------------------------------------------------------

BOOL SalGraphics::CreateFontSubset( const rtl::OUString& rToFile,
    ImplFontData* pFont, long* pGlyphIDs, sal_uInt8* pEncoding,
    sal_Int32* pWidths, int nGlyphs, FontSubsetInfo& rInfo )
{
    // TODO: replace "return FALSE" error with something that cleans up
    BOOL bRet = FALSE;

    // create matching ImplFontSelectData
    ImplFontSelectData aIFSD;
    aIFSD.mpFontData        = pFont;
    aIFSD.mnOrientation     = 0;
    aIFSD.mbVertical        = FALSE;
    aIFSD.mbNonAntialiased  = 0;
    // aIFSD.maName;
    // aIFSD.maStyleName;
    aIFSD.mnWidth           = 0;
    aIFSD.mnHeight          = 1000;     // make compatible to PS units
    aIFSD.meFamily          = pFont->meFamily;
    aIFSD.meCharSet         = pFont->meCharSet;
    aIFSD.meLanguage        = 0;
    aIFSD.meWidthType       = pFont->meWidthType;
    aIFSD.meWeight          = pFont->meWeight;
    aIFSD.meItalic          = pFont->meItalic;
    aIFSD.mePitch           = pFont->mePitch;
    aIFSD.mnOrientation     = 0;
    aIFSD.mbVertical        = false;    // TODO: how to get vertical mode from pFont?
    aIFSD.mbNonAntialiased  = false;

    // TODO: much better solution: move SetFont and restoration of old font to caller
    HFONT hOrigFont = maGraphicsData.mhFont;
    maGraphicsData.mhFont = NULL;   // avoid deletion of current font
    SetFont( &aIFSD );

#ifdef DEBUG
    // get font metrics
    TEXTMETRICA aWinMetric;
    if( !::GetTextMetricsA( maGraphicsData.mhDC, &aWinMetric ) )
        return FALSE;

    DBG_ASSERT( !(aWinMetric.tmPitchAndFamily & TMPF_DEVICE), "cannot subset device font" );
    DBG_ASSERT( aWinMetric.tmPitchAndFamily & TMPF_TRUETYPE, "can only subset TT font" );
    //else if( aWinMetric.tmPitchAndFamily & TMPF_VECTOR )
    //   rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TYPE1;
#endif

    // get raw font file data
    DWORD nFontSize = ::GetFontData( maGraphicsData.mhDC, 0, 0, NULL, 0 );
    if( nFontSize == GDI_ERROR )
        return FALSE;
    char* pRawFontData = new char[ nFontSize ];
    DWORD nFontSize2 = ::GetFontData( maGraphicsData.mhDC, 0, 0, (void*)pRawFontData, nFontSize );
    if( nFontSize != nFontSize2 )
        return FALSE;

    // open font file
    int nFaceNum = 0; // TODO: find correct face number
    TrueTypeFont* pSftTTF = NULL;
    int nRC = ::OpenTTFont( pRawFontData, nFontSize, nFaceNum, &pSftTTF );
    if( nRC != SF_OK )
        return FALSE;

    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( pSftTTF, &aTTInfo );
    rInfo.m_nFontType   = SAL_FONTSUBSETINFO_TYPE_TRUETYPE;
    rInfo.m_aPSName     = ImplSalGetUniString( aTTInfo.psname );
    rInfo.m_nAscent     = +aTTInfo.winAscent;
    rInfo.m_nDescent    = -aTTInfo.winDescent;
    rInfo.m_aFontBBox   = Rectangle( Point( aTTInfo.xMin, aTTInfo.yMin ),
                                    Point( aTTInfo.xMax, aTTInfo.yMax ) );
    rInfo.m_nCapHeight  = aTTInfo.yMax; // Well ...

    // subset glyphs and get their properties
    // take care that subset fonts require the NotDef glyph in pos 0
    int nOrigCount = nGlyphs;
    DBG_ASSERT( nGlyphs < 256, "too many glyphs for subsetting" );
    USHORT    aShortIDs[ 256 ];
    sal_uInt8 aTempEncs[ 256 ];

    int nNotDef=-1, i;
    for( i = 0; i < nGlyphs; ++i )
    {
        aTempEncs[i] = pEncoding[i];
        aShortIDs[i] = static_cast<USHORT>(pGlyphIDs[i]);
        // find NotDef glyph
        if( !pGlyphIDs[i] )
            nNotDef = i;
    }

    if( nNotDef != 0 )
    {
        // add fake NotDef glyph if needed
        if( nNotDef < 0 )
            nNotDef = nGlyphs++;

        // NotDef glyph must be in pos 0 => swap glyphids
        aShortIDs[ nNotDef ] = aShortIDs[0];
        aTempEncs[ nNotDef ] = aTempEncs[0];
        aShortIDs[0] = 0;
        aTempEncs[0] = 0;
    }

    // fill pWidth array
    TTSimpleGlyphMetrics* pMetrics =
        ::GetTTSimpleGlyphMetrics( pSftTTF, aShortIDs, nGlyphs, aIFSD.mbVertical );
    if( !pMetrics )
        return FALSE;
    sal_uInt16 nNotDefAdv   = pMetrics[0].adv;
    pMetrics[0].adv         = pMetrics[nNotDef].adv;
    pMetrics[nNotDef].adv   = nNotDefAdv;
    for( i = 0; i < nOrigCount; ++i )
        pWidths[i] = pMetrics[i].adv;
    free( pMetrics );

    // write subset into destination file
    rtl::OUString aSysPath;
    if( osl_File_E_None != osl_getSystemPathFromFileURL( rToFile.pData, &aSysPath.pData ) )
        return FALSE;
    rtl_TextEncoding aThreadEncoding = osl_getThreadTextEncoding();
    ByteString aToFile( rtl::OUStringToOString( aSysPath, aThreadEncoding ) );
    nRC = ::CreateTTFromTTGlyphs( pSftTTF, aToFile.GetBuffer(), aShortIDs,
            aTempEncs, nGlyphs, 0, NULL, TTCF_IncludeOS2 );
    if( nRC == SF_OK )
        bRet = TRUE;

    // cleanup:
    if( hOrigFont )
    {
        // restore original font, destroy temporary font
        HFONT hTempFont = maGraphicsData.mhFont;
        ::SelectObject( maGraphicsData.mhDC, hOrigFont );
        maGraphicsData.mhFont = hOrigFont;
        DeleteObject( hTempFont );
    }
    // close source font of subsetter
    if( pSftTTF )
        ::CloseTTFont( pSftTTF );
    // delete source font raw data
    delete[] pRawFontData;
    return bRet;
}

//--------------------------------------------------------------------------

const void* SalGraphics::GetEmbedFontData( ImplFontData* pFont, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
{
    // TODO: how to get access to Type 1 font files on this platform?
    return NULL;
}

//--------------------------------------------------------------------------

void SalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    // TODO: once GetEmbedFontData() above does something check implementation below
    free( (void*)pData );
}
