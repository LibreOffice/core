/*************************************************************************
 *
 *  $RCSfile: outdev3.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: th $ $Date: 2001-04-06 12:47:30 $
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

#include <math.h>
#include <string.h>

#define _SV_OUTDEV_CXX

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#endif

#ifndef REMOTE_APPSERVER
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#else
#ifndef _SV_RMOUTDEV_HXX
#include <rmoutdev.hxx>
#endif
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <metric.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_OUTDATA_HXX
#include <outdata.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <print.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <edit.hxx>
#endif

#include <unohelp.hxx>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HDL_
#include <com/sun/star/beans/PropertyValues.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#if defined UNX
#define GLYPH_FONT_HEIGHT   128
#elif defined OS2
#define GLYPH_FONT_HEIGHT   176
#else
#define GLYPH_FONT_HEIGHT   256
#endif

// =======================================================================

DBG_NAMEEX( OutputDevice );
DBG_NAMEEX( Font );

// =======================================================================

using namespace ::com::sun::star;
using namespace ::rtl;

// =======================================================================

#define TEXT_DRAW_ELLIPSIS  (TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_PATHELLIPSIS | TEXT_DRAW_NEWSELLIPSIS)

// =======================================================================

#define UNDERLINE_LAST      UNDERLINE_BOLDWAVE
#define STRIKEOUT_LAST      STRIKEOUT_X

// -----------------------------------------------------------------------

static void ImplRotatePos( long nOriginX, long nOriginY, long& rX, long& rY,
                           short nOrientation )
{
    if ( (nOrientation >= 0) && !(nOrientation % 900) )
    {
        if ( (nOrientation >= 3600) )
            nOrientation %= 3600;

        if ( nOrientation )
        {
            rX -= nOriginX;
            rY -= nOriginY;

            if ( nOrientation == 900 )
            {
                long nTemp = rX;
                rX = rY;
                rY = -nTemp;
            }
            else if ( nOrientation == 1800 )
            {
                rX = -rX;
                rY = -rY;
            }
            else /* ( nOrientation == 2700 ) */
            {
                long nTemp = rX;
                rX = -rY;
                rY = nTemp;
            }

            rX += nOriginX;
            rY += nOriginY;
        }
    }
    else
    {
        double nRealOrientation = nOrientation*F_PI1800;
        double nCos = cos( nRealOrientation );
        double nSin = sin( nRealOrientation );

        // Translation...
        long nX = rX-nOriginX;
        long nY = rY-nOriginY;

        // Rotation...
        rX =  ((long)(nCos*nX + nSin*nY)) + nOriginX;
        rY = -((long)(nSin*nX - nCos*nY)) + nOriginY;
    }
}

// =======================================================================

void OutputDevice::ImplUpdateFontData( BOOL bNewFontLists )
{
    if ( mpFontEntry )
    {
        mpFontCache->Release( mpFontEntry );
        mpFontEntry = NULL;
    }
    if ( bNewFontLists )
    {
        if ( mpGetDevFontList )
        {
            delete mpGetDevFontList;
            mpGetDevFontList = NULL;
        }
        if ( mpGetDevSizeList )
        {
            delete mpGetDevSizeList;
            mpGetDevSizeList = NULL;
        }
    }

    if ( GetOutDevType() == OUTDEV_PRINTER )
    {
        mpFontCache->Clear();

        if ( bNewFontLists )
        {
#ifndef REMOTE_APPSERVER
            // we need a graphics
            if ( ImplGetGraphics() )
#endif
            {
                mpFontList->Clear();
                mpGraphics->GetDevFontList( mpFontList );
            }
        }
    }

    mbInitFont = TRUE;
    mbNewFont = TRUE;

    // Bei Fenstern auch alle Child-Fenster mit updaten
    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        Window* pChild = ((Window*)this)->mpFirstChild;
        while ( pChild )
        {
            pChild->ImplUpdateFontData( TRUE );
            pChild = pChild->mpNext;
        }
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplUpdateAllFontData( BOOL bNewFontLists )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Alle Fenster updaten
    Window* pFrame = pSVData->maWinData.mpFirstFrame;
    while ( pFrame )
    {
        pFrame->ImplUpdateFontData( bNewFontLists );

        Window* pSysWin = pFrame->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            pSysWin->ImplUpdateFontData( bNewFontLists );
            pSysWin = pSysWin->mpNextOverlap;
        }

        pFrame = pFrame->mpFrameData->mpNextFrame;
    }

    // Alle VirDev's updaten
    VirtualDevice* pVirDev = pSVData->maGDIData.mpFirstVirDev;
    while ( pVirDev )
    {
        pVirDev->ImplUpdateFontData( bNewFontLists );
        pVirDev = pVirDev->mpNext;
    }

    // Alle Printer updaten
    Printer* pPrinter = pSVData->maGDIData.mpFirstPrinter;
    while ( pPrinter )
    {
        pPrinter->ImplUpdateFontData( bNewFontLists );
        pPrinter = pPrinter->mpNext;
    }

    // Globale Fontlisten leeren, damit diese geupdatet werden
    pSVData->maGDIData.mpScreenFontCache->Clear();
    if ( bNewFontLists )
    {
        pSVData->maGDIData.mpScreenFontList->Clear();
        pFrame = pSVData->maWinData.mpFirstFrame;
        if ( pFrame )
        {
#ifndef REMOTE_APPSERVER
            if ( pFrame->ImplGetGraphics() )
#endif
                pFrame->mpGraphics->GetDevFontList( pFrame->mpFrameData->mpFontList );
        }
    }
}

// =======================================================================

struct ImplLocaliziedFontName
{
    const char*         mpEnglishName;
    const sal_Unicode*  mpLocaliziedNames;
};

static sal_Unicode const aBatang[] = { 0xBC14, 0xD0D5, 0, 0 };
static sal_Unicode const aBatangChe[] = { 0xBC14, 0xD0D5, 0xCCB4, 0, 0 };
static sal_Unicode const aGungsuh[] = { 0xAD81, 0xC11C, 0, 0 };
static sal_Unicode const aGungsuhChe[] = { 0xAD81, 0xC11C, 0xCCB4, 0, 0 };
static sal_Unicode const aGulim[] = { 0xAD74, 0xB9BC, 0, 0 };
static sal_Unicode const aGulimChe[] = { 0xAD74, 0xB9BC, 0xCCB4, 0, 0 };
static sal_Unicode const aDotum[] = { 0xB3CB, 0xC6C0, 0, 0 };
static sal_Unicode const aDotumChe[] = { 0xB3CB, 0xC6C0, 0xCCB4, 0, 0 };
static sal_Unicode const aSimSun[] = { 0x5B8B, 0x4F53, 0, 0 };
static sal_Unicode const aNSimSun[] = { 0x65B0, 0x5B8B, 0x4F53, 0, 0 };
static sal_Unicode const aMingLiU[] = { 0x7D30, 0x660E, 0x9AD4, 0, 0 };
static sal_Unicode const aPMingLiU[] = { 0x65B0, 0x7D30, 0x660E, 0x9AD4, 0, 0 };
static sal_Unicode const aMSGothic[] = { 'm', 's', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
static sal_Unicode const aMSPGothic[] = { 'm', 's', 'p', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
static sal_Unicode const aMSMincho[] = { 'm', 's', 0x660E, 0x671D, 0, 0 };
static sal_Unicode const aMSPMincho[] = { 'm', 's', 'p', 0x660E, 0x671D, 0, 0 };

static ImplLocaliziedFontName const aImplLocaliziedNamesList[] =
{
{   "batang",               aBatang },
{   "batangche",            aBatangChe },
{   "gungshu",              aGungsuh },
{   "gungshuche",           aGungsuhChe },
{   "gulim",                aGulim },
{   "gulimche",             aGulimChe },
{   "dotum",                aDotum },
{   "dotumche",             aDotumChe },
{   "simsun",               aSimSun },
{   "nsimsun",              aNSimSun },
{   "mingliu",              aMingLiU },
{   "pmingliu",             aPMingLiU },
{   "msgothic",             aMSGothic },
{   "mspgothic",            aMSPGothic },
{   "msmincho",             aMSMincho },
{   "mspmincho",            aMSPMincho },
{   NULL,                   NULL },
};

// -----------------------------------------------------------------------

static void ImplGetEnglishSearchName( String& rName,
                                      BOOL bDelScript = TRUE,
                                      BOOL* pbScript = NULL  )
{
    BOOL        bTranslate = FALSE;
    xub_StrLen  i;
    xub_StrLen  nLen = rName.Len();

    // Remove trailing whitespaces
    i = nLen;
    while ( i && (rName.GetChar( i-1 ) < 32) )
        i--;
    if ( i != nLen )
        rName.Erase( i );

    // Remove Script at the end
    // Scriptname must be the last part of the fontname and
    // looks like "fontname (scriptname)". So there can only be a
    // script name at the and of the fontname, when the last char is ')'
    if ( (nLen >= 3) && rName.GetChar( nLen-1 ) == ')' )
    {
        int nOpen = 1;
        xub_StrLen nTempLen = nLen-2;
        while ( nTempLen )
        {
            if ( rName.GetChar( nTempLen ) == '(' )
            {
                nOpen--;
                if ( !nOpen )
                {
                    // Remove Space at the end
                    if ( nTempLen && (rName.GetChar( nTempLen-1 ) == ' ') )
                        nTempLen--;
                    rName.Erase( nTempLen );
                    nLen = nTempLen;
                    break;
                }
            }
            if ( rName.GetChar( nTempLen ) == ')' )
                nOpen++;
            nTempLen--;
        }
    }

    // This function removes all whitespaces, convert to Lowercase-ASCII
    i = 0;
    while ( i < nLen )
    {
        sal_Unicode c = rName.GetChar( i );
        if ( c > 127 )
        {
            // Translate to Lowercase-ASCII
            // FullWidth-ASCII to half ASCII
            if ( (c >= 0xFF00) && (c <= 0xFF5E) )
            {
                c -= 0xFF00-0x0020;
                // Upper to Lower
                if ( (c >= 0x0041) && (c <= 0x005A) )
                    c += 0x0020;
                rName.SetChar( i, c );
            }
            else
            {
                // Only Fontnames with None-Ascii-Characters must be translated
                bTranslate = TRUE;
            }
        }
        // not lowercase Ascii
        else if ( !((c >= 0x0061) && (c <= 0x007A)) )
        {
            // To Lowercase-Ascii
            if ( (c >= 0x0041) && (c <= 0x005A) )
            {
                c += 0x0020;
                rName.SetChar( i, c );
            }
            else if ( !((c >= 0x0030) && (c <= 0x0039)) ) // not 0-9
            {
                // Remove white spaces and special characters
                rName.Erase( i, 1 );
                nLen--;
                continue;
            }
        }

        i++;
    }

    // Translate localizied name to English ASCII name
    const ImplLocaliziedFontName* pTranslateNames = aImplLocaliziedNamesList;
    while ( bTranslate && pTranslateNames->mpEnglishName )
    {
        const sal_Unicode* pLocaliziedName = pTranslateNames->mpLocaliziedNames;
        while ( *pLocaliziedName )
        {
            if ( rName.Equals( pLocaliziedName ) )
            {
                rName.AssignAscii( pTranslateNames->mpEnglishName );
                bTranslate = FALSE;
                break;
            }

            // Run to the end of the Token (\0\0 is the end mark)
            while ( *pLocaliziedName )
                pLocaliziedName++;
            pLocaliziedName++;
        }

        pTranslateNames++;
    }
};

// =======================================================================

static const char* const aImplKillTrailingList[] =
{
    "ms",
    "monotype",
    "mt",
    "itc",
    "adobe",
    "sun",
    "cg",
    "hg",
    NULL,
};

// -----------------------------------------------------------------------

static const char* const aImplKillLeadingList[] =
{
    "ms",
    "monotype",
    "mt",
    "itc",
    "adobe",
    "sun",
    // Scripts, for compatibility with older versions
    "ce",
    "we",
    "cyr",
    "tur",
    "wt",
    "greek",
    "wl",
    NULL,
};

// -----------------------------------------------------------------------

static const char* const aImplKillList[] =
{
    "ms",
    "monotype",
    "mt",
    "itc",
    "adobe",
    "sun",
    NULL,
};

// -----------------------------------------------------------------------

struct ImplFontAttrWeightSearchData
{
    const char*             mpStr;
    FontWeight              meWeight;
};

static ImplFontAttrWeightSearchData const aImplWeightAttrSearchList[] =
{
{   "extrablack",           WEIGHT_BLACK },
{   "ultrablack",           WEIGHT_BLACK },
{   "black",                WEIGHT_BLACK },
{   "heavy",                WEIGHT_BLACK },
{   "ultrabold",            WEIGHT_ULTRABOLD },
{   "semibold",             WEIGHT_SEMIBOLD },
{   "bold",                 WEIGHT_BOLD },
{   "ultralight",           WEIGHT_ULTRALIGHT },
{   "semilight",            WEIGHT_SEMILIGHT },
{   "light",                WEIGHT_LIGHT },
{   "demi",                 WEIGHT_SEMIBOLD },
{   "medium",               WEIGHT_MEDIUM },
{   NULL,                   WEIGHT_DONTKNOW },
};

// -----------------------------------------------------------------------

struct ImplFontAttrWidthSearchData
{
    const char*             mpStr;
    FontWidth               meWidth;
};

static ImplFontAttrWidthSearchData const aImplWidthAttrSearchList[] =
{
{   "narrow",               WIDTH_CONDENSED },
{   "semicondensed",        WIDTH_SEMI_CONDENSED },
{   "ultracondensed",       WIDTH_ULTRA_CONDENSED },
{   "semiexpanded",         WIDTH_SEMI_EXPANDED },
{   "ultraexpanded",        WIDTH_ULTRA_EXPANDED },
{   "expanded",             WIDTH_EXPANDED },
{   "wide",                 WIDTH_ULTRA_EXPANDED },
{   "condensed",            WIDTH_CONDENSED },
{   "cond",                 WIDTH_CONDENSED },
{   "cn",                   WIDTH_CONDENSED },
{   NULL,                   WIDTH_DONTKNOW },
};

// -----------------------------------------------------------------------

// IMPL_FONT_ATTR_STANDARD      - Standard-Font like Arial, Times, Courier, ...
// IMPL_FONT_ATTR_NORMAL        - normal Font for writing text like Arial, Verdana, Arial Narrow, Trebuchet, Times, Courier, ...
// IMPL_FONT_ATTR_SYMBOL        - Font with symbols
// IMPL_FONT_ATTR_DECORATIVE    - Readable and normally used for drawings
// IMPL_FONT_ATTR_SPECIAL       - very special design
// IMPL_FONT_ATTR_TITLING       - only uppercase characters
// IMPL_FONT_ATTR_FULL          - Font with normally all characters
// IMPL_FONT_ATTR_CAPTITALS     - only uppercase characters, but lowercase characters smaller as the uppercase characters
// IMPL_FONT_ATTR_TYPEWRITER    - like a typewriter: Courier, ...
// IMPL_FONT_ATTR_SCRIPT        - Handwriting or Script
// IMPL_FONT_ATTR_HANDWRITING   - More Handwriting with normal letters
// IMPL_FONT_ATTR_CHANCERY      - Like Zapf Chancery
// IMPL_FONT_ATTR_COMIC         - Like Comic Sans MS
// IMPL_FONT_ATTR_BRUSHSCRIPT   - More Script
#define IMPL_FONT_ATTR_STANDARD      ((ULONG)0x00000001)
#define IMPL_FONT_ATTR_NORMAL        ((ULONG)0x00000002)
#define IMPL_FONT_ATTR_SYMBOL        ((ULONG)0x00000004)
#define IMPL_FONT_ATTR_FIXED         ((ULONG)0x00000008)
#define IMPL_FONT_ATTR_SANSSERIF     ((ULONG)0x00000010)
#define IMPL_FONT_ATTR_SERIF         ((ULONG)0x00000020)
#define IMPL_FONT_ATTR_DECORATIVE    ((ULONG)0x00000040)
#define IMPL_FONT_ATTR_SPECIAL       ((ULONG)0x00000080)
#define IMPL_FONT_ATTR_CJK           ((ULONG)0x00000100)
#define IMPL_FONT_ATTR_CJK_JP        ((ULONG)0x00000200)
#define IMPL_FONT_ATTR_CJK_ZH        ((ULONG)0x00000400)
#define IMPL_FONT_ATTR_CJK_TW        ((ULONG)0x00000800)
#define IMPL_FONT_ATTR_CJK_KR        ((ULONG)0x00001000)
#define IMPL_FONT_ATTR_ARABIC        ((ULONG)0x00002000)
#define IMPL_FONT_ATTR_NONELATIN     ((ULONG)0x00004000)
#define IMPL_FONT_ATTR_FULL          ((ULONG)0x00008000)
#define IMPL_FONT_ATTR_ITALIC        ((ULONG)0x00010000)
#define IMPL_FONT_ATTR_TITLING       ((ULONG)0x00020000)
#define IMPL_FONT_ATTR_CAPITALS      ((ULONG)0x00040000)
#define IMPL_FONT_ATTR_OUTLINE       ((ULONG)0x00080000)
#define IMPL_FONT_ATTR_SHADOW        ((ULONG)0x00100000)
#define IMPL_FONT_ATTR_ROUNDED       ((ULONG)0x00200000)
#define IMPL_FONT_ATTR_TYPEWRITER    ((ULONG)0x00400000)
#define IMPL_FONT_ATTR_SCRIPT        ((ULONG)0x00800000)
#define IMPL_FONT_ATTR_HANDWRITING   ((ULONG)0x01000000)
#define IMPL_FONT_ATTR_CHANCERY      ((ULONG)0x02000000)
#define IMPL_FONT_ATTR_COMIC         ((ULONG)0x04000000)
#define IMPL_FONT_ATTR_BRUSHSCRIPT   ((ULONG)0x08000000)
#define IMPL_FONT_ATTR_OLDSTYLE      ((ULONG)0x10000000)
#define IMPL_FONT_ATTR_NEWSTYLE      ((ULONG)0x20000000)
#define IMPL_FONT_ATTR_GOTHIC        ((ULONG)0x40000000)
#define IMPL_FONT_ATTR_SCHOOLBOOK    ((ULONG)0x80000000)

struct ImplFontAttrTypeSearchData
{
    const char*             mpStr;
    ULONG                   mnType;
};

static ImplFontAttrTypeSearchData const aImplTypeAttrSearchList[] =
{
{   "titling",              IMPL_FONT_ATTR_TITLING },
{   "captitals",            IMPL_FONT_ATTR_CAPITALS },
{   "captital",             IMPL_FONT_ATTR_CAPITALS },
{   "italic",               IMPL_FONT_ATTR_ITALIC },
{   "oblique",              IMPL_FONT_ATTR_ITALIC },
{   "rounded",              IMPL_FONT_ATTR_ROUNDED },
{   "outline",              IMPL_FONT_ATTR_OUTLINE },
{   "shadow",               IMPL_FONT_ATTR_SHADOW },
{   "handwriting",          IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_SCRIPT },
{   "hand",                 IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_SCRIPT },
{   "signet",               IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_SCRIPT },
{   "script",               IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_SCRIPT },
{   "calligraphy",          IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_SCRIPT },
{   "chancery",             IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_SCRIPT },
{   "oldstyle",             IMPL_FONT_ATTR_OLDSTYLE },
{   "oldface",              IMPL_FONT_ATTR_OLDSTYLE },
{   "old",                  IMPL_FONT_ATTR_OLDSTYLE },
{   "new",                  IMPL_FONT_ATTR_NEWSTYLE },
{   "gothic",               IMPL_FONT_ATTR_GOTHIC },
{   "schoolbook",           IMPL_FONT_ATTR_SCHOOLBOOK },
{   "typewriter",           IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "lineprinter",          IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "monospaced",           IMPL_FONT_ATTR_FIXED },
{   "monospace",            IMPL_FONT_ATTR_FIXED },
{   "fixed",                IMPL_FONT_ATTR_FIXED },
{   "sansserif",            IMPL_FONT_ATTR_SANSSERIF },
{   "sans",                 IMPL_FONT_ATTR_SANSSERIF },
{   "swiss",                IMPL_FONT_ATTR_SANSSERIF },
{   "serif",                IMPL_FONT_ATTR_SERIF },
{   "roman",                IMPL_FONT_ATTR_SERIF },
{   "bright",               IMPL_FONT_ATTR_SERIF },
{   "symbols",              IMPL_FONT_ATTR_SYMBOL },
{   "symbol",               IMPL_FONT_ATTR_SYMBOL },
{   "dingbats",             IMPL_FONT_ATTR_SYMBOL },
{   "bats",                 IMPL_FONT_ATTR_SYMBOL },
{   "math",                 IMPL_FONT_ATTR_SYMBOL },
{   NULL,                   0 },
};

// -----------------------------------------------------------------------

static BOOL ImplKillTrailing( String& rName, const char* pStr )
{
    const xub_Unicode* pNameStr = rName.GetBuffer();
    while ( (*pNameStr == (xub_Unicode)(unsigned char)*pStr) && *pStr )
    {
        pNameStr++;
        pStr++;
    }
    if ( *pStr )
        return FALSE;

    rName.Erase( 0, (xub_StrLen)(pNameStr-rName.GetBuffer()) );
    return TRUE;
}

// -----------------------------------------------------------------------

static BOOL ImplKillLeading( String& rName, const char* pStr )
{
    const char* pTempStr = pStr;
    while ( *pTempStr )
        pTempStr++;

    xub_StrLen nStrLen = (xub_StrLen)(pTempStr-pStr);
    const xub_Unicode* pNameStr = rName.GetBuffer();
    pNameStr += rName.Len()-nStrLen;
    while ( (*pNameStr == (xub_Unicode)(unsigned char)*pStr) && *pStr )
    {
        pNameStr++;
        pStr++;
    }
    if ( *pStr )
        return FALSE;

    rName.Erase( rName.Len()-nStrLen );
    return TRUE;
}

// -----------------------------------------------------------------------

static BOOL ImplFindAndErase( String& rName, const char* pStr )
{
    xub_StrLen nPos = rName.SearchAscii( pStr );
    if ( nPos == STRING_NOTFOUND )
        return FALSE;

    const char* pTempStr = pStr;
    while ( *pTempStr )
        pTempStr++;
    rName.Erase( nPos, (xub_StrLen)(pTempStr-pStr) );
    return TRUE;
}

// -----------------------------------------------------------------------

static void ImplGetMapName( const String& rOrgName,
                            String& rShortName, String& rFamilyName,
                            FontWeight& rWeight, FontWidth& rWidth,
                            ULONG& rType )
{
    const char* const* ppStr;

    rShortName = rOrgName;

    // Kill trailing vendor names and other unimportant data
    ppStr = aImplKillTrailingList;
    while ( *ppStr )
    {
        if ( ImplKillTrailing( rShortName, *ppStr ) )
            break;
        ppStr++;
    }

    // Kill leading vendor names and other unimportant data
    ppStr = aImplKillLeadingList;
    while ( *ppStr )
    {
        if ( ImplKillLeading( rShortName, *ppStr ) )
            break;
        ppStr++;
    }

    rFamilyName = rShortName;

    // Kill attributes from the name and update the data
    // Weight
    const ImplFontAttrWeightSearchData* pWeightList = aImplWeightAttrSearchList;
    while ( pWeightList->mpStr )
    {
        if ( ImplFindAndErase( rFamilyName, pWeightList->mpStr ) )
        {
            if ( (rWeight == WEIGHT_DONTKNOW) || (rWeight == WEIGHT_NORMAL) )
                rWeight = pWeightList->meWeight;
            break;
        }
        pWeightList++;
    }

    // Width
    const ImplFontAttrWidthSearchData* pWidthList = aImplWidthAttrSearchList;
    while ( pWidthList->mpStr )
    {
        if ( ImplFindAndErase( rFamilyName, pWidthList->mpStr ) )
        {
            if ( (rWidth == WIDTH_DONTKNOW) || (rWidth == WIDTH_NORMAL) )
                rWidth = pWidthList->meWidth;
            break;
        }
        pWidthList++;
    }

    // Type
    rType = 0;
    const ImplFontAttrTypeSearchData* pTypeList = aImplTypeAttrSearchList;
    while ( pTypeList->mpStr )
    {
        if ( ImplFindAndErase( rFamilyName, pTypeList->mpStr ) )
            rType |= pTypeList->mnType;
        pTypeList++;
    }

    // Other unimportant data (vendor names)
    ppStr = aImplKillList;
    while ( *ppStr )
    {
        if ( ImplFindAndErase( rFamilyName, *ppStr ) )
            break;
        ppStr++;
    }
}

// =======================================================================

static char const aImplSubsSerif[] = "thorndale;timesnewroman;times;roman;lucidaserif;lucidabright;bookman;garamond;timmons;serif";
static char const aImplSubsSans[] = "albany;arial;helvetica;lucidasans;lucida;geneva;helmet;sansserif";
static char const aImplSubsFixed[] = "cumberland;couriernew;courier;lucidatypewriter;lucidasanstypewriter;monospaced";
static char const aImplSubsSymbol[] = "starbats;wingdings;zapfdingbats;symbol;lucidadingbats;lucidasansdingbats";
static char const aImplSubsBrushScript[] = "palacescript;arioso;monotypecorsiva;corsiva;zapfchancery;lucidacalligraphy;lucidahandwriting";
static char const aImplSubsOutline[] = "monotypeoldstyleboldoutline;imprint;imprintmtshadow;chevaraoutline;chevara;gallia;colonnamt;algerian;castellar";
static char const aImplSubsBroadway[] = "broadway;mtbroadway;latinwide;falstaff;impact";
static char const aImplSubsSheffield[] = "sheffield;conga;centurygothic;copperlategothic;felixtitling";
static char const aImplSubsSansNarrow[] = "arialnarrow;helveticanarrow;helmetcondensed";
static char const aImplSubsSansUnicode[] = "andalewtui;arialunicodems;lucidaunicode";
static char const aImplSubsJPGothic[] = "msgothic;mspgothic;andalewtui";
static char const aImplSubsJPMincho[] = "msmincho;mspmincho;hgminchoj;hgminchol;minchol;mincho;andalewtui";
static char const aImplSubsZH[] = "simsun;nsimsun;andalewtui";
static char const aImplSubsTW[] = "mingliu;pmingliu;andalewtui";
static char const aImplSubsKR[] = "batang;batangche;gulim;gulimche;dotum;dotumche;gungsuh;gungsuhche;myeomjo;andalewtui";

// -----------------------------------------------------------------------

struct ImplFontNameAttr
{
    const char*             mpName;
    const char*             mpSubstitution1;
    const char*             mpSubstitution2;
    const char*             mpSubstitution3;
    FontWeight              meWeight;
    FontWidth               meWidth;
    ULONG                   mnType;
};

// List is sorted alphabetic
static ImplFontNameAttr const aImplFontNameList[] =
{
{   "albany",               aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "algerian",             aImplSubsOutline, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_TITLING },
{   "almanac",              aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "andalesans",           aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "arial",                aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "arialnarrow",          aImplSubsSansNarrow, aImplSubsSans, aImplSubsSansUnicode, WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "arialunicode",         aImplSubsSansUnicode,aImplSubsSans, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_FULL },
{   "arioso",               aImplSubsBrushScript, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "batang",               aImplSubsKR, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "batangche",            aImplSubsKR, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "bookman",              aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "bookmanoldstyle",      aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_OLDSTYLE },
{   "broadway",             aImplSubsBroadway, NULL, NULL, WEIGHT_BOLD, WIDTH_NORMAL, IMPL_FONT_ATTR_DECORATIVE },
{   "castellar",            aImplSubsOutline, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_TITLING },
{   "century",              aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "centuryschoolbook",    aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_SCHOOLBOOK },
{   "cgtimes",              aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "chevara",              aImplSubsOutline, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_CAPITALS },
{   "chevaraoutline",       aImplSubsOutline, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_CAPITALS },
{   "colonna",              aImplSubsOutline, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE },
{   "courier",              aImplSubsFixed, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED | IMPL_FONT_ATTR_STANDARD },
{   "couriernew",           aImplSubsFixed, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED | IMPL_FONT_ATTR_STANDARD },
{   "cumberland",           aImplSubsFixed, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED | IMPL_FONT_ATTR_STANDARD },
{   "dotum",                aImplSubsKR, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "dotumche",             aImplSubsKR, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "extra",                aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "frutiger",             aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "garamond",             aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "gothic",               aImplSubsJPGothic, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "gulim",                aImplSubsKR, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "gulimche",             aImplSubsKR, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "gungsuh",              aImplSubsKR, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "gungsuhche",           aImplSubsKR, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "helmet",               aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "helmetcondensed",      aImplSubsSansNarrow, aImplSubsSans, aImplSubsSansUnicode, WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "helvetica",            aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "helveticanarrow",      aImplSubsSansNarrow, aImplSubsSans, NULL, WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "holidays",             aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "imprintmtshadow",      aImplSubsOutline, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE },
{   "marlett",              aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "mincho",               aImplSubsJPMincho, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "minchoj",              aImplSubsJPMincho, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "minchol",              aImplSubsJPMincho, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "mingliu",              aImplSubsTW, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TW },
{   "monospace",            aImplSubsFixed, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "monospaced",           aImplSubsFixed, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "myeomjo",              aImplSubsKR, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "nsimsun",              aImplSubsZH, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_ZH },
{   "ocean",                aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "omega",                aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "outlook",              aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "palacescript",         aImplSubsBrushScript, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "palatino",             aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "pgothic",              aImplSubsJPGothic, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "pmincho",              aImplSubsJPMincho, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "pmingliu",             aImplSubsTW, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TW },
{   "sansserif",            aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "segoe",                aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "serif",                aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "sheffield",            aImplSubsSheffield, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL },
{   "simsun",               aImplSubsZH, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_ZH },
{   "sorts",                aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "sorts2",               aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "starbats",             aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "starmath",             aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "symbol",               aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "tahoma",               aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "thorndale",            aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_STANDARD },
{   "times",                aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_STANDARD },
{   "timesnewroman",        aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_STANDARD },
{   "timmons",              aImplSubsSerif, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "trebuchet",            aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "univers",              aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "utah",                 aImplSubsSans, aImplSubsSansUnicode, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "vacation",             aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "verdana",              aImplSubsSans, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "webdings",             aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "webdings2",            aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "wingdings",            aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "wingdings2",           aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "wingdings3",           aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "zapfdingbats",         aImplSubsSymbol, NULL, NULL, WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
};

// -----------------------------------------------------------------------

static int ImplStrMatchCompare( const String& rStr1, const char* pStr2 )
{
    const sal_Unicode* pStr1 = rStr1.GetBuffer();
    while ( (*pStr1 == (xub_Unicode)(unsigned char)*pStr2) && *pStr1 )
    {
        pStr1++;
        pStr2++;
    }

    if ( !(*pStr1) )
        return 0;
    else
        return *pStr1-((xub_Unicode)(unsigned char)*pStr2);
}

// -----------------------------------------------------------------------

static const ImplFontNameAttr* ImplGetFontNameAttr( const String& rName )
{
    int nCount = sizeof( aImplFontNameList ) / sizeof( ImplFontNameAttr );
    for( int nLower = 0, nUpper = nCount-1; nLower <= nUpper; )
    {
        long nMid = (nUpper + nLower) >> 1;
        int nComp = ImplStrMatchCompare( rName, aImplFontNameList[nMid].mpName );
        if ( !nComp )
        {
            // Find shortest match
            while ( nMid &&
                    (ImplStrMatchCompare( rName, aImplFontNameList[nMid-1].mpName ) == 0) )
                nMid--;
            return &(aImplFontNameList[nMid]);
        }
        else if ( nComp < 0 )
            nUpper = nMid-1;
        else /* ( nComp > 0 ) */
            nLower = nMid+1;
    }

    return NULL;
}

// =======================================================================

struct ImplFontSubstEntry
{
    String                  maName;
    String                  maReplaceName;
    String                  maSearchName;
    String                  maSearchReplaceName;
    USHORT                  mnFlags;
    ImplFontSubstEntry*     mpNext;
};

// =======================================================================

void ImplFreeOutDevFontData()
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    while ( pEntry )
    {
        ImplFontSubstEntry* pNext = pEntry->mpNext;
        delete pEntry;
        pEntry = pNext;
    }
}

// =======================================================================

void OutputDevice::BeginFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maGDIData.mbFontSubChanged = FALSE;
}

// -----------------------------------------------------------------------

void OutputDevice::EndFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maGDIData.mbFontSubChanged )
    {
        ImplUpdateAllFontData( FALSE );

        Application* pApp = GetpApp();
        DataChangedEvent aDCEvt( DATACHANGED_FONTSUBSTITUTION );
        pApp->DataChanged( aDCEvt );
        pApp->NotifyAllWindows( aDCEvt );
        pSVData->maGDIData.mbFontSubChanged = FALSE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::AddFontSubstitute( const XubString& rFontName,
                                      const XubString& rReplaceFontName,
                                      USHORT nFlags )
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = new ImplFontSubstEntry;

    pEntry->maName              = rFontName;
    pEntry->maReplaceName       = rReplaceFontName;
    pEntry->maSearchName        = rFontName;
    pEntry->maSearchReplaceName = rReplaceFontName;
    pEntry->mnFlags             = nFlags;
    pEntry->mpNext              = pSVData->maGDIData.mpFirstFontSubst;
    ImplGetEnglishSearchName( pEntry->maSearchName );
    ImplGetEnglishSearchName( pEntry->maSearchReplaceName );

    pSVData->maGDIData.mpFirstFontSubst = pEntry;
    pSVData->maGDIData.mbFontSubChanged = TRUE;
}

