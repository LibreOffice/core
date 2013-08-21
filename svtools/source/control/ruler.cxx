/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <string.h>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <tools/poly.hxx>
#include <vcl/i18nhelp.hxx>

#include <svtools/ruler.hxx>

// =======================================================================

#define RULER_OFF           3
#define RULER_TEXTOFF       5
#define RULER_RESIZE_OFF    4
#define RULER_MIN_SIZE      3

#define RULER_TICK1_WIDTH   1
#define RULER_TICK2_WIDTH   3

#define RULER_VAR_SIZE      8

#define RULER_TAB_HEIGHT2   2
#define RULER_TAB_WIDTH2    2
#define RULER_TAB_CWIDTH    8
#define RULER_TAB_CWIDTH2   4
#define RULER_TAB_CWIDTH3   4
#define RULER_TAB_CWIDTH4   2
#define RULER_TAB_DHEIGHT   4
#define RULER_TAB_DHEIGHT2  1
#define RULER_TAB_DWIDTH    5
#define RULER_TAB_DWIDTH2   3
#define RULER_TAB_DWIDTH3   3
#define RULER_TAB_DWIDTH4   1

#define RULER_UPDATE_LINES  0x01
#define RULER_UPDATE_DRAW   0x02

#define RULER_CLIP          150

// =======================================================================

#define RULER_UNIT_MM       0
#define RULER_UNIT_CM       1
#define RULER_UNIT_M        2
#define RULER_UNIT_KM       3
#define RULER_UNIT_INCH     4
#define RULER_UNIT_FOOT     5
#define RULER_UNIT_MILE     6
#define RULER_UNIT_POINT    7
#define RULER_UNIT_PICA     8
#define RULER_UNIT_CHAR     9
#define RULER_UNIT_LINE    10
#define RULER_UNIT_COUNT    11

// -----------------
// - ImplRulerData -
// -----------------
class ImplRulerData
{
    friend              class Ruler;

private:
    RulerLine*          pLines;
    RulerBorder*        pBorders;
    RulerIndent*        pIndents;
    RulerTab*           pTabs;
    long                nNullVirOff;
    long                nRulVirOff;
    long                nRulWidth;
    long                nPageOff;
    long                nPageWidth;
    long                nNullOff;
    long                nMargin1;
    long                nMargin2;
    long                nLeftFrameMargin;
    long                nRightFrameMargin;
    sal_uInt16              nLines;
    sal_uInt16              nBorders;
    sal_uInt16              nIndents;
    sal_uInt16              nTabs;
    sal_uInt16              nMargin1Style;
    sal_uInt16              nMargin2Style;
    sal_Bool                bAutoPageWidth;
    sal_Bool                bTextRTL;

public:
                        ImplRulerData();
                        ~ImplRulerData();
    ImplRulerData&      operator=( const ImplRulerData& rData );
};

static RulerUnitData aImplRulerUnitTab[RULER_UNIT_COUNT] =
{
{ MAP_100TH_MM,        100,    25,     50,    100,     100, 3, " mm"    }, // MM
{ MAP_100TH_MM,       1000,   250,    500,   1000,    1000, 3, " cm"    }, // CM
{ MAP_MM,             1000,   250,    500,   1000,   10000, 4, " m"     }, // M
{ MAP_CM,           100000, 25000,  50000, 100000,  100000, 6, " km"    }, // KM
{ MAP_100TH_INCH,      100,    10,     50,    100,    2540, 3, "\""     }, // INCH
{ MAP_100TH_INCH,     1200,   120,    600,   1200,   30480, 3, "'"      }, // FOOT
{ MAP_10TH_INCH,    633600, 63360, 316800, 633600, 1609344, 4, " miles" }, // MILE
{ MAP_POINT,             1,    12,     12,     36,     353, 2, " pt"    }, // POINT
{ MAP_100TH_MM,        423,   423,    423,    846,     423, 3, " pi"    }, // PICA
{ MAP_100TH_MM,        371,   371,    371,    743,     371, 3, " ch"    }, // CHAR
{ MAP_100TH_MM,        551,   551,    551,   1102,     551, 3, " li"    }  // LINE
};

// =======================================================================

struct ImplRulerHitTest
{
    long        nPos;
    RulerType   eType;
    sal_uInt16      nAryPos;
    sal_uInt16      mnDragSize;
    sal_Bool        bSize;
    sal_Bool        bSizeBar;
    sal_Bool        bExpandTest;
    ImplRulerHitTest() :
        bExpandTest( sal_False ) {}
};

// =======================================================================

ImplRulerData::ImplRulerData()
{
    memset( this, 0, sizeof( ImplRulerData ) );

    // PageBreite == EditWinBreite
    bAutoPageWidth   = sal_True;
}

// -----------------------------------------------------------------------

ImplRulerData::~ImplRulerData()
{
    delete[] pLines;
    delete[] pBorders;
    delete[] pIndents;
    delete[] pTabs;
}

// -----------------------------------------------------------------------

ImplRulerData& ImplRulerData::operator=( const ImplRulerData& rData )
{
    if( this == &rData )
        return *this;

    delete[] pLines;
    delete[] pBorders;
    delete[] pIndents;
    delete[] pTabs;

    memcpy( this, &rData, sizeof( ImplRulerData ) );

    if ( rData.pLines )
    {
        pLines = new RulerLine[nLines];
        memcpy( pLines, rData.pLines, nLines*sizeof( RulerLine ) );
    }

    if ( rData.pBorders )
    {
        pBorders = new RulerBorder[nBorders];
        memcpy( pBorders, rData.pBorders, nBorders*sizeof( RulerBorder ) );
    }

    if ( rData.pIndents )
    {
        pIndents = new RulerIndent[nIndents];
        memcpy( pIndents, rData.pIndents, nIndents*sizeof( RulerIndent ) );
    }

    if ( rData.pTabs )
    {
        pTabs = new RulerTab[nTabs];
        memcpy( pTabs, rData.pTabs, nTabs*sizeof( RulerTab ) );
    }

    return *this;
}

// =======================================================================

void Ruler::ImplInit( WinBits nWinBits )
{
    // Default WinBits setzen
    if ( !(nWinBits & WB_VERT) )
    {
        nWinBits |= WB_HORZ;

        // --- RTL --- no UI mirroring for horizontal rulers, because
        // the document is also not mirrored
        EnableRTL( sal_False );
    }

    // Variablen initialisieren
    mnWinStyle      = nWinBits;             // Window-Style
    mnBorderOff     = 0;                    // Border-Offset
    mnWinOff        = 0;                    // EditWinOffset
    mnWinWidth      = 0;                    // EditWinWidth
    mnWidth         = 0;                    // Fensterbreite
    mnHeight        = 0;                    // Fensterhoehe
    mnVirOff        = 0;                    // Offset des VirtualDeice vom linke/oberen Rand
    mnVirWidth      = 0;                    // Breite bzw. Hoehe vom VirtualDevice
    mnVirHeight     = 0;                    // Hoehe bzw. Breite vom VirtualDevice
    mnDragPos       = 0;                    // Drag-Position (NullPunkt)
    mnUpdateEvtId   = 0;                    // Noch kein Update-Event verschickt
    mnDragAryPos    = 0;                    // Drag-Array-Index
    mnDragSize      = 0;                    // Wird beim Draggen die Groesse geaendert
    mnDragScroll    = 0;                    // Soll beim Draggen gescrollt werden
    mnDragModifier  = 0;                    // Modifier-Tasten beim Draggen
    mnExtraStyle    = 0;                    // Style des Extra-Feldes
    mnExtraClicks   = 0;                    // Click-Anzahl fuer Extra-Feld
    mnExtraModifier = 0;                    // Modifier-Tasten beim Click im Extrafeld
    mnCharWidth     = 371;
    mnLineHeight    = 551;
    mbCalc          = sal_True;                 // Muessen Pagebreiten neu berechnet werden
    mbFormat        = sal_True;                 // Muss neu ausgegeben werden
    mbDrag          = sal_False;                // Sind wir im Drag-Modus
    mbDragDelete    = sal_False;                // Wird Maus beim Draggen unten rausgezogen
    mbDragCanceled  = sal_False;                // Wurde Dragging abgebrochen
    mbAutoWinWidth  = sal_True;                 // EditWinBreite == RulerBreite
    mbActive        = sal_True;                 // Ist Lineal aktiv
    mnUpdateFlags   = 0;                    // Was soll im Update-Handler upgedatet werden
    mpData          = mpSaveData;           // Wir zeigen auf die normalen Daten
    meExtraType     = RULER_EXTRA_DONTKNOW; // Was im ExtraFeld dargestellt wird
    meDragType      = RULER_TYPE_DONTKNOW;  // Gibt an, was gedragt wird

    // Units initialisieren
    mnUnitIndex     = RULER_UNIT_CM;
    meUnit          = FUNIT_CM;
    maZoom          = Fraction( 1, 1 );
    meSourceUnit    = MAP_100TH_MM;

    // Border-Breiten berechnen
    if ( nWinBits & WB_BORDER )
        mnBorderWidth = 1;
    else
        mnBorderWidth = 0;

    // Einstellungen setzen
    ImplInitSettings( sal_True, sal_True, sal_True );

    // Setup the default size
    Rectangle aRect;
    GetTextBoundRect( aRect, OUString( "0123456789" ) );
    long nDefHeight = aRect.GetHeight() + RULER_OFF*2 + RULER_TEXTOFF*2 + mnBorderWidth;

    Size aDefSize;
    if ( nWinBits & WB_HORZ )
        aDefSize.Height() = nDefHeight;
    else
        aDefSize.Width() = nDefHeight;
    SetOutputSizePixel( aDefSize );
    SetType(WINDOW_RULER);
}

// -----------------------------------------------------------------------

Ruler::Ruler( Window* pParent, WinBits nWinStyle ) :
    Window( pParent, nWinStyle & WB_3DLOOK ),
    maVirDev( *this ),
    maMapMode( MAP_100TH_MM ),
    mpSaveData(new ImplRulerData),
    mpData(0),
    mpDragData(new ImplRulerData)
{
    ImplInit( nWinStyle );
}

// -----------------------------------------------------------------------

Ruler::~Ruler()
{
    if ( mnUpdateEvtId )
        Application::RemoveUserEvent( mnUpdateEvtId );
    delete mpSaveData;
    delete mpDragData;
}

// -----------------------------------------------------------------------

void Ruler::ImplVDrawLine( long nX1, long nY1, long nX2, long nY2 )
{
    if ( nX1 < -RULER_CLIP )
    {
        nX1 = -RULER_CLIP;
        if ( nX2 < -RULER_CLIP )
            return;
    }
    long nClip = mnVirWidth+RULER_CLIP;
    if ( nX2 > nClip )
    {
        nX2 = nClip;
        if ( nX1 > nClip )
            return;
    }

    if ( mnWinStyle & WB_HORZ )
        maVirDev.DrawLine( Point( nX1, nY1 ), Point( nX2, nY2 ) );
    else
        maVirDev.DrawLine( Point( nY1, nX1 ), Point( nY2, nX2 ) );
}

// -----------------------------------------------------------------------

void Ruler::ImplVDrawRect( long nX1, long nY1, long nX2, long nY2 )
{
    if ( nX1 < -RULER_CLIP )
    {
        nX1 = -RULER_CLIP;
        if ( nX2 < -RULER_CLIP )
            return;
    }
    long nClip = mnVirWidth+RULER_CLIP;
    if ( nX2 > nClip )
    {
        nX2 = nClip;
        if ( nX1 > nClip )
            return;
    }

    if ( mnWinStyle & WB_HORZ )
        maVirDev.DrawRect( Rectangle( nX1, nY1, nX2, nY2 ) );
    else
        maVirDev.DrawRect( Rectangle( nY1, nX1, nY2, nX2 ) );
}

