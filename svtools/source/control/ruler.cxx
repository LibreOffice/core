/*************************************************************************
 *
 *  $RCSfile: ruler.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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

#ifndef _INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#ifdef VCL
#include <vcl/svapp.hxx>
#else
#include <vcl/svapp.hxx>
#endif
#endif
#ifndef _POLY_HXX
#include <vcl/poly.hxx>
#endif

#define _SV_RULER_CXX
#define private public
#include <ruler.hxx>

// =======================================================================

#define RULER_OFF           2
#define RULER_TEXTOFF       2
#define RULER_RESIZE_OFF    4
#define RULER_LINE_WIDTH    7
#define RULER_MIN_SIZE      3

#define RULER_TICK1_WIDTH   1
#define RULER_TICK2_WIDTH   3
#define RULER_TICK3_WIDTH   5

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
#define RULER_UNIT_COUNT    9

struct ImplRulerUnitData
{
    MapUnit         eMapUnit;           // MAP_UNIT zum Umrechnen
    long            nTickUnit;          // Teiler fuer Einheit
    long            nTick1;             // Schrittweite
    long            nTick2;             // Tick fuer halbe Werte
    long            nTick3;             // Tick fuer Zahlenausgabe
    long            n100THMM;           // Teiler fuer Einheit
    USHORT          nUnitDigits;        // Anzahl Nachkommastellen
    sal_Char        aUnitStr[8];        // Einheiten-String
};

static ImplRulerUnitData aImplRulerUnitTab[RULER_UNIT_COUNT] =
{
{ MAP_100TH_MM,        100,    25,     50,    100,     100, 3, " mm"    }, // MM
{ MAP_100TH_MM,       1000,   250,    500,   1000,    1000, 3, " cm"    }, // CM
{ MAP_MM,             1000,   250,    500,   1000,   10000, 4, " m"     }, // M
{ MAP_CM,           100000, 25000,  50000, 100000,  100000, 6, " km"    }, // KM
{ MAP_100TH_INCH,      100,    10,     50,    100,    2540, 3, "\""     }, // INCH
{ MAP_100TH_INCH,     1200,   120,    600,   1200,   30480, 3, "'"      }, // FOOT
{ MAP_10TH_INCH,    633600, 63360, 316800, 633600, 1609344, 4, " miles" }, // MILE
{ MAP_POINT,             1,    12,     12,     36,     353, 2, " pt"    }, // POINT
{ MAP_100TH_MM,        423,   423,    423,    846,     423, 3, " pi"    }  // PICA
};

// =======================================================================

struct ImplRulerHitTest
{
    long        nPos;
    RulerType   eType;
    USHORT      nAryPos;
    USHORT      mnDragSize;
    BOOL        bSize;
    BOOL        bSizeBar;
};

// =======================================================================

ImplRulerData::ImplRulerData()
{
    memset( this, 0, sizeof( ImplRulerData ) );

    // PageBreite == EditWinBreite
    bAutoPageWidth   = TRUE;
}

// -----------------------------------------------------------------------

ImplRulerData::~ImplRulerData()
{
    if ( pLines )
        delete pLines;

    if ( pArrows )
        delete pArrows;

    if ( pBorders )
        delete pBorders;

    if ( pIndents )
        delete pIndents;

    if ( pTabs )
        delete pTabs;
}

// -----------------------------------------------------------------------

ImplRulerData& ImplRulerData::operator=( const ImplRulerData& rData )
{
    if ( pLines )
        delete pLines;

    if ( pArrows )
        delete pArrows;

    if ( pBorders )
        delete pBorders;

    if ( pIndents )
        delete pIndents;

    if ( pTabs )
        delete pTabs;

    memcpy( this, &rData, sizeof( ImplRulerData ) );

    if ( rData.pLines )
    {
        pLines = new RulerLine[nLines];
        memcpy( pLines, rData.pLines, nLines*sizeof( RulerLine ) );
    }

    if ( rData.pArrows )
    {
        pArrows = new RulerArrow[nArrows];
        memcpy( pArrows, rData.pArrows, nArrows*sizeof( RulerArrow ) );
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
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // Default WinBits setzen
    if ( !(nWinBits & WB_VERT) )
        nWinBits |= WB_HORZ;

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
    mbCalc          = TRUE;                 // Muessen Pagebreiten neu berechnet werden
    mbFormat        = TRUE;                 // Muss neu ausgegeben werden
    mbDrag          = FALSE;                // Sind wir im Drag-Modus
    mbDragDelete    = FALSE;                // Wird Maus beim Draggen unten rausgezogen
    mbDragCanceled  = FALSE;                // Wurde Dragging abgebrochen
    mbAutoWinWidth  = TRUE;                 // EditWinBreite == RulerBreite
    mbActive        = TRUE;                 // Ist Lineal aktiv
    mnUpdateFlags   = 0;                    // Was soll im Update-Handler upgedatet werden
    mpData          = &maData;              // Wir zeigen auf die normalen Daten
    meExtraType     = RULER_EXTRA_DONTKNOW; // Was im ExtraFeld dargestellt wird
    meDragType      = RULER_TYPE_DONTKNOW;  // Gibt an, was gedragt wird

    // Units initialisieren
    mnUnitIndex     = RULER_UNIT_CM;
    meUnit          = FUNIT_CM;
    maZoom          = Fraction( 1, 1 );
    meSourceUnit    = MAP_100TH_MM;

    // Border-Breiten berechnen
    if ( nWinBits & WB_BORDER )
    {
        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            mnBorderWidth = 2;
        else
            mnBorderWidth = 1;
    }
    else
        mnBorderWidth = 0;

    // Einstellungen setzen
    ImplInitSettings( TRUE, TRUE, TRUE );

    // Default-Groesse setzen
    long nDefHeight = GetTextHeight() + RULER_OFF*2 + RULER_TEXTOFF*2 + mnBorderWidth;
    Size aDefSize;
    if ( nWinBits & WB_HORZ )
        aDefSize.Height() = nDefHeight;
    else
        aDefSize.Width() = nDefHeight;
    SetOutputSizePixel( aDefSize );
}

// -----------------------------------------------------------------------

Ruler::Ruler( Window* pParent, WinBits nWinStyle ) :
    Window( pParent, nWinStyle & WB_3DLOOK ),
    maVirDev( *this ),
    maMapMode( MAP_100TH_MM )
{
    ImplInit( nWinStyle );
}

// -----------------------------------------------------------------------

Ruler::~Ruler()
{
    if ( mnUpdateEvtId )
        Application::RemoveUserEvent( mnUpdateEvtId );
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

void Ruler::ImplVDrawText( long nX, long nY, const String& rText )
{
    if ( (nX > -RULER_CLIP) && (nX < mnVirWidth+RULER_CLIP) )
    {
        if ( mnWinStyle & WB_HORZ )
            maVirDev.DrawText( Point( nX, nY ), rText );
        else
            maVirDev.DrawText( Point( nY, nX ), rText );
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplInvertLines( BOOL bErase )
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
        for ( USHORT i = 0; i < mpData->nLines; i++ )
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
    long    nTick = 0;
    long    nTick3 = aImplRulerUnitTab[mnUnitIndex].nTick3;
    long    nTickCount = aImplRulerUnitTab[mnUnitIndex].nTick1;
    Size    aPixSize = maVirDev.LogicToPixel( Size( nTick3, nTick3 ), maMapMode );
    long    nTickWidth;
    long    nX;
    long    nY;
    BOOL    bNoTicks = FALSE;

    // Groessenvorberechnung
    if ( mnWinStyle & WB_HORZ )
        nTickWidth = aPixSize.Width();
    else
        nTickWidth = aPixSize.Height();
    long nMaxWidth = maVirDev.PixelToLogic( Size( mpData->nPageWidth, 0 ), maMapMode ).Width();
    if ( nMaxWidth < 0 )
        nMaxWidth *= -1;
    nMaxWidth /= aImplRulerUnitTab[mnUnitIndex].nTickUnit;
    UniString aNumStr( UniString::CreateFromInt32( nMaxWidth ) );
    long nTxtWidth = GetTextWidth( aNumStr );
    if ( (nTxtWidth*2) > nTickWidth )
    {
        long nMulti     = 1;
        long nOrgTick3  = nTick3;
        long nTextOff   = 2;
        while ( nTickWidth < nTxtWidth+nTextOff )
        {
            long nOldMulti = nMulti;
            if ( !nTickWidth )
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
                bNoTicks = TRUE;
                break;
            }
            if ( nMulti >= 100 )
                nTextOff = 4;
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
        maVirDev.SetLineColor( GetSettings().GetStyleSettings().GetWindowTextColor() );

    if ( !bNoTicks )
    {
        long nTxtWidth2;
        long nTxtHeight2 = GetTextHeight()/2;
        while ( ((nStart-n) >= nMin) || ((nStart+n) <= nMax) )
        {
            // Null-Punkt
            if ( !nTick )
            {
                if ( nStart > nMin )
                {
                    // Nur 0 malen, wenn Margin1 nicht gleich dem NullPunkt ist
                    if ( (mpData->nMargin1Style & RULER_STYLE_INVISIBLE) || (mpData->nMargin1 != 0) )
                    {
                        aNumStr = (sal_Unicode)'0';
                        nTxtWidth2 = maVirDev.GetTextWidth( aNumStr )/2;
                        if ( mnWinStyle & WB_HORZ )
                            nX = nStart-nTxtWidth2;
                        else
                            nX = nStart+nTxtWidth2;
                        ImplVDrawText( nX, nCenter-nTxtHeight2, aNumStr );
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

                // Tick3 - Ausgabe (Text)
                if ( !(nTick % nTick3) )
                {
                    aNumStr = UniString::CreateFromInt32( nTick / aImplRulerUnitTab[mnUnitIndex].nTickUnit );
                    nTxtWidth2 = GetTextWidth( aNumStr )/2;

                    nX = nStart+n;
                    nY = nCenter-nTxtHeight2;
                    if ( nX < nMax )
                    {
                        if ( mnWinStyle & WB_HORZ )
                            nX -= nTxtWidth2;
                        else
                            nX += nTxtWidth2;
                        ImplVDrawText( nX, nY, aNumStr );
                    }
                    nX = nStart-n;
                    if ( nX > nMin )
                    {
                        if ( mnWinStyle & WB_HORZ )
                            nX -= nTxtWidth2;
                        else
                            nX += nTxtWidth2;
                        ImplVDrawText( nX, nY, aNumStr );
                    }
                }
                // Tick/Tick2 - Ausgabe (Striche)
                else
                {
                    if ( !(nTick % aImplRulerUnitTab[mnUnitIndex].nTick2) )
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

            nTick += nTickCount;
        }
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawArrows( long nCenter )
{
    USHORT          i;
    long            n1;
    long            n2;
    long            n3;
    long            n4;
    long            nLogWidth;
    String          aStr;
    String          aStr2;
    BOOL            bDrawUnit;
    long            nTxtWidth;
    long            nTxtHeight2 = GetTextHeight()/2;
    International   aIntn = Application::GetAppInternational();

    aIntn.SetNumTrailingZeros( FALSE );
    maVirDev.SetLineColor( GetSettings().GetStyleSettings().GetWindowTextColor() );
    for ( i = 0; i < mpData->nArrows; i++ )
    {
        n1 = mpData->pArrows[i].nPos+mpData->nNullVirOff+1;
        n2 = n1+mpData->pArrows[i].nWidth-2;

        // Einheit umrechnen
        nLogWidth = mpData->pArrows[i].nLogWidth;
        if ( meSourceUnit == MAP_TWIP )
        {
            if ( nLogWidth >= 100000 )
                nLogWidth = (nLogWidth*254)/144;
            else
                nLogWidth = (nLogWidth*2540)/1440;
        }
        if ( nLogWidth >= 1000000 )
            nLogWidth = (nLogWidth / aImplRulerUnitTab[mnUnitIndex].n100THMM) * 1000;
        else
            nLogWidth = (nLogWidth*1000) / aImplRulerUnitTab[mnUnitIndex].n100THMM;
        aStr = aIntn.GetNum( nLogWidth, aImplRulerUnitTab[mnUnitIndex].nUnitDigits );

        // Einheit an den String haengen
        aStr2 = aStr;
        aStr2.AppendAscii( aImplRulerUnitTab[mnUnitIndex].aUnitStr );

        // Textbreite ermitteln
        bDrawUnit = TRUE;
        nTxtWidth = GetTextWidth( aStr2 );
        if ( nTxtWidth < mpData->pArrows[i].nWidth-10 )
            aStr = aStr2;
        else
        {
            nTxtWidth = GetTextWidth( aStr );
            if ( nTxtWidth > mpData->pArrows[i].nWidth-10 )
                bDrawUnit = FALSE;
        }

        // Ist genuegen Platz fuer Einheiten-String vorhanden
        if ( bDrawUnit )
        {
            n3 = n1 + ((n2-n1)/2) - 1;
            if ( mnWinStyle & WB_HORZ )
                n3 -= nTxtWidth/2;
            else
                n3 += nTxtWidth/2;
            if ( mnWinStyle & WB_HORZ )
            {
                n4 = n3 + nTxtWidth + 2;
                ImplVDrawLine( n1, nCenter, n3, nCenter );
                ImplVDrawLine( n4, nCenter, n2, nCenter );
            }
            else
            {
                n4 = n3 - nTxtWidth - 2;
                ImplVDrawLine( n1, nCenter, n4, nCenter );
                ImplVDrawLine( n3, nCenter, n2, nCenter );
            }
            ImplVDrawText( n3, nCenter-nTxtHeight2, aStr );
        }
        else
            ImplVDrawLine( n1, nCenter, n2, nCenter );
        ImplVDrawLine( n1+1, nCenter-1, n1+1, nCenter+1 );
        ImplVDrawLine( n1+2, nCenter-2, n1+2, nCenter+2 );
        ImplVDrawLine( n2-1, nCenter-1, n2-1, nCenter+1 );
        ImplVDrawLine( n2-2, nCenter-2, n2-2, nCenter+2 );
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
    USHORT  i;

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
                if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
                    maVirDev.SetFillColor( rStyleSettings.GetFaceColor() );
                else
                    maVirDev.SetFillColor( rStyleSettings.GetWindowColor() );
                ImplVDrawRect( n1, nVirTop, n2, nVirBottom );
                if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
                {
                    maVirDev.SetLineColor( rStyleSettings.GetLightColor() );
                    ImplVDrawLine( n1+1, nVirTop, n1+1, nVirBottom );
                    ImplVDrawLine( n1, nVirTop, n2, nVirTop );
                    maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
                    ImplVDrawLine( n1, nVirTop, n1, nVirBottom );
                    ImplVDrawLine( n1, nVirBottom, n2, nVirBottom );
                    ImplVDrawLine( n2-1, nVirTop, n2-1, nVirBottom );
                    maVirDev.SetLineColor( rStyleSettings.GetWindowTextColor() );
                    ImplVDrawLine( n2, nVirTop, n2, nVirBottom );
                }
                else
                {
                    maVirDev.SetLineColor( rStyleSettings.GetWindowTextColor() );
                    ImplVDrawLine( n1, nVirTop, n1, nVirBottom );
                    ImplVDrawLine( n2, nVirTop, n2, nVirBottom );
                }

                if ( mpData->pBorders[i].nStyle & RULER_BORDER_VARIABLE )
                {
                    if ( n2-n1 > RULER_VAR_SIZE+4 )
                    {
                        nTemp1 = n1 + (((n2-n1+1)-RULER_VAR_SIZE) / 2);
                        nTemp2 = nVirTop + (((nVirBottom-nVirTop+1)-RULER_VAR_SIZE) / 2);
                        long nTemp3 = nTemp1+RULER_VAR_SIZE-1;
                        long nTemp4 = nTemp2+RULER_VAR_SIZE-1;
                        long nTempY = nTemp2;
                        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
                            maVirDev.SetLineColor( rStyleSettings.GetLightColor() );
                        else
                            maVirDev.SetLineColor( rStyleSettings.GetWindowTextColor() );
                        while ( nTempY <= nTemp4 )
                        {
                            ImplVDrawLine( nTemp1, nTempY, nTemp3, nTempY );
                            nTempY += 2;
                        }
                        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
                        {
                            nTempY = nTemp2+1;
                            maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
                            while ( nTempY <= nTemp4 )
                            {
                                ImplVDrawLine( nTemp1, nTempY, nTemp3, nTempY );
                                nTempY += 2;
                            }
                        }
                    }
                }

                if ( mpData->pBorders[i].nStyle & RULER_BORDER_SIZEABLE )
                {
                    if ( n2-n1 > RULER_VAR_SIZE+10 )
                    {
                        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
                        {
                            maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
                            ImplVDrawLine( n1+4, nVirTop+3, n1+4, nVirBottom-3 );
                            ImplVDrawLine( n2-5, nVirTop+3, n2-5, nVirBottom-3 );
                            maVirDev.SetLineColor( rStyleSettings.GetLightColor() );
                            ImplVDrawLine( n1+5, nVirTop+3, n1+5, nVirBottom-3 );
                            ImplVDrawLine( n2-4, nVirTop+3, n2-4, nVirBottom-3 );
                        }
                        else
                        {
                            maVirDev.SetLineColor( rStyleSettings.GetWindowTextColor() );
                            ImplVDrawLine( n1+4, nVirTop+3, n1+4, nVirBottom-3 );
                            ImplVDrawLine( n2-4, nVirTop+3, n2-4, nVirBottom-3 );
                        }
                    }
                }
            }
            else
            {
                n = n1+((n2-n1)/2);
                if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
                    maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
                else
                    maVirDev.SetLineColor( rStyleSettings.GetWindowTextColor() );
                if ( mpData->pBorders[i].nStyle & RULER_BORDER_SNAP )
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

void Ruler::ImplDrawIndent( const Polygon& rPoly, USHORT nStyle )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    Point   aPos1;
    Point   aPos2;
    USHORT  nIndentStyle = nStyle & RULER_INDENT_STYLE;

    if ( nStyle & RULER_STYLE_INVISIBLE )
        return;

    if ( nStyle & RULER_STYLE_DONTKNOW )
    {
        maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
        maVirDev.SetFillColor( rStyleSettings.GetFaceColor() );
    }
    else
    {
        maVirDev.SetLineColor( rStyleSettings.GetDarkShadowColor() );
        maVirDev.SetFillColor( rStyleSettings.GetFaceColor() );
    }

    maVirDev.DrawPolygon( rPoly );

    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) && !(nStyle & RULER_STYLE_DONTKNOW) )
    {
        if ( nIndentStyle == RULER_INDENT_BOTTOM )
        {
            maVirDev.SetLineColor( rStyleSettings.GetLightColor() );
            aPos1 = rPoly.GetPoint( 2 );
            aPos1.X()++;
            aPos2 = rPoly.GetPoint( 1 );
            aPos2.X()++;
            maVirDev.DrawLine( aPos2, aPos1 );
            aPos2.X()--;
            aPos2.Y()++;
            aPos1 = rPoly.GetPoint( 0 );
            aPos1.Y()++;
            maVirDev.DrawLine( aPos2, aPos1 );
            maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
            aPos2 = rPoly.GetPoint( 4 );
            aPos2.Y()++;
            maVirDev.DrawLine( aPos1, aPos2 );
            aPos2.X()--;
            aPos1 = rPoly.GetPoint( 3 );
            aPos1.X()--;
            maVirDev.DrawLine( aPos2, aPos1 );
            aPos1.Y()--;
            aPos2 = rPoly.GetPoint( 2 );
            aPos2.X()++;
            aPos2.Y()--;
            maVirDev.DrawLine( aPos2, aPos1 );
        }
        else
        {
            maVirDev.SetLineColor( rStyleSettings.GetLightColor() );
            aPos1 = rPoly.GetPoint( 2 );
            aPos1.X()++;
            aPos1.Y()++;
            aPos2 = rPoly.GetPoint( 3 );
            aPos2.Y()++;
            maVirDev.DrawLine( aPos1, aPos2 );
            aPos2 = rPoly.GetPoint( 1 );
            aPos2.X()++;
            maVirDev.DrawLine( aPos1, aPos2 );
            aPos2.X()--;
            aPos2.Y()--;
            aPos1 = rPoly.GetPoint( 0 );
            aPos1.Y()--;
            maVirDev.DrawLine( aPos2, aPos1 );
            maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
            aPos2 = rPoly.GetPoint( 4 );
            aPos2.Y()--;
            maVirDev.DrawLine( aPos1, aPos2 );
            aPos2.X()--;
            aPos1 = rPoly.GetPoint( 3 );
            aPos1.X()--;
            maVirDev.DrawLine( aPos2, aPos1 );
        }

        maVirDev.SetLineColor( rStyleSettings.GetDarkShadowColor() );
        maVirDev.SetFillColor();
        maVirDev.DrawPolygon( rPoly );
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawIndents( long nMin, long nMax, long nVirTop, long nVirBottom )
{
    USHORT  i;
    long    n;
    long    nIndentHeight = (mnVirHeight/2) - 1;
    long    nIndentWidth2 = nIndentHeight-3;
    Polygon aPoly( 5 );

    for ( i = 0; i < mpData->nIndents; i++ )
    {
        if ( mpData->pIndents[i].nStyle & RULER_STYLE_INVISIBLE )
            continue;

        USHORT  nStyle = mpData->pIndents[i].nStyle;
        USHORT  nIndentStyle = nStyle & RULER_INDENT_STYLE;

        n = mpData->pIndents[i].nPos+mpData->nNullVirOff;

        if ( (n >= nMin) && (n <= nMax) )
        {
            if ( nIndentStyle == RULER_INDENT_BOTTOM )
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

            ImplDrawIndent( aPoly, nStyle );
        }
    }
}

// -----------------------------------------------------------------------

static void ImplCenterTabPos( Point& rPos, USHORT nStyle )
{
    rPos.Y() += RULER_TAB_HEIGHT/2;
    if ( nStyle == RULER_TAB_LEFT )
        rPos.X() -= RULER_TAB_WIDTH/2;
    else if ( nStyle == RULER_TAB_RIGHT )
        rPos.X() += RULER_TAB_WIDTH/2;
}

// -----------------------------------------------------------------------

static void ImplDrawRulerTab( OutputDevice* pDevice,
                             const Point& rPos, USHORT nStyle )
{
    if ( nStyle & RULER_STYLE_INVISIBLE )
        return;

    USHORT nTabStyle = nStyle & RULER_TAB_STYLE;

    if ( nTabStyle == RULER_TAB_DEFAULT )
    {
        pDevice->DrawRect( Rectangle( rPos.X() - RULER_TAB_DWIDTH2 + 1,
                                      rPos.Y() - RULER_TAB_DHEIGHT2 + 1,
                                      rPos.X() - RULER_TAB_DWIDTH2 + RULER_TAB_DWIDTH,
                                      rPos.Y() ) );
        pDevice->DrawRect( Rectangle( rPos.X() - RULER_TAB_DWIDTH2 + RULER_TAB_DWIDTH3,
                                      rPos.Y() - RULER_TAB_DHEIGHT + 1,
                                      rPos.X() - RULER_TAB_DWIDTH2 + RULER_TAB_DWIDTH3 + RULER_TAB_DWIDTH4 - 1,
                                      rPos.Y() ) );

    }
    else if ( nTabStyle == RULER_TAB_LEFT )
    {
        pDevice->DrawRect( Rectangle( rPos.X(),
                                      rPos.Y() - RULER_TAB_HEIGHT2 + 1,
                                      rPos.X() + RULER_TAB_WIDTH - 1,
                                      rPos.Y() ) );
        pDevice->DrawRect( Rectangle( rPos.X(),
                                      rPos.Y() - RULER_TAB_HEIGHT + 1,
                                      rPos.X() + RULER_TAB_WIDTH2 - 1,
                                      rPos.Y() ) );
    }
    else if ( nTabStyle == RULER_TAB_RIGHT )
    {
        pDevice->DrawRect( Rectangle( rPos.X() - RULER_TAB_WIDTH + 1,
                                      rPos.Y() - RULER_TAB_HEIGHT2 + 1,
                                      rPos.X(),
                                      rPos.Y() ) );
        pDevice->DrawRect( Rectangle( rPos.X() - RULER_TAB_WIDTH2 + 1,
                                      rPos.Y() - RULER_TAB_HEIGHT + 1,
                                      rPos.X(),
                                      rPos.Y() ) );
    }
    else
    {
        pDevice->DrawRect( Rectangle( rPos.X() - RULER_TAB_CWIDTH2 + 1,
                                      rPos.Y() - RULER_TAB_HEIGHT2 + 1,
                                      rPos.X() - RULER_TAB_CWIDTH2 + RULER_TAB_CWIDTH,
                                      rPos.Y() ) );
        pDevice->DrawRect( Rectangle( rPos.X() - RULER_TAB_CWIDTH2 + RULER_TAB_CWIDTH3,
                                      rPos.Y() - RULER_TAB_HEIGHT + 1,
                                      rPos.X() - RULER_TAB_CWIDTH2 + RULER_TAB_CWIDTH3 + RULER_TAB_CWIDTH4 - 1,
                                      rPos.Y() ) );

        if ( nTabStyle == RULER_TAB_DECIMAL )
        {
            pDevice->DrawRect( Rectangle( rPos.X() - RULER_TAB_CWIDTH2 + RULER_TAB_CWIDTH - 1,
                                          rPos.Y() - RULER_TAB_HEIGHT + 1 + 1,
                                          rPos.X() - RULER_TAB_CWIDTH2 + RULER_TAB_CWIDTH,
                                          rPos.Y() - RULER_TAB_HEIGHT + 1 + 2 ) );
        }
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawTab( OutputDevice* pDevice, const Point& rPos, USHORT nStyle )
{
    if ( nStyle & RULER_STYLE_INVISIBLE )
        return;

    pDevice->SetLineColor();
    if ( nStyle & RULER_STYLE_DONTKNOW )
        pDevice->SetFillColor( GetSettings().GetStyleSettings().GetFaceColor() );
    else
        pDevice->SetFillColor( GetSettings().GetStyleSettings().GetWindowTextColor() );

    ImplDrawRulerTab( pDevice, rPos, nStyle );
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawTabs( long nMin, long nMax, long nVirBottom )
{
    for ( USHORT i = 0; i < mpData->nTabs; i++ )
    {
        if ( mpData->pTabs[i].nStyle & RULER_STYLE_INVISIBLE )
            continue;

        long n = mpData->pTabs[i].nPos+mpData->nNullVirOff;
        if ( (n >= nMin) && (n <= nMax) )
            ImplDrawTab( &maVirDev, Point( n, nVirBottom ), mpData->pTabs[i].nStyle );
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplInitSettings( BOOL bFont,
                              BOOL bForeground, BOOL bBackground )
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
            aColor = rStyleSettings.GetButtonTextColor();
        SetTextColor( aColor );
        SetTextFillColor();
    }

    if ( bBackground )
    {
        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else
            aColor = rStyleSettings.GetFaceColor();
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
        mpData->nRulWidth = Min( mnWinWidth, mpData->nPageWidth-nNotVisPageWidth );
        if ( nRulWinOff+mpData->nRulWidth > mnWidth )
            mpData->nRulWidth = mnWidth-nRulWinOff;
    }
    else
    {
        if ( mbAutoWinWidth )
            mnWinWidth = mnHeight - mnVirOff;
        if ( mpData->bAutoPageWidth )
            mpData->nPageWidth = mnWinWidth;
        mpData->nRulWidth = Min( mnWinWidth, mpData->nPageWidth-nNotVisPageWidth );
        if ( nRulWinOff+mpData->nRulWidth > mnHeight )
            mpData->nRulWidth = mnHeight-nRulWinOff;
    }

    mbCalc = FALSE;
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
    nVirRight   = nVirLeft+mpData->nRulWidth-1;
    nVirTop     = 0;
    nVirBottom  = mnVirHeight-1;

    if ( !IsReallyVisible() )
        return;

    Size    aVirDevSize;
    BOOL    b3DLook = !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO);

    // VirtualDevice initialisieren
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
        maVirDev.SetOutputSizePixel( aVirDevSize, TRUE );
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
    if ( b3DLook )
        maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
    else
        maVirDev.SetLineColor( rStyleSettings.GetWindowTextColor() );
    ImplVDrawLine( nVirLeft, nVirTop, nP2, nVirTop );
    if ( b3DLook )
        maVirDev.SetLineColor( rStyleSettings.GetLightColor() );
    ImplVDrawLine( nVirLeft, nVirBottom, nP2, nVirBottom );

    // Jetzt wird zwischen dem Schatten ausgegeben
    nVirTop++;
    nVirBottom--;

    // Margin1, Margin2 und Zwischenraum ausgeben
    maVirDev.SetLineColor();
    if ( b3DLook )
        maVirDev.SetFillColor( rStyleSettings.GetFaceColor() );
    else
        maVirDev.SetFillColor( rStyleSettings.GetWindowColor() );
    if ( nM1 > nVirLeft )
        ImplVDrawRect( nP1, nVirTop, nM1-1, nVirBottom );
    if ( nM2 < nP2 )
        ImplVDrawRect( nM2+1, nVirTop, nP2, nVirBottom );
    if ( nM2-nM1 > 0 )
    {
        maVirDev.SetFillColor( rStyleSettings.GetWindowColor() );
        ImplVDrawRect( nM1, nVirTop, nM2-1, nVirBottom );
    }
    if ( b3DLook )
    {
        maVirDev.SetLineColor( rStyleSettings.GetLightColor() );
        if ( nM1 > nVirLeft )
        {
            ImplVDrawLine( nP1+1, nVirTop, nM1-1, nVirTop );
            if ( nP1 >= nVirLeft )
            {
                ImplVDrawLine( nP1+1, nVirTop, nP1+1, nVirBottom );
                ImplVDrawLine( nP1, nVirBottom+1, nP1+1, nVirBottom+1 );
            }
        }
        if ( nM2 < nP2 )
        {
            ImplVDrawLine( nM2+1, nVirTop, nM2+1, nVirBottom );
            ImplVDrawLine( nM2+1, nVirTop, nP2, nVirTop );
            if ( nP2 <= nVirRight+1 )
            {
                ImplVDrawLine( nP2, nVirTop-1, nP2, nVirBottom );
                ImplVDrawLine( nP2-1, nVirBottom+1, nP2, nVirBottom+1 );
            }
        }
        maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
        if ( nM1 > nVirLeft )
        {
            ImplVDrawLine( nM1-1, nVirTop, nM1-1, nVirBottom );
            ImplVDrawLine( nP1, nVirBottom, nM1-1, nVirBottom );
            if ( nP1 >= nVirLeft )
            {
                ImplVDrawLine( nP1, nVirTop, nP1, nVirBottom );
                ImplVDrawLine( nP1, nVirBottom, nP1+1, nVirBottom );
            }
        }
        if ( nM2 < nP2 )
        {
            ImplVDrawLine( nM2+1, nVirBottom, nP2-1, nVirBottom );
            if ( nP2 <= nVirRight+1 )
                ImplVDrawLine( nP2-1, nVirTop, nP2-1, nVirBottom );
        }
        if ( nM2-nM1 > 0 )
        {
            maVirDev.SetLineColor( rStyleSettings.GetDarkShadowColor() );
            if ( nM1 >= nVirLeft )
                ImplVDrawLine( nM1, nVirTop, nM1, nVirBottom );
            ImplVDrawLine( nM1, nVirTop, nM2-1, nVirTop );
            maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
            ImplVDrawLine( nM1, nVirBottom, nM2-1, nVirBottom );
            if ( nM2 <= nVirRight )
                ImplVDrawLine( nM2, nVirTop, nM2, nVirBottom );
        }
    }
    else
    {
        maVirDev.SetLineColor( rStyleSettings.GetWindowTextColor() );
        if ( nP1 >= nVirLeft )
            ImplVDrawLine( nP1, nVirTop, nP1, nVirBottom+1 );
        if ( nM1 > nP1 )
            ImplVDrawLine( nM1, nVirTop, nM1, nVirBottom );
        if ( nM2 < nP2 )
            ImplVDrawLine( nM2, nVirTop, nM2, nVirBottom );
        if ( nP2 <= nVirRight+1 )
            ImplVDrawLine( nP2, nVirTop, nP2, nVirBottom+1 );
    }

    // Lineal-Beschriftung (nur wenn keine Bemassungspfeile)
    if ( !mpData->pArrows )
    {
        long    nMin = nVirLeft;
        long    nMax = nP2;
        long    nStart = nNullVirOff;
        long    nCenter = nVirTop+((nVirBottom-nVirTop)/2);

        // Nicht Schatten uebermalen
        if ( nP1 > nVirLeft )
            nMin++;
        if ( nP2 < nVirRight )
            nMax--;

        // Beschriftung ausgeben
        ImplDrawTicks( nMin, nMax, nStart, nCenter );
    }

    // Spalten ausgeben
    if ( mpData->pBorders )
        ImplDrawBorders( nVirLeft, nP2, nVirTop, nVirBottom );

    // Einzuege ausgeben
    if ( mpData->pIndents )
        ImplDrawIndents( nVirLeft, nP2, nVirTop-1, nVirBottom+1 );

    // Tabs
    if ( mpData->pTabs )
        ImplDrawTabs( nVirLeft, nP2, nVirBottom+1 );

    // Bemassungspfeile
    if ( mpData->pArrows )
        ImplDrawArrows( nVirTop+((nVirBottom-nVirTop)/2) );

    // Wir haben formatiert
    mbFormat = FALSE;
}

// -----------------------------------------------------------------------

void Ruler::ImplInitExtraField( BOOL bUpdate )
{
    // Extra-Field beruecksichtigen
    if ( mnWinStyle & WB_EXTRAFIELD )
    {
        maExtraRect.Left()   = RULER_OFF;
        maExtraRect.Top()    = RULER_OFF;
        maExtraRect.Right()  = RULER_OFF+mnVirHeight-1;
        maExtraRect.Bottom() = RULER_OFF+mnVirHeight-1;
        mnVirOff = maExtraRect.Right()+1;
    }
    else
    {
        maExtraRect.SetEmpty();
        mnVirOff = 0;
    }

    if ( bUpdate )
    {
        mbCalc      = TRUE;
        mbFormat    = TRUE;
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
        if ( mnWinStyle & WB_HORZ )
        {
            aOffPos.X() = mnVirOff;
            aOffPos.Y() = RULER_OFF;
        }
        else
        {
            aOffPos.X() = RULER_OFF;
            aOffPos.Y() = mnVirOff;
        }
        DrawOutDev( aOffPos, aVirDevSize, Point(), aVirDevSize, maVirDev );

        // Positionslinien neu malen
        ImplInvertLines( TRUE );
    }
}

// -----------------------------------------------------------------------

void Ruler::ImplDrawExtra( BOOL bPaint )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    Rectangle   aRect = maExtraRect;
    BOOL        bEraseRect = FALSE;

    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
    {
        aRect.Left()    += 2;
        aRect.Top()     += 2;
        aRect.Right()   -= 2;
        aRect.Bottom()  -= 2;
    }
    else
    {
        aRect.Left()    += 1;
        aRect.Top()     += 1;
        aRect.Right()   -= 1;
        aRect.Bottom()  -= 1;
    }

    if ( !bPaint && !(mnExtraStyle & RULER_STYLE_HIGHLIGHT) )
    {
        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            SetFillColor( rStyleSettings.GetFaceColor() );
        else
            SetFillColor( rStyleSettings.GetWindowColor() );
        bEraseRect = TRUE;
    }
    else
    {
        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) && (mnExtraStyle & RULER_STYLE_HIGHLIGHT) )
        {
            SetFillColor( rStyleSettings.GetCheckedColor() );
            bEraseRect = TRUE;
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
        SetLineColor( rStyleSettings.GetWindowTextColor() );
        DrawLine( Point( aRect.Left()+1, aRect.Top()+4 ),
                  Point( aRect.Right()-1, aRect.Top()+4 ) );
        DrawLine( Point( aRect.Left()+4, aRect.Top()+1 ),
                  Point( aRect.Left()+4, aRect.Bottom()-1 ) );
    }
    else if ( meExtraType == RULER_EXTRA_TAB )
    {
        USHORT nTabStyle = mnExtraStyle & RULER_TAB_STYLE;
        Point aCenter = aRect.Center();
        ImplCenterTabPos( aCenter, nTabStyle );
        ImplDrawTab( this, aCenter, nTabStyle );
    }

    if ( (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) && (mnExtraStyle & RULER_STYLE_HIGHLIGHT) )
        Invert( aRect );
}

// -----------------------------------------------------------------------

void Ruler::ImplUpdate( BOOL bMustCalc )
{
    // Hier schon Linien loeschen, damit Sie vor dem Neuberechnen schon
    // geloscht sind, da danach die alten Positionen nicht mehr bestimmt
    // werden koennen
    if ( !mbFormat )
        ImplInvertLines();

    // Flags setzen
    if ( bMustCalc )
        mbCalc = TRUE;
    mbFormat = TRUE;

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

BOOL Ruler::ImplHitTest( const Point& rPos, ImplRulerHitTest* pHitTest ) const
{
    USHORT      i;
    USHORT      nStyle;
    long        nHitBottom;
    long        nX;
    long        nY;
    long        n1;
    long        n2;

    if ( !mbActive )
        return FALSE;

    // Position ermitteln
    if ( mnWinStyle & WB_HORZ )
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

    // Initialisieren
    memset( pHitTest, 0, sizeof( ImplRulerHitTest ) );

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
        return FALSE;
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
                        return TRUE;
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
            if ( !(nStyle & RULER_STYLE_INVISIBLE) )
            {
                nStyle &= RULER_INDENT_STYLE;
                n1 = mpData->pIndents[i-1].nPos;

                if ( nStyle == RULER_INDENT_BOTTOM )
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
                    return TRUE;
                }
            }
        }
    }

    // Jetzt zaehlt nichts mehr, was links oder rechts uebersteht
    if ( (nXTemp < mpData->nRulVirOff) || (nXTemp > mpData->nRulVirOff+mpData->nRulWidth) )
    {
        pHitTest->nPos = 0;
        pHitTest->eType = RULER_TYPE_OUTSIDE;
        return FALSE;
    }

    // Danach die Spalten testen
    for ( i = mpData->nBorders; i; i-- )
    {
        n1 = mpData->pBorders[i-1].nPos;
        n2 = n1 + mpData->pBorders[i-1].nWidth;

        // Spalten werden mit mindestens 3 Pixel breite gezeichnet
        if ( !mpData->pBorders[i-1].nWidth )
        {
            n1--;
            n2++;
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
                        pHitTest->bSizeBar = TRUE;
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
                        pHitTest->bSize = TRUE;
                        pHitTest->mnDragSize = RULER_DRAGSIZE_1;
                    }
                    else if ( nX >= n2-nMOff )
                    {
                        pHitTest->bSize = TRUE;
                        pHitTest->mnDragSize = RULER_DRAGSIZE_2;
                    }
                    else
                    {
                        if ( nStyle & RULER_BORDER_MOVEABLE )
                        {
                            pHitTest->bSizeBar = TRUE;
                            pHitTest->mnDragSize = RULER_DRAGSIZE_MOVE;
                        }
                    }
                }

                return TRUE;
            }
        }
    }

    // Und zum Schluss die Raender
    if ( (mpData->nMargin1Style & (RULER_MARGIN_SIZEABLE | RULER_STYLE_INVISIBLE)) == RULER_MARGIN_SIZEABLE )
    {
        n1 = mpData->nMargin1;
        if ( (nX >= n1-RULER_MOUSE_MARGINWIDTH) && (nX <= n1+RULER_MOUSE_MARGINWIDTH) )
        {
            pHitTest->eType = RULER_TYPE_MARGIN1;
            pHitTest->bSize = TRUE;
            return TRUE;
        }
    }
    if ( (mpData->nMargin2Style & (RULER_MARGIN_SIZEABLE | RULER_STYLE_INVISIBLE)) == RULER_MARGIN_SIZEABLE )
    {
        n1 = mpData->nMargin2;
        if ( (nX >= n1-RULER_MOUSE_MARGINWIDTH) && (nX <= n1+RULER_MOUSE_MARGINWIDTH) )
        {
            pHitTest->eType = RULER_TYPE_MARGIN2;
            pHitTest->bSize = TRUE;
            return TRUE;
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
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Ruler::ImplDocHitTest( const Point& rPos, RulerType eDragType,
                            ImplRulerHitTest* pHitTest ) const
{
    Point aPos = rPos;

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
        if ( ImplHitTest( aPos, pHitTest ) )
        {
            if ( (pHitTest->eType == eDragType) || (eDragType == RULER_TYPE_DONTKNOW) )
                return TRUE;
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
        if ( ImplHitTest( aPos, pHitTest ) )
        {
            if ( (pHitTest->eType == eDragType) || (eDragType == RULER_TYPE_DONTKNOW) )
                return TRUE;
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
                return TRUE;
        }
    }

    // Auf DontKnow setzen
    pHitTest->eType = RULER_TYPE_DONTKNOW;

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Ruler::ImplStartDrag( ImplRulerHitTest* pHitTest, USHORT nModifier )
{
    // Wenn eine Spalte angeklick wurde, die weder verschiebar noch
    // in der Groesse aenderbar ist, brauchen wir auch kein Drag ausloesen
    if ( (pHitTest->eType == RULER_TYPE_BORDER) &&
         !pHitTest->bSize && !pHitTest->bSizeBar )
        return FALSE;

    // Dragdaten setzen
    meDragType      = pHitTest->eType;
    mnDragPos       = pHitTest->nPos;
    mnDragAryPos    = pHitTest->nAryPos;
    mnDragSize      = pHitTest->mnDragSize;
    mnDragModifier  = nModifier;
    maDragData      = maData;
    mpData          = &maDragData;

    // Handler rufen
    if ( StartDrag() )
    {
        // Wenn der Handler das Draggen erlaubt, dann das Draggen
        // initialisieren
        ImplInvertLines();
        mbDrag = TRUE;
        mnStartDragPos = mnDragPos;
        StartTracking();
        return TRUE;
    }
    else
    {
        // Ansonsten muessen wir die Daten zuruecksetzen
        meDragType      = RULER_TYPE_DONTKNOW;
        mnDragPos       = 0;
        mnDragAryPos    = 0;
        mnDragSize      = 0;
        mnDragModifier  = 0;
        mpData          = &maData;
    }

    return FALSE;
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
    mbDragDelete = FALSE;
    if ( nY < 0 )
    {
        if ( !mbDragCanceled )
        {
            // Daten wiederherstellen
            mbDragCanceled = TRUE;
            ImplRulerData aTempData;
            aTempData = maDragData;
            maDragData = maData;
            mbCalc = TRUE;
            mbFormat = TRUE;

            // Handler rufen
            mnDragPos = mnStartDragPos;
            Drag();

            // Und neu ausgeben (zeitverzoegert)
/*
            mnUpdateFlags |= RULER_UPDATE_DRAW;
            if ( mnUpdateEvtId )
                Application::RemoveUserEvent( mnUpdateEvtId );
            mnUpdateEvtId = Application::PostUserEvent( LINK( this, Ruler, ImplUpdateHdl ), NULL );
*/
            ImplDraw();

            // Daten wieder wie vor dem Cancel herstellen
            maDragData = aTempData;
        }
    }
    else
    {
        mbDragCanceled = FALSE;

        // +2, damit nicht so schnell die Tabs geloescht werden
        if ( nY > nOutHeight+2 )
            mbDragDelete = TRUE;

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
        maDragData = maData;
    else
        maData = maDragData;
    mpData = &maData;
    mbDrag = FALSE;

    // Handler rufen
    EndDrag();

    // Drag-Werte zuruecksetzen
    meDragType      = RULER_TYPE_DONTKNOW;
    mnDragPos       = 0;
    mnDragAryPos    = 0;
    mnDragSize      = 0;
    mbDragCanceled  = FALSE;
    mbDragDelete    = FALSE;
    mnDragModifier  = 0;
    mnDragScroll    = 0;
    mnStartDragPos  = 0;

    // Und neu ausgeben
    ImplDraw();
}