// -----------------------------------------------------------------------

void OutputDevice::RemoveFontSubstitute( USHORT n )
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    ImplFontSubstEntry* pPrev = NULL;
    USHORT              nCount = 0;
    while ( pEntry )
    {
        if ( nCount == n )
        {
            pSVData->maGDIData.mbFontSubChanged = TRUE;
            if ( pPrev )
                pPrev->mpNext = pEntry->mpNext;
            else
                pSVData->maGDIData.mpFirstFontSubst = pEntry->mpNext;
            delete pEntry;
            break;
        }

        nCount++;
        pPrev = pEntry;
        pEntry = pEntry->mpNext;
    }
}

// -----------------------------------------------------------------------

USHORT OutputDevice::GetFontSubstituteCount()
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    USHORT              nCount = 0;
    while ( pEntry )
    {
        nCount++;
        pEntry = pEntry->mpNext;
    }

    return nCount;
}

// -----------------------------------------------------------------------

void OutputDevice::GetFontSubstitute( USHORT n,
                                      XubString& rFontName,
                                      XubString& rReplaceFontName,
                                      USHORT& rFlags )
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    USHORT              nCount = 0;
    while ( pEntry )
    {
        if ( nCount == n )
        {
            rFontName           = pEntry->maName;
            rReplaceFontName    = pEntry->maReplaceName;
            rFlags              = pEntry->mnFlags;
            break;
        }

        nCount++;
        pEntry = pEntry->mpNext;
    }
}

// -----------------------------------------------------------------------

static BOOL ImplFontSubstitute( XubString& rFontName,
                                USHORT nFlags1, USHORT nFlags2 )
{
#ifdef DBG_UTIL
    String aTempName = rFontName;
    ImplGetEnglishSearchName( aTempName );
    DBG_ASSERT( aTempName == rFontName, "ImplFontSubstitute() called without a searchname" );
#endif

    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    while ( pEntry )
    {
        if ( ((pEntry->mnFlags & nFlags1) == nFlags2) &&
             (pEntry->maSearchName == rFontName) )
        {
            rFontName = pEntry->maSearchReplaceName;
            return TRUE;
        }

        pEntry = pEntry->mpNext;
    }

    return FALSE;
}

// =======================================================================

static char const aImplDefSansUnicode[] = "Andale WT UI;Arial Unicode MS;Lucida Sans Unicode";
static char const aImplDefSansUI[] = "WarpSans;MS Sans Serif;Geneva;Helv;Dialog;Albany;Lucida;Helvetica;Charcoal;Chicago;Tahoma;Arial;Helmet;Interface System;Sans Serif";
static char const aImplDefSans[] = "Albany;Arial;Helvetica;Lucida;Helmet;SansSerif";
static char const aImplDefSerif[] = "Thorndale;Times New Roman;Times;Lucida Serif;Lucida Bright;Timmons;Serif";
static char const aImplDefFixed[] = "Cumberland;Courier New;Courier;Lucida Typewriter;Lucida Sans Typewriter;Monospaced";
static char const aImplDefSymbol[] = "StarBats;WingDings;Zapf Dingbats;Symbol";
static char const aImplDef_CJK_JP_Mincho[] = "MS Mincho;HG Mincho J;HG Mincho L;HG Mincho";
static char const aImplDef_CJK_JP_Gothic[] = "MS Gothic;HG Gothic J;HG Gothic";
static char const aImplDef_CJK_ZH[] = "SimSun";
static char const aImplDef_CJK_TW[] = "MingLiU";
static char const aImplDef_CJK_KR_Batang[] = "Batang";
static char const aImplDef_CJK_KR_Gulim[] = "Gulim";

// -----------------------------------------------------------------------

static void ImplAddTokenFontNames( String& rName, const char* pFontNames )
{
    const char* pStr = pFontNames;
    do
    {
        if ( !(*pStr) || (*pStr == ';') )
        {
            String      aName( pFontNames, pStr-pFontNames, RTL_TEXTENCODING_ASCII_US );
            String      aTempName;
            xub_StrLen  nIndex = 0;
            do
            {
                aTempName = rName.GetToken( 0, ';', nIndex );
                if ( aName == aTempName )
                {
                    aName.Erase();
                    break;
                }
            }
            while ( nIndex != STRING_NOTFOUND );

            if ( aName.Len() )
            {
                if ( rName.Len() )
                    rName += ';';
                rName += aName;
            }

            if ( !(*pStr) )
                break;

            pFontNames = pStr+1;
        }

        pStr++;
    }
    while ( 1 );
}

// -----------------------------------------------------------------------

Font OutputDevice::GetDefaultFont( USHORT nType,
                                   LanguageType eLang,
                                   BOOL bOnlyOne,
                                   const OutputDevice* pOutDev )
{
    Font            aFont;
    const char*     pSearch1 = NULL;
    const char*     pSearch2 = NULL;
    const char*     pSearch3 = NULL;

    switch ( nType )
    {
        case FONT_DEFAULT_SANS_UNICODE:
        case FONT_DEFAULT_UI_SANS:
            pSearch1 = aImplDefSansUnicode;
            if ( nType == FONT_DEFAULT_UI_SANS )
                pSearch2 = aImplDefSansUI;
            else
                pSearch2 = aImplDefSans;
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case FONT_DEFAULT_SANS:
        case FONT_DEFAULT_LATIN_HEADING:
        case FONT_DEFAULT_LATIN_SPREADSHEET:
        case FONT_DEFAULT_LATIN_DISPLAY:
            pSearch1 = aImplDefSans;
            pSearch2 = aImplDefSansUnicode;
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case FONT_DEFAULT_SERIF:
        case FONT_DEFAULT_LATIN_TEXT:
        case FONT_DEFAULT_LATIN_PRESENTATION:
            pSearch1 = aImplDefSerif;
            aFont.SetFamily( FAMILY_ROMAN );
            break;

        case FONT_DEFAULT_FIXED:
        case FONT_DEFAULT_LATIN_FIXED:
        case FONT_DEFAULT_UI_FIXED:
            aFont.SetPitch( PITCH_FIXED );
            aFont.SetFamily( FAMILY_MODERN );
            pSearch1 = aImplDefFixed;
            break;

        case FONT_DEFAULT_SYMBOL:
            aFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
            pSearch1 = aImplDefSymbol;
            break;

        case FONT_DEFAULT_CJK_TEXT:
        case FONT_DEFAULT_CJK_PRESENTATION:
        case FONT_DEFAULT_CJK_SPREADSHEET:
        case FONT_DEFAULT_CJK_HEADING:
        case FONT_DEFAULT_CJK_DISPLAY:
            if ( (eLang == LANGUAGE_CHINESE) ||
                 (eLang == LANGUAGE_CHINESE_SIMPLIFIED) ||
                 (eLang == LANGUAGE_CHINESE_SINGAPORE) )
                pSearch1 = aImplDef_CJK_ZH;
            else if ( (eLang == LANGUAGE_CHINESE_TRADITIONAL) ||
                      (eLang == LANGUAGE_CHINESE_HONGKONG) ||
                      (eLang == LANGUAGE_CHINESE_MACAU) )
                pSearch1 = aImplDef_CJK_TW;
            else if ( eLang == LANGUAGE_KOREAN )
            {
                if ( nType == FONT_DEFAULT_CJK_DISPLAY )
                    pSearch1 = aImplDef_CJK_KR_Gulim;
                else
                    pSearch1 = aImplDef_CJK_KR_Batang;
            }
            else
            {
                if ( (nType == FONT_DEFAULT_CJK_DISPLAY) ||
                     (nType == FONT_DEFAULT_CJK_SPREADSHEET) ||
                     (nType == FONT_DEFAULT_CJK_PRESENTATION) )
                    pSearch1 = aImplDef_CJK_JP_Gothic;
                else
                    pSearch1 = aImplDef_CJK_JP_Mincho;
            }
            pSearch2 = aImplDefSansUnicode;
            break;

        case FONT_DEFAULT_CTL_TEXT:
        case FONT_DEFAULT_CTL_PRESENTATION:
        case FONT_DEFAULT_CTL_SPREADSHEET:
        case FONT_DEFAULT_CTL_HEADING:
        case FONT_DEFAULT_CTL_DISPLAY:
            pSearch1 = "Arial Unicode MS";
            pSearch2 = aImplDefSansUnicode;
            break;
    }

    if ( pSearch1 )
    {
        aFont.SetSize( Size( 0, 12 ) );
        aFont.SetWeight( WEIGHT_NORMAL );
        if ( aFont.GetPitch() == PITCH_DONTKNOW )
            aFont.SetPitch ( PITCH_VARIABLE );
        if ( aFont.GetCharSet() == RTL_TEXTENCODING_DONTKNOW )
            aFont.SetCharSet( gsl_getSystemTextEncoding() );

        // Should be found the standard font on the given device
        if ( bOnlyOne && pOutDev )
        {
            // Create Token String
            String aNames( pSearch1, RTL_TEXTENCODING_ASCII_US );
            if ( pSearch2 )
            {
                aNames += ';';
                aNames.AppendAscii( pSearch2 );
                if ( pSearch3 )
                {
                    aNames += ';';
                    aNames.AppendAscii( pSearch2 );
                }
            }

            // Search Font in the FontList
            String      aTempName;
            xub_StrLen  nIndex = 0;
            do
            {
                aTempName = aNames.GetToken( 0, ';', nIndex );
                ImplGetEnglishSearchName( aTempName );
                ImplDevFontListData* pFoundData = pOutDev->mpFontList->ImplFind( aTempName );
                if ( pFoundData )
                {
                    aFont.SetName( pFoundData->mpFirst->maName );
                    break;
                }
            }
            while ( nIndex != STRING_NOTFOUND );
        }

        // No Name, than set all names
        if ( !aFont.GetName().Len() )
        {
            if ( bOnlyOne )
            {
                const char* pStr = pSearch1;
                while ( *pStr && (*pStr != '!') )
                    pStr++;
                String aName( pSearch1, pStr-pSearch1, RTL_TEXTENCODING_ASCII_US );
                aFont.SetName( aName );
            }
            else
            {
                String aName( pSearch1, RTL_TEXTENCODING_ASCII_US );
                if ( pSearch2 )
                    ImplAddTokenFontNames( aName, pSearch2 );
                if ( pSearch3 )
                    ImplAddTokenFontNames( aName, pSearch3 );
                aFont.SetName( aName );
            }
        }
    }

    return aFont;
}

// =======================================================================

ImplDevFontList::ImplDevFontList() :
    List( CONTAINER_MAXBLOCKSIZE, 96, 32 )
{
}

// -----------------------------------------------------------------------

ImplDevFontList::~ImplDevFontList()
{
    // Alle Eintraege loeschen
    ImplDevFontListData* pEntry = First();
    while ( pEntry )
    {
        // Liste der Font loeschen
        ImplFontData* pFontData = pEntry->mpFirst;
        do
        {
            ImplFontData* pTempFontData = pFontData;
            pFontData = pFontData->mpNext;
            delete pTempFontData;
        }
        while ( pFontData );
        // Entry loeschen
        delete pEntry;

        pEntry = Next();
    }
}

// -----------------------------------------------------------------------

static StringCompare ImplCompareFontDataWithoutSize( const ImplFontData* pEntry1,
                                                     const ImplFontData* pEntry2 )
{
    // Vergleichen nach Groesse, Breite, Weight, Italic, StyleName
    if ( pEntry1->meWidthType < pEntry2->meWidthType )
        return COMPARE_LESS;
    else if ( pEntry1->meWidthType > pEntry2->meWidthType )
        return COMPARE_GREATER;

    if ( pEntry1->meWeight < pEntry2->meWeight )
        return COMPARE_LESS;
    else if ( pEntry1->meWeight > pEntry2->meWeight )
        return COMPARE_GREATER;

    if ( pEntry1->meItalic < pEntry2->meItalic )
        return COMPARE_LESS;
    else if ( pEntry1->meItalic > pEntry2->meItalic )
        return COMPARE_GREATER;

    return pEntry1->maStyleName.CompareTo( pEntry2->maStyleName );
}

// -----------------------------------------------------------------------

static StringCompare ImplCompareFontData( const ImplFontData* pEntry1,
                                          const ImplFontData* pEntry2 )
{
    StringCompare eComp = ImplCompareFontDataWithoutSize( pEntry1, pEntry2 );
    if ( eComp != COMPARE_EQUAL )
        return eComp;

    if ( pEntry1->mnHeight < pEntry2->mnHeight )
        return COMPARE_LESS;
    else if ( pEntry1->mnHeight > pEntry2->mnHeight )
        return COMPARE_GREATER;

    if ( pEntry1->mnWidth < pEntry2->mnWidth )
        return COMPARE_LESS;
    else if ( pEntry1->mnWidth > pEntry2->mnWidth )
        return COMPARE_GREATER;

    return COMPARE_EQUAL;
}

// -----------------------------------------------------------------------

void ImplDevFontList::Add( ImplFontData* pNewData )
{
    XubString aSearchName = pNewData->maName;
    ImplGetEnglishSearchName( aSearchName );

    // Add Font
    ULONG                   nIndex;
    ImplDevFontListData*    pFoundData = ImplFind( aSearchName, &nIndex );
    BOOL                    bInsert = TRUE;

    if ( !pFoundData )
    {
        pFoundData                  = new ImplDevFontListData;
        pFoundData->maName          = pNewData->maName;
        pFoundData->maSearchName    = aSearchName;
        pFoundData->mpFirst         = pNewData;
        pFoundData->meFamily        = FAMILY_DONTKNOW;
        pFoundData->mePitch         = PITCH_DONTKNOW;
        pFoundData->mbScalable      = FALSE;
        pFoundData->mbSymbol        = FALSE;
        pNewData->mpNext            = NULL;
        Insert( pFoundData, nIndex );
        bInsert = FALSE;
    }

    // set Match data
    if ( !pFoundData->mbScalable )
    {
        if ( (pNewData->meType == TYPE_SCALABLE) && (pNewData->mnHeight == 0) )
            pFoundData->mbScalable = TRUE;
    }
    if ( !pFoundData->mbSymbol )
    {
        if ( pNewData->meCharSet == RTL_TEXTENCODING_SYMBOL )
            pFoundData->mbSymbol = TRUE;
    }
    if ( pFoundData->meFamily == FAMILY_DONTKNOW )
        pFoundData->meFamily = pNewData->meFamily;
    if ( pFoundData->mePitch == PITCH_DONTKNOW )
        pFoundData->mePitch = pNewData->mePitch;

    // Add map/alias names
    if ( pNewData->maMapNames.Len() )
    {
        String      aName;
        xub_StrLen  nIndex = 0;
        do
        {
            aName = pNewData->maMapNames.GetToken( 0, ';', nIndex );
            ImplGetEnglishSearchName( aName );
            if ( aName != aSearchName )
            {
                // Test, if Alias exists already
                String      aTempName;
                xub_StrLen  nIndex2 = 0;
                do
                {
                    aTempName = pFoundData->maMapNames.GetToken( 0, ';', nIndex2 );
                    if ( aName == aTempName )
                    {
                        aName.Erase();
                        break;
                    }
                }
                while ( nIndex2 != STRING_NOTFOUND );

                if ( aName.Len() )
                {
                    if ( pFoundData->maMapNames.Len() )
                        pFoundData->maMapNames += ';';
                    pFoundData->maMapNames += aName;
                }
            }
        }
        while ( nIndex != STRING_NOTFOUND );
    }

    if ( bInsert )
    {
        // replace Name (saves memory)
        if ( pNewData->maName == pFoundData->maName )
            pNewData->maName = pFoundData->maName;

        ImplFontData*   pPrev = NULL;
        ImplFontData*   pTemp = pFoundData->mpFirst;
        do
        {
            StringCompare eComp = ImplCompareFontData( pNewData, pTemp );
            if ( eComp != COMPARE_GREATER )
            {
                // Wenn Font gleich ist, nehmen wir einen Devicefont,
                // oder ignorieren den Font
                if ( eComp == COMPARE_EQUAL )
                {
                    // Wir nehmen den Font mit der besseren Quality,
                    // ansonsten ziehen wir den Device-Font vor
                    if ( (pNewData->mnQuality > pTemp->mnQuality) ||
                         ((pNewData->mnQuality == pTemp->mnQuality) &&
                          (pNewData->mbDevice && !pTemp->mbDevice)) )
                    {
                        pNewData->mpNext = pTemp->mpNext;
                        if ( pPrev )
                            pPrev->mpNext = pNewData;
                        else
                            pFoundData->mpFirst = pNewData;
                        delete pTemp;
                    }
                    else
                        delete pNewData;

                    bInsert = FALSE;
                }
                break;
            }

            pPrev = pTemp;
            pTemp = pTemp->mpNext;
        }
        while ( pTemp );

        if ( bInsert )
        {
            pNewData->mpNext = pTemp;
            if ( pPrev )
                pPrev->mpNext = pNewData;
            else
                pFoundData->mpFirst = pNewData;
        }
    }
}

// -----------------------------------------------------------------------

ImplDevFontListData* ImplDevFontList::ImplFind( const XubString& rFontName, ULONG* pIndex ) const
{
#ifdef DBG_UTIL
    String aTempName = rFontName;
    ImplGetEnglishSearchName( aTempName );
    DBG_ASSERT( aTempName == rFontName, "ImplDevFontList::ImplFind() called without a searchname" );
#endif

    ULONG nCount = Count();
    if ( !nCount )
    {
        if ( pIndex )
            *pIndex = LIST_APPEND;
        return NULL;
    }

    // Fonts in der Liste suchen
    ImplDevFontListData*    pCompareData;
    ImplDevFontListData*    pFoundData = NULL;
    ULONG                   nLow = 0;
    ULONG                   nHigh = nCount-1;
    ULONG                   nMid;
    StringCompare           eCompare;

    do
    {
        nMid = (nLow + nHigh) / 2;
        pCompareData = Get( nMid );
        eCompare = rFontName.CompareTo( pCompareData->maSearchName );
        if ( eCompare == COMPARE_LESS )
        {
            if ( !nMid )
                break;
            nHigh = nMid-1;
        }
        else
        {
            if ( eCompare == COMPARE_GREATER )
                nLow = nMid + 1;
            else
            {
                pFoundData = pCompareData;
                break;
            }
        }
    }
    while ( nLow <= nHigh );

    if ( pIndex )
    {
        eCompare = rFontName.CompareTo( pCompareData->maSearchName );
        if ( eCompare == COMPARE_GREATER )
            *pIndex = (nMid+1);
        else
            *pIndex = nMid;
    }

    return pFoundData;
}

// -----------------------------------------------------------------------

ImplDevFontListData* ImplDevFontList::FindFont( const XubString& rFontName ) const
{
    XubString aName = rFontName;
    ImplGetEnglishSearchName( aName );
    return ImplFind( aName );
}

// -----------------------------------------------------------------------

ImplDevFontListData* ImplDevFontList::ImplFindFontFromToken( const char* pStr ) const
{
    const char* pTempStr = pStr;
    while ( *pTempStr )
    {
        if ( *pTempStr == ';' )
        {
            String aName( pStr, pTempStr-pStr, RTL_TEXTENCODING_ASCII_US );
            if ( aName.Len() )
            {
                ImplDevFontListData* pData = ImplFind( aName );
                if ( pData )
                    return pData;
            }
            pStr = pTempStr+1;
        }

        pTempStr++;
    }

    String aName( pStr, pTempStr-pStr, RTL_TEXTENCODING_ASCII_US );
    if ( aName.Len() )
        return ImplFind( aName );
    else
        return NULL;
}

// -----------------------------------------------------------------------

void ImplDevFontList::Clear()
{
    // Alle Eintraege loeschen
    ImplDevFontListData* pEntry = First();
    while ( pEntry )
    {
        // Liste der Font loeschen
        ImplFontData* pFontData = pEntry->mpFirst;
        do
        {
            ImplFontData* pTempFontData = pFontData;
            pFontData = pFontData->mpNext;
            delete pTempFontData;
        }
        while ( pFontData );
        // Entry loeschen
        delete pEntry;

        pEntry = Next();
    }

    List::Clear();
}

// =======================================================================

void ImplGetDevSizeList::Add( long nNewHeight )
{
    ULONG n = Count();
    if ( !n || (nNewHeight > Get( n-1 )) )
        Insert( (void*)nNewHeight, LIST_APPEND );
    else
    {
        for ( ULONG i=0 ; i < n; i++ )
        {
            long nHeight = Get( i );

            if ( nNewHeight <= nHeight )
            {
                if ( nNewHeight != nHeight )
                    Insert( (void*)nNewHeight, i );
                break;
            }
        }
    }
}

// =======================================================================

ImplFontEntry::~ImplFontEntry()
{
    if ( mpWidthAry )
        delete mpWidthAry;

    if ( mpKernPairs )
        delete mpKernPairs;
}

// =======================================================================

ImplFontCache::ImplFontCache( BOOL bPrinter )
{
    mpFirstEntry    = NULL;
    mnRef0Count     = 0;
    mbPrinter       = bPrinter;
}

// -----------------------------------------------------------------------

ImplFontCache::~ImplFontCache()
{
    // Alle Eintraege loeschen
    ImplFontEntry* pTemp;
    ImplFontEntry* pEntry = mpFirstEntry;
    while ( pEntry )
    {
        pTemp = pEntry->mpNext;
        delete pEntry;
        pEntry = pTemp;
    }
}

// -----------------------------------------------------------------------

