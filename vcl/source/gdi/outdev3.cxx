/*************************************************************************
 *
 *  $RCSfile: outdev3.cxx,v $
 *
 *  $Revision: 1.56 $
 *
 *  last change: $Author: th $ $Date: 2001-07-25 18:11:20 $
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
#ifndef _SV_EVENT_HXX
#include <event.hxx>
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
#ifndef _SV_FONTCVT_HXX
#include <fontcvt.hxx>
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

// =======================================================================

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
static sal_Unicode const aSimHei[] = { 0x9ED1, 0x4F53, 0, 0 };
static sal_Unicode const aSimKai[] = { 0x6977, 0x4F53, 0, 0 };
static sal_Unicode const azycjkSun[] = { 0x4E2D, 0x6613, 0x5B8B, 0x4F53, 0, 0 };
static sal_Unicode const azycjkHei[] = { 0x4E2D, 0x6613, 0x9ED1, 0x4F53, 0, 0 };
static sal_Unicode const azycjkKai[] = { 0x4E2D, 0x6613, 0x6977, 0x4F53, 0, 0 };
static sal_Unicode const aFZHei[] = { 0x65B9, 0x6B63, 0x9ED1, 0x4F53, 0, 0 };
static sal_Unicode const aFZKai[] = { 0x65B9, 0x6B63, 0x6977, 0x4F53, 0, 0 };
static sal_Unicode const aFZSongYI[] = { 0x65B9, 0x6B63, 0x5B8B, 0x4E00, 0, 0 };
static sal_Unicode const aFZShuSong[] = { 0x65B9, 0x6B63, 0x4E66, 0x5B8B, 0, 0 };
static sal_Unicode const aFZFangSong[] = { 0x65B9, 0x6B63, 0x4EFF, 0x5B8B, 0, 0 };
// Attention: this fonts includes the wrong encoding vector - so we double the names with correct and wrong encoding
// First one is the GB-Encoding (we think the correct one), second is the big5 encoded name
static sal_Unicode const aMHei[] = { 'm', 0x7B80, 0x9ED1, 0, 'm', 0x6F60, 0x7AAA, 0, 0 };
static sal_Unicode const aMKai[] = { 'm', 0x7B80, 0x6977, 0x566C, 0, 'm', 0x6F60, 0x7FF1, 0x628E, 0, 0 };
static sal_Unicode const aMSong[] = { 'm', 0x7B80, 0x5B8B, 0, 'm', 0x6F60, 0x51BC, 0, 0 };
static sal_Unicode const aCFangSong[] = { 'm', 0x7B80, 0x592B, 0x5B8B, 0, 'm', 0x6F60, 0x6E98, 0x51BC, 0, 0 };

static sal_Unicode const aMingLiU[] = { 0x7D30, 0x660E, 0x9AD4, 0, 0 };
static sal_Unicode const aPMingLiU[] = { 0x65B0, 0x7D30, 0x660E, 0x9AD4, 0, 0 };
static sal_Unicode const aHei[] = { 0x6865, 0, 0 };
static sal_Unicode const aKai[] = { 0x6B61, 0, 0 };
static sal_Unicode const aMing[] = { 0x6D69, 0x6E67, 0, 0 };

static sal_Unicode const aMSGothic[] = { 'm', 's', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
static sal_Unicode const aMSPGothic[] = { 'm', 's', 'p', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0, 0 };
static sal_Unicode const aMSMincho[] = { 'm', 's', 0x660E, 0x671D, 0, 0 };
static sal_Unicode const aMSPMincho[] = { 'm', 's', 'p', 0x660E, 0x671D, 0, 0 };
static sal_Unicode const aHGMinchoL[] = { 'h', 'g', 0x660E, 0x671D, 'l', 0, 0 };
static sal_Unicode const aHGGothicB[] = { 'h', 'g', 0x30B4, 0x30B7, 0x30C3, 0x30AF, 'b', 0, 0 };
static sal_Unicode const aHGHeiseiMin[] = { 'h', 'g', 0x5E73, 0x6210, 0x660E, 0x671D, 0x4F53, 0, 'h', 'g', 0x5E73, 0x6210, 0x660E, 0x671D, 0x4F53, 'w', '3', 'x', '1', '2', 0, 0 };

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
{   "simhei",               aSimHei },
{   "simkai",               aSimKai },
{   "zycjksun",             azycjkSun },
{   "zycjkhei",             azycjkHei },
{   "zycjkkai",             azycjkKai },
{   "fzhei",                aFZHei },
{   "fzkai",                aFZKai },
{   "fzsong",               aFZSongYI },
{   "fzshusong",            aFZShuSong },
{   "fzfangsong",           aFZFangSong },
{   "mhei",                 aMHei },
{   "mkai",                 aMKai },
{   "msong",                aMSong },
{   "cfangsong",            aCFangSong },
{   "mingliu",              aMingLiU },
{   "pmingliu",             aPMingLiU },
{   "hei",                  aHei },
{   "kai",                  aKai },
{   "ming",                 aMing },
{   "msgothic",             aMSGothic },
{   "mspgothic",            aMSPGothic },
{   "msmincho",             aMSMincho },
{   "mspmincho",            aMSPMincho },
{   "hgminchol",            aHGMinchoL },
{   "hggothicb",            aHGGothicB },
{   "hgheiseimin",          aHGHeiseiMin },
{   NULL,                   NULL },
};

// -----------------------------------------------------------------------

void ImplGetEnglishSearchFontName( String& rName )
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

// -----------------------------------------------------------------------

String GetFontToken( const String& rStr, xub_StrLen nToken, xub_StrLen& rIndex )
{
    const sal_Unicode*  pStr            = rStr.GetBuffer();
    xub_StrLen          nLen            = (xub_StrLen)rStr.Len();
    xub_StrLen          nTok            = 0;
    xub_StrLen          nFirstChar      = rIndex;
    xub_StrLen          i               = nFirstChar;

    // Bestimme die Token-Position und Laenge
    pStr += i;
    while ( i < nLen )
    {
        // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
        if ( (*pStr == ';') || (*pStr == ',') )
        {
            nTok++;

            if ( nTok == nToken )
                nFirstChar = i+1;
            else
            {
                if ( nTok > nToken )
                    break;
            }
        }

        pStr++;
        i++;
    }

    if ( nTok >= nToken )
    {
        if ( i < nLen )
            rIndex = i+1;
        else
            rIndex = STRING_NOTFOUND;
        return String( rStr, nFirstChar, i-nFirstChar );
    }
    else
    {
        rIndex = STRING_NOTFOUND;
        return String();
    }
}

// =======================================================================

static const char* const aImplKillLeadingList[] =
{
    "microsoft",
    "monotype",
    "linotype",
    "adobe",
    "nimbus",
    "zycjk",
    "itc",
    "sun",
    "amt",
    "ms",
    "mt",
    "cg",
    "hg",
    "fz",
    NULL
};

// -----------------------------------------------------------------------

static const char* const aImplKillTrailingList[] =
{
    "microsoft",
    "monotype",
    "linotype",
    "adobe",
    "nimbus",
    "itc",
    "sun",
    "amt",
    "ms",
    "mt",
    // Scripts, for compatibility with older versions
    "we",
    "cyr",
    "tur",
    "wt",
    "greek",
    "wl",
    // CJK extensions
    "gb",
    "big5",
    "pro",
    "z01",
    "z02",
    "z03",
    "z13",
    "b01",
    "w3x12",
    // Old Printer Fontnames
    "5cpi",
    "6cpi",
    "7cpi",
    "8cpi",
    "9cpi",
    "10cpi",
    "11cpi",
    "12cpi",
    "13cpi",
    "14cpi",
    "15cpi",
    "16cpi",
    "18cpi",
    "24cpi",
    "scale",
    "pc",
    NULL
};

// -----------------------------------------------------------------------

static const char* const aImplKillTrailingWithExceptionsList[] =
{
    "ce", "monospa", "oldfa", NULL,
    "ps", "ca", NULL,
    NULL
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

// IMPL_FONT_ATTR_DEFAULT       - Default-Font like Andale Sans UI, Palace Script, Albany, Thorndale, Cumberland, ...
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
// IMPL_FONT_ATTR_OTHERSTYLE    - OldStyle, ... so negativ points
#define IMPL_FONT_ATTR_DEFAULT       ((ULONG)0x00000001)
#define IMPL_FONT_ATTR_STANDARD      ((ULONG)0x00000002)
#define IMPL_FONT_ATTR_NORMAL        ((ULONG)0x00000004)
#define IMPL_FONT_ATTR_SYMBOL        ((ULONG)0x00000008)
#define IMPL_FONT_ATTR_FIXED         ((ULONG)0x00000010)
#define IMPL_FONT_ATTR_SANSSERIF     ((ULONG)0x00000020)
#define IMPL_FONT_ATTR_SERIF         ((ULONG)0x00000040)
#define IMPL_FONT_ATTR_DECORATIVE    ((ULONG)0x00000080)
#define IMPL_FONT_ATTR_SPECIAL       ((ULONG)0x00000100)
#define IMPL_FONT_ATTR_ITALIC        ((ULONG)0x00000200)
#define IMPL_FONT_ATTR_TITLING       ((ULONG)0x00000400)
#define IMPL_FONT_ATTR_CAPITALS      ((ULONG)0x00000800)
#define IMPL_FONT_ATTR_CJK           ((ULONG)0x00001000)
#define IMPL_FONT_ATTR_CJK_JP        ((ULONG)0x00002000)
#define IMPL_FONT_ATTR_CJK_SC        ((ULONG)0x00004000)
#define IMPL_FONT_ATTR_CJK_TC        ((ULONG)0x00008000)
#define IMPL_FONT_ATTR_CJK_KR        ((ULONG)0x00010000)
#define IMPL_FONT_ATTR_CTL           ((ULONG)0x00020000)
#define IMPL_FONT_ATTR_NONELATIN     ((ULONG)0x00040000)
#define IMPL_FONT_ATTR_FULL          ((ULONG)0x00080000)
#define IMPL_FONT_ATTR_OUTLINE       ((ULONG)0x00100000)
#define IMPL_FONT_ATTR_SHADOW        ((ULONG)0x00200000)
#define IMPL_FONT_ATTR_ROUNDED       ((ULONG)0x00400000)
#define IMPL_FONT_ATTR_TYPEWRITER    ((ULONG)0x00800000)
#define IMPL_FONT_ATTR_SCRIPT        ((ULONG)0x01000000)
#define IMPL_FONT_ATTR_HANDWRITING   ((ULONG)0x02000000)
#define IMPL_FONT_ATTR_CHANCERY      ((ULONG)0x04000000)
#define IMPL_FONT_ATTR_COMIC         ((ULONG)0x08000000)
#define IMPL_FONT_ATTR_BRUSHSCRIPT   ((ULONG)0x10000000)
#define IMPL_FONT_ATTR_GOTHIC        ((ULONG)0x20000000)
#define IMPL_FONT_ATTR_SCHOOLBOOK    ((ULONG)0x40000000)
#define IMPL_FONT_ATTR_OTHERSTYLE    ((ULONG)0x80000000)

#define IMPL_FONT_ATTR_CJK_ALLLANG   (IMPL_FONT_ATTR_CJK_JP | IMPL_FONT_ATTR_CJK_SC | IMPL_FONT_ATTR_CJK_TC | IMPL_FONT_ATTR_CJK_KR)
#define IMPL_FONT_ATTR_ALLSCRIPT     (IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_BRUSHSCRIPT)
#define IMPL_FONT_ATTR_ALLSUBSCRIPT  (IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_BRUSHSCRIPT)
#define IMPL_FONT_ATTR_ALLSERIFSTYLE (IMPL_FONT_ATTR_ALLSCRIPT |\
                                      IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_SERIF |\
                                      IMPL_FONT_ATTR_FIXED | IMPL_FONT_ATTR_ITALIC |\
                                      IMPL_FONT_ATTR_GOTHIC | IMPL_FONT_ATTR_SCHOOLBOOK |\
                                      IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_OUTLINE)

struct ImplFontAttrTypeSearchData
{
    const char*             mpStr;
    ULONG                   mnType;
};

static ImplFontAttrTypeSearchData const aImplTypeAttrSearchList[] =
{
{   "monotype",             0 },
{   "linotype",             0 },
{   "titling",              IMPL_FONT_ATTR_TITLING },
{   "captitals",            IMPL_FONT_ATTR_CAPITALS },
{   "captital",             IMPL_FONT_ATTR_CAPITALS },
{   "caps",                 IMPL_FONT_ATTR_CAPITALS },
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
{   "corsiva",              IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_SCRIPT },
{   "gothic",               IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_GOTHIC },
{   "schoolbook",           IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_SCHOOLBOOK },
{   "schlbk",               IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_SCHOOLBOOK },
{   "typewriter",           IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "lineprinter",          IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "monospaced",           IMPL_FONT_ATTR_FIXED },
{   "monospace",            IMPL_FONT_ATTR_FIXED },
{   "mono",                 IMPL_FONT_ATTR_FIXED },
{   "fixed",                IMPL_FONT_ATTR_FIXED },
{   "sansserif",            IMPL_FONT_ATTR_SANSSERIF },
{   "sans",                 IMPL_FONT_ATTR_SANSSERIF },
{   "swiss",                IMPL_FONT_ATTR_SANSSERIF },
{   "serif",                IMPL_FONT_ATTR_SERIF },
{   "bright",               IMPL_FONT_ATTR_SERIF },
{   "symbols",              IMPL_FONT_ATTR_SYMBOL },
{   "symbol",               IMPL_FONT_ATTR_SYMBOL },
{   "dingbats",             IMPL_FONT_ATTR_SYMBOL },
{   "dings",                IMPL_FONT_ATTR_SYMBOL },
{   "ding",                 IMPL_FONT_ATTR_SYMBOL },
{   "bats",                 IMPL_FONT_ATTR_SYMBOL },
{   "math",                 IMPL_FONT_ATTR_SYMBOL },
{   "oldstyle",             IMPL_FONT_ATTR_OTHERSTYLE },
{   "oldface",              IMPL_FONT_ATTR_OTHERSTYLE },
{   "old",                  IMPL_FONT_ATTR_OTHERSTYLE },
{   "new",                  0 },
{   "modern",               0 },
{   "lucida",               0 },
{   "regular",              0 },
{   "extended",             0 },
{   "extra",                IMPL_FONT_ATTR_OTHERSTYLE },
{   "ext",                  0 },
{   "scalable",             0 },
{   "scale",                0 },
{   "nimbus",               0 },
{   "adobe",                0 },
{   "itc",                  0 },
{   "amt",                  0 },
{   "mt",                   0 },
{   "ms",                   0 },
{   "cpi",                  0 },
{   "no",                   0 },
{   NULL,                   0 },
};

// -----------------------------------------------------------------------

static BOOL ImplKillLeading( String& rName, const char* const* ppStr )
{
    while ( *ppStr )
    {
        const char*         pStr = *ppStr;
        const xub_Unicode*  pNameStr = rName.GetBuffer();
        while ( (*pNameStr == (xub_Unicode)(unsigned char)*pStr) && *pStr )
        {
            pNameStr++;
            pStr++;
        }
        if ( !*pStr )
        {
            rName.Erase( 0, (xub_StrLen)(pNameStr-rName.GetBuffer()) );
            return TRUE;
        }

        ppStr++;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

static xub_StrLen ImplIsTrailing( const sal_Unicode* pEndName, const char* pStr )
{
    const char* pTempStr = pStr;
    while ( *pTempStr )
        pTempStr++;

    xub_StrLen nStrLen = (xub_StrLen)(pTempStr-pStr);
    const sal_Unicode* pNameStr = pEndName-nStrLen;
    while ( (*pNameStr == (xub_Unicode)(unsigned char)*pStr) && *pStr )
    {
        pNameStr++;
        pStr++;
    }

    if ( *pStr )
        return 0;
    else
        return nStrLen;
}

// -----------------------------------------------------------------------

static BOOL ImplKillTrailing( String& rName, const char* const* ppStr )
{
    const xub_Unicode* pEndName = rName.GetBuffer()+rName.Len();
    while ( *ppStr )
    {
        xub_StrLen nTrailLen = ImplIsTrailing( pEndName, *ppStr );
        if ( nTrailLen )
        {
            rName.Erase( rName.Len()-nTrailLen );
            return TRUE;
        }

        ppStr++;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

static BOOL ImplKillTrailingWithExceptions( String& rName, const char* const* ppStr )
{
    const xub_Unicode* pEndName = rName.GetBuffer()+rName.Len();
    while ( *ppStr )
    {
        xub_StrLen nTrailLen = ImplIsTrailing( pEndName, *ppStr );
        if ( nTrailLen )
        {
            const xub_Unicode* pEndNameTemp = pEndName-nTrailLen;
            while ( *ppStr )
            {
                if ( ImplIsTrailing( pEndNameTemp, *ppStr ) )
                    return FALSE;
                ppStr++;
            }

            rName.Erase( rName.Len()-nTrailLen );
            return TRUE;
        }
        else
        {
            while ( *ppStr )
                ppStr++;
        }

        ppStr++;
    }

    return FALSE;
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
    rShortName = rOrgName;

    // Kill leading vendor names and other unimportant data
    ImplKillLeading( rShortName, aImplKillLeadingList );

    // Kill trailing vendor names and other unimportant data
    ImplKillTrailing( rShortName, aImplKillTrailingList );
    ImplKillTrailingWithExceptions( rShortName, aImplKillTrailingWithExceptionsList );

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

    // Remove numbers
    xub_StrLen i = 0;
    while ( i < rFamilyName.Len() )
    {
        sal_Unicode c = rFamilyName.GetChar( i );
        if ( (c >= 0x0030) && (c <= 0x0039) )
            rFamilyName.Erase( i, 1 );
        else
            i++;
    }
}

// =======================================================================

static char const aImplSubsSansUnicode[] = "andalesansui;arialunicodems;lucidaunicode";
static char const aImplSubsSans[] = "albany;arial;helvetica;lucidasans;lucida;geneva;helmet;sansserif;nimbussansl;nimbussans";
static char const aImplSubsSerif[] = "thorndale;timesnewroman;times;timesroman;newyork;timmons;serif;lucidaserif;lucidabright;roman;nimbusromanno9;nimbusromanno9l;bookman;itcbookman;garamond;garamondmt;palatino";
static char const aImplSubsFixed[] = "cumberland;couriernew;courier;lucidatypewriter;lucidasanstypewriter;monaco;monospaced;nimbusmono;nimbusmonol";
static char const aImplSubsStarSymbol[] = "starsymbol;opensymbol;starbats;wingdings;zapfdingbats;itczapfdingbats;monotypesorts;dingbats;lucidadingbats;lucidasansdingbats;webdings;symbol;standardsymbols;standardsymbolsl";
static char const aImplSubsDingBats[] = "starsymbol;zapfdingbats;itczapfdingbats;monotypesorts;dingbats;opensymbol";
static char const aImplSubsSymbol[] = "starsymbol;symbol;standardsymbols;standardsymbolsl;mtsymbol;opensymbol";
static char const aImplSubsWingdings[] = "starsymbol;wingdings;morewingbats;opensymbol";

static char const aImplSubsAndaleSans[] = "andalesans;verdana;trebuchetms";
static char const aImplSubsSansNarrow[] = "arialnarrow;helveticanarrow;helmetcondensed;nimbussanslcondensed;nimbussanscondensed";
static char const aImplSubsBroadway[] = "broadway;mtbroadway;broadwaymt";
static char const aImplSubsComic[] = "comicsansms;kidprint;";
static char const aImplSubsPalaceScript[] = "palacescript;palacescriptmt;arioso;shelley";
static char const aImplSubsSheffield[] = "sheffield;conga;centurygothic;copperlategothic;felixtitling";

static char const aImplSubsJPGothic[] = "msgothic;mspgothic;hggothic;hggothicb;hggothice;gothic;andalesansui";
static char const aImplSubsJPMincho[] = "hgmincholightj;msmincho;mspmincho;hgminchoj;hgminchol;minchol;mincho;hgheiseimin;heiseimin";
static char const aImplSubsSCSun[] = "msunglightsc;simsun;nsimsun;zycjksun";
static char const aImplSubsSCHei[] = "simhei;fzhei;zycjkhei;mhei;hei;andalesansui";
static char const aImplSubsSCKai[] = "simkai;fzkai;zycjkkai;mkai;kai";
static char const aImplSubsSCSong[] = "song;fzsongyi;fzsong;msong;shusong;fzshusong";
static char const aImplSubsSCFangSong[] = "fangsong;fzfangsong;cfangsong";
static char const aImplSubsTCMing[] = "msunglighttc;mingliu;pmingliu;ming";
static char const aImplSubsTCHei[] = "hei;andalesansui";
static char const aImplSubsTCKai[] = "kai";
static char const aImplSubsSTCHei[] = "hei;andalesansui;simhei;fzhei;zycjkhei;mhei"; // SC and TC names, because the original name could be SC or TC
static char const aImplSubsSTCKai[] = "kai;simkai;fzkai;zycjkkai;mkai"; // SC and TC names, because the original name could be SC or TC
static char const aImplSubsKRBatang[] = "hymyeongjolightk;myeongjo;batang;batangche;gungsuh;gungsuhche;myeomjo";
static char const aImplSubsKRGulim[] = "gulim;gulimche;dotum;dotumche;roundgothic;kodig;andalesansui";

static char const aImplSubsCenturyGothic[] = "centurygothic;avantgarde;itcavantgarde;gothic;sheffield;conga";
static char const aImplSubsNewCenturySchoolbook[] = "newcenturyschlbk;newcenturyschoolbook;centuryschoolbook;centuryschoolbookl";
static char const aImplSubsBookman[] = "bookman;itcbookman;bookmanl;bookmanoldstyle;";
static char const aImplSubsPalatino[] = "palatino;bookantiqua;palladio;palladiol";

static char const aImplSubsImprintShadow[] = "imprintmtshadow;imprintshadow;imprint;chevaraoutline;chevara;gallia;colonnamt;algerian;castellar";
static char const aImplSubsOutline[] = "monotypeoldstyleboldoutline;monotypeoldstyleoutline;chevaraoutline;imprintmtshadow;imprintshadow;imprint;colonnamt;castellar";
static char const aImplSubsShadow[] = "imprintmtshadow;imprintshadow;imprint;chevara;gallia;algerian";
static char const aImplSubsFalstaff[] = "falstaff;widelatin;latinwide;impact";
static char const aImplSubsZapfChancery[] = "zapfchancery;itczapfchancery;monotypecorsiva;corsiva;chancery;chanceryl;lucidacalligraphy;lucidahandwriting";


static char const aImplMSSubsArial[] = "Arial";
static char const aImplMSSubsTimesNewRoman[] = "Times New Roman";
static char const aImplMSSubsCourierNew[] = "Courier New";
static char const aImplMSSubsArialUnicodeMS[] = "Arial Unicode MS;Andale Sans UI";

static char const aImplPSSubsHelvetica[] = "Helvetica";
static char const aImplPSSubsTimes[] = "Times";
static char const aImplPSSubsCourier[] = "Courier";

static char const aImplHTMLSubsSerif[] = "serif";
static char const aImplHTMLSubsSansSerif[] = "sans-serif";
static char const aImplHTMLSubsMonospace[] = "monospace";
static char const aImplHTMLSubsCursive[] = "cursive";
static char const aImplHTMLSubsFantasy[] = "fantasy";

// -----------------------------------------------------------------------

struct ImplFontNameAttr
{
    const char*             mpName;
    const char*             mpSubstitution1;
    const char*             mpSubstitution2;
    const char*             mpSubstitution3;
    const char*             mpSubstitution4;
    const char*             mpMSSubstitution;
    const char*             mpPSSubstitution;
    const char*             mpHTMLSubstitution;
    FontWeight              meWeight;
    FontWidth               meWidth;
    ULONG                   mnType;
};

// Not classified
// Cloisterblack - old english style - comes with Applixware

// List is sorted alphabetic
static ImplFontNameAttr const aImplFontNameList[] =
{
{   "albany",               aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, aImplMSSubsArial, aImplPSSubsHelvetica, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT },
{   "algerian",             aImplSubsShadow, aImplSubsImprintShadow, aImplSubsOutline, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_TITLING },
{   "almanac",              aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "andalesans",           aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "andalesansui",         aImplSubsSansUnicode, aImplSubsSans, NULL, NULL, aImplMSSubsArialUnicodeMS, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_FULL | IMPL_FONT_ATTR_DEFAULT },
{   "andalewtui",           aImplSubsSansUnicode, aImplSubsSans, NULL, NULL, aImplMSSubsArialUnicodeMS, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_FULL | IMPL_FONT_ATTR_DEFAULT },
{   "andy",                 aImplSubsComic, aImplSubsZapfChancery, NULL, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_ITALIC },
{   "antiqueolive",         aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "arial",                aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, aImplPSSubsHelvetica, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "arialnarrow",          aImplSubsSansNarrow, aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "arialunicode",         aImplSubsSansUnicode, aImplSubsSans, NULL, NULL, NULL, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_FULL },
{   "arioso",               aImplSubsPalaceScript, aImplSubsZapfChancery, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "avantgarde",           aImplSubsCenturyGothic, aImplSubsSans, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "baskervilleoldface",   aImplSubsSerif, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_OTHERSTYLE },
{   "batang",               aImplSubsKRBatang, aImplSubsKRGulim, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "batangche",            aImplSubsKRBatang, aImplSubsKRGulim, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "bell",                 aImplSubsSerif, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "blackadder",           aImplSubsPalaceScript, aImplSubsZapfChancery, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "bookantiqua",          aImplSubsPalatino, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "bookman",              aImplSubsBookman, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "bookmanl",             aImplSubsBookman, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "bookmanoldstyle",      aImplSubsBookman, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_OTHERSTYLE },
{   "bradleyhand",          aImplSubsComic, aImplSubsZapfChancery, aImplSubsPalaceScript, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_ITALIC },
{   "broadway",             aImplSubsBroadway, aImplSubsFalstaff, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_BLACK, WIDTH_EXPANDED, IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL },
{   "calisto",              aImplSubsSerif, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "castellar",            aImplSubsOutline, aImplSubsImprintShadow, aImplSubsShadow, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_TITLING },
{   "century",              aImplSubsNewCenturySchoolbook, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "centurygothic",        aImplSubsCenturyGothic, aImplSubsSans, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "centuryschoolbook",    aImplSubsNewCenturySchoolbook, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_SCHOOLBOOK },
{   "centuryschoolbookl",   aImplSubsNewCenturySchoolbook, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_SCHOOLBOOK },
{   "cfangsong",            aImplSubsSCFangSong, aImplSubsSCKai, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "cgtimes",              aImplSubsSerif, NULL, NULL, NULL, NULL, aImplPSSubsTimes, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "chancery",             aImplSubsZapfChancery, aImplSubsPalaceScript, aImplSubsComic, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_ITALIC },
{   "chanceryl",            aImplSubsZapfChancery, aImplSubsPalaceScript, aImplSubsComic, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_ITALIC },
{   "chevara",              aImplSubsShadow, aImplSubsImprintShadow, aImplSubsOutline, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_CAPITALS },
{   "chevaraoutline",       aImplSubsOutline, aImplSubsImprintShadow, aImplSubsShadow, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_CAPITALS },
{   "chicago",              aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, aImplPSSubsHelvetica, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "chiller",              aImplSubsComic, aImplSubsZapfChancery, aImplSubsPalaceScript, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_ITALIC | IMPL_FONT_ATTR_SPECIAL },
{   "colonna",              aImplSubsOutline, aImplSubsImprintShadow, aImplSubsShadow, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL },
{   "comicsans",            aImplSubsComic, aImplSubsZapfChancery, NULL, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_ITALIC | IMPL_FONT_ATTR_STANDARD },
{   "comicsansms",          aImplSubsComic, aImplSubsZapfChancery, NULL, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_ITALIC | IMPL_FONT_ATTR_STANDARD },
{   "conga",                aImplSubsSheffield, aImplSubsCenturyGothic, aImplSubsSans, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_CAPITALS },
{   "copperlategothic",     aImplSubsSheffield, aImplSubsCenturyGothic, aImplSubsSans, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_CAPITALS },
{   "corsiva",              aImplSubsZapfChancery, aImplSubsPalaceScript, NULL, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_ITALIC },
{   "courier",              aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED | IMPL_FONT_ATTR_STANDARD },
{   "couriernew",           aImplSubsFixed, NULL, NULL, NULL, NULL, aImplPSSubsCourier, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED | IMPL_FONT_ATTR_STANDARD },
{   "cumberland",           aImplSubsFixed, NULL, NULL, NULL, aImplMSSubsCourierNew, aImplPSSubsCourier, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED | IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT },
{   "curlz",                aImplSubsComic, aImplSubsZapfChancery, aImplSubsPalaceScript, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_ITALIC | IMPL_FONT_ATTR_SPECIAL },
{   "cursive",              aImplSubsComic, aImplSubsZapfChancery, aImplSubsPalaceScript, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "dingbats",             aImplSubsDingBats, aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "dotum",                aImplSubsKRGulim, aImplSubsKRBatang, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "dotumche",             aImplSubsKRGulim, aImplSubsKRBatang, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "edwardianscript",      aImplSubsPalaceScript, aImplSubsZapfChancery, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "elite",                aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "elited",               aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "engravers",            aImplSubsSheffield, aImplSubsCenturyGothic, aImplSubsSans, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_CAPITALS },
{   "extra",                aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "falstaff",             aImplSubsFalstaff, aImplSubsBroadway, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_BLACK, WIDTH_ULTRA_EXPANDED, IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL },
{   "fangsong",             aImplSubsSCFangSong, aImplSubsSCKai, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "fantasy",              aImplSubsImprintShadow, aImplSubsOutline, aImplSubsShadow, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_DECORATIVE },
{   "felixtitling",         aImplSubsSheffield, aImplSubsCenturyGothic, aImplSubsSans, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_CAPITALS },
{   "forte",                aImplSubsComic, aImplSubsZapfChancery, NULL, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_ITALIC | IMPL_FONT_ATTR_DECORATIVE },
{   "frenchscript",         aImplSubsPalaceScript, aImplSubsZapfChancery, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC | IMPL_FONT_ATTR_SPECIAL },
{   "frutiger",             aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "fujiyama",             aImplSubsSansNarrow, aImplSubsSans, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SANSSERIF },
{   "fzhei",                aImplSubsSCHei, aImplSubsSCSun, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "fzkai",                aImplSubsSCKai, aImplSubsSCFangSong, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "gallia",               aImplSubsShadow, aImplSubsImprintShadow, aImplSubsOutline, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_TITLING },
{   "garamond",             aImplSubsSerif, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "geneva",               aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, aImplPSSubsHelvetica, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "gigi",                 aImplSubsComic, aImplSubsZapfChancery, aImplSubsPalaceScript, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_ITALIC | IMPL_FONT_ATTR_SPECIAL },
{   "gothic",               aImplSubsJPGothic, aImplSubsJPMincho, aImplSubsCenturyGothic, aImplSubsSansUnicode, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "gothicb",              aImplSubsJPGothic, aImplSubsJPMincho, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "gothicl",              aImplSubsJPGothic, aImplSubsJPMincho, aImplSubsCenturyGothic, aImplSubsSansUnicode, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "gulim",                aImplSubsKRGulim, aImplSubsKRBatang, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "gulimche",             aImplSubsKRGulim, aImplSubsKRBatang, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "gungsuh",              aImplSubsKRBatang, aImplSubsKRGulim, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "gungsuhche",           aImplSubsKRBatang, aImplSubsKRGulim, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "hei",                  aImplSubsSTCHei, aImplSubsTCMing, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC },
{   "heiseimin",            aImplSubsJPMincho, aImplSubsJPGothic, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "helmet",               aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, aImplMSSubsArial, aImplPSSubsHelvetica, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "helmetcondensed",      aImplSubsSansNarrow, aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "helv",                 aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "helvetica",            aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_STANDARD },
{   "helveticanarrow",      aImplSubsSansNarrow, aImplSubsSans, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "hggothic",             aImplSubsJPGothic, aImplSubsJPMincho, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "hggothicb",            aImplSubsJPGothic, aImplSubsJPMincho, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "hggothice",            aImplSubsJPGothic, aImplSubsJPMincho, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "hgminchoj",            aImplSubsJPMincho, aImplSubsJPGothic, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "hgminchol",            aImplSubsJPMincho, aImplSubsJPGothic, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "hgmincholightj",       aImplSubsJPMincho, aImplSubsJPGothic, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "holidaypi",            aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "holidays",             aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "hymyeongjolightk",     aImplSubsKRBatang, aImplSubsKRGulim, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "impact",               aImplSubsFalstaff, aImplSubsBroadway, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_BLACK, WIDTH_ULTRA_EXPANDED, IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL },
{   "imprint",              aImplSubsImprintShadow, aImplSubsOutline, aImplSubsShadow, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_DECORATIVE },
{   "imprintmtshadow",      aImplSubsImprintShadow, aImplSubsOutline, aImplSubsShadow, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_DECORATIVE },
{   "imprintshadow",        aImplSubsImprintShadow, aImplSubsOutline, aImplSubsShadow, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_DECORATIVE },
{   "informalroman",        aImplSubsZapfChancery, aImplSubsComic, aImplSubsPalaceScript, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "kai",                  aImplSubsSTCKai, aImplSubsTCMing, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC },
{   "kidprint",             aImplSubsComic, aImplSubsZapfChancery, NULL, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_ITALIC | IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT },
{   "kodig",                aImplSubsKRGulim, aImplSubsKRBatang, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "kristen",              aImplSubsComic, aImplSubsZapfChancery, NULL, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_COMIC | IMPL_FONT_ATTR_ITALIC },
{   "kunstlerscript",       aImplSubsPalaceScript, aImplSubsZapfChancery, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "latinwide",            aImplSubsFalstaff, aImplSubsBroadway, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_BLACK, WIDTH_ULTRA_EXPANDED, IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL },
{   "lettergothic",         aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "linedraw",             aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "lineprinter",          aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "losangeles",           "onyx", aImplSubsSansNarrow, aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_SPECIAL },
{   "lucidabright",         aImplSubsSerif, NULL, NULL, NULL, NULL, aImplPSSubsTimes, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "lucidacalligraphic",   aImplSubsZapfChancery, aImplSubsPalaceScript, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_ITALIC },
{   "lucidacalligraphy",    aImplSubsZapfChancery, aImplSubsPalaceScript, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_ITALIC },
{   "lucidaconsole",        aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "lucidadingbats",       aImplSubsDingBats, aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "lucidahandwriting",    aImplSubsComic, aImplSubsZapfChancery, aImplSubsPalaceScript, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_ITALIC },
{   "lucidasans",           aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, aImplPSSubsHelvetica, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "lucidasansdingbats",   aImplSubsDingBats, aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "lucidasanstyp",        aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "lucidasanstypewriter", aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "lucidasansunicode",    aImplSubsSansUnicode, aImplSubsSans, NULL, NULL, NULL, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "lucidatypewriter",     aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "marlett",              aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "mhei",                 aImplSubsSCHei, aImplSubsSCSun, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "mincho",               aImplSubsJPMincho, aImplSubsJPGothic, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "minchoj",              aImplSubsJPMincho, aImplSubsJPGothic, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "minchol",              aImplSubsJPMincho, aImplSubsJPGothic, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "ming",                 aImplSubsTCMing, aImplSubsTCHei, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC },
{   "mingli",               aImplSubsTCMing, aImplSubsTCHei, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC },
{   "mingliu",              aImplSubsTCMing, aImplSubsTCHei, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC },
{   "mkai",                 aImplSubsSCKai, aImplSubsSCFangSong, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "monaco",               aImplSubsFixed, NULL, NULL, NULL, NULL, aImplPSSubsCourier, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "mono",                 aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "monol",                aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "monospace",            aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "monospaced",           aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "morewingbats",         aImplSubsWingdings, aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "msgothic",             aImplSubsJPGothic, aImplSubsJPMincho, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "msmincho",             aImplSubsJPMincho, aImplSubsJPGothic, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "msong",                aImplSubsSCSong, aImplSubsSCSun, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "mspmincho",            aImplSubsJPMincho, aImplSubsJPGothic, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "msuigothic",           aImplSubsJPGothic, aImplSubsJPMincho, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "msunglightsc",         aImplSubsSCSun, aImplSubsSCSong, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "msunglighttc",         aImplSubsTCMing, aImplSubsTCHei, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC },
{   "mtbroadway",           aImplSubsBroadway, aImplSubsFalstaff, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_BLACK, WIDTH_ULTRA_EXPANDED, IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL },
{   "myeongjo",             aImplSubsKRBatang, aImplSubsKRGulim, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "newcenturyschlbk",     aImplSubsNewCenturySchoolbook, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_SCHOOLBOOK },
{   "newcenturyschoolbook", aImplSubsNewCenturySchoolbook, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_SCHOOLBOOK },
{   "newyork",              aImplSubsSerif, NULL, NULL, NULL, NULL, aImplPSSubsTimes, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "nlq",                  aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "nsimsun",              aImplSubsSCSun, aImplSubsSCSong, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "ocean",                aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "omega",                aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "onyx",                 "losangeles", aImplSubsSansNarrow, aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_SPECIAL },
{   "opensymbol",           "starsymbol", aImplSubsSansUnicode, aImplSubsStarSymbol, NULL, aImplMSSubsArialUnicodeMS, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "outlook",              aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "palacescript",         aImplSubsPalaceScript, aImplSubsZapfChancery, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC | IMPL_FONT_ATTR_DEFAULT },
{   "palatino",             aImplSubsPalatino, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "palladio",             aImplSubsPalatino, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "palladiol",            aImplSubsPalatino, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "pepita",               aImplSubsZapfChancery, aImplSubsPalaceScript, aImplSubsComic, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_ITALIC },
{   "pgothic",              aImplSubsJPGothic, aImplSubsJPMincho, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "pica",                 aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "picad",                aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_TYPEWRITER | IMPL_FONT_ATTR_FIXED },
{   "pmincho",              aImplSubsJPMincho, aImplSubsJPGothic, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP },
{   "pmingliu",             aImplSubsTCMing, aImplSubsTCHei, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC },
{   "pristina",             aImplSubsZapfChancery, aImplSubsPalaceScript, aImplSubsComic, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_ITALIC },
{   "ps",                   aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "psd",                  aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "psnlq",                aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "roman",                aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "romanno9",             aImplSubsSerif, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "romanno9l",            aImplSubsSerif, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "romanps",              aImplSubsFixed, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsMonospace,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_FIXED },
{   "roundgothic",          aImplSubsKRGulim, aImplSubsKRBatang, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR },
{   "sans",                 aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "sanscondensed",        aImplSubsSansNarrow, aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "sansl",                aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "sanslcondensed",       aImplSubsSansNarrow, aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "sansserif",            aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "schoolbook",           aImplSubsNewCenturySchoolbook, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_SCHOOLBOOK },
{   "script",               aImplSubsPalaceScript, aImplSubsZapfChancery, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "segoe",                aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "serif",                aImplSubsSerif, NULL, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "sheffield",            aImplSubsSheffield, aImplSubsCenturyGothic, aImplSubsSans, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_CAPITALS | IMPL_FONT_ATTR_DEFAULT },
{   "shelley",              aImplSubsPalaceScript, aImplSubsZapfChancery, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "shusong",              aImplSubsSCSong, aImplSubsSCSun, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "simhei",               aImplSubsSCHei, aImplSubsSCSun, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "simkai",               aImplSubsSCKai, aImplSubsSCFangSong, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "simsong",              aImplSubsSCSong, aImplSubsSCSun, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "simsun",               aImplSubsSCSun, aImplSubsSCSong, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "snowcap",              aImplSubsImprintShadow, aImplSubsOutline, aImplSubsShadow, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW | IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_DECORATIVE },
{   "song",                 aImplSubsSCSong, aImplSubsSCSun, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "sorts",                aImplSubsDingBats, aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "sorts2",               aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "sorts3",               aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "spartan",              aImplSubsSheffield, aImplSubsCenturyGothic, aImplSubsSans, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_CAPITALS },
{   "standardsymbols",      aImplSubsSymbol, aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "standardsymbolsl",     aImplSubsSymbol, aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "starbats",             aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "starmath",             aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "starsymbol",           "opensymbol", aImplSubsSansUnicode, aImplSubsStarSymbol, NULL, aImplMSSubsArialUnicodeMS, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "swiss",                aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "swissnarrow",          aImplSubsSansNarrow, aImplSubsSans, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "symbol",               aImplSubsSymbol, aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "tahoma",               aImplSubsSansUnicode, aImplSubsSans, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "thorndale",            aImplSubsSerif, NULL, NULL, NULL, aImplMSSubsTimesNewRoman, aImplPSSubsTimes, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT },
{   "times",                aImplSubsSerif, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_STANDARD },
{   "timesnewroman",        aImplSubsSerif, NULL, NULL, NULL, NULL, aImplPSSubsTimes, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_STANDARD },
{   "timesroman",           aImplSubsSerif, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "timmons",              aImplSubsSerif, NULL, NULL, NULL, aImplMSSubsTimesNewRoman, aImplPSSubsTimes, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "tmsrmn",               aImplSubsSerif, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "trebuchet",            aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "univers",              aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, aImplPSSubsHelvetica, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "universcondensed",     aImplSubsSansNarrow, aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_CONDENSED, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "utah",                 aImplSubsSans, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "utopia",               aImplSubsNewCenturySchoolbook, aImplSubsSerif, NULL, NULL, NULL, NULL, aImplHTMLSubsSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SERIF },
{   "vacation",             aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "verdana",              aImplSubsSans, NULL, NULL, NULL, NULL, NULL, aImplHTMLSubsSansSerif,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_NORMAL | IMPL_FONT_ATTR_SANSSERIF },
{   "vinerhand",            aImplSubsComic, aImplSubsZapfChancery, aImplSubsPalaceScript, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_HANDWRITING | IMPL_FONT_ATTR_ITALIC },
{   "vivaldi",              aImplSubsPalaceScript, aImplSubsZapfChancery, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "vladimirscript",       aImplSubsPalaceScript, aImplSubsZapfChancery, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_BRUSHSCRIPT | IMPL_FONT_ATTR_ITALIC },
{   "webdings",             aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "webdings2",            aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "webdings3",            aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "widelatin",            aImplSubsFalstaff, aImplSubsBroadway, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_BLACK, WIDTH_ULTRA_EXPANDED, IMPL_FONT_ATTR_SERIF | IMPL_FONT_ATTR_DECORATIVE | IMPL_FONT_ATTR_SPECIAL },
{   "wingdings",            aImplSubsWingdings, aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "wingdings2",           aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "wingdings3",           aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "wingdings4",           aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "zapfcalligraphic",     aImplSubsZapfChancery, aImplSubsPalaceScript, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_ITALIC },
{   "zapfcalligraphy",      aImplSubsZapfChancery, aImplSubsPalaceScript, aImplSubsComic, NULL, NULL, NULL, aImplHTMLSubsCursive,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_ITALIC },
{   "zapfchancery",         aImplSubsZapfChancery, aImplSubsPalaceScript, aImplSubsComic, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SCRIPT | IMPL_FONT_ATTR_CHANCERY | IMPL_FONT_ATTR_ITALIC },
{   "zapfdingbats",         aImplSubsDingBats, aImplSubsStarSymbol, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_SPECIAL | IMPL_FONT_ATTR_SYMBOL },
{   "zycjkhei",             aImplSubsSCHei, aImplSubsSCSun, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "zycjkkai",             aImplSubsSCKai, aImplSubsSCFangSong, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC },
{   "zycjksun",             aImplSubsSCSun, aImplSubsSCSong, aImplSubsSansUnicode, NULL, NULL, NULL, NULL,
                            WEIGHT_NORMAL, WIDTH_NORMAL, IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC }
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
    ImplGetEnglishSearchFontName( pEntry->maSearchName );
    ImplGetEnglishSearchFontName( pEntry->maSearchReplaceName );

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
    ImplGetEnglishSearchFontName( aTempName );
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

static char const aImplDefSansUnicode[] = "Andale Sans UI;Arial Unicode MS;Lucida Sans Unicode;Tahoma";
static char const aImplDefSansUI[] = "Interface User;WarpSans;Geneva;Tahoma;MS Sans Serif;Helv;Dialog;Albany;Lucida;Helvetica;Charcoal;Chicago;Arial;Helmet;Interface System;Sans Serif";
static char const aImplDefSans[] = "Albany;Arial;Helvetica;Lucida;Geneva;Helmet;SansSerif";
static char const aImplDefSerif[] = "Thorndale;Times New Roman;Times;Lucida Serif;Lucida Bright;Timmons;New York;Serif";
static char const aImplDefFixed[] = "Cumberland;Courier New;Courier;Lucida Sans Typewriter;Lucida Typewriter;Monaco;Monospaced";
static char const aImplDefSymbol[] = "StarSymbol;OpenSymbol;Andale Sans UI;Arial Unicode MS;StarBats;Zapf Dingbats;WingDings;Symbol";
static char const aImplDef_CJK_JP_Mincho[] = "HG Mincho Light J;MS Mincho;HG Mincho J;HG Mincho L;HG Mincho;Mincho;MS PMincho";
static char const aImplDef_CJK_JP_Gothic[] = "HG Mincho Light J;MS Gothic;HG Gothic J;HG Gothic B;HG Gothic;Gothic;MS PGothic";
static char const aImplDef_CJK_JP_UIGothic[] = "Andale Sans UI;MS Gothic;HG Gothic J;HG Gothic B;HG Gothic;Gothic;MS PGothic";
static char const aImplDef_CJK_SC[] = "MSung Light SC;SimSun;Song;FZSongYi;FZShuSong;NSimSun";
static char const aImplDef_CJK_TC[] = "MSung Light TC;MingLiU;Ming;PMingLiU";
static char const aImplDef_CJK_KR_Batang[] = "HY MyeongJo Light;Batang;Myeongjo";
static char const aImplDef_CJK_KR_Gulim[] = "Andale Sans UI;Gulim;Roundgothic";

// -----------------------------------------------------------------------

static BOOL ImplIsFontToken( const String& rName, const String& rToken )
{
    BOOL        bRet = FALSE;
    String      aTempName;
    xub_StrLen  nIndex = 0;
    do
    {
        aTempName = GetFontToken( rName, 0, nIndex );
        if ( rToken == aTempName )
        {
            bRet = TRUE;
            break;
        }
    }
    while ( nIndex != STRING_NOTFOUND );

    return bRet;
}

// -----------------------------------------------------------------------

static void ImplAppendFontToken( String& rName, const String rNewToken )
{
    if ( rName.Len() )
    {
        rName.Append( ';' );
        rName.Append( rNewToken );
    }
    else
        rName = rNewToken;
}

// -----------------------------------------------------------------------

static void ImplAppendFontToken( String& rName, const char* pNewToken )
{
    if ( rName.Len() )
    {
        rName.Append( ';' );
        rName.AppendAscii( pNewToken );
    }
    else
        rName.AssignAscii( pNewToken );
}

// -----------------------------------------------------------------------

static void ImplAddTokenFontName( String& rName, const String& rNewToken )
{
    if ( !ImplIsFontToken( rName, rNewToken ) )
        ImplAppendFontToken( rName, rNewToken );
}

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
                aTempName = GetFontToken( rName, 0, nIndex );
                if ( aName == aTempName )
                {
                    aName.Erase();
                    break;
                }
            }
            while ( nIndex != STRING_NOTFOUND );

            if ( aName.Len() )
                ImplAppendFontToken( rName, aName );

            if ( !(*pStr) )
                break;

            pFontNames = pStr+1;
        }

        pStr++;
    }
    while ( 1 );
}

// -----------------------------------------------------------------------

Font OutputDevice::GetDefaultFont( USHORT nType, LanguageType eLang,
                                   ULONG nFlags, const OutputDevice* pOutDev )
{
    Font            aFont;
    const char*     pSearch1 = NULL;
    const char*     pSearch2 = NULL;
    const char*     pSearch3 = NULL;

    switch ( nType )
    {
        case DEFAULTFONT_SANS_UNICODE:
        case DEFAULTFONT_UI_SANS:
            pSearch1 = aImplDefSansUnicode;
            if ( nType == DEFAULTFONT_UI_SANS )
                pSearch2 = aImplDefSansUI;
            else
                pSearch2 = aImplDefSans;
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case DEFAULTFONT_SANS:
        case DEFAULTFONT_LATIN_HEADING:
        case DEFAULTFONT_LATIN_SPREADSHEET:
        case DEFAULTFONT_LATIN_DISPLAY:
            pSearch1 = aImplDefSans;
            pSearch2 = aImplDefSansUnicode;
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case DEFAULTFONT_SERIF:
        case DEFAULTFONT_LATIN_TEXT:
        case DEFAULTFONT_LATIN_PRESENTATION:
            pSearch1 = aImplDefSerif;
            aFont.SetFamily( FAMILY_ROMAN );
            break;

        case DEFAULTFONT_FIXED:
        case DEFAULTFONT_LATIN_FIXED:
        case DEFAULTFONT_UI_FIXED:
            aFont.SetPitch( PITCH_FIXED );
            aFont.SetFamily( FAMILY_MODERN );
            pSearch1 = aImplDefFixed;
            break;

        case DEFAULTFONT_SYMBOL:
            aFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
            pSearch1 = aImplDefSymbol;
            break;

        case DEFAULTFONT_CJK_TEXT:
        case DEFAULTFONT_CJK_PRESENTATION:
        case DEFAULTFONT_CJK_SPREADSHEET:
        case DEFAULTFONT_CJK_HEADING:
        case DEFAULTFONT_CJK_DISPLAY:
            if ( (eLang == LANGUAGE_CHINESE) ||
                 (eLang == LANGUAGE_CHINESE_SIMPLIFIED) ||
                 (eLang == LANGUAGE_CHINESE_SINGAPORE) )
                pSearch1 = aImplDef_CJK_SC;
            else if ( (eLang == LANGUAGE_CHINESE_TRADITIONAL) ||
                      (eLang == LANGUAGE_CHINESE_HONGKONG) ||
                      (eLang == LANGUAGE_CHINESE_MACAU) )
                pSearch1 = aImplDef_CJK_TC;
            else if ( (eLang == LANGUAGE_KOREAN) ||
                      (eLang == LANGUAGE_KOREAN_JOHAB) )
            {
                if ( nType == DEFAULTFONT_CJK_DISPLAY )
                    pSearch1 = aImplDef_CJK_KR_Gulim;
                else
                {
                    pSearch1 = aImplDef_CJK_KR_Batang;
                    pSearch2 = aImplDef_CJK_KR_Gulim;
                }
            }
            else
            {
                if ( (nType == DEFAULTFONT_CJK_DISPLAY) ||
                     (nType == DEFAULTFONT_CJK_SPREADSHEET) )
                    pSearch1 = aImplDef_CJK_JP_UIGothic;
                else if ( nType == DEFAULTFONT_CJK_PRESENTATION )
                    pSearch1 = aImplDef_CJK_JP_Gothic;
                else
                {
                    pSearch1 = aImplDef_CJK_JP_Mincho;
                    pSearch2 = aImplDef_CJK_JP_Gothic;
                }
            }
            pSearch3 = aImplDefSansUnicode;
            break;

        case DEFAULTFONT_CTL_TEXT:
        case DEFAULTFONT_CTL_PRESENTATION:
        case DEFAULTFONT_CTL_SPREADSHEET:
        case DEFAULTFONT_CTL_HEADING:
        case DEFAULTFONT_CTL_DISPLAY:
            pSearch1 = "Arial Unicode MS";
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

        // Should we only return available fonts on the given device
        if ( pOutDev )
        {
            // Create Token String
            String aNames( pSearch1, RTL_TEXTENCODING_ASCII_US );
            if ( pSearch2 )
            {
                aNames += ';';
                aNames.AppendAscii( pSearch2 );
            }
            if ( pSearch3 )
            {
                aNames += ';';
                aNames.AppendAscii( pSearch2 );
            }

            // Search Font in the FontList
            String      aName;
            String      aTempName;
            xub_StrLen  nIndex = 0;
            do
            {
                aTempName = GetFontToken( aNames, 0, nIndex );
                ImplGetEnglishSearchFontName( aTempName );
                ImplDevFontListData* pFoundData = pOutDev->mpFontList->ImplFind( aTempName );
                if ( pFoundData )
                {
                    ImplAddTokenFontName( aName, pFoundData->mpFirst->maName );
                    if ( nFlags & DEFAULTFONT_FLAGS_ONLYONE )
                        break;
                }
            }
            while ( nIndex != STRING_NOTFOUND );
            aFont.SetName( aName );
        }

        // No Name, than set all names
        if ( !aFont.GetName().Len() )
        {
            if ( nFlags & DEFAULTFONT_FLAGS_ONLYONE )
            {
                const char* pStr = pSearch1;
                while ( *pStr && (*pStr != ';') )
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

String GetSubsFontName( const String& rName, ULONG nFlags )
{
    String aName;
    String aOrgName = GetFontToken( rName, 0 );
    ImplGetEnglishSearchFontName( aOrgName );

    // Search Font in FontList
    const ImplFontNameAttr* pFontAttr = ImplGetFontNameAttr( aOrgName );
    if ( pFontAttr )
    {
        String aTempName;

        if ( (nFlags & SUBSFONT_MS) && pFontAttr->mpMSSubstitution )
            ImplAppendFontToken( aTempName, pFontAttr->mpMSSubstitution );
        if ( (nFlags & SUBSFONT_PS) && pFontAttr->mpPSSubstitution )
            ImplAppendFontToken( aTempName, pFontAttr->mpPSSubstitution );
        if ( (nFlags & SUBSFONT_HTML) && pFontAttr->mpHTMLSubstitution )
            ImplAppendFontToken( aTempName, pFontAttr->mpHTMLSubstitution );

        // Add all names which aren't in the original font name
        String      aTempToken;
        xub_StrLen  nIndex = 0;
        do
        {
            aTempToken = GetFontToken( aTempName, 0, nIndex );
            if ( !ImplIsFontToken( rName, aTempToken ) )
            {
                ImplAppendFontToken( aName, aTempToken );
                if ( nFlags & SUBSFONT_ONLYONE )
                    break;
            }
        }
        while ( nIndex != STRING_NOTFOUND );
    }

    return aName;
}

// =======================================================================

static BOOL ImplIsCJKFont( const String& rFontName )
{
    // Test, if Fontname includes CJK characters --> In this case we
    // mention that it is a CJK font
    const sal_Unicode* pStr = rFontName.GetBuffer();
    while ( *pStr )
    {
        if ( ((*pStr >= 0x3000) && (*pStr <= 0xD7AF)) ||
             ((*pStr >= 0xFF00) && (*pStr <= 0xFFEE)) )
            return TRUE;

        pStr++;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

static void ImplCalcType( ULONG& rType, FontWeight& rWeight, FontWidth& rWidth,
                          FontFamily eFamily, const ImplFontNameAttr* pFontAttr )
{
    if ( eFamily != FAMILY_DONTKNOW )
    {
        if ( eFamily == FAMILY_SWISS )
            rType |= IMPL_FONT_ATTR_SANSSERIF;
        else if ( eFamily == FAMILY_ROMAN )
            rType |= IMPL_FONT_ATTR_SERIF;
        else if ( eFamily == FAMILY_SCRIPT )
            rType |= IMPL_FONT_ATTR_SCRIPT;
        else if ( eFamily == FAMILY_MODERN )
            rType |= IMPL_FONT_ATTR_FIXED;
        else if ( eFamily == FAMILY_DECORATIVE )
            rType |= IMPL_FONT_ATTR_DECORATIVE;
    }

    if ( pFontAttr )
    {
        rType |= pFontAttr->mnType;

        if ( ((rWeight == WEIGHT_DONTKNOW) || (rWeight == WEIGHT_NORMAL)) &&
             (pFontAttr->meWeight != WEIGHT_DONTKNOW) )
            rWeight = pFontAttr->meWeight;
        if ( ((rWidth == WIDTH_DONTKNOW) || (rWidth == WIDTH_NORMAL)) &&
             (pFontAttr->meWidth != WIDTH_DONTKNOW) )
            rWidth = pFontAttr->meWidth;
    }
}

// =======================================================================

ImplDevFontList::ImplDevFontList() :
    List( CONTAINER_MAXBLOCKSIZE, 96, 32 )
{
    mbMatchData = FALSE;
    mbMapNames  = FALSE;
}

// -----------------------------------------------------------------------

ImplDevFontList::~ImplDevFontList()
{
    ImplClear();
}

// -----------------------------------------------------------------------

void ImplDevFontList::ImplClear()
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

void ImplDevFontList::Clear()
{
    ImplClear();
    List::Clear();
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

    StringCompare eCompare = pEntry1->maName.CompareTo( pEntry2->maName );
    if ( eCompare == COMPARE_EQUAL )
        eCompare = pEntry1->maStyleName.CompareTo( pEntry2->maStyleName );

    return eCompare;
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
    ImplGetEnglishSearchFontName( aSearchName );

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
        pFoundData->meFamily        = pNewData->meFamily;
        pFoundData->mePitch         = pNewData->mePitch;
        pFoundData->mnTypeFaces     = 0;
        pFoundData->meMatchWeight   = WEIGHT_DONTKNOW;
        pFoundData->meMatchWidth    = WIDTH_DONTKNOW;
        pFoundData->mnMatchType     = 0;
        pNewData->mpNext            = NULL;
        Insert( pFoundData, nIndex );
        bInsert = FALSE;
    }
    else
    {
        if ( pFoundData->meFamily == FAMILY_DONTKNOW )
            pFoundData->meFamily = pNewData->meFamily;
        if ( pFoundData->mePitch == PITCH_DONTKNOW )
            pFoundData->mePitch = pNewData->mePitch;
    }

    // set Match data
    if ( (pNewData->meType == TYPE_SCALABLE) && (pNewData->mnHeight == 0) )
        pFoundData->mnTypeFaces |= IMPL_DEVFONT_SCALABLE;
    if ( pNewData->meCharSet == RTL_TEXTENCODING_SYMBOL )
        pFoundData->mnTypeFaces |= IMPL_DEVFONT_SYMBOL;
    else
        pFoundData->mnTypeFaces |= IMPL_DEVFONT_NONESYMBOL;
    if ( pNewData->meWeight != WEIGHT_DONTKNOW )
    {
        if ( pNewData->meWeight >= WEIGHT_SEMIBOLD )
            pFoundData->mnTypeFaces |= IMPL_DEVFONT_BOLD;
        else if ( pNewData->meWeight <= WEIGHT_SEMILIGHT )
            pFoundData->mnTypeFaces |= IMPL_DEVFONT_LIGHT;
        else
            pFoundData->mnTypeFaces |= IMPL_DEVFONT_NORMAL;
    }
    if ( pNewData->meItalic == ITALIC_NONE )
        pFoundData->mnTypeFaces |= IMPL_DEVFONT_NONEITALIC;
    else if ( (pNewData->meItalic == ITALIC_NORMAL) ||
              (pNewData->meItalic == ITALIC_OBLIQUE) )
        pFoundData->mnTypeFaces |= IMPL_DEVFONT_ITALIC;

    // Add map/alias names
    if ( pNewData->maMapNames.Len() )
    {
        String      aName;
        xub_StrLen  nIndex = 0;
        do
        {
            aName = GetFontToken( pNewData->maMapNames, 0, nIndex );
            ImplGetEnglishSearchFontName( aName );
            if ( aName != aSearchName )
            {
                ImplAddTokenFontName( pFoundData->maMapNames, aName );
                mbMapNames = TRUE;
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
    ImplGetEnglishSearchFontName( aTempName );
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
    ImplGetEnglishSearchFontName( aName );
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

void ImplDevFontList::InitMatchData()
{
    if ( mbMatchData )
        return;

    // Calculate MatchData for all Entries
    ImplDevFontListData* pEntry = First();
    while ( pEntry )
    {
        // Get all information about the matching font
        const ImplFontNameAttr* pTempFontAttr;
        String                  aTempShortName;
        ImplGetMapName( pEntry->maSearchName, aTempShortName, pEntry->maMatchFamilyName,
                        pEntry->meMatchWeight, pEntry->meMatchWidth, pEntry->mnMatchType );
        pTempFontAttr = ImplGetFontNameAttr( pEntry->maSearchName );
        if ( !pTempFontAttr && (aTempShortName != pEntry->maSearchName) )
            pTempFontAttr = ImplGetFontNameAttr( aTempShortName );
        ImplCalcType( pEntry->mnMatchType, pEntry->meMatchWeight, pEntry->meMatchWidth,
                      pEntry->meFamily, pTempFontAttr );
        if ( ImplIsCJKFont( pEntry->maName ) )
            pEntry->mnMatchType |= IMPL_FONT_ATTR_CJK;

        pEntry = Next();
    }

    mbMatchData = TRUE;
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
    LanguageType eLanguage      = rFont.GetLanguage();
    FontWeight eWeight          = rFont.GetWeight();
    FontItalic eItalic          = rFont.GetItalic();
    FontPitch ePitch            = rFont.GetPitch();
    short nOrientation          = rFont.GetOrientation();
    BOOL bVertical              = rFont.IsVertical();

    // normalize orientation between 0 and 3600
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
             (eFamily       == pFontSelData->meFamily)    &&
             (ePitch        == pFontSelData->mePitch)     &&
             (nWidth        == pFontSelData->mnWidth)     &&
             (eCharSet      == pFontSelData->meCharSet)   &&
             (eLanguage     == pFontSelData->meLanguage)  &&
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

    const ImplCvtChar*      pConvertFontTab = NULL;
    ImplFontData*           pFontData = NULL;
    ImplDevFontListData*    pFoundData;
    String                  aSearchName;
    USHORT                  nSubstFlags1 = FONT_SUBSTITUTE_ALWAYS;
    USHORT                  nSubstFlags2 = FONT_SUBSTITUTE_ALWAYS;
    xub_StrLen              nFirstNameIndex = 0;
    xub_StrLen              nIndex = 0;
    int                     nToken = 0;
    ULONG                   i;

    if ( mbPrinter )
        nSubstFlags1 |= FONT_SUBSTITUTE_SCREENONLY;

    // Test if one Font in the name list is available
    do
    {
        nToken++;
        aSearchName = GetFontToken( rName, 0, nIndex );
        ImplGetEnglishSearchFontName( aSearchName );
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
                aSearchName = GetFontToken( rName, 0, nIndex );
                ImplGetEnglishSearchFontName( aSearchName );
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
            nIndex = 0;
            aSearchName = GetFontToken( rName, 0, nIndex );
            ImplGetEnglishSearchFontName( aSearchName );
        }

        const ImplFontNameAttr* pFontAttr = NULL;
        String                  aSearchShortName;
        String                  aSearchFamilyName;
        ULONG                   nSearchType = 0;
        FontWeight              eSearchWeight = eWeight;
        FontWidth               eSearchWidth = rFont.GetWidthType();
        BOOL                    bSymbolEncoding = (eCharSet == RTL_TEXTENCODING_SYMBOL);
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

        if ( !pFoundData && bSymbolEncoding )
            pFoundData = pFontList->ImplFindFontFromToken( aImplSubsStarSymbol );

        // If we haven't found a font, we try this with the other Font Token names, if availble
        if ( !pFoundData && (nToken > 1) )
        {
            while ( nIndex != STRING_NOTFOUND )
            {
                const ImplFontNameAttr* pTempFontAttr;
                String                  aTempName = GetFontToken( rName, 0, nIndex );
                String                  aTempShortName;
                String                  aTempFamilyName;
                ULONG                   nTempType = 0;
                FontWeight              eTempWeight = eWeight;
                FontWidth               eTempWidth = WIDTH_DONTKNOW;
                ImplGetEnglishSearchFontName( aTempName );
                ImplGetMapName( aTempName, aTempShortName, aTempFamilyName,
                                eTempWeight, eTempWidth, nTempType );

                // Temp, if ShortName is available
                if ( aTempShortName != aTempName )
                    pFoundData = pFontList->ImplFind( aTempShortName );

                if ( !pFoundData && aTempName.Len() )
                {
                    pTempFontAttr = ImplGetFontNameAttr( aTempName );
                    if ( !pTempFontAttr && (aTempShortName != aTempName) )
                        pTempFontAttr = ImplGetFontNameAttr( aTempShortName );

                    // Try Substitution
                    if ( pTempFontAttr && pTempFontAttr->mpSubstitution1 )
                    {
                        pFoundData = pFontList->ImplFindFontFromToken( pTempFontAttr->mpSubstitution1 );
                        if ( !pFoundData && pTempFontAttr->mpSubstitution2 )
                        {
                            pFoundData = pFontList->ImplFindFontFromToken( pTempFontAttr->mpSubstitution2 );
                            if ( !pFoundData && pTempFontAttr->mpSubstitution3 )
                            {
                                pFoundData = pFontList->ImplFindFontFromToken( pTempFontAttr->mpSubstitution3 );
                                if ( !pFoundData && pTempFontAttr->mpSubstitution4 )
                                    pFoundData = pFontList->ImplFindFontFromToken( pTempFontAttr->mpSubstitution4 );
                            }
                        }
                    }
                }
            }
        }

        // Try to find the font over the alias names
        if ( !pFoundData && pFontList->AreMapNamesAvailable() && aSearchName.Len() )
        {
            i = 0;
            do
            {
                ImplDevFontListData* pData = pFontList->Get( i );
                i++;

                // Test mapping names
                // If match one matching name, than this is our font!
                if ( pData->maMapNames.Len() )
                {
                    String      aTempName;
                    xub_StrLen  nIndex = 0;
                    do
                    {
                        aTempName = GetFontToken( pData->maMapNames, 0, nIndex );
                        // Test, if the Font name match with one of the mapping names
                        if ( (aTempName == aSearchName) || (aTempName == aSearchShortName) )
                        {
                            // Found - we use this font
                            pFoundData = pData;
                            break;
                        }
                    }
                    while ( nIndex != STRING_NOTFOUND );
                }
            }
            while ( !pFoundData && (i < nFontCount) );
        }

        // If we haven't found a font over the name, we try to find the best match over the attributes
        if ( !pFoundData && aSearchName.Len() )
        {
            ImplCalcType( nSearchType, eSearchWeight, eSearchWidth,
                          eFamily, pFontAttr );
            if ( (eItalic != ITALIC_NONE) && (eItalic != ITALIC_DONTKNOW) )
                nSearchType |= IMPL_FONT_ATTR_ITALIC;
            LanguageType eLang = rFont.GetLanguage();
            if ( (eLang == LANGUAGE_CHINESE) ||
                 (eLang == LANGUAGE_CHINESE_SIMPLIFIED) ||
                 (eLang == LANGUAGE_CHINESE_SINGAPORE) )
                nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC;
            else if ( (eLang == LANGUAGE_CHINESE_TRADITIONAL) ||
                      (eLang == LANGUAGE_CHINESE_HONGKONG) ||
                      (eLang == LANGUAGE_CHINESE_MACAU) )
                nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC;
            else if ( (eLang == LANGUAGE_KOREAN) ||
                      (eLang == LANGUAGE_KOREAN_JOHAB) )
                nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR;
            else if ( eLang == LANGUAGE_JAPANESE )
                nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP;
            else
            {
                if ( ImplIsCJKFont( rName ) )
                    nSearchType |= IMPL_FONT_ATTR_CJK;
            }
            if ( bSymbolEncoding )
                nSearchType |= IMPL_FONT_ATTR_SYMBOL;

            // We must only match, if we have something to match
            if ( nSearchType ||
                 ((eSearchWeight != WEIGHT_DONTKNOW) && (eSearchWeight != WEIGHT_NORMAL)) ||
                 ((eSearchWidth != WIDTH_DONTKNOW) && (eSearchWidth != WIDTH_NORMAL)) )
            {
                pFontList->InitMatchData();

                long    nTestMatch;
                long    nBestMatch = 40000;
                ULONG   nBestType = 0;
                for ( ULONG i = 0; i < nFontCount; i++ )
                {
                    ImplDevFontListData* pData = pFontList->Get( i );

                    // Get all information about the matching font
                    ULONG       nMatchType = pData->mnMatchType;
                    FontWeight  eMatchWeight = pData->meMatchWeight;
                    FontWidth   eMatchWidth = pData->meMatchWidth;

                    // Calculate Match Value
                    // 1000000000
                    //  100000000
                    //   10000000   CJK, CTL, None-Latin, Symbol
                    //    1000000   FamilyName, Script, Fixed, -Special, -Decorative,
                    //              Titling, Capitals, Outline, Shadow
                    //     100000   Match FamilyName, Serif, SansSerif, Italic,
                    //              Width, Weight
                    //      10000   Scalable, Standard, Default,
                    //              full, Normal, Knownfont,
                    //              Otherstyle, +Special, +Decorative,
                    //       1000   Typewriter, Rounded, Gothic, Schollbook
                    //        100
                    nTestMatch = 0;

                    // Test, if the choosen font should be CJK, CTL, None-Latin
                    if ( nSearchType & IMPL_FONT_ATTR_CJK )
                    {
                        // Matching language
                        if ( (nSearchType & IMPL_FONT_ATTR_CJK_ALLLANG) ==
                             (nMatchType & IMPL_FONT_ATTR_CJK_ALLLANG) )
                            nTestMatch += 10000000*3;
                        if ( nMatchType & IMPL_FONT_ATTR_CJK )
                            nTestMatch += 10000000*2;
                        if ( nMatchType & IMPL_FONT_ATTR_FULL )
                            nTestMatch += 10000000;
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_CJK )
                        nTestMatch -= 10000000;

                    if ( nSearchType & IMPL_FONT_ATTR_CTL )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_CTL )
                            nTestMatch += 10000000*2;
                        if ( nMatchType & IMPL_FONT_ATTR_FULL )
                            nTestMatch += 10000000;
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_CTL )
                        nTestMatch -= 10000000;

                    if ( nSearchType & IMPL_FONT_ATTR_NONELATIN )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_NONELATIN )
                            nTestMatch += 10000000*2;
                        if ( nMatchType & IMPL_FONT_ATTR_FULL )
                            nTestMatch += 10000000;
                    }

                    if ( nSearchType & IMPL_FONT_ATTR_SYMBOL )
                    {
                        // prefer some special known symbol fonts
                        if ( pData->maSearchName.EqualsAscii( "starsymbol" ) )
                            nTestMatch += 10000000*6+(10000*3);
                        else if ( pData->maSearchName.EqualsAscii( "opensymbol" ) )
                            nTestMatch += 10000000*6;
                        else if ( pData->maSearchName.EqualsAscii( "starbats" ) ||
                                  pData->maSearchName.EqualsAscii( "wingdings" ) ||
                                  pData->maSearchName.EqualsAscii( "monotypesorts" ) ||
                                  pData->maSearchName.EqualsAscii( "dingbats" ) ||
                                  pData->maSearchName.EqualsAscii( "zapfdingbats" ) )
                            nTestMatch += 10000000*5;
                        else if ( pData->mnTypeFaces & IMPL_DEVFONT_SYMBOL )
                            nTestMatch += 10000000*4;
                        else
                        {
                            if ( nMatchType & IMPL_FONT_ATTR_SYMBOL )
                                nTestMatch += 10000000*2;
                            if ( nMatchType & IMPL_FONT_ATTR_FULL )
                                nTestMatch += 10000000;
                        }
                    }
                    else if ( (pData->mnTypeFaces & (IMPL_DEVFONT_SYMBOL | IMPL_DEVFONT_NONESYMBOL)) == IMPL_DEVFONT_SYMBOL )
                        nTestMatch -= 10000000;
                    else if ( nMatchType & IMPL_FONT_ATTR_SYMBOL )
                        nTestMatch -= 10000;


                    if ( (aSearchFamilyName == pData->maMatchFamilyName) &&
                         aSearchFamilyName.Len() )
                        nTestMatch += 1000000*3;

                    if ( nSearchType & IMPL_FONT_ATTR_ALLSCRIPT )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_ALLSCRIPT )
                        {
                            nTestMatch += 1000000*2;
                            if ( nSearchType & IMPL_FONT_ATTR_ALLSUBSCRIPT )
                            {
                                if ( (nSearchType & IMPL_FONT_ATTR_ALLSUBSCRIPT) ==
                                     (nMatchType & IMPL_FONT_ATTR_ALLSUBSCRIPT) )
                                    nTestMatch += 1000000*2;
                                if ( (nSearchType & IMPL_FONT_ATTR_BRUSHSCRIPT) &&
                                     !(nMatchType & IMPL_FONT_ATTR_BRUSHSCRIPT) )
                                    nTestMatch -= 1000000;
                            }
                        }
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_ALLSCRIPT )
                        nTestMatch -= 1000000;

                    if ( nSearchType & IMPL_FONT_ATTR_FIXED )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_FIXED )
                        {
                            nTestMatch += 1000000*2;
                            // Typewriter has now a higher prio
                            if ( (nSearchType & IMPL_FONT_ATTR_TYPEWRITER) &&
                                 (nMatchType & IMPL_FONT_ATTR_TYPEWRITER) )
                                 nTestMatch += 10000*2;
                        }
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_FIXED )
                        nTestMatch -= 1000000;

                    if ( nSearchType & IMPL_FONT_ATTR_SPECIAL )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_SPECIAL )
                            nTestMatch += 10000;
                        else if ( !(nSearchType & IMPL_FONT_ATTR_ALLSERIFSTYLE) )
                        {
                            if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                                nTestMatch += 1000*2;
                            else if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                                nTestMatch += 1000;
                        }
                    }
                    else if ( (nMatchType & IMPL_FONT_ATTR_SPECIAL) &&
                              !(nSearchType & IMPL_FONT_ATTR_SYMBOL) )
                        nTestMatch -= 1000000;
                    if ( nSearchType & IMPL_FONT_ATTR_DECORATIVE )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_DECORATIVE )
                            nTestMatch += 10000;
                        else if ( !(nSearchType & IMPL_FONT_ATTR_ALLSERIFSTYLE) )
                        {
                            if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                                nTestMatch += 1000*2;
                            else if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                                nTestMatch += 1000;
                        }
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_DECORATIVE )
                        nTestMatch -= 1000000;

                    if ( nSearchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
                    {
                        if ( nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
                            nTestMatch += 1000000*2;
                        if ( (nSearchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS)) ==
                             (nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS)) )
                            nTestMatch += 1000000;
                        else if ( (nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS)) &&
                                  (nMatchType & (IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT)) )
                            nTestMatch += 1000000;
                    }
                    else if ( nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
                        nTestMatch -= 1000000;
                    if ( nSearchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
                    {
                        if ( nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
                            nTestMatch += 1000000*2;
                        if ( (nSearchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW)) ==
                             (nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW)) )
                            nTestMatch += 1000000;
                        else if ( (nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW)) &&
                                  (nMatchType & (IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT)) )
                            nTestMatch += 1000000;
                    }
                    else if ( nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
                        nTestMatch -= 1000000;


                    if ( (aSearchFamilyName.Len() && pData->maMatchFamilyName.Len()) &&
                         ((aSearchFamilyName.Search( pData->maMatchFamilyName ) != STRING_NOTFOUND) ||
                          (pData->maMatchFamilyName.Search( aSearchFamilyName ) != STRING_NOTFOUND)) )
                        nTestMatch += 100000*2;

                    if ( nSearchType & IMPL_FONT_ATTR_SERIF )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                            nTestMatch += 1000000*2;
                        else if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                            nTestMatch -= 1000000;
                    }

                    if ( nSearchType & IMPL_FONT_ATTR_SANSSERIF )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                            nTestMatch += 1000000;
                        else if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                            nTestMatch -= 1000000;
                    }

                    if ( nSearchType & IMPL_FONT_ATTR_ITALIC )
                    {
                        if ( pData->mnTypeFaces & IMPL_DEVFONT_ITALIC )
                            nTestMatch += 1000000*3;
                        if ( nMatchType & IMPL_FONT_ATTR_ITALIC )
                            nTestMatch += 1000000;
                    }
                    else if ( !(nSearchType & IMPL_FONT_ATTR_ALLSCRIPT) &&
                              ((nMatchType & IMPL_FONT_ATTR_ITALIC) ||
                               !(pData->mnTypeFaces & IMPL_DEVFONT_NONEITALIC)) )
                        nTestMatch -= 1000000*2;

                    if ( (eSearchWidth != WIDTH_DONTKNOW) && (eSearchWidth != WIDTH_NORMAL) )
                    {
                        if ( eSearchWidth < WIDTH_NORMAL )
                        {
                            if ( eSearchWidth == eMatchWidth )
                                nTestMatch += 1000000*3;
                            else if ( (eMatchWidth < WIDTH_NORMAL) && (eMatchWidth != WIDTH_DONTKNOW) )
                                nTestMatch += 1000000;
                        }
                        else
                        {
                            if ( eSearchWidth == eMatchWidth )
                                nTestMatch += 1000000*3;
                            else if ( eMatchWidth > WIDTH_NORMAL )
                                nTestMatch += 1000000;
                        }
                    }
                    else if ( (eMatchWidth != WIDTH_DONTKNOW) && (eMatchWidth != WIDTH_NORMAL) )
                        nTestMatch -= 1000000;

                    if ( (eSearchWeight != WEIGHT_DONTKNOW) && (eSearchWeight != WEIGHT_NORMAL) && (eSearchWeight != WEIGHT_MEDIUM) )
                    {
                        if ( eSearchWeight < WEIGHT_NORMAL )
                        {
                            if ( pData->mnTypeFaces & IMPL_DEVFONT_LIGHT )
                                nTestMatch += 1000000;
                            if ( (eMatchWeight < WEIGHT_NORMAL) && (eMatchWeight != WEIGHT_DONTKNOW) )
                                nTestMatch += 1000000;
                        }
                        else
                        {
                            if ( pData->mnTypeFaces & IMPL_DEVFONT_BOLD )
                                nTestMatch += 1000000;
                            if ( eMatchWeight > WEIGHT_BOLD )
                                nTestMatch += 1000000;
                        }
                    }
                    else if ( ((eMatchWeight != WEIGHT_DONTKNOW) && (eMatchWeight != WEIGHT_NORMAL) && (eMatchWeight != WEIGHT_MEDIUM)) ||
                              !(pData->mnTypeFaces & IMPL_DEVFONT_NORMAL) )
                        nTestMatch -= 1000000;

                    // skalierbare Schriften haben schon einen echten Vorteil
                    // gegenueber nicht skalierbaren Schriften
                    if ( pData->mnTypeFaces & IMPL_DEVFONT_SCALABLE )
                        nTestMatch += 10000*4;
                    else
                        nTestMatch -= 10000*4;
                    if ( nMatchType & IMPL_FONT_ATTR_STANDARD )
                        nTestMatch += 10000*2;
                    if ( nMatchType & IMPL_FONT_ATTR_DEFAULT )
                        nTestMatch += 10000;
                    if ( nMatchType & IMPL_FONT_ATTR_FULL )
                        nTestMatch += 10000;
                    if ( nMatchType & IMPL_FONT_ATTR_NORMAL )
                        nTestMatch += 10000;
                    if ( nMatchType & IMPL_FONT_ATTR_OTHERSTYLE )
                    {
                        if ( !(nMatchType & IMPL_FONT_ATTR_OTHERSTYLE) )
                            nTestMatch -= 10000;
                    }
                    else if ( nMatchType & IMPL_FONT_ATTR_OTHERSTYLE )
                        nTestMatch -= 10000;

                    if ( (nSearchType & IMPL_FONT_ATTR_ROUNDED) ==
                         (nMatchType & IMPL_FONT_ATTR_ROUNDED) )
                         nTestMatch += 1000;
                    if ( (nSearchType & IMPL_FONT_ATTR_TYPEWRITER) ==
                         (nMatchType & IMPL_FONT_ATTR_TYPEWRITER) )
                         nTestMatch += 1000;
                    if ( nSearchType & IMPL_FONT_ATTR_GOTHIC )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_GOTHIC )
                            nTestMatch += 1000*3;
                        if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                            nTestMatch += 1000*2;
                    }
                    if ( nSearchType & IMPL_FONT_ATTR_SCHOOLBOOK )
                    {
                        if ( nMatchType & IMPL_FONT_ATTR_SCHOOLBOOK )
                            nTestMatch += 1000*3;
                        if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                            nTestMatch += 1000*2;
                    }

                    if ( nTestMatch > nBestMatch )
                    {
                        pFoundData  = pData;
                        nBestMatch  = nTestMatch;
                        nBestType   = nMatchType;
                    }
                    else if ( nTestMatch == nBestMatch )
                    {
                        // We have some fonts, where we think that they are more useful
                        if ( nMatchType & IMPL_FONT_ATTR_DEFAULT )
                        {
                            pFoundData  = pData;
                            nBestType   = nMatchType;
                        }
                        else if ( (nMatchType & IMPL_FONT_ATTR_STANDARD) &&
                                  !(nBestType & IMPL_FONT_ATTR_DEFAULT) )
                        {
                            pFoundData  = pData;
                            nBestType   = nMatchType;
                        }
                    }
                }

                // Overwrite Attributes
                if ( pFoundData )
                {
                    if ( (eSearchWeight >= WEIGHT_BOLD) &&
                         (eSearchWeight > eWeight) &&
                         (pFoundData->mnTypeFaces & IMPL_DEVFONT_BOLD) )
                        eWeight = eSearchWeight;
                    if ( (eSearchWeight < WEIGHT_NORMAL) &&
                         (eSearchWeight < eWeight) &&
                         (eSearchWeight != WEIGHT_DONTKNOW) &&
                         (pFoundData->mnTypeFaces & IMPL_DEVFONT_LIGHT) )
                        eWeight = eSearchWeight;
                    if ( (nSearchType & IMPL_FONT_ATTR_ITALIC) &&
                         ((eItalic == ITALIC_DONTKNOW) || (eItalic == ITALIC_NONE)) &&
                         (pFoundData->mnTypeFaces & IMPL_DEVFONT_ITALIC) )
                        eItalic = ITALIC_NORMAL;
                }
            }
        }

        if ( !pFoundData )
        {
            // Try to use a Standard Unicode or a Standard Font to get
            // as max as possible characters
            pFoundData = pFontList->ImplFindFontFromToken( aImplSubsSansUnicode );
            if ( !pFoundData )
            {
                pFoundData = pFontList->ImplFindFontFromToken( aImplSubsSans );
                if ( !pFoundData )
                {
                    pFoundData = pFontList->ImplFindFontFromToken( aImplSubsSerif );
                    if ( !pFoundData )
                        pFoundData = pFontList->ImplFindFontFromToken( aImplSubsFixed );
                }
            }

            // If nothing helps, we use the first one
            if ( !pFoundData )
                pFoundData = pFontList->Get( 0 );
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

            if ( (rName == pCurFontData->maName) ||
                 rName.EqualsIgnoreCaseAscii( pCurFontData->maName ) )
                nMatch += 240000;

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

        // If we haven't the font, try to find a Font-TextConversion table
        if ( aSearchName != pFoundData->maSearchName )
            pConvertFontTab = ImplGetRecodeData( aSearchName, pFoundData->maSearchName );
    }

    // Daten initialisieren und in die Liste aufnehmen
    pEntry                          = new ImplFontEntry;
    pEntry->mpNext                  = mpFirstEntry;
    pEntry->mpConversion            = pConvertFontTab;
    pEntry->mnWidthInit             = 0;
    pEntry->mnWidthAryCount         = 0;
    pEntry->mnWidthArySize          = 0;
    pEntry->mpWidthAry              = NULL;
    pEntry->mpKernPairs             = NULL;
    pEntry->mnOwnOrientation        = 0;
    pEntry->mnOrientation           = 0;
    pEntry->mbInit                  = FALSE;
    pEntry->mnRefCount              = 1;
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
    pFontSelData->meLanguage        = eLanguage;
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

            pFontEntry->maMetric.mnAscent       = 0;
            pFontEntry->maMetric.mnDescent      = 0;
            pFontEntry->maMetric.mnLeading      = 0;
            pFontEntry->maMetric.mnSlant        = 0;
            pFontEntry->maMetric.mnFirstChar    = 0;
            pFontEntry->maMetric.mnLastChar     = 0;
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
                pFontEntry->maMetric.maName     = GetFontToken( pFontEntry->maFontSelData.maName, 0 );
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
            // Query the first latin area, because we need a
            // WidthFactor for some methods (e.g. GetTextBreak)
            pFontEntry->mnWidthFactor = pGraphics->GetCharWidth( IMPL_CACHE_A1_FIRST, IMPL_CACHE_A1_LAST,
                                                                 pFontEntry->maWidthAry+IMPL_CACHE_A1_INDEX );
            if ( pFontEntry->mnWidthFactor )
                pFontEntry->mnWidthInit |= IMPL_CACHE_A1_BIT;
            else
                pFontEntry->mnWidthFactor = IMPL_FACTOR_NOTINIT;
#else
            long nFactor = 0;
            pGraphics->GetFontMetric(
                pFontEntry->maMetric,
                nFactor, IMPL_CACHE_A1_FIRST, IMPL_CACHE_A1_LAST,
                pFontEntry->maWidthAry+IMPL_CACHE_A1_INDEX,
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
    ImplFontEntry* pFontEntry = mpFontEntry;

    USHORT nChar = (USHORT)c;
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

static void ImplSortKernPairs( ImplKernPairData* pKernPairs, long l, long r )
{
    long                i = l;
    long                j = r;
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
            ImplSortKernPairs( pFontEntry->mpKernPairs, 0, (long)pFontEntry->mnKernPairs-1 );
    }
}

// -----------------------------------------------------------------------

// returns asian kerning values in quarter of character width units
// to enable automatic halfwidth substitution for fullwidth punctuation
// return value is negative for l, positive for r, zero for neutral

// If the range doesn't match in 0x3000 and 0x30FB, please change
// also ImplCalcKerning.

static int CalcAsianKerning( sal_Unicode c, bool bLeft )
{
    // http://www.asahi-net.or.jp/~sd5a-ucd/freetexts/jis/x4051/1995/appendix.html
    static signed char nTable[0x30] =
    {
         0, -2, -2,  0,   0,  0,  0,  0,  +2, -2, +2, -2,  +2, -2, +2, -2,
        +2, -2,  0,  0,  +2, -2, +2, -2,   0,  0,  0,  0,   0, +2, -2, -2,
         0,  0,  0,  0,   0,  0,  0,  0,   0,  0, -2, -2,  +2, +2, -2, -2
    };

    int nResult;
    if( c>=0x3000 && c<0x3030 )
        nResult = nTable[ c - 0x3000 ];
    else switch( c )
    {
        // TODO: only for monospaced font, but how to test if it is all monospaced?
        //case ':': case ';': case '!':
        case 0x30FB:
            nResult = bLeft ? -1 : +1; break;   // 25% left and right
        default:
            nResult = 0; break;
    }
    return nResult;
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

    if( (maFont.GetKerning() & KERNING_FONTSPECIFIC) && nKernPairs )
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

    if ( (maFont.GetKerning() & KERNING_ASIAN) &&
         (ImplGetCharWidth(0x3001) == ImplGetCharWidth(0x3007)) )  // monospaced font?
    {
        for( i = 0; i < nLen-1; ++i )
        {
            sal_Unicode nFirst = pStr[i];
            sal_Unicode nNext  = pStr[i+1];

            if ( (nFirst >= 0x3000) && (nNext >= 0x3000) &&
                 (nFirst <= 0x30FB) && (nNext <= 0x30FB) )
            {
                long nKernFirst = +CalcAsianKerning( nFirst, true );
                long nKernNext  = -CalcAsianKerning( nNext, false );

                long nAmount = (nKernFirst < nKernNext) ? nKernFirst : nKernNext;
                if( nAmount<0 && nKernFirst!=0 && nKernNext!=0 )
                {
                    nAmount *= ImplGetCharWidth( nFirst );
                    nAmount /= 4 * mpFontEntry->mnWidthFactor;
                    nWidth += nAmount;
                    for( xub_StrLen n = i; n < nAryLen; ++n )
                        pDXAry[n] += nAmount;
                }
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
    Color           aUnderlineColor = GetTextLineColor();
    Color           aStrikeoutColor = GetTextColor();
    long            nBaseY = nY;
    long            nLineHeight;
    long            nLinePos;
    long            nLinePos2;
    long            nLeft;
    BOOL            bNormalLines = TRUE;

    if ( !IsTextLineColor() )
        aUnderlineColor = GetTextColor();

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
                              nLineWidth, mpFontEntry->mnOrientation, aUnderlineColor );
            nLinePos += nLineWidthHeight+nLineDY;
            ImplDrawWaveLine( nBaseX, nBaseY, nX, nLinePos, nWidth, nLineHeight,
                              nLineWidth, mpFontEntry->mnOrientation, aUnderlineColor );
        }
        else
        {
            nLinePos -= nLineWidthHeight/2;
            ImplDrawWaveLine( nBaseX, nBaseY, nX, nLinePos, nWidth, nLineHeight,
                              nLineWidth, mpFontEntry->mnOrientation, aUnderlineColor );
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
        SetTextColor( aStrikeoutColor );
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
#ifdef REMOTE_APPSERVER
        Color aOldLineColor = GetLineColor();
        Color aOldFillColor = GetFillColor();
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
#ifndef REMOTE_APPSERVER
            if ( mbLineColor || mbInitLineColor )
            {
                mpGraphics->SetLineColor();
                mbInitLineColor = TRUE;
            }
            mpGraphics->SetFillColor( ImplColorToSal( aUnderlineColor ) );
            mbInitFillColor = TRUE;
#else
            SetLineColor();
            SetFillColor( aUnderlineColor );
            if ( mbInitLineColor )
                ImplInitLineColor();
            if ( mbInitFillColor )
                ImplInitFillColor();
#endif

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
#ifndef REMOTE_APPSERVER
            if ( mbLineColor || mbInitLineColor )
            {
                mpGraphics->SetLineColor();
                mbInitLineColor = TRUE;
            }
            mpGraphics->SetFillColor( ImplColorToSal( aStrikeoutColor ) );
            mbInitFillColor = TRUE;
#else
            SetLineColor();
            SetFillColor( aStrikeoutColor );
            if ( mbInitLineColor )
                ImplInitLineColor();
            if ( mbInitFillColor )
                ImplInitFillColor();
#endif

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
                               aAccentPolyFlags );
                double dScale = ((double)nDotSize)/1000.0;
                aPoly.Scale( dScale, dScale );
                Polygon aTemp;
                aPoly.GetSimple( aTemp );
                Rectangle aBoundRect = aTemp.GetBoundRect();
                rWidth = aBoundRect.GetWidth();
                nDotSize = aBoundRect.GetHeight();
                rPolyPoly.Insert( aTemp );
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
    long                nEmphasisWidth2;
    long                nEmphasisHeight2;
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

    nEmphasisWidth2     = nEmphasisWidth / 2;
    nEmphasisHeight2    = nEmphasisHeight / 2;
    nOffY += nEmphasisHeight2;

    while ( i < nLen )
    {
        if ( ImplIsLineCharacter( *(pStr+i) ) )
        {
            long nStartX = ImplGetTextWidth( pStr, i, pDXAry );
            long nEndX = ImplGetTextWidth( pStr, i+1, pDXAry );
            long nOutX = nOffX + nStartX + ((nEndX-nStartX-nEmphasisWidth)/2) + nEmphasisWidth2;
            long nOutY = nOffY;
            if ( mpFontEntry->mnOrientation )
                ImplRotatePos( nBaseX, nBaseY, nOutX, nOutY, mpFontEntry->mnOrientation );
            nOutX -= nEmphasisWidth2;
            nOutY -= nEmphasisHeight2;
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
            if ( (GetTextColor().GetColor() == COL_BLACK) ||
                 (GetTextColor().GetLuminance() < 8) )
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
            ImplDrawTextDirect( nX-1, nY-1, pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX+1, nY+1, pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX-1, nY+1, pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX,   nY+1, pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX-1, nY,   pStr, nLen, pDXAry, mbTextLines );
            ImplDrawTextDirect( nX+1, nY,   pStr, nLen, pDXAry, mbTextLines );
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

void OutputDevice::DrawText( const Point& rStartPt, const String& rOrigStr,
                             xub_StrLen nIndex, xub_StrLen nLen )
{
    DBG_TRACE( "OutputDevice::DrawText()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAction( rStartPt, rOrigStr, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    // get string length for calculating extents
    if ( (ULONG)nLen+nIndex > rOrigStr.Len() )
    {
        if ( nIndex < rOrigStr.Len() )
            nLen = rOrigStr.Len() - nIndex;
        else
            nLen = 0;
    }

    // don't bother if there is nothing to do
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

    String aStr = rOrigStr;
    if( mpFontEntry->mpConversion )
        ImplRecodeString( mpFontEntry->mpConversion, aStr, nIndex, nLen );

    Point aStartPt = ImplLogicToDevicePixel( rStartPt );

    // Pointer auf den String-Buffer setzen und um den Index korrigieren
    const sal_Unicode* pStr = aStr.GetBuffer();
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

long OutputDevice::GetTextWidth( const String& rOrigStr,
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

    if ( nIndex < rOrigStr.Len() )
    {
        // String-Laenge fuer die Ermittlung der Groesse setzen
        if ( (ULONG)nLen+nIndex > rOrigStr.Len() )
            nLen = rOrigStr.Len() - nIndex;

        if ( nLen )
        {
            String aStr = rOrigStr;
            if( pFontEntry->mpConversion )
                ImplRecodeString( pFontEntry->mpConversion, aStr, nIndex, nLen );

            // Also Fixed-Fonts are calculated char by char, because
            // not every Font or in every CJK Fonts all characters have
            // the same width
            const sal_Unicode*  pStr = aStr.GetBuffer();
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

void OutputDevice::DrawTextArray( const Point& rStartPt, const String& rOrigStr,
                                  const long* pDXAry,
                                  xub_StrLen nIndex, xub_StrLen nLen )
{
    DBG_TRACE( "OutputDevice::DrawTextArray()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextArrayAction( rStartPt, rOrigStr, pDXAry, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    // get string length for calculating text extents
    if ( (ULONG)nLen+nIndex > rOrigStr.Len() )
    {
        if ( nIndex < rOrigStr.Len() )
            nLen = rOrigStr.Len() - nIndex;
        else
            nLen = 0;
    }

    // don't bother if there is nothing to do
    if ( !nLen )
        return;

    // use DrawText when there is no position array
    if ( !pDXAry || (nLen < 2) )
    {
        // calls here to prevent double recording in metafile
        DrawText( rStartPt, rOrigStr, nIndex, nLen );
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

    String aStr = rOrigStr;
    if( mpFontEntry->mpConversion )
        ImplRecodeString( mpFontEntry->mpConversion, aStr, nIndex, nLen );

    // Pointer auf den String-Buffer setzen und um den Index korrigieren
    const sal_Unicode* pStr = aStr.GetBuffer() + nIndex;

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

long OutputDevice::GetTextArray( const String& rOrigStr, long* pDXAry,
                                 xub_StrLen nIndex, xub_StrLen nLen ) const
{
    DBG_TRACE( "OutputDevice::GetTextArray()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( !pDXAry )
        return GetTextWidth( rOrigStr, nIndex, nLen );

    // get string length for calculating text extents
    if ( (ULONG)nLen+nIndex > rOrigStr.Len() )
    {
        if ( nIndex < rOrigStr.Len() )
            nLen = rOrigStr.Len() - nIndex;
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

    ImplFontEntry* pFontEntry = mpFontEntry;

    String aStr = rOrigStr;
    if( pFontEntry->mpConversion )
        ImplRecodeString( pFontEntry->mpConversion, aStr, nIndex, nLen );

    const sal_Unicode*  pStr = aStr.GetBuffer() + nIndex;
    const sal_Unicode*  pTempStr = pStr;
    long                nOffset = 0;
    xub_StrLen          i;

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
                                    const String& rOrigStr,
                                    xub_StrLen nIndex, xub_StrLen nLen )
{
    DBG_TRACE( "OutputDevice::DrawStretchText()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaStretchTextAction( rStartPt, nWidth, rOrigStr, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    // get string string length
    if ( (ULONG)nLen+nIndex > rOrigStr.Len() )
    {
        if ( nIndex < rOrigStr.Len() )
            nLen = rOrigStr.Len() - nIndex;
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

    String aStr = rOrigStr;
    if( mpFontEntry->mpConversion )
        ImplRecodeString( mpFontEntry->mpConversion, aStr, nIndex, nLen );

    Point aStartPt = ImplLogicToDevicePixel( rStartPt );
    nWidth = ImplLogicWidthToDevicePixel( nWidth );

    // Pointer auf den String-Buffer setzen und um den Index korrigieren
    const sal_Unicode* pStr = aStr.GetBuffer() + nIndex;

    // Breiten-Array fuer errechnete Werte allocieren und
    // mit den Breiten der einzelnen Character fuellen lassen
    long  aStackAry[128];
    long* pDXAry = (long*)ImplGetStackBuffer( sizeof(long)*nLen, aStackAry, sizeof( aStackAry ) );
    ImplFillDXAry( pDXAry, pStr, nLen, (long)nWidth );
    ImplDrawText( aStartPt.X(), aStartPt.Y(), pStr, nLen, pDXAry );
    ImplReleaseStackBuffer( pDXAry, aStackAry );
}

// -----------------------------------------------------------------------

xub_StrLen OutputDevice::GetTextBreak( const String& rOrigStr, long nTextWidth,
                                       xub_StrLen nIndex, xub_StrLen nLen,
                                       long nCharExtra ) const
{
    DBG_TRACE( "OutputDevice::GetTextBreak()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( nIndex > rOrigStr.Len() )
        return 0;

    if ( mbNewFont )
    {
        if ( !((OutputDevice*)this)->ImplNewFont() )
            return 0;
    }

    long nWidthFactor = 1000;
    if ( nWidthFactor < mpFontEntry->mnWidthFactor )
        nWidthFactor = mpFontEntry->mnWidthFactor;

    nCharExtra *= nWidthFactor;

    if ( mbMap )
    {
        nTextWidth = ImplLogicWidthToDevicePixel( nTextWidth * 8 );
        nTextWidth *= (nWidthFactor + 4) / 8;
        if ( nCharExtra )
            nCharExtra = ImplLogicWidthToDevicePixel( nCharExtra );
    }
    else
        nTextWidth *= nWidthFactor;

    String aStr = rOrigStr;
    if( mpFontEntry->mpConversion )
        ImplRecodeString( mpFontEntry->mpConversion, aStr, nIndex, nLen );

    // calc last index position
    ULONG nLastIndex = (ULONG)nIndex + nLen;
    if ( nLastIndex > aStr.Len() )
        nLastIndex = aStr.Len();

    long nCalcWidth = 0;
    const sal_Unicode* pStr = aStr.GetBuffer() + nIndex;
    for(; nIndex < nLastIndex; ++nIndex, ++pStr )
    {
        nCalcWidth += (ImplGetCharWidth(*pStr) * nWidthFactor) / mpFontEntry->mnWidthFactor;

        if ( nCalcWidth > nTextWidth )
            return nIndex;

        // apply kerning
        if ( mbKerning )
            nCalcWidth += ImplCalcKerning( pStr, 2, NULL, 0 ) * nWidthFactor;
        nCalcWidth += nCharExtra;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

xub_StrLen OutputDevice::GetTextBreak( const String& rOrigStr, long nTextWidth,
                                       sal_Unicode nExtraChar, xub_StrLen& rExtraCharPos,
                                       xub_StrLen nIndex, xub_StrLen nLen,
                                       long nCharExtra ) const
{
    DBG_TRACE( "OutputDevice::GetTextBreak()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( nIndex > rOrigStr.Len() )
        return 0;

    if ( mbNewFont )
    {
        if ( !((OutputDevice*)this)->ImplNewFont() )
            return 0;
    }

    long nWidthFactor = 1000;
    if ( mpFontEntry->mnWidthFactor > nWidthFactor )
        nWidthFactor = mpFontEntry->mnWidthFactor;

    nCharExtra *= nWidthFactor;

    if ( mbMap )
    {
        nTextWidth = ImplLogicWidthToDevicePixel( nTextWidth * 8 );
        nTextWidth *= (nWidthFactor + 4) / 8;
        if ( nCharExtra )
            nCharExtra = ImplLogicWidthToDevicePixel( nCharExtra );
    }
    else
        nTextWidth *= nWidthFactor;

    String aStr = rOrigStr;
    if( mpFontEntry->mpConversion )
        ImplRecodeString( mpFontEntry->mpConversion, aStr, nIndex, nLen );

    // calc last index position
    ULONG nLastIndex = (ULONG)nIndex + nLen;
    if ( nLastIndex > aStr.Len() )
        nLastIndex = aStr.Len();

    xub_StrLen nIndex2 = STRING_LEN;
    long nTextWidth2 = nTextWidth;
    nTextWidth2 -= (ImplGetCharWidth(nExtraChar) * nWidthFactor) / mpFontEntry->mnWidthFactor;
    nTextWidth2 -= nCharExtra;
    if ( nTextWidth2 < 0 )
        nIndex2 = 0;

    long nCalcWidth = 0;
    const sal_Unicode* pStr = aStr.GetBuffer() + nIndex;
    for(; nIndex < nLastIndex; ++nIndex, ++pStr )
    {
        nCalcWidth += (ImplGetCharWidth(*pStr) * nWidthFactor) / mpFontEntry->mnWidthFactor;

        if ( nCalcWidth > nTextWidth2 )
        {
            if ( nIndex2 == STRING_LEN )
                nIndex2 = nIndex;
        }
        if ( nCalcWidth > nTextWidth )
        {
            rExtraCharPos = (nIndex2 == STRING_LEN) ? nIndex : nIndex2;
            return nIndex;
        }

        // apply kerning
        if ( mbKerning )
            nCalcWidth += ImplCalcKerning( pStr, 2, NULL, 0 ) * nWidthFactor;
        nCalcWidth += nCharExtra;
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
            sal_Unicode c = nFirstChar;
            if( mpFontEntry->mpConversion )
                c = ImplRecodeChar( mpFontEntry->mpConversion, c );
            *pWidthAry = ImplDevicePixelToLogicWidth( ImplGetCharWidth( c ) ) / mpFontEntry->mnWidthFactor;
            pWidthAry++;
            nFirstChar++;
            nCharCount--;
        }
    }
    else
    {
        while ( nCharCount )
        {
            sal_Unicode c = nFirstChar;
            if( mpFontEntry->mpConversion )
                c = ImplRecodeChar( mpFontEntry->mpConversion, c );
            *pWidthAry = ImplGetCharWidth( c ) / mpFontEntry->mnWidthFactor;
            pWidthAry++;
            nFirstChar++;
            nCharCount--;
        }
    }
}

// -----------------------------------------------------------------------

void OutputDevice::DrawText( const Rectangle& rRect,
                             const String& rOrigStr, USHORT nStyle )
{
    DBG_TRACE( "OutputDevice::DrawText( const Rectangle& )" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextRectAction( rRect, rOrigStr, nStyle ) );

    if ( !IsDeviceOutputNecessary() || !rOrigStr.Len() || rRect.IsEmpty() )
        return;

    // better call it here because ImplDrawMnemonicLine() won't
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
        DrawText( aRect, rOrigStr, nStyle & ~TEXT_DRAW_DISABLE );
        SetTextColor( GetSettings().GetStyleSettings().GetShadowColor() );
    }

    long        nWidth          = rRect.GetWidth();
    long        nHeight         = rRect.GetHeight();

    if ( ((nWidth <= 0) || (nHeight <= 0)) && (nStyle & TEXT_DRAW_CLIP) )
        return;

    Point       aPos            = rRect.TopLeft();
    long        nTextHeight     = GetTextHeight();
    TextAlign   eAlign          = GetTextAlign();
    xub_StrLen  nMnemonicPos    = STRING_NOTFOUND;

    String aStr = rOrigStr;
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
                        if( mpFontEntry->mpConversion )
                            cMnemonic = ImplRecodeChar( mpFontEntry->mpConversion, cMnemonic );
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
            nMnemonicX = mnOutOffX + aTempPos.X() + ImplLogicWidthToDevicePixel( GetTextWidth( aStr, 0, nMnemonicPos ) );
            nMnemonicY = mnOutOffY + aTempPos.Y() + ImplLogicWidthToDevicePixel( GetFontMetric().GetAscent() );
            cMnemonic  = aStr.GetChar( nMnemonicPos );
            if( mpFontEntry->mpConversion )
                cMnemonic = ImplRecodeChar( mpFontEntry->mpConversion, cMnemonic );
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
                                     const String& rOrigStr, USHORT nStyle,
                                     TextRectInfo* pInfo ) const
{
    DBG_TRACE( "OutputDevice::GetTextRect()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Rectangle           aRect = rRect;
    xub_StrLen          nLines;
    long                nWidth = rRect.GetWidth();
    long                nMaxWidth;
    long                nTextHeight = GetTextHeight();

    String aStr = rOrigStr;
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

String OutputDevice::GetEllipsisString( const String& rOrigStr, long nMaxWidth,
                                        USHORT nStyle ) const
{
    DBG_TRACE( "OutputDevice::GetEllipsisString()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    String aStr = rOrigStr;
    xub_StrLen nIndex = GetTextBreak( aStr, nMaxWidth );

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
                aStr += rOrigStr.GetChar( 0 );
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
            xub_StrLen nLastContent = aStr.Len();
            while ( nLastContent )
            {
                nLastContent--;
                if ( ImplIsCharIn( aStr.GetChar( nLastContent ), pSepChars ) )
                    break;
            }
            while ( nLastContent &&
                    ImplIsCharIn( aStr.GetChar( nLastContent-1 ), pSepChars ) )
                nLastContent--;

            XubString aLastStr( aStr, nLastContent, aStr.Len() );
            XubString aTempLastStr( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
            aTempLastStr += aLastStr;
            if ( GetTextWidth( aTempLastStr ) > nMaxWidth )
                aStr = GetEllipsisString( aStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS );
            else
            {
                USHORT nFirstContent = 0;
                while ( nFirstContent < nLastContent )
                {
                    nFirstContent++;
                    if ( ImplIsCharIn( aStr.GetChar( nFirstContent ), pSepChars ) )
                        break;
                }
                while ( (nFirstContent < nLastContent) &&
                        ImplIsCharIn( aStr.GetChar( nFirstContent ), pSepChars ) )
                    nFirstContent++;

                if ( nFirstContent >= nLastContent )
                    aStr = GetEllipsisString( aStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS );
                else
                {
                    if ( nFirstContent > 4 )
                        nFirstContent = 4;
                    XubString aFirstStr( aStr, 0, nFirstContent );
                    aFirstStr.AppendAscii( "..." );
                    XubString aTempStr = aFirstStr;
                    aTempStr += aLastStr;
                    if ( GetTextWidth( aTempStr ) > nMaxWidth )
                        aStr = GetEllipsisString( aStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS );
                    else
                    {
                        do
                        {
                            aStr = aTempStr;
                            while ( nFirstContent < nLastContent )
                            {
                                nLastContent--;
                                if ( ImplIsCharIn( aStr.GetChar( nLastContent ), pSepChars ) )
                                    break;
                            }
                            while ( (nFirstContent < nLastContent) &&
                                    ImplIsCharIn( aStr.GetChar( nLastContent-1 ), pSepChars ) )
                                nLastContent--;

                            if ( nFirstContent < nLastContent )
                            {
                                XubString aTempLastStr( aStr, nLastContent, aStr.Len() );
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

long OutputDevice::GetCtrlTextWidth( const String& rStr,
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

String OutputDevice::GetNonMnemonicString( const String& rStr, xub_StrLen& rMnemonicPos )
{
    String   aStr    = rStr;
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

    // We want set correct Family and Pitch data, if we can't query the
    // data from the system
    if ( (aMetric.GetFamily() == FAMILY_DONTKNOW) ||
         (aMetric.GetPitch() == PITCH_DONTKNOW) )
    {
        const ImplFontNameAttr* pTempFontAttr;
        String                  aTempName = pMetric->maName;
        String                  aTempShortName;
        String                  aTempFamilyName;
        ULONG                   nTempType = 0;
        FontWeight              eTempWeight = WEIGHT_DONTKNOW;
        FontWidth               eTempWidth = WIDTH_DONTKNOW;
        ImplGetEnglishSearchFontName( aTempName );
        ImplGetMapName( aTempName, aTempShortName, aTempFamilyName,
                        eTempWeight, eTempWidth, nTempType );
        pTempFontAttr = ImplGetFontNameAttr( aTempName );
        if ( !pTempFontAttr && (aTempShortName != aTempName) )
            pTempFontAttr = ImplGetFontNameAttr( aTempShortName );
        if ( pTempFontAttr && pTempFontAttr->mpHTMLSubstitution )
        {
            // We use the HTML-Substitution string to overwrite these only
            // for standard fonts - other fonts should be the default, because
            // it's not easy to define the correct definition (for example
            // for none latin fonts or special fonts).
            if ( aMetric.GetFamily() == FAMILY_DONTKNOW )
            {
                if ( pTempFontAttr->mpHTMLSubstitution == aImplHTMLSubsSerif )
                    aMetric.SetFamily( FAMILY_ROMAN );
                else if ( pTempFontAttr->mpHTMLSubstitution == aImplHTMLSubsSansSerif )
                    aMetric.SetFamily( FAMILY_SWISS );
                else if ( pTempFontAttr->mpHTMLSubstitution == aImplHTMLSubsMonospace )
                    aMetric.SetFamily( FAMILY_MODERN );
                else if ( pTempFontAttr->mpHTMLSubstitution == aImplHTMLSubsCursive )
                    aMetric.SetFamily( FAMILY_SCRIPT );
                else if ( pTempFontAttr->mpHTMLSubstitution == aImplHTMLSubsFantasy )
                    aMetric.SetFamily( FAMILY_DECORATIVE );
            }
            if ( aMetric.GetPitch() == PITCH_DONTKNOW )
            {
                if ( pTempFontAttr->mpHTMLSubstitution == aImplHTMLSubsMonospace )
                    aMetric.SetPitch( PITCH_FIXED );
            }
        }
    }

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

    // #83068#
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

        if( mpFontEntry->mpConversion )
            cChar = ImplRecodeChar( mpFontEntry->mpConversion, cChar );

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

    if( mpFontEntry->mpConversion )
        cChar = ImplRecodeChar( mpFontEntry->mpConversion, cChar );

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

        // #83068#
        aFont.SetOrientation( 0 );

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

        if( mpFontEntry->mpConversion )
            cChar = ImplRecodeChar( mpFontEntry->mpConversion, cChar );

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

    if( mpFontEntry->mpConversion )
        cChar = ImplRecodeChar( mpFontEntry->mpConversion, cChar );

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
    // #83068#
    else if( GetFont().GetOrientation() )
        rPolyPoly.Rotate( Point(), GetFont().GetOrientation() );

    return bRet;
}

// -----------------------------------------------------------------------

BOOL OutputDevice::GetFontCharMap( FontCharMap& rFontCharMap ) const
{
    rFontCharMap.ImplSetDefaultRanges();

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

    ULONG nPairs = mpGraphics->GetFontCodeRanges( NULL );
    if( !nPairs )
        return FALSE;

    sal_UCS4* pCodes = new sal_UCS4[ 2 * nPairs ];
    mpGraphics->GetFontCodeRanges( pCodes );
    rFontCharMap.ImplSetRanges( nPairs, pCodes );
    return TRUE;
}