// -----------------------------------------------------------------------

void Ruler::ImplVDrawText( long nX, long nY, const OUString& rText, long nMin, long nMax )
{
    Rectangle aRect;
    maVirDev.GetTextBoundRect( aRect, rText );

    long nShiftX = ( aRect.GetWidth() / 2 ) + aRect.Left();
    long nShiftY = ( aRect.GetHeight() / 2 ) + aRect.Top();

    if ( (nX > -RULER_CLIP) && (nX < mnVirWidth+RULER_CLIP) && ( nX < nMax - nShiftX ) && ( nX > nMin + nShiftX ) )
    {
        if ( mnWinStyle & WB_HORZ )
            maVirDev.DrawText( Point( nX - nShiftX, nY - nShiftY ), rText );
        else
            maVirDev.DrawText( Point( nY - nShiftX, nX - nShiftY ), rText );
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplInvertLines( sal_Bool bErase )
{
    // Positionslinien
    if ( mpData->nLines && mbActive && !mbDrag && !mbFormat &&
         !(mnUpdateFlags & RULER_UPDATE_LINES) )
    {
        long n;
        long nNullWinOff = mpData->nNullVirOff+mnVirOff;
        long nRulX1 = mpData->nRulVirOff+mnVirOff;
        long nRulX2 = nRulX1+mpData->nRulWidth;
        long nY = (RULER_OFF*2)+mnVirHeight-1;

        // Rectangle berechnen
        Rectangle aRect;
        if ( mnWinStyle & WB_HORZ )
            aRect.Bottom() = nY;
        else
            aRect.Right() = nY;

        // Linien ausgeben
        for ( sal_uInt16 i = 0; i < mpData->nLines; i++ )
        {
            n = mpData->pLines[i].nPos+nNullWinOff;
            if ( (n >= nRulX1) && (n < nRulX2) )
            {
                if ( mnWinStyle & WB_HORZ )
                {
                    aRect.Left()   = n;
                    aRect.Right()  = n;
                }
                else
                {
                    aRect.Top()    = n;
                    aRect.Bottom() = n;
                }
                if ( bErase )
                {
                    Rectangle aTempRect = aRect;
                    if ( mnWinStyle & WB_HORZ )
                        aTempRect.Bottom() = RULER_OFF-1;
                    else
                        aTempRect.Right() = RULER_OFF-1;
                    Erase( aTempRect );
                    if ( mnWinStyle & WB_HORZ )
                    {
                        aTempRect.Bottom() = aRect.Bottom();
                        aTempRect.Top()    = aTempRect.Bottom()-RULER_OFF+1;
                    }
                    else
                    {
                        aTempRect.Right()  = aRect.Right();
                        aTempRect.Left()   = aTempRect.Right()-RULER_OFF+1;
                    }
                    Erase( aTempRect );
                }
                Invert( aRect );
            }
        }
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawTicks( long nMin, long nMax, long nStart, long nCenter )
{
    long    n = 0;
    long    nTick3 = aImplRulerUnitTab[mnUnitIndex].nTick3;
    long    nTickCount = aImplRulerUnitTab[mnUnitIndex].nTick1;
    Size    aPixSize = maVirDev.LogicToPixel( Size( nTick3, nTick3 ), maMapMode );
    long    nTickWidth;
    sal_Bool    bNoTicks = sal_False;

    long    nTickUnit = 0;
    long    nTick2 = 0;
    if ( mnUnitIndex == RULER_UNIT_CHAR )
    {
        if ( mnCharWidth == 0 )
            mnCharWidth = 371;
        nTick3 = mnCharWidth*2;
        nTickCount = mnCharWidth;
        nTickUnit = mnCharWidth;
        nTick2 = mnCharWidth;
    }
    else if ( mnUnitIndex == RULER_UNIT_LINE )
    {
        if ( mnLineHeight == 0 )
            mnLineHeight = 551;
        nTick3 = mnLineHeight*2;
        nTickCount = mnLineHeight;
        nTickUnit = mnLineHeight;
        nTick2 = mnLineHeight;
    }
    aPixSize = maVirDev.LogicToPixel( Size( nTick3, nTick3 ), maMapMode );

    // Groessenvorberechnung
    // Sizes calculation
    if ( mnWinStyle & WB_HORZ )
        nTickWidth = aPixSize.Width();
    else
    {
        Font aFont = GetFont();
        if ( mnWinStyle & WB_RIGHT_ALIGNED )
            aFont.SetOrientation( 2700 );
        else
            aFont.SetOrientation( 900 );
        maVirDev.SetFont( aFont );
        nTickWidth = aPixSize.Height();
    }
    long nMaxWidth = maVirDev.PixelToLogic( Size( mpData->nPageWidth, 0 ), maMapMode ).Width();
    if ( nMaxWidth < 0 )
        nMaxWidth = -nMaxWidth;

    if (( mnUnitIndex == RULER_UNIT_CHAR ) || ( mnUnitIndex == RULER_UNIT_LINE ))
        nMaxWidth /= nTickUnit;
    else
        nMaxWidth /= aImplRulerUnitTab[mnUnitIndex].nTickUnit;
    OUString aNumStr = OUString::number(nMaxWidth);
    long nTxtWidth = GetTextWidth( aNumStr );
    const long nTextOff   = 4;
    if ( nTickWidth < nTxtWidth+nTextOff )
    {
        // Calculate the scale of the ruler
        long nMulti     = 1;
        long nOrgTick3  = nTick3;
        while ( nTickWidth < nTxtWidth+nTextOff )
        {
            long nOldMulti = nMulti;
            if ( !nTickWidth ) //If nTickWidth equals 0
                nMulti *= 10;
            else if ( nMulti < 10 )
                nMulti++;
            else if ( nMulti < 100 )
                nMulti += 10;
            else if ( nMulti < 1000 )
                nMulti += 100;
            else
                nMulti += 1000;
            // Ueberlauf, dann geben wir nichts aus, da wir bei so einem
            // unsinnigen Massstab sowieso nichts vernuenftiges anzeigen
            // koennen
            if ( nMulti < nOldMulti )
            {
                bNoTicks = sal_True;
                break;
            }

            nTick3 = nOrgTick3 * nMulti;
            aPixSize = maVirDev.LogicToPixel( Size( nTick3, nTick3 ), maMapMode );
            if ( mnWinStyle & WB_HORZ )
                nTickWidth = aPixSize.Width();
            else
                nTickWidth = aPixSize.Height();
        }
        nTickCount = nTick3;
    }
    else
        maVirDev.SetLineColor( GetSettings().GetStyleSettings().GetShadowColor() );

    if ( !bNoTicks )
    {
        long nTick = 0;
        while ( ((nStart-n) >= nMin) || ((nStart+n) <= nMax) )
        {
            // Null-Punkt
            if ( !nTick )
            {
                if ( nStart > nMin )
                {
                    // 0 is only painted when Margin1 is not equal to zero
                    if ( (mpData->nMargin1Style & RULER_STYLE_INVISIBLE) || (mpData->nMargin1 != 0) )
                    {
                        aNumStr = "0";
                        ImplVDrawText( nStart, nCenter, aNumStr );
                    }
                }
            }
            else
            {
                aPixSize = maVirDev.LogicToPixel( Size( nTick, nTick ), maMapMode );

                if ( mnWinStyle & WB_HORZ )
                    n = aPixSize.Width();
                else
                    n = aPixSize.Height();

                // Tick3 - Output (Text)
                if ( !(nTick % nTick3) )
                {
                    if ( ( mnUnitIndex == RULER_UNIT_CHAR ) || ( mnUnitIndex == RULER_UNIT_LINE ) )
                        aNumStr = OUString::number( nTick / nTickUnit );
                    else
                        aNumStr = OUString::number( nTick / aImplRulerUnitTab[mnUnitIndex].nTickUnit );

                    ImplVDrawText( nStart + n, nCenter, aNumStr, nMin, nMax );
                    ImplVDrawText( nStart - n, nCenter, aNumStr, nMin, nMax );
                }
                // Tick/Tick2 - Output (Strokes)
                else
                {
                    if ( ( mnUnitIndex != RULER_UNIT_CHAR ) && ( mnUnitIndex != RULER_UNIT_LINE ) )
                        nTick2 = aImplRulerUnitTab[mnUnitIndex].nTick2;
                    if ( !(nTick % nTick2 ) )
                        nTickWidth = RULER_TICK2_WIDTH;
                    else
                        nTickWidth = RULER_TICK1_WIDTH;
                    long nT1 = nCenter-(nTickWidth/2);
                    long nT2 = nT1+nTickWidth-1;
                    long nT;

                    nT = nStart+n;
                    if ( nT < nMax )
                        ImplVDrawLine( nT, nT1, nT, nT2 );
                    nT = nStart-n;
                    if ( nT > nMin )
                        ImplVDrawLine( nT, nT1, nT, nT2 );
                }
            }
            // #i49017# with some zoom factors the value nTick can overflow
            if( ((sal_uLong)nTick + (sal_uLong)nTickCount) > (sal_uLong)LONG_MAX)
                break;
            nTick += nTickCount;
        }
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawBorders( long nMin, long nMax, long nVirTop, long nVirBottom )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    long    n;
    long    n1;
    long    n2;
    long    nTemp1;
    long    nTemp2;
    sal_uInt16  i;

    for ( i = 0; i < mpData->nBorders; i++ )
    {
        if ( mpData->pBorders[i].nStyle & RULER_STYLE_INVISIBLE )
            continue;

        n1 = mpData->pBorders[i].nPos+mpData->nNullVirOff;
        n2 = n1+mpData->pBorders[i].nWidth;

        if ( ((n1 >= nMin) && (n1 <= nMax)) || ((n2 >= nMin) && (n2 <= nMax)) )
        {
            if ( (n2-n1) > 3 )
            {
                maVirDev.SetLineColor();
                maVirDev.SetFillColor( rStyleSettings.GetFaceColor() );
                ImplVDrawRect( n1, nVirTop, n2, nVirBottom );

                maVirDev.SetLineColor( rStyleSettings.GetLightColor() );
                ImplVDrawLine( n1+1, nVirTop, n1+1, nVirBottom );
                ImplVDrawLine( n1, nVirTop, n2, nVirTop );
                maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
                ImplVDrawLine( n1, nVirTop, n1, nVirBottom );
                ImplVDrawLine( n1, nVirBottom, n2, nVirBottom );
                ImplVDrawLine( n2-1, nVirTop, n2-1, nVirBottom );
                maVirDev.SetLineColor( rStyleSettings.GetDarkShadowColor() );
                ImplVDrawLine( n2, nVirTop, n2, nVirBottom );

                if ( mpData->pBorders[i].nStyle & RULER_BORDER_VARIABLE )
                {
                    if ( n2-n1 > RULER_VAR_SIZE+4 )
                    {
                        nTemp1 = n1 + (((n2-n1+1)-RULER_VAR_SIZE) / 2);
                        nTemp2 = nVirTop + (((nVirBottom-nVirTop+1)-RULER_VAR_SIZE) / 2);
                        long nTemp3 = nTemp1+RULER_VAR_SIZE-1;
                        long nTemp4 = nTemp2+RULER_VAR_SIZE-1;
                        long nTempY = nTemp2;

                        maVirDev.SetLineColor( rStyleSettings.GetLightColor() );
                        while ( nTempY <= nTemp4 )
                        {
                            ImplVDrawLine( nTemp1, nTempY, nTemp3, nTempY );
                            nTempY += 2;
                        }

                        nTempY = nTemp2+1;
                        maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
                        while ( nTempY <= nTemp4 )
                        {
                            ImplVDrawLine( nTemp1, nTempY, nTemp3, nTempY );
                            nTempY += 2;
                        }
                    }
                }

                if ( mpData->pBorders[i].nStyle & RULER_BORDER_SIZEABLE )
                {
                    if ( n2-n1 > RULER_VAR_SIZE+10 )
                    {
                        maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
                        ImplVDrawLine( n1+4, nVirTop+3, n1+4, nVirBottom-3 );
                        ImplVDrawLine( n2-5, nVirTop+3, n2-5, nVirBottom-3 );
                        maVirDev.SetLineColor( rStyleSettings.GetLightColor() );
                        ImplVDrawLine( n1+5, nVirTop+3, n1+5, nVirBottom-3 );
                        ImplVDrawLine( n2-4, nVirTop+3, n2-4, nVirBottom-3 );
                    }
                }
            }
            else
            {
                n = n1+((n2-n1)/2);
                maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );

                if ( mpData->pBorders[i].nStyle & RULER_BORDER_SNAP )
                    ImplVDrawLine( n, nVirTop, n, nVirBottom );
                else if ( mpData->pBorders[i].nStyle & RULER_BORDER_MARGIN )
                    ImplVDrawLine( n, nVirTop, n, nVirBottom );
                else
                {
                    ImplVDrawLine( n-1, nVirTop, n-1, nVirBottom );
                    ImplVDrawLine( n+1, nVirTop, n+1, nVirBottom );
                    maVirDev.SetLineColor();
                    maVirDev.SetFillColor( rStyleSettings.GetWindowColor() );
                    ImplVDrawRect( n, nVirTop, n, nVirBottom );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawIndent( const Polygon& rPoly, sal_uInt16 nStyle )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( nStyle & RULER_STYLE_INVISIBLE )
        return;

    maVirDev.SetLineColor( rStyleSettings.GetDarkShadowColor() );
    maVirDev.SetFillColor( rStyleSettings.GetWorkspaceColor() );
    maVirDev.DrawPolygon( rPoly );
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawIndents( long nMin, long nMax, long nVirTop, long nVirBottom )
{
    sal_uInt16  j;
    long    n;
    long    nIndentHeight = (mnVirHeight/2) - 1;
    long    nIndentWidth2 = nIndentHeight-3;
    Polygon aPoly( 5 );

    for ( j = 0; j < mpData->nIndents; j++ )
    {
        if ( mpData->pIndents[j].nStyle & RULER_STYLE_INVISIBLE )
            continue;

        sal_uInt16  nStyle = mpData->pIndents[j].nStyle;
        sal_uInt16  nIndentStyle = nStyle & RULER_INDENT_STYLE;

        n = mpData->pIndents[j].nPos+mpData->nNullVirOff;

        if ( (n >= nMin) && (n <= nMax) )
        {
            if(nIndentStyle == RULER_INDENT_BORDER)
            {
                const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
                maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
                ImplVDrawLine( n, nVirTop+1, n, nVirBottom-1 );
            }
            else if ( nIndentStyle == RULER_INDENT_BOTTOM )
            {
                aPoly.SetPoint( Point( n+0, nVirBottom-nIndentHeight ), 0 );
                aPoly.SetPoint( Point( n-nIndentWidth2, nVirBottom-3 ), 1 );
                aPoly.SetPoint( Point( n-nIndentWidth2, nVirBottom ), 2 );
                aPoly.SetPoint( Point( n+nIndentWidth2, nVirBottom ), 3 );
                aPoly.SetPoint( Point( n+nIndentWidth2, nVirBottom-3 ), 4 );
            }
            else
            {
                aPoly.SetPoint( Point( n+0, nVirTop+nIndentHeight ), 0 );
                aPoly.SetPoint( Point( n-nIndentWidth2, nVirTop+3 ), 1 );
                aPoly.SetPoint( Point( n-nIndentWidth2, nVirTop ), 2 );
                aPoly.SetPoint( Point( n+nIndentWidth2, nVirTop ), 3 );
                aPoly.SetPoint( Point( n+nIndentWidth2, nVirTop+3 ), 4 );
            }

            if(0 == (mnWinStyle & WB_HORZ))
            {
                Point aTmp;
                for(sal_uInt16 i = 0; i < 5; i++)
                {
                    aTmp = aPoly[i];
                    Point aSet(nVirBottom - aTmp.Y(), aTmp.X());
                    aPoly[i] = aSet;
                }
            }
            if(RULER_INDENT_BORDER != nIndentStyle)
                ImplDrawIndent( aPoly, nStyle );
        }
    }
}

// -----------------------------------------------------------------------

static void ImplCenterTabPos( Point& rPos, sal_uInt16 nTabStyle )
{
    sal_Bool bRTL  = 0 != (nTabStyle & RULER_TAB_RTL);
    nTabStyle &= RULER_TAB_STYLE;
    rPos.Y() += RULER_TAB_HEIGHT/2;
    if ( (!bRTL && nTabStyle == RULER_TAB_LEFT) ||( bRTL && nTabStyle == RULER_TAB_RIGHT))
        rPos.X() -= RULER_TAB_WIDTH/2;
    else if ( (!bRTL && nTabStyle == RULER_TAB_RIGHT) ||( bRTL && nTabStyle == RULER_TAB_LEFT))
        rPos.X() += RULER_TAB_WIDTH/2;
}

// -----------------------------------------------------------------------
static void lcl_RotateRect_Impl(Rectangle& rRect, const long nReference, sal_Bool bRightAligned)
{
    if(!rRect.IsEmpty())
    {
        Rectangle aTmp(rRect);
        rRect.Top() = aTmp.Left();
        rRect.Bottom() = aTmp.Right();
        rRect.Left() = aTmp.Top();
        rRect.Right() = aTmp.Bottom();
        if(bRightAligned)
        {
            long nRef = 2 * nReference;
            rRect.Left() = nRef - rRect.Left();
            rRect.Right() = nRef - rRect.Right();
        }
    }
}
// -----------------------------------------------------------------------

static void ImplDrawRulerTab( OutputDevice* pDevice,
                             const Point& rPos, sal_uInt16 nStyle, WinBits nWinBits )
{
    if ( nStyle & RULER_STYLE_INVISIBLE )
        return;

    sal_uInt16 nTabStyle = nStyle & RULER_TAB_STYLE;
    sal_Bool bRTL = 0 != (nStyle & RULER_TAB_RTL);
    Rectangle aRect1, aRect2, aRect3;
    aRect3.SetEmpty();
    if ( nTabStyle == RULER_TAB_DEFAULT )
    {
        aRect1.Left() =     rPos.X() - RULER_TAB_DWIDTH2 + 1                ;
        aRect1.Top() =      rPos.Y() - RULER_TAB_DHEIGHT2 + 1               ;
        aRect1.Right() =    rPos.X() - RULER_TAB_DWIDTH2 + RULER_TAB_DWIDTH ;
        aRect1.Bottom() =   rPos.Y();
        aRect2.Left() =     rPos.X() - RULER_TAB_DWIDTH2 + RULER_TAB_DWIDTH3;
        aRect2.Top() =      rPos.Y() - RULER_TAB_DHEIGHT + 1;
        aRect2.Right() =    rPos.X() - RULER_TAB_DWIDTH2 + RULER_TAB_DWIDTH3 + RULER_TAB_DWIDTH4 - 1;
        aRect2.Bottom() =   rPos.Y();

    }
    else if ( (!bRTL && nTabStyle == RULER_TAB_LEFT) ||( bRTL && nTabStyle == RULER_TAB_RIGHT))
    {
        aRect1.Left() =     rPos.X();
        aRect1.Top() =      rPos.Y() - RULER_TAB_HEIGHT2 + 1;
        aRect1.Right() =    rPos.X() + RULER_TAB_WIDTH - 1;
        aRect1.Bottom() =   rPos.Y();
        aRect2.Left() =     rPos.X();
        aRect2.Top() =      rPos.Y() - RULER_TAB_HEIGHT + 1;
        aRect2.Right() =    rPos.X() + RULER_TAB_WIDTH2 - 1;
        aRect2.Bottom() =   rPos.Y();
    }
    else if ( (!bRTL && nTabStyle == RULER_TAB_RIGHT) ||( bRTL && nTabStyle == RULER_TAB_LEFT))
    {
        aRect1.Left() =     rPos.X() - RULER_TAB_WIDTH + 1;
        aRect1.Top() =      rPos.Y() - RULER_TAB_HEIGHT2 + 1;
        aRect1.Right() =    rPos.X();
        aRect1.Bottom() =   rPos.Y();
        aRect2.Left() =     rPos.X() - RULER_TAB_WIDTH2 + 1;
        aRect2.Top() =      rPos.Y() - RULER_TAB_HEIGHT + 1;
        aRect2.Right() =    rPos.X();
        aRect2.Bottom() =   rPos.Y();
    }
    else
    {
        aRect1.Left() =     rPos.X() - RULER_TAB_CWIDTH2 + 1;
        aRect1.Top() =      rPos.Y() - RULER_TAB_HEIGHT2 + 1;
        aRect1.Right() =    rPos.X() - RULER_TAB_CWIDTH2 + RULER_TAB_CWIDTH;
        aRect1.Bottom() =   rPos.Y();
        aRect2.Left() =     rPos.X() - RULER_TAB_CWIDTH2 + RULER_TAB_CWIDTH3;
        aRect2.Top() =      rPos.Y() - RULER_TAB_HEIGHT + 1;
        aRect2.Right() =    rPos.X() - RULER_TAB_CWIDTH2 + RULER_TAB_CWIDTH3 + RULER_TAB_CWIDTH4 - 1;
        aRect2.Bottom() =   rPos.Y();

        if ( nTabStyle == RULER_TAB_DECIMAL )
        {
            aRect3.Left() = rPos.X() - RULER_TAB_CWIDTH2 + RULER_TAB_CWIDTH - 1;
            aRect3.Top()  = rPos.Y() - RULER_TAB_HEIGHT + 1 + 1;
            aRect3.Right() = rPos.X() - RULER_TAB_CWIDTH2 + RULER_TAB_CWIDTH;
            aRect3.Bottom()= rPos.Y() - RULER_TAB_HEIGHT + 1 + 2 ;
        }
    }
    if( 0 == (nWinBits&WB_HORZ) )
    {
        sal_Bool bRightAligned = 0 != (nWinBits&WB_RIGHT_ALIGNED);
        lcl_RotateRect_Impl(aRect1, rPos.Y(), bRightAligned);
        lcl_RotateRect_Impl(aRect2, rPos.Y(), bRightAligned);
        lcl_RotateRect_Impl(aRect3, rPos.Y(), bRightAligned);
    }
    pDevice->DrawRect( aRect1 );
    pDevice->DrawRect( aRect2 );
    if(!aRect2.IsEmpty())
        pDevice->DrawRect( aRect3 );

}

// -----------------------------------------------------------------------

void Ruler::ImplDrawTab( OutputDevice* pDevice, const Point& rPos, sal_uInt16 nStyle )
{
    if ( nStyle & RULER_STYLE_INVISIBLE )
        return;

    pDevice->SetLineColor();
    if ( nStyle & RULER_STYLE_DONTKNOW )
        pDevice->SetFillColor( GetSettings().GetStyleSettings().GetFaceColor() );
    else
        pDevice->SetFillColor( GetSettings().GetStyleSettings().GetDarkShadowColor() );

    if(mpData->bTextRTL)
        nStyle |= RULER_TAB_RTL;
    ImplDrawRulerTab( pDevice, rPos, nStyle, GetStyle());
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawTabs( long nMin, long nMax, long nVirTop, long nVirBottom )
{
    for ( sal_uInt16 i = 0; i < mpData->nTabs; i++ )
    {
        if ( mpData->pTabs[i].nStyle & RULER_STYLE_INVISIBLE )
            continue;

        long n;
            n = mpData->pTabs[i].nPos;
        n += +mpData->nNullVirOff;
        long nTopBottom = GetStyle() & WB_RIGHT_ALIGNED ? nVirTop : nVirBottom;
        if ( (n >= nMin) && (n <= nMax) )
            ImplDrawTab( &maVirDev, Point( n, nTopBottom ), mpData->pTabs[i].nStyle );
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplInitSettings( sal_Bool bFont,
                              sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetToolFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bForeground || bFont )
    {
        Color aColor;
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        else
            aColor = rStyleSettings.GetDarkShadowColor();
        SetTextColor( aColor );
        SetTextFillColor();
    }

    if ( bBackground )
    {
        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else
            aColor = rStyleSettings.GetWorkspaceColor();
        SetBackground( aColor );
    }

    maVirDev.SetSettings( GetSettings() );
    maVirDev.SetBackground( GetBackground() );
    Font aFont = GetFont();
    if ( mnWinStyle & WB_VERT )
        aFont.SetOrientation( 900 );
    maVirDev.SetFont( aFont );
    maVirDev.SetTextColor( GetTextColor() );
    maVirDev.SetTextFillColor( GetTextFillColor() );
}

// -----------------------------------------------------------------------

void Ruler::ImplCalc()
{
    // Offset berechnen
    mpData->nRulVirOff = mnWinOff + mpData->nPageOff;
    if ( mpData->nRulVirOff > mnVirOff )
        mpData->nRulVirOff -= mnVirOff;
    else
        mpData->nRulVirOff = 0;
    long nRulWinOff = mpData->nRulVirOff+mnVirOff;

    // Nicht sichtbaren Bereich der Page berechnen
    long nNotVisPageWidth;
    if ( mpData->nPageOff < 0 )
    {
        nNotVisPageWidth = -(mpData->nPageOff);
        if ( nRulWinOff < mnWinOff )
            nNotVisPageWidth -= mnWinOff-nRulWinOff;
    }
    else
        nNotVisPageWidth = 0;

    // Breite berechnen
    if ( mnWinStyle & WB_HORZ )
    {
        if ( mbAutoWinWidth )
            mnWinWidth = mnWidth - mnVirOff;
        if ( mpData->bAutoPageWidth )
            mpData->nPageWidth = mnWinWidth;
        mpData->nRulWidth = std::min( mnWinWidth, mpData->nPageWidth-nNotVisPageWidth );
        if ( nRulWinOff+mpData->nRulWidth > mnWidth )
            mpData->nRulWidth = mnWidth-nRulWinOff;
    }
    else
    {
        if ( mbAutoWinWidth )
            mnWinWidth = mnHeight - mnVirOff;
        if ( mpData->bAutoPageWidth )
            mpData->nPageWidth = mnWinWidth;
        mpData->nRulWidth = std::min( mnWinWidth, mpData->nPageWidth-nNotVisPageWidth );
        if ( nRulWinOff+mpData->nRulWidth > mnHeight )
            mpData->nRulWidth = mnHeight-nRulWinOff;
    }

    mbCalc = sal_False;
}

// -----------------------------------------------------------------------

void Ruler::ImplFormat()
{
    // Wenn schon formatiert ist, brauchen wir es nicht nochmal
    if ( !mbFormat )
        return;

    // Wenn Fenster noch keine Groesse hat, brauchen wir noch nichts machen
    if ( !mnVirWidth )
        return;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    long    nP1;            // Pixel-Position von Page1
    long    nP2;            // Pixel-Position von Page2
    long    nM1;            // Pixel-Position von Margin1
    long    nM2;            // Pixel-Position von Margin2
    long    nVirTop;        // Obere/Linke-Kante bei Ausgabe
    long    nVirBottom;     // Untere/Rechte-Kante bei Ausgabe
    long    nVirLeft;       // Linke/Obere-Kante bei Ausgabe
    long    nVirRight;      // Rechte/Untere-Kante bei Ausgabe
    long    nNullVirOff;    // Fuer schnellere Berechnung

    // Werte berechnen
    if ( mbCalc )
        ImplCalc();
    mpData->nNullVirOff = mnWinOff+mpData->nPageOff+mpData->nNullOff-mnVirOff;

    nNullVirOff = mpData->nNullVirOff;
    nVirLeft    = mpData->nRulVirOff;
    nVirRight   = nVirLeft + mpData->nRulWidth - 1;
    nVirTop     = 0;
    nVirBottom  = mnVirHeight - 1;

    if ( !IsReallyVisible() )
        return;

    Size    aVirDevSize;

    // initialize VirtualDevice
    if ( mnWinStyle & WB_HORZ )
    {
        aVirDevSize.Width() = mnVirWidth;
        aVirDevSize.Height() = mnVirHeight;
    }
    else
    {
        aVirDevSize.Height() = mnVirWidth;
        aVirDevSize.Width() = mnVirHeight;
    }
    if ( aVirDevSize != maVirDev.GetOutputSizePixel() )
        maVirDev.SetOutputSizePixel( aVirDevSize, sal_True );
    else
        maVirDev.Erase();

    // Raender berechnen
    if ( !(mpData->nMargin1Style & RULER_STYLE_INVISIBLE) )
    {
        nM1 = mpData->nMargin1+nNullVirOff;
        if ( mpData->bAutoPageWidth )
        {
            nP1 = nVirLeft;
            if ( nM1 < nVirLeft )
                nP1--;
        }
        else
            nP1 = nNullVirOff-mpData->nNullOff;
    }
    else
    {
        nM1 = nVirLeft-1;
        nP1 = nM1;
    }
    if ( !(mpData->nMargin2Style & RULER_STYLE_INVISIBLE) )
    {
        nM2 = mpData->nMargin2+nNullVirOff;
        if ( mpData->bAutoPageWidth )
        {
            nP2 = nVirRight;
            if ( nM2 > nVirRight )
                nP2++;
        }
        else
            nP2 = nNullVirOff-mpData->nNullOff+mpData->nPageWidth;
        if ( nM2 > nP2 )
            nM2 = nP2;
    }
    else
    {
        nM2 = nVirRight+1;
        nP2 = nM2;
    }

    // Obere/untere Kante ausgeben
    maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
    ImplVDrawLine( nVirLeft, nVirTop+1, nM1, nVirTop+1 ); //top left line
    ImplVDrawLine( nM2, nVirTop+1, nP2 -1, nVirTop+1 );      //top right line

    // Jetzt wird zwischen dem Schatten ausgegeben
    nVirTop++;
    nVirBottom--;

    // Margin1, Margin2 und Zwischenraum ausgeben
    maVirDev.SetLineColor();
    maVirDev.SetFillColor( rStyleSettings.GetWorkspaceColor() );
    if ( nM1 > nVirLeft )
        ImplVDrawRect( nP1, nVirTop+1, nM1, nVirBottom ); //left gray rectangle
    if ( nM2 < nP2 )
        ImplVDrawRect( nM2, nVirTop+1, nP2, nVirBottom ); //right gray rectangle
    if ( nM2-nM1 > 0 )
    {
        maVirDev.SetFillColor( rStyleSettings.GetWindowColor() );
        ImplVDrawRect( nM1+1, nVirTop, nM2-1, nVirBottom ); //center rectangle
    }
    maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
    if ( nM1 > nVirLeft )
    {
        ImplVDrawLine( nM1, nVirTop+1, nM1, nVirBottom );//right line of the left rectangle
        ImplVDrawLine( nP1, nVirBottom, nM1, nVirBottom );//bottom line of the left rectangle
        if ( nP1 >= nVirLeft )
        {
            ImplVDrawLine( nP1, nVirTop+1, nP1, nVirBottom );//left line of the left rectangle
            ImplVDrawLine( nP1, nVirBottom, nP1+1, nVirBottom );//?
        }
    }
    if ( nM2 < nP2 )
    {
        ImplVDrawLine( nM2, nVirBottom, nP2-1, nVirBottom );//bottom line of the right rectangle
        ImplVDrawLine( nM2, nVirTop+1, nM2, nVirBottom );//left line of the right rectangle
        if ( nP2 <= nVirRight+1 )
            ImplVDrawLine( nP2-1, nVirTop+1, nP2-1, nVirBottom );//right line of the right rectangle
    }

    // Lineal-Beschriftung (nur wenn keine Bemassungspfeile)
    long nMin = nVirLeft;
    long nMax = nP2;
    long nCenter = nVirTop + ((nVirBottom - nVirTop) / 2);
    long nStart = 0;

    if (mpData->bTextRTL)
        nStart = mpData->nRightFrameMargin + nNullVirOff;
    else
        nStart = mpData->nLeftFrameMargin + nNullVirOff;

    // Nicht Schatten uebermalen
    if ( nP1 > nVirLeft )
        nMin++;
    if ( nP2 < nVirRight )
        nMax--;

    // Draw captions
    ImplDrawTicks( nMin, nMax, nStart, nCenter );

    // Draw borders
    if ( mpData->pBorders )
        ImplDrawBorders( nVirLeft, nP2, nVirTop, nVirBottom );

    // Draw indents
    if ( mpData->pIndents )
        ImplDrawIndents( nVirLeft, nP2, nVirTop-1, nVirBottom+1 );

    // Tabs
    if ( mpData->pTabs )
    {
        ImplDrawTabs( nVirLeft, nP2, nVirTop-1, nVirBottom+1 );
    }

    // Wir haben formatiert
    mbFormat = sal_False;
}

// -----------------------------------------------------------------------

void Ruler::ImplInitExtraField( sal_Bool bUpdate )
{
    Size aWinSize = GetOutputSizePixel();

    // Extra-Field beruecksichtigen
    if ( mnWinStyle & WB_EXTRAFIELD )
    {
        maExtraRect.Left()   = RULER_OFF;
        maExtraRect.Top()    = RULER_OFF;
        maExtraRect.Right()  = RULER_OFF+mnVirHeight-1;
        maExtraRect.Bottom() = RULER_OFF+mnVirHeight-1;
        if(mpData->bTextRTL)
        {
            if(mnWinStyle & WB_HORZ)
                maExtraRect.Move(aWinSize.Width() - maExtraRect.GetWidth() - maExtraRect.Left(), 0);
            else
                maExtraRect.Move(0, aWinSize.Height() - maExtraRect.GetHeight() - maExtraRect.Top());
            mnVirOff = 0;
        }
        else
            mnVirOff = maExtraRect.Right()+1;

    }
    else
    {
        maExtraRect.SetEmpty();
        mnVirOff = 0;
    }

    // mnVirWidth depends on mnVirOff
    if ( (mnVirWidth > RULER_MIN_SIZE) ||
     ((aWinSize.Width() > RULER_MIN_SIZE) && (aWinSize.Height() > RULER_MIN_SIZE)) )
    {
        if ( mnWinStyle & WB_HORZ )
            mnVirWidth = aWinSize.Width()-mnVirOff;
        else
            mnVirWidth = aWinSize.Height()-mnVirOff;

        if ( mnVirWidth < RULER_MIN_SIZE )
            mnVirWidth = 0;
    }

    if ( bUpdate )
    {
        mbCalc      = sal_True;
        mbFormat    = sal_True;
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplDraw()
{
    if ( mbFormat )
        ImplFormat();

    if ( IsReallyVisible() )
    {
        // Lineal ueber das VirtualDevice ausgeben
        Point   aOffPos;
        Size    aVirDevSize = maVirDev.GetOutputSizePixel();
//        Size    aVirDevSize2 = maVirDev.GetOutputSizePixel();
        if ( mnWinStyle & WB_HORZ )
        {
            aOffPos.X() = mnVirOff;
            if(mpData->bTextRTL)
                aVirDevSize.Width() -= maExtraRect.GetWidth();

//  else
//      aVirDevSize.Width() -= mnVirOff;
            aOffPos.Y() = RULER_OFF;
        }
        else
        {
            aOffPos.X() = RULER_OFF;
            aOffPos.Y() = mnVirOff;
//  else
//      aVirDevSize.Height() -= mnVirOff;
        }
        DrawOutDev( aOffPos, aVirDevSize, Point(), aVirDevSize, maVirDev );

        // Positionslinien neu malen
        ImplInvertLines( sal_True );
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawExtra( sal_Bool bPaint )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    Rectangle   aRect = maExtraRect;
    sal_Bool        bEraseRect = sal_False;

    aRect.Left()    += 2;
    aRect.Top()     += 2;
    aRect.Right()   -= 2;
    aRect.Bottom()  -= 2;

    if ( !bPaint && !(mnExtraStyle & RULER_STYLE_HIGHLIGHT) )
    {
        SetFillColor( rStyleSettings.GetWorkspaceColor() );
        bEraseRect = sal_True;
    }
    else
    {
        if ( mnExtraStyle & RULER_STYLE_HIGHLIGHT )
        {
            SetFillColor( rStyleSettings.GetCheckedColor() );
            bEraseRect = sal_True;
        }
    }

    if ( bEraseRect )
    {
        SetLineColor();
        DrawRect( aRect );
    }

    // Inhalt ausgeben
    if ( meExtraType == RULER_EXTRA_NULLOFFSET )
    {
        SetLineColor( rStyleSettings.GetButtonTextColor() );
        DrawLine( Point( aRect.Left()+1, aRect.Top()+4 ),
                  Point( aRect.Right()-1, aRect.Top()+4 ) );
        DrawLine( Point( aRect.Left()+4, aRect.Top()+1 ),
                  Point( aRect.Left()+4, aRect.Bottom()-1 ) );
    }
    else if ( meExtraType == RULER_EXTRA_TAB )
    {
        sal_uInt16 nTabStyle = mnExtraStyle & RULER_TAB_STYLE;
        if(mpData->bTextRTL)
            nTabStyle |= RULER_TAB_RTL;
        Point aCenter = aRect.Center();
        Point aDraw(aCenter);
        ImplCenterTabPos( aDraw, nTabStyle );
        WinBits nWinBits = GetStyle();
        if(0 == (nWinBits&WB_HORZ) )
        {
            if(0 != (nWinBits&WB_RIGHT_ALIGNED))
                aDraw.Y() = 2 * aCenter.Y() - aDraw.Y();
            if(mpData->bTextRTL)
            {
                long nTemp = aDraw.X();
                aDraw.X() = aDraw.Y();
                aDraw.Y() = nTemp;
            }
        }
        ImplDrawTab( this, aDraw, nTabStyle );
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplUpdate( sal_Bool bMustCalc )
{
    // Hier schon Linien loeschen, damit Sie vor dem Neuberechnen schon
    // geloscht sind, da danach die alten Positionen nicht mehr bestimmt
    // werden koennen
    if ( !mbFormat )
        ImplInvertLines();

    // Flags setzen
    if ( bMustCalc )
        mbCalc = sal_True;
    mbFormat = sal_True;

    // Wenn wir am Draggen sind, wird nach dem Drag-Handler automatisch
    // das Lineal neu upgedatet
    if ( mbDrag )
        return;

    // Gegebenenfalls Update ausloesen
    if ( IsReallyVisible() && IsUpdateMode() )
    {
        mnUpdateFlags |= RULER_UPDATE_DRAW;
        if ( !mnUpdateEvtId )
            mnUpdateEvtId = Application::PostUserEvent( LINK( this, Ruler, ImplUpdateHdl ), NULL );
    }
}

// -----------------------------------------------------------------------

sal_Bool Ruler::ImplHitTest( const Point& rPos, ImplRulerHitTest* pHitTest,
                         sal_Bool bRequireStyle, sal_uInt16 nRequiredStyle ) const
{
    sal_uInt16      i;
    sal_uInt16      nStyle;
    long        nHitBottom;
    long        nX;
    long        nY;
    long        n1;
    long        n2;

    if ( !mbActive )
        return sal_False;

    // Position ermitteln
    sal_Bool bIsHori = 0 != (mnWinStyle & WB_HORZ);
    if ( bIsHori )
    {
        nX = rPos.X();
        nY = rPos.Y();
    }
    else
    {
        nX = rPos.Y();
        nY = rPos.X();
    }
    nHitBottom = mnVirHeight+(RULER_OFF*2);

    // #i32608#
    pHitTest->nAryPos = 0;
    pHitTest->mnDragSize = 0;
    pHitTest->bSize = sal_False;
    pHitTest->bSizeBar = sal_False;

    // Damit ueberstehende Tabs und Einzuege mit beruecksichtigt werden
    long nXExtraOff;
    if ( mpData->pTabs || mpData->pIndents )
        nXExtraOff = (mnVirHeight/2) - 4;
    else
        nXExtraOff = 0;

    // Test auf ausserhalb
    nX -= mnVirOff;
    long nXTemp = nX;
    if ( (nX < mpData->nRulVirOff-nXExtraOff) || (nX > mpData->nRulVirOff+mpData->nRulWidth+nXExtraOff) ||
         (nY < 0) || (nY > nHitBottom) )
    {
        pHitTest->nPos = 0;
        pHitTest->eType = RULER_TYPE_OUTSIDE;
        return sal_False;
    }

    nX -= mpData->nNullVirOff;
    pHitTest->nPos  = nX;
    pHitTest->eType = RULER_TYPE_DONTKNOW;

    // Zuerst die Tabs testen
    Rectangle aRect;
    if ( mpData->pTabs )
    {
        aRect.Bottom()  = nHitBottom;
        aRect.Top()     = aRect.Bottom()-RULER_TAB_HEIGHT-RULER_OFF;

        for ( i = mpData->nTabs; i; i-- )
        {
            nStyle = mpData->pTabs[i-1].nStyle;
            if ( !(nStyle & RULER_STYLE_INVISIBLE) )
            {
                nStyle &= RULER_TAB_STYLE;

                // Default-Tabs werden nur angezeigt
                if ( nStyle != RULER_TAB_DEFAULT )
                {
                    n1 = mpData->pTabs[i-1].nPos;

                    if ( nStyle == RULER_TAB_LEFT )
                    {
                        aRect.Left()    = n1;
                        aRect.Right()   = n1+RULER_TAB_WIDTH-1;
                    }
                    else if ( nStyle == RULER_TAB_RIGHT )
                    {
                        aRect.Right()   = n1;
                        aRect.Left()    = n1-RULER_TAB_WIDTH-1;
                    }
                    else
                    {
                        aRect.Left()    = n1-RULER_TAB_CWIDTH2+1;
                        aRect.Right()   = n1-RULER_TAB_CWIDTH2+RULER_TAB_CWIDTH;
                    }

                    if ( aRect.IsInside( Point( nX, nY ) ) )
                    {
                        pHitTest->eType     = RULER_TYPE_TAB;
                        pHitTest->nAryPos   = i-1;
                        return sal_True;
                    }
                }
            }
        }
    }

    // Dann die Einzuege
    if ( mpData->pIndents )
    {
        long nIndentHeight = (mnVirHeight/2) - 1;
        long nIndentWidth2 = nIndentHeight-3;

        for ( i = mpData->nIndents; i; i-- )
        {
            nStyle = mpData->pIndents[i-1].nStyle;
            if ( (! bRequireStyle || nStyle == nRequiredStyle) &&
                 !(nStyle & RULER_STYLE_INVISIBLE) )
            {
                nStyle &= RULER_INDENT_STYLE;
                n1 = mpData->pIndents[i-1].nPos;

                if ( (nStyle == RULER_INDENT_BOTTOM) ^ (!bIsHori) )
                {
                    aRect.Left()    = n1-nIndentWidth2;
                    aRect.Right()   = n1+nIndentWidth2;
                    aRect.Top()     = nHitBottom-nIndentHeight-RULER_OFF+1;
                    aRect.Bottom()  = nHitBottom;
                }
                else
                {
                    aRect.Left()    = n1-nIndentWidth2;
                    aRect.Right()   = n1+nIndentWidth2;
                    aRect.Top()     = 0;
                    aRect.Bottom()  = nIndentHeight+RULER_OFF-1;
                }

                if ( aRect.IsInside( Point( nX, nY ) ) )
                {
                    pHitTest->eType     = RULER_TYPE_INDENT;
                    pHitTest->nAryPos   = i-1;
                    return sal_True;
                }
            }
        }
    }

    // Jetzt zaehlt nichts mehr, was links oder rechts uebersteht
    if ( (nXTemp < mpData->nRulVirOff) || (nXTemp > mpData->nRulVirOff+mpData->nRulWidth) )
    {
        pHitTest->nPos = 0;
        pHitTest->eType = RULER_TYPE_OUTSIDE;
        return sal_False;
    }

    // Danach die Spalten testen
    int nBorderTolerance = 1;
    if(pHitTest->bExpandTest)
    {
        nBorderTolerance++;
    }

    for ( i = mpData->nBorders; i; i-- )
    {
        n1 = mpData->pBorders[i-1].nPos;
        n2 = n1 + mpData->pBorders[i-1].nWidth;

        // Spalten werden mit mindestens 3 Pixel breite gezeichnet
        if ( !mpData->pBorders[i-1].nWidth )
        {
             n1 -= nBorderTolerance;
             n2 += nBorderTolerance;

        }

        if ( (nX >= n1) && (nX <= n2) )
        {
            nStyle = mpData->pBorders[i-1].nStyle;
            if ( !(nStyle & RULER_STYLE_INVISIBLE) )
            {
                pHitTest->eType     = RULER_TYPE_BORDER;
                pHitTest->nAryPos   = i-1;

                if ( !(nStyle & RULER_BORDER_SIZEABLE) )
                {
                    if ( nStyle & RULER_BORDER_MOVEABLE )
                    {
                        pHitTest->bSizeBar = sal_True;
                        pHitTest->mnDragSize = RULER_DRAGSIZE_MOVE;
                    }
                }
                else
                {
                    long nMOff = RULER_MOUSE_BORDERWIDTH;
                    while ( nMOff*2 >= (n2-n1-RULER_MOUSE_BORDERMOVE) )
                    {
                        if ( nMOff < 2 )
                        {
                            nMOff = 0;
                            break;
                        }
                        else
                            nMOff--;
                    }

                    if ( nX <= n1+nMOff )
                    {
                        pHitTest->bSize = sal_True;
                        pHitTest->mnDragSize = RULER_DRAGSIZE_1;
                    }
                    else if ( nX >= n2-nMOff )
                    {
                        pHitTest->bSize = sal_True;
                        pHitTest->mnDragSize = RULER_DRAGSIZE_2;
                    }
                    else
                    {
                        if ( nStyle & RULER_BORDER_MOVEABLE )
                        {
                            pHitTest->bSizeBar = sal_True;
                            pHitTest->mnDragSize = RULER_DRAGSIZE_MOVE;
                        }
                    }
                }

                return sal_True;
            }
        }
    }

    // Und zum Schluss die Raender
    int nMarginTolerance = pHitTest->bExpandTest ? nBorderTolerance : RULER_MOUSE_MARGINWIDTH;

    if ( (mpData->nMargin1Style & (RULER_MARGIN_SIZEABLE | RULER_STYLE_INVISIBLE)) == RULER_MARGIN_SIZEABLE )
    {
        n1 = mpData->nMargin1;
        if ( (nX >= n1 - nMarginTolerance) && (nX <= n1 + nMarginTolerance) )
        {
            pHitTest->eType = RULER_TYPE_MARGIN1;
            pHitTest->bSize = sal_True;
            return sal_True;
        }
    }
    if ( (mpData->nMargin2Style & (RULER_MARGIN_SIZEABLE | RULER_STYLE_INVISIBLE)) == RULER_MARGIN_SIZEABLE )
    {
        n1 = mpData->nMargin2;
        if ( (nX >= n1 - nMarginTolerance) && (nX <= n1 + nMarginTolerance) )
        {
            pHitTest->eType = RULER_TYPE_MARGIN2;
            pHitTest->bSize = sal_True;
            return sal_True;
        }
    }

    // Jetzt nocheinmal die Tabs testen, nur mit etwas mehr spielraum
    if ( mpData->pTabs )
    {
        aRect.Top()     = RULER_OFF;
        aRect.Bottom()  = nHitBottom;

        for ( i = mpData->nTabs; i; i-- )
        {
            nStyle = mpData->pTabs[i-1].nStyle;
            if ( !(nStyle & RULER_STYLE_INVISIBLE) )
            {
                nStyle &= RULER_TAB_STYLE;

                // Default-Tabs werden nur angezeigt
                if ( nStyle != RULER_TAB_DEFAULT )
                {
                    n1 = mpData->pTabs[i-1].nPos;

                    if ( nStyle == RULER_TAB_LEFT )
                    {
                        aRect.Left()    = n1;
                        aRect.Right()   = n1+RULER_TAB_WIDTH-1;
                    }
                    else if ( nStyle == RULER_TAB_RIGHT )
                    {
                        aRect.Right()   = n1;
                        aRect.Left()    = n1-RULER_TAB_WIDTH-1;
                    }
                    else
                    {
                        aRect.Left()    = n1-RULER_TAB_CWIDTH2+1;
                        aRect.Right()   = n1-RULER_TAB_CWIDTH2+RULER_TAB_CWIDTH;
                    }

                    aRect.Left()--;
                    aRect.Right()++;

                    if ( aRect.IsInside( Point( nX, nY ) ) )
                    {
                        pHitTest->eType     = RULER_TYPE_TAB;
                        pHitTest->nAryPos   = i-1;
                        return sal_True;
                    }
                }
            }
        }
    }

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Ruler::ImplDocHitTest( const Point& rPos, RulerType eDragType,
                            ImplRulerHitTest* pHitTest ) const
{
    Point aPos = rPos;
    sal_Bool bRequiredStyle = sal_False;
    sal_uInt16 nRequiredStyle = 0;

    if (eDragType == RULER_TYPE_INDENT)
    {
        bRequiredStyle = sal_True;
        nRequiredStyle = RULER_INDENT_BOTTOM;
    }

    if ( mnWinStyle & WB_HORZ )
        aPos.X() += mnWinOff;
    else
        aPos.Y() += mnWinOff;

    if ( (eDragType == RULER_TYPE_INDENT) || (eDragType == RULER_TYPE_DONTKNOW) )
    {
        if ( mnWinStyle & WB_HORZ )
            aPos.Y() = RULER_OFF+1;
        else
            aPos.X() = RULER_OFF+1;

        // HitTest durchfuehren
        if ( ImplHitTest( aPos, pHitTest, bRequiredStyle, nRequiredStyle ) )
        {
            if ( (pHitTest->eType == eDragType) || (eDragType == RULER_TYPE_DONTKNOW) )
                return sal_True;
        }
    }

    if ( (eDragType == RULER_TYPE_INDENT) || (eDragType == RULER_TYPE_TAB) ||
         (eDragType == RULER_TYPE_DONTKNOW) )
    {
        if ( mnWinStyle & WB_HORZ )
            aPos.Y() = mnHeight-RULER_OFF-1;
        else
            aPos.X() = mnWidth-RULER_OFF-1;

        // HitTest durchfuehren
        if ( ImplHitTest( aPos, pHitTest, bRequiredStyle, nRequiredStyle ) )
        {
            if ( (pHitTest->eType == eDragType) || (eDragType == RULER_TYPE_DONTKNOW) )
                return sal_True;
        }
    }

    if ( (eDragType == RULER_TYPE_MARGIN1) || (eDragType == RULER_TYPE_MARGIN2) ||
         (eDragType == RULER_TYPE_BORDER) || (eDragType == RULER_TYPE_DONTKNOW) )
    {
        if ( mnWinStyle & WB_HORZ )
            aPos.Y() = RULER_OFF + (mnVirHeight/2);
        else
            aPos.X() = RULER_OFF + (mnVirHeight/2);

        // HitTest durchfuehren
        if ( ImplHitTest( aPos, pHitTest ) )
        {
            if ( (pHitTest->eType == eDragType) || (eDragType == RULER_TYPE_DONTKNOW) )
                return sal_True;
        }
    }

    // Auf DontKnow setzen
    pHitTest->eType = RULER_TYPE_DONTKNOW;

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Ruler::ImplStartDrag( ImplRulerHitTest* pHitTest, sal_uInt16 nModifier )
{
    // Wenn eine Spalte angeklick wurde, die weder verschiebar noch
    // in der Groesse aenderbar ist, brauchen wir auch kein Drag ausloesen
    if ( (pHitTest->eType == RULER_TYPE_BORDER) &&
         !pHitTest->bSize && !pHitTest->bSizeBar )
        return sal_False;

    // Dragdaten setzen
    meDragType      = pHitTest->eType;
    mnDragPos       = pHitTest->nPos;
    mnDragAryPos    = pHitTest->nAryPos;
    mnDragSize      = pHitTest->mnDragSize;
    mnDragModifier  = nModifier;
    *mpDragData     = *mpSaveData;
    mpData          = mpDragData;

    // Handler rufen
    if ( StartDrag() )
    {
        // Wenn der Handler das Draggen erlaubt, dann das Draggen
        // initialisieren
        ImplInvertLines();
        mbDrag = sal_True;
        mnStartDragPos = mnDragPos;
        StartTracking();
        return sal_True;
    }
    else
    {
        // Ansonsten muessen wir die Daten zuruecksetzen
        meDragType      = RULER_TYPE_DONTKNOW;
        mnDragPos       = 0;
        mnDragAryPos    = 0;
        mnDragSize      = 0;
        mnDragModifier  = 0;
        mpData          = mpSaveData;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void Ruler::ImplDrag( const Point& rPos )
{
    long  nX;
    long  nY;
    long  nOutHeight;

    if ( mnWinStyle & WB_HORZ )
    {
        nX          = rPos.X();
        nY          = rPos.Y();
        nOutHeight  = mnHeight;
    }
    else
    {
        nX          = rPos.Y();
        nY          = rPos.X();
        nOutHeight  = mnWidth;
    }

    // X berechnen und einpassen
    nX -= mnVirOff;
    if ( nX < mpData->nRulVirOff )
    {
        nX = mpData->nRulVirOff;
        mnDragScroll = RULER_SCROLL_1;
    }
    else if ( nX > mpData->nRulVirOff+mpData->nRulWidth )
    {
        nX = mpData->nRulVirOff+mpData->nRulWidth;
        mnDragScroll = RULER_SCROLL_2;
    }
    nX -= mpData->nNullVirOff;

    // Wenn oberhalb oder links vom Lineal, dann alte Werte
    mbDragDelete = sal_False;
    if ( nY < 0 )
    {
        if ( !mbDragCanceled )
        {
            // Daten wiederherstellen
            mbDragCanceled = sal_True;
            ImplRulerData aTempData;
            aTempData = *mpDragData;
            *mpDragData = *mpSaveData;
            mbCalc = sal_True;
            mbFormat = sal_True;

            // Handler rufen
            mnDragPos = mnStartDragPos;
            Drag();

            // Und neu ausgeben (zeitverzoegert)
            ImplDraw();

            // Daten wieder wie vor dem Cancel herstellen
            *mpDragData = aTempData;
        }
    }
    else
    {
        mbDragCanceled = sal_False;

        // +2, damit nicht so schnell die Tabs geloescht werden
        if ( nY > nOutHeight+2 )
            mbDragDelete = sal_True;

        mnDragPos = nX;

        // Handler rufen
        Drag();

        // Und neu ausgeben
        if ( mbFormat )
            ImplDraw();
    }

    mnDragScroll = 0;
}

// -----------------------------------------------------------------------

void Ruler::ImplEndDrag()
{
    // Werte uebernehmen
    if ( mbDragCanceled )
        *mpDragData = *mpSaveData;
    else
        *mpSaveData = *mpDragData;
    mpData = mpSaveData;
    mbDrag = sal_False;

    // Handler rufen
    EndDrag();

    // Drag-Werte zuruecksetzen
    meDragType      = RULER_TYPE_DONTKNOW;
    mnDragPos       = 0;
    mnDragAryPos    = 0;
    mnDragSize      = 0;
    mbDragCanceled  = sal_False;
    mbDragDelete    = sal_False;
    mnDragModifier  = 0;
    mnDragScroll    = 0;
    mnStartDragPos  = 0;

    // Und neu ausgeben
    ImplDraw();
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(Ruler, ImplUpdateHdl)
{
    mnUpdateEvtId = 0;

    // Feststellen, was upgedatet werden muss
    if ( mnUpdateFlags & RULER_UPDATE_DRAW )
    {
        mnUpdateFlags = 0;
        ImplDraw();
    }
    else if ( mnUpdateFlags & RULER_UPDATE_LINES )
    {
        mnUpdateFlags = 0;
        ImplInvertLines();
    }

    return 0;
}

// -----------------------------------------------------------------------

void Ruler::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && !IsTracking() )
    {
        Point   aMousePos = rMEvt.GetPosPixel();
        sal_uInt16  nMouseClicks = rMEvt.GetClicks();
        sal_uInt16  nMouseModifier = rMEvt.GetModifier();

        // Gegebenenfalls Lineal updaten (damit mit den richtigen Daten
        // gearbeitet wird und die Anzeige auch zur Bearbeitung passt)
        if ( mbFormat )
        {
            ImplDraw();
            mnUpdateFlags &= ~RULER_UPDATE_DRAW;
        }

        if ( maExtraRect.IsInside( aMousePos ) )
        {
            mnExtraClicks = nMouseClicks;
            mnExtraModifier = nMouseModifier;
            ExtraDown();
            mnExtraClicks = 0;
            mnExtraModifier = 0;
        }
        else
        {
            ImplRulerHitTest aHitTest;

            if ( nMouseClicks == 1 )
            {
                if ( ImplHitTest( aMousePos, &aHitTest ) )
                    ImplStartDrag( &aHitTest, nMouseModifier );
                else
                {
                    // Position innerhalb des Lineal-Bereiches
                    if ( aHitTest.eType == RULER_TYPE_DONTKNOW )
                    {
                        mnDragPos = aHitTest.nPos;
                        Click();
                        mnDragPos = 0;

                        // Nocheinmal HitTest durchfuehren, da durch den Click
                        // zum Beispiel ein neuer Tab gesetzt werden konnte
                        if ( ImplHitTest( aMousePos, &aHitTest ) )
                            ImplStartDrag( &aHitTest, nMouseModifier );
                    }
                }
            }
            else
            {
                if ( ImplHitTest( aMousePos, &aHitTest ) )
                {
                    mnDragPos    = aHitTest.nPos;
                    mnDragAryPos = aHitTest.nAryPos;
                }
                meDragType = aHitTest.eType;

                DoubleClick();

                meDragType      = RULER_TYPE_DONTKNOW;
                mnDragPos       = 0;
                mnDragAryPos    = 0;
            }
        }
    }
}

// -----------------------------------------------------------------------

void Ruler::MouseMove( const MouseEvent& rMEvt )
{
    PointerStyle ePtrStyle = POINTER_ARROW;

    // Gegebenenfalls Lineal updaten (damit mit den richtigen Daten
    // gearbeitet wird und die Anzeige auch zur Bearbeitung passt)
    if ( mbFormat )
    {
        ImplDraw();
        mnUpdateFlags &= ~RULER_UPDATE_DRAW;
    }

    ImplRulerHitTest aHitTest;
    if ( ImplHitTest( rMEvt.GetPosPixel(), &aHitTest ) )
    {
        if ( aHitTest.bSize )
        {
            if ( mnWinStyle & WB_HORZ )
                ePtrStyle = POINTER_ESIZE;
            else
                ePtrStyle = POINTER_SSIZE;
        }
        else if ( aHitTest.bSizeBar )
        {
            if ( mnWinStyle & WB_HORZ )
                ePtrStyle = POINTER_HSIZEBAR;
            else
                ePtrStyle = POINTER_VSIZEBAR;
        }
    }

    SetPointer( Pointer( ePtrStyle ) );
}

// -----------------------------------------------------------------------

void Ruler::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        // Bei Abbruch, den alten Status wieder herstellen
        if ( rTEvt.IsTrackingCanceled() )
        {
            mbDragCanceled = sal_True;
            mbFormat       = sal_True;
        }

        ImplEndDrag();
    }
    else
        ImplDrag( rTEvt.GetMouseEvent().GetPosPixel() );
}

// -----------------------------------------------------------------------

void Ruler::Paint( const Rectangle& )
{
    ImplDraw();

    // Extra-Field beruecksichtigen
    if ( mnWinStyle & WB_EXTRAFIELD )
        ImplDrawExtra( sal_True );
}

// -----------------------------------------------------------------------

void Ruler::Resize()
{
    Size aWinSize = GetOutputSizePixel();

    long nNewHeight;
    if ( mnWinStyle & WB_HORZ )
    {
        if ( aWinSize.Height() != mnHeight )
            nNewHeight = aWinSize.Height();
        else
            nNewHeight = 0;
    }
    else
    {
        if ( aWinSize.Width() != mnWidth )
            nNewHeight = aWinSize.Width();
        else
            nNewHeight = 0;
    }

    // Hier schon Linien loeschen
    sal_Bool bVisible = IsReallyVisible();
    if ( bVisible && mpData->nLines )
    {
        ImplInvertLines();
        mnUpdateFlags |= RULER_UPDATE_LINES;
        if ( !mnUpdateEvtId )
            mnUpdateEvtId = Application::PostUserEvent( LINK( this, Ruler, ImplUpdateHdl ), NULL );
    }
    mbFormat = sal_True;

    // Wenn sich die Hoehe bzw. Breite aendert, dann muessen besimmte Werte
    // neu berechnet werden
    //extra field should always be updated
    ImplInitExtraField( mpData->bTextRTL );
    if ( nNewHeight )
    {
        mbCalc = sal_True;
        mnVirHeight = nNewHeight - mnBorderWidth - (RULER_OFF*2);
    }
    else
    {
        if ( mpData->bAutoPageWidth )
            ImplUpdate( sal_True );
        else if ( mbAutoWinWidth )
            mbCalc = sal_True;
    }

    // Gegebenenfalls ein Teil vom Rand loeschen, da 3D-Effekt/Trennlinie am
    // Fensterrand
    if ( bVisible )
    {
        if ( nNewHeight )
            Invalidate();
        else if ( mpData->bAutoPageWidth )
        {
            // Nur bei AutoPageWidth haben wir rechts einen 3D-Effekt,
            // der sich der Fensterbreite anpasst und deshalb neu gezeichnet
            // werden muss
            Rectangle aRect;

            if ( mnWinStyle & WB_HORZ )
            {
                if ( mnWidth < aWinSize.Width() )
                    aRect.Left() = mnWidth-RULER_RESIZE_OFF;
                else
                    aRect.Left() = aWinSize.Width()-RULER_RESIZE_OFF;
                aRect.Right()   = aRect.Left()+RULER_RESIZE_OFF;
                aRect.Top()     = RULER_OFF;
                aRect.Bottom()  = RULER_OFF+mnVirHeight;
            }
            else
            {
                if ( mnHeight < aWinSize.Height() )
                    aRect.Top() = mnHeight-RULER_RESIZE_OFF;
                else
                    aRect.Top() = aWinSize.Height()-RULER_RESIZE_OFF;
                aRect.Bottom() = aRect.Top()+RULER_RESIZE_OFF;
                aRect.Left()    = RULER_OFF;
                aRect.Right()   = RULER_OFF+mnVirHeight;
            }

            Invalidate( aRect );
        }
    }

    // Neue Groesse merken
    mnWidth  = aWinSize.Width();
    mnHeight = aWinSize.Height();
}

// -----------------------------------------------------------------------

void Ruler::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
        ImplFormat();
    else if ( nType == STATE_CHANGE_UPDATEMODE )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            ImplDraw();
    }
    else if ( (nType == STATE_CHANGE_ZOOM) ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( sal_True, sal_False, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( sal_False, sal_True, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( sal_False, sal_False, sal_True );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void Ruler::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_DISPLAY) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        mbFormat = sal_True;
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

long Ruler::StartDrag()
{
    if ( maStartDragHdl.IsSet() )
        return maStartDragHdl.Call( this );
    else
        return sal_False;
}

// -----------------------------------------------------------------------

void Ruler::Drag()
{
    maDragHdl.Call( this );
}

// -----------------------------------------------------------------------

void Ruler::EndDrag()
{
    maEndDragHdl.Call( this );
}

// -----------------------------------------------------------------------

void Ruler::Click()
{
    maClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void Ruler::DoubleClick()
{
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void Ruler::ExtraDown()
{
    maExtraDownHdl.Call( this );
}

// -----------------------------------------------------------------------

void Ruler::Activate()
{
    mbActive = sal_True;

    // Positionslinien wieder anzeigen (erst hinter mbActive=sal_True rufen, da
    // von ImplInvertLines() ausgewertet wird). Das Zeichnen der Linien
    // wird verzoegert, damit im vermutlich noch nicht gepainteten Zustand
    // Linien gezeichnet werden.
    mnUpdateFlags |= RULER_UPDATE_LINES;
    if ( !mnUpdateEvtId )
        mnUpdateEvtId = Application::PostUserEvent( LINK( this, Ruler, ImplUpdateHdl ), NULL );
}

// -----------------------------------------------------------------------

void Ruler::Deactivate()
{
    // Positionslinien loeschen (schon vor mbActive=sal_False rufen, da
    // von ImplInvertLines() ausgewertet wird)
    ImplInvertLines();

    mbActive = sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Ruler::StartDocDrag( const MouseEvent& rMEvt, RulerType eDragType )
{
    if ( !mbDrag )
    {
        Point               aMousePos = rMEvt.GetPosPixel();
        sal_uInt16              nMouseClicks = rMEvt.GetClicks();
        sal_uInt16              nMouseModifier = rMEvt.GetModifier();
        ImplRulerHitTest    aHitTest;
        if(eDragType != RULER_TYPE_DONTKNOW)
            aHitTest.bExpandTest = sal_True;

        // Gegebenenfalls Lineal updaten (damit mit den richtigen Daten
        // gearbeitet wird und die Anzeige auch zur Bearbeitung passt)
        if ( mbFormat )
        {
            ImplDraw();
            mnUpdateFlags &= ~RULER_UPDATE_DRAW;
        }

        if ( nMouseClicks == 1 )
        {
            if ( ImplDocHitTest( aMousePos, eDragType, &aHitTest ) )
            {
                Pointer aPtr;

                if ( aHitTest.bSize )
                {
                    if ( mnWinStyle & WB_HORZ )
                        aPtr = Pointer( POINTER_ESIZE );
                    else
                        aPtr = Pointer( POINTER_SSIZE );
                }
                else if ( aHitTest.bSizeBar )
                {
                    if ( mnWinStyle & WB_HORZ )
                        aPtr = Pointer( POINTER_HSIZEBAR );
                    else
                        aPtr = Pointer( POINTER_VSIZEBAR );
                }
                SetPointer( aPtr );
                return ImplStartDrag( &aHitTest, nMouseModifier );
            }
        }
        else if ( nMouseClicks == 2 )
        {
            if ( ImplDocHitTest( aMousePos, eDragType, &aHitTest ) )
            {
                mnDragPos    = aHitTest.nPos;
                mnDragAryPos = aHitTest.nAryPos;
            }
            eDragType = aHitTest.eType;

            DoubleClick();

            eDragType       = RULER_TYPE_DONTKNOW;
            mnDragPos       = 0;
            mnDragAryPos    = 0;

            return sal_True;
        }
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void Ruler::CancelDrag()
{
    if ( mbDrag )
    {
        ImplDrag( Point( -1, -1 ) );
        ImplEndDrag();
    }
}

// -----------------------------------------------------------------------

RulerType Ruler::GetType( const Point& rPos, sal_uInt16* pAryPos ) const
{
    ImplRulerHitTest aHitTest;

    // Gegebenenfalls Lineal updaten (damit mit den richtigen Daten
    // gearbeitet wird und die Anzeige auch zur Bearbeitung passt)
    if ( IsReallyVisible() && mbFormat )
    {
        ((Ruler*)this)->ImplDraw();
        ((Ruler*)this)->mnUpdateFlags &= ~RULER_UPDATE_DRAW;
    }

    // HitTest durchfuehren
    ImplHitTest( rPos, &aHitTest );

    // Werte zurueckgeben
    if ( pAryPos )
        *pAryPos = aHitTest.nAryPos;
    return aHitTest.eType;
}

// -----------------------------------------------------------------------

void Ruler::SetWinPos( long nNewOff, long nNewWidth )
{
    // Gegebenenfalls werden die Breiten automatisch berechnet
    if ( !nNewWidth )
        mbAutoWinWidth = sal_True;
    else
        mbAutoWinWidth = sal_False;

    // Werte setzen (werden in ImplFormat gegebenenfalls mitberechnet)
    mnWinOff = nNewOff;
    mnWinWidth = nNewWidth;
    ImplUpdate( sal_True );
}

// -----------------------------------------------------------------------

void Ruler::SetPagePos( long nNewOff, long nNewWidth )
{
    // Muessen wir ueberhaupt was machen
    if ( (mpData->nPageOff == nNewOff) && (mpData->nPageWidth == nNewWidth) )
        return;

    // Gegebenenfalls werden die Breiten automatisch berechnet
    if ( !nNewWidth )
        mpData->bAutoPageWidth = sal_True;
    else
        mpData->bAutoPageWidth = sal_False;

    // Werte setzen (werden in ImplFormat gegebenenfalls mitberechnet)
    mpData->nPageOff     = nNewOff;
    mpData->nPageWidth   = nNewWidth;
    ImplUpdate( sal_True );
}

// -----------------------------------------------------------------------

void Ruler::SetBorderPos( long nOff )
{
    if ( mnWinStyle & WB_BORDER )
    {
        if ( mnBorderOff != nOff )
        {
            mnBorderOff = nOff;

            if ( IsReallyVisible() && IsUpdateMode() )
                Invalidate();
        }
    }
}

// -----------------------------------------------------------------------

void Ruler::SetUnit( FieldUnit eNewUnit )
{
    if ( meUnit != eNewUnit )
    {
        meUnit = eNewUnit;
        switch ( meUnit )
        {
            case FUNIT_MM:
                mnUnitIndex = RULER_UNIT_MM;
                break;
            case FUNIT_CM:
                mnUnitIndex = RULER_UNIT_CM;
                break;
            case FUNIT_M:
                mnUnitIndex = RULER_UNIT_M;
                break;
            case FUNIT_KM:
                mnUnitIndex = RULER_UNIT_KM;
                break;
            case FUNIT_INCH:
                mnUnitIndex = RULER_UNIT_INCH;
                break;
            case FUNIT_FOOT:
                mnUnitIndex = RULER_UNIT_FOOT;
                break;
            case FUNIT_MILE:
                mnUnitIndex = RULER_UNIT_MILE;
                break;
            case FUNIT_POINT:
                mnUnitIndex = RULER_UNIT_POINT;
                break;
            case FUNIT_PICA:
                mnUnitIndex = RULER_UNIT_PICA;
                break;
            case FUNIT_CHAR:
                mnUnitIndex = RULER_UNIT_CHAR;
                break;
            case FUNIT_LINE:
                mnUnitIndex = RULER_UNIT_LINE;
                break;
            default:
                SAL_WARN( "svtools.control", "Ruler::SetUnit() - Wrong Unit" );
                break;
        }

        maMapMode.SetMapUnit( aImplRulerUnitTab[mnUnitIndex].eMapUnit );
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

void Ruler::SetZoom( const Fraction& rNewZoom )
{
    DBG_ASSERT( rNewZoom.GetNumerator(), "Ruler::SetZoom() with scale 0 is not allowed" );

    if ( maZoom != rNewZoom )
    {
        maZoom = rNewZoom;
        maMapMode.SetScaleX( maZoom );
        maMapMode.SetScaleY( maZoom );
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

void Ruler::SetExtraType( RulerExtra eNewExtraType, sal_uInt16 nStyle )
{
    if ( mnWinStyle & WB_EXTRAFIELD )
    {
        meExtraType  = eNewExtraType;
        mnExtraStyle = nStyle;
        if ( IsReallyVisible() && IsUpdateMode() )
            ImplDrawExtra( sal_False );
    }
}

// -----------------------------------------------------------------------

void Ruler::SetNullOffset( long nPos )
{
    if ( mpData->nNullOff != nPos )
    {
        mpData->nNullOff = nPos;
        ImplUpdate();
    }
}

void Ruler::SetLeftFrameMargin( long nPos )
{
    if ( (mpData->nLeftFrameMargin != nPos) )
    {
        mpData->nLeftFrameMargin  = nPos;
        ImplUpdate();
    }
}

void Ruler::SetRightFrameMargin( long nPos )
{
    if ( (mpData->nRightFrameMargin != nPos) )
    {
        mpData->nRightFrameMargin  = nPos;
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

void Ruler::SetMargin1( long nPos, sal_uInt16 nMarginStyle )
{
    if ( (mpData->nMargin1 != nPos) || (mpData->nMargin1Style != nMarginStyle) )
    {
        mpData->nMargin1      = nPos;
        mpData->nMargin1Style = nMarginStyle;
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

void Ruler::SetMargin2( long nPos, sal_uInt16 nMarginStyle )
{
    DBG_ASSERT( (nPos >= mpData->nMargin1) ||
                (mpData->nMargin1Style & RULER_STYLE_INVISIBLE) ||
                (mpData->nMargin2Style & RULER_STYLE_INVISIBLE),
                "Ruler::SetMargin2() - Margin2 < Margin1" );

    if ( (mpData->nMargin2 != nPos) || (mpData->nMargin2Style != nMarginStyle) )
    {
        mpData->nMargin2      = nPos;
        mpData->nMargin2Style = nMarginStyle;
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

void Ruler::SetLines( sal_uInt16 n, const RulerLine* pLineAry )
{
    // To determine if what has changed
    if ( mpData->nLines == n )
    {
        sal_uInt16           i = n;
        const RulerLine* pAry1 = mpData->pLines;
        const RulerLine* pAry2 = pLineAry;
        while ( i )
        {
            if ( (pAry1->nPos   != pAry2->nPos)   ||
                 (pAry1->nStyle != pAry2->nStyle) )
                break;
            pAry1++;
            pAry2++;
            i--;
        }
        if ( !i )
            return;
    }

    // New values and new share issue
    sal_Bool bMustUpdate;
    if ( IsReallyVisible() && IsUpdateMode() )
        bMustUpdate = sal_True;
    else
        bMustUpdate = sal_False;

    // Delete old lines
    if ( bMustUpdate )
        ImplInvertLines();

    // New data set
    if ( !n || !pLineAry )
    {
        if ( !mpData->pLines )
            return;
        delete[] mpData->pLines;
        mpData->nLines = 0;
        mpData->pLines = NULL;
    }
    else
    {
        if ( mpData->nLines != n )
        {
            delete[] mpData->pLines;
            mpData->nLines = n;
            mpData->pLines = new RulerLine[n];
        }

        memcpy( mpData->pLines, pLineAry, n*sizeof( RulerLine ) );

        // Linien neu ausgeben
        if ( bMustUpdate )
            ImplInvertLines();
    }
}

// -----------------------------------------------------------------------

void Ruler::SetBorders( sal_uInt16 n, const RulerBorder* pBrdAry )
{
    if ( !n || !pBrdAry )
    {
        if ( !mpData->pBorders )
            return;
        delete[] mpData->pBorders;
        mpData->nBorders = 0;
        mpData->pBorders = NULL;
    }
    else
    {
        if ( mpData->nBorders != n )
        {
            delete[] mpData->pBorders;
            mpData->nBorders = n;
            mpData->pBorders = new RulerBorder[n];
        }
        else
        {
            sal_uInt16             i = n;
            const RulerBorder* pAry1 = mpData->pBorders;
            const RulerBorder* pAry2 = pBrdAry;
            while ( i )
            {
                if ( (pAry1->nPos   != pAry2->nPos)   ||
                     (pAry1->nWidth != pAry2->nWidth) ||
                     (pAry1->nStyle != pAry2->nStyle) )
                    break;
                pAry1++;
                pAry2++;
                i--;
            }
            if ( !i )
                return;
        }

        memcpy( mpData->pBorders, pBrdAry, n*sizeof( RulerBorder ) );
    }

    ImplUpdate();
}

// -----------------------------------------------------------------------

void Ruler::SetIndents( sal_uInt16 n, const RulerIndent* pIndentAry )
{

    if ( !n || !pIndentAry )
    {
        if ( !mpData->pIndents )
            return;
        delete[] mpData->pIndents;
        mpData->nIndents = 0;
        mpData->pIndents = NULL;
    }
    else
    {
        if ( mpData->nIndents != n )
        {
            delete[] mpData->pIndents;
            mpData->nIndents = n;
            mpData->pIndents = new RulerIndent[n];
        }
        else
        {
            sal_uInt16             i = n;
            const RulerIndent* pAry1 = mpData->pIndents;
            const RulerIndent* pAry2 = pIndentAry;
            while ( i )
            {
                if ( (pAry1->nPos   != pAry2->nPos) ||
                     (pAry1->nStyle != pAry2->nStyle) )
                    break;
                pAry1++;
                pAry2++;
                i--;
            }
            if ( !i )
                return;
        }

        memcpy( mpData->pIndents, pIndentAry, n*sizeof( RulerIndent ) );
    }

    ImplUpdate();
}

// -----------------------------------------------------------------------

void Ruler::SetTabs( sal_uInt16 n, const RulerTab* pTabAry )
{
    if ( !n || !pTabAry )
    {
        if ( !mpData->pTabs )
            return;
        delete[] mpData->pTabs;
        mpData->nTabs = 0;
        mpData->pTabs = NULL;
    }
    else
    {
        if ( mpData->nTabs != n )
        {
            delete[] mpData->pTabs;
            mpData->nTabs = n;
            mpData->pTabs = new RulerTab[n];
        }
        else
        {
            sal_uInt16          i = n;
            const RulerTab* pAry1 = mpData->pTabs;
            const RulerTab* pAry2 = pTabAry;
            while ( i )
            {
                if ( (pAry1->nPos   != pAry2->nPos) ||
                     (pAry1->nStyle != pAry2->nStyle) )
                    break;
                pAry1++;
                pAry2++;
                i--;
            }
            if ( !i )
                return;
        }

        memcpy( mpData->pTabs, pTabAry, n*sizeof( RulerTab ) );
    }

    ImplUpdate();
}

// -----------------------------------------------------------------------

void Ruler::SetStyle( WinBits nStyle )
{
    if ( mnWinStyle != nStyle )
    {
        mnWinStyle = nStyle;
        ImplInitExtraField( sal_True );
    }
}

// -----------------------------------------------------------------------

void Ruler::DrawTab( OutputDevice* pDevice, const Color &rFillColor, const Point& rPos, sal_uInt16 nStyle )
{
    Point                   aPos( rPos );
    sal_uInt16              nTabStyle = nStyle & (RULER_TAB_STYLE | RULER_TAB_RTL);

    pDevice->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
    pDevice->SetLineColor();
    pDevice->SetFillColor(rFillColor);
    ImplCenterTabPos( aPos, nTabStyle );
    ImplDrawRulerTab( pDevice, aPos, nTabStyle, nStyle  );
    pDevice->Pop();
}

void Ruler::SetTextRTL(sal_Bool bRTL)
{
    if(mpData->bTextRTL != bRTL)
    {
        mpData->bTextRTL = bRTL;
        if ( IsReallyVisible() && IsUpdateMode() )
            ImplInitExtraField( sal_True );
    }

}
long Ruler::GetPageOffset() const { return mpData->nPageOff; }
long Ruler::GetPageWidth() const { return mpData->nPageWidth; }
long Ruler::GetNullOffset() const { return mpData->nNullOff; }
long    Ruler::GetMargin1() const { return mpData->nMargin1; }
long    Ruler::GetMargin2() const { return mpData->nMargin2; }
long Ruler::GetRulerVirHeight() const { return mnVirHeight; }

RulerUnitData Ruler::GetCurrentRulerUnit() const
{
    return aImplRulerUnitTab[mnUnitIndex];
}

void Ruler::DrawTicks()
{
    mbFormat = sal_True;
    ImplDraw();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