ImplFontEntry* ImplFontCache::Get( ImplDevFontList* pFontList,
                                   const Font& rFont, const Size& rSize )
{
#ifdef DBG_UTIL
    {
    // Test all Fonttables for correct sorting
    static BOOL bInit = FALSE;
    if ( !bInit )
    {
        const ImplFontNameAttr* pList = aImplFontNameList;
        int                     nCount = sizeof( aImplFontNameList ) / sizeof( ImplFontNameAttr );
        int                     i;
        for ( i = 1; i < nCount; i++ )
        {
            if ( ImplStrMatchCompare( String( pList[i].mpName, RTL_TEXTENCODING_ASCII_US ),
                                      pList[i-1].mpName ) <= 0 )
            {
                ByteString aStr( "ImplFontNameList not sorted: " );
                aStr += pList[i-1].mpName;
                aStr.Append( " >= " );
                aStr += pList[i].mpName;
                DBG_ERROR( aStr.GetBuffer() );
            }
        }
        bInit = TRUE;
    }
    }
#endif

    const XubString& rName      = rFont.GetName();
    const XubString& rStyleName = rFont.GetStyleName();
    long nWidth                 = rSize.Width();
    long nHeight                = rSize.Height();
    FontFamily eFamily          = rFont.GetFamily();
    CharSet eCharSet            = rFont.GetCharSet();
    FontWeight eWeight          = rFont.GetWeight();
    FontItalic eItalic          = rFont.GetItalic();
    FontPitch ePitch            = rFont.GetPitch();
    short nOrientation          = rFont.GetOrientation();
    BOOL bVertical              = rFont.IsVertical();

    // Make Orientation positiv and between 0 and 2700
    if ( nOrientation )
    {
        while ( nOrientation < 0 )
            nOrientation += 3600;
        nOrientation %= 3600;
    }

    // Groesse anpassen
    if ( nHeight < 0 )
        nHeight = -nHeight;
    if ( nWidth < 0 )
        nWidth = -nWidth;

    // Eintrag suchen
    ImplFontEntry* pPrevEntry = NULL;
    ImplFontEntry* pEntry = mpFirstEntry;
    while ( pEntry )
    {
        ImplFontSelectData* pFontSelData = &(pEntry->maFontSelData);
        if ( (nHeight       == pFontSelData->mnHeight)    &&
             (eWeight       == pFontSelData->meWeight)    &&
             (eItalic       == pFontSelData->meItalic)    &&
             (rName         == pFontSelData->maName)      &&
             (rStyleName    == pFontSelData->maStyleName) &&
             (eCharSet      == pFontSelData->meCharSet)   &&
             (eFamily       == pFontSelData->meFamily)    &&
             (ePitch        == pFontSelData->mePitch)     &&
             (nWidth        == pFontSelData->mnWidth)     &&
             (bVertical     == pFontSelData->mbVertical)  &&
             (nOrientation  == pFontSelData->mnOrientation) )
        {
            if ( !pEntry->mnRefCount )
                mnRef0Count--;

            pEntry->mnRefCount++;

            // Entry nach vorne bringen
            if ( pPrevEntry )
            {
                pPrevEntry->mpNext = pEntry->mpNext;
                pEntry->mpNext = mpFirstEntry;
                mpFirstEntry = pEntry;
            }

            return pEntry;
        }

        pPrevEntry = pEntry;
        pEntry = pEntry->mpNext;
    }

    ImplFontData*           pFontData = NULL;
    ImplDevFontListData*    pFoundData;
    String                  aSearchName;
    USHORT                  nSubstFlags1 = FONT_SUBSTITUTE_ALWAYS;
    USHORT                  nSubstFlags2 = FONT_SUBSTITUTE_ALWAYS;
    xub_StrLen              nFirstNameIndex = 0;
    xub_StrLen              nIndex = 0;
    int                     nToken = 0;

    if ( mbPrinter )
        nSubstFlags1 |= FONT_SUBSTITUTE_SCREENONLY;

    // Test if one Font in the name list is available
    do
    {
        nToken++;
        aSearchName = rName.GetToken( 0, ';', nIndex );
        ImplGetEnglishSearchName( aSearchName );
        ImplFontSubstitute( aSearchName, nSubstFlags1, nSubstFlags2 );
        pFoundData = pFontList->ImplFind( aSearchName );
        if ( pFoundData )
            break;
    }
    while ( nIndex != STRING_NOTFOUND );

    // Danach versuchen wir es nocheinmal unter Beruecksichtigung
    // der gloablen Fontersetzungstabelle, wobei wir jetzt auch
    // die Fonts nehmen, die ersetzt werden sollen, wenn sie
    // nicht vorhanden sind
    if ( !pFoundData )
    {
        nSubstFlags1 &= ~FONT_SUBSTITUTE_ALWAYS;
        nSubstFlags2 &= ~FONT_SUBSTITUTE_ALWAYS;
        nIndex = 0;
        do
        {
            if ( nToken > 1 )
            {
                aSearchName = rName.GetToken( 0, ';', nIndex );
                ImplGetEnglishSearchName( aSearchName );
            }
            else
                nIndex = STRING_NOTFOUND;
            ImplFontSubstitute( aSearchName, nSubstFlags1, nSubstFlags2 );
            pFoundData = pFontList->ImplFind( aSearchName );
            if ( pFoundData )
                break;
        }
        while ( nIndex != STRING_NOTFOUND );
    }

    ULONG nFontCount = pFontList->Count();
    if ( !pFoundData && nFontCount )
    {
        // Wenn kein Font mit dem entsprechenden Namen existiert, versuchen
        // wir ueber den Namen und die Attribute einen passenden Font zu
        // finden - wir nehmen dazu das erste Token
        if ( nToken > 1 )
        {
            aSearchName = rName.GetToken( 0, ';', nIndex );
            ImplGetEnglishSearchName( aSearchName );
        }

        const ImplFontNameAttr* pFontAttr;
        String                  aSearchShortName;
        String                  aSearchFamilyName;
        ULONG                   nSearchType = 0;
        FontWeight              eSearchWeight = eWeight;
        FontWidth               eSearchWidth = WIDTH_DONTKNOW;
        ImplGetMapName( aSearchName, aSearchShortName, aSearchFamilyName,
                        eSearchWeight, eSearchWidth, nSearchType );

        // Search, if ShortName is available
        if ( aSearchShortName != aSearchName )
            pFoundData = pFontList->ImplFind( aSearchShortName );

        if ( !pFoundData && aSearchName.Len() )
        {
            pFontAttr = ImplGetFontNameAttr( aSearchName );
            if ( !pFontAttr && (aSearchShortName != aSearchName) )
                pFontAttr = ImplGetFontNameAttr( aSearchShortName );

            // Try Substitution
            if ( pFontAttr && pFontAttr->mpSubstitution1 )
            {
                pFoundData = pFontList->ImplFindFontFromToken( pFontAttr->mpSubstitution1 );
                if ( !pFoundData && pFontAttr->mpSubstitution2 )
                {
                    pFoundData = pFontList->ImplFindFontFromToken( pFontAttr->mpSubstitution2 );
                    if ( !pFoundData && pFontAttr->mpSubstitution3 )
                        pFoundData = pFontList->ImplFindFontFromToken( pFontAttr->mpSubstitution3 );
                }
            }
        }

        if ( !pFoundData )
        {
            // wir versuchen zuerst einen Font zu finden, der ueber den Namen
            // matched
            ULONG nTestMatch;
            ULONG nBestMatch = 0;
            for ( ULONG i = 0; i < nFontCount; i++ )
            {
                ImplDevFontListData* pData = pFontList->Get( i );

                nTestMatch = 0;

#if 0
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

                // skalierbare Schriften haben schon einen echten Vorteil
                // gegenueber nicht skalierbaren Schriften
                if ( pData->mbScalable )
                    nTestMatch += 500000000;

                // Wir gehen davon aus, wenn der Name groesstenteils matcht,
                // das er schon zur richtigen Familie gehoert

                // Beim matchen ignorieren wir alle Sonderzeichen
                ULONG nTestMatch = ImplStrMatch( aNoSymbolName, pData->maMatchName2 );
                if ( nTestMatch >= nBestMatch )
                {
                    // Match nur erlaubt, wenn auch die Attribute uebereinstimmen
                    BOOL    bTestFamily = pData->meMatchFamily != FAMILY_DONTKNOW;
                    BOOL    bTestSymbol = pData->mbSymbol;
                    BOOL    bTestFixed  = pData->meMatchPitch == PITCH_FIXED;
                    if ( (bFixed == bTestFixed) && (bSymbol == bTestSymbol) &&
                         (!bFamily || !bTestFamily || (eSearchFamily == pData->meMatchFamily)) )
                    {
                        xub_StrLen nAttrMatch = 0;
                        // Die Anzahl der uebereinstimmenden Attribute zaehlen
                        const char** pTypeList = aImplTypeList;
                        while ( *pTypeList )
                        {
                            if ( (aNoSymbolName.Search( *pTypeList ) != STRING_NOTFOUND) &&
                                 (pData->maMatchName2.Search( *pTypeList ) != STRING_NOTFOUND) )
                                nAttrMatch++;
                            pTypeList++;
                        }

                        // Wenn beide Matches gleich gut sind,
                        // entscheiden die uebereinstimmenden Attribute
                        if ( nBestMatch == nTestMatch )
                        {
                            if ( (nAttrMatch > nBestAttrMatch) ||
                                 ((nAttrMatch == nBestAttrMatch) &&
                                  (pData->maMatchName2.Len() < nBestStrLen)) )
                            {
                                pFoundData = pData;
                                nBestMatch = nTestMatch;
                                nBestAttrMatch = nAttrMatch;
                                nBestStrLen = pData->maMatchName2.Len();
                            }
                        }
                        else
                        {
                            pFoundData = pData;
                            nBestMatch = nTestMatch;
                            nBestAttrMatch = nAttrMatch;
                            nBestStrLen = pData->maMatchName2.Len();
                        }
                    }
                }



    // Test, if Fontname includes CJK characters --> In this case we
    // mention that it is a CJK font
    const sal_Unicode* pStr = rFamilyName;
    while ( *pStr )
    {
        if ( ((*pStr >= 0x3000) && (*pStr <= 0xD7AF)) ||
             ((*pStr >= 0xFF00) && (*pStr <= 0xFFEE)) )
        {
            rType |= IMPL_FONT_ATTR_CJK;
            break;
        }

        pStr++;
    }

#endif

                if ( nTestMatch > nBestMatch )
                {
                    pFoundData = pData;
                    nBestMatch = nTestMatch;
                }
            }

            if ( !pFoundData )
            {
                // Try to use a Standard Font depend from the
                // attributes


                // Try to use a Standard Unicode or a Standard Font to get
                // as max as possible characters
                if ( !pFoundData )
                {
                    pFoundData = pFontList->ImplFindFontFromToken( aImplSubsSansUnicode );
                    if ( !pFoundData )
                    {
                        pFoundData = pFontList->ImplFindFontFromToken( aImplSubsSerif );
                        if ( !pFoundData )
                        {
                            pFoundData = pFontList->ImplFindFontFromToken( aImplSubsSans );
                            if ( !pFoundData )
                                pFoundData = pFontList->ImplFindFontFromToken( aImplSubsFixed );
                        }
                    }
                }

                // If nothing helps, we use the first one
                if ( !pFoundData )
                    pFoundData = pFontList->Get( 0 );
            }
        }
    }

    // We have found a useable Font, than we look with Font mapps at best
    // with the wished attributes
    if ( pFoundData )
    {
        ULONG           nBestMatch = 0;         // Der groessere Wert ist der bessere
        ULONG           nBestHeightMatch = 0;   // Der kleinere Wert ist der bessere
        ULONG           nBestWidthMatch = 0;    // Der kleinere Wert ist der bessere
        ULONG           nMatch;
        ULONG           nHeightMatch;
        ULONG           nWidthMatch;
        ImplFontData*   pCurFontData;

        // FontName+StyleName should map to FamilyName+StyleName
        const xub_Unicode* pCompareStyleName = NULL;
        if ( (aSearchName.Len() > pFoundData->maSearchName.Len()) &&
             aSearchName.Equals( pFoundData->maSearchName, 0, pFoundData->maSearchName.Len() ) )
            pCompareStyleName = aSearchName.GetBuffer()+pFoundData->maSearchName.Len()+1;

        pCurFontData = pFoundData->mpFirst;
        while ( pCurFontData )
        {
            nMatch = 0;
            nHeightMatch = 0;
            nWidthMatch = 0;

            if ( pCompareStyleName &&
                 pCurFontData->maStyleName.EqualsIgnoreCaseAscii( pCompareStyleName ) )
                nMatch += 120000;

            if ( (ePitch != PITCH_DONTKNOW) && (ePitch == pCurFontData->mePitch) )
                nMatch += 60000;

            if ( (eFamily != FAMILY_DONTKNOW) && (eFamily == pCurFontData->meFamily) )
                nMatch += 30000;

            // Normale Schriftbreiten bevorzugen, da wir noch keine Daten
            // von den Applikationen bekommen
            if ( pCurFontData->meWidthType == WIDTH_NORMAL )
                nMatch += 15000;

            if ( eWeight != WEIGHT_DONTKNOW )
            {
                USHORT nReqWeight;
                USHORT nGivenWeight;
                USHORT nWeightDiff;
                // schmale Fonts werden bei nicht Bold vor fetten
                // Fonts bevorzugt
                if ( eWeight > WEIGHT_MEDIUM )
                    nReqWeight = ((USHORT)eWeight)+100;
                else
                    nReqWeight = (USHORT)eWeight;
                if ( pCurFontData->meWeight > WEIGHT_MEDIUM )
                    nGivenWeight = ((USHORT)pCurFontData->meWeight)+100;
                else
                    nGivenWeight = (USHORT)pCurFontData->meWeight;
                if ( nReqWeight > nGivenWeight )
                    nWeightDiff = nReqWeight-nGivenWeight;
                else
                    nWeightDiff = nGivenWeight-nReqWeight;

                if ( nWeightDiff == 0 )
                    nMatch += 1000;
                else if ( nWeightDiff == 1 )
                    nMatch += 700;
                else if ( nWeightDiff < 50 )
                    nMatch += 200;
            }
            if ( eItalic == ITALIC_NONE )
            {
                if ( pCurFontData->meItalic == ITALIC_NONE )
                    nMatch += 900;
            }
            else
            {
                if ( eItalic == pCurFontData->meItalic )
                    nMatch += 900;
                else if ( pCurFontData->meItalic != ITALIC_NONE )
                    nMatch += 600;
            }

            if ( pCurFontData->mbDevice )
                nMatch += 40;
            if ( pCurFontData->meType == TYPE_SCALABLE )
            {
                if ( nOrientation )
                    nMatch += 80;
                else
                {
                    if ( nWidth )
                        nMatch += 25;
                    else
                        nMatch += 5;
                }
            }
            else
            {
                if ( nHeight == pCurFontData->mnHeight )
                {
                    nMatch += 20;
                    if ( nWidth == pCurFontData->mnWidth )
                        nMatch += 10;
                }
                else
                {
                    // Dann kommt die Size-Abweichung in die
                    // Bewertung rein. Hier bevorzugen wir
                    // nach Moeglichkeit den kleineren Font
                    if ( nHeight < pCurFontData->mnHeight )
                        nHeightMatch += pCurFontData->mnHeight-nHeight;
                    else
                        nHeightMatch += (nHeight-pCurFontData->mnHeight-nHeight)+10000;
                    if ( nWidth && pCurFontData->mnWidth && (nWidth != pCurFontData->mnWidth) )
                    {
                        if ( nWidth < pCurFontData->mnWidth )
                            nWidthMatch += pCurFontData->mnWidth-nWidth;
                        else
                            nWidthMatch += nWidth-pCurFontData->mnWidth-nWidth;
                    }
                }
            }

            if ( nMatch > nBestMatch )
            {
                pFontData           = pCurFontData;
                nBestMatch          = nMatch;
                nBestHeightMatch    = nHeightMatch;
                nBestWidthMatch     = nWidthMatch;
            }
            else if ( nMatch == nBestMatch )
            {
                // Wenn 2 gleichwertig sind, kommt die Size-Bewertung
                // Hier gewinnt der jenige, der die niedrigere Abweichung
                // in der Groesse hat (also den kleinsten Match)
                if ( nHeightMatch < nBestHeightMatch )
                {
                    pFontData           = pCurFontData;
                    nBestHeightMatch    = nHeightMatch;
                    nBestWidthMatch     = nWidthMatch;
                }
                else if ( nHeightMatch == nBestHeightMatch )
                {
                    if ( nWidthMatch < nBestWidthMatch )
                    {
                        pFontData       = pCurFontData;
                        nBestWidthMatch = nWidthMatch;
                    }
                }
            }

            pCurFontData = pCurFontData->mpNext;
        }
    }

    // Daten initialisieren und in die Liste aufnehmen
    pEntry                          = new ImplFontEntry;
    pEntry->mbInit                  = FALSE;
    pEntry->mnRefCount              = 1;
    pEntry->mpNext                  = mpFirstEntry;
    pEntry->mnWidthInit             = 0;
    pEntry->mnWidthAryCount         = 0;
    pEntry->mnWidthArySize          = 0;
    pEntry->mpWidthAry              = NULL;
    pEntry->mpKernPairs             = NULL;
    pEntry->mnOwnOrientation        = 0;
    pEntry->mnOrientation           = 0;
    mpFirstEntry                    = pEntry;

    // Font-Selection-Daten setzen
    ImplFontSelectData* pFontSelData = &(pEntry->maFontSelData);
    pFontSelData->mpFontData        = pFontData;
    pFontSelData->mpSysSelData      = NULL;
    pFontSelData->maName            = rName;
    pFontSelData->maStyleName       = rStyleName;
    pFontSelData->mnWidth           = nWidth;
    pFontSelData->mnHeight          = nHeight;
    pFontSelData->meFamily          = eFamily;
    pFontSelData->meCharSet         = eCharSet;
    pFontSelData->meWidthType       = WIDTH_DONTKNOW;
    pFontSelData->meWeight          = eWeight;
    pFontSelData->meItalic          = eItalic;
    pFontSelData->mePitch           = ePitch;
    pFontSelData->mnOrientation     = nOrientation;
    pFontSelData->mbVertical        = bVertical;

    return pEntry;
}

// -----------------------------------------------------------------------

void ImplFontCache::Release( ImplFontEntry* pEntry )
{
    pEntry->mnRefCount--;

    if ( !pEntry->mnRefCount )
    {
        if ( mnRef0Count >= MAXFONT_CACHE )
        {
            // Letzten Entry mit RefCount 0 loeschen
            ImplFontEntry* pPrevDelEntry = mpFirstEntry;
            ImplFontEntry* pDelEntry = pPrevDelEntry->mpNext;
            USHORT nCurRef0Count = !(pPrevDelEntry->mnRefCount);
            while ( pDelEntry )
            {
                if ( !pDelEntry->mnRefCount )
                    nCurRef0Count++;

                if ( nCurRef0Count >= MAXFONT_CACHE )
                {
                    pPrevDelEntry->mpNext = pDelEntry->mpNext;
                    delete pDelEntry;
                    break;
                }

                pPrevDelEntry = pDelEntry;
                pDelEntry = pDelEntry->mpNext;
            }
        }
        else
            mnRef0Count++;
    }
}

// -----------------------------------------------------------------------

void ImplFontCache::Clear()
{
    // Alle Eintraege loeschen
    ImplFontEntry* pTemp;
    ImplFontEntry* pEntry = mpFirstEntry;
    while ( pEntry )
    {
        DBG_ASSERT( !pEntry->mnRefCount, "ImplFontCache::Clear() - Font in use" );
        pTemp = pEntry->mpNext;
        delete pEntry;
        pEntry = pTemp;
    }

    mpFirstEntry    = NULL;
    mnRef0Count     = 0;
}

// =======================================================================

class ImplTextLineInfo
{
private:
    long        mnWidth;
    xub_StrLen  mnIndex;
    xub_StrLen  mnLen;

public:
                ImplTextLineInfo( long nWidth, xub_StrLen nIndex, xub_StrLen nLen )
                {
                    mnWidth = nWidth;
                    mnIndex = nIndex;
                    mnLen   = nLen;
                }

    long        GetWidth() const { return mnWidth; }
    xub_StrLen  GetIndex() const { return mnIndex; }
    xub_StrLen  GetLen() const { return mnLen; }
};

#define MULTITEXTLINEINFO_RESIZE    16
typedef ImplTextLineInfo* PImplTextLineInfo;

class ImplMultiTextLineInfo
{
private:
    PImplTextLineInfo*  mpLines;
    xub_StrLen          mnLines;
    xub_StrLen          mnSize;

public:
                        ImplMultiTextLineInfo();
                        ~ImplMultiTextLineInfo();

    void                AddLine( ImplTextLineInfo* pLine );
    void                Clear();

    ImplTextLineInfo*   GetLine( USHORT nLine ) const
                            { return mpLines[nLine]; }
    xub_StrLen          Count() const { return mnLines; }

private:
                            ImplMultiTextLineInfo( const ImplMultiTextLineInfo& );
    ImplMultiTextLineInfo&  operator=( const ImplMultiTextLineInfo& );
};

ImplMultiTextLineInfo::ImplMultiTextLineInfo()
{
    mpLines         = new PImplTextLineInfo[MULTITEXTLINEINFO_RESIZE];
    mnLines         = 0;
    mnSize          = MULTITEXTLINEINFO_RESIZE;
}

ImplMultiTextLineInfo::~ImplMultiTextLineInfo()
{
    for ( xub_StrLen i = 0; i < mnLines; i++ )
        delete mpLines[i];
    delete mpLines;
}

void ImplMultiTextLineInfo::AddLine( ImplTextLineInfo* pLine )
{
    if ( mnSize == mnLines )
    {
        mnSize += MULTITEXTLINEINFO_RESIZE;
        PImplTextLineInfo* pNewLines = new PImplTextLineInfo[mnSize];
        memcpy( pNewLines, mpLines, mnLines*sizeof(PImplTextLineInfo) );
        mpLines = pNewLines;
    }

    mpLines[mnLines] = pLine;
    mnLines++;
}

void ImplMultiTextLineInfo::Clear()
{
    for ( xub_StrLen i = 0; i < mnLines; i++ )
        delete mpLines[i];
    mnLines = 0;
}

// =======================================================================

static FontEmphasisMark ImplGetEmphasisMarkStyle( const Font& rFont )
{
    FontEmphasisMark nEmphasisMark = rFont.GetEmphasisMark();

    // If no Position is set, then calculate the default position, which
    // depends on the language
    if ( !(nEmphasisMark & (EMPHASISMARK_POS_ABOVE | EMPHASISMARK_POS_BELOW)) )
    {
        LanguageType eLang = rFont.GetLanguage();
        // In Chinese Simplified the EmphasisMarks are below/left
        if ( (eLang == LANGUAGE_CHINESE_SIMPLIFIED) ||
             (eLang == LANGUAGE_CHINESE_SINGAPORE) )
            nEmphasisMark |= EMPHASISMARK_POS_BELOW;
        else
        {
            eLang = rFont.GetCJKContextLanguage();
            // In Chinese Simplified the EmphasisMarks are below/left
            if ( (eLang == LANGUAGE_CHINESE_SIMPLIFIED) ||
                 (eLang == LANGUAGE_CHINESE_SINGAPORE) )
                nEmphasisMark |= EMPHASISMARK_POS_BELOW;
            else
                nEmphasisMark |= EMPHASISMARK_POS_ABOVE;
        }
    }

    return nEmphasisMark;
}

// -----------------------------------------------------------------------

static BOOL ImplIsUnderlineAbove( const Font& rFont )
{
    if ( !rFont.IsVertical() )
        return FALSE;

    LanguageType eLang = rFont.GetLanguage();
    // In all Chinese Languages the underline is left
    if ( (eLang == LANGUAGE_CHINESE)                ||
         (eLang == LANGUAGE_CHINESE_TRADITIONAL)    ||
         (eLang == LANGUAGE_CHINESE_SIMPLIFIED)     ||
         (eLang == LANGUAGE_CHINESE_HONGKONG)       ||
         (eLang == LANGUAGE_CHINESE_SINGAPORE)      ||
         (eLang == LANGUAGE_CHINESE_MACAU) )
        return FALSE;
    else
    {
        eLang = rFont.GetCJKContextLanguage();
        // In all Chinese Languages the underline is left
        if ( (eLang == LANGUAGE_CHINESE)                ||
             (eLang == LANGUAGE_CHINESE_TRADITIONAL)    ||
             (eLang == LANGUAGE_CHINESE_SIMPLIFIED)     ||
             (eLang == LANGUAGE_CHINESE_HONGKONG)       ||
             (eLang == LANGUAGE_CHINESE_SINGAPORE)      ||
             (eLang == LANGUAGE_CHINESE_MACAU) )
            return FALSE;
    }

    return TRUE;
}

// =======================================================================

