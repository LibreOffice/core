/*************************************************************************
 *
 *  $RCSfile: outdev3.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-23 15:08:25 $
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

#define WSstrcmp strcmp

// =======================================================================

DBG_NAMEEX( OutputDevice );
DBG_NAMEEX( Font );

// =======================================================================

#define OUTDEV_CHARCONVERT_REPLACE          FALSE

using namespace ::com::sun::star;
using namespace ::rtl;


// =======================================================================

#define MAX_DX_WORDS        120
#define TEXT_DRAW_ELLIPSIS  (TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_PATHELLIPSIS | TEXT_DRAW_NEWSELLIPSIS)

// =======================================================================

#define UNDERLINE_LAST      UNDERLINE_BOLDWAVE
#define STRIKEOUT_LAST      STRIKEOUT_X

// -----------------------------------------------------------------------

static void ImplRotatePos( long nOriginX, long nOriginY, long& rX, long& rY,
                           short nOrientation )
{
    double nRealOrientation = nOrientation*F_PI1800;
    double nCos = cos( nRealOrientation );
    double nSin = sin( nRealOrientation );

    // Translation...
    long nX = rX-nOriginX;
    long nY = rY-nOriginY;

    // Rotation...
    rX = ((long)   ( nCos*nX + nSin*nY )) + nOriginX;
    rY = ((long) - ( nSin*nX - nCos*nY )) + nOriginY;
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
                mpFontList->InitStdFonts();
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
            {
                pFrame->mpGraphics->GetDevFontList( pFrame->mpFrameData->mpFontList );
                pFrame->mpFrameData->mpFontList->InitStdFonts();
            }
        }
    }
}

// =======================================================================

struct ImplFontSubstEntry
{
    XubString               maName;
    XubString               maReplaceName;
    XubString               maMatchName;
    XubString               maMatchReplaceName;
    USHORT                  mnFlags;
    ImplFontSubstEntry*     mpNext;
};

// =======================================================================

static void ImplStrEraseAllSymbols( XubString& rStr )
{
    xub_StrLen  i = 0;
    xub_Unicode c = rStr.GetChar( i );
    while ( c )
    {
        // Alle Zeichen kleiner 0 zwischen 9-A, Z-a und z-127 loeschen
        if ( (c < 48) || ((c > 57) && (c < 65)) || ((c > 90) && (c < 97)) ||
             ((c > 122) && (c <= 127)) )
            rStr.Erase( i, 1 );
        else
            i++;
        c = rStr.GetChar( i );
    }
}

// -----------------------------------------------------------------------

static xub_StrLen ImplStrMatch( const XubString& rStr1, const XubString& rStr2 )
{
    xub_StrLen          nMatch = 0;
    const xub_Unicode*  pStr1 = rStr1.GetBuffer();
    const xub_Unicode*  pStr2 = rStr2.GetBuffer();
    while ( (*pStr1 == *pStr2) && *pStr1 )
    {
        pStr1++;
        pStr2++;
        nMatch++;
    }
    return nMatch;
}

// -----------------------------------------------------------------------

static int ImplStrFullMatch( const XubString& rStr1, const char* pStr2 )
{
    const xub_Unicode*  pStr1 = rStr1.GetBuffer();
    while ( (*pStr1 == (xub_Unicode)(unsigned char)*pStr2) && *pStr1 )
    {
        pStr1++;
        pStr2++;
    }
    return !(*pStr1);
}

// =======================================================================

#if 0

#define FONT_ATTR_SYMBOL        ((ULONG)0x00000001)
#define FONT_ATTR_FIXED         ((ULONG)0x00000002)
#define FONT_ATTR_ITALIC        ((ULONG)0x00000004)
#define FONT_ATTR_NORMAL        ((ULONG)0x00000008)
#define FONT_ATTR_STANDARD      ((ULONG)0x00000010)
#define FONT_ATTR_SPECIAL       ((ULONG)0x00000020)
#define FONT_ATTR_TITLING       ((ULONG)0x00000040)
#define FONT_ATTR_SERIF         ((ULONG)0x00000080)
#define FONT_ATTR_NONSERIF      ((ULONG)0x00000100)
#define FONT_ATTR_ROUNDED       ((ULONG)0x00000200)
#define FONT_ATTR_OUTLINE       ((ULONG)0x00000400)
#define FONT_ATTR_SHADOW        ((ULONG)0x00000800)
#define FONT_ATTR_SCRIPT        ((ULONG)0x00001000)
#define FONT_ATTR_HANDWRITING   ((ULONG)0x00002000)
#define FONT_ATTR_DECORATION    ((ULONG)0x00004000)
#define FONT_ATTR_CHARSCRIPT    ((ULONG)0x00008000)
#define FONT_ATTR_CHANCERY      ((ULONG)0x00010000)
#define FONT_ATTR_OLDSTYLE      ((ULONG)0x00020000)
#define FONT_ATTR_FAVOR1        ((ULONG)0x01000000)
#define FONT_ATTR_FAVOR2        ((ULONG)0x02000000)
#define FONT_ATTR_FAVOR3        ((ULONG)0x04000000)
#define FONT_ATTR_FAVOR4        ((ULONG)0x08000000)
#define FONT_ATTR_FOUND         ((ULONG)0x80000000)

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

struct ImplFontAttrTypeSearchData
{
    const char*             mpStr;
    ULONG                   mnType;
};

static ImplFontAttrTypeSearchData const aImplTypeAttrSearchList[] =
{
{   "titling",              FONT_ATTR_TITLING },
{   "outline",              FONT_ATTR_OUTLINE },
{   "shadow",               FONT_ATTR_SHADOW },
{   NULL,                   0 },
};

// =======================================================================

struct ImplFontNameAttr
{
    const char*             mpName;
    FontFamily              meFamily;
    FontWeight              meWeight;
    FontWidth               meWidth;
    ULONG                   mnType;
};

static const ImplFontNameAttr aImplFullList[] =
{
{   "Bookman",              FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_NORMAL | FONT_ATTR_STANDARD | FONT_ATTR_SERIF },
{   NULL,                   FAMILY_DONTKNOW,WEIGHT_DONTKNOW,WIDTH_DONTKNOW, 0 }
};

static const ImplFontNameAttr aImplMatchList[] =
{
{   "bookman",              FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_NORMAL | FONT_ATTR_STANDARD | FONT_ATTR_SERIF },
{   "comicsansms",          FAMILY_SCRIPT,  WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_NONSERIF | FONT_ATTR_SCRIPT | FONT_ATTR_CHARSCRIPT | FONT_ATTR_FAVOR3 },
{   "kristenitc",           FAMILY_SCRIPT,  WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_NONSERIF | FONT_ATTR_SCRIPT | FONT_ATTR_CHARSCRIPT | FONT_ATTR_FAVOR3 },
{   "maiandragd",           FAMILY_SCRIPT,  WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_NONSERIF | FONT_ATTR_SCRIPT | FONT_ATTR_CHARSCRIPT | FONT_ATTR_FAVOR3 },
{   "arioso",               FAMILY_SCRIPT,  WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_NONSERIF | FONT_ATTR_SCRIPT | FONT_ATTR_ITALIC | FONT_ATTR_DECORATION | FONT_ATTR_OLDSTYLE | FONT_ATTR_FAVOR3 },
{   "tempussansitc",        FAMILY_SCRIPT,  WEIGHT_LIGHT,   WIDTH_NORMAL,   FONT_ATTR_NONSERIF | FONT_ATTR_SCRIPT | FONT_ATTR_CHARSCRIPT },
{   "papyrus",              FAMILY_SCRIPT,  WEIGHT_LIGHT,   WIDTH_NORMAL,   FONT_ATTR_NONSERIF | FONT_ATTR_SCRIPT | FONT_ATTR_CHARSCRIPT },
{   "lucidashadowtitling",  FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_SERIF | FONT_ATTR_TITLING | FONT_ATTR_OUTLINE | FONT_ATTR_SHADOW },
{   "lucidaopenboldtitling",FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_SERIF | FONT_ATTR_TITLING | FONT_ATTR_OUTLINE },
{   "lucidaopentitling",    FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_SERIF | FONT_ATTR_TITLING | FONT_ATTR_OUTLINE },
{   "lucidaopen",           FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_SERIF | FONT_ATTR_OUTLINE },
{   "lucidashadow",         FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_SERIF | FONT_ATTR_OUTLINE | FONT_ATTR_SHADOW },
{   "chevara",              FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_EXPANDED, FONT_ATTR_SERIF | FONT_ATTR_TITLING | FONT_ATTR_OUTLINE },
{   "colonnamt",            FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_SERIF | FONT_ATTR_SPECIAL | FONT_ATTR_OUTLINE },
{   "imprintmtshadow",      FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_SERIF | FONT_ATTR_OUTLINE | FONT_ATTR_SHADOW },
{   "castellar",            FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_SERIF | FONT_ATTR_TITLING | FONT_ATTR_OUTLINE },
{   "algerian",             FAMILY_ROMAN,   WEIGHT_NORMAL,  WIDTH_NORMAL,   FONT_ATTR_SERIF | FONT_ATTR_SPECIAL | FONT_ATTR_TITLING | FONT_ATTR_OUTLINE | FONT_ATTR_SHADOW | FONT_ATTR_OLDSTYLE },
{   NULL,                   FAMILY_DONTKNOW,WEIGHT_DONTKNOW,WIDTH_DONTKNOW, 0 }
};

#endif

static const char* aImplSwissMatchList[] =
{
    "arial",
    "avantgarde",
    "cgomega",
    "centurygothic",
    "charcoal",
    "chicago",
    "frutiger",
    "geneva",
    "haettenschweiler",
    "helmet",
    "helv",
    "lucida",
    "impact",
    "tahoma",
    "univers",
    "vagrounded",
    "verdana",
    NULL
};

static const char* aImplSwissSearchList[] =
{
    "sansserif",
    "swiss",
    NULL
};

static const char* aImplRomanMatchList[] =
{
    "algerian",
    "antiqua",
    "caliso",
    "clarendon",
    "colonna",
    "garamond",
    "newyork",
    "palatino",
    "timmons",
    NULL
};

static const char* aImplRomanSearchList[] =
{
    "book",
    "times",
    "roman",
    "bright",
    NULL
};

static const char* aImplFixedMatchList[] =
{
    "lineprinter",
    "monaco",
    "typewriter",
    NULL
};

static const char* aImplFixedSearchList[] =
{
    "console",
    "courier",
    "fixed",
    "letter",
    "monospace",
    "terminal",
    NULL
};

static const char* aImplScriptMatchList[] =
{
    "arioso",
    "coronet",
    "cursive",
    "marigold",
    "zapfchancery",
    NULL
};

static const char* aImplScriptSearchList[] =
{
    "script",
    "signet",
    "handwriting",
    "calligraphy",
    NULL
};

static const char* aImplSymbolMatchList[] =
{
    "marlett",
    "monotypesorts",
    "msoutlook",
    NULL
};

static const char* aImplSymbolSearchList[] =
{
    "symbol",
    "bats",
    "dings",
    "math",
    NULL
};

static const char* aImplTypeList[] =
{
    "black",
    "bold",
    "condensed",
    "expanded",
    "narrow",
    "outline",
    NULL
};

// =======================================================================

static const char* aImplSearchScriptList[] =
{
  "ce",
  "we",
  "cyr",
  "tur",
  "wt",
  "greek",
  "wl",
  NULL
};

// -----------------------------------------------------------------------

struct ImplScriptSearchList
{
    const char*     mpScript;
    rtl_Script      meScript;
};

static void ImplCutScriptAndSpaces( XubString& rName )
{
    rName.EraseLeadingAndTrailingChars( ' ' );

    USHORT nLen = rName.Len();
    if ( nLen < 3 )
        return;

    // Scriptname must be the last part of the fontname and
    // looks like "fontname (scriptname)". So there can only be a
    // script name at the and of the fontname, when the last char is
    // ')'.
    if ( rName.GetChar( nLen-1 ) == ')' )
    {
        int nOpen = 1;
        nLen -= 2;
        while ( nLen )
        {
            if ( rName.GetChar( nLen ) == '(' )
            {
                nOpen--;
                if ( !nOpen && nLen && (rName.GetChar( nLen-1 ) == ' ') )
                {
                    XubString aScript = rName.Copy( nLen+1, rName.Len()-1-nLen-1 );
                    rName.Erase( nLen-1 );
                    return;
                }
            }
            if ( rName.GetChar( nLen ) == ')' )
                nOpen++;
            nLen--;
        }
    }

    // For compatibility with older version we must search for a
    // script name at the end of a fontname without brakets
    USHORT nSpacePos = rName.SearchBackward( ' ' );
    if ( nSpacePos && (nSpacePos != STRING_NOTFOUND) )
    {
        XubString aScript = rName.Copy( nSpacePos+1 );
        const char** pScript = aImplSearchScriptList;
        while ( *pScript )
        {
            if ( aScript.EqualsAscii( *pScript ) )
            {
                rName.Erase( nSpacePos );
                break;
            }
            pScript++;
        }
    }
}

// =======================================================================

#if 0

static const ImplFontNameAttr* ImplFindFontAttr( const XubString& rFontName )
{
    const ImplFontNameAttr* pList;

    pList = aImplFullList;
    while ( pList->mpName )
    {
        if ( rFontName.EqualsAscii( pList->mpName ) )
            return pList;
        pList++;
    }

    pList = aImplMatchList;
    while ( pList->mpName )
    {
        if ( ImplStrFullMatch( rFontName, pList->mpName ) )
            return pList;
        pList++;
    }

    return NULL;
}

// -----------------------------------------------------------------------

static void ImplGetFontAttr( const XubString& rFontName,
                             FontFamily& rFamily, CharSet& rCharSet,
                             FontPitch& rPitch )
{
    if ( (rFamily == FAMILY_DONTKNOW) || (rPitch == PITCH_DONTKNOW) ||
         (rCharSet == CHARSET_DONTKNOW) )
    {
    }
}

#endif

// =======================================================================

BOOL ImplTestFontName( const XubString& rName,
                       const sal_Char** pMatchList,
                       const sal_Char** pSearchList )
{
    const char** pAlias;

    pAlias = pMatchList;
    while ( *pAlias )
    {
        if ( ImplStrFullMatch( rName, *pAlias ) )
            return TRUE;
        pAlias++;
    }

    pAlias = pSearchList;
    while ( *pAlias )
    {
        if ( rName.SearchAscii( *pAlias ) != STRING_NOTFOUND )
            return TRUE;
        pAlias++;
    }

    return FALSE;
}

// =======================================================================

static void ImplFontAttrFromName( const XubString& rFontName,
                                  FontFamily& rFamily, CharSet& rCharSet,
                                  FontPitch& rPitch )
{
    if ( rFamily == FAMILY_DONTKNOW )
    {
        if ( ImplTestFontName( rFontName, aImplRomanMatchList, aImplRomanSearchList ) )
            rFamily = FAMILY_ROMAN;
        else if ( ImplTestFontName( rFontName, aImplSwissMatchList, aImplSwissSearchList ) )
            rFamily = FAMILY_SWISS;
        else if ( ImplTestFontName( rFontName, aImplScriptMatchList, aImplScriptSearchList ) )
            rFamily = FAMILY_SCRIPT;
    }

    if ( rPitch == PITCH_DONTKNOW )
    {
        if ( ImplTestFontName( rFontName, aImplFixedMatchList, aImplFixedSearchList ) )
            rPitch = PITCH_FIXED;
    }

    if ( rCharSet == RTL_TEXTENCODING_DONTKNOW )
    {
        if ( ImplTestFontName( rFontName, aImplSymbolMatchList, aImplSymbolSearchList ) )
            rCharSet = RTL_TEXTENCODING_SYMBOL;
    }
}

// =======================================================================

static const char* aImplStdSwissList[] =
{
    "helvetica",
    "arial",
    "lucida sans",
    "lucidasans",
    "lucida",
    "geneva",
    "helmet",
    NULL
};

static const char* aImplStdRomanList[] =
{
    "times",
    "times new roman",
    "timesnewroman",
    "roman",
    "lucida serif",
    "lucidaserif",
    "lucida bright",
    "lucidabright",
    "bookman",
    "garamond",
    "timmons",
    NULL
};

static const char* aImplStdFixedList[] =
{
    "courier",
    "courier new",
    "lucida typewriter",
    "lucidatypewriter",
    "lucida sans typewriter",
    "lucidasanstypewriter",
    NULL
};

static const char* aImplStdScriptList[] =
{
    "zapf chancery",
    "zapfchancery",
    "lucida calligraphy",
    "lucidacalligraphy",
    "lucida handwriting",
    "lucidahandwriting",
    "arioso",
    "script",
    "marigold",
    NULL
};

static const char* aImplStdSymbolList[] =
{
    "starbats",
    "symbol",
    "zapf dingbats",
    "zapfdingbats",
    "wingdings",
    "lucida dingbats",
    "lucidadingbats",
    "lucida sans dingbats",
    "lucidasansdingbats",
    NULL
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

static ImplFontData* ImplFindScript( ImplDevFontListData* pData,
                                     rtl_Script eScript )
{
    // Testen, ob ein Font mit einem entsprechendem
    // Script vorhanden ist
    ImplFontData* pCurFontData = pData->mpFirst;
    while ( pCurFontData )
    {
        // Detect Unicode Font !!!
        if ( pData->maMatchName.EqualsAscii( "arial unicode ms" ) )
            return pCurFontData;
        if ( eScript == pCurFontData->meScript )
            return pCurFontData;
        pCurFontData = pCurFontData->mpNext;
    }

    return NULL;
}

// -----------------------------------------------------------------------

/* !!! UNICODE - Duerfte nicht mehr gebraucht werden !!!
static rtl_TextEncoding ImplGetFakeEncoding( rtl_TextEncoding eEncoding )
{
    rtl_TextEncoding eSystemEncoding = GetSystemCharSet();
    // MS_1252 und 8859_1 sind kompatible
    if ( ((eEncoding == RTL_TEXTENCODING_MS_1252) ||
          (eEncoding == RTL_TEXTENCODING_ISO_8859_1)) &&
         ((eSystemEncoding == RTL_TEXTENCODING_MS_1252) ||
          (eSystemEncoding == RTL_TEXTENCODING_ISO_8859_1)) )
        return eEncoding;
    else
    {
        // Wir testen, ob beide Zeichensaetze dem gleichem Script
        // entsprechen, um so der Applikation das gleiche Encoding
        // vorzugaukeln. Dies ist beispielsweise bei Russisch wichtig
        // da hier Fonts mit unterschiedlichem Encoding auftauchen
        // koennen.
        rtl_Script eSrcScript;
        rtl_Script eSystemScript;
        rtl_TextEncodingInfo    aTEncInfo;
        aTEncInfo.StructSize    = sizeof( aTEncInfo );
        aTEncInfo.Script        = SCRIPT_DONTKNOW;
        if ( !rtl_getTextEncodingInfo( eEncoding, &aTEncInfo ) )
            return eEncoding;
        else
            eSrcScript = aTEncInfo.Script;
        aTEncInfo.Script        = SCRIPT_DONTKNOW;
        if ( !rtl_getTextEncodingInfo( eSystemEncoding, &aTEncInfo ) )
            return eEncoding;
        else
            eSystemScript = aTEncInfo.Script;
        if ( eSrcScript == eSystemScript )
            eEncoding = eSystemEncoding;
    }

    return eEncoding;
}
*/

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
    pEntry->maMatchName         = rFontName;
    pEntry->maMatchReplaceName  = rReplaceFontName;
    pEntry->mnFlags             = nFlags;
    pEntry->mpNext              = pSVData->maGDIData.mpFirstFontSubst;
    pEntry->maMatchName.ToLowerAscii();
    pEntry->maMatchReplaceName.ToLowerAscii();
    ImplCutScriptAndSpaces( pEntry->maMatchName );
    ImplCutScriptAndSpaces( pEntry->maMatchReplaceName );
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
    ImplSVData*         pSVData = ImplGetSVData();
    ImplFontSubstEntry* pEntry = pSVData->maGDIData.mpFirstFontSubst;
    while ( pEntry )
    {
        if ( ((pEntry->mnFlags & nFlags1) == nFlags2) &&
             (pEntry->maMatchName == rFontName) )
        {
            rFontName = pEntry->maMatchReplaceName;
            return TRUE;
        }

        pEntry = pEntry->mpNext;
    }

    return FALSE;
}

