/*************************************************************************
 *
 *  $RCSfile: salgdi3.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 10:04:10 $
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
#include <malloc.h>

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

#ifdef GCP_KERN_HACK
#include <algorithm>
#endif // GCP_KERN_HACK

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

//#ifdef FORCE_VERTICAL_NAME

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

// =======================================================================

// these variables can be static because they store system wide settings
static BOOL bImplSalCourierScalable = FALSE;
static BOOL bImplSalCourierNew = FALSE;

static BOOL FontHasCJKUnicodeRange( const SalGraphics* pGraphics, BOOL &bHasKoreanRange );

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
            // It is unclear why these two (undefined?) code page numbers are
            // handled specially here:
            case 1004:  eTextEncoding = RTL_TEXTENCODING_MS_1252; break;
            case 65400: eTextEncoding = RTL_TEXTENCODING_SYMBOL; break;
            default:
                eTextEncoding = rtl_getTextEncodingFromWindowsCodePage(nCP);
                break;
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

static void ImplSalGetVerticalFontNameW( HDC hDC, UniString& rName,
                                         bool bTestAvail )
{
    if (rName.Len() == 0 || rName.GetChar(0) == '@')
        return;

#ifdef FORCE_VERTICAL_NAME
    rName.Insert( (sal_Unicode)'@', 0 );
    return;
#endif // FORCE_VERTICAL_NAME

    // Vertical fonts starts with a @
    UniString aTemp = rName;
    aTemp.Insert( (sal_Unicode)'@', 0 );

    BOOL bAvailable = !bTestAvail;
    if (!bAvailable)
    {
        // Test, if vertical Font available
        LOGFONTW aLogFont;
        memset( &aLogFont, 0, sizeof( aLogFont ) );
        aLogFont.lfCharSet = DEFAULT_CHARSET;

        UINT nNameLen = aTemp.Len();
        if ( nNameLen > (sizeof( aLogFont.lfFaceName )/sizeof( wchar_t ))-1 )
            nNameLen = (sizeof( aLogFont.lfFaceName )/sizeof( wchar_t ))-1;
        memcpy( aLogFont.lfFaceName, aTemp.GetBuffer(),
                nNameLen*sizeof( wchar_t ) );
        aLogFont.lfFaceName[nNameLen] = 0;

        EnumFontFamiliesExW( hDC, &aLogFont,
                             (FONTENUMPROCW)SalEnumQueryFontProcExW,
                             (LPARAM)(void*)&bAvailable, 0 );
    }
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
                                   LOGFONTW& rLogFont,
                                   bool bTestVerticalAvail )
{
    UniString   aName;
    if ( pFont->mpFontData )
        aName = pFont->mpFontData->maName;
    else
        aName = pFont->maName.GetToken( 0 );

    // Test for vertical
    if ( pFont->mbVertical )
        ImplSalGetVerticalFontNameW( hDC, aName, bTestVerticalAvail );

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
    rLogFont.lfOrientation     = rLogFont.lfEscapement;
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
    if ( !GetTextMetricsW( hDC, &aWinMetric ) )
    {
        // font doesn't work => try a replacement
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
    // when the font doesn't work try a replacement
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

void SalGraphics::RemovingFont( ImplFontData* )
{
    // dummy implementation on W32
}

// -----------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData* pFont, int nFallbackLevel )
{
    HFONT hNewFont = 0;
    HFONT hOldFont;

    if ( aSalShlData.mbWNT )
    {
        LOGFONTW aLogFont;
        ImplGetLogFontFromFontSelect( maGraphicsData.mhDC, pFont, aLogFont, true );

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

    if( !maGraphicsData.mhDefFont )
    {
        // keep default font
        maGraphicsData.mhDefFont = hOldFont;
    }
    else
    {
        // dereference unused fonts
        for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
        {
            if( maGraphicsData.mhFonts[i] )
            {
                DeleteFont( maGraphicsData.mhFonts[i] );
                maGraphicsData.mhFonts[i] = 0;
            }
        }
    }

    // store new font in correct layer
    maGraphicsData.mhFonts[ nFallbackLevel ] = hNewFont;

    if( !nFallbackLevel )
    {
        maGraphicsData.mnFontCharSetCount = 0;
        maGraphicsData.mbFontKernInit = TRUE;
        if ( maGraphicsData.mpFontKernPairs )
        {
            delete[] maGraphicsData.mpFontKernPairs;
            maGraphicsData.mpFontKernPairs = NULL;
        }
        maGraphicsData.mnFontKernPairCount = 0;
    }

    if (maGraphicsData.mxTextLayoutCache.get() != 0)
        maGraphicsData.mxTextLayoutCache->flush( nFallbackLevel );

    // some printers have higher internal resolution, so their
    // text output would be different from what we calculated
    // => suggest DrawTextArray to workaround this problem
    if ( maGraphicsData.mbPrinter )
        return SAL_SETFONT_USEDRAWTEXTARRAY;
    else
        return 0;
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
            pMetric->mnIntLeading       = aWinMetric.tmInternalLeading;
            pMetric->mnExtLeading       = aWinMetric.tmExternalLeading;
            pMetric->mnAscent           = aWinMetric.tmAscent;
            pMetric->mnDescent          = aWinMetric.tmDescent;
            // #107888# workround for Asian...
            // TODO: assess workaround below for CWS >= extleading
            BOOL bHasKoreanRange;
            if ( FontHasCJKUnicodeRange( this, bHasKoreanRange ))
            {
                pMetric->mnIntLeading  += aWinMetric.tmExternalLeading;

                // #109280# The line height for Asian fonts is too small.
                // Therefore we add half of the external leading to the
                // ascent, the other half is added to the descent.
                const long nHalfTmpExtLeading = aWinMetric.tmExternalLeading / 2;
                const long nOtherHalfTmpExtLeading = aWinMetric.tmExternalLeading -
                                                     nHalfTmpExtLeading;

                // #110641# external leading for Asian fonts.
                // The factor 0.3 has been verified during experiments.
                const long nCJKExtLeading = 0.30 * (pMetric->mnAscent + pMetric->mnDescent);

                if ( nCJKExtLeading > aWinMetric.tmExternalLeading )
                    pMetric->mnExtLeading = nCJKExtLeading - aWinMetric.tmExternalLeading;
                else
                    pMetric->mnExtLeading = 0;

                pMetric->mnAscent   += nHalfTmpExtLeading;
                pMetric->mnDescent  += nOtherHalfTmpExtLeading;
            }
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
            pMetric->mnIntLeading       = aWinMetric.tmInternalLeading;
            pMetric->mnExtLeading       = aWinMetric.tmExternalLeading;
            // #107888# workround for Asian...
            // TODO: assess workaround below for CWS >= extleading
            BOOL bHasKoreanRange;
            if ( FontHasCJKUnicodeRange( this, bHasKoreanRange ))    // #107888# worakround for Asian...
            {
                pMetric->mnIntLeading  += aWinMetric.tmExternalLeading;
                pMetric->mnAscent      += aWinMetric.tmExternalLeading;
                // #109280# korean only: increase descent for wavelines and improved line space
                if( bHasKoreanRange )
                    pMetric->mnDescent += aWinMetric.tmExternalLeading;
            }
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
        delete[] pOldPairs;
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

    if ( maGraphicsData.mbFontKernInit )
    {
        if( maGraphicsData.mpFontKernPairs )
        {
            delete[] maGraphicsData.mpFontKernPairs;
            maGraphicsData.mpFontKernPairs = NULL;
        }
        maGraphicsData.mnFontKernPairCount = 0;

        if ( aSalShlData.mbWNT )
        {
            KERNINGPAIR* pPairs = NULL;
            int nCount = ::GetKerningPairsW( maGraphicsData.mhDC, 0, NULL );
            if( nCount )
            {
#ifdef GCP_KERN_HACK
                pPairs = new KERNINGPAIR[ nCount+1 ];
                maGraphicsData.mpFontKernPairs = pPairs;
                maGraphicsData.mnFontKernPairCount = nCount;
                ::GetKerningPairsW( maGraphicsData.mhDC, nCount, pPairs );
#else // GCP_KERN_HACK
                pPairs = pKernPairs;
                nCount = (nCount < nPairs) : nCount : nPairs;
                ::GetKerningPairsW( maGraphicsData.mhDC, nCount, pPairs );
                return nCount;
#endif // GCP_KERN_HACK
            }
        }
        else
        {
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
        }

        maGraphicsData.mbFontKernInit = FALSE;

        std::sort( maGraphicsData.mpFontKernPairs,
            maGraphicsData.mpFontKernPairs + maGraphicsData.mnFontKernPairCount,
            ImplCmpKernData );
    }

    if( !pKernPairs )
        return maGraphicsData.mnFontKernPairCount;
    else if( maGraphicsData.mpFontKernPairs )
    {
        if ( nPairs < maGraphicsData.mnFontKernPairCount )
            nPairs = maGraphicsData.mnFontKernPairCount;
        memcpy( pKernPairs, maGraphicsData.mpFontKernPairs,
                nPairs*sizeof( ImplKernPairData ) );
        return nPairs;
    }

    return 0;
}

// -----------------------------------------------------------------------

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static unsigned GetUShort( const unsigned char* p ){ return((p[0]<<8)+p[1]);}
static signed GetSShort( const unsigned char* p ){ return((short)((p[0]<<8)+p[1]));}
static inline DWORD CalcTag( const char p[4]) { return (p[0]+(p[1]<<8)+(p[2]<<16)+(p[3]<<24)); }

static BOOL FontHasCJKUnicodeRange( const SalGraphics* pGraphics, BOOL &bHasKoreanRange )
{
    BOOL bCJKCapable = FALSE;
    bHasKoreanRange = FALSE;

    if ( pGraphics )
    {
        const DWORD Cos2Tag = CalcTag( "OS/2" );
        DWORD nRCos2 = GetFontData( pGraphics->maGraphicsData.mhDC, Cos2Tag, 0, NULL, 0 );
        if ( nRCos2 != GDI_ERROR )
        {
            int nLength = nRCos2;
            unsigned char* pOS2map = (unsigned char*)alloca( nLength );
            nRCos2 = GetFontData( pGraphics->maGraphicsData.mhDC, Cos2Tag, 0, pOS2map, nLength );
            sal_uInt32 version = GetUShort( pOS2map );
            if ( version >= 0x0001 && nLength >= 58 )
            {
                // We need at least version 0x0001 (TrueType rev 1.66)
                // to have access to the needed struct members.
                sal_uInt32 ulUnicodeRange1 = GetUInt( pOS2map + 42 );
                sal_uInt32 ulUnicodeRange2 = GetUInt( pOS2map + 46 );
                sal_uInt32 ulUnicodeRange3 = GetUInt( pOS2map + 50 );
                sal_uInt32 ulUnicodeRange4 = GetUInt( pOS2map + 54 );

                // Check for CJK capabilities of the current font
                sal_uInt32 nResult = ( ulUnicodeRange2 & 0x2fff0000 ) |
                                     ( ulUnicodeRange3 & 0x00000001 );
                sal_uInt32 nKorean = ( ulUnicodeRange1 & 0x10000000 ) |
                                     ( ulUnicodeRange2 & 0x00100000 ) |
                                     ( ulUnicodeRange2 & 0x01000000 );
                bCJKCapable = ( nResult != 0 );
                bHasKoreanRange = bCJKCapable && ( nKorean != 0 );
            }
        }
    }

    return bCJKCapable;
}

ULONG SalGraphics::GetFontCodeRanges( sal_uInt32* pCodePairs ) const
{
    int nRangeCount = 0;

    // get SFNT font's CMAP table
    const DWORD CmapTag = CalcTag( "cmap" );
    DWORD nRC = GetFontData( maGraphicsData.mhDC, CmapTag, 0, NULL, 0 );
    if( nRC == GDI_ERROR )
        return 0;

    int nLength = nRC;
    std::vector<unsigned char> aCmap( nLength );
    unsigned char* pCmap = &aCmap[0];
    nRC = GetFontData( maGraphicsData.mhDC, CmapTag, 0, pCmap, nLength );
    if( nRC == GDI_ERROR )
        return 0;
    // check for the CMAP table corruption
    if( GetUShort( pCmap ) != 0 )
        return 0;

    int nOffset = 0;
    int nFormat = -1;

    // find suitable subtable
    int nSubTables = GetUShort( pCmap + 2 );
    for( const unsigned char* p = pCmap + 4; --nSubTables >= 0; p += 8 )
    {
        int nPlatform = GetUShort( p );
        int nEncoding = GetUShort( p+2 );
        // ignore non-unicode encodings
        if( nEncoding!=0 && nEncoding!=1 )
            continue;
        nOffset = GetUInt( p+4 );
        nFormat = GetUShort( pCmap + nOffset );
        if( (nFormat == 4) || (nFormat == 12) )
            break;
    }

    sal_uInt32* pCP = pCodePairs;

    // format 4, the most common 16bit unicode mapping table
    if( (nFormat == 4) && ((nOffset+16) < nLength) )
    {
        int nSegCount = GetUShort( pCmap + nOffset + 6 );
        nRangeCount = nSegCount/2 - 1;
        if( pCP )
        {
            const unsigned char* pLimit = pCmap + nOffset + 14;
            const unsigned char* pBegin = pLimit + 2 + nSegCount;
            for( int i = 0; i < nRangeCount; ++i )
            {
                sal_uInt32 cMinChar = GetUShort( pBegin + 2*i );
                sal_uInt32 cMaxChar = GetUShort( pLimit + 2*i );
                if( cMaxChar == 0xFFFF )
                    continue;
                *(pCP++) = cMinChar;
                *(pCP++) = cMaxChar + 1;
            }
            nRangeCount = (pCP - pCodePairs) / 2;
        }
    }

    // format 12, the most common 32bit unicode mapping table
    if( (nFormat == 12) && ((nOffset+16) < nLength) )
    {
        nRangeCount = GetUInt( pCmap + nOffset + 12 ) / 12;
        if( pCP )
        {
            const unsigned char* pGroup = pCmap + nOffset + 16;
            for( int i = 0; i < nRangeCount; ++i )
            {
                *(pCP++) = GetUShort( pGroup + 0 );
                *(pCP++) = GetUShort( pGroup + 4 ) + 1;
                pGroup += 12;
            }
        }
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

struct TempFontItem
{
    ::rtl::OUString maFontFilePath;
    ::rtl::OString maResourcePath;
    TempFontItem* mpNextItem;
};

bool ImplAddTempFont( SalData& rSalData, const String& rFontFileURL )
{
    int nRet = 0;
    ::rtl::OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );

#ifdef FR_PRIVATE
    OSVERSIONINFO aVersion;
    aVersion.dwOSVersionInfoSize = sizeof(aVersion);
    if( ::GetVersionEx( &aVersion ) && (aVersion.dwMajorVersion >= 5) )
    {
        nRet = AddFontResourceExW( aUSytemPath.getStr(), FR_PRIVATE, NULL );
    }
    else
#endif // FR_PRIVATE
    {
        static int nCounter = 0;
        char aFileName[] = "soAA.fot";
        aFileName[2] = 'A' + (15 & (nCounter>>4));
        aFileName[3] = 'A' + (15 & nCounter);
        char aResourceName[512];
        int nMaxLen = sizeof(aResourceName)/sizeof(*aResourceName) - 16;
        int nLen = ::GetTempPathA( nMaxLen, aResourceName );
        ::strcpy( aResourceName + nLen, aFileName );
        ::DeleteFileA( aResourceName );

        rtl_TextEncoding theEncoding = osl_getThreadTextEncoding();
        ::rtl::OString aCFileName = rtl::OUStringToOString( aUSytemPath, theEncoding );
        // TODO: font should be private => need to investigate why it doesn't work then
        if( !::CreateScalableFontResourceA( 0, aResourceName, aCFileName.getStr(), NULL ) )
            return false;
        ++nCounter;

        nRet = ::AddFontResourceA( aResourceName );
        if( nRet > 0 )
        {
            TempFontItem* pNewItem = new TempFontItem;
            pNewItem->maResourcePath = rtl::OString( aResourceName );
            pNewItem->maFontFilePath = aUSytemPath.getStr();
            pNewItem->mpNextItem = rSalData.mpTempFontItem;
            rSalData.mpTempFontItem = pNewItem;
        }
    }

    if( nRet > 0 )
        return true;
    return false;
}

// -----------------------------------------------------------------------

void ImplReleaseTempFonts( SalData& rSalData )
{
    int nCount = 0;
    while( TempFontItem* p = rSalData.mpTempFontItem )
    {
        ++nCount;
        if( p->maResourcePath.getLength() )
        {
            const char* pResourcePath = p->maResourcePath.getStr();
            ::RemoveFontResourceA( pResourcePath );
            ::DeleteFileA( pResourcePath );
        }
        else
        {
            if( aSalShlData.mbWNT )
                ::RemoveFontResourceW( p->maFontFilePath.getStr() );
            else
            {
                // poor man's string conversion because converter is gone
                int nLen = p->maFontFilePath.getLength();
                char* pNameA = new char[ nLen + 1 ];
                for( int i = 0; i < nLen; ++i )
                    pNameA[i] = (char)(p->maFontFilePath.getStr())[i];
                pNameA[ nLen ] = 0;
                ::RemoveFontResourceA( pNameA );
                delete[] pNameA;
            }
        }

        rSalData.mpTempFontItem = p->mpNextItem;
        delete p;
    }

    // notify everybody
    if( nCount )
        ::SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, NULL );
}

// -----------------------------------------------------------------------

String ImplGetFontNameFromFile( SalData& rSalData, const String& rFontFileURL )
{
    int nRet = 0;
    ::rtl::OUString aUSytemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFontFileURL, aUSytemPath ) );

    String aFontName;

    // Create temporary file name
    char aFileName[] = "soAAT.fot";
    char aResourceName[512];
    int nMaxLen = sizeof(aResourceName)/sizeof(*aResourceName) - 16;
    int nLen = ::GetTempPathA( nMaxLen, aResourceName );
#if (_MSC_VER < 1300)
    ::strncpy( aResourceName + nLen, aFileName, std::max( 0, nMaxLen - nLen ));
#else
    ::strncpy( aResourceName + nLen, aFileName, max( 0, nMaxLen - nLen ));
#endif
    ::DeleteFileA( aResourceName );

    // Create font resource file (typically with a .fot file name extension).
    rtl_TextEncoding theEncoding = osl_getThreadTextEncoding();
    ::rtl::OString aCFileName = rtl::OUStringToOString( aUSytemPath, theEncoding );
    ::CreateScalableFontResourceA( 0, aResourceName, aCFileName.getStr(), NULL );

    // Open font resource file
    rtl::OUString aFotFileName = rtl::OStringToOUString( aResourceName, osl_getThreadTextEncoding() );
    osl::FileBase::getFileURLFromSystemPath( aFotFileName, aFotFileName );
    osl::File aFotFile( aFotFileName );
    osl::FileBase::RC aError = aFotFile.open( osl_File_OpenFlag_Read );
    if ( aError == osl::FileBase::E_None )
    {
        static bool bInit = false;
        static char aFontTag[] = "FONTRES:";
        static char aSkip[256];
        if ( !bInit )
        {
            // Initialize skipping array
            memset( aSkip, strlen( aFontTag ), sizeof( aSkip ));
            aSkip['F'] = 7;
            aSkip['O'] = 6;
            aSkip['N'] = 5;
            aSkip['T'] = 4;
            aSkip['R'] = 3;
            aSkip['E'] = 2;
            aSkip['S'] = 1;
            aSkip[':'] = 0;
        }

        char        aBuffer[2048];
        sal_Bool    bFound( sal_True );
        sal_Bool    bEOF( sal_False );
        sal_Int32   nPatternLen = strlen( aFontTag );
        sal_uInt64  nBytesRead;
        ByteString  fontName;

        // Read .fot file header and retrieve font name
        aFotFile.read( aBuffer, sizeof( aBuffer ), nBytesRead );
        if ( nBytesRead > nPatternLen )
        {
            sal_Int32 i, j;
            for ( i = nPatternLen-1, j = nPatternLen-1; j > 0; i--, j-- )
            {
                while ( aBuffer[i] != aFontTag[j] )
                {
                    sal_Int32 nSkip = aSkip[aBuffer[i]];
                    i += ( nPatternLen-j > nSkip ) ? nPatternLen-j : nSkip;
                    if ( i >= nBytesRead )
                    {
                        // Font tag not found
                        aFotFile.close();
                        ::DeleteFileA( aResourceName );
                        return String();
                    }
                    else
                        j = nPatternLen-1;
                }
            }

            // Retrieve font name from position (add tag size)
            i += nPatternLen;
            while ( i < nBytesRead && aBuffer[i] != 0 )
                fontName += aBuffer[i++];
        }

        // Convert byte string to unicode string
        aFontName = String( fontName, osl_getThreadTextEncoding() );
    }

    // Clean up
    aFotFile.close();
    ::DeleteFileA( aResourceName );

    return aFontName;
}

// -----------------------------------------------------------------------

ImplFontData* SalGraphics::AddTempDevFont( const String& rFontFileURL, const String& rFontName )
{
    String aFontName( rFontName );

    // Retrieve font name from font resource
     if( !aFontName.Len() )
        aFontName = ImplGetFontNameFromFile( *GetSalData(), rFontFileURL );

    if ( !aFontName.Len() )
        return NULL;

    if( ::ImplIsFontAvailable( maGraphicsData.mhDC, aFontName ) )
        return NULL;

    // remember temp font for cleanup later
    if( !ImplAddTempFont( *GetSalData(), rFontFileURL ) )
        return NULL;

    UINT nPreferedCharSet = DEFAULT_CHARSET;
    if ( !aSalShlData.mbWNT )
    {
        // for W98 assume prefered charset capability of temp font
        CHARSETINFO aCharSetInfo;
        DWORD nCP = GetACP();
        if ( TranslateCharsetInfo( (DWORD*)nCP, &aCharSetInfo, TCI_SRCCODEPAGE ) )
            nPreferedCharSet = aCharSetInfo.ciCharset;
    }

    // create matching FontData struct
    ImplFontData* pFontData = new ImplFontData;
    pFontData->maName       = aFontName;
    pFontData->mnQuality    = 1000;
    pFontData->mbDevice     = TRUE;
    pFontData->mpSysData    = (void*)nPreferedCharSet;
    pFontData->meCharSet    = nPreferedCharSet;
    pFontData->meFamily     = FAMILY_DONTKNOW;
    pFontData->meWidthType  = WIDTH_DONTKNOW;
    pFontData->meWeight     = WEIGHT_DONTKNOW;
    pFontData->meItalic     = ITALIC_DONTKNOW;
    pFontData->mePitch      = PITCH_DONTKNOW;;
    pFontData->meType       = TYPE_SCALABLE;
    pFontData->mnWidth      = 0;
    pFontData->mnHeight     = 0;
    pFontData->mbSubsettable= FALSE;
    pFontData->mbEmbeddable = FALSE;

#if 0   // TODO: improve ImplFontData using "FONTRES:" from *.fot file
    pFontData->maSearchName = // using "FONTRES:" from file
    if( rFontName != pFontData->maName )
        pFontData->maMapName = aFontName;
#endif

    return pFontData;
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
            String aEmptyString;
            while( aFontDir.getNextItem( aDirItem, 10 ) == osl::FileBase::E_None )
            {
                osl::FileStatus aFileStatus( FileStatusMask_FileURL );
                rcOSL = aDirItem.getFileStatus( aFileStatus );
                if ( rcOSL == osl::FileBase::E_None )
                    AddTempDevFont( aFileStatus.getFileURL(), aEmptyString );
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
        EnumFontFamiliesExW( maGraphicsData.mhDC, &aLogFont,
            (FONTENUMPROCW)SalEnumFontsProcExW, (LPARAM)(void*)&aInfo, 0 );
    }
    else
    {
        LOGFONTA aLogFont;
        memset( &aLogFont, 0, sizeof( aLogFont ) );
        aLogFont.lfCharSet = DEFAULT_CHARSET;
        aInfo.mpLogFontA = &aLogFont;
        EnumFontFamiliesExA( maGraphicsData.mhDC, &aLogFont,
            (FONTENUMPROCA)SalEnumFontsProcExA, (LPARAM)(void*)&aInfo, 0 );
    }

    // Feststellen, was es fuer Courier-Schriften auf dem Bildschirm gibt,
    // um in SetFont() evt. Courier auf Courier New zu mappen
    if ( !maGraphicsData.mbPrinter )
    {
        bImplSalCourierScalable = aInfo.mbImplSalCourierScalable;
        bImplSalCourierNew      = aInfo.mbImplSalCourierNew;
    }
}

// ----------------------------------------------------------------------------

void SalGraphics::GetDevFontSubstList( OutputDevice* pOutDev )
{}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( long nIndex, Rectangle& rRect, const OutputDevice* )
{
    HDC hDC = maGraphicsData.mhDC;

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_METRICS;
    if( !(nIndex & GF_ISCHAR) )
        nGGOFlags |= GGO_GLYPH_INDEX;
    nIndex &= GF_IDXMASK;

    GLYPHMETRICS aGM;
    DWORD nSize = GDI_ERROR;
    if ( aSalShlData.mbWNT )
        nSize = ::GetGlyphOutlineW( hDC, nIndex, nGGOFlags, &aGM, 0, NULL, &aMat );
    else if( (nGGOFlags & GGO_GLYPH_INDEX) || (nIndex <= 255) )
        nSize = ::GetGlyphOutlineA( hDC, nIndex, nGGOFlags, &aGM, 0, NULL, &aMat );

    if( nSize == GDI_ERROR )
        return false;

    rRect = Rectangle( Point( +aGM.gmptGlyphOrigin.x, -aGM.gmptGlyphOrigin.y ),
        Size( aGM.gmBlackBoxX, aGM.gmBlackBoxY ) );
    return true;
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphOutline( long nIndex, PolyPolygon& rPolyPoly, const OutputDevice* )
{
    rPolyPoly.Clear();

    BOOL bRet = FALSE;
    HDC  hDC = maGraphicsData.mhDC;

    // use unity matrix
    MAT2 aMat;
    aMat.eM11 = aMat.eM22 = FixedFromDouble( 1.0 );
    aMat.eM12 = aMat.eM21 = FixedFromDouble( 0.0 );

    UINT nGGOFlags = GGO_NATIVE;
    if( !(nIndex & GF_ISCHAR) )
        nGGOFlags |= GGO_GLYPH_INDEX;
    nIndex &= GF_IDXMASK;

    GLYPHMETRICS aGlyphMetrics;
    DWORD nSize1 = GDI_ERROR;
    if ( aSalShlData.mbWNT )
        nSize1 = ::GetGlyphOutlineW( hDC, nIndex, nGGOFlags, &aGlyphMetrics, 0, NULL, &aMat );
    else if( (nGGOFlags & GGO_GLYPH_INDEX) || (nIndex <= 255) )
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

// TODO:  Replace this class with boost::scoped_array
class ScopedCharArray
{
public:
    inline explicit ScopedCharArray(char * pArray): m_pArray(pArray) {}

    inline ~ScopedCharArray() { delete[] m_pArray; }

    inline char * get() const { return m_pArray; }

private:
    char * m_pArray;
};

class ScopedFont
{
public:
    explicit ScopedFont(SalGraphicsData & rData);

    ~ScopedFont();

private:
    SalGraphicsData & m_rData;
    HFONT m_hOrigFont;
};

ScopedFont::ScopedFont(SalGraphicsData & rData): m_rData(rData)
{
    m_hOrigFont = m_rData.mhFonts[0];
    m_rData.mhFonts[0] = 0; // avoid deletion of current font
}

ScopedFont::~ScopedFont()
{
    if( m_hOrigFont )
    {
        // restore original font, destroy temporary font
        HFONT hTempFont = m_rData.mhFonts[0];
        m_rData.mhFonts[0] = m_hOrigFont;
        SelectObject( m_rData.mhDC, m_hOrigFont );
        DeleteObject( hTempFont );
    }
}

class ScopedTrueTypeFont
{
public:
    inline ScopedTrueTypeFont(): m_pFont(0) {}

    ~ScopedTrueTypeFont();

    int open(void * pBuffer, sal_uInt32 nLen, sal_uInt32 nFaceNum);

    inline TrueTypeFont * get() const { return m_pFont; }

private:
    TrueTypeFont * m_pFont;
};

ScopedTrueTypeFont::~ScopedTrueTypeFont()
{
    if (m_pFont != 0)
        CloseTTFont(m_pFont);
}

int ScopedTrueTypeFont::open(void * pBuffer, sal_uInt32 nLen,
                             sal_uInt32 nFaceNum)
{
    OSL_ENSURE(m_pFont == 0, "already open");
    return OpenTTFont(pBuffer, nLen, nFaceNum, &m_pFont);
}

BOOL SalGraphics::CreateFontSubset( const rtl::OUString& rToFile,
    ImplFontData* pFont, long* pGlyphIDs, sal_uInt8* pEncoding,
    sal_Int32* pWidths, int nGlyphs, FontSubsetInfo& rInfo )
{
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
    ScopedFont aOldFont(maGraphicsData);
    SetFont( &aIFSD, 0 );

#if OSL_DEBUG_LEVEL > 1
    // get font metrics
    TEXTMETRICA aWinMetric;
    if( !::GetTextMetricsA( maGraphicsData.mhDC, &aWinMetric ) )
        return FALSE;

    DBG_ASSERT( !(aWinMetric.tmPitchAndFamily & TMPF_DEVICE), "cannot subset device font" );
    DBG_ASSERT( aWinMetric.tmPitchAndFamily & TMPF_TRUETYPE, "can only subset TT font" );
#endif

    // get raw font file data
    DWORD nFontSize = ::GetFontData( maGraphicsData.mhDC, 0, 0, NULL, 0 );
    if( nFontSize == GDI_ERROR )
        return FALSE;
    ScopedCharArray xRawFontData(new char[ nFontSize ]);
    DWORD nFontSize2 = ::GetFontData( maGraphicsData.mhDC, 0, 0, (void*)xRawFontData.get(), nFontSize );
    if( nFontSize != nFontSize2 )
        return FALSE;

    // open font file
    sal_uInt32 nFaceNum = 0;
    if( !*xRawFontData.get() )  // TTC candidate
        nFaceNum = ~0;  // indicate "TTC font extracts only"

    ScopedTrueTypeFont aSftTTF;
    int nRC = aSftTTF.open( xRawFontData.get(), nFontSize, nFaceNum );
    if( nRC != SF_OK )
        return FALSE;

    TTGlobalFontInfo aTTInfo;
    ::GetTTGlobalFontInfo( aSftTTF.get(), &aTTInfo );
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
    USHORT    aShortIDs[ 256 ];
    sal_uInt8 aTempEncs[ 256 ];

    int nNotDef=-1, i;
    for( i = 0; i < nGlyphs; ++i )
    {
        aTempEncs[i] = pEncoding[i];
        sal_uInt32 nGlyphIdx = pGlyphIDs[i] & GF_IDXMASK;
        if( pGlyphIDs[i] & GF_ISCHAR )
        {
            nGlyphIdx = MapChar( aSftTTF.get(), nGlyphIdx, (pGlyphIDs[i] & GF_ROTMASK) ? 1 : 0 );
            if( nGlyphIdx == 0 && SYMBOL_CHARSET == (BYTE)pFont->mpSysData )
            {
                // #i12824# emulate symbol aliasing U+FXXX <-> U+0XXX
                nGlyphIdx = pGlyphIDs[i] & GF_IDXMASK;
                nGlyphIdx = (nGlyphIdx & 0xF000) ? (nGlyphIdx & 0x00FF) : (nGlyphIdx | 0xF000 );
                nGlyphIdx = MapChar( aSftTTF.get(), nGlyphIdx, (pGlyphIDs[i] & GF_ROTMASK) ? 1 : 0 );
            }
        }
        aShortIDs[i] = static_cast<USHORT>( nGlyphIdx );
        if( !nGlyphIdx )
            if( nNotDef < 0 )
                nNotDef = i; // first NotDef glyph found
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
    DBG_ASSERT( nGlyphs < 257, "too many glyphs for subsetting" );

    // fill pWidth array
    TTSimpleGlyphMetrics* pMetrics =
        ::GetTTSimpleGlyphMetrics( aSftTTF.get(), aShortIDs, nGlyphs, aIFSD.mbVertical );
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
    nRC = ::CreateTTFromTTGlyphs( aSftTTF.get(), aToFile.GetBuffer(), aShortIDs,
            aTempEncs, nGlyphs, 0, NULL, 0 );
    return nRC == SF_OK;
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

const std::map< sal_Unicode, sal_Int32 >* SalGraphics::GetFontEncodingVector( ImplFontData* pFont, const std::map< sal_Unicode, rtl::OString >** pNonEncoded )
{
    // TODO: once GetEmbedFontData() above does something this needs implementation
    if( pNonEncoded )
        *pNonEncoded = NULL;
    return NULL;
}