void OutputDevice::ImplInitFont()
{
    DBG_TESTSOLARMUTEX();

    if ( mbInitFont )
    {
        // Set Antialisied Mode
        BOOL bNonAntialiased = (GetAntialiasing() & ANTIALIASING_DISABLE_TEXT) != 0;
        mpFontEntry->maFontSelData.mbNonAntialiased = bNonAntialiased;

        // Select Font
        mpFontEntry->mnSetFontFlags = mpGraphics->SetFont( &(mpFontEntry->maFontSelData) );
        mbInitFont = FALSE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitTextColor()
{
    DBG_TESTSOLARMUTEX();

    if ( mbInitTextColor )
    {
#ifndef REMOTE_APPSERVER
        mpGraphics->SetTextColor( ImplColorToSal( GetTextColor() ) );
#else
        mpGraphics->SetTextColor( GetTextColor() );
#endif
        mbInitTextColor = FALSE;
    }
}

// -----------------------------------------------------------------------

int OutputDevice::ImplNewFont()
{
    DBG_TESTSOLARMUTEX();

    if ( !mbNewFont )
        return TRUE;

    mbNewFont = FALSE;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return FALSE;
    }
    SalGraphics* pGraphics = mpGraphics;
#else
    // Da wegen Clipping hier NULL zurueckkommen kann, koennen wir nicht
    // den Rueckgabewert nehmen
    ImplGetServerGraphics();
    ImplServerGraphics* pGraphics = mpGraphics;
#endif

    // Groesse umrechnen
    Size aSize = ImplLogicToDevicePixel( maFont.GetSize() );
    if ( !aSize.Height() )
    {
        // Nur dann Defaultgroesse setzen, wenn Fonthoehe auch in logischen
        // Koordinaaten 0 ist
        if ( maFont.GetSize().Height() )
            aSize.Height() = 1;
        else
            aSize.Height() = (12*mnDPIY)/72;
    }

    // Neuen Font besorgen
    ImplFontEntry* pOldEntry = mpFontEntry;
    mpFontEntry = mpFontCache->Get( mpFontList, maFont, aSize );
    ImplFontEntry* pFontEntry = mpFontEntry;

    // Feststellen, ob Font neu selektiert werden muss
    if ( pFontEntry != pOldEntry )
        mbInitFont = TRUE;

    // these two may be filled in remote version
    ImplKernPairData* pKernPairs = NULL;
    long nKernPairs = 0;
    // Wenn Font nicht initialisiert ist, dann sofort selektieren
    if ( !pFontEntry->mbInit )
    {
        ImplInitFont();

        // und die Font-Daten besorgen
        if ( pGraphics )
        {
            pFontEntry->mbInit = TRUE;

            pFontEntry->maMetric.mnWidth        = pFontEntry->maFontSelData.mnWidth;
            pFontEntry->maMetric.meFamily       = pFontEntry->maFontSelData.meFamily;
            pFontEntry->maMetric.meCharSet      = pFontEntry->maFontSelData.meCharSet;
            pFontEntry->maMetric.meWeight       = pFontEntry->maFontSelData.meWeight;
            pFontEntry->maMetric.meItalic       = pFontEntry->maFontSelData.meItalic;
            pFontEntry->maMetric.mePitch        = pFontEntry->maFontSelData.mePitch;
            pFontEntry->maMetric.mnOrientation  = pFontEntry->maFontSelData.mnOrientation;
            if ( pFontEntry->maFontSelData.mpFontData )
            {
                pFontEntry->maMetric.meType     = pFontEntry->maFontSelData.mpFontData->meType;
                pFontEntry->maMetric.maName     = pFontEntry->maFontSelData.mpFontData->maName;
                pFontEntry->maMetric.maStyleName= pFontEntry->maFontSelData.mpFontData->maStyleName;
                pFontEntry->maMetric.mbDevice   = pFontEntry->maFontSelData.mpFontData->mbDevice;
            }
            else
            {
                pFontEntry->maMetric.meType     = TYPE_DONTKNOW;
                pFontEntry->maMetric.maName     = pFontEntry->maFontSelData.maName.GetToken( 0 );
                pFontEntry->maMetric.maStyleName= pFontEntry->maFontSelData.maStyleName;
                pFontEntry->maMetric.mbDevice   = FALSE;
            }
            pFontEntry->maMetric.mnUnderlineSize            = 0;
            pFontEntry->maMetric.mnUnderlineOffset          = 0;
            pFontEntry->maMetric.mnBUnderlineSize           = 0;
            pFontEntry->maMetric.mnBUnderlineOffset         = 0;
            pFontEntry->maMetric.mnDUnderlineSize           = 0;
            pFontEntry->maMetric.mnDUnderlineOffset1        = 0;
            pFontEntry->maMetric.mnDUnderlineOffset2        = 0;
            pFontEntry->maMetric.mnWUnderlineSize           = 0;
            pFontEntry->maMetric.mnWUnderlineOffset         = 0;
            pFontEntry->maMetric.mnAboveUnderlineSize       = 0;
            pFontEntry->maMetric.mnAboveUnderlineOffset     = 0;
            pFontEntry->maMetric.mnAboveBUnderlineSize      = 0;
            pFontEntry->maMetric.mnAboveBUnderlineOffset    = 0;
            pFontEntry->maMetric.mnAboveDUnderlineSize      = 0;
            pFontEntry->maMetric.mnAboveDUnderlineOffset1   = 0;
            pFontEntry->maMetric.mnAboveDUnderlineOffset2   = 0;
            pFontEntry->maMetric.mnAboveWUnderlineSize      = 0;
            pFontEntry->maMetric.mnAboveWUnderlineOffset    = 0;
            pFontEntry->maMetric.mnStrikeoutSize            = 0;
            pFontEntry->maMetric.mnStrikeoutOffset          = 0;
            pFontEntry->maMetric.mnBStrikeoutSize           = 0;
            pFontEntry->maMetric.mnBStrikeoutOffset         = 0;
            pFontEntry->maMetric.mnDStrikeoutSize           = 0;
            pFontEntry->maMetric.mnDStrikeoutOffset1        = 0;
            pFontEntry->maMetric.mnDStrikeoutOffset2        = 0;
#ifndef REMOTE_APPSERVER
            pGraphics->GetFontMetric( &(pFontEntry->maMetric) );
            pFontEntry->mnWidthFactor = IMPL_FACTOR_NOTINIT;
#else
            long nFactor = 0;
            pGraphics->GetFontMetric(
                pFontEntry->maMetric,
                nFactor, IMPL_CACHE_A1_FIRST, IMPL_CACHE_A1_LAST,
                pFontEntry->maWidthAry+IMPL_CACHE_A1_INDEX;
                (maFont.GetKerning() & KERNING_FONTSPECIFIC) != 0,
                &pKernPairs, nKernPairs );
            pFontEntry->mnWidthFactor = nFactor;
            if ( pFontEntry->mnWidthFactor )
                pFontEntry->mnWidthInit |= IMPL_CACHE_A1_BIT;
            else
                pFontEntry->mnWidthFactor = IMPL_FACTOR_NOTINIT;
#endif

            pFontEntry->mbFixedFont     = pFontEntry->maMetric.mePitch == PITCH_FIXED;
            pFontEntry->mnLineHeight    = pFontEntry->maMetric.mnAscent+pFontEntry->maMetric.mnDescent;
            pFontEntry->mbInitKernPairs = FALSE;
            pFontEntry->mnKernPairs     = nKernPairs;

            if ( pFontEntry->maFontSelData.mnOrientation && !pFontEntry->maMetric.mnOrientation &&
                (meOutDevType != OUTDEV_PRINTER) )
            {
                pFontEntry->mnOwnOrientation = pFontEntry->maFontSelData.mnOrientation;
                pFontEntry->mnOrientation = pFontEntry->mnOwnOrientation;
            }
            else
                pFontEntry->mnOrientation = pFontEntry->maMetric.mnOrientation;
        }
    }

    // Wenn Kerning gewuenscht ist, die Kerning-Werte ermitteln
    if ( maFont.GetKerning() & KERNING_FONTSPECIFIC )
    {
        ImplInitKerningPairs( pKernPairs, nKernPairs );
        mbKerning = (pFontEntry->mnKernPairs) != 0;
    }
    else
        mbKerning = FALSE;
    if ( maFont.GetKerning() & KERNING_ASIAN )
        mbKerning = TRUE;

    // Calculate the EmphasisArea
    mnEmphasisAscent = 0;
    mnEmphasisDescent = 0;
    if ( maFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
    {
        FontEmphasisMark    nEmphasisMark = ImplGetEmphasisMarkStyle( maFont );
        long                nEmphasisHeight = (pFontEntry->mnLineHeight*250)/1000;
        if ( nEmphasisHeight < 1 )
            nEmphasisHeight = 1;
        if ( nEmphasisMark & EMPHASISMARK_POS_BELOW )
            mnEmphasisDescent = nEmphasisHeight;
        else
            mnEmphasisAscent = nEmphasisHeight;
    }

    // Je nach TextAlign den TextOffset berechnen
    TextAlign eAlign = maFont.GetAlign();
    if ( eAlign == ALIGN_BASELINE )
    {
        mnTextOffX = 0;
        mnTextOffY = 0;
    }
    else if ( eAlign == ALIGN_TOP )
    {
        mnTextOffX = 0;
        mnTextOffY = pFontEntry->maMetric.mnAscent+mnEmphasisAscent;
        if ( pFontEntry->mnOrientation )
            ImplRotatePos( 0, 0, mnTextOffX, mnTextOffY, pFontEntry->mnOrientation );
    }
    else // eAlign == ALIGN_BOTTOM
    {
        mnTextOffX = 0;
        mnTextOffY = -pFontEntry->maMetric.mnDescent+mnEmphasisDescent;
        if ( pFontEntry->mnOrientation )
            ImplRotatePos( 0, 0, mnTextOffX, mnTextOffY, pFontEntry->mnOrientation );
    }

    mbTextLines     = ((maFont.GetUnderline() != UNDERLINE_NONE) && (maFont.GetUnderline() != UNDERLINE_DONTKNOW)) ||
                      ((maFont.GetStrikeout() != STRIKEOUT_NONE) && (maFont.GetStrikeout() != STRIKEOUT_DONTKNOW));
    mbTextSpecial   = maFont.IsShadow() || maFont.IsOutline() ||
                      (maFont.GetRelief() != RELIEF_NONE);

    if ( pOldEntry )
        mpFontCache->Release( pOldEntry );

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::ImplGetCharWidths( sal_Unicode c1, sal_Unicode c2,
                                      long* pAry ) const
{
    // Um die Zeichenbreite zu ermitteln, brauchen wir einen Graphics und der
    // Font muss natuerlich auch selektiert sein
#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !((OutputDevice*)this)->ImplGetGraphics() )
            return FALSE;
    }
#else
    // Da wegen Clipping hier NULL zurueckkommen kann, koennen wir nicht
    // den Rueckgabewert nehmen
    ((OutputDevice*)this)->ImplGetServerGraphics();
#endif

    if ( mbNewFont )
        ((OutputDevice*)this)->ImplNewFont();
    if ( mbInitFont )
        ((OutputDevice*)this)->ImplInitFont();

#ifndef REMOTE_APPSERVER
    long nWidthFactor = mpGraphics->GetCharWidth( c1, c2, pAry );
    if ( !nWidthFactor )
        return FALSE;
    if ( mpFontEntry->mnWidthFactor == IMPL_FACTOR_NOTINIT )
        mpFontEntry->mnWidthFactor = nWidthFactor;
    DBG_ASSERT( (nWidthFactor == mpFontEntry->mnWidthFactor),
                "OutputDevice::ImplGetCharWidths() - other WidthFactor" );
#else
    mpGraphics->GetCharWidth( c1, c2, pAry );
#endif

    return TRUE;
}

// -----------------------------------------------------------------------

long OutputDevice::ImplGetCharWidth( sal_Unicode c ) const
{
    ImplFontEntry*                  pFontEntry = mpFontEntry;
    USHORT                          nChar = (USHORT)c;
    if ( nChar < IMPL_WIDTH_CACHE_MAX )
    {
        ULONG   nTestBit = 0;
        USHORT  nFirst;
        USHORT  nLast;
        USHORT  nIndex;

        // Test Standard-Areas (Latin, Extended Latin, Greek and Cyrillic)
        if ( (nChar >= IMPL_CACHE_A1_FIRST) && (nChar <= IMPL_CACHE_A1_LAST) )
        {
            nTestBit    = IMPL_CACHE_A1_BIT;
            nFirst      = IMPL_CACHE_A1_FIRST;
            nLast       = IMPL_CACHE_A1_LAST;
            nIndex      = IMPL_CACHE_A1_INDEX;
        }
        else if ( (nChar >= IMPL_CACHE_A2_FIRST) && (nChar <= IMPL_CACHE_A2_LAST) )
        {
            nTestBit    = IMPL_CACHE_A2_BIT;
            nFirst      = IMPL_CACHE_A2_FIRST;
            nLast       = IMPL_CACHE_A2_LAST;
            nIndex      = IMPL_CACHE_A2_INDEX;
        }
        else if ( (nChar >= IMPL_CACHE_A3_FIRST) && (nChar <= IMPL_CACHE_A3_LAST) )
        {
            nTestBit    = IMPL_CACHE_A3_BIT;
            nFirst      = IMPL_CACHE_A3_FIRST;
            nLast       = IMPL_CACHE_A3_LAST;
            nIndex      = IMPL_CACHE_A3_INDEX;
        }
        else if ( (nChar >= IMPL_CACHE_A4_FIRST) && (nChar <= IMPL_CACHE_A4_LAST) )
        {
            nTestBit    = IMPL_CACHE_A4_BIT;
            nFirst      = IMPL_CACHE_A4_FIRST;
            nLast       = IMPL_CACHE_A4_LAST;
            nIndex      = IMPL_CACHE_A4_INDEX;
        }

        if ( nTestBit )
        {
            // Characters not queried yet
            if ( !(pFontEntry->mnWidthInit & nTestBit) )
            {
                if ( !ImplGetCharWidths( nFirst, nLast,
                                         &(pFontEntry->maWidthAry[nIndex]) ) )
                    return 0;
                pFontEntry->mnWidthInit |= nTestBit;
            }

            nIndex += nChar-nFirst;
            DBG_ASSERT( nIndex < IMPL_WIDTH_CACHE_COUNT,
                        "OutputDevice::ImplGetCharWidth() - nIndex >= IMPL_WIDTH_CACHE_COUNT" );
            return pFontEntry->maWidthAry[nIndex];
        }
    }

    ImplWidthInfoData*  pInfo;
    ImplWidthInfoData*  pWidthAry =  pFontEntry->mpWidthAry;
    USHORT              nWidthCount = pFontEntry->mnWidthAryCount;
    USHORT              nInsIndex;

    if ( nWidthCount )
    {
        USHORT  nLow;
        USHORT  nHigh;
        USHORT  nMid;
        USHORT  nCompareChar;

        nLow  = 0;
        nHigh = nWidthCount-1;
        do
        {
            nMid = (nLow+nHigh)/2;
            pInfo = pWidthAry+nMid;
            nCompareChar = pInfo->mnChar;
            if ( nChar < nCompareChar )
            {
                if ( !nMid )
                    break;
                nHigh = nMid-1;
            }
            else
            {
                if ( nChar > nCompareChar )
                    nLow = nMid+1;
                else
                    return pInfo->mnWidth;
            }
        }
        while ( nLow <= nHigh );

        if ( nChar > nCompareChar )
            nInsIndex = nMid+1;
        else
            nInsIndex = nMid;
    }
    else
    {
        pFontEntry->mnWidthArySize = IMPL_WIDTH_ARY_INIT;
        pFontEntry->mpWidthAry = new ImplWidthInfoData[pFontEntry->mnWidthArySize];
        pWidthAry = pFontEntry->mpWidthAry;
        nInsIndex = 0;
    }

    if ( nWidthCount == pFontEntry->mnWidthArySize )
    {
        USHORT nOldSize = pFontEntry->mnWidthArySize;
        pFontEntry->mnWidthArySize += IMPL_WIDTH_ARY_RESIZE;
        pFontEntry->mpWidthAry = new ImplWidthInfoData[pFontEntry->mnWidthArySize];
        memcpy( pFontEntry->mpWidthAry, pWidthAry, nOldSize*sizeof(ImplWidthInfoData) );
        delete pWidthAry;
        pWidthAry = pFontEntry->mpWidthAry;
    }

    long nWidth;
    if ( !ImplGetCharWidths( nChar, nChar, &nWidth ) )
        return 0;

    // Breite in Liste einfuegen und zurueckgeben
    pInfo = pWidthAry+nInsIndex;
    memmove( pInfo+1, pInfo, (nWidthCount-nInsIndex)*sizeof(ImplWidthInfoData) );
    pFontEntry->mnWidthAryCount++;
    pInfo->mnChar = nChar;
    pInfo->mnWidth = nWidth;
    return nWidth;
}

// -----------------------------------------------------------------------

static void ImplSortKernPairs( ImplKernPairData* pKernPairs, ULONG l, ULONG r )
{
    ULONG               i = l;
    ULONG               j = r;
    ImplKernPairData*   pComp = pKernPairs + ((l+r) >> 1);
    sal_uInt32          nComp = *((sal_uInt32*)pComp);

    do
    {
        while ( *((sal_uInt32*)(pKernPairs+i)) < nComp )
            i++;
        while ( nComp < *((sal_uInt32*)(pKernPairs+j)) )
            j--;
        if ( i <= j )
        {
            ImplKernPairData aTemp = *(pKernPairs+i);
            *(pKernPairs+i) = *(pKernPairs+j);
            *(pKernPairs+j) = aTemp;
            i++;
            j--;
        }
    }
    while ( i <= j );

    if ( l < j )
        ImplSortKernPairs( pKernPairs, l, j );
    if ( i < r )
        ImplSortKernPairs( pKernPairs, i, r );
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitKerningPairs( ImplKernPairData* pKernPairs, long nKernPairs )
{
    if ( mbNewFont )
    {
        if ( !ImplNewFont() )
            return;
    }

    ImplFontEntry* pFontEntry = mpFontEntry;
    if ( !pFontEntry->mbInitKernPairs )
    {
        if ( mbInitFont )
            ImplInitFont();
        pFontEntry->mbInitKernPairs = TRUE;
#ifndef REMOTE_APPSERVER
        pFontEntry->mnKernPairs = mpGraphics->GetKernPairs( 0, NULL );
        if ( pFontEntry->mnKernPairs )
        {
            pKernPairs = new ImplKernPairData[pFontEntry->mnKernPairs];
            memset( pKernPairs, 0, sizeof(ImplKernPairData)*pFontEntry->mnKernPairs );
            pFontEntry->mnKernPairs = mpGraphics->GetKernPairs( pFontEntry->mnKernPairs, pKernPairs );
            pFontEntry->mpKernPairs = pKernPairs;
        }
#else
        if ( !pKernPairs )
            nKernPairs = mpGraphics->GetKernPairs( &pKernPairs );
        if ( nKernPairs )
            pFontEntry->mpKernPairs = pKernPairs;
#endif

        // Sort Kerning Pairs
        if ( pFontEntry->mpKernPairs )
            ImplSortKernPairs( pFontEntry->mpKernPairs, 0, pFontEntry->mnKernPairs-1 );
    }
}

// -----------------------------------------------------------------------

long OutputDevice::ImplCalcKerning( const sal_Unicode* pStr, xub_StrLen nLen,
                                    long* pDXAry, xub_StrLen nAryLen ) const
{
    if ( !nLen )
        return 0;

    ImplFontEntry* pEntry           = mpFontEntry;
    ImplKernPairData* pKernPairs    = pEntry->mpKernPairs;
    ULONG nKernPairs                = pEntry->mnKernPairs;
    long nWidth                     = 0;
    xub_StrLen i;

    if ( (maFont.GetKerning() & KERNING_FONTSPECIFIC) && nKernPairs )
    {
#ifdef DBG_UTIL
        {
        ImplKernPairData    aTestPair;
#ifdef __LITTLEENDIAN
        sal_uInt32          nTestComp  = ((sal_uInt32)((USHORT)0xAABB) << 16) | (USHORT)0xCCDD;
#else
        sal_uInt32          nTestComp  = ((sal_uInt32)((USHORT)0xCCDD) << 16) | (USHORT)0xAABB;
#endif
        aTestPair.mnChar1 = 0xCCDD;
        aTestPair.mnChar2 = 0xAABB;
        DBG_ASSERT( nTestComp == *((ULONG*)&aTestPair), "Code doesn't work in this Version" );
        }
#endif

        const sal_Unicode* pTempStr = pStr;
        for ( i = 0; i < nLen-1; i++ )
        {
            USHORT nIndex = (USHORT)*pTempStr;
            pTempStr++;
#ifdef __LITTLEENDIAN
            sal_uInt32 nComp  = ((sal_uInt32)((USHORT)*pTempStr) << 16) | nIndex;
#else
            sal_uInt32 nComp  = ((sal_uInt32)nIndex << 16) | ((USHORT)*pTempStr);
#endif

            // Search for Kerning Pair
            for( long nLower = 0, nUpper = (long)nKernPairs-1; nLower <= nUpper; )
            {
                long nMid = (nUpper + nLower) >> 1;
                ImplKernPairData* pCurKernPair = pKernPairs+nMid;
                sal_uInt32 nCurComp = *((sal_uInt32*)pCurKernPair);
                if ( nComp == nCurComp )
                {
                    long nAmount = pCurKernPair->mnKern;
                    nWidth += nAmount;
                    if ( pDXAry )
                    {
                        for ( xub_StrLen n = i; n < nAryLen; n++ )
                            pDXAry[n] += nAmount;
                    }
                    break;
                }
                else if ( nComp < nCurComp )
                    nUpper = nMid-1;
                else /* ( nComp > nCurComp ) */
                    nLower = nMid+1;
            }
        }
    }

    if ( maFont.GetKerning() & KERNING_ASIAN )
    {
        const sal_Unicode* pTempStr = pStr;
        for ( i = 0; i < nLen-1; i++ )
        {
            USHORT nFirst = (USHORT)*pTempStr;
            pTempStr++;
            USHORT nNext = (USHORT)*pTempStr;
            if ( ((nFirst >= 0x3001) && (nFirst <= 0x301F)) ||
                 ((nNext >= 0x3001) && (nNext <= 0x301F)) )
            {
/*
                long nAmount = pKernPairs[j].mnKern;
                nWidth += nAmount;
                if ( pDXAry )
                {
                    for ( USHORT n = i; n < nAryLen; n++ )
                        pDXAry[n] += nAmount;
                }
*/
            }
        }
    }

    return nWidth;
}

// -----------------------------------------------------------------------

long OutputDevice::ImplGetTextWidth( const xub_Unicode* pStr, xub_StrLen nLen,
                                     const long* pDX )
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nWidth = 0;

    if ( nLen )
    {
        if ( pDX )
        {
            if ( nLen > 1 )
                nWidth += pDX[nLen-2];
            nWidth += ImplGetCharWidth( pStr[nLen-1] ) / mpFontEntry->mnWidthFactor;
        }
        else
        {
            // Also Fixed-Fonts are calculated char by char, because
            // not every Font or in every CJK Fonts all characters have
            // the same width
            const sal_Unicode*  pTempStr = pStr;
            xub_StrLen          nTempLen = nLen;
            while ( nTempLen )
            {
                nWidth += ImplGetCharWidth( *pTempStr );
                nTempLen--;
                pTempStr++;
            }
            nWidth /= mpFontEntry->mnWidthFactor;

            // Kerning beruecksichtigen
            if ( mbKerning )
                nWidth += ImplCalcKerning( pStr, nLen, NULL, 0 );
        }
    }

    return nWidth;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawTextRect( long nBaseX, long nBaseY,
                                     long nX, long nY, long nWidth, long nHeight )
{
    short nOrientation = mpFontEntry->mnOrientation;
    if ( nOrientation )
    {
        // Rotate rect without rounding problems for 90 degree rotations
        if ( !(nOrientation % 900) )
        {
            nX -= nBaseX;
            nY -= nBaseY;

            if ( nOrientation == 900 )
            {
                long nTemp = nX;
                nX = nY;
                nY = -nTemp;
                nTemp = nWidth;
                nWidth = nHeight;
                nHeight = nTemp;
                nY -= nHeight;
            }
            else if ( nOrientation == 1800 )
            {
                nX = -nX;
                nY = -nY;
                nX -= nWidth;
                nY -= nHeight;
            }
            else /* ( nOrientation == 2700 ) */
            {
                long nTemp = nX;
                nX = -nY;
                nY = nTemp;
                nTemp = nWidth;
                nWidth = nHeight;
                nHeight = nTemp;
                nX -= nWidth;
            }

            nX += nBaseX;
            nY += nBaseY;
        }
        else
        {
            // Da Polygone kleiner gezeichnet werden
            nHeight++;
            nWidth++;
            Rectangle aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
            Polygon   aPoly( aRect );
            aPoly.Rotate( Point( nBaseX, nBaseY ), mpFontEntry->mnOrientation );
#ifndef REMOTE_APPSERVER
            ImplDrawPolygon( aPoly );
#else
            mpGraphics->DrawPolygon( aPoly );
#endif
            return;
        }
    }

#ifndef REMOTE_APPSERVER
    mpGraphics->DrawRect( nX, nY, nWidth, nHeight );
#else
    Rectangle aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
    mpGraphics->DrawRect( aRect );
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawTextBackground( long nX, long nY,
                                           const xub_Unicode* pStr, xub_StrLen nLen,
                                           const long* pDXAry )
{
#ifndef REMOTE_APPSERVER
    if ( mbLineColor || mbInitLineColor )
    {
        mpGraphics->SetLineColor();
        mbInitLineColor = TRUE;
    }
    mpGraphics->SetFillColor( ImplColorToSal( GetTextFillColor() ) );
    mbInitFillColor = TRUE;

    ImplDrawTextRect( nX, nY, nX, nY-mpFontEntry->maMetric.mnAscent-mnEmphasisAscent,
                      ImplGetTextWidth( pStr, nLen, pDXAry ),
                      mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent );
#else
    Color aOldLineColor = GetLineColor();
    Color aOldFillColor = GetFillColor();
    SetLineColor();
    SetFillColor( GetTextFillColor() );
    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();
    ImplDrawTextRect( nX, nY, nX, nY-mpFontEntry->maMetric.mnAscent-mnEmphasisAscent,
                      ImplGetTextWidth( pStr, nLen, pDXAry ),
                      mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent );
    SetLineColor( aOldLineColor );
    SetFillColor( aOldFillColor );
#endif
}

// -----------------------------------------------------------------------

Rectangle OutputDevice::ImplGetTextBoundRect( long nX, long nY,
                                              const xub_Unicode* pStr, xub_StrLen nLen,
                                              const long* pDXAry )
{
    if ( !nLen )
        return Rectangle();

    if ( mbNewFont )
        ImplNewFont();

    if ( mbInitFont )
        ImplInitFont();

    long nBaseX = nX, nBaseY = nY;
    long nWidth = ImplGetTextWidth( pStr, nLen, pDXAry );
    long nHeight = mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent;

    nY -= mpFontEntry->maMetric.mnAscent+mnEmphasisAscent;

    if ( mpFontEntry->mnOrientation )
    {
        if ( !(mpFontEntry->mnOrientation % 900) )
        {
            long nX2 = nX+nWidth;
            long nY2 = nY+nHeight;
            ImplRotatePos( nBaseX, nBaseY, nX, nY, mpFontEntry->mnOrientation );
            ImplRotatePos( nBaseX, nBaseY, nX2, nY2, mpFontEntry->mnOrientation );
            nWidth = nX2-nX;
            nHeight = nY2-nY;
        }
        else
        {
            // Da Polygone kleiner gezeichnet werden
            nHeight++;
            nWidth++;
            Rectangle aRect( Point( nX, nY ), Size( nWidth, nHeight ) );
            Polygon   aPoly( aRect );
            aPoly.Rotate( Point( nBaseX, nBaseY ), mpFontEntry->mnOrientation );
            return aPoly.GetBoundRect();
        }
    }

    return Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitTextLineSize()
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nLineHeight;
    long            nLineHeight2;
    long            nBLineHeight;
    long            nBLineHeight2;
    long            n2LineHeight;
    long            n2LineDY;
    long            n2LineDY2;
    long            nUnderlineOffset;
    long            nStrikeoutOffset;
    long            nDescent;

    nDescent = pFontEntry->maMetric.mnDescent;
    if ( !nDescent )
    {
        nDescent = pFontEntry->maMetric.mnAscent*100/1000;
        if ( !nDescent )
            nDescent = 1;
    }

    nLineHeight = ((nDescent*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;
    nLineHeight2 = nLineHeight / 2;
    if ( !nLineHeight2 )
        nLineHeight2 = 1;

    nBLineHeight = ((nDescent*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;
    nBLineHeight2 = nBLineHeight/2;
    if ( !nBLineHeight2 )
        nBLineHeight2 = 1;

    n2LineHeight = ((nDescent*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;
    n2LineDY = n2LineHeight;
    if ( n2LineDY <= 0 )
        n2LineDY = 1;
    n2LineDY2 = n2LineDY/2;
    if ( !n2LineDY2 )
        n2LineDY2 = 1;

    nUnderlineOffset = nDescent/2 + 1;
    nStrikeoutOffset = -((pFontEntry->maMetric.mnAscent-pFontEntry->maMetric.mnLeading)/3);

    if ( !pFontEntry->maMetric.mnUnderlineSize )
    {
        pFontEntry->maMetric.mnUnderlineSize        = nLineHeight;
        pFontEntry->maMetric.mnUnderlineOffset      = nUnderlineOffset - nLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnBUnderlineSize )
    {
        pFontEntry->maMetric.mnBUnderlineSize       = nBLineHeight;
        pFontEntry->maMetric.mnBUnderlineOffset     = nUnderlineOffset - nBLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnDUnderlineSize )
    {
        pFontEntry->maMetric.mnDUnderlineSize       = n2LineHeight;
        pFontEntry->maMetric.mnDUnderlineOffset1    = nUnderlineOffset - n2LineDY2 - n2LineHeight;
        pFontEntry->maMetric.mnDUnderlineOffset2    = pFontEntry->maMetric.mnDUnderlineOffset1 + n2LineDY + n2LineHeight;
    }
    if ( !pFontEntry->maMetric.mnWUnderlineSize )
    {
        long nWCalcSize = pFontEntry->maMetric.mnDescent;
        if ( nWCalcSize < 6 )
        {
            if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
                pFontEntry->maMetric.mnWUnderlineSize = nWCalcSize;
            else
                pFontEntry->maMetric.mnWUnderlineSize = 3;
        }
        else
            pFontEntry->maMetric.mnWUnderlineSize = ((nWCalcSize*50)+50) / 100;
        pFontEntry->maMetric.mnWUnderlineOffset     = nUnderlineOffset;
    }

    if ( !pFontEntry->maMetric.mnStrikeoutSize )
    {
        pFontEntry->maMetric.mnStrikeoutSize        = nLineHeight;
        pFontEntry->maMetric.mnStrikeoutOffset      = nStrikeoutOffset - nLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnBStrikeoutSize )
    {
        pFontEntry->maMetric.mnBStrikeoutSize       = nBLineHeight;
        pFontEntry->maMetric.mnBStrikeoutOffset     = nStrikeoutOffset - nBLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnDStrikeoutSize )
    {
        pFontEntry->maMetric.mnDStrikeoutSize       = n2LineHeight;
        pFontEntry->maMetric.mnDStrikeoutOffset1    = nStrikeoutOffset - n2LineDY2 - n2LineHeight;
        pFontEntry->maMetric.mnDStrikeoutOffset2    = pFontEntry->maMetric.mnDStrikeoutOffset1 + n2LineDY + n2LineHeight;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitAboveTextLineSize()
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nLineHeight;
    long            nLineHeight2;
    long            nBLineHeight;
    long            nBLineHeight2;
    long            n2LineHeight;
    long            n2LineDY;
    long            n2LineDY2;
    long            nUnderlineOffset;
    long            nLeading;

    nLeading = pFontEntry->maMetric.mnLeading;
    if ( !nLeading )
    {
        // If no leading is available, we take 15% of the Ascent
        nLeading = pFontEntry->maMetric.mnAscent*150/1000;
        if ( !nLeading )
            nLeading = 1;
    }

    nLineHeight = ((nLeading*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;
    nLineHeight2 = nLineHeight / 2;
    if ( !nLineHeight2 )
        nLineHeight2 = 1;

    nBLineHeight = ((nLeading*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;
    nBLineHeight2 = nBLineHeight/2;
    if ( !nBLineHeight2 )
        nBLineHeight2 = 1;

    n2LineHeight = ((nLeading*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;
    n2LineDY = n2LineHeight;
    if ( n2LineDY <= 0 )
        n2LineDY = 1;
    n2LineDY2 = n2LineDY/2;
    if ( !n2LineDY2 )
        n2LineDY2 = 1;

    nUnderlineOffset = -(pFontEntry->maMetric.mnAscent-((nLeading/2)-1));

    if ( !pFontEntry->maMetric.mnAboveUnderlineSize )
    {
        pFontEntry->maMetric.mnAboveUnderlineSize       = nLineHeight;
        pFontEntry->maMetric.mnAboveUnderlineOffset     = nUnderlineOffset - nLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnAboveBUnderlineSize )
    {
        pFontEntry->maMetric.mnAboveBUnderlineSize      = nBLineHeight;
        pFontEntry->maMetric.mnAboveBUnderlineOffset    = nUnderlineOffset - nBLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnAboveDUnderlineSize )
    {
        pFontEntry->maMetric.mnAboveDUnderlineSize      = n2LineHeight;
        pFontEntry->maMetric.mnAboveDUnderlineOffset1   = nUnderlineOffset - n2LineDY2 - n2LineHeight;
        pFontEntry->maMetric.mnAboveDUnderlineOffset2   = pFontEntry->maMetric.mnAboveDUnderlineOffset1 + n2LineDY + n2LineHeight;
    }
    if ( !pFontEntry->maMetric.mnAboveWUnderlineSize )
    {
        long nWCalcSize = nLeading;
        if ( nWCalcSize < 6 )
        {
            if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
                pFontEntry->maMetric.mnAboveWUnderlineSize = nWCalcSize;
            else
                pFontEntry->maMetric.mnAboveWUnderlineSize = 3;
        }
        else
            pFontEntry->maMetric.mnAboveWUnderlineSize = ((nWCalcSize*50)+50) / 100;
        pFontEntry->maMetric.mnAboveWUnderlineOffset    = nUnderlineOffset;
    }
}

// -----------------------------------------------------------------------

static void ImplDrawWavePixel( long nOriginX, long nOriginY,
                               long nCurX, long nCurY,
                               short nOrientation,
#ifndef REMOTE_APPSERVER
                               SalGraphics* pGraphics,
#else
                               ImplServerGraphics* pGraphics,
#endif
                               BOOL bDrawPixAsRect,
                               long nPixWidth, long nPixHeight )
{
    if ( nOrientation )
        ImplRotatePos( nOriginX, nOriginY, nCurX, nCurY, nOrientation );

    if ( bDrawPixAsRect )
    {
#ifndef REMOTE_APPSERVER
        pGraphics->DrawRect( nCurX, nCurY, nPixWidth, nPixHeight );
#else
        Point       aPos( nCurX, nCurY );
        Size        aSize( nPixWidth, nPixHeight );
        Rectangle   aRect( aPos, aSize );
        pGraphics->DrawRect( aRect );
#endif
    }
    else
    {
#ifndef REMOTE_APPSERVER
        pGraphics->DrawPixel( nCurX, nCurY );
#else
        Point aPos( nCurX, nCurY );
        pGraphics->DrawPixel( aPos );
#endif
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawWaveLine( long nBaseX, long nBaseY,
                                     long nStartX, long nStartY,
                                     long nWidth, long nHeight,
                                     long nLineWidth, short nOrientation,
                                     const Color& rColor )
{
    if ( !nHeight )
        return;

    // Bei Hoehe von 1 Pixel reicht es, eine Linie auszugeben
    if ( (nLineWidth == 1) && (nHeight == 1) )
    {
#ifndef REMOTE_APPSERVER
        mpGraphics->SetLineColor( ImplColorToSal( rColor ) );
        mbInitLineColor = TRUE;
#else
        Color aOldLineColor = GetLineColor();
        SetLineColor( rColor );
        if ( mbInitLineColor )
            ImplInitLineColor();
#endif

        long nEndX = nStartX+nWidth;
        long nEndY = nStartY;
        if ( nOrientation )
        {
            ImplRotatePos( nBaseX, nBaseY, nStartX, nStartY, nOrientation );
            ImplRotatePos( nBaseX, nBaseY, nEndX, nEndY, nOrientation );
        }
#ifndef REMOTE_APPSERVER
        mpGraphics->DrawLine( nStartX, nStartY, nEndX, nEndY );
#else
        mpGraphics->DrawLine( Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
#endif

#ifdef REMOTE_APPSERVER
        SetLineColor( aOldLineColor );
#endif
    }
    else
    {
        long    nCurX = nStartX;
        long    nCurY = nStartY;
        long    nDiffX = 2;
        long    nDiffY = nHeight-1;
        long    nCount = nWidth;
        long    nOffY = -1;
        long    nFreq;
        long    i;
        long    nPixWidth;
        long    nPixHeight;
        BOOL    bDrawPixAsRect;
#ifdef REMOTE_APPSERVER
        Color   aOldLineColor = GetLineColor();
        Color   aOldFillColor = GetFillColor();
#endif
        // Auf Druckern die Pixel per DrawRect() ausgeben
        if ( (GetOutDevType() == OUTDEV_PRINTER) || (nLineWidth > 1) )
        {
#ifndef REMOTE_APPSERVER
            if ( mbLineColor || mbInitLineColor )
            {
                mpGraphics->SetLineColor();
                mbInitLineColor = TRUE;
            }
            mpGraphics->SetFillColor( ImplColorToSal( rColor ) );
            mbInitFillColor = TRUE;
#else
            SetLineColor();
            SetFillColor( rColor );
            if ( mbInitLineColor )
                ImplInitLineColor();
            if ( mbInitFillColor )
                ImplInitFillColor();
#endif
            bDrawPixAsRect  = TRUE;
            nPixWidth       = nLineWidth;
            nPixHeight      = ((nLineWidth*mnDPIX)+(mnDPIY/2))/mnDPIY;
        }
        else
        {
#ifndef REMOTE_APPSERVER
            mpGraphics->SetLineColor( ImplColorToSal( rColor ) );
            mbInitLineColor = TRUE;
#else
            Color aOldLineColor = GetLineColor();
            SetLineColor( rColor );
            if ( mbInitLineColor )
                ImplInitLineColor();
#endif
            nPixWidth       = 1;
            nPixHeight      = 1;
            bDrawPixAsRect  = FALSE;
        }

        if ( !nDiffY )
        {
            while ( nWidth )
            {
                ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                   mpGraphics,
                                   bDrawPixAsRect, nPixWidth, nPixHeight );
                nCurX++;
                nWidth--;
            }
        }
        else
        {
            nCurY += nDiffY;
            nFreq = nCount / (nDiffX+nDiffY);
            while ( nFreq-- )
            {
                for( i = nDiffY; i; --i )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                    nCurY += nOffY;
                }
                for( i = nDiffX; i; --i )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                }
                nOffY = -nOffY;
            }
            nFreq = nCount % (nDiffX+nDiffY);
            if ( nFreq )
            {
                for( i = nDiffY; i && nFreq; --i, --nFreq )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                    nCurY += nOffY;
                }
                for( i = nDiffX; i && nFreq; --i, --nFreq )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                }
            }
        }

#ifdef REMOTE_APPSERVER
        SetLineColor( aOldLineColor );
        SetFillColor( aOldFillColor );
#endif
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawTextLine( long nBaseX,
                                     long nX, long nY, long nWidth,
                                     FontStrikeout eStrikeout,
                                     FontUnderline eUnderline,
                                     BOOL bUnderlineAbove )
{
    if ( !nWidth )
        return;

    ImplFontEntry*  pFontEntry = mpFontEntry;
    Color           aTextLineColor = GetTextLineColor();
    long            nBaseY = nY;
    long            nLineHeight;
    long            nLinePos;
    long            nLinePos2;
    long            nLeft;
    BOOL            bNormalLines = TRUE;

    if ( !IsTextLineColor() )
        aTextLineColor = GetTextColor();

    if ( (eUnderline == UNDERLINE_SMALLWAVE) ||
         (eUnderline == UNDERLINE_WAVE) ||
         (eUnderline == UNDERLINE_DOUBLEWAVE) ||
         (eUnderline == UNDERLINE_BOLDWAVE) )
    {
        if ( bUnderlineAbove )
        {
            if ( !pFontEntry->maMetric.mnAboveWUnderlineSize )
                ImplInitAboveTextLineSize();
            nLinePos = pFontEntry->maMetric.mnAboveWUnderlineOffset;
            nLineHeight = pFontEntry->maMetric.mnAboveWUnderlineSize;
        }
        else
        {
            if ( !pFontEntry->maMetric.mnWUnderlineSize )
                ImplInitTextLineSize();
            nLinePos = pFontEntry->maMetric.mnWUnderlineOffset;
            nLineHeight = pFontEntry->maMetric.mnWUnderlineSize;
        }
        if ( (eUnderline == UNDERLINE_SMALLWAVE) &&
             (nLineHeight > 3) )
            nLineHeight = 3;
        long nLineWidth = (mnDPIX/300);
        if ( !nLineWidth )
            nLineWidth = 1;
        if ( eUnderline == UNDERLINE_BOLDWAVE )
            nLineWidth *= 2;
        nLinePos += nY - (nLineHeight / 2);
        long nLineWidthHeight = ((nLineWidth*mnDPIX)+(mnDPIY/2))/mnDPIY;
        if ( eUnderline == UNDERLINE_DOUBLEWAVE )
        {
            long nOrgLineHeight = nLineHeight;
            nLineHeight /= 3;
            if ( nLineHeight < 2 )
            {
                if ( nOrgLineHeight > 1 )
                    nLineHeight = 2;
                else
                    nLineHeight = 1;
            }
            long nLineDY = nOrgLineHeight-(nLineHeight*2);
            if ( nLineDY < nLineWidthHeight )
                nLineDY = nLineWidthHeight;
            long nLineDY2 = nLineDY/2;
            if ( !nLineDY2 )
                nLineDY2 = 1;
            nLinePos -= nLineWidthHeight-nLineDY2;
            ImplDrawWaveLine( nBaseX, nBaseY, nX, nLinePos, nWidth, nLineHeight,
                              nLineWidth, mpFontEntry->mnOrientation, aTextLineColor );
            nLinePos += nLineWidthHeight+nLineDY;
            ImplDrawWaveLine( nBaseX, nBaseY, nX, nLinePos, nWidth, nLineHeight,
                              nLineWidth, mpFontEntry->mnOrientation, aTextLineColor );
        }
        else
        {
            nLinePos -= nLineWidthHeight/2;
            ImplDrawWaveLine( nBaseX, nBaseY, nX, nLinePos, nWidth, nLineHeight,
                              nLineWidth, mpFontEntry->mnOrientation, aTextLineColor );
        }

        if ( (eStrikeout == STRIKEOUT_NONE) ||
             (eStrikeout == STRIKEOUT_DONTKNOW) )
            bNormalLines = FALSE;
    }

    if ( bNormalLines &&
         ((eStrikeout == STRIKEOUT_SLASH) || (eStrikeout == STRIKEOUT_X)) )
    {
        BOOL bOldMap = IsMapModeEnabled();
        EnableMapMode( FALSE );
        Color aOldColor = GetTextColor();
        SetTextColor( aTextLineColor );
        ImplInitTextColor();
        xub_Unicode c;
        if ( eStrikeout == STRIKEOUT_SLASH )
            c = '/';
        else /* ( eStrikeout == STRIKEOUT_X ) */
            c = 'X';
        // Strikeout-String zusammenbauen
        XubString aStrikeoutText( c );
        aStrikeoutText += aStrikeoutText.GetChar( 0 );
        long nStrikeoutWidth = GetTextWidth( aStrikeoutText );
        long nChars = nWidth/(nStrikeoutWidth/2);
        aStrikeoutText.Fill( (USHORT)(nChars+1), c );
        // String solange kuerzen, bis er nicht allzuweit uebersteht
        long nMaxWidth = nStrikeoutWidth/4;
        if ( nMaxWidth < 2 )
            nMaxWidth = 2;
        nMaxWidth += nWidth;
        long nFullStrikeoutWidth = GetTextWidth( aStrikeoutText );
        while ( (nFullStrikeoutWidth > nMaxWidth) && aStrikeoutText.Len() )
        {
            aStrikeoutText.Erase( aStrikeoutText.Len()-1 );
            nFullStrikeoutWidth = GetTextWidth( aStrikeoutText );
        }
        if ( mpFontEntry->mnOrientation )
            ImplRotatePos( nBaseX, nBaseY, nX, nY, mpFontEntry->mnOrientation );
        ImplDrawTextDirect( nX, nY,
                            aStrikeoutText.GetBuffer(), aStrikeoutText.Len(),
                            NULL, FALSE );
        SetTextColor( aOldColor );
        ImplInitTextColor();
        EnableMapMode( bOldMap );

        if ( (eUnderline == UNDERLINE_NONE) ||
             (eUnderline == UNDERLINE_DONTKNOW) ||
             (eUnderline == UNDERLINE_SMALLWAVE) ||
             (eUnderline == UNDERLINE_WAVE) ||
             (eUnderline == UNDERLINE_DOUBLEWAVE) ||
             (eUnderline == UNDERLINE_BOLDWAVE) )
            bNormalLines = FALSE;
    }

    if ( bNormalLines )
    {
#ifndef REMOTE_APPSERVER
        if ( mbLineColor || mbInitLineColor )
        {
            mpGraphics->SetLineColor();
            mbInitLineColor = TRUE;
        }
        mpGraphics->SetFillColor( ImplColorToSal( aTextLineColor ) );
        mbInitFillColor = TRUE;
#else
        Color aOldLineColor = GetLineColor();
        Color aOldFillColor = GetFillColor();
        SetLineColor();
        SetFillColor( aTextLineColor );
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( mbInitFillColor )
            ImplInitFillColor();
#endif

        if ( eUnderline > UNDERLINE_LAST )
            eUnderline = UNDERLINE_SINGLE;

        if ( (eUnderline == UNDERLINE_SINGLE) ||
             (eUnderline == UNDERLINE_DOTTED) ||
             (eUnderline == UNDERLINE_DASH) ||
             (eUnderline == UNDERLINE_LONGDASH) ||
             (eUnderline == UNDERLINE_DASHDOT) ||
             (eUnderline == UNDERLINE_DASHDOTDOT) )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveUnderlineSize )
                    ImplInitAboveTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnAboveUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnAboveUnderlineOffset;
            }
            else
            {
                if ( !pFontEntry->maMetric.mnUnderlineSize )
                    ImplInitTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnUnderlineOffset;
            }
        }
        else if ( (eUnderline == UNDERLINE_BOLD) ||
                  (eUnderline == UNDERLINE_BOLDDOTTED) ||
                  (eUnderline == UNDERLINE_BOLDDASH) ||
                  (eUnderline == UNDERLINE_BOLDLONGDASH) ||
                  (eUnderline == UNDERLINE_BOLDDASHDOT) ||
                  (eUnderline == UNDERLINE_BOLDDASHDOTDOT) )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveBUnderlineSize )
                    ImplInitAboveTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnAboveBUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnAboveBUnderlineOffset;
            }
            else
            {
                if ( !pFontEntry->maMetric.mnBUnderlineSize )
                    ImplInitTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnBUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnBUnderlineOffset;
            }
        }
        else if ( eUnderline == UNDERLINE_DOUBLE )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveDUnderlineSize )
                    ImplInitAboveTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnAboveDUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnAboveDUnderlineOffset1;
                nLinePos2   = nY + pFontEntry->maMetric.mnAboveDUnderlineOffset2;
            }
            else
            {
                if ( !pFontEntry->maMetric.mnDUnderlineSize )
                    ImplInitTextLineSize();
                nLineHeight = pFontEntry->maMetric.mnDUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnDUnderlineOffset1;
                nLinePos2   = nY + pFontEntry->maMetric.mnDUnderlineOffset2;
            }
        }
        else
            nLineHeight = 0;

        if ( nLineHeight )
        {
            nLeft = nX;

            if ( (eUnderline == UNDERLINE_SINGLE) ||
                 (eUnderline == UNDERLINE_BOLD) )
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
            else if ( eUnderline == UNDERLINE_DOUBLE )
            {
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos2, nWidth, nLineHeight );
            }
            else if ( (eUnderline == UNDERLINE_DOTTED) ||
                      (eUnderline == UNDERLINE_BOLDDOTTED) )
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;
                long nTempWidth = nDotWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempWidth > nEnd )
                        nTempWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                }
            }
            else if ( (eUnderline == UNDERLINE_DASH) ||
                      (eUnderline == UNDERLINE_LONGDASH) ||
                      (eUnderline == UNDERLINE_BOLDDASH) ||
                      (eUnderline == UNDERLINE_BOLDLONGDASH) )
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;
                long nMinDashWidth;
                long nMinSpaceWidth;
                long nSpaceWidth;
                long nDashWidth;
                if ( (eUnderline == UNDERLINE_LONGDASH) ||
                     (eUnderline == UNDERLINE_BOLDLONGDASH) )
                {
                    nMinDashWidth = nDotWidth*6;
                    nMinSpaceWidth = nDotWidth*2;
                    nDashWidth = 200;
                    nSpaceWidth = 100;
                }
                else
                {
                    nMinDashWidth = nDotWidth*4;
                    nMinSpaceWidth = (nDotWidth*150)/100;
                    nDashWidth = 100;
                    nSpaceWidth = 50;
                }
                nDashWidth = ((nDashWidth*mnDPIX)+1270)/2540;
                nSpaceWidth = ((nSpaceWidth*mnDPIX)+1270)/2540;
                // DashWidth wird gegebenenfalls verbreitert, wenn
                // die dicke der Linie im Verhaeltnis zur Laenge
                // zu dick wird
                if ( nDashWidth < nMinDashWidth )
                    nDashWidth = nMinDashWidth;
                if ( nSpaceWidth < nMinSpaceWidth )
                    nSpaceWidth = nMinSpaceWidth;
                long nTempWidth = nDashWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempWidth > nEnd )
                        nTempWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempWidth, nLineHeight );
                    nLeft += nDashWidth+nSpaceWidth;
                }
            }
            else if ( (eUnderline == UNDERLINE_DASHDOT) ||
                      (eUnderline == UNDERLINE_BOLDDASHDOT) )
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;
                long nDashWidth = ((100*mnDPIX)+1270)/2540;
                long nMinDashWidth = nDotWidth*4;
                // DashWidth wird gegebenenfalls verbreitert, wenn
                // die dicke der Linie im Verhaeltnis zur Laenge
                // zu dick wird
                if ( nDashWidth < nMinDashWidth )
                    nDashWidth = nMinDashWidth;
                long nTempDotWidth = nDotWidth;
                long nTempDashWidth = nDashWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempDotWidth > nEnd )
                        nTempDotWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                    if ( nLeft > nEnd )
                        break;
                    if ( nLeft+nTempDashWidth > nEnd )
                        nTempDashWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDashWidth, nLineHeight );
                    nLeft += nDashWidth+nDotWidth;
                }
            }
            else if ( (eUnderline == UNDERLINE_DASHDOTDOT) ||
                      (eUnderline == UNDERLINE_BOLDDASHDOTDOT) )
            {
                long nDotWidth = nLineHeight*mnDPIY;
                nDotWidth += mnDPIY/2;
                nDotWidth /= mnDPIY;
                long nDashWidth = ((100*mnDPIX)+1270)/2540;
                long nMinDashWidth = nDotWidth*4;
                // DashWidth wird gegebenenfalls verbreitert, wenn
                // die dicke der Linie im Verhaeltnis zur Laenge
                // zu dick wird
                if ( nDashWidth < nMinDashWidth )
                    nDashWidth = nMinDashWidth;
                long nTempDotWidth = nDotWidth;
                long nTempDashWidth = nDashWidth;
                long nEnd = nLeft+nWidth;
                while ( nLeft < nEnd )
                {
                    if ( nLeft+nTempDotWidth > nEnd )
                        nTempDotWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                    if ( nLeft > nEnd )
                        break;
                    if ( nLeft+nTempDotWidth > nEnd )
                        nTempDotWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                    nLeft += nDotWidth*2;
                    if ( nLeft > nEnd )
                        break;
                    if ( nLeft+nTempDashWidth > nEnd )
                        nTempDashWidth = nEnd-nLeft;
                    ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDashWidth, nLineHeight );
                    nLeft += nDashWidth+nDotWidth;
                }
            }
        }

        if ( eStrikeout > STRIKEOUT_LAST )
            eStrikeout = STRIKEOUT_SINGLE;

        if ( eStrikeout == STRIKEOUT_SINGLE )
        {
            if ( !pFontEntry->maMetric.mnStrikeoutSize )
                ImplInitTextLineSize();
            nLineHeight = pFontEntry->maMetric.mnStrikeoutSize;
            nLinePos    = nY + pFontEntry->maMetric.mnStrikeoutOffset;
        }
        else if ( eStrikeout == STRIKEOUT_BOLD )
        {
            if ( !pFontEntry->maMetric.mnBStrikeoutSize )
                ImplInitTextLineSize();
            nLineHeight = pFontEntry->maMetric.mnBStrikeoutSize;
            nLinePos    = nY + pFontEntry->maMetric.mnBStrikeoutOffset;
        }
        else if ( eStrikeout == STRIKEOUT_DOUBLE )
        {
            if ( !pFontEntry->maMetric.mnDStrikeoutSize )
                ImplInitTextLineSize();
            nLineHeight = pFontEntry->maMetric.mnDStrikeoutSize;
            nLinePos    = nY + pFontEntry->maMetric.mnDStrikeoutOffset1;
            nLinePos2   = nY + pFontEntry->maMetric.mnDStrikeoutOffset2;
        }
        else
            nLineHeight = 0;

        if ( nLineHeight )
        {
            nLeft = nX;

            if ( (eStrikeout == STRIKEOUT_SINGLE) ||
                 (eStrikeout == STRIKEOUT_BOLD) )
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
            else if ( eStrikeout == STRIKEOUT_DOUBLE )
            {
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos2, nWidth, nLineHeight );
            }
        }