// =======================================================================

ImplDevFontList::ImplDevFontList() :
    List( CONTAINER_MAXBLOCKSIZE, 96, 32 )
{
#if 0
    mbIsInitMatchData = FALSE;
#endif
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
    // Vergleichen nach CharSet, Groesse, Breite, Weight, Italic, StyleName
    if ( pEntry1->meCharSet < pEntry2->meCharSet )
        return COMPARE_LESS;
    else if ( pEntry1->meCharSet > pEntry2->meCharSet )
        return COMPARE_GREATER;

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
    aSearchName.ToLowerAscii();

    // Query Script for FontTest
    rtl_TextEncodingInfo    aTEncInfo;
    aTEncInfo.StructSize    = sizeof( aTEncInfo );
    aTEncInfo.Script        = SCRIPT_DONTKNOW;
    rtl_getTextEncodingInfo( pNewData->meCharSet, &aTEncInfo );
    pNewData->meScript = aTEncInfo.Script;

    // Add Font
    ULONG                   nIndex;
    ImplDevFontListData*    pFoundData = ImplFind( aSearchName, &nIndex );
    BOOL                    bDelete = FALSE;

    if ( !pFoundData )
    {
        pFoundData                  = new ImplDevFontListData;
        pFoundData->maName          = pNewData->maName;
        pFoundData->maMatchName     = aSearchName;
        pFoundData->maMatchName2    = aSearchName;
        pFoundData->mpFirst         = pNewData;
#if 0
        pFoundData->mbScalable      = FALSE;
        ImplStrEraseAllSymbols( pFoundData->maMatchName2 );
/*
        pFoundData->meMatchFamily   = pNewData->meFamily;
        pFoundData->meMatchPitch    = pNewData->mePitch;
        CharSet eCharSet = pNewData->meCharSet;
        ImplFontAttrFromName( pFoundData->maMatchName2, pFoundData->meMatchFamily,
                              eCharSet, pFoundData->meMatchPitch );
        pFoundData->mbSymbol        = eCharSet == RTL_TEXTENCODING_SYMBOL;
*/
#else
        pFoundData->meMatchFamily   = pNewData->meFamily;
        pFoundData->meMatchPitch    = pNewData->mePitch;
        pFoundData->mnMatch         = 0;
        CharSet eCharSet = pNewData->meCharSet;
        ImplStrEraseAllSymbols( pFoundData->maMatchName2 );
        ImplFontAttrFromName( pFoundData->maMatchName2, pFoundData->meMatchFamily,
                              eCharSet, pFoundData->meMatchPitch );
        pFoundData->mbSymbol        = eCharSet == RTL_TEXTENCODING_SYMBOL;
#endif

        pNewData->mpNext            = NULL;
        Insert( pFoundData, nIndex );
    }
    else
    {
        // Name ersetzen (spart Speicherplatz)
        pNewData->maName = pFoundData->maName;

        BOOL bInsert = TRUE;
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
                    {
                        bDelete = TRUE;
                        delete pNewData;
                    }

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

    // Match zusammenzaehlen
    if ( !bDelete )
    {
#if 0
        if ( (pNewData->meType == TYPE_SCALABLE) && (pNewData->mnHeight == 0) )
            pFoundData->mbScalable = TRUE;
#else
        if ( (pNewData->meType == TYPE_SCALABLE) && (pNewData->mnHeight == 0) )
        {
            if ( pNewData->meWidthType == WIDTH_NORMAL )
                pFoundData->mnMatch += 30;
            else
                pFoundData->mnMatch += 3;
            if ( pNewData->meItalic == ITALIC_NONE )
                pFoundData->mnMatch += 20;
            else
                pFoundData->mnMatch += 2;
            if ( (pNewData->meWeight == WEIGHT_NORMAL) || (pNewData->meWeight == WEIGHT_MEDIUM) )
                pFoundData->mnMatch += 10;
            else
                pFoundData->mnMatch += 1;
        }
#endif
    }
}

// -----------------------------------------------------------------------

ImplDevFontListData* ImplDevFontList::ImplFind( const XubString& rFontName, ULONG* pIndex ) const
{
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
        eCompare = rFontName.CompareTo( pCompareData->maMatchName );
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
        eCompare = rFontName.CompareTo( pCompareData->maMatchName );
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
    aName.ToLowerAscii();
    ImplCutScriptAndSpaces( aName );
    return ImplFind( aName );
}

// -----------------------------------------------------------------------

ImplDevFontListData* ImplDevFontList::FindStdFont( const sal_Char** pStdFontNames,
                                                   rtl_Script eScript ) const
{
    // We want a scalable font with a system script
    ImplDevFontListData* pRasterFoundData = NULL;
    ImplDevFontListData* pWrongScriptRasterData = NULL;
    ImplDevFontListData* pWrongScriptData = NULL;
    while ( *pStdFontNames )
    {
        XubString aStdName( *pStdFontNames, RTL_TEXTENCODING_ASCII_US );
        ImplDevFontListData* pFoundData = ImplFind( aStdName );
        if ( pFoundData )
        {
            if ( (eScript == SCRIPT_SYMBOL) ||
                 ImplFindScript( pFoundData, eScript ) )
            {
                if ( pFoundData->mpFirst->meType != TYPE_RASTER )
                    return pFoundData;
                else if ( !pRasterFoundData )
                    pRasterFoundData = pFoundData;
            }
            else
            {
                if ( pFoundData->mpFirst->meType != TYPE_RASTER )
                {
                    if ( !pWrongScriptData )
                        pWrongScriptData = pFoundData;
                }
                else
                {
                    if ( !pWrongScriptRasterData )
                        pWrongScriptRasterData = pFoundData;
                }
            }
        }
        pStdFontNames++;
    }

    // Wenn keine passende Schrift, dann die Reihenfolge:
    // - passender Zeichensatz
    // - Skalierbar
    // - eine passende Schrift die nicht skalierbar und den
    //   falschen Zeichensatz hat
    // - keine
    if ( pRasterFoundData )
        return pRasterFoundData;
    else if ( pWrongScriptData )
        return pWrongScriptData;
    else
        return pWrongScriptRasterData;
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

    // Standard-Fonts loeschen
    for ( USHORT i = 0; i < IMPL_STDFONT_COUNT; i++ )
        mpStdFontAry[i] = NULL;
#if 0
    mbIsInitMatchData = FALSE;
#endif
}

// -----------------------------------------------------------------------

#if 0
void ImplDevFontList::InitMatchData()
{
    if ( mbIsInitMatchData )
        return;
    mbIsInitMatchData = TRUE;
/*
    // Fuer alle Eintraege die Matchdaten ermitteln
    ImplDevFontListData* pEntry = First();
    while ( pEntry )
    {
        ImplFontData* pFontData = pEntry->mpFirst;

        pEntry = Next();
    }
*/
}
#endif

// -----------------------------------------------------------------------

void ImplDevFontList::InitStdFonts()
{
    rtl_Script              eSystemScript;
    rtl_TextEncodingInfo    aTextEncInfo;
    aTextEncInfo.StructSize = sizeof( aTextEncInfo );
    aTextEncInfo.Script = SCRIPT_DONTKNOW;
    rtl_getTextEncodingInfo( gsl_getSystemTextEncoding(), &aTextEncInfo );
    eSystemScript = aTextEncInfo.Script;

    mpStdFontAry[IMPL_STDFONT_SWISS]    = FindStdFont( aImplStdSwissList, eSystemScript );
    mpStdFontAry[IMPL_STDFONT_ROMAN]    = FindStdFont( aImplStdRomanList, eSystemScript );
    mpStdFontAry[IMPL_STDFONT_FIXED]    = FindStdFont( aImplStdFixedList, eSystemScript );
    mpStdFontAry[IMPL_STDFONT_SCRIPT]   = FindStdFont( aImplStdScriptList, eSystemScript );
    mpStdFontAry[IMPL_STDFONT_SYMBOL]   = FindStdFont( aImplStdSymbolList, SCRIPT_SYMBOL );
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

    if ( mpKernInfo )
        delete mpKernInfo;
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
    const XubString& rName      = rFont.GetName();
    const XubString& rStyleName = rFont.GetStyleName();
    long nWidth                 = rSize.Width();
    long nHeight                = rSize.Height();
    FontFamily eFamily          = rFont.GetFamily();
    CharSet eCharSet            = rFont.GetCharSet();
    FontWeight eWeight          = rFont.GetWeight();
    FontItalic eItalic          = rFont.GetItalic();
    FontPitch ePitch            = rFont.GetPitch();
    short nOrientation          = (short)(rFont.GetOrientation() % 3600);
    BOOL bVertical              = rFont.IsVertical();
    BOOL bShadow                = rFont.IsShadow();
    BOOL bOutline               = rFont.IsOutline();

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

    // Wir suchen zuerst ueber den Namen den passenden Font
    XubString               aLowerFontName = rName;
    XubString               aFirstName;
    XubString               aTempName;
    XubString               aTempName2;
    ImplDevFontListData*    pFoundData;
    ImplDevFontListData*    pTempFoundData;
    xub_StrLen              nFirstNameIndex = 0;
    xub_StrLen              nIndex = 0;
    USHORT                  nSubstFlags1 = FONT_SUBSTITUTE_ALWAYS;
    USHORT                  nSubstFlags2 = FONT_SUBSTITUTE_ALWAYS;
    rtl_Script              eScript = SCRIPT_DONTKNOW;
    rtl_Script              eCharSetScript;
    rtl_TextEncodingInfo    aTextEncInfo;
    aTextEncInfo.StructSize = sizeof( aTextEncInfo );
    aTextEncInfo.Script = SCRIPT_DONTKNOW;
    rtl_getTextEncodingInfo( eCharSet, &aTextEncInfo );
    eCharSetScript = aTextEncInfo.Script;
    if ( mbPrinter )
        nSubstFlags1 |= FONT_SUBSTITUTE_SCREENONLY;
    aLowerFontName.ToLowerAscii();
    pTempFoundData = NULL;
    do
    {
        aTempName = aLowerFontName.GetToken( 0, ';', nIndex );
        ImplCutScriptAndSpaces( aTempName );
        if ( !aFirstName.Len() )
        {
            aFirstName = aTempName;
            nFirstNameIndex = nIndex;
        }
        ImplFontSubstitute( aTempName, nSubstFlags1, nSubstFlags2 );
        pFoundData = pFontList->ImplFind( aTempName );
        if ( pFoundData )
        {
            if ( eCharSetScript != SCRIPT_DONTKNOW )
            {
                if ( !pTempFoundData )
                {
                    aTempName2 = aTempName;
                    pTempFoundData = pFoundData;
                }

                // Testen, ob ein Font mit einem entsprechendem
                // Script vorhanden ist
                if ( ImplFindScript( pFoundData, eCharSetScript ) )
                {
                    aLowerFontName = aTempName;
                    break;
                }
                else
                    pFoundData = NULL;
            }
            else
            {
                aLowerFontName = aTempName;
                break;
            }
        }
    }
    while ( nIndex != STRING_NOTFOUND );
    if ( !pFoundData && pTempFoundData )
    {
        pFoundData = pTempFoundData;
        aLowerFontName = aTempName2;
    }

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
            aTempName = aLowerFontName.GetToken( 0, ';', nIndex );
            ImplCutScriptAndSpaces( aTempName );
            if ( ImplFontSubstitute( aTempName, nSubstFlags1, nSubstFlags2 ) )
            {
                pFoundData = pFontList->ImplFind( aTempName );
                if ( pFoundData )
                {
                    aLowerFontName = aTempName;
                    break;
                }
            }
        }
        while ( nIndex != STRING_NOTFOUND );
    }

    // Wenn kein Font mit dem entsprechenden Namen existiert, versuchen
    // wir ueber den Namen und die Attribute einen passenden Font zu
    // finden
    ULONG nFontCount = pFontList->Count();
    if ( !pFoundData && nFontCount )
    {
#if 0
        pFontList->InitMatchData();

        // 1. Token vom Fontnamen nehmen und Sonderzeichen entfernen
        XubString aNoSymbolName = aFirstName;
        ImplCutScriptAndSpaces( aNoSymbolName );
        ImplStrEraseAllSymbols( aNoSymbolName );

        // Script evtl. aus CharSet gewinnen, wenn nicht ueber den Fontnamen
        // ermittelt werden konnte
        if ( eScript == SCRIPT_DONTKNOW )
            eScript = eCharSetScript;

        // wir versuchen zuerst einen Font zu finden, der ueber den Namen
        // matched
        ULONG nTestMatch;
        ULONG nBestMatch = 0;
        for ( ULONG i = 0; i < nFontCount; i++ )
        {
            ImplDevFontListData* pData = pFontList->Get( i );

            nTestMatch = 0;

            // Wir wollen schon Zeichen erkennen
            if ( eScript != SCRIPT_DONTKNOW )
            {
                if ( ImplFindScript( pData, eScript ) )
                    nTestMatch += 1000000000;
            }

            // skalierbare Schriften haben schon einen echten Vorteil
            // gegenueber nicht skalierbaren Schriften
            if ( pData->mbScalable )
                nTestMatch += 500000000;

/*
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
*/

            if ( nTestMatch > nBestMatch )
            {
                pFoundData = pData;
                nBestMatch = nTestMatch;
            }
        }

        if ( !pFoundData )
        {
            pFoundData = pFontList->GetStandardFont( IMPL_STDFONT_ROMAN );
            // Wenn alles nichts hilft, nehmen wir den ersten
            if ( !pFoundData )
                pFontList->Get( 0 );
        }
#else
        // 1. Token vom Fontnamen nehmen und Sonderzeichen entfernen
        XubString aNoSymbolName = aFirstName;
        ImplStrEraseAllSymbols( aNoSymbolName );

        // Suchattribute ermitteln
        BOOL        bFamily;
        BOOL        bSymbol;
        BOOL        bFixed;
        FontFamily  eSearchFamily = eFamily;
        CharSet     eSearchCharSet = eCharSet;
        FontPitch   eSearchPitch = ePitch;
        ImplFontAttrFromName( aNoSymbolName, eSearchFamily, eSearchCharSet, eSearchPitch );
        bFamily = eSearchFamily != FAMILY_DONTKNOW;
        bSymbol = eSearchCharSet == RTL_TEXTENCODING_SYMBOL;
        bFixed = eSearchPitch == PITCH_FIXED;
        // Solange in der Namesliste suchen, bis wir auswertbare
        // Attribute gefunden haben
        xub_StrLen nTempIndex = nFirstNameIndex;
        while ( (nTempIndex != STRING_NOTFOUND) && !bFamily && !bFixed && !bSymbol )
        {
            aTempName = aLowerFontName.GetToken( 0, ';', nTempIndex );
            ImplCutScriptAndSpaces( aTempName );
            if ( !aTempName.Len() )
                break;
            ImplStrEraseAllSymbols( aTempName );
            ImplFontAttrFromName( aTempName, eSearchFamily, eSearchCharSet, eSearchPitch );
            bFamily = eSearchFamily != FAMILY_DONTKNOW;
            bSymbol = eSearchCharSet == RTL_TEXTENCODING_SYMBOL;
            bFixed = eSearchPitch == PITCH_FIXED;
        }
        aLowerFontName = aFirstName;

        // wir versuchen zuerst einen Font zu finden, der ueber den Namen
        // matched
        ULONG           i;
        xub_StrLen      nBestMatch = 5;
        xub_StrLen      nBestAttrMatch = 0;
        xub_StrLen      nBestStrLen = 0xFFFF;
        for ( i = 0; i < nFontCount; i++ )
        {
            ImplDevFontListData* pData = pFontList->Get( i );
            // Beim matchen ignorieren wir alle Sonderzeichen
            xub_StrLen nTestMatch = ImplStrMatch( aNoSymbolName, pData->maMatchName2 );
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
                        if ( (aNoSymbolName.SearchAscii( *pTypeList ) != STRING_NOTFOUND) &&
                             (pData->maMatchName2.SearchAscii( *pTypeList ) != STRING_NOTFOUND) )
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
        }

        // Wenn wir immer noch keinen passenden Font gefunden haben, versuchen
        // wir es ueber die Attribute
        if ( !pFoundData )
        {
            if ( bFixed )
            {
                pFoundData = pFontList->GetStandardFont( IMPL_STDFONT_FIXED );
                if ( !pFoundData )
                {
                    nBestMatch = 0;
                    for ( i = 0; i < nFontCount; i++ )
                    {
                        ImplDevFontListData* pData = pFontList->Get( i );
                        if ( (pData->meMatchPitch == PITCH_FIXED) &&
                             !pData->mbSymbol &&
                             (pData->meMatchFamily != FAMILY_DECORATIVE) )
                        {
                            if ( pData->mnMatch > nBestMatch )
                            {
                                pFoundData = pData;
                                nBestMatch = pData->mnMatch;
                            }
                        }
                    }
                }
            }

            if ( bFamily && !pFoundData )
            {
                if ( eSearchFamily == FAMILY_SWISS )
                    pFoundData = pFontList->GetStandardFont( IMPL_STDFONT_SWISS );
                else if ( eSearchFamily == FAMILY_ROMAN )
                    pFoundData = pFontList->GetStandardFont( IMPL_STDFONT_ROMAN );
                else if ( eSearchFamily == FAMILY_SCRIPT )
                    pFoundData = pFontList->GetStandardFont( IMPL_STDFONT_SCRIPT );
                if ( !pFoundData )
                {
                    nBestMatch = 0;
                    for ( i = 0; i < nFontCount; i++ )
                    {
                        ImplDevFontListData* pData = pFontList->Get( i );
                        if ( (pData->meMatchFamily == eSearchFamily) &&
                             !pData->mbSymbol &&
                             (pData->meMatchPitch != PITCH_FIXED) )
                        {
                            if ( pData->mnMatch > nBestMatch )
                            {
                                pFoundData = pData;
                                nBestMatch = pData->mnMatch;
                            }
                        }
                    }
                }
            }

            if ( bSymbol && !pFoundData )
            {
                pFoundData = pFontList->GetStandardFont( IMPL_STDFONT_SYMBOL );
                if ( !pFoundData )
                {
                    nBestMatch = 0;
                    for ( i = 0; i < nFontCount; i++ )
                    {
                        ImplDevFontListData* pData = pFontList->Get( i );
                        if ( pData->mbSymbol )
                        {
                            if ( pData->mnMatch > nBestMatch )
                            {
                                pFoundData = pData;
                                nBestMatch = pData->mnMatch;
                            }
                        }
                    }
                }
            }

            if ( !pFoundData )
            {
                pFoundData = pFontList->GetStandardFont( IMPL_STDFONT_ROMAN );
                // Wenn alles nichts hilft, nehmen wir den ersten
                if ( !pFoundData )
                    pFontList->Get( 0 );
            }
        }
#endif
    }

    // Script evtl. aus CharSet gewinnen, wenn nicht ueber den Fontnamen
    // ermittelt werden konnte
    if ( eScript == SCRIPT_DONTKNOW )
        eScript = eCharSetScript;

    // Jetzt suchen wir den Font ueber die Attribute
    ImplFontData* pFontData = NULL;
    if ( pFoundData )
    {
        ULONG           nBestMatch = 0;         // Der groessere Wert ist der bessere
        ULONG           nBestHeightMatch = 0;   // Der kleinere Wert ist der bessere
        ULONG           nBestWidthMatch = 0;    // Der kleinere Wert ist der bessere
        ULONG           nMatch;
        ULONG           nHeightMatch;
        ULONG           nWidthMatch;
        ImplFontData*   pCurFontData;

//          !!!!! Wegen Office-Fehler !!!!
//        XubString       aCompareStyleName = rStyleName;
//        aCompareStyleName.ToLowerAscii();

        // Vorallem wegen OS2-System-Standard-Fonts vergleichen wir
        // den FontNamen mit FontName + StyleName, damit
        // WarpSans Bold auch einen fetten Font ergibt
        const xub_Unicode* pCompareStyleName = NULL;
        if ( (aLowerFontName.Len() > pFoundData->maMatchName.Len()) &&
             aLowerFontName.Equals( pFoundData->maMatchName, 0, pFoundData->maMatchName.Len() ) )
            pCompareStyleName = aLowerFontName.GetBuffer()+pFoundData->maMatchName.Len()+1;

        pCurFontData = pFoundData->mpFirst;
        while ( pCurFontData )
        {
            nMatch = 0;
            nHeightMatch = 0;
            nWidthMatch = 0;

//            if ( aCompareStyleName.Len() &&
//                 aCompareStyleName.EqualsIgnoreCaseAscii( pCurFontData->maStyleName ) )
//                nMatch += 120000;
            if ( pCompareStyleName &&
                 pCurFontData->maStyleName.EqualsIgnoreCaseAscii( pCompareStyleName ) )
                nMatch += 120000;

            if ( eCharSet != RTL_TEXTENCODING_DONTKNOW )
            {
                if ( eCharSet == pCurFontData->meCharSet )
                    nMatch += 60000;
            }

            if ( eScript == pCurFontData->meScript )
                nMatch += 30000;

            if ( (ePitch != PITCH_DONTKNOW) && (ePitch == pCurFontData->mePitch) )
                nMatch += 15000;

            if ( (eFamily != FAMILY_DONTKNOW) && (eFamily == pCurFontData->meFamily) )
                nMatch += 7500;

            // Normale Schriftbreiten bevorzugen, da wir noch keine Daten
            // von den Applikationen bekommen
            if ( pCurFontData->meWidthType == WIDTH_NORMAL )
                nMatch += 3750;

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
    pEntry->mnWidthAryCount         = 0;
    pEntry->mnWidthArySize          = 0;
    pEntry->mpWidthAry              = NULL;
    pEntry->mpKernPairs             = NULL;
    pEntry->mpKernInfo              = NULL;
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

void OutputDevice::ImplInitFont()
{
    DBG_TESTSOLARMUTEX();

    if ( mbInitFont )
    {
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
            pFontEntry->maMetric.mnSuperscriptSize      = 0;
            pFontEntry->maMetric.mnSuperscriptOffset    = 0;
            pFontEntry->maMetric.mnSubscriptSize        = 0;
            pFontEntry->maMetric.mnSubscriptOffset      = 0;
            pFontEntry->maMetric.mnUnderlineSize        = 0;
            pFontEntry->maMetric.mnUnderlineOffset      = 0;
            pFontEntry->maMetric.mnBUnderlineSize       = 0;
            pFontEntry->maMetric.mnBUnderlineOffset     = 0;
            pFontEntry->maMetric.mnDUnderlineSize       = 0;
            pFontEntry->maMetric.mnDUnderlineOffset1    = 0;
            pFontEntry->maMetric.mnDUnderlineOffset2    = 0;
            pFontEntry->maMetric.mnWUnderlineSize       = 0;
            pFontEntry->maMetric.mnWUnderlineOffset     = 0;
            pFontEntry->maMetric.mnStrikeoutSize        = 0;
            pFontEntry->maMetric.mnStrikeoutOffset      = 0;
            pFontEntry->maMetric.mnBStrikeoutSize       = 0;
            pFontEntry->maMetric.mnBStrikeoutOffset     = 0;
            pFontEntry->maMetric.mnDStrikeoutSize       = 0;
            pFontEntry->maMetric.mnDStrikeoutOffset1    = 0;
            pFontEntry->maMetric.mnDStrikeoutOffset2    = 0;
#ifndef REMOTE_APPSERVER
            pGraphics->GetFontMetric( &(pFontEntry->maMetric) );
            pFontEntry->mnWidthFactor = pGraphics->GetCharWidth( 0, CHARCACHE_STD-1, pFontEntry->maWidthAry );
#else
            long nFactor = 0;

            pGraphics->GetFontMetric(
                pFontEntry->maMetric,
                nFactor, 0, CHARCACHE_STD-1, pFontEntry->maWidthAry,
                maFont.IsKerning(), &pKernPairs, nKernPairs
                );
            pFontEntry->mnWidthFactor = nFactor;
#endif
            if ( !pFontEntry->mnWidthFactor )
            {
                memset( pFontEntry->maWidthAry, 0, sizeof(long)*(CHARCACHE_STD-1) );
                pFontEntry->mnWidthFactor = 1;
            }

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
    if ( maFont.IsKerning() )
    {
        ImplInitKerningPairs( pKernPairs, nKernPairs );
        mbKerning = (pFontEntry->mnKernPairs) != 0;
    }
    else
        mbKerning = FALSE;

    // Je nach TextAlign den TextOffset berechnen
    TextAlign eAlign = maFont.GetAlign();
    if ( eAlign == ALIGN_BASELINE )
    {
        mnTextOffX = 0;
        mnTextOffY = 0;
    }
    else if ( eAlign == ALIGN_TOP )
    {
        long nOrientation = pFontEntry->mnOrientation;
        if ( nOrientation )
        {
            mnTextOffX = 0;
            mnTextOffY = pFontEntry->maMetric.mnAscent;
            ImplRotatePos( 0, 0, mnTextOffX, mnTextOffY, nOrientation );
        }
        else
        {
            mnTextOffX = 0;
            mnTextOffY = pFontEntry->maMetric.mnAscent;
        }
    }
    else // eAlign == ALIGN_BOTTOM
    {
        long nOrientation = pFontEntry->mnOrientation;
        if ( nOrientation )
        {
            mnTextOffX = 0;
            mnTextOffY = -pFontEntry->maMetric.mnDescent;
            ImplRotatePos( 0, 0, mnTextOffX, mnTextOffY, nOrientation );
        }
        else
        {
            mnTextOffX = 0;
            mnTextOffY = -pFontEntry->maMetric.mnDescent;
        }
    }

    mbTextLines     = ((maFont.GetUnderline() != UNDERLINE_NONE) && (maFont.GetUnderline() != UNDERLINE_DONTKNOW)) ||
                      ((maFont.GetStrikeout() != STRIKEOUT_NONE) && (maFont.GetStrikeout() != STRIKEOUT_DONTKNOW));
    mbTextSpecial   = maFont.IsShadow() || maFont.IsOutline();

    if ( pOldEntry )
        mpFontCache->Release( pOldEntry );

    return TRUE;
}

// -----------------------------------------------------------------------

long OutputDevice::ImplGetCharWidth( sal_Unicode c ) const
{
    USHORT nChar = (USHORT)c;
    if ( nChar < CHARCACHE_STD )
        return mpFontEntry->maWidthAry[nChar];

    ImplFontEntry*      pFontEntry = mpFontEntry;
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
        pFontEntry->mnWidthArySize = WIDTHARY_INIT;
        pFontEntry->mpWidthAry = new ImplWidthInfoData[pFontEntry->mnWidthArySize];
        pWidthAry = pFontEntry->mpWidthAry;
        nInsIndex = 0;
    }

    if ( nWidthCount == pFontEntry->mnWidthArySize )
    {
        USHORT nOldSize = pFontEntry->mnWidthArySize;
        pFontEntry->mnWidthArySize += WIDTHARY_RESIZE;
        pFontEntry->mpWidthAry = new ImplWidthInfoData[pFontEntry->mnWidthArySize];
        memcpy( pFontEntry->mpWidthAry, pWidthAry, nOldSize*sizeof(ImplWidthInfoData) );
        delete pWidthAry;
        pWidthAry = pFontEntry->mpWidthAry;
    }

    // Um die Zeichenbreite zu ermitteln, brauchen wir einen Graphics und der
    // Font muss natuerlich auch selektiert sein
#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !((OutputDevice*)this)->ImplGetGraphics() )
            return 0;
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

    long nWidth;
#ifndef REMOTE_APPSERVER
    long nWidthFactor = mpGraphics->GetCharWidth( nChar, nChar, &nWidth );
#else
    long nWidthFactor = pFontEntry->mnWidthFactor;
    mpGraphics->GetCharWidth( nChar, nChar, &nWidth );
#endif
    if ( !nWidthFactor )
        return 0;

    DBG_ASSERT( (nWidthFactor == pFontEntry->mnWidthFactor),
                "OutputDevice::ImplGetCharWidth() - other WidthFactor" );

    // Breite in Liste einfuegen und zurueckgeben
    pInfo = pWidthAry+nInsIndex;
    memmove( pInfo+1, pInfo, (nWidthCount-nInsIndex)*sizeof(ImplWidthInfoData) );
    pFontEntry->mnWidthAryCount++;
    pInfo->mnChar = nChar;
    pInfo->mnWidth = nWidth;
    return nWidth;
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
            ImplKernInfoData* pKernInfo     = new ImplKernInfoData;
            pKernPairs    = new ImplKernPairData[pFontEntry->mnKernPairs];
            memset( pKernPairs, 0, sizeof(ImplKernPairData)*pFontEntry->mnKernPairs );
            pFontEntry->mnKernPairs         = mpGraphics->GetKernPairs( pFontEntry->mnKernPairs, pKernPairs );
            pFontEntry->mpKernInfo          = pKernInfo;
            pFontEntry->mpKernPairs         = pKernPairs;

            // Wir akzeptieren erstmal nur max. 65535-Paare
            USHORT nMaxKernPairs;
            if ( pFontEntry->mnKernPairs > 0xFFFF )
                nMaxKernPairs = 0xFFFF;
            else
                nMaxKernPairs = (USHORT)pFontEntry->mnKernPairs;
            memset( pKernInfo->maFirstAry, 0xFF, sizeof( pKernInfo->maFirstAry ) );
            memset( pKernInfo->maLastAry, 0, sizeof( pKernInfo->maLastAry ) );
            for ( USHORT i = 0; i < nMaxKernPairs; i++ )
            {
                USHORT nFirst = pKernPairs[i].mnChar1;
                if ( nFirst < 0xFF )
                {
                    if ( i < pKernInfo->maFirstAry[nFirst] )
                        pKernInfo->maFirstAry[nFirst] = i;
                    pKernInfo->maLastAry[nFirst] = i;
                }
            }
        }
#else
        // Wir arbeiten erstmal nur mit USHORT
        if( ! pKernPairs )
            nKernPairs = mpGraphics->GetKernPairs( &pKernPairs );
        if ( nKernPairs )
        {
            ImplKernInfoData* pKernInfo = new ImplKernInfoData;
            pFontEntry->mpKernInfo  = pKernInfo;
            pFontEntry->mpKernPairs = pKernPairs;

            // Wir akzeptieren erstmal nur max. 65535-Paare
            USHORT nMaxKernPairs;
            if( ! pFontEntry->mnKernPairs )
                pFontEntry->mnKernPairs = nKernPairs;
            if ( pFontEntry->mnKernPairs > 0xFFFF )
                nMaxKernPairs = 0xFFFF;
            else
                nMaxKernPairs = (USHORT)pFontEntry->mnKernPairs;
            memset( pKernInfo->maFirstAry, 0xFF, sizeof( pKernInfo->maFirstAry ) );
            memset( pKernInfo->maLastAry, 0, sizeof( pKernInfo->maLastAry ) );
            for ( USHORT i = 0; i < nMaxKernPairs; i++ )
            {
                USHORT nFirst = pKernPairs[i].mnChar1;
                if ( nFirst < 0xFF )
                {
                    if ( i < pKernInfo->maFirstAry[nFirst] )
                        pKernInfo->maFirstAry[nFirst] = i;
                    pKernInfo->maLastAry[nFirst] = i;
                }
            }
        }
#endif
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
    ImplKernInfoData* pKernInfo     = pEntry->mpKernInfo;
    long nWidth                     = 0;

#ifdef DBG_UTIL
    {
    ImplKernPairData    aTestPair;
#ifdef __LITTLEENDIAN
    ULONG               nTestComp  = ((ULONG)((USHORT)0xAABB) << 16) | (USHORT)0xCCDD;
#else
    ULONG               nTestComp  = ((ULONG)((USHORT)0xCCDD) << 16) | (USHORT)0xAABB;
#endif
    aTestPair.mnChar1 = 0xCCDD;
    aTestPair.mnChar2 = 0xAABB;
    DBG_ASSERT( nTestComp == *((ULONG*)&aTestPair), "Code doesn't work in this Version" );
    }
#endif

    for ( USHORT i = 0; i < nLen-1; i++ )
    {
        USHORT  nIndex = (USHORT)(unsigned char)pStr[i];
        USHORT  nFirst = pKernInfo->maFirstAry[nIndex];
        USHORT  nLast  = pKernInfo->maLastAry[nIndex];
#ifdef __LITTLEENDIAN
        ULONG   nComp  = ((ULONG)((USHORT)(unsigned char)pStr[i+1]) << 16) | nIndex;
#else
        ULONG   nComp  = ((ULONG)nIndex << 16) | ((USHORT)(unsigned char)pStr[i+1]);
#endif
        for ( USHORT j = nFirst; j <= nLast; j++ )
        {
            if ( nComp == *((ULONG*)&(pKernPairs[j])) )
            {
                long nAmount = pKernPairs[j].mnKern;
                nWidth += nAmount;
                if ( pDXAry )
                {
                    for ( USHORT n = i; n < nAryLen; n++ )
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
    long            nFactor = pFontEntry->mnWidthFactor;
    long            nWidth = 0;

    if ( nLen )
    {
        if ( pDX )
        {
            if ( nLen > 1 )
                nWidth += pDX[nLen-2];
            nWidth += ImplGetCharWidth( pStr[nLen-1] ) / nFactor;
        }
        else
        {
            // Bei Fixed-Fonts reicht eine Multiplikation
            // Not TRUE for all Fonts, like CJK Fonts
//            if ( pFontEntry->mbFixedFont )
//                nWidth = ImplGetCharWidth( 'A' ) * nLen;
//            else
            {
                const sal_Unicode*  pTempStr = pStr;
                xub_StrLen          nTempLen = nLen;
                while ( nTempLen )
                {
                    nWidth += ImplGetCharWidth( *pTempStr );
                    nTempLen--;
                    pTempStr++;
                }
            }
            nWidth /= nFactor;

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

    ImplDrawTextRect( nX, nY, nX, nY-mpFontEntry->maMetric.mnAscent,
                      ImplGetTextWidth( pStr, nLen, pDXAry ),
                      mpFontEntry->mnLineHeight );
#else
    Color aOldLineColor = GetLineColor();
    Color aOldFillColor = GetFillColor();
    SetLineColor();
    SetFillColor( GetTextFillColor() );
    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();
    ImplDrawTextRect( nX, nY, nX, nY-mpFontEntry->maMetric.mnAscent,
                      ImplGetTextWidth( pStr, nLen, pDXAry ),
                      mpFontEntry->mnLineHeight );
    SetLineColor( aOldLineColor );
    SetFillColor( aOldFillColor );
#endif
}

// -----------------------------------------------------------------------

Rectangle OutputDevice::ImplGetTextBoundRect( long nX, long nY,
                                              const xub_Unicode* pStr, xub_StrLen nLen,
                                              const long* pDXAry )
{
    if( !nLen )
        return Rectangle();

    if ( mbNewFont )
        ImplNewFont();

    if ( mbInitFont )
        ImplInitFont();

    long nBaseX = nX, nBaseY = nY;
    long nWidth = ImplGetTextWidth( pStr, nLen, pDXAry ), nHeight = mpFontEntry->mnLineHeight;

    nY -= mpFontEntry->maMetric.mnAscent;

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

    nLineHeight = ((mpFontEntry->maMetric.mnDescent*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;
    nLineHeight2 = nLineHeight / 2;
    if ( !nLineHeight2 )
        nLineHeight2 = 1;

    nBLineHeight = ((mpFontEntry->maMetric.mnDescent*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;
    nBLineHeight2 = nBLineHeight/2;
    if ( !nBLineHeight2 )
        nBLineHeight2 = 1;

    n2LineHeight = ((mpFontEntry->maMetric.mnDescent*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;
    n2LineDY = n2LineHeight;
    if ( n2LineDY <= 0 )
        n2LineDY = 1;
    n2LineDY2 = n2LineDY/2;
    if ( !n2LineDY2 )
        n2LineDY2 = 1;

    BOOL bVertical = maFont.IsVertical();
    long nLeading = mpFontEntry->maMetric.mnLeading;
    if ( bVertical )
    {
        if ( !nLeading )
        {
            if ( mpFontEntry->maMetric.mnDescent )
                nLeading = mpFontEntry->maMetric.mnDescent;
            else
            {
                nLeading = mpFontEntry->maMetric.mnAscent*4/5;
                if ( !nLeading )
                    nLeading = 1;
            }
        }
        nUnderlineOffset = -(mpFontEntry->maMetric.mnAscent-nLeading/2);
    }
    else
        nUnderlineOffset = mpFontEntry->maMetric.mnDescent/2 + 1;
    nStrikeoutOffset = -((mpFontEntry->maMetric.mnAscent-mpFontEntry->maMetric.mnLeading)/3);

    if ( !pFontEntry->maMetric.mnUnderlineSize )
    {
        pFontEntry->maMetric.mnUnderlineSize    = nLineHeight;
        pFontEntry->maMetric.mnUnderlineOffset  = nUnderlineOffset - nLineHeight2;
        if ( bVertical && (nLeading < mpFontEntry->maMetric.mnDescent) )
        {
            if ( nLineHeight > 1 )
                pFontEntry->maMetric.mnUnderlineSize--;
        }
    }
    if ( !pFontEntry->maMetric.mnBUnderlineSize )
    {
        pFontEntry->maMetric.mnBUnderlineSize   = nBLineHeight;
        pFontEntry->maMetric.mnBUnderlineOffset = nUnderlineOffset - nBLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnDUnderlineSize )
    {
        pFontEntry->maMetric.mnDUnderlineSize    = n2LineHeight;
        pFontEntry->maMetric.mnDUnderlineOffset1 = nUnderlineOffset - n2LineDY2 - n2LineHeight;
        pFontEntry->maMetric.mnDUnderlineOffset2 = pFontEntry->maMetric.mnDUnderlineOffset1 + n2LineDY + n2LineHeight;
    }
    if ( !pFontEntry->maMetric.mnWUnderlineSize )
    {
        long nWCalcSize = mpFontEntry->maMetric.mnDescent;
        if ( bVertical )
            nWCalcSize = nLeading;
        if ( nWCalcSize < 6 )
        {
            if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
                pFontEntry->maMetric.mnWUnderlineSize = nWCalcSize;
            else
                pFontEntry->maMetric.mnWUnderlineSize = 3;
        }
        else
            pFontEntry->maMetric.mnWUnderlineSize = ((nWCalcSize*50)+50) / 100;
        pFontEntry->maMetric.mnWUnderlineOffset = nUnderlineOffset;
    }
    if ( !pFontEntry->maMetric.mnStrikeoutSize )
    {
        pFontEntry->maMetric.mnStrikeoutSize    = nLineHeight;
        pFontEntry->maMetric.mnStrikeoutOffset  = nStrikeoutOffset - nLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnBStrikeoutSize )
    {
        pFontEntry->maMetric.mnBStrikeoutSize   = nBLineHeight;
        pFontEntry->maMetric.mnBStrikeoutOffset = nStrikeoutOffset - nBLineHeight2;
    }
    if ( !pFontEntry->maMetric.mnDStrikeoutSize )
    {
        pFontEntry->maMetric.mnDStrikeoutSize    = n2LineHeight;
        pFontEntry->maMetric.mnDStrikeoutOffset1 = nStrikeoutOffset - n2LineDY2 - n2LineHeight;
        pFontEntry->maMetric.mnDStrikeoutOffset2 = pFontEntry->maMetric.mnDStrikeoutOffset1 + n2LineDY + n2LineHeight;
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
                                     FontUnderline eUnderline )
{
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
        if ( !pFontEntry->maMetric.mnWUnderlineSize )
            ImplInitTextLineSize();
        nLineHeight = pFontEntry->maMetric.mnWUnderlineSize;
        if ( (eUnderline == UNDERLINE_SMALLWAVE) &&
             (nLineHeight > 3) )
            nLineHeight = 3;
        long nLineWidth = (mnDPIX/300);
        if ( !nLineWidth )
            nLineWidth = 1;
        if ( eUnderline == UNDERLINE_BOLDWAVE )
            nLineWidth *= 2;
        nLinePos = nY + pFontEntry->maMetric.mnWUnderlineOffset - (nLineHeight / 2);
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
            if ( !pFontEntry->maMetric.mnUnderlineSize )
                ImplInitTextLineSize();
            nLineHeight = pFontEntry->maMetric.mnUnderlineSize;
            nLinePos    = nY + pFontEntry->maMetric.mnUnderlineOffset;
        }
        else if ( (eUnderline == UNDERLINE_BOLD) ||
                  (eUnderline == UNDERLINE_BOLDDOTTED) ||
                  (eUnderline == UNDERLINE_BOLDDASH) ||
                  (eUnderline == UNDERLINE_BOLDLONGDASH) ||
                  (eUnderline == UNDERLINE_BOLDDASHDOT) ||
                  (eUnderline == UNDERLINE_BOLDDASHDOTDOT) )
        {
            if ( !pFontEntry->maMetric.mnBUnderlineSize )
                ImplInitTextLineSize();
            nLineHeight = pFontEntry->maMetric.mnBUnderlineSize;
            nLinePos    = nY + pFontEntry->maMetric.mnBUnderlineOffset;
        }
        else if ( eUnderline == UNDERLINE_DOUBLE )
        {
            if ( !pFontEntry->maMetric.mnDUnderlineSize )
                ImplInitTextLineSize();
            nLineHeight = pFontEntry->maMetric.mnDUnderlineSize;
            nLinePos    = nY + pFontEntry->maMetric.mnDUnderlineOffset1;
            nLinePos2   = nY + pFontEntry->maMetric.mnDUnderlineOffset2;
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

void OutputDevice::ImplDrawTextLines( long nX, long nY,
                                      const sal_Unicode* pStr, xub_StrLen nLen,
                                      const long* pDXAry,
                                      FontStrikeout eStrikeout,
                                      FontUnderline eUnderline,
                                      BOOL bWordLine )
{
    if ( bWordLine )
    {
        ::rtl::OUString aText( pStr, nLen );
        uno::Reference < i18n::XBreakIterator > xBI = vcl::unohelper::CreateBreakIterator();
        uno::Reference< linguistic2::XHyphenator > xHyph;
        i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, 1 );
        i18n::LineBreakUserOptions aUserOptions;

        i18n::Boundary aBoundary = xBI->getWordBoundary( aText, 0, GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES, TRUE );
        while ( ( aBoundary.startPos >= 0 ) && ( aBoundary.startPos < nLen ) )
        {
            xub_StrLen nWordEnd = Max( (xub_StrLen)aBoundary.endPos, nLen );
            long nTempX = ImplGetTextWidth( pStr, aBoundary.startPos, pDXAry );
            long nWidth = ImplGetTextWidth( pStr+aBoundary.startPos, aBoundary.endPos-aBoundary.startPos, pDXAry );
            ImplDrawTextLine( nX, nX + nTempX, nY, nWidth, eStrikeout, eUnderline );
            aBoundary = xBI->nextWord( aText, aBoundary.endPos, GetSettings().GetLocale(), i18n::WordType::ANYWORD_IGNOREWHITESPACES );
        }
    }
    else
    {
        ImplDrawTextLine( nX, nX, nY, ImplGetTextWidth( pStr, nLen, pDXAry ), eStrikeout, eUnderline );
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawMnemonicLine( long nX, long nY, xub_Unicode c )
{
    ImplDrawTextLines( nX, nY, &c, 1, NULL, STRIKEOUT_NONE, UNDERLINE_SINGLE, FALSE );
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
    long            nHeight = mpFontEntry->mnLineHeight;
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
            Point       aTempPoint;
            Polygon     aPoly( Rectangle( aTempPoint, aSize ) );
            long        nOldOffX = mnOutOffX;
            long        nOldOffY = mnOutOffY;
            BOOL        bOldMap = mbMap;

            aTempPoint.Y() = nOff;
            aPoly.Rotate( aTempPoint, mpFontEntry->mnOwnOrientation );
            const Rectangle aBound( aPoly.GetBoundRect() );

            mnOutOffX   = 0L;
            mnOutOffY   = 0L;
            mbMap       = FALSE;

            DrawMask( Point( nX + aBound.Left(),
                             nY + aBound.Top() - mpFontEntry->maMetric.mnAscent ),
                      aBmp, GetTextColor() );

            mnOutOffX   = nOldOffX;
            mnOutOffY   = nOldOffY;
            mbMap       = bOldMap;
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
                long* pCharWidthAry = pFontEntry->maWidthAry;
                long  nFactor = pFontEntry->mnWidthFactor;
                long  nOffset = 0;
                long  aStackAry[128];
                long* pTempDXAry = (long*)ImplGetStackBuffer( sizeof(long)*(nLen-1), aStackAry, sizeof( aStackAry ) );
                for ( USHORT i = 0; i < nLen-1; i++ )
                {
                    nOffset += ImplGetCharWidth( pStr[i] );
                    pTempDXAry[i] = nOffset / nFactor;
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
                long*               pCharWidthAry = pFontEntry->maWidthAry;
                long                nFactor = pFontEntry->mnWidthFactor;
                long                nOffset = 0;
                long                nDiff;
                long                nTempX = nX;
                const sal_Unicode*  pTempStr = pStr;
                xub_StrLen          nCombineChars = 1;
                for ( xub_StrLen i = 0; i < nLen-1; i++ )
                {
                    nOffset += ImplGetCharWidth( pStr[i] );
                    nDiff = (nOffset/nFactor) - pDXAry[i];
                    if ( (nDiff < -1) || (nDiff > 0) )
                    {
                        mpGraphics->DrawText( nTempX, nY, pTempStr, nCombineChars );
                        nTempX    = nX+pDXAry[i];
                        nOffset   = pDXAry[i]*nFactor;
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
                               maFont.IsWordLineMode() );
        }
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawSpecialText( long nX, long nY,
                                        const xub_Unicode* pStr, xub_StrLen nLen,
                                        const long* pDXAry )
{
    Color aOldColor = GetTextColor();
    Color aOldTextLineColor = GetTextLineColor();

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
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long*           pCharWidthAry = pFontEntry->maWidthAry;
    long            nFactor = pFontEntry->mnWidthFactor;

    // Breiten-Array fuer errechnete Werte mit den Breiten der einzelnen
    // Character fuellen
    xub_StrLen  i;
    long        nSum = 0;
    for ( i = 0; i < nLen; i++ )
    {
        // Characterbreiten ueber Array holen
        nSum += ImplGetCharWidth( pStr[i] );
        pDXAry[i] = nSum / nFactor;
    }
    nSum /= nFactor;

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
    if ( rStr.Len() && ( nWidth > 0 ) )
    {
        ::rtl::OUString aText( rStr );
        uno::Reference < i18n::XBreakIterator > xBI;
        uno::Reference< linguistic2::XHyphenator > xHyph;
        i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, 1 );
        i18n::LineBreakUserOptions aUserOptions;

        USHORT nPos = 0;
        USHORT nLen = rStr.Len();
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
                nBreakPos = aLBR.breakIndex;
                if ( nBreakPos <= nPos )
                    nBreakPos = nSoftBreak;
                nLineWidth = GetTextWidth( rStr, nPos, nBreakPos-nPos );
            }

            if ( nLineWidth > nMaxLineWidth )
                nMaxLineWidth = nLineWidth;

            if ( nBreakPos == nPos )
                nBreakPos++;

            rLineInfo.AddLine( new ImplTextLineInfo( nLineWidth, nPos, nBreakPos-nPos ) );

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

    return nMaxLineWidth;
}

// =======================================================================

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
                                 FontUnderline eUnderline )
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
    ImplDrawTextLine( aPos.X(), aPos.X(), aPos.Y(), nWidth, eStrikeout, eUnderline );
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
        long*           pCharWidthAry = pFontEntry->maWidthAry;
        long            nFactor = pFontEntry->mnWidthFactor;
        USHORT          i;

        // DX-Array berechnen
        long  nOffset = 0;
        long  aStackAry[128];
        long* pDXAry = (long*)ImplGetStackBuffer( sizeof(long)*(nLen-1), aStackAry, sizeof( aStackAry ) );
        for ( i = 0; i < nLen-1; i++ )
        {
            nOffset += pCharWidthAry[(unsigned char)pStr[i]];
            pDXAry[i] = nOffset / nFactor;
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
            long* pCharWidthAry = pFontEntry->maWidthAry;

            // Bei Fixed-Fonts reicht eine Multiplikation
            // Not TRUE for all Fonts, like CJK Fonts
//            if ( pFontEntry->mbFixedFont )
//            {
//                nWidth = pCharWidthAry['A'] * nLen;
//                nWidth /= pFontEntry->mnWidthFactor;
//            }
//            else
            {
                const sal_Unicode*  pStr = rStr.GetBuffer();
                const sal_Unicode*  pTempStr;
                USHORT          nTempLen;
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

                // Kerning beruecksichtigen (tun wir nur bei Fonts ohne feste Breite)
                if ( mbKerning )
                    nWidth += ImplCalcKerning( pStr, nLen, NULL, 0 );
            }
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

    long nHeight = mpFontEntry->mnLineHeight;

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
    long*               pCharWidthAry = pFontEntry->maWidthAry;
    long                nFactor = pFontEntry->mnWidthFactor;
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
        pDXAry[i] = nOffset / nFactor;
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
    long*               pCharWidthAry = pFontEntry->maWidthAry;
    long                nFactor = pFontEntry->mnWidthFactor;
    const sal_Unicode*  pStr;
    long                nCalcWidth = 0;
    xub_StrLen          nLastIndex;

    if ( mbMap )
    {
        nTextWidth = ImplLogicWidthToDevicePixel( nTextWidth*10 );
        nTextWidth *= nFactor;
        nTextWidth /= 10;
        if ( nCharExtra )
        {
            nCharExtra = ImplLogicWidthToDevicePixel( nCharExtra*10 );
            nCharExtra *= nFactor;
            nCharExtra /= 10;
        }
    }
    else
    {
        nCharExtra *= nFactor;
        nTextWidth *= nFactor;
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
            nCalcWidth += ImplCalcKerning( pStr, 2, NULL, 0 )*nFactor;
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
    long*               pCharWidthAry = pFontEntry->maWidthAry;
    long                nFactor = pFontEntry->mnWidthFactor;
    const sal_Unicode*  pStr;
    long                nTextWidth2;
    long                nCalcWidth = 0;
    xub_StrLen          nIndex2 = STRING_LEN;
    xub_StrLen          nLastIndex;

    if ( mbMap )
    {
        nTextWidth = ImplLogicWidthToDevicePixel( nTextWidth*10 );
        nTextWidth *= nFactor;
        nTextWidth /= 10;
        if ( nCharExtra )
        {
            nCharExtra = ImplLogicWidthToDevicePixel( nCharExtra*10 );
            nCharExtra *= nFactor;
            nCharExtra /= 10;
        }
    }
    else
    {
        nCharExtra *= nFactor;
        nTextWidth *= nFactor;
    }

    // Letzte Index-Position ermitteln
    if ( (ULONG)nIndex+nLen > rStr.Len() )
        nLastIndex = rStr.Len();
    else
        nLastIndex = nIndex + nLen;

    nTextWidth2 = nTextWidth - ImplGetCharWidth( nExtraChar ) - nCharExtra;

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
            nCalcWidth += ImplCalcKerning( pStr, 2, NULL, 0 )*nFactor;
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

    long        nFactor = mpFontEntry->mnWidthFactor;
    sal_Unicode nCharCount = nLastChar-nFirstChar+1;

    if ( mbMap )
    {
        while ( nCharCount )
        {
            *pWidthAry = ImplDevicePixelToLogicWidth( ImplGetCharWidth( nFirstChar ) ) / nFactor;
            pWidthAry++;
            nFirstChar++;
            nCharCount--;
        }
    }
    else
    {
        while ( nCharCount )
        {
            *pWidthAry = ImplGetCharWidth( nFirstChar ) / nFactor;
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
// !!! UNICODE !!!        aFontInfo.SetCharSet( ImplGetFakeEncoding( pData->meCharSet ) );
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
    if ( !mbKerning )
        aMetric.SetKerning( FALSE );

    // restliche Metricen setzen
    aMetric.mpImplMetric->meType        = pMetric->meType;
    aMetric.mpImplMetric->mbDevice      = pMetric->mbDevice;
    aMetric.mpImplMetric->mnAscent      = ImplDevicePixelToLogicHeight( pMetric->mnAscent );
    aMetric.mpImplMetric->mnDescent     = ImplDevicePixelToLogicHeight( pMetric->mnDescent );
    aMetric.mpImplMetric->mnLeading     = ImplDevicePixelToLogicHeight( pMetric->mnLeading );
    aMetric.mpImplMetric->mnLineHeight  = ImplDevicePixelToLogicHeight( pMetric->mnAscent+pMetric->mnDescent );
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

#ifndef REMOTE_APPSERVER
    if ( mpGraphics || ImplGetGraphics() )
    {
        Font*   pOldFont;
        long    nLeft, nTop, nWidth, nHeight;
        long    nFontWidth, nFontHeight;
        long    nOrgWidth, nOrgHeight;

        if ( bOptimize )
        {
            pOldFont = new Font( GetFont() );

            Font    aFont( *pOldFont );
            Size    aFontSize( LogicToPixel( aFont.GetSize() ) );

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
                nOrgWidth = pOldFont->GetSize().Width();
                nOrgHeight = pOldFont->GetSize().Height();
            }
            else
            {
                aFont.SetSize( PixelToLogic( Size( 0, 500 ) ) );
                ((OutputDevice*)this)->SetFont( aFont );
                nFontWidth = nFontHeight = aFont.GetSize().Height();
                nOrgWidth = nOrgHeight = pOldFont->GetSize().Height();
            }
        }

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

        if ( bOptimize )
        {
            ((OutputDevice*)this)->SetFont( *pOldFont );
            delete pOldFont;
        }

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
            long            nHeight = mpFontEntry->mnLineHeight;
            Point           aOffset( nWidth >> 1, 8 );
            Size            aSize( nWidth + ( aOffset.X() << 1 ), nHeight + ( aOffset.Y() << 1 ) );

            if ( pVDev->SetOutputSizePixel( aSize ) )
            {
                Font    aFont( GetFont() );
                Bitmap  aBmp;

                aFont.SetShadow( FALSE );
                aFont.SetOutline( FALSE );
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
                    long                nRight, nBottom;
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
        Font*       pOldFont;
        USHORT*     pPolySizes = NULL;
        SalPoint*   pPoints = NULL;
        BYTE*       pFlags = NULL;
        long        nFontWidth, nFontHeight;
        long        nOrgWidth, nOrgHeight;
        ULONG       nPolyCount;

        if ( bOptimize )
        {
            pOldFont = new Font( GetFont() );

            Font    aFont( *pOldFont );
            Size    aFontSize( LogicToPixel( aFont.GetSize() ) );

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
                nOrgWidth = pOldFont->GetSize().Width();
                nOrgHeight = pOldFont->GetSize().Height();
            }
            else
            {
                aFont.SetSize( PixelToLogic( Size( 0, 500 ) ) );
                ((OutputDevice*)this)->SetFont( aFont );
                nFontWidth = nFontHeight = aFont.GetSize().Height();
                nOrgWidth = nOrgHeight = pOldFont->GetSize().Height();
            }
        }

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

        if ( bOptimize )
        {
            ((OutputDevice*)this)->SetFont( *pOldFont );
            delete pOldFont;
        }

        if ( !bRet && (OUTDEV_PRINTER != meOutDevType) )
        {
            if ( bOptimize )
            {
                if( mbNewFont )
                    ImplNewFont();
                if( mbInitFont )
                    ImplInitFont();
            }

            Font            aFont( GetFont() );
            VirtualDevice*  pVDev = new VirtualDevice( 1 );
            const Size      aFontSize( pVDev->LogicToPixel( Size( 0, GLYPH_FONT_HEIGHT ), MAP_POINT ) );
            const long      nOrgWidth = ImplGetTextWidth( &cChar, 1, NULL );
            const long      nOrgHeight = mpFontEntry->mnLineHeight;

            aFont.SetShadow( FALSE );
            aFont.SetOutline( FALSE );
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
            const long      nHeight = pVDev->mpFontEntry->mnLineHeight;
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
    }
#else
    if ( mbNewFont )
        ImplNewFont();
    if ( mbInitFont )
        ImplInitFont();

    bRet = mpGraphics->GetGlyphOutline( cChar, rPolyPoly, bOptimize );

    if( bRet )
    {
        for( USHORT i = 0UL, nCount = rPolyPoly.Count(); i < nCount; i++ )
        {
            Polygon& rPoly = rPolyPoly[ i ];

            for( USHORT n = 0, nSize = rPoly.GetSize(); n < nSize; n++ )
            {
                Point& rPt = rPoly[ n ];
                rPt.X() = ImplDevicePixelToLogicWidth( rPt.X() );
                rPt.Y() = ImplDevicePixelToLogicHeight( rPt.Y() );
            }
        }
    }

#endif

    if( !bRet )
        rPolyPoly = PolyPolygon();

    return bRet;
}