// -----------------------------------------------------------------------

IMPL_LINK( Ruler, ImplUpdateHdl, void*, EMPTYARG )
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
        USHORT  nMouseClicks = rMEvt.GetClicks();
        USHORT  nMouseModifier = rMEvt.GetModifier();

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
            mbDragCanceled = TRUE;
            mbFormat       = TRUE;
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

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    // Extra-Field beruecksichtigen
    if ( mnWinStyle & WB_EXTRAFIELD )
    {
        if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        {
            SetLineColor( rStyleSettings.GetShadowColor() );
            DrawLine( Point( maExtraRect.Left(), maExtraRect.Top() ),
                      Point( maExtraRect.Right()-1, maExtraRect.Top() ) );
            DrawLine( Point( maExtraRect.Left(), maExtraRect.Top() ),
                      Point( maExtraRect.Left(), maExtraRect.Bottom()-1 ) );
            DrawLine( Point( maExtraRect.Left(), maExtraRect.Bottom()-1 ),
                      Point( maExtraRect.Right()-1, maExtraRect.Bottom()-1 ) );
            DrawLine( Point( maExtraRect.Right()-1, maExtraRect.Top() ),
                      Point( maExtraRect.Right()-1, maExtraRect.Bottom()-1 ) );
            SetLineColor( rStyleSettings.GetLightColor() );
            DrawLine( Point( maExtraRect.Left()+1, maExtraRect.Top()+1 ),
                      Point( maExtraRect.Right()-2, maExtraRect.Top()+1 ) );
            DrawLine( Point( maExtraRect.Left()+1, maExtraRect.Top()+1 ),
                      Point( maExtraRect.Left()+1, maExtraRect.Bottom()-2 ) );
            DrawLine( Point( maExtraRect.Left(), maExtraRect.Bottom() ),
                      Point( maExtraRect.Right(), maExtraRect.Bottom() ) );
            DrawLine( Point( maExtraRect.Right(), maExtraRect.Top() ),
                      Point( maExtraRect.Right(), maExtraRect.Bottom() ) );
        }
        else
        {
            SetLineColor( rStyleSettings.GetWindowTextColor() );
            SetFillColor( rStyleSettings.GetWindowColor() );
            DrawRect( maExtraRect );
        }

        // Imhalt vom Extrafeld ausgeben
        ImplDrawExtra( TRUE );
    }

    if ( mnWinStyle & WB_BORDER )
    {
        if ( mnWinStyle & WB_HORZ )
        {
            if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            {
                SetLineColor( rStyleSettings.GetShadowColor() );
                DrawLine( Point( mnBorderOff, mnHeight-2 ),
                          Point( mnWidth, mnHeight-2 ) );
                if ( mnBorderOff )
                {
                    DrawLine( Point( mnBorderOff-1, mnHeight-2 ),
                              Point( mnBorderOff-1, mnHeight-1 ) );
                }
            }
            SetLineColor( rStyleSettings.GetWindowTextColor() );
            DrawLine( Point( mnBorderOff, mnHeight-1 ),
                      Point( mnWidth, mnHeight-1 ) );
        }
        else
        {
            if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
            {
                SetLineColor( rStyleSettings.GetShadowColor() );
                DrawLine( Point( mnWidth-2, mnBorderOff ),
                          Point( mnWidth-2, mnHeight ) );
                if ( mnBorderOff )
                {
                    DrawLine( Point( mnWidth-2, mnBorderOff-1 ),
                              Point( mnWidth-1, mnBorderOff-1 ) );
                }
            }
            SetLineColor( rStyleSettings.GetWindowTextColor() );
            DrawLine( Point( mnWidth-1, mnBorderOff ),
                      Point( mnWidth-1, mnHeight ) );
        }
    }
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
    BOOL bVisible = IsReallyVisible();
    if ( bVisible && mpData->nLines )
    {
        ImplInvertLines();
        mnUpdateFlags |= RULER_UPDATE_LINES;
        if ( !mnUpdateEvtId )
            mnUpdateEvtId = Application::PostUserEvent( LINK( this, Ruler, ImplUpdateHdl ), NULL );
    }
    mbFormat = TRUE;

    // Wenn sich die Hoehe bzw. Breite aendert, dann muessen besimmte Werte
    // neu berechnet werden
    if ( nNewHeight )
    {
        mbCalc = TRUE;
        mnVirHeight = nNewHeight - mnBorderWidth - (RULER_OFF*2);
        ImplInitExtraField( FALSE );
    }
    else
    {
        if ( mpData->bAutoPageWidth )
            ImplUpdate( TRUE );
        else if ( mbAutoWinWidth )
            mbCalc = TRUE;
    }

    // Wenn Ruler eine Groesse hat, dann Groesse vom VirtualDevice setzen
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
        ImplInitSettings( TRUE, FALSE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( FALSE, FALSE, TRUE );
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
        mbFormat = TRUE;
        ImplInitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

long Ruler::StartDrag()
{
    if ( maStartDragHdl.IsSet() )
        return maStartDragHdl.Call( this );
    else
        return FALSE;
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
    mbActive = TRUE;

    // Positionslinien wieder anzeigen (erst hinter mbActive=TRUE rufen, da
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
    // Positionslinien loeschen (schon vor mbActive=FALSE rufen, da
    // von ImplInvertLines() ausgewertet wird)
    ImplInvertLines();

    mbActive = FALSE;
}

// -----------------------------------------------------------------------

BOOL Ruler::StartDocDrag( const MouseEvent& rMEvt, RulerType eDragType )
{
    if ( !mbDrag )
    {
        Point               aMousePos = rMEvt.GetPosPixel();
        USHORT              nMouseClicks = rMEvt.GetClicks();
        USHORT              nMouseModifier = rMEvt.GetModifier();
        ImplRulerHitTest    aHitTest;

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

            return TRUE;
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

RulerType Ruler::GetDocType( const Point& rPos, RulerType eDragType,
                             USHORT* pAryPos ) const
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
    ImplDocHitTest( rPos, eDragType, &aHitTest );

    // Werte zurueckgeben
    if ( pAryPos )
        *pAryPos = aHitTest.nAryPos;
    return aHitTest.eType;
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

RulerType Ruler::GetType( const Point& rPos, USHORT* pAryPos ) const
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
        mbAutoWinWidth = TRUE;
    else
        mbAutoWinWidth = FALSE;

    // Werte setzen (werden in ImplFormat gegebenenfalls mitberechnet)
    mnWinOff = nNewOff;
    mnWinWidth = nNewWidth;
    ImplUpdate( TRUE );
}

// -----------------------------------------------------------------------

void Ruler::SetPagePos( long nNewOff, long nNewWidth )
{
    // Muessen wir ueberhaupt was machen
    if ( (mpData->nPageOff == nNewOff) && (mpData->nPageWidth == nNewWidth) )
        return;

    // Gegebenenfalls werden die Breiten automatisch berechnet
    if ( !nNewWidth )
        mpData->bAutoPageWidth = TRUE;
    else
        mpData->bAutoPageWidth = FALSE;

    // Werte setzen (werden in ImplFormat gegebenenfalls mitberechnet)
    mpData->nPageOff     = nNewOff;
    mpData->nPageWidth   = nNewWidth;
    ImplUpdate( TRUE );
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

#ifdef DBG_UTIL
            default:
                DBG_ERRORFILE( "Ruler::SetUnit() - Wrong Unit" );
                break;
#endif
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

void Ruler::SetExtraType( RulerExtra eNewExtraType, USHORT nStyle )
{
    if ( mnWinStyle & WB_EXTRAFIELD )
    {
        meExtraType  = eNewExtraType;
        mnExtraStyle = nStyle;
        if ( IsReallyVisible() && IsUpdateMode() )
            ImplDrawExtra( FALSE );
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

// -----------------------------------------------------------------------

void Ruler::SetMargin1( long nPos, USHORT nMarginStyle )
{
    if ( (mpData->nMargin1 != nPos) || (mpData->nMargin1Style != nMarginStyle) )
    {
        mpData->nMargin1      = nPos;
        mpData->nMargin1Style = nMarginStyle;
        ImplUpdate();
    }
}

// -----------------------------------------------------------------------

void Ruler::SetMargin2( long nPos, USHORT nMarginStyle )
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

void Ruler::SetLines( USHORT n, const RulerLine* pLineAry )
{
    // Testen, ob sich was geaendert hat
    if ( mpData->nLines == n )
    {
        USHORT           i = n;
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

    // Neue Werte setzen und neu ausgeben
    BOOL bMustUpdate;
    if ( IsReallyVisible() && IsUpdateMode() )
        bMustUpdate = TRUE;
    else
        bMustUpdate = FALSE;

    // Alte Linien loeschen
    if ( bMustUpdate )
        ImplInvertLines();

    // Neue Daten setzen
    if ( !n || !pLineAry )
    {
        if ( !mpData->pLines )
            return;
        delete mpData->pLines;
        mpData->nLines = 0;
        mpData->pLines = NULL;
    }
    else
    {
        if ( mpData->nLines != n )
        {
            if ( mpData->pLines )
                delete mpData->pLines;
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

void Ruler::SetArrows( USHORT n, const RulerArrow* pArrowAry )
{
    if ( !n || !pArrowAry )
    {
        if ( !mpData->pArrows )
            return;
        delete mpData->pArrows;
        mpData->nArrows = 0;
        mpData->pArrows = NULL;
    }
    else
    {
        if ( mpData->nArrows != n )
        {
            if ( mpData->pArrows )
                delete mpData->pArrows;
            mpData->nArrows = n;
            mpData->pArrows = new RulerArrow[n];
        }
        else
        {
            USHORT            i = n;
            const RulerArrow* pAry1 = mpData->pArrows;
            const RulerArrow* pAry2 = pArrowAry;
            while ( i )
            {
                if ( (pAry1->nPos      != pAry2->nPos)      ||
                     (pAry1->nWidth    != pAry2->nWidth)    ||
                     (pAry1->nLogWidth != pAry2->nLogWidth) ||
                     (pAry1->nStyle    != pAry2->nStyle) )
                    break;
                pAry1++;
                pAry2++;
                i--;
            }
            if ( !i )
                return;
        }

        memcpy( mpData->pArrows, pArrowAry, n*sizeof( RulerArrow ) );
    }

    ImplUpdate();
}

// -----------------------------------------------------------------------

void Ruler::SetBorders( USHORT n, const RulerBorder* pBrdAry )
{
    if ( !n || !pBrdAry )
    {
        if ( !mpData->pBorders )
            return;
        delete mpData->pBorders;
        mpData->nBorders = 0;
        mpData->pBorders = NULL;
    }
    else
    {
        if ( mpData->nBorders != n )
        {
            if ( mpData->pBorders )
                delete mpData->pBorders;
            mpData->nBorders = n;
            mpData->pBorders = new RulerBorder[n];
        }
        else
        {
            USHORT             i = n;
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

void Ruler::SetIndents( USHORT n, const RulerIndent* pIndentAry )
{
    DBG_ASSERT( mnWinStyle & WB_HORZ,
                "Ruler::SetIndents() not allowed when WB_VERT" );

    if ( !n || !pIndentAry )
    {
        if ( !mpData->pIndents )
            return;
        delete mpData->pIndents;
        mpData->nIndents = 0;
        mpData->pIndents = NULL;
    }
    else
    {
        if ( mpData->nIndents != n )
        {
            if ( mpData->pIndents )
                delete mpData->pIndents;
            mpData->nIndents = n;
            mpData->pIndents = new RulerIndent[n];
        }
        else
        {
            USHORT             i = n;
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

void Ruler::SetTabs( USHORT n, const RulerTab* pTabAry )
{
    DBG_ASSERT( mnWinStyle & WB_HORZ,
                "Ruler::SetTabs() not allowed when WB_VERT" );

    if ( !n || !pTabAry )
    {
        if ( !mpData->pTabs )
            return;
        delete mpData->pTabs;
        mpData->nTabs = 0;
        mpData->pTabs = NULL;
    }
    else
    {
        if ( mpData->nTabs != n )
        {
            if ( mpData->pTabs )
                delete mpData->pTabs;
            mpData->nTabs = n;
            mpData->pTabs = new RulerTab[n];
        }
        else
        {
            USHORT          i = n;
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
        ImplInitExtraField( TRUE );
    }
}

// -----------------------------------------------------------------------

void Ruler::DrawTab( OutputDevice* pDevice, const Point& rPos, USHORT nStyle )
{
    const StyleSettings&    rStyleSettings = pDevice->GetSettings().GetStyleSettings();
    Point                   aPos( rPos );
    USHORT                  nTabStyle = nStyle & RULER_TAB_STYLE;

    pDevice->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
    pDevice->SetLineColor();
    pDevice->SetFillColor( pDevice->GetSettings().GetStyleSettings().GetWindowTextColor() );
    ImplCenterTabPos( aPos, nTabStyle );
    ImplDrawRulerTab( pDevice, aPos, nTabStyle );
    pDevice->Pop();
}