#ifdef REMOTE_APPSERVER
        SetLineColor( aOldLineColor );
        SetFillColor( aOldFillColor );
#endif
    }
}

// -----------------------------------------------------------------------

static BOOL ImplIsLineCharacter( sal_Unicode c )
{
    // !(Control+Space, C1-Control+HardSpace, General Space Punctuation)
    if ( ((c > 0x0020) && (c < 0x0080)) ||
         ((c > 0x00A0) && (c < 0x2000)) ||
         (c > 0x200F) )
        return TRUE;
    return FALSE;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawTextLines( long nX, long nY,
                                      const sal_Unicode* pStr, xub_StrLen nLen,
                                      const long* pDXAry,
                                      FontStrikeout eStrikeout,
                                      FontUnderline eUnderline,
                                      BOOL bWordLine, BOOL bUnderlineAbove )
{
    if ( bWordLine )
    {
        BOOL        bLine = FALSE;
        xub_StrLen  nLineStart = 0;
        xub_StrLen  nLineEnd = 0;
        while ( nLineEnd < nLen )
        {
            BOOL bCurLine = ImplIsLineCharacter( *(pStr+nLineEnd) );

            // draw a new line?
            if ( bLine && !bCurLine )
            {
                // Query Size to text start and draw the Line to text end
                long nStartX = ImplGetTextWidth( pStr, nLineStart, pDXAry );
                long nEndX = ImplGetTextWidth( pStr, nLineEnd, pDXAry );
                ImplDrawTextLine( nX, nX+nStartX, nY, nEndX-nStartX, eStrikeout, eUnderline, bUnderlineAbove );
            }
            if ( bLine != bCurLine )
            {
                bLine = bCurLine;
                nLineStart = nLineEnd;
            }
            nLineEnd++;
        }

        if ( bLine && nLen )
        {
            // Query Size to text start and draw the Line to text end
            long nStartX = ImplGetTextWidth( pStr, nLineStart, pDXAry );
            long nEndX = ImplGetTextWidth( pStr, nLineEnd, pDXAry );
            ImplDrawTextLine( nX, nX+nStartX, nY, nEndX-nStartX, eStrikeout, eUnderline, bUnderlineAbove );
        }
    }
    else
        ImplDrawTextLine( nX, nX, nY, ImplGetTextWidth( pStr, nLen, pDXAry ), eStrikeout, eUnderline, bUnderlineAbove );
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawMnemonicLine( long nX, long nY, xub_Unicode c )
{
    ImplDrawTextLine( nX, nX, nY, ImplGetTextWidth( &c, 1, NULL ), STRIKEOUT_NONE, UNDERLINE_SINGLE, FALSE );
}

// -----------------------------------------------------------------------

void OutputDevice::ImplGetEmphasisMark( PolyPolygon& rPolyPoly, BOOL& rPolyLine,
                                        Rectangle& rRect1, Rectangle& rRect2,
                                        long& rYOff, long& rWidth,
                                        FontEmphasisMark eEmphasis,
                                        long nHeight, short nOrient )
{
    static const BYTE aAccentPolyFlags[24] =
    {
        0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 0, 2, 0, 2, 2
    };

    static const long aAccentPos[48] =
    {
         78,      0,
        348,     79,
        599,    235,
        843,    469,
        938,    574,
        990,    669,
        990,    773,
        990,    843,
        964,    895,
        921,    947,
        886,    982,
        860,    999,
        825,    999,
        764,    999,
        721,    964,
        686,    895,
        625,    791,
        556,    660,
        469,    504,
        400,    400,
        261,    252,
         61,     61,
          0,     27,
          9,      0
    };

    rWidth      = 0;
    rYOff       = 0;
    rPolyLine   = FALSE;

    if ( !nHeight )
        return;

    FontEmphasisMark    nEmphasisStyle = eEmphasis & EMPHASISMARK_STYLE;
    long                nDotSize = 0;
    switch ( nEmphasisStyle )
    {
        case EMPHASISMARK_DOT:
            // Dot has 55% of the height
            nDotSize = (nHeight*550)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                long nRad = nDotSize/2;
                Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                rPolyPoly.Insert( aPoly );
            }
            rYOff = ((nHeight*250)/1000)/2; // Center to the anthoer EmphasisMarks
            rWidth = nDotSize;
            break;

        case EMPHASISMARK_CIRCLE:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                long nRad = nDotSize/2;
                Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                rPolyPoly.Insert( aPoly );
                // BorderWidth is 15%
                long nBorder = (nDotSize*150)/1000;
                if ( nBorder <= 1 )
                    rPolyLine = TRUE;
                else
                {
                    Polygon aPoly2( Point( nRad, nRad ),
                                    nRad-nBorder, nRad-nBorder );
                    rPolyPoly.Insert( aPoly2 );
                }
            }
            rWidth = nDotSize;
            break;

        case EMPHASISMARK_DISC:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                long nRad = nDotSize/2;
                Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                rPolyPoly.Insert( aPoly );
            }
            rWidth = nDotSize;
            break;

        case EMPHASISMARK_ACCENT:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
            {
                if ( nDotSize == 1 )
                {
                    rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
                    rWidth = nDotSize;
                }
                else
                {
                    rRect1 = Rectangle( Point(), Size( 1, 1 ) );
                    rRect2 = Rectangle( Point( 1, 1 ), Size( 1, 1 ) );
                }
            }
            else
            {
                Polygon aPoly( sizeof( aAccentPos ) / sizeof( long ) / 2,
                               (const Point*)aAccentPos,
                               NULL ); // aAccentPolyFlags );
                double dScale = ((double)nDotSize)/1000.0;
                aPoly.Scale( dScale, dScale );
                Rectangle aBoundRect = aPoly.GetBoundRect();
                rWidth = aBoundRect.GetWidth();
                nDotSize = aBoundRect.GetHeight();
                if ( nOrient )
                    aPoly.Rotate( Point( 0, 0 ), nOrient );
                rPolyPoly.Insert( aPoly );
            }
            break;
    }

    // Calculate Position
    long nOffY = 1+(mnDPIY/300); // One visible pixel space
    long nSpaceY = nHeight-nDotSize;
    if ( nSpaceY >= nOffY*2 )
        rYOff += nOffY;
    if ( !(eEmphasis & EMPHASISMARK_POS_BELOW) )
        rYOff += nDotSize;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawEmphasisMark( long nX, long nY,
                                         const PolyPolygon& rPolyPoly, BOOL bPolyLine,
                                         const Rectangle& rRect1, const Rectangle& rRect2 )
{
    if ( rPolyPoly.Count() )
    {
        if ( bPolyLine )
        {
            Polygon aPoly = rPolyPoly.GetObject( 0 );
            aPoly.Move( nX, nY );
            DrawPolyLine( aPoly );
        }
        else
        {
            PolyPolygon aPolyPoly = rPolyPoly;
            aPolyPoly.Move( nX, nY );
            DrawPolyPolygon( aPolyPoly );
        }
    }

    if ( !rRect1.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect1.Left(),
                                nY+rRect1.Top() ), rRect1.GetSize() );
        DrawRect( aRect );
    }

    if ( !rRect2.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect2.Left(),
                                nY+rRect2.Top() ), rRect2.GetSize() );
        DrawRect( aRect );
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawEmphasisMarks( long nX, long nY,
                                          const sal_Unicode* pStr, xub_StrLen nLen,
                                          const long* pDXAry )
{
    Color               aOldColor       = GetTextColor();
    Color               aOldLineColor   = GetLineColor();
    Color               aOldFillColor   = GetFillColor();
    BOOL                bOldMap         = mbMap;
    GDIMetaFile*        pOldMetaFile    = mpMetaFile;
    mpMetaFile = NULL;
    mbMap = FALSE;

    FontEmphasisMark    nEmphasisMark = ImplGetEmphasisMarkStyle( maFont );
    PolyPolygon         aPolyPoly;
    Rectangle           aRect1;
    Rectangle           aRect2;
    long                nEmphasisYOff;
    long                nEmphasisWidth;
    long                nEmphasisHeight;
    BOOL                bPolyLine;

    if ( nEmphasisMark & EMPHASISMARK_POS_BELOW )
        nEmphasisHeight = mnEmphasisDescent;
    else
        nEmphasisHeight = mnEmphasisAscent;

    ImplGetEmphasisMark( aPolyPoly, bPolyLine,
                         aRect1, aRect2,
                         nEmphasisYOff, nEmphasisWidth,
                         nEmphasisMark,
                         nEmphasisHeight, mpFontEntry->mnOrientation );

    if ( bPolyLine )
    {
        SetLineColor( GetTextColor() );
        SetFillColor();
    }
    else
    {
        SetLineColor();
        SetFillColor( GetTextColor() );
    }

    long                nOffX = nX - mnOutOffX;
    long                nOffY = nY - mnOutOffY;
    long                nBaseX = nOffX;
    long                nBaseY = nOffY;
    xub_StrLen          i = 0;
    if ( nEmphasisMark & EMPHASISMARK_POS_BELOW )
        nOffY += mpFontEntry->maMetric.mnDescent+nEmphasisYOff;
    else
        nOffY -= mpFontEntry->maMetric.mnAscent+nEmphasisYOff;
    while ( i < nLen )
    {
        if ( ImplIsLineCharacter( *(pStr+i) ) )
        {
            long nStartX = ImplGetTextWidth( pStr, i, pDXAry );
            long nEndX = ImplGetTextWidth( pStr, i+1, pDXAry );
            long nOutX = nOffX + nStartX + ((nEndX-nStartX-nEmphasisWidth)/2);
            long nOutY = nOffY;
            if ( mpFontEntry->mnOrientation )
                ImplRotatePos( nBaseX, nBaseY, nOutX, nOutY, mpFontEntry->mnOrientation );
            ImplDrawEmphasisMark( nOutX, nOutY,
                                  aPolyPoly, bPolyLine,
                                  aRect1, aRect2 );
        }

        i++;
    }

    SetLineColor( aOldLineColor );
    SetFillColor( aOldFillColor );
    mbMap = bOldMap;
    mpMetaFile = pOldMetaFile;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::ImplDrawRotateText( long nX, long nY,
                                       const xub_Unicode* pStr, xub_StrLen nLen,
                                       const long* pDXAry )
{
    if ( !mpOutDevData )
        ImplInitOutDevData();
    if ( !mpOutDevData->mpRotateDev )
        mpOutDevData->mpRotateDev = new VirtualDevice( *this, 1 );

    VirtualDevice*  pVDev = mpOutDevData->mpRotateDev;
    long            nWidth = ImplGetTextWidth( pStr, nLen, pDXAry );
    long            nHeight = mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent;
    Size            aSize( nWidth, nHeight );

    if ( pVDev->SetOutputSizePixel( aSize ) )
    {
        Font    aFont( GetFont() );
        Bitmap  aBmp;
        long    nOff;

        nX -= mnTextOffX;
        nY -= mnTextOffY;
        if ( GetTextAlign() == ALIGN_TOP )
        {
            nOff = 0L;
            nY  += mpFontEntry->maMetric.mnAscent;
        }
        else if ( GetTextAlign() == ALIGN_BOTTOM )
        {
            nOff = mpFontEntry->maMetric.mnAscent;
            nY  += -mpFontEntry->maMetric.mnDescent;
        }
        else
            nOff = mpFontEntry->maMetric.mnAscent;

        aFont.SetShadow( FALSE );
        aFont.SetOutline( FALSE );
        aFont.SetRelief( RELIEF_NONE );
        aFont.SetOrientation( 0 );
        aFont.SetSize( Size( mpFontEntry->maFontSelData.mnWidth, mpFontEntry->maFontSelData.mnHeight ) );
        pVDev->SetFont( aFont );
        // Da Farben und Alignment noch im Font haengen, muessen diese jedesmal
        // gesetzt werden
        pVDev->SetTextAlign( ALIGN_TOP );
        pVDev->SetTextColor( Color( COL_BLACK ) );
        pVDev->SetTextFillColor();
        pVDev->ImplNewFont();
        pVDev->ImplInitFont();
        pVDev->ImplInitTextColor();
        pVDev->ImplDrawText( 0, 0, pStr, nLen, pDXAry );

        aBmp = pVDev->GetBitmap( Point(), aSize );
        if ( !!aBmp && aBmp.Rotate( mpFontEntry->mnOwnOrientation, COL_WHITE ) )
        {
            Point           aTempPoint;
            Polygon         aPoly( Rectangle( aTempPoint, aSize ) );
            long            nOldOffX = mnOutOffX;
            long            nOldOffY = mnOutOffY;
            GDIMetaFile*    pOldMetaFile = mpMetaFile;
            BOOL            bOldMap = mbMap;

            aTempPoint.Y() = nOff;
            aPoly.Rotate( aTempPoint, mpFontEntry->mnOwnOrientation );
            const Rectangle aBound( aPoly.GetBoundRect() );

            mnOutOffX   = 0L;
            mnOutOffY   = 0L;
            mpMetaFile  = NULL;
            mbMap       = FALSE;

            DrawMask( Point( nX + aBound.Left(),
                             nY + aBound.Top() - (mpFontEntry->maMetric.mnAscent+mnEmphasisAscent) ),
                      aBmp, GetTextColor() );

            mnOutOffX   = nOldOffX;
            mnOutOffY   = nOldOffY;
            mbMap       = bOldMap;
            mpMetaFile  = pOldMetaFile;
        }

        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawTextDirect( long nX, long nY,
                                       const xub_Unicode* pStr, xub_StrLen nLen,
                                       const long* pDXAry,
                                       BOOL bTextLines )
{
    BOOL bDraw = FALSE;
    ImplFontEntry* pFontEntry = mpFontEntry;
    if ( pFontEntry->mnOwnOrientation )
        bDraw = ImplDrawRotateText( nX, nY, pStr, nLen, pDXAry );
    if ( !bDraw )
    {
        if ( !pDXAry )
        {
#ifndef REMOTE_APPSERVER
            if ( pFontEntry->mnSetFontFlags & SAL_SETFONT_USEDRAWTEXTARRAY )
            {
                long                nOffset = 0;
                long                aStackAry[128];
                long*               pTempDXAry = (long*)ImplGetStackBuffer( sizeof(long)*(nLen-1), aStackAry, sizeof( aStackAry ) );
                const xub_Unicode*  pTempStr = pStr;
                for ( xub_StrLen i = 0; i < nLen-1; i++ )
                {
                    nOffset += ImplGetCharWidth( *pTempStr );
                    pTempDXAry[i] = nOffset / mpFontEntry->mnWidthFactor;
                    pTempStr++;
                }
                mpGraphics->DrawTextArray( nX, nY, pStr, nLen, pTempDXAry );
                ImplReleaseStackBuffer( pTempDXAry, aStackAry );
            }
            else
#endif
                mpGraphics->DrawText( nX, nY, pStr, nLen );
        }
        else
        {
#ifndef REMOTE_APPSERVER
            if ( pFontEntry->mnSetFontFlags & SAL_SETFONT_USEDRAWTEXT )
            {
                long                nOffset = 0;
                long                nDiff;
                long                nTempX = nX;
                const sal_Unicode*  pTempStr = pStr;
                xub_StrLen          nCombineChars = 1;
                for ( xub_StrLen i = 0; i < nLen-1; i++ )
                {
                    nOffset += ImplGetCharWidth( pStr[i] );
                    nDiff = (nOffset/mpFontEntry->mnWidthFactor) - pDXAry[i];
                    if ( (nDiff < -1) || (nDiff > 0) )
                    {
                        mpGraphics->DrawText( nTempX, nY, pTempStr, nCombineChars );
                        nTempX    = nX+pDXAry[i];
                        nOffset   = pDXAry[i]*mpFontEntry->mnWidthFactor;
                        pTempStr += nCombineChars;
                        nCombineChars = 1;
                    }
                    else
                        nCombineChars++;
                }
                mpGraphics->DrawText( nTempX, nY, pTempStr, nCombineChars );
            }
            else
#endif
                mpGraphics->DrawTextArray( nX, nY, pStr, nLen, pDXAry );
        }

        if ( bTextLines )
        {
            ImplDrawTextLines( nX, nY, pStr, nLen, pDXAry,
                               maFont.GetStrikeout(),
                               maFont.GetUnderline(),
                               maFont.IsWordLineMode(),
                               ImplIsUnderlineAbove( maFont ) );
        }

        // EmphasisMark
        if ( maFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
        {
            ImplDrawEmphasisMarks( nX, nY, pStr, nLen, pDXAry );
        }
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawSpecialText( long nX, long nY,
                                        const xub_Unicode* pStr, xub_StrLen nLen,
                                        const long* pDXAry )
{
    Color       aOldColor           = GetTextColor();
    Color       aOldTextLineColor   = GetTextLineColor();
    FontRelief  eRelief             = maFont.GetRelief();

    if ( eRelief != RELIEF_NONE )
    {
        Color   aReliefColor( COL_LIGHTGRAY );
        Color   aTextColor( aOldColor );
        Color   aTextLineColor( aOldTextLineColor );

        // We don't have a automatic Color, so Black is always drawn
        // in White
        if ( aTextColor.GetColor() == COL_BLACK )
            aTextColor = Color( COL_WHITE );
        if ( aTextLineColor.GetColor() == COL_BLACK )
            aTextLineColor = Color( COL_WHITE );

        // Relief-Color is Black for White Text, in all other cases
        // we set this to LightGray
        if ( aTextColor.GetColor() == COL_WHITE )
            aReliefColor = Color( COL_BLACK );
        SetTextLineColor( aReliefColor );
        SetTextColor( aReliefColor );
        ImplInitTextColor();

        // Calculate Offset - for High resolution printers the offset
        // should be greater so that the effect is visible
        long nOff = 1;
        nOff += mnDPIX/300;

        if ( eRelief == RELIEF_ENGRAVED )
            nOff = -nOff;
        ImplDrawTextDirect( nX+nOff, nY+nOff, pStr, nLen, pDXAry, mbTextLines );

        SetTextLineColor( aTextLineColor );
        SetTextColor( aTextColor );
        ImplInitTextColor();
        ImplDrawTextDirect( nX, nY, pStr, nLen, pDXAry, mbTextLines );

        SetTextLineColor( aOldTextLineColor );
        if ( aTextColor != aOldColor )
        {
            SetTextColor( aOldColor );
            ImplInitTextColor();
        }
    }
    else
    {
        if ( maFont.IsShadow() )
        {
            long nOff = 1 + ((mpFontEntry->mnLineHeight-24)/24);
            if ( maFont.IsOutline() )
                nOff++;
            SetTextLineColor();
            if ( GetTextColor().GetColor() == COL_BLACK )
                SetTextColor( Color( COL_LIGHTGRAY ) );
            else
                SetTextColor( Color( COL_BLACK ) );
            ImplInitTextColor();
            ImplDrawTextDirect( nX+nOff, nY+nOff, pStr, nLen, pDXAry, mbTextLines );
            SetTextColor( aOldColor );
            SetTextLineColor( aOldTextLineColor );
            ImplInitTextColor();

            if ( !maFont.IsOutline() )
                ImplDrawTextDirect( nX, nY, pStr, nLen, pDXAry, mbTextLines );
        }

        if ( maFont.IsOutline() )
        {
            ImplDrawTextDirect( nX-1, nY+1, pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX,   nY+1, pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX+1, nY+1, pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX-1, nY,   pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX+1, nY,   pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX-1, nY-1, pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX,   nY-1, pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX+1, nY-1, pStr, nLen, pDXAry, mbTextLines );

            SetTextColor( Color( COL_WHITE ) );
            SetTextLineColor( Color( COL_WHITE ) );
            ImplInitTextColor();
            ImplDrawTextDirect( nX, nY, pStr, nLen, pDXAry, mbTextLines );
            SetTextColor( aOldColor );
            SetTextLineColor( aOldTextLineColor );
            ImplInitTextColor();
        }
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawText( long nX, long nY,
                                 const xub_Unicode* pStr, xub_StrLen nLen, const long* pDXAry )
{
    nX += mnTextOffX;
    nY += mnTextOffY;

    if ( IsTextFillColor() )
        ImplDrawTextBackground( nX, nY, pStr, nLen, pDXAry );

    if ( mbTextSpecial )
        ImplDrawSpecialText( nX, nY, pStr, nLen, pDXAry );
    else
        ImplDrawTextDirect( nX, nY, pStr, nLen, pDXAry, mbTextLines );
}

// -----------------------------------------------------------------------

void OutputDevice::ImplFillDXAry( long* pDXAry,
                                  const xub_Unicode* pStr, xub_StrLen nLen, long nWidth )
{
    ImplFontEntry* pFontEntry = mpFontEntry;

    // Breiten-Array fuer errechnete Werte mit den Breiten der einzelnen
    // Character fuellen
    xub_StrLen  i;
    long        nSum = 0;
    for ( i = 0; i < nLen; i++ )
    {
        // Characterbreiten ueber Array holen
        nSum += ImplGetCharWidth( pStr[i] );
        pDXAry[i] = nSum / mpFontEntry->mnWidthFactor;
    }
    nSum /= mpFontEntry->mnWidthFactor;

    // Differenz zwischen Soll- und Ist-Laenge errechnen
    // Zusaetzliche Pixel per Character errechnen
    // Anzahl der zusaetzlich verbliebenen Pixel errechnen
    long nDelta        = (long)nWidth - nSum;
    long nDeltaPerChar = 0;
    long nDeltaRest    = 0;
    if ( nLen > 1 )
    {
        nDeltaPerChar = nDelta / (long)(nLen-1);
        nDeltaRest    = nDelta % (long)(nLen-1);
    }
    long nDeltaRestAbs = Abs( nDeltaRest );

    long nErrorSum = nDeltaRestAbs;
    long nDeltaSum = 0;
    for ( i = 0; i < nLen-1; i++, nErrorSum += nDeltaRestAbs )
    {
        nDeltaSum += nDeltaPerChar;
        if ( nErrorSum >= nLen-1 )
        {
            nErrorSum -= nLen-1;
            if ( nDeltaRest > 0 )
                nDeltaSum++;
            else if ( nDeltaRest < 0 )
                nDeltaSum--;
        }
        pDXAry[i] += nDeltaSum;
    }
}

// -----------------------------------------------------------------------

long OutputDevice::ImplGetTextLines( ImplMultiTextLineInfo& rLineInfo,
                                     long nWidth, const XubString& rStr,
                                     USHORT nStyle ) const
{
    DBG_ASSERTWARNING( nWidth >= 0, "ImplGetTextLines: nWidth <= 0!" );

    if ( nWidth <= 0 )
        nWidth = 1;

    long nMaxLineWidth  = 0;
    rLineInfo.Clear();
    if ( rStr.Len() && (nWidth > 0) )
    {
        ::rtl::OUString aText( rStr );
        uno::Reference < i18n::XBreakIterator > xBI;
        uno::Reference< linguistic2::XHyphenator > xHyph;
        i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, uno::Sequence <beans::PropertyValue>(), 1 );
        i18n::LineBreakUserOptions aUserOptions;

        xub_StrLen nPos = 0;
        xub_StrLen nLen = rStr.Len();
        while ( nPos < nLen )
        {
            xub_StrLen nBreakPos = nPos;
            while ( ( nBreakPos < nLen ) && ( rStr.GetChar( nBreakPos ) != _CR ) && ( rStr.GetChar( nBreakPos ) != _LF ) )
                nBreakPos++;

            long nLineWidth = GetTextWidth( rStr, nPos, nBreakPos-nPos );
            if ( ( nLineWidth > nWidth ) && ( nStyle & TEXT_DRAW_WORDBREAK ) )
            {
                if ( !xBI.is() )
                    xBI = vcl::unohelper::CreateBreakIterator();

                xub_StrLen nSoftBreak = GetTextBreak( rStr, nWidth, nPos, nBreakPos - nPos );
                DBG_ASSERT( nSoftBreak < nBreakPos, "Break?!" );
                i18n::LineBreakResults aLBR = xBI->getLineBreak( aText, nSoftBreak, GetSettings().GetLocale(), nPos, aHyphOptions, aUserOptions );
                nBreakPos = (xub_StrLen)aLBR.breakIndex;
                if ( nBreakPos <= nPos )
                    nBreakPos = nSoftBreak;
                nLineWidth = GetTextWidth( rStr, nPos, nBreakPos-nPos );
            }

            if ( nLineWidth > nMaxLineWidth )
                nMaxLineWidth = nLineWidth;

            rLineInfo.AddLine( new ImplTextLineInfo( nLineWidth, nPos, nBreakPos-nPos ) );

            if ( nBreakPos == nPos )
                nBreakPos++;
            nPos = nBreakPos;

            if ( ( rStr.GetChar( nPos ) == _CR ) || ( rStr.GetChar( nPos ) == _LF ) )
            {
                nPos++;
                // CR/LF?
                if ( ( nPos < nLen ) && ( rStr.GetChar( nPos ) == _LF ) && ( rStr.GetChar( nPos-1 ) == _CR ) )
                    nPos++;
            }
        }
    }
#ifdef DBG_UTIL
    for ( USHORT nL = 0; nL < rLineInfo.Count(); nL++ )
    {
        ImplTextLineInfo* pLine = rLineInfo.GetLine( nL );
        String aLine( rStr, pLine->GetIndex(), pLine->GetLen() );
        DBG_ASSERT( aLine.Search( _CR ) == STRING_NOTFOUND, "ImplGetTextLines - Found CR!" );
        DBG_ASSERT( aLine.Search( _LF ) == STRING_NOTFOUND, "ImplGetTextLines - Found LF!" );
    }
#endif

    return nMaxLineWidth;
}

// =======================================================================

void OutputDevice::SetAntialiasing( USHORT nMode )
{
    if ( mnAntialiasing != nMode )
    {
        mnAntialiasing = nMode;
        mbInitFont = TRUE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::SetFont( const Font& rNewFont )
{
    DBG_TRACE( "OutputDevice::SetFont()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rNewFont, Font, NULL );

    Font aFont( rNewFont );

    if ( mnDrawMode & (DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT | DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT |
                       DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL | DRAWMODE_GRAYFILL | DRAWMODE_NOFILL |
                       DRAWMODE_GHOSTEDFILL) )
    {
        Color aTextColor( aFont.GetColor() );
        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aTextColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aTextColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const UINT8 cLum = aTextColor.GetLuminance();
            aTextColor = Color( cLum, cLum, cLum );
        }
        if ( mnDrawMode & DRAWMODE_GHOSTEDTEXT )
        {
            aTextColor = Color( (aTextColor.GetRed() >> 1 ) | 0x80,
                                (aTextColor.GetGreen() >> 1 ) | 0x80,
                                (aTextColor.GetBlue() >> 1 ) | 0x80 );
        }
        aFont.SetColor( aTextColor );

        BOOL bTransFill = aFont.IsTransparent();
        if ( !bTransFill )
        {
            Color aTextFillColor( aFont.GetFillColor() );
            if ( mnDrawMode & DRAWMODE_BLACKFILL )
                aTextFillColor = Color( COL_BLACK );
            else if ( mnDrawMode & DRAWMODE_WHITEFILL )
                aTextFillColor = Color( COL_WHITE );
            else if ( mnDrawMode & DRAWMODE_GRAYFILL )
            {
                const UINT8 cLum = aTextFillColor.GetLuminance();
                aTextFillColor = Color( cLum, cLum, cLum );
            }
            else if ( mnDrawMode & DRAWMODE_NOFILL )
            {
                aTextFillColor = Color( COL_TRANSPARENT );
                bTransFill = TRUE;
            }
            if ( !bTransFill && (mnDrawMode & DRAWMODE_GHOSTEDFILL) )
            {
                aTextFillColor = Color( (aTextFillColor.GetRed() >> 1) | 0x80,
                                        (aTextFillColor.GetGreen() >> 1) | 0x80,
                                        (aTextFillColor.GetBlue() >> 1) | 0x80 );
            }
            aFont.SetFillColor( aTextFillColor );
        }
    }

    if ( mpMetaFile )
    {
        const Color& rTextFillColor = aFont.GetFillColor();

        mpMetaFile->AddAction( new MetaFontAction( aFont ) );
        mpMetaFile->AddAction( new MetaTextAlignAction( aFont.GetAlign() ) );
        mpMetaFile->AddAction( new MetaTextColorAction( aFont.GetColor() ) );
        mpMetaFile->AddAction( new MetaTextFillColorAction( aFont.GetFillColor(), !aFont.IsTransparent() ) );
    }

    if ( !maFont.IsSameInstance( aFont ) )
    {
        if ( maFont.GetColor() != aFont.GetColor() )
            mbInitTextColor = TRUE;
        maFont      = aFont;
        mbNewFont   = TRUE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextColor( const Color& rColor )
{
    DBG_TRACE( "OutputDevice::SetTextColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );

    if ( mnDrawMode & ( DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT |
                        DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT ) )
    {
        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const UINT8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }

        if ( mnDrawMode & DRAWMODE_GHOSTEDTEXT )
        {
            aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                            (aColor.GetGreen() >> 1) | 0x80,
                            (aColor.GetBlue() >> 1) | 0x80 );
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextColorAction( aColor ) );

    if ( maFont.GetColor() != aColor )
    {
        maFont.SetColor( aColor );
        mbInitTextColor = TRUE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextFillColor()
{
    DBG_TRACE( "OutputDevice::SetTextFillColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( Color(), FALSE ) );

    if ( maFont.GetColor() != Color( COL_TRANSPARENT ) )
        maFont.SetFillColor( Color( COL_TRANSPARENT ) );
    if ( !maFont.IsTransparent() )
        maFont.SetTransparent( TRUE );
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextFillColor( const Color& rColor )
{
    DBG_TRACE( "OutputDevice::SetTextFillColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );
    BOOL bTransFill = ImplIsColorTransparent( aColor );

    if ( !bTransFill )
    {
        if ( mnDrawMode & ( DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL |
                            DRAWMODE_GRAYFILL | DRAWMODE_NOFILL |
                            DRAWMODE_GHOSTEDFILL ) )
        {
            if ( mnDrawMode & DRAWMODE_BLACKFILL )
                aColor = Color( COL_BLACK );
            else if ( mnDrawMode & DRAWMODE_WHITEFILL )
                aColor = Color( COL_WHITE );
            else if ( mnDrawMode & DRAWMODE_GRAYFILL )
            {
                const UINT8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if ( mnDrawMode & DRAWMODE_NOFILL )
            {
                aColor = Color( COL_TRANSPARENT );
                bTransFill = TRUE;
            }
            if ( !bTransFill && (mnDrawMode & DRAWMODE_GHOSTEDFILL) )
            {
                aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                                (aColor.GetGreen() >> 1) | 0x80,
                                (aColor.GetBlue() >> 1) | 0x80 );
            }
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( aColor, TRUE ) );

    if ( maFont.GetFillColor() != aColor )
        maFont.SetFillColor( aColor );
    if ( maFont.IsTransparent() != bTransFill )
        maFont.SetTransparent( bTransFill );
}

// -----------------------------------------------------------------------

Color OutputDevice::GetTextFillColor() const
{
    if ( maFont.IsTransparent() )
        return Color( COL_TRANSPARENT );
    else
        return maFont.GetFillColor();
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextLineColor()
{
    DBG_TRACE( "OutputDevice::SetTextLineColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( Color(), FALSE ) );

    maTextLineColor = Color( COL_TRANSPARENT );
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextLineColor( const Color& rColor )
{
    DBG_TRACE( "OutputDevice::SetTextLineColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );

    if ( mnDrawMode & ( DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT |
                        DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT ) )
    {
        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const UINT8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }

        if ( mnDrawMode & DRAWMODE_GHOSTEDTEXT )
        {
            aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                            (aColor.GetGreen() >> 1) | 0x80,
                            (aColor.GetBlue() >> 1) | 0x80 );
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( aColor, TRUE ) );

    maTextLineColor = aColor;
}

// -----------------------------------------------------------------------

void OutputDevice::SetTextAlign( TextAlign eAlign )
{
    DBG_TRACE( "OutputDevice::SetTextAlign()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAlignAction( eAlign ) );

    if ( maFont.GetAlign() != eAlign )
    {
        maFont.SetAlign( eAlign );
        mbNewFont = TRUE;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::DrawTextLine( const Point& rPos, long nWidth,
                                 FontStrikeout eStrikeout,
                                 FontUnderline eUnderline,
                                 BOOL bUnderlineAbove )
{
    DBG_TRACE( "OutputDevice::DrawTextLine()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineAction( rPos, nWidth, eStrikeout, eUnderline ) );

    if ( ((eUnderline == UNDERLINE_NONE) || (eUnderline == UNDERLINE_DONTKNOW)) &&
         ((eStrikeout == STRIKEOUT_NONE) || (eStrikeout == STRIKEOUT_DONTKNOW)) )
        return;

    if ( !IsDeviceOutputNecessary() )
        return;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;
#else
    if ( !ImplGetServerGraphics() )
        return;
#endif

    if ( mbNewFont )
    {
        if ( !ImplNewFont() )
            return;
    }

    Point aPos = ImplLogicToDevicePixel( rPos );
    nWidth = ImplLogicWidthToDevicePixel( nWidth );
    aPos.X() += mnTextOffX;
    aPos.Y() += mnTextOffY;
    ImplDrawTextLine( aPos.X(), aPos.X(), aPos.Y(), nWidth, eStrikeout, eUnderline, bUnderlineAbove );
}

// ------------------------------------------------------------------------

BOOL OutputDevice::IsTextUnderlineAbove( const Font& rFont )
{
    return ImplIsUnderlineAbove( rFont );
}

// ------------------------------------------------------------------------

void OutputDevice::DrawWaveLine( const Point& rStartPos, const Point& rEndPos,
                                 USHORT nStyle )
{
    DBG_TRACE( "OutputDevice::DrawWaveLine()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

#ifndef REMOTE_APPSERVER
    if ( !IsDeviceOutputNecessary() )
        return;

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    Point   aStartPt = ImplLogicToDevicePixel( rStartPos );
    Point   aEndPt = ImplLogicToDevicePixel( rEndPos );
    long    nStartX = aStartPt.X();
    long    nStartY = aStartPt.Y();
    long    nEndX = aEndPt.X();
    long    nEndY = aEndPt.Y();
    short   nOrientation = 0;

    if ( (nStartY != nEndY) || (nStartX > nEndX) )
    {
        long nDX = nEndX - nStartX;
        double nO = atan2( -nEndY + nStartY, ((nDX == 0L) ? 0.000000001 : nDX) );
        nO /= F_PI1800;
        nOrientation = (short)nO;
        ImplRotatePos( nStartX, nStartY, nEndX, nEndY, -nOrientation );
    }

    long nWaveHeight;
    if ( nStyle == WAVE_NORMAL )
    {
        nWaveHeight = 3;
        nStartY++;
        nEndY++;
    }
    else if( nStyle == WAVE_SMALL )
    {
        nWaveHeight = 2;
        nStartY++;
        nEndY++;
    }
    else // WAVE_FLAT
        nWaveHeight = 1;

    ImplDrawWaveLine( nStartX, nStartY, nStartX, nStartY,
                      nEndX-nStartX, nWaveHeight, 1,
                      nOrientation, GetLineColor() );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();

        Point aPos1 = ImplLogicToDevicePixel( rStartPos );
        Point aPos2 = ImplLogicToDevicePixel( rEndPos );
        pGraphics->DrawWaveLine( aPos1, aPos2, nStyle );
    }
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::DrawText( const Point& rStartPt, const XubString& rStr,
                             xub_StrLen nIndex, xub_StrLen nLen )
{
    DBG_TRACE( "OutputDevice::DrawText()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAction( rStartPt, rStr, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    // String-Laenge fuer die Ermittlung der Groesse setzen
    if ( (ULONG)nLen+nIndex > rStr.Len() )
    {
        if ( nIndex < rStr.Len() )
            nLen = rStr.Len()-nIndex;
        else
            nLen = 0;
    }

    // Ist die Ausgabe leer, dann mache nichts
    if ( !nLen )
        return;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;
#else
    if ( !ImplGetServerGraphics() )
        return;
#endif

    if ( mbNewFont )
        ImplNewFont();
    if ( mbInitFont )
        ImplInitFont();
    if ( mbInitTextColor )
        ImplInitTextColor();

    Point aStartPt = ImplLogicToDevicePixel( rStartPt );

    // Pointer auf den String-Buffer setzen und um den Index korrigieren
    const sal_Unicode* pStr = rStr.GetBuffer();
    pStr += nIndex;

    if ( mbKerning )
    {
        ImplFontEntry*  pFontEntry = mpFontEntry;
        USHORT          i;

        // DX-Array berechnen
        long  nOffset = 0;
        long  aStackAry[128];
        long* pDXAry = (long*)ImplGetStackBuffer( sizeof(long)*(nLen-1), aStackAry, sizeof( aStackAry ) );
        const sal_Unicode* pTempStr = pStr;
        for ( i = 0; i < nLen-1; i++ )
        {
            nOffset += ImplGetCharWidth( *pTempStr );
            pDXAry[i] = nOffset / mpFontEntry->mnWidthFactor;
            pTempStr++;
        }
        ImplCalcKerning( pStr, nLen, pDXAry, nLen-1 );
        ImplDrawText( aStartPt.X(), aStartPt.Y(), pStr, nLen, pDXAry );
        ImplReleaseStackBuffer( pDXAry, aStackAry );
    }
    else
        ImplDrawText( aStartPt.X(), aStartPt.Y(), pStr, nLen, NULL );
}

// -----------------------------------------------------------------------

long OutputDevice::GetTextWidth( const XubString& rStr,
                                 xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_TRACE( "OutputDevice::GetTextWidth()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mbNewFont )
    {
        if ( !((OutputDevice*)this)->ImplNewFont() )
            return 0;
    }

    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nWidth  = 0;

    if ( nIndex < rStr.Len() )
    {
        // String-Laenge fuer die Ermittlung der Groesse setzen
        if ( (ULONG)nLen+nIndex > rStr.Len() )
            nLen = rStr.Len()-nIndex;

        if ( nLen )
        {
            // Also Fixed-Fonts are calculated char by char, because
            // not every Font or in every CJK Fonts all characters have
            // the same width
            const sal_Unicode*  pStr = rStr.GetBuffer();
            const sal_Unicode*  pTempStr;
            xub_StrLen          nTempLen;
            pStr += nIndex;
            pTempStr = pStr;
            nTempLen = nLen;
            while ( nTempLen )
            {
                nWidth += ImplGetCharWidth( *pTempStr );
                nTempLen--;
                pTempStr++;
            }
            nWidth /= pFontEntry->mnWidthFactor;

            if ( mbKerning )
                nWidth += ImplCalcKerning( pStr, nLen, NULL, 0 );
        }
    }

    if ( mbMap )
        nWidth = ImplDevicePixelToLogicWidth( nWidth );

    return nWidth;
}

// -----------------------------------------------------------------------

long OutputDevice::GetTextHeight() const
{
    DBG_TRACE( "OutputDevice::GetTextHeight()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mbNewFont )
    {
        if ( !((OutputDevice*)this)->ImplNewFont() )
            return 0;
    }

    long nHeight = mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent;

    if ( mbMap )
        nHeight = ImplDevicePixelToLogicHeight( nHeight );

    return nHeight;
}

// -----------------------------------------------------------------------

void OutputDevice::DrawTextArray( const Point& rStartPt, const XubString& rStr,
                                  const long* pDXAry,
                                  xub_StrLen nIndex, xub_StrLen nLen )
{
    DBG_TRACE( "OutputDevice::DrawTextArray()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextArrayAction( rStartPt, rStr, pDXAry, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    // String-Laenge fuer die Ermittlung der Groesse setzen
    if ( (ULONG)nLen+nIndex > rStr.Len() )
    {
        if ( nIndex < rStr.Len() )
            nLen = rStr.Len()-nIndex;
        else
            nLen = 0;
    }

    // Ist die Ausgabe leer, dann mache nichts
    if ( !nLen )
        return;

    // Bei keinem Pos-Array, DrawText benutzen
    if ( !pDXAry || (nLen < 2) )
    {
        // hier Aufrufen, damit keine doppelte MetaFile Aufzeichnung
        DrawText( rStartPt, rStr, nIndex, nLen );
        return;
    }

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;
#else
    if ( !ImplGetServerGraphics() )
        return;
#endif

    if ( mbNewFont )
    {
        if ( !ImplNewFont() )
            return;
    }
    if ( mbInitFont )
        ImplInitFont();
    if ( mbInitTextColor )
        ImplInitTextColor();

    // Pointer auf den String-Buffer setzen und um den Index korrigieren
    const sal_Unicode* pStr = rStr.GetBuffer();
    pStr += nIndex;

    Point aStartPt = ImplLogicToDevicePixel( rStartPt );
    if ( mbMap )
    {
        long    nLogStartX = rStartPt.X();
        long    nPixStartX = aStartPt.X();
        long    aStackAry[128];
        long*   pPixDXAry = (long*)ImplGetStackBuffer( sizeof(long)*(nLen-1), aStackAry, sizeof( aStackAry ) );
        for ( xub_StrLen i = 0; i < (nLen-1); i++ )
            pPixDXAry[i] = ImplLogicXToDevicePixel( nLogStartX+pDXAry[i] )-nPixStartX;
        ImplDrawText( aStartPt.X(), aStartPt.Y(), pStr, nLen, pPixDXAry );
        ImplReleaseStackBuffer( pPixDXAry, aStackAry );
    }
    else
        ImplDrawText( aStartPt.X(), aStartPt.Y(), pStr, nLen, pDXAry );
}

// -----------------------------------------------------------------------

long OutputDevice::GetTextArray( const UniString& rStr, long* pDXAry,
                                 xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_TRACE( "OutputDevice::GetTextArray()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( !pDXAry )
        return GetTextWidth( rStr, nIndex, nLen );

    // String-Laenge fuer die Ermittlung der Groesse setzen
    if ( (ULONG)nLen+nIndex > rStr.Len() )
    {
        if ( nIndex < rStr.Len() )
            nLen = rStr.Len()-nIndex;
        else
            nLen = 0;
    }

    if ( !nLen )
        return 0;

    if ( mbNewFont )
    {
        if ( !((OutputDevice*)this)->ImplNewFont() )
            return 0;
    }

    ImplFontEntry*      pFontEntry = mpFontEntry;
    const sal_Unicode*  pTempStr;
    const sal_Unicode*  pStr;
    long                nOffset = 0;
    xub_StrLen          i;
    pStr = rStr.GetBuffer();
    pStr += nIndex;
    pTempStr = pStr;

    // Breiten ermitteln
    for ( i = 0; i < nLen; i++ )
    {
        nOffset += ImplGetCharWidth( *pTempStr );
        pDXAry[i] = nOffset / mpFontEntry->mnWidthFactor;
        pTempStr++;
    }

    // Kerning beruecksichtigen
    if ( mbKerning )
        ImplCalcKerning( pStr, nLen, pDXAry, nLen );

    // Breite und Hoehe ermitteln
    long nWidth = pDXAry[nLen-1];

    // Wenn MapMode gesetzt, dann Werte umrechnen
    if ( mbMap )
    {
        for ( i = 0; i < nLen; i++ )
            pDXAry[i] = ImplDevicePixelToLogicWidth( pDXAry[i] );

        nWidth = ImplDevicePixelToLogicWidth( nWidth );
    }

    return nWidth;
}

// -----------------------------------------------------------------------

void OutputDevice::DrawStretchText( const Point& rStartPt, ULONG nWidth,
                                    const UniString& rStr,
                                    xub_StrLen nIndex, xub_StrLen nLen )
{
    DBG_TRACE( "OutputDevice::DrawStretchText()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaStretchTextAction( rStartPt, nWidth, rStr, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    // String-Laenge fuer die Ermittlung der Groesse setzen
    if ( (ULONG)nLen+nIndex > rStr.Len() )
    {
        if ( nIndex < rStr.Len() )
            nLen = rStr.Len()-nIndex;
        else
            nLen = 0;
    }

    // Ist die Ausgabe leer, dann mache nichts
    if ( !nLen )
        return;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;
#else
    if ( !ImplGetServerGraphics() )
        return;
#endif

    if ( mbNewFont )
    {
        if ( !ImplNewFont() )
            return;
    }
    if ( mbInitFont )
        ImplInitFont();
    if ( mbInitTextColor )
        ImplInitTextColor();

    Point aStartPt = ImplLogicToDevicePixel( rStartPt );
    nWidth = ImplLogicWidthToDevicePixel( nWidth );

    // Pointer auf den String-Buffer setzen und um den Index korrigieren
    const sal_Unicode* pStr = rStr.GetBuffer();
    pStr += nIndex;

    // Breiten-Array fuer errechnete Werte allocieren und
    // mit den Breiten der einzelnen Character fuellen lassen
    long  aStackAry[128];
    long* pDXAry = (long*)ImplGetStackBuffer( sizeof(long)*nLen, aStackAry, sizeof( aStackAry ) );
    ImplFillDXAry( pDXAry, pStr, nLen, (long)nWidth );
    ImplDrawText( aStartPt.X(), aStartPt.Y(), pStr, nLen, pDXAry );
    ImplReleaseStackBuffer( pDXAry, aStackAry );
}

// -----------------------------------------------------------------------

xub_StrLen OutputDevice::GetTextBreak( const XubString& rStr, long nTextWidth,
                                       xub_StrLen nIndex, xub_StrLen nLen,
                                       long nCharExtra ) const
{
    DBG_TRACE( "OutputDevice::GetTextBreak()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( nIndex > rStr.Len() )
        return 0;

    if ( mbNewFont )
    {
        if ( !((OutputDevice*)this)->ImplNewFont() )
            return 0;
    }

    ImplFontEntry*      pFontEntry = mpFontEntry;
    const sal_Unicode*  pStr;
    long                nCalcWidth = 0;
    xub_StrLen          nLastIndex;

    if ( mbMap )
    {
        nTextWidth = ImplLogicWidthToDevicePixel( nTextWidth*10 );
        nTextWidth *= mpFontEntry->mnWidthFactor;
        nTextWidth /= 10;
        if ( nCharExtra )
        {
            nCharExtra = ImplLogicWidthToDevicePixel( nCharExtra*10 );
            nCharExtra *= mpFontEntry->mnWidthFactor;
            nCharExtra /= 10;
        }
    }
    else
    {
        nCharExtra *= mpFontEntry->mnWidthFactor;
        nTextWidth *= mpFontEntry->mnWidthFactor;
    }

    // Letzte Index-Position ermitteln
    if ( (ULONG)nIndex+nLen > rStr.Len() )
        nLastIndex = rStr.Len();
    else
        nLastIndex = nIndex + nLen;

    pStr = rStr.GetBuffer();
    pStr += nIndex;
    while ( nIndex < nLastIndex )
    {
        nCalcWidth += ImplGetCharWidth( *pStr );

        if ( nCalcWidth > nTextWidth )
            return nIndex;

        // Kerning beruecksichtigen
        if ( mbKerning )
            nCalcWidth += ImplCalcKerning( pStr, 2, NULL, 0 )*mpFontEntry->mnWidthFactor;
        nCalcWidth += nCharExtra;

        nIndex++;
        pStr++;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

xub_StrLen OutputDevice::GetTextBreak( const XubString& rStr, long nTextWidth,
                                       sal_Unicode nExtraChar, xub_StrLen& rExtraCharPos,
                                       xub_StrLen nIndex, xub_StrLen nLen,
                                       long nCharExtra ) const
{
    DBG_TRACE( "OutputDevice::GetTextBreak()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( nIndex > rStr.Len() )
        return 0;

    if ( mbNewFont )
    {
        if ( !((OutputDevice*)this)->ImplNewFont() )
            return 0;
    }

    ImplFontEntry*      pFontEntry = mpFontEntry;
    const sal_Unicode*  pStr;
    long                nTextWidth2;
    long                nCalcWidth = 0;
    xub_StrLen          nIndex2 = STRING_LEN;
    xub_StrLen          nLastIndex;

    if ( mbMap )
    {
        nTextWidth = ImplLogicWidthToDevicePixel( nTextWidth*10 );
        nTextWidth *= mpFontEntry->mnWidthFactor;
        nTextWidth /= 10;
        if ( nCharExtra )
        {
            nCharExtra = ImplLogicWidthToDevicePixel( nCharExtra*10 );
            nCharExtra *= mpFontEntry->mnWidthFactor;
            nCharExtra /= 10;
        }
    }
    else
    {
        nCharExtra *= mpFontEntry->mnWidthFactor;
        nTextWidth *= mpFontEntry->mnWidthFactor;
    }

    // Letzte Index-Position ermitteln
    if ( (ULONG)nIndex+nLen > rStr.Len() )
        nLastIndex = rStr.Len();
    else
        nLastIndex = nIndex + nLen;

    nTextWidth2 = nTextWidth - ImplGetCharWidth( nExtraChar ) - nCharExtra;
    if( nTextWidth2 < 0 )
        nIndex2 = 0;

    pStr = rStr.GetBuffer();
    pStr += nIndex;
    while ( nIndex < nLastIndex )
    {
        nCalcWidth += ImplGetCharWidth( *pStr );

        if ( nCalcWidth > nTextWidth2 )
        {
            if ( nIndex2 == STRING_LEN )
                nIndex2 = nIndex;
        }
        if ( nCalcWidth > nTextWidth )
        {
            if ( nIndex2 == STRING_LEN )
                rExtraCharPos = nIndex;
            else
                rExtraCharPos = nIndex2;
            return nIndex;
        }

        // Kerning beruecksichtigen
        if ( mbKerning )
            nCalcWidth += ImplCalcKerning( pStr, 2, NULL, 0 )*mpFontEntry->mnWidthFactor;
        nCalcWidth += nCharExtra;

        nIndex++;
        pStr++;
    }

    rExtraCharPos = nIndex2;
    return STRING_LEN;
}

// -----------------------------------------------------------------------

void OutputDevice::GetCharWidth( sal_Unicode nFirstChar, sal_Unicode nLastChar,
                                 long* pWidthAry ) const
{
    DBG_TRACE( "OutputDevice::GetCharWidth()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_ASSERT( nFirstChar <= nLastChar, "OutputDevice::GetCharWidth(): nFirst > nLast" );

    if ( mbNewFont )
    {
        if ( !((OutputDevice*)this)->ImplNewFont() )
            return;
    }

    sal_Unicode nCharCount = nLastChar-nFirstChar+1;
    if ( mbMap )
    {
        while ( nCharCount )
        {
            *pWidthAry = ImplDevicePixelToLogicWidth( ImplGetCharWidth( nFirstChar ) ) / mpFontEntry->mnWidthFactor;
            pWidthAry++;
            nFirstChar++;
            nCharCount--;
        }
    }
    else
    {
        while ( nCharCount )
        {
            *pWidthAry = ImplGetCharWidth( nFirstChar ) / mpFontEntry->mnWidthFactor;
            pWidthAry++;
            nFirstChar++;
            nCharCount--;
        }
    }
}

// -----------------------------------------------------------------------

void OutputDevice::DrawText( const Rectangle& rRect,
                             const XubString& rStr, USHORT nStyle )
{
    DBG_TRACE( "OutputDevice::DrawText( const Rectangle& )" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextRectAction( rRect, rStr, nStyle ) );

    if ( !IsDeviceOutputNecessary() || !rStr.Len() || rRect.IsEmpty() )
        return;

    // Vorsichtshalber hier auch schon Aufrufen, da ImplDrawMnemonicLine()
    // dies nicht macht
#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;
#else
    if ( !ImplGetServerGraphics() )
        return;
#endif

    Color aOldTextColor;
    Color aOldTextFillColor;
    BOOL  bRestoreFillColor;
    if ( nStyle & TEXT_DRAW_DISABLE )
    {
        aOldTextColor = GetTextColor();
        if ( IsTextFillColor() )
        {
            bRestoreFillColor = TRUE;
            aOldTextFillColor = GetTextFillColor();
        }
        else
            bRestoreFillColor = FALSE;
        SetTextColor( GetSettings().GetStyleSettings().GetLightColor() );
        Rectangle aRect = rRect;
        aRect.Move( 1, 1 );
        DrawText( aRect, rStr, nStyle & ~TEXT_DRAW_DISABLE );
        SetTextColor( GetSettings().GetStyleSettings().GetShadowColor() );
    }

    long        nWidth          = rRect.GetWidth();
    long        nHeight         = rRect.GetHeight();

    if ( ((nWidth <= 0) || (nHeight <= 0)) && (nStyle & TEXT_DRAW_CLIP) )
        return;

    XubString   aStr            = rStr;
    Point       aPos            = rRect.TopLeft();
    long        nTextHeight     = GetTextHeight();
    TextAlign   eAlign          = GetTextAlign();
    xub_StrLen  nMnemonicPos    = STRING_NOTFOUND;

    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );

    // Mehrzeiligen Text behandeln wir anders
    if ( nStyle & TEXT_DRAW_MULTILINE )
    {
        XubString               aLastLine;
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        long                    nMaxTextWidth;
        xub_StrLen              i;
        xub_StrLen              nLines;
        xub_StrLen              nFormatLines;

        if ( nTextHeight )
        {
            nMaxTextWidth = ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle );
            nLines = (xub_StrLen)(nHeight/nTextHeight);
            nFormatLines = aMultiLineInfo.Count();
            if ( !nLines )
                nLines = 1;
            if ( nFormatLines > nLines )
            {
                if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
                {
                    // Letzte Zeile zusammenbauen und kuerzen
                    nFormatLines = nLines-1;
                    pLineInfo = aMultiLineInfo.GetLine( nFormatLines );
                    aLastLine = aStr.Copy( pLineInfo->GetIndex() );
                    aLastLine.ConvertLineEnd( LINEEND_LF );
                    // Alle LineFeed's durch Spaces ersetzen
                    xub_StrLen nLastLineLen = aLastLine.Len();
                    for ( i = 0; i < nLastLineLen; i++ )
                    {
                        if ( aLastLine.GetChar( i ) == _LF )
                            aLastLine.SetChar( i, ' ' );
                    }
                    aLastLine = GetEllipsisString( aLastLine, nWidth, nStyle );
                    nStyle &= ~(TEXT_DRAW_VCENTER | TEXT_DRAW_BOTTOM);
                    nStyle |= TEXT_DRAW_TOP;
                }
            }
            else
            {
                if ( nMaxTextWidth <= nWidth )
                    nStyle &= ~TEXT_DRAW_CLIP;
            }

            // Muss in der Hoehe geclippt werden?
            if ( nFormatLines*nTextHeight > nHeight )
                nStyle |= TEXT_DRAW_CLIP;

            // Clipping setzen
            if ( nStyle & TEXT_DRAW_CLIP )
            {
                Push( PUSH_CLIPREGION );
                IntersectClipRegion( rRect );
            }

            // Vertikales Alignment
            if ( nStyle & TEXT_DRAW_BOTTOM )
                aPos.Y() += nHeight-(nFormatLines*nTextHeight);
            else if ( nStyle & TEXT_DRAW_VCENTER )
                aPos.Y() += (nHeight-(nFormatLines*nTextHeight))/2;

            // Font Alignment
            if ( eAlign == ALIGN_BOTTOM )
                aPos.Y() += nTextHeight;
            else if ( eAlign == ALIGN_BASELINE )
                aPos.Y() += GetFontMetric().GetAscent();

            // Alle Zeilen ausgeben, bis auf die letzte
            for ( i = 0; i < nFormatLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( nStyle & TEXT_DRAW_RIGHT )
                    aPos.X() += nWidth-pLineInfo->GetWidth();
                else if ( nStyle & TEXT_DRAW_CENTER )
                    aPos.X() += (nWidth-pLineInfo->GetWidth())/2;
                xub_StrLen nIndex   = pLineInfo->GetIndex();
                xub_StrLen nLineLen = pLineInfo->GetLen();
                DrawText( aPos, aStr, nIndex, nLineLen );
                if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) )
                {
                    if ( (nMnemonicPos >= nIndex) && (nMnemonicPos < nIndex+nLineLen) )
                    {
                        long        nMnemonicX;
                        long        nMnemonicY;
                        xub_Unicode cMnemonic;
                        Point       aTempPos = LogicToPixel( aPos );
                        cMnemonic  = aStr.GetChar( nMnemonicPos );
                        nMnemonicX = mnOutOffX + aTempPos.X() + ImplLogicWidthToDevicePixel( GetTextWidth( aStr, nIndex, nMnemonicPos-nIndex ) );
                        nMnemonicY = mnOutOffY + aTempPos.Y() + ImplLogicWidthToDevicePixel( GetFontMetric().GetAscent() );
                        ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, cMnemonic );
                    }
                }
                aPos.Y() += nTextHeight;
                aPos.X() = rRect.Left();
            }

            // Gibt es noch eine letzte Zeile, dann diese linksbuendig ausgeben,
            // da die Zeile gekuerzt wurde
            if ( aLastLine.Len() )
                DrawText( aPos, aLastLine );

            // Clipping zuruecksetzen
            if ( nStyle & TEXT_DRAW_CLIP )
                Pop();
        }
    }
    else
    {
        long nTextWidth = GetTextWidth( aStr );

        // Evt. Text kuerzen
        if ( nTextWidth > nWidth )
        {
            if ( nStyle & TEXT_DRAW_ELLIPSIS )
            {
                aStr = GetEllipsisString( aStr, nWidth, nStyle );
                nStyle &= ~(TEXT_DRAW_CENTER | TEXT_DRAW_RIGHT);
                nStyle |= TEXT_DRAW_LEFT;
                nTextWidth = GetTextWidth( aStr );
            }
        }
        else
        {
            if ( nTextHeight <= nHeight )
                nStyle &= ~TEXT_DRAW_CLIP;
        }

        // Vertikales Alignment
        if ( nStyle & TEXT_DRAW_RIGHT )
            aPos.X() += nWidth-nTextWidth;
        else if ( nStyle & TEXT_DRAW_CENTER )
            aPos.X() += (nWidth-nTextWidth)/2;

        // Font Alignment
        if ( eAlign == ALIGN_BOTTOM )
            aPos.Y() += nTextHeight;
        else if ( eAlign == ALIGN_BASELINE )
            aPos.Y() += GetFontMetric().GetAscent();

        if ( nStyle & TEXT_DRAW_BOTTOM )
            aPos.Y() += nHeight-nTextHeight;
        else if ( nStyle & TEXT_DRAW_VCENTER )
            aPos.Y() += (nHeight-nTextHeight)/2;

        long        nMnemonicX;
        long        nMnemonicY;
        xub_Unicode cMnemonic;
        if ( nMnemonicPos != STRING_NOTFOUND )
        {
            Point aTempPos = LogicToPixel( aPos );
            cMnemonic  = aStr.GetChar( nMnemonicPos );
            nMnemonicX = mnOutOffX + aTempPos.X() + ImplLogicWidthToDevicePixel( GetTextWidth( aStr, 0, nMnemonicPos ) );
            nMnemonicY = mnOutOffY + aTempPos.Y() + ImplLogicWidthToDevicePixel( GetFontMetric().GetAscent() );
        }

        if ( nStyle & TEXT_DRAW_CLIP )
        {
            Push( PUSH_CLIPREGION );
            IntersectClipRegion( rRect );
            DrawText( aPos, aStr );
            if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) )
            {
                if ( nMnemonicPos != STRING_NOTFOUND )
                    ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, cMnemonic );
            }
            Pop();
        }
        else
        {
            DrawText( aPos, aStr );
            if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) )
            {
                if ( nMnemonicPos != STRING_NOTFOUND )
                    ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, cMnemonic );
            }
        }
    }

    if ( nStyle & TEXT_DRAW_DISABLE )
    {
        SetTextColor( aOldTextColor );
        if ( bRestoreFillColor )
            SetTextFillColor( aOldTextFillColor );
    }
}

// -----------------------------------------------------------------------

Rectangle OutputDevice::GetTextRect( const Rectangle& rRect,
                                     const XubString& rStr, USHORT nStyle,
                                     TextRectInfo* pInfo ) const
{
    DBG_TRACE( "OutputDevice::GetTextRect()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Rectangle           aRect = rRect;
    XubString           aStr = rStr;
    xub_StrLen          nLines;
    long                nWidth = rRect.GetWidth();
    long                nMaxWidth;
    long                nTextHeight = GetTextHeight();

    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = GetNonMnemonicString( aStr );

    if ( nStyle & TEXT_DRAW_MULTILINE )
    {
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        xub_StrLen              nFormatLines;
        xub_StrLen              i;

        nMaxWidth = 0;
        ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle );
        nFormatLines = aMultiLineInfo.Count();
        if ( !nTextHeight )
            nTextHeight = 1;
        nLines = (USHORT)(aRect.GetHeight()/nTextHeight);
        if ( pInfo )
            pInfo->mnLineCount = nFormatLines;
        if ( !nLines )
            nLines = 1;
        if ( nFormatLines <= nLines )
            nLines = nFormatLines;
        else
        {
            if ( !(nStyle & TEXT_DRAW_ENDELLIPSIS) )
                nLines = nFormatLines;
            else
            {
                if ( pInfo )
                    pInfo->mbEllipsis = TRUE;
                nMaxWidth = nWidth;
            }
        }
        if ( pInfo )
        {
            BOOL bMaxWidth = nMaxWidth == 0;
            pInfo->mnMaxWidth = 0;
            for ( i = 0; i < nLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( bMaxWidth && (pLineInfo->GetWidth() > nMaxWidth) )
                    nMaxWidth = pLineInfo->GetWidth();
                if ( pLineInfo->GetWidth() > pInfo->mnMaxWidth )
                    pInfo->mnMaxWidth = pLineInfo->GetWidth();
            }
        }
        else if ( !nMaxWidth )
        {
            for ( i = 0; i < nLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( pLineInfo->GetWidth() > nMaxWidth )
                    nMaxWidth = pLineInfo->GetWidth();
            }
        }
    }
    else
    {
        nLines      = 1;
        nMaxWidth   = GetTextWidth( aStr );

        if ( pInfo )
        {
            pInfo->mnLineCount  = 1;
            pInfo->mnMaxWidth   = nMaxWidth;
        }

        if ( (nMaxWidth > nWidth) && (nStyle & TEXT_DRAW_ELLIPSIS) )
        {
            if ( pInfo )
                pInfo->mbEllipsis = TRUE;
            nMaxWidth = nWidth;
        }
    }

    if ( nStyle & TEXT_DRAW_RIGHT )
        aRect.Left() = aRect.Right()-nMaxWidth+1;
    else if ( nStyle & TEXT_DRAW_CENTER )
    {
        aRect.Left() += (nWidth-nMaxWidth)/2;
        aRect.Right() = aRect.Left()+nMaxWidth-1;
    }
    else
        aRect.Right() = aRect.Left()+nMaxWidth-1;

    if ( nStyle & TEXT_DRAW_BOTTOM )
        aRect.Top() = aRect.Bottom()-(nTextHeight*nLines)+1;
    else if ( nStyle & TEXT_DRAW_VCENTER )
    {
        aRect.Top()   += (aRect.GetHeight()-(nTextHeight*nLines))/2;
        aRect.Bottom() = aRect.Top()+(nTextHeight*nLines)-1;
    }
    else
        aRect.Bottom() = aRect.Top()+(nTextHeight*nLines)-1;

    return aRect;
}

// -----------------------------------------------------------------------

static BOOL ImplIsCharIn( xub_Unicode c, const sal_Char* pStr )
{
    while ( *pStr )
    {
        if ( *pStr == c )
            return TRUE;
        pStr++;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

XubString OutputDevice::GetEllipsisString( const XubString& rStr, long nMaxWidth,
                                           USHORT nStyle ) const
{
    DBG_TRACE( "OutputDevice::GetEllipsisString()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    XubString   aStr = rStr;
    xub_StrLen  nIndex = GetTextBreak( aStr, nMaxWidth );

    if ( nIndex != STRING_LEN )
    {
        if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
        {
            aStr.Erase( nIndex );
            if ( nIndex > 1 )
            {
                aStr.AppendAscii( "..." );
                while ( aStr.Len() && (GetTextWidth( aStr ) > nMaxWidth) )
                {
                    if ( (nIndex > 1) || (nIndex == aStr.Len()) )
                        nIndex--;
                    aStr.Erase( nIndex, 1 );
                }
            }

            if ( !aStr.Len() && (nStyle & TEXT_DRAW_CLIP) )
                aStr += rStr.GetChar( 0 );
        }
        else if ( nStyle & (TEXT_DRAW_PATHELLIPSIS | TEXT_DRAW_NEWSELLIPSIS) )
        {
            static sal_Char const   aPathSepChars[] = "\\/:";
            static sal_Char const   aNewsSepChars[] = ".";
            const sal_Char*         pSepChars;

            if ( nStyle & TEXT_DRAW_PATHELLIPSIS )
                pSepChars = aPathSepChars;
            else
                pSepChars = aNewsSepChars;

            // Letztes Teilstueck ermitteln
            xub_StrLen nLastContent = rStr.Len();
            while ( nLastContent )
            {
                nLastContent--;
                if ( ImplIsCharIn( rStr.GetChar( nLastContent ), pSepChars ) )
                    break;
            }
            while ( nLastContent &&
                    ImplIsCharIn( rStr.GetChar( nLastContent-1 ), pSepChars ) )
                nLastContent--;

            XubString aLastStr( rStr, nLastContent, rStr.Len() );
            XubString aTempLastStr( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
            aTempLastStr += aLastStr;
            if ( GetTextWidth( aTempLastStr ) > nMaxWidth )
                aStr = GetEllipsisString( rStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS );
            else
            {
                USHORT nFirstContent = 0;
                while ( nFirstContent < nLastContent )
                {
                    nFirstContent++;
                    if ( ImplIsCharIn( rStr.GetChar( nFirstContent ), pSepChars ) )
                        break;
                }
                while ( (nFirstContent < nLastContent) &&
                        ImplIsCharIn( rStr.GetChar( nFirstContent ), pSepChars ) )
                    nFirstContent++;

                if ( nFirstContent >= nLastContent )
                    aStr = GetEllipsisString( rStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS );
                else
                {
                    if ( nFirstContent > 4 )
                        nFirstContent = 4;
                    XubString aFirstStr( rStr, 0, nFirstContent );
                    aFirstStr.AppendAscii( "..." );
                    XubString aTempStr = aFirstStr;
                    aTempStr += aLastStr;
                    if ( GetTextWidth( aTempStr ) > nMaxWidth )
                        aStr = GetEllipsisString( rStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS );
                    else
                    {
                        do
                        {
                            aStr = aTempStr;
                            while ( nFirstContent < nLastContent )
                            {
                                nLastContent--;
                                if ( ImplIsCharIn( rStr.GetChar( nLastContent ), pSepChars ) )
                                    break;
                            }
                            while ( (nFirstContent < nLastContent) &&
                                    ImplIsCharIn( rStr.GetChar( nLastContent-1 ), pSepChars ) )
                                nLastContent--;

                            if ( nFirstContent < nLastContent )
                            {
                                XubString aTempLastStr( rStr, nLastContent, rStr.Len() );
                                aTempStr = aFirstStr;
                                aTempStr += aTempLastStr;
                                if ( GetTextWidth( aTempStr ) > nMaxWidth )
                                    break;
                            }
                        }
                        while ( nFirstContent < nLastContent );
                    }
                }
            }
        }
    }

    return aStr;
}

// -----------------------------------------------------------------------

void OutputDevice::DrawCtrlText( const Point& rPos, const XubString& rStr,
                                 xub_StrLen nIndex, xub_StrLen nLen,
                                 USHORT nStyle )
{
    DBG_TRACE( "OutputDevice::DrawCtrlText()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( !IsDeviceOutputNecessary() || (nIndex >= rStr.Len()) )
        return;

    // Vorsichtshalber hier auch schon Aufrufen, da ImplDrawMnemonicLine()
    // dies nicht macht
#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;
#else
    if ( !ImplGetServerGraphics() )
        return;
#endif

    XubString   aStr = rStr;
    xub_StrLen  nMnemonicPos = STRING_NOTFOUND;
    long        nMnemonicX;
    long        nMnemonicY;
    xub_Unicode cMnemonic;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
    {
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );
        if ( nMnemonicPos != STRING_NOTFOUND )
        {
            if ( nMnemonicPos < nIndex )
                nIndex--;
            else if ( (nLen < STRING_LEN) &&
                      (nMnemonicPos >= nIndex) && (nMnemonicPos < (ULONG)(nIndex+nLen)) )
                nLen--;
            Point aTempPos = LogicToPixel( rPos );
            cMnemonic  = aStr.GetChar( nMnemonicPos );
            nMnemonicX = mnOutOffX + aTempPos.X() + ImplLogicWidthToDevicePixel( GetTextWidth( aStr, 0, nMnemonicPos ) );
            nMnemonicY = mnOutOffY + aTempPos.Y() + ImplLogicWidthToDevicePixel( GetFontMetric().GetAscent() );
        }
    }

    if ( nStyle & TEXT_DRAW_DISABLE )
    {
        Color aOldTextColor;
        Color aOldTextFillColor;
        BOOL  bRestoreFillColor;
        aOldTextColor = GetTextColor();
        if ( IsTextFillColor() )
        {
            bRestoreFillColor = TRUE;
            aOldTextFillColor = GetTextFillColor();
        }
        else
            bRestoreFillColor = FALSE;
        SetTextColor( GetSettings().GetStyleSettings().GetLightColor() );
        DrawText( Point( rPos.X()+1, rPos.Y()+1 ), aStr, nIndex, nLen );
        if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) )
        {
            if ( nMnemonicPos != STRING_NOTFOUND )
                ImplDrawMnemonicLine( nMnemonicX+1, nMnemonicY+1, cMnemonic );
        }
        SetTextColor( GetSettings().GetStyleSettings().GetShadowColor() );
        DrawText( rPos, aStr, nIndex, nLen );
        if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) )
        {
            if ( nMnemonicPos != STRING_NOTFOUND )
                ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, cMnemonic );
        }
        SetTextColor( aOldTextColor );
        if ( bRestoreFillColor )
            SetTextFillColor( aOldTextFillColor );
    }
    else
    {
        DrawText( rPos, aStr, nIndex, nLen );
        if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) )
        {
            if ( nMnemonicPos != STRING_NOTFOUND )
                ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, cMnemonic );
        }
    }
}

// -----------------------------------------------------------------------

long OutputDevice::GetCtrlTextWidth( const XubString& rStr,
                                     xub_StrLen nIndex, xub_StrLen nLen,
                                     USHORT nStyle ) const
{
    DBG_TRACE( "OutputDevice::GetCtrlTextSize()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( nStyle & TEXT_DRAW_MNEMONIC )
    {
        xub_StrLen  nMnemonicPos;
        XubString   aStr = GetNonMnemonicString( rStr, nMnemonicPos );
        if ( nMnemonicPos != STRING_NOTFOUND )
        {
            if ( nMnemonicPos < nIndex )
                nIndex--;
            else if ( (nLen < STRING_LEN) &&
                      (nMnemonicPos >= nIndex) && (nMnemonicPos < (ULONG)(nIndex+nLen)) )
                nLen--;
        }
        return GetTextWidth( aStr, nIndex, nLen );
    }
    else
        return GetTextWidth( rStr, nIndex, nLen );
}

// -----------------------------------------------------------------------

XubString OutputDevice::GetNonMnemonicString( const XubString& rStr, xub_StrLen& rMnemonicPos )
{
    XubString   aStr    = rStr;
    xub_StrLen  nLen    = aStr.Len();
    xub_StrLen  i       = 0;

    rMnemonicPos = STRING_NOTFOUND;
    while ( i < nLen )
    {
        if ( aStr.GetChar( i ) == '~' )
        {
            if ( aStr.GetChar( i+1 ) != '~' )
            {
                if ( rMnemonicPos == STRING_NOTFOUND )
                    rMnemonicPos = i;
                aStr.Erase( i, 1 );
                nLen--;
            }
            else
            {
                aStr.Erase( i, 1 );
                nLen--;
                i++;
            }
        }
        else
            i++;
    }

    return aStr;
}

// -----------------------------------------------------------------------

USHORT OutputDevice::GetDevFontCount() const
{
    DBG_TRACE( "OutputDevice::GetDevFontCount()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    // Wenn wir schon eine Liste der Fonts haben, dann nicht iterieren
    if ( mpGetDevFontList )
        return (USHORT)mpGetDevFontList->Count();

    ((OutputDevice*)this)->mpGetDevFontList = new ImplGetDevFontList;

    // Fill Fontlist
    ImplDevFontListData* pFontListData = mpFontList->First();
    while ( pFontListData )
    {
        ImplFontData*   pLastData = NULL;
        ImplFontData*   pData = pFontListData->mpFirst;
        while ( pData )
        {
            // Compare with the last font, because we wan't in the list
            // only fonts, that have different attributes, but not
            // different sizes
            if ( !pLastData ||
                 (ImplCompareFontDataWithoutSize( pLastData, pData ) != 0) )
                mpGetDevFontList->Add( pData );

            pLastData = pData;
            pData = pData->mpNext;
        }

        pFontListData = mpFontList->Next();
    }

    return (USHORT)mpGetDevFontList->Count();
}

// -----------------------------------------------------------------------

FontInfo OutputDevice::GetDevFont( USHORT nDevFont ) const
{
    DBG_TRACE( "OutputDevice::GetDevFont()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    FontInfo    aFontInfo;
    USHORT      nCount = GetDevFontCount();

    // Wertebereich ueberpruefen
    if ( nDevFont < nCount )
    {
        ImplFontData* pData = mpGetDevFontList->Get( nDevFont );
        aFontInfo.SetName( pData->maName );
        aFontInfo.SetStyleName( pData->maStyleName );
        aFontInfo.SetCharSet( pData->meCharSet );
        aFontInfo.SetFamily( pData->meFamily );
        aFontInfo.SetPitch( pData->mePitch );
        aFontInfo.SetWeight( pData->meWeight );
        aFontInfo.SetItalic( pData->meItalic );
        aFontInfo.mpImplMetric->meType = pData->meType;
        aFontInfo.mpImplMetric->mbDevice = pData->mbDevice;
    }

    return aFontInfo;
}

// -----------------------------------------------------------------------

USHORT OutputDevice::GetDevFontSizeCount( const Font& rFont ) const
{
    DBG_TRACE( "OutputDevice::GetDevFontSizeCount()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    XubString aFontName = rFont.GetName();

    // Wenn die Liste schon existiert und der FontName sich nicht
    // unterscheidet, dann brauchen wir Sie nicht neu erzeugen
    if ( mpGetDevSizeList )
    {
        if ( mpGetDevSizeList->GetFontName() == aFontName )
            return (USHORT)mpGetDevSizeList->Count();
        else
        {
            mpGetDevSizeList->Clear();
            mpGetDevSizeList->SetFontName( aFontName );
        }
    }
    else
        ((OutputDevice*)this)->mpGetDevSizeList = new ImplGetDevSizeList( aFontName );

    // Fonts aus unserer Fontliste in die GetDevFontSizeListe eintragen
    ImplDevFontListData* pFontListData = mpFontList->FindFont( aFontName );
    if ( pFontListData )
    {
        ImplFontData* pData = pFontListData->mpFirst;
        do
        {
            mpGetDevSizeList->Add( pData->mnHeight );
            pData = pData->mpNext;
        }
        while ( pData );
    }

    return (USHORT)mpGetDevSizeList->Count();
}

// -----------------------------------------------------------------------

Size OutputDevice::GetDevFontSize( const Font& rFont, USHORT nSize ) const
{
    DBG_TRACE( "OutputDevice::GetDevFontSize()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    USHORT nCount = GetDevFontSizeCount( rFont );

    // Wertebereich ueberpruefen
    if ( nSize >= nCount )
        return Size();

    // Wenn MapMode gesetzt ist, wird auf ,5-Points gerundet
    Size aSize( 0, mpGetDevSizeList->Get( nSize ) );
    if ( mbMap )
    {
        aSize.Height() *= 10;
        MapMode aMap( MAP_10TH_INCH, Point(), Fraction( 1, 72 ), Fraction( 1, 72 ) );
        aSize = PixelToLogic( aSize, aMap );
        aSize.Height() += 5;
        aSize.Height() /= 10;
        long nRound = aSize.Height() % 5;
        if ( nRound >= 3 )
            aSize.Height() += (5-nRound);
        else
            aSize.Height() -= nRound;
        aSize.Height() *= 10;
        aSize = LogicToPixel( aSize, aMap );
        aSize = PixelToLogic( aSize );
        aSize.Height() += 5;
        aSize.Height() /= 10;
    }
    return aSize;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::IsFontAvailable( const XubString& rFontName ) const
{
    DBG_TRACE( "OutputDevice::IsFontAvailable()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    return (mpFontList->FindFont( rFontName ) != 0);
}

// -----------------------------------------------------------------------

FontMetric OutputDevice::GetFontMetric() const
{
    DBG_TRACE( "OutputDevice::GetFontMetric()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    FontMetric aMetric;

    if ( mbNewFont )
    {
        if ( !((OutputDevice*)this)->ImplNewFont() )
            return aMetric;
    }

    ImplFontEntry*      pEntry = mpFontEntry;
    ImplFontMetricData* pMetric = &(pEntry->maMetric);

    // Mappen und StarView Struktur fuellen
    aMetric.Font::operator=( maFont );

    // Fontdaten ermitteln und setzen
    aMetric.SetName( pMetric->maName );
    aMetric.SetStyleName( pMetric->maStyleName );
    aMetric.SetSize( PixelToLogic( Size( pMetric->mnWidth, pMetric->mnAscent+pMetric->mnDescent-pMetric->mnLeading ) ) );
    aMetric.SetCharSet( pMetric->meCharSet );
    aMetric.SetFamily( pMetric->meFamily );
    aMetric.SetPitch( pMetric->mePitch );
    aMetric.SetWeight( pMetric->meWeight );
    aMetric.SetItalic( pMetric->meItalic );
    if ( pEntry->mnOwnOrientation )
        aMetric.SetOrientation( pEntry->mnOwnOrientation );
    else
        aMetric.SetOrientation( pMetric->mnOrientation );
    if ( !pEntry->mnKernPairs )
        aMetric.SetKerning( aMetric.GetKerning() & ~KERNING_FONTSPECIFIC );

    // restliche Metricen setzen
    aMetric.mpImplMetric->meType        = pMetric->meType;
    aMetric.mpImplMetric->mbDevice      = pMetric->mbDevice;
    aMetric.mpImplMetric->mnAscent      = ImplDevicePixelToLogicHeight( pMetric->mnAscent+mnEmphasisAscent );
    aMetric.mpImplMetric->mnDescent     = ImplDevicePixelToLogicHeight( pMetric->mnDescent+mnEmphasisDescent );
    aMetric.mpImplMetric->mnLeading     = ImplDevicePixelToLogicHeight( pMetric->mnLeading+mnEmphasisAscent );
    aMetric.mpImplMetric->mnLineHeight  = ImplDevicePixelToLogicHeight( pMetric->mnAscent+pMetric->mnDescent+mnEmphasisAscent+mnEmphasisDescent );
    aMetric.mpImplMetric->mnSlant       = ImplDevicePixelToLogicHeight( pMetric->mnSlant );
    aMetric.mpImplMetric->mnFirstChar   = pMetric->mnFirstChar;
    aMetric.mpImplMetric->mnLastChar    = pMetric->mnLastChar;

    return aMetric;
}

// -----------------------------------------------------------------------

FontMetric OutputDevice::GetFontMetric( const Font& rFont ) const
{
    // Uebergebenen Font selektieren, Metric abfragen und alten wieder
    // selektieren
    Font aOldFont = GetFont();
    ((OutputDevice*)this)->SetFont( rFont );
    FontMetric aMetric( GetFontMetric() );
    ((OutputDevice*)this)->SetFont( aOldFont );
    return aMetric;
}

// -----------------------------------------------------------------------

ULONG OutputDevice::GetKerningPairCount() const
{
    DBG_TRACE( "OutputDevice::GetKerningPairCount()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    ((OutputDevice*)this)->ImplInitKerningPairs();
    return mpFontEntry->mnKernPairs;
}

// -----------------------------------------------------------------------

void OutputDevice::GetKerningPairs( ULONG nPairs, KerningPair* pKernPairs ) const
{
    DBG_TRACE( "OutputDevice::GetKerningPairs()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    ((OutputDevice*)this)->ImplInitKerningPairs();
    if ( nPairs > mpFontEntry->mnKernPairs )
        nPairs = mpFontEntry->mnKernPairs;
    if ( nPairs )
        memcpy( pKernPairs, mpFontEntry->mpKernPairs, nPairs*sizeof( KerningPair ) );
}

// -----------------------------------------------------------------------

BOOL OutputDevice::GetGlyphBoundRect( xub_Unicode cChar, Rectangle& rRect, BOOL bOptimize )
{
    DBG_TRACE( "OutputDevice::GetGlyphBoundRect()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    BOOL bRet = FALSE;

#ifdef NEW_GLYPH
    if( GetFont().GetOrientation() )
    {
        PolyPolygon aPolyPoly;

        if( GetGlyphOutline( cChar, aPolyPoly, bOptimize ) )
        {
            rRect = aPolyPoly.GetBoundRect();
            bRet = TRUE;
        }

        return bRet;
    }
#endif // NEW_GLYPH

#ifndef REMOTE_APPSERVER

    if ( mpGraphics || ImplGetGraphics() )
    {
        Font    aOldFont( GetFont() );
        Font    aFont( aOldFont );
        long    nLeft, nTop, nWidth, nHeight;
        long    nFontWidth, nFontHeight;
        long    nOrgWidth, nOrgHeight;

        if ( bOptimize )
        {
            Size aFontSize( LogicToPixel( aFont.GetSize() ) );

            if ( aFontSize.Width() && aFontSize.Height() )
            {
                const double fFactor = (double) aFontSize.Width() / aFontSize.Height();

                if ( fFactor < 1.0 )
                {
                    aFontSize.Width() = FRound( fFactor * 500. );
                    aFontSize.Height() = 500;
                }
                else
                {
                    aFontSize.Width() = 500;
                    aFontSize.Height() = FRound( 500. / fFactor );
                }

                aFont.SetSize( PixelToLogic( aFontSize ) );
                ((OutputDevice*)this)->SetFont( aFont );
                nFontWidth = aFont.GetSize().Width();
                nFontHeight = aFont.GetSize().Height();
                nOrgWidth = aOldFont.GetSize().Width();
                nOrgHeight = aOldFont.GetSize().Height();
            }
            else
            {
                aFont.SetSize( PixelToLogic( Size( 0, 500 ) ) );
                nFontWidth = nFontHeight = aFont.GetSize().Height();
                nOrgWidth = nOrgHeight = aOldFont.GetSize().Height();
            }
        }

        ((OutputDevice*)this)->SetFont( aFont );

        if ( mbNewFont )
            ImplNewFont();

        if ( mbInitFont )
            ImplInitFont();

        if ( mpGraphics->GetGlyphBoundRect( cChar, &nLeft, &nTop, &nWidth, &nHeight ) )
        {
            if ( bOptimize )
            {
                nLeft = ImplDevicePixelToLogicWidth( nLeft ) * nOrgWidth / nFontWidth;
                nTop = ImplDevicePixelToLogicHeight( nTop ) * nOrgHeight / nFontHeight;
                nWidth = ImplDevicePixelToLogicWidth( nWidth ) * nOrgWidth / nFontWidth;
                nHeight = ImplDevicePixelToLogicHeight( nHeight ) * nOrgHeight / nFontHeight;
            }
            else
            {
                nLeft = ImplDevicePixelToLogicWidth( nLeft );
                nTop = ImplDevicePixelToLogicHeight( nTop );
                nWidth = ImplDevicePixelToLogicWidth( nWidth );
                nHeight = ImplDevicePixelToLogicHeight( nHeight );
            }

            rRect = Rectangle( Point( nLeft, nTop ), Size( nWidth, nHeight ) );
            bRet = TRUE;
        }

        ((OutputDevice*)this)->SetFont( aOldFont );
    }
#else
    if ( mbNewFont )
        ImplNewFont();
    if ( mbInitFont )
        ImplInitFont();

    bRet = mpGraphics->GetGlyphBoundRect( cChar, rRect, bOptimize );

    if ( bRet )
    {
        rRect = Rectangle( Point( ImplDevicePixelToLogicWidth( rRect.Left() ),
                                  ImplDevicePixelToLogicHeight( rRect.Top() ) ),
                           Size( ImplDevicePixelToLogicWidth( rRect.GetWidth() ),
                                 ImplDevicePixelToLogicHeight( rRect.GetHeight() ) ) );
    }
#endif

    if ( !bRet && (OUTDEV_PRINTER != meOutDevType) )
    {
        if ( bOptimize )
        {
            if ( mbNewFont )
                ImplNewFont();

            if ( mbInitFont )
                ImplInitFont();
        }

        VirtualDevice*  pVDev = new VirtualDevice( 1 );
        long            nWidth = ImplGetTextWidth( &cChar, 1, NULL );
        long            nHeight = mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent;
        Point           aOffset( nWidth >> 1, 8 );
        Size            aSize( nWidth + ( aOffset.X() << 1 ), nHeight + ( aOffset.Y() << 1 ) );

        if ( pVDev->SetOutputSizePixel( aSize ) )
        {
            Font    aFont( GetFont() );
            Bitmap  aBmp;

            aFont.SetShadow( FALSE );
            aFont.SetOutline( FALSE );
            aFont.SetRelief( RELIEF_NONE );
            aFont.SetOrientation( 0 );
            aFont.SetSize( Size( mpFontEntry->maFontSelData.mnWidth, mpFontEntry->maFontSelData.mnHeight ) );

            pVDev->SetFont( aFont );
            pVDev->SetTextAlign( ALIGN_TOP );
            pVDev->SetTextColor( Color( COL_BLACK ) );
            pVDev->SetTextFillColor();
            pVDev->ImplNewFont();
            pVDev->ImplInitFont();
            pVDev->ImplInitTextColor();
            pVDev->ImplDrawText( aOffset.X(), aOffset.Y(), &cChar, 1, NULL );
            aBmp = pVDev->GetBitmap( Point(), aSize );
            delete pVDev;

            BitmapReadAccess* pAcc = aBmp.AcquireReadAccess();

            if ( pAcc )
            {
                const long          nW = pAcc->Width();
                const long          nW1 = nW - 1L;
                const long          nH = pAcc->Height();
                long                nLeft, nTop, nRight, nBottom;
                const BitmapColor   aBlack( pAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
                BOOL                bLineDone;

                nLeft = nW;
                nTop = nH;
                nRight = nBottom = -1L;

                for( long nY = 0L; nY < nH; nY++ )
                {
                    bLineDone = FALSE;

                    for( long nX = 0L; ( nX < nW ) && !bLineDone; nX++ )
                    {
                        if( pAcc->GetPixel( nY, nX ) == aBlack )
                        {
                            // find y minimum
                            if( nY < nTop )
                                nTop = nY;

                            // find y maximum
                            if( nY > nBottom )
                                nBottom = nY;

                            // find x minimum
                            if( nX < nLeft )
                                nLeft = nX;

                            // find x maximum (last pixel in line)
                            for( long nX2 = nW1; nX2 >= nX; nX2-- )
                            {
                                if( pAcc->GetPixel( nY, nX2 ) == aBlack )
                                {
                                    if( nX2 > nRight )
                                        nRight = nX2;

                                    bLineDone = TRUE;
                                    break;
                                }
                            }
                        }
                    }
                }

                if( nLeft < nW && nTop < nH && nRight > -1L && nBottom > -1L )
                {
                    nLeft -= aOffset.X(), nTop -= aOffset.Y();
                    nRight -= aOffset.X(), nBottom -= aOffset.Y();

                    nWidth = ImplDevicePixelToLogicWidth( nRight - nLeft + 1L );
                    nHeight = ImplDevicePixelToLogicHeight( nBottom - nTop + 1L );
                    nLeft = ImplDevicePixelToLogicWidth( nLeft );
                    nTop = ImplDevicePixelToLogicHeight( nTop );
                    rRect = Rectangle( Point( nLeft, nTop ), Size( nWidth, nHeight ) );
                    bRet = TRUE;
                }

                aBmp.ReleaseAccess( pAcc );
            }
        }
        else
            delete pVDev;
    }

    if ( !bRet )
        rRect.SetEmpty();

    return bRet;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::GetGlyphOutline( xub_Unicode cChar, PolyPolygon& rPolyPoly, BOOL bOptimize )
{
    DBG_TRACE( "OutputDevice::GetGlyphOutline()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    BOOL bRet = FALSE;

#ifndef REMOTE_APPSERVER
   if ( mpGraphics || ImplGetGraphics() )
   {
        Font        aOldFont( GetFont() );
        Font        aFont( aOldFont );
        USHORT*     pPolySizes = NULL;
        SalPoint*   pPoints = NULL;
        BYTE*       pFlags = NULL;
        long        nFontWidth, nFontHeight;
        long        nOrgWidth, nOrgHeight;
        ULONG       nPolyCount;

#ifdef NEW_GLYPH
        aFont.SetOrientation( 0 );
#endif // NEW_GLYPH

        if( bOptimize )
        {
            Size aFontSize( LogicToPixel( aFont.GetSize() ) );

            if( aFontSize.Width() && aFontSize.Height() )
            {
                const double fFactor = (double) aFontSize.Width() / aFontSize.Height();

                if ( fFactor < 1.0 )
                {
                    aFontSize.Width() = FRound( fFactor * 500. );
                    aFontSize.Height() = 500;
                }
                else
                {
                    aFontSize.Width() = 500;
                    aFontSize.Height() = FRound( 500. / fFactor );
                }

                aFont.SetSize( PixelToLogic( aFontSize ) );
                nFontWidth = aFont.GetSize().Width();
                nFontHeight = aFont.GetSize().Height();
                nOrgWidth = aOldFont.GetSize().Width();
                nOrgHeight = aOldFont.GetSize().Height();
            }
            else
            {
                aFont.SetSize( PixelToLogic( Size( 0, 500 ) ) );
                nFontWidth = nFontHeight = aFont.GetSize().Height();
                nOrgWidth = nOrgHeight = aOldFont.GetSize().Height();
            }
        }

        ((OutputDevice*)this)->SetFont( aFont );

        if ( mbNewFont )
            ImplNewFont();

        if ( mbInitFont )
            ImplInitFont();

        nPolyCount = mpGraphics->GetGlyphOutline( cChar, &pPolySizes, &pPoints, &pFlags );
        if ( nPolyCount && pPolySizes && pPoints && pFlags )
        {
            ULONG nTotalPos = 0UL;

            rPolyPoly.Clear();

            for( ULONG i = 0UL; i < nPolyCount; i++ )
            {
                const USHORT nSize = pPolySizes[ i ];

                if( nSize )
                {
                    Polygon aPoly( nSize );
                    Point*  pPt = aPoly.ImplGetPointAry();
                    BYTE*   pFl = aPoly.ImplGetFlagAry();

                    memcpy( pFl, pFlags + nTotalPos, nSize );

                    for( USHORT n = 0; n < nSize; n++ )
                    {
                        const SalPoint& rSalPt = pPoints[ nTotalPos++ ];
                        Point&          rPt = pPt[ n ];

                        if( bOptimize )
                        {
                            rPt.X() = ImplDevicePixelToLogicWidth( rSalPt.mnX ) *
                                      nOrgWidth / nFontWidth;
                            rPt.Y() = ImplDevicePixelToLogicHeight( rSalPt.mnY ) *
                                      nOrgHeight / nFontHeight;
                        }
                        else
                        {
                            rPt.X() = ImplDevicePixelToLogicWidth( rSalPt.mnX );
                            rPt.Y() = ImplDevicePixelToLogicHeight( rSalPt.mnY );
                        }
                    }

                    rPolyPoly.Insert( aPoly );
                }
            }

            bRet = TRUE;
        }

        delete[] pPolySizes;
        delete[] pPoints;
        delete[] pFlags;

        ((OutputDevice*)this)->SetFont( aOldFont );
    }
#else
    if ( mbNewFont )
        ImplNewFont();
    if ( mbInitFont )
        ImplInitFont();

    bRet = mpGraphics->GetGlyphOutline( cChar, rPolyPoly, bOptimize );

    if ( bRet )
    {
        for( USHORT i = 0UL, nCount = rPolyPoly.Count(); i < nCount; i++ )
        {
            Polygon& rPoly = rPolyPoly[i];

            for( USHORT n = 0, nSize = rPoly.GetSize(); n < nSize; n++ )
            {
                Point& rPt = rPoly[ n ];
                rPt.X() = ImplDevicePixelToLogicWidth( rPt.X() );
                rPt.Y() = ImplDevicePixelToLogicHeight( rPt.Y() );
            }
        }
    }
#endif

    if ( !bRet && (OUTDEV_PRINTER != meOutDevType) )
    {
        if ( bOptimize )
        {
            if ( mbNewFont )
                ImplNewFont();
            if ( mbInitFont )
                ImplInitFont();
        }

        Font            aFont( GetFont() );
        VirtualDevice*  pVDev = new VirtualDevice( 1 );
        const Size      aFontSize( pVDev->LogicToPixel( Size( 0, GLYPH_FONT_HEIGHT ), MAP_POINT ) );
        const long      nOrgWidth = ImplGetTextWidth( &cChar, 1, NULL );
        const long      nOrgHeight = mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent;

        aFont.SetShadow( FALSE );
        aFont.SetOutline( FALSE );
        aFont.SetRelief( RELIEF_NONE );
        aFont.SetOrientation( 0 );
        aFont.SetSize( aFontSize );
        pVDev->SetFont( aFont );
        pVDev->SetTextAlign( ALIGN_TOP );
        pVDev->SetTextColor( Color( COL_BLACK ) );
        pVDev->SetTextFillColor();
        pVDev->ImplNewFont();
        pVDev->ImplInitFont();
        pVDev->ImplInitTextColor();

        const long      nWidth = pVDev->ImplGetTextWidth( &cChar, 1, NULL );
        const long      nHeight = pVDev->mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent;
        const Point     aOffset( nWidth >> 1, 8 );
        const Size      aSize( nWidth + ( aOffset.X() << 1 ), nHeight + ( aOffset.Y() << 1 ) );
        const double    fScaleX = ( nOrgWidth && nWidth )  ? ( (double) nOrgWidth / nWidth ) : 0.0;
        const double    fScaleY = ( nOrgHeight && nHeight ) ?  ( (double) nOrgHeight / nHeight ) : 0.0;

        if ( pVDev->SetOutputSizePixel( aSize ) )
        {
            Bitmap  aBmp;

            pVDev->ImplDrawText( aOffset.X(), aOffset.Y(), &cChar, 1, NULL );
            aBmp = pVDev->GetBitmap( Point(), aSize );
            delete pVDev;

            if( aBmp.Vectorize( rPolyPoly, BMP_VECTORIZE_OUTER | BMP_VECTORIZE_REDUCE_EDGES ) )
            {
                const long nOffX = aOffset.X(), nOffY = aOffset.Y();

                for( USHORT i = 0UL, nCount = rPolyPoly.Count(); i < nCount; i++ )
                {
                    Polygon& rPoly = rPolyPoly[ i ];

                    for( USHORT n = 0, nSize = rPoly.GetSize(); n < nSize; n++ )
                    {
                        Point& rPt = rPoly[ n ];
                        rPt.X() = FRound( ImplDevicePixelToLogicWidth( rPt.X() - nOffX  ) * fScaleX );
                        rPt.Y() = FRound( ImplDevicePixelToLogicHeight( rPt.Y() - nOffY ) * fScaleY );
                    }
                }

                bRet = TRUE;
            }
        }
        else
            delete pVDev;
    }

    if( !bRet )
        rPolyPoly = PolyPolygon();
#ifdef NEW_GLYPH
    else if( GetFont().GetOrientation() )
        rPolyPoly.Rotate( Point(), GetFont().GetOrientation() );
#endif // NEW_GLYPH

    return bRet;
}
