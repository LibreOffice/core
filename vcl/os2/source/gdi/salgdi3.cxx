/*************************************************************************
 *
 *  $RCSfile: salgdi3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:34 $
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

#define INCL_GRE_STRINGS

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <tools/svpm.h>

#define _SV_SALGDI3_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
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

// -----------
// - Defines -
// -----------

#define SAL_DRAWTEXT_STACKBUF           128

#define FONTTAB_FACTOR_PRINTER          18
#define FONTTAB_FACTOR_DISPLAY          9
static unsigned char aFontCharTab32[32] =
{
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
   16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31
};

// =======================================================================

static PM_USHORT ImplSalToCharSet( CharSet eCharSet )
{
    // !!! Fuer DBCS-Systeme muss dieser Code auskommentiert werden und 0
    // !!! zurueckgegeben werden, solange die DBCS-Charsets nicht
    // !!! durchgereicht werden

    switch ( eCharSet )
    {
        case CHARSET_IBMPC_437:
            return 437;

        case CHARSET_IBMPC_850:
            return 850;

        case CHARSET_IBMPC_860:
            return 860;

        case CHARSET_IBMPC_861:
            return 861;

        case CHARSET_IBMPC_863:
            return 863;

        case CHARSET_IBMPC_865:
            return 865;

        case CHARSET_ANSI:
            return 1004;

        case CHARSET_SYMBOL:
            return 65400;
    }

    return 0;
}

// -----------------------------------------------------------------------

static CharSet ImplCharSetToSal( PM_USHORT usCodePage )
{
    switch ( usCodePage )
    {
        case 437:
            return CHARSET_IBMPC_437;

        case 850:
            return CHARSET_IBMPC_850;

        case 860:
            return CHARSET_IBMPC_860;

        case 861:
            return CHARSET_IBMPC_861;

        case 863:
            return CHARSET_IBMPC_863;

        case 865:
            return CHARSET_IBMPC_865;

        case 1004:
            return CHARSET_ANSI;

        case 65400:
            return CHARSET_SYMBOL;

    }

    return CHARSET_DONTKNOW;
}

// -----------------------------------------------------------------------

static FontWeight ImplWeightToSal( PM_USHORT nWeight )
{
    // Falls sich jemand an die alte Doku gehalten hat
    if ( nWeight > 999 )
        nWeight /= 1000;

    switch ( nWeight )
    {
        case 1:
            return WEIGHT_THIN;

        case 2:
            return WEIGHT_ULTRALIGHT;

        case 3:
            return WEIGHT_LIGHT;

        case 4:
            return WEIGHT_SEMILIGHT;

        case 5:
            return WEIGHT_NORMAL;

        case 6:
            return WEIGHT_SEMIBOLD;

        case 7:
            return WEIGHT_BOLD;

        case 8:
            return WEIGHT_ULTRABOLD;

        case 9:
            return WEIGHT_BLACK;
    }

    return WEIGHT_DONTKNOW;
}

// -----------------------------------------------------------------------

static XubString ImpStyleNameToSal( const xub_Unicode* pFamilyName,
                                   const xub_Unicode* pFaceName,
                                   USHORT nLen )
{
    if ( !nLen )
        nLen = WSstrlen( pFamilyName );

    // FamilyName gegebenenfalls abschneiden
    if ( WSstrncmp( pFamilyName, pFaceName, nLen ) == 0 )
    {
        USHORT nFaceLen = (USHORT)WSstrlen( pFaceName+nLen );
        // Ist Facename laenger, schneiden wir den FamilyName ab
        if ( nFaceLen > 1 )
            return XubString( pFaceName+(nLen+1), nFaceLen-1 );
        else
            return XubString();
    }
    else
        return XubString( pFaceName );
}

// =======================================================================

void SalGraphics::SetTextColor( SalColor nSalColor )
{
    CHARBUNDLE cb;

    cb.lColor = RGBCOLOR( SALCOLOR_RED( nSalColor ),
                          SALCOLOR_GREEN( nSalColor ),
                          SALCOLOR_BLUE( nSalColor ) );

    // set default color attributes
    GpiSetAttrs( maGraphicsData.mhPS,
                 PRIM_CHAR,
                 CBB_COLOR,
                 0,
                 &cb );
}

// -----------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData* pFont )
{
    ImplFontData* pFontData = pFont->mpFontData;
    FATTRS        aFAttrs;
    BOOL          bOutline;

    memset( &aFAttrs, 0, sizeof( FATTRS ) );
    aFAttrs.usRecordLength = sizeof( FATTRS );

    aFAttrs.usCodePage      = ImplSalToCharSet( pFont->meCharSet );
    aFAttrs.lMaxBaselineExt = pFont->mnHeight;
    aFAttrs.lAveCharWidth   = pFont->mnWidth;

    // do we have a pointer to the FONTMETRICS of the selected font? -> use it!
    if ( pFontData )
    {
        PFONTMETRICS pFontMetric = (PFONTMETRICS)pFontData->mpSysData;
        strcpy( (char*)(aFAttrs.szFacename), pFontMetric->szFacename );
        aFAttrs.lMatch     = pFontMetric->lMatch;
        aFAttrs.idRegistry = pFontMetric->idRegistry;

        bOutline = (pFontMetric->fsDefn & FM_DEFN_OUTLINE) != 0;
        maGraphicsData.mbFontIsFixed = (pFontMetric->fsType & FM_TYPE_FIXED) != 0;

        if ( bOutline )
        {
            aFAttrs.fsFontUse |= FATTR_FONTUSE_OUTLINE;
            if ( pFont->mnOrientation )
                aFAttrs.fsFontUse |= FATTR_FONTUSE_TRANSFORMABLE;
        }
        else
        {
            aFAttrs.lMaxBaselineExt = pFontData->mnHeight;
            aFAttrs.lAveCharWidth   = pFontData->mnWidth;
        }

        if ( (pFont->mpFontData->meItalic == ITALIC_NONE) && (pFont->meItalic != ITALIC_NONE) )
             aFAttrs.fsSelection |= FATTR_SEL_ITALIC;
        if ( ((short)pFont->meWeight - (short)pFont->mpFontData->meWeight >= 2) )
             aFAttrs.fsSelection |= FATTR_SEL_BOLD;
    }
    else
    {
        String aName = pFont->maName.GetToken( 0 );
        strncpy( (char*)(aFAttrs.szFacename), aName, sizeof( aFAttrs.szFacename ) );

        bOutline = FALSE;
        maGraphicsData.mbFontIsFixed = FALSE;

        if ( pFont->meItalic != ITALIC_NONE )
            aFAttrs.fsSelection |= FATTR_SEL_ITALIC;
        if ( pFont->meWeight > WEIGHT_MEDIUM )
            aFAttrs.fsSelection |= FATTR_SEL_BOLD;
    }

    if ( GpiCreateLogFont( maGraphicsData.mhPS, NULL, 10, &aFAttrs ) == GPI_ERROR )
        return SAL_SETFONT_REMOVEANDMATCHNEW;

    CHARBUNDLE aBundle;

    PM_ULONG nAttrsDefault = 0;
    PM_ULONG nAttrs = CBB_SET;
    aBundle.usSet = 10;

    nAttrs |= CBB_BOX;
    if ( bOutline )
    {
        aBundle.sizfxCell.cy = MAKEFIXED( pFont->mnHeight, 0 );

        if ( !pFont->mnWidth )
        {
            LONG nXFontRes;
            LONG nYFontRes;
            LONG nHeight;

            // Auf die Aufloesung achten, damit das Ergebnis auch auf
            // Drucken mit 180*360 DPI stimmt. Ausserdem muss gerundet
            // werden, da auf meinem OS2 beispielsweise als
            // Bildschirmaufloesung 3618*3622 PixelPerMeter zurueck-
            // gegeben wird
            DevQueryCaps( maGraphicsData.mhDC, CAPS_HORIZONTAL_RESOLUTION, 1, &nXFontRes );
            DevQueryCaps( maGraphicsData.mhDC, CAPS_VERTICAL_RESOLUTION, 1, &nYFontRes );
            nHeight = pFont->mnHeight;
            nHeight *= nXFontRes;
            nHeight += nYFontRes/2;
            nHeight /= nYFontRes;
            aBundle.sizfxCell.cx = MAKEFIXED( nHeight, 0 );
        }
        else
            aBundle.sizfxCell.cx = MAKEFIXED( pFont->mnWidth, 0 );
    }
    else
        nAttrsDefault |= CBB_BOX;

    // set orientation for outlinefonts
    nAttrs |= CBB_ANGLE;
    if ( pFont->mnOrientation )
    {
        if ( bOutline )
        {
            double alpha = (double)(pFont->mnOrientation);
            alpha *= 0.0017453292;   // *PI / 1800
            maGraphicsData.mnOrientationY = (long) (1000.0 * sin( alpha ));
            maGraphicsData.mnOrientationX = (long) (1000.0 * cos( alpha ));
            aBundle.ptlAngle.x = maGraphicsData.mnOrientationX;
            aBundle.ptlAngle.y = maGraphicsData.mnOrientationY;
        }
        else
        {
            nAttrsDefault |= CBB_ANGLE;
            maGraphicsData.mnOrientationX = 1;
            maGraphicsData.mnOrientationY = 0;
        }
    }
    else
    {
        nAttrsDefault |= CBB_ANGLE;
        maGraphicsData.mnOrientationX = 1;
        maGraphicsData.mnOrientationY = 0;
    }

    GpiSetAttrs( maGraphicsData.mhPS, PRIM_CHAR,
                 nAttrs,
                 nAttrsDefault,
                 &aBundle );

    // save info about font
    maGraphicsData.mbFontIsOutline = bOutline;

    return SAL_SETFONT_USEDRAWTEXTARRAY;
}

// -----------------------------------------------------------------------

long SalGraphics::GetCharWidth( USHORT nChar1, USHORT nChar2, long* pWidthAry )
{
    POINTL        aNullPt = { 0, 0 };
    POINTL        aFontCharPtBuf[33];
    unsigned char aFontCharTabBuf[FONTTAB_FACTOR_PRINTER];
    long          nFontTabFactor = maGraphicsData.mbPrinter
                                   ? FONTTAB_FACTOR_PRINTER
                                   : FONTTAB_FACTOR_DISPLAY;

    // Orientation? -> rotate to 0 degree!
    if ( (maGraphicsData.mnOrientationY != 0) && (maGraphicsData.mnOrientationX != 1) )
    {
        CHARBUNDLE aBundle;
        aBundle.ptlAngle.x = 1;
        aBundle.ptlAngle.y = 0;
        GpiSetAttrs( maGraphicsData.mhPS, PRIM_CHAR, CBB_ANGLE, 0, &aBundle );
    }

    // Fixed-Font und Fontbreitentabelle bestimmen
    // Wir benutzen GpiQueryCharStringPos() (und nicht GpiQueryWidthTable())
    // um die Genauigkeit zu erhoehen. In dem String stehen bis auf die
    // ersten 32-Zeichen FONTATB_FACTOR* in der Tabelle um die Genauigkeit
    // um den Faktor FONTTAB_FACTOR (9 bzw. 18) zu steigern. Dies muessen wir tun,
    // da auf einen ganzen String die Genauigkeit der Zeichenbreiten
    // groesser als Pixel ist. Somit muessen die Werte, die in der Tabelle
    // abgefragt werden durch FONTTAB_FACTOR dividiert werden.
    USHORT i = 0;
    if ( maGraphicsData.mbFontIsFixed )
    {
        // Bei FixedFonts brauchen wir nicht soviele Zeichen
        // uebergeben, da alle gleich breit sind
        memset( aFontCharTabBuf, 'x', nFontTabFactor );
        GpiQueryCharStringPosAt( maGraphicsData.mhPS, &aNullPt, 0, nFontTabFactor,
                                 (PCH)aFontCharTabBuf, NULL, aFontCharPtBuf );

        long nWidth = aFontCharPtBuf[nFontTabFactor].x - aFontCharPtBuf[0].x;
        long nFontTabFactor2 = nFontTabFactor/2;
        // Breite auf ganze Pixel alignen, damit Schriftbild besser aussieht
        nWidth = (nWidth+nFontTabFactor2)/nFontTabFactor;
        for ( ; i < 256; i++ )
            pWidthAry[i] = nWidth;
        nFontTabFactor = 1;
    }
    else
    {
        // Die ersten 32 Zeichen haben nur eine einfache Genauigkeit, damit
        // wir nicht soviele Werte abfragen muessen
        GpiQueryCharStringPosAt( maGraphicsData.mhPS, &aNullPt, 0, 32,
                                 (PCH)aFontCharTab32, NULL, aFontCharPtBuf );
        for ( ; i < 32; i++ )
            pWidthAry[ i ] = (aFontCharPtBuf[i+1].x - aFontCharPtBuf[i].x) * nFontTabFactor;

        // Die restlichen Zeichen mit (fast) genauer Aufloesung
        for ( ; i < 256; i++ )
        {
            memset( aFontCharTabBuf, i, nFontTabFactor );
            GpiQueryCharStringPosAt( maGraphicsData.mhPS, &aNullPt, 0, nFontTabFactor,
                                     (PCH)aFontCharTabBuf, NULL, aFontCharPtBuf );
            pWidthAry[ i ] = aFontCharPtBuf[nFontTabFactor].x - aFontCharPtBuf[0].x;
        }
    }

    // Orientation? -> rotate back!
    if ( (maGraphicsData.mnOrientationY != 0) && (maGraphicsData.mnOrientationX != 1) )
    {
        CHARBUNDLE aBundle;
        aBundle.ptlAngle.x = maGraphicsData.mnOrientationX;
        aBundle.ptlAngle.y = maGraphicsData.mnOrientationY;
        GpiSetAttrs( maGraphicsData.mhPS, PRIM_CHAR, CBB_ANGLE, 0, &aBundle );
    }

    return nFontTabFactor;
}

// -----------------------------------------------------------------------

void SalGraphics::GetFontMetric( ImplFontMetricData* pMetric )
{
    FONTMETRICS aOS2Metric;
    GpiQueryFontMetrics( maGraphicsData.mhPS, sizeof( aOS2Metric ), &aOS2Metric );

    pMetric->maName             = aOS2Metric.szFamilyname;
    pMetric->maStyleName        = ImpStyleNameToSal( aOS2Metric.szFamilyname,
                                                     aOS2Metric.szFacename,
                                                     strlen( aOS2Metric.szFamilyname ) );

    pMetric->meCharSet          = ImplCharSetToSal( aOS2Metric.usCodePage );
    pMetric->meWeight           = ImplWeightToSal( aOS2Metric.usWeightClass );

    if ( aOS2Metric.panose.bFamilyType == 3 )
        pMetric->meFamily = FAMILY_SCRIPT;
    else
        pMetric->meFamily = FAMILY_DONTKNOW;

    if ( aOS2Metric.fsType & FM_TYPE_FIXED )
        pMetric->mePitch = PITCH_FIXED;
    else
        pMetric->mePitch = PITCH_VARIABLE;

    if ( aOS2Metric.fsSelection & FM_SEL_ITALIC )
        pMetric->meItalic = ITALIC_NORMAL;
    else
        pMetric->meItalic = ITALIC_NONE;

    if ( aOS2Metric.fsDefn & FM_DEFN_OUTLINE )
    {
        pMetric->meType        = TYPE_SCALABLE;
        pMetric->mnWidth       = aOS2Metric.lEmInc;
    }
    else
    {
        pMetric->meType        = TYPE_RASTER;
        pMetric->mnWidth       = aOS2Metric.lAveCharWidth;
        pMetric->mnOrientation = 0;
    }

    if ( aOS2Metric.fsDefn & FM_DEFN_GENERIC )
        pMetric->mbDevice = FALSE;
    else
        pMetric->mbDevice = TRUE;

    pMetric->mnAscent           = aOS2Metric.lMaxAscender;
    pMetric->mnDescent          = aOS2Metric.lMaxDescender;
    pMetric->mnLeading          = aOS2Metric.lInternalLeading;
    pMetric->mnSlant            = 0;
    pMetric->mnFirstChar        = aOS2Metric.sFirstChar;
    pMetric->mnLastChar         = aOS2Metric.sLastChar;
}

// -----------------------------------------------------------------------

ULONG SalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData* pKernPairs )
{
    if ( !nPairs || !pKernPairs )
    {
        FONTMETRICS aOS2Metric;
        GpiQueryFontMetrics( maGraphicsData.mhPS, sizeof( aOS2Metric ), &aOS2Metric );
        return aOS2Metric.sKerningPairs;
    }

    DBG_ASSERT( sizeof( KERNINGPAIRS ) == sizeof( ImplKernPairData ),
                "SalGraphics::GetKernPairs(): KERNINGPAIRS != ImplKernPairData" );
    // Einige Treiber liefern weniger Kerning-Paare, als wir mit
    // GpiQueryFontMetrics() ermittelt haben. Deshalb hier den Rueckgabewert
    // anpassen, damit der unabhaengige Teil entsprechend darauf reagieren
    // kann
    nPairs = GpiQueryKerningPairs( maGraphicsData.mhPS, nPairs, (KERNINGPAIRS*)pKernPairs );
    return nPairs;
}

// -----------------------------------------------------------------------

void SalGraphics::GetDevFontList( ImplDevFontList* pList )
{
    PFONTMETRICS    pFontMetrics;
    ULONG           nFontMetricCount;
    SalData*        pSalData;

    if ( !maGraphicsData.mbPrinter )
    {
        // Bei Bildschirm-Devices cachen wir die Liste global, da
        // dies im unabhaengigen Teil auch so gemacht wird und wir
        // ansonsten auf geloeschten Systemdaten arbeiten koennten
        pSalData = GetSalData();
        nFontMetricCount    = pSalData->mnFontMetricCount;
        pFontMetrics        = pSalData->mpFontMetrics;
        // Bei Bildschirm-Devices holen wir uns die Fontliste jedesmal neu
        if ( pFontMetrics )
        {
            delete pFontMetrics;
            pFontMetrics        = NULL;
            nFontMetricCount    = 0;
        }
    }
    else
    {
        nFontMetricCount    = maGraphicsData.mnFontMetricCount;
        pFontMetrics        = maGraphicsData.mpFontMetrics;
    }

    // do we have to create the cached font list first?
    if ( !pFontMetrics )
    {
        // query the number of fonts available
        LONG nTemp = 0;
        nFontMetricCount = GpiQueryFonts( maGraphicsData.mhPS,
                                          QF_PUBLIC | QF_PRIVATE,
                                          NULL, &nTemp,
                                          sizeof( FONTMETRICS ), NULL );

        // procede only if at least one is available!
        if ( nFontMetricCount )
        {
            // allocate memory for font list
            pFontMetrics = new FONTMETRICS[nFontMetricCount];

            // query font list
            GpiQueryFonts( maGraphicsData.mhPS,
                           QF_PUBLIC | QF_PRIVATE,
                           NULL,
                           (PLONG)&nFontMetricCount,
                           (LONG) sizeof( FONTMETRICS ),
                           pFontMetrics );
        }

        if ( !maGraphicsData.mbPrinter )
        {
            pSalData->mnFontMetricCount         = nFontMetricCount;
            pSalData->mpFontMetrics             = pFontMetrics;
        }
        else
        {
            maGraphicsData.mnFontMetricCount    = nFontMetricCount;
            maGraphicsData.mpFontMetrics        = pFontMetrics;
        }
    }

    // copy data from the font list
    for( ULONG i = 0; i < nFontMetricCount; i++ )
    {
        PFONTMETRICS pFontMetric = &pFontMetrics[i];

        // Bildschirm-Bitmap-Font's werden nicht fuer den Drucker angeboten
        if ( maGraphicsData.mbPrinter )
        {
            if ( (pFontMetric->fsDefn & (FM_DEFN_OUTLINE | FM_DEFN_GENERIC)) == FM_DEFN_GENERIC )
                // Font nicht aufnehmen
                continue;
        }

        // create new font list element
        ImplFontData* pData         = new ImplFontData;
        pData->maName               = pFontMetric->szFamilyname;
        pData->maStyleName          = ImpStyleNameToSal( pFontMetric->szFamilyname,
                                                         pFontMetric->szFacename,
                                                         strlen( pFontMetric->szFamilyname) );
        pData->mpSysData            = (void*)pFontMetric;
        pData->meCharSet            = ImplCharSetToSal( pFontMetric->usCodePage );
        pData->meWeight             = ImplWeightToSal( pFontMetric->usWeightClass );
        pData->meWidthType          = WIDTH_DONTKNOW;
        if ( pFontMetric->panose.bFamilyType == 3 )
            pData->meFamily = FAMILY_SCRIPT;
        else
            pData->meFamily = FAMILY_DONTKNOW;
        if ( pFontMetric->fsType & FM_TYPE_FIXED )
            pData->mePitch = PITCH_FIXED;
        else
            pData->mePitch = PITCH_VARIABLE;
        if ( pFontMetric->fsSelection & FM_SEL_ITALIC )
            pData->meItalic = ITALIC_NORMAL;
        else
            pData->meItalic = ITALIC_NONE;
        if ( pFontMetric->fsDefn & FM_DEFN_OUTLINE )
            pData->meType = TYPE_SCALABLE;
        else
            pData->meType = TYPE_RASTER;
        if ( pFontMetric->fsDefn & FM_DEFN_GENERIC )
            pData->mbDevice = FALSE;
        else
            pData->mbDevice = TRUE;
        if ( pData->meType != TYPE_RASTER )
        {
            pData->mnWidth          = 0;
            pData->mnHeight         = 0;
            pData->mbOrientation    = TRUE;
        }
        else
        {
            pData->mnWidth          = pFontMetric->lAveCharWidth;
            pData->mnHeight         = pFontMetric->lMaxBaselineExt;
            pData->mbOrientation    = FALSE;
        }
        pData->mnQuality = 0;

        // add font list element to font list
        pList->Add( pData );
    }
}

// -----------------------------------------------------------------------

void SalGraphics::DrawText( long nX, long nY, const xub_Unicode* pStr, USHORT nLen )
{
    DBG_ERROR( "SalGraphics::DrawText called!" );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawTextArray( long nX, long nY,
                                 const xub_Unicode* pStr, USHORT nLen,
                                 const long* pDXAry )
{
    POINTL  aPt;
    LONG    aStackAry[ SAL_DRAWTEXT_STACKBUF ];
    LONG*   pOS2DXAry;

    if ( nLen <= SAL_DRAWTEXT_STACKBUF )
        pOS2DXAry = aStackAry;
    else
        pOS2DXAry = new LONG[nLen];

    aPt.x = nX;
    aPt.y = maGraphicsData.mnHeight - nY;

    pOS2DXAry[0] = (LONG)pDXAry[0];
    for ( USHORT i = 1; i < nLen-1; i++ )
        pOS2DXAry[i] = (LONG)pDXAry[i]-pDXAry[i-1];
    pOS2DXAry[nLen-1] = 0;

    // OS2 kann max. 512-Zeichen lange Strings ausgeben, deshalb kuerzen
    // wir den String auf eine Laenge von 512-Zeichen
    if ( nLen > 512 )
    {
        // Wir versuchen links die Zeichen zu ueberspringen, die evtl. im
        // nicht sichtbaren Bereich liegen
        LONG nWidth = 0;
        LONG nCharWidth;
        while ( nLen > 512 )
        {
            nCharWidth = *pOS2DXAry;
            if ( (aPt.x+nWidth+nCharWidth) >= -999 )
                break;
            nWidth += nCharWidth;
            pStr++;
            pOS2DXAry++;
            nLen--;
        }

        if ( nLen > 512 )
            nLen = 512;

        aPt.x = aPt.x + nWidth;
    }

    GpiCharStringPosAt( maGraphicsData.mhPS, &aPt, NULL, CHS_VECTOR, nLen, (PCH)pStr, pOS2DXAry );

    if ( pOS2DXAry != aStackAry )
        delete pOS2DXAry;
}

// -----------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( xub_Unicode cChar, long* pX, long* pY,
                                     long* pWidth, long* pHeight )
{
    return FALSE;
}

// -----------------------------------------------------------------------

///---------------------------- PUBLIC ROUTINE -------------------------------
// Using the following macro in PMDDIM.H you can use a GRE call to get the char outline
// of system outline fonts.  This call does not work for system bitmap fonts or device fonts.
// Truetype fonts should also work however the data returned for truetype fonts will be mostly
// scanline data until you install the latest fixpak ("31 or 32" I Think). The fix pak will improve
// the data returned for truetype fonts to include curves.  This call was
// implemented in merlin and should be in fix pak 26 for Warp as well.
//
//       #define GreQueryCharOutline(a,b,c,d,e)
// (LONG)Gre32Entry7((HDC)(a),(ULONG)(b),(ULONG)(PBYTE)(c),(ULONG)(d),(ULONG)(e),0L,0x00004256L)
//       #define NGreQueryCharOutline 0x00004256L
//
// LONG APIENTRY QueryCharOutline32 (HDC hdc, ULONG ulCode, PBYTE pBuffer,
//                                   ULONG ulLen, ULONG fl, ULONG hddc,
//                                   ULONG ulFunN)
//
// This function returns outline data information of the specified
// character glyph
//
// Parameters: hdc         Device context handle
//             ulCode      Code point
//             pBuffer     Pointer to outline data to be returned
//             ulLen       Length in bytes of pBuffer
//             fl          Options flags
//                            QCO_FORMAT_GOCA
//                            QCO_FORMAT_IFI
//                            QCO_NO_TRANSFORM
//             hddc
//             ulFunN
// Returns:
//             The number of bytes needed to store character outline
//             GPI_ALTERROR
//

// -----------------------------------------------------------------------

ULONG SalGraphics::GetGlyphOutline( xub_Unicode cChar, USHORT** ppPolySizes,
                                    SalPoint** ppPoints, BYTE** ppFlags )
{
    return 0;
}
