/*************************************************************************
 *
 *  $RCSfile: frmsel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:08 $
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

// include ---------------------------------------------------------------

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#pragma hdrstop

#define _SVX_FRMSEL_CXX
#define ITEMID_BOX          SID_ATTR_BORDER_OUTER
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#include "frmsel.hxx"
#include "linelink.hxx"

#ifndef _SVX_BOXITEM_HXX //autogen
#include <boxitem.hxx>
#endif

// class SvxFrameLine ----------------------------------------------------

struct SvxLineStruct SvxFrameLine::NO_LINE           = { 0,0,0 };
struct SvxLineStruct SvxFrameLine::THIN_LINE         = { 1,0,0 };
struct SvxLineStruct SvxFrameLine::THICK_LINE        = { 3,0,0 };
struct SvxLineStruct SvxFrameLine::THIN_DOUBLE_LINE  = { 1,1,1 };
struct SvxLineStruct SvxFrameLine::THICK_DOUBLE_LINE = { 2,1,2 };

//------------------------------------------------------------------------

SvxFrameLine::SvxFrameLine()
    :   aStartPos    ( 0, 0 ),
        aMidPos      ( 0, 0 ),
        aEndPos      ( 0, 0 ),
        aColor       ( COL_BLACK ),
        theState     ( SVX_FRMLINESTATE_HIDE ),
#ifndef MIPS
        theStyle     ( NO_LINE ),
        theCoreStyle ( NO_LINE ),
#endif
        bIsSelected  ( FALSE )
{
#if defined MIPS
    theStyle.nLeft=0;
    theStyle.nMiddle=0;
    theStyle.nRight=0;
    theCoreStyle.nLeft=0;
    theCoreStyle.nMiddle=0;
    theCoreStyle.nRight=0;
#endif
}

//------------------------------------------------------------------------

#define WIDTH_THICK     (DEF_LINE_WIDTH_2 *100)

void SvxFrameLine::SetStyle( const SvxLineStruct& aStyle )
{
    theCoreStyle = aStyle;
    /*
     * Mapping: (in TWIPS, 1pt = 100 * 20 TWIPS = duenn)
     * alles was <=0                    ist -> NO_LINE
     * alles was <=DEF_SLINE_WIDTH_2    ist -> THIN_LINE
     * alles andere                         -> THICK_LINE
     */
    if ( ( aStyle.nLeft <= 0 ) && ( aStyle.nRight <= 0 ) )
    {
        theStyle = NO_LINE;
        theState = SVX_FRMLINESTATE_HIDE;
    }
    else if ( ( aStyle.nRight == 0 ) &&             // einzelne Linie
              ( aStyle.nLeft  >  0 ) &&             // sichtbar
              ( aStyle.nLeft  < WIDTH_THICK ) )     // duenn
    {
        theStyle = THIN_LINE;
        theState = SVX_FRMLINESTATE_SHOW;
    }
    else if ( ( aStyle.nRight == 0 ) &&             // einzelne Linie
              ( aStyle.nLeft   > 0 ) &&             // sichtbar
              ( aStyle.nLeft  >= WIDTH_THICK ) )    // dick
    {
        theStyle = THICK_LINE;
        theState = SVX_FRMLINESTATE_SHOW;
    }
    else if ( ( aStyle.nRight > 0 ) &&              // doppelte Linie
              ( aStyle.nLeft  > 0 ) &&              // sichtbar
              ( aStyle.nLeft  < WIDTH_THICK ) )     // duenn
    {
        theStyle = THIN_DOUBLE_LINE;
        theState = SVX_FRMLINESTATE_SHOW;
    }
    else if ( ( aStyle.nRight > 0 ) &&              // doppelte Linie
              ( aStyle.nLeft  > 0 ) &&              // sichtbar
              ( aStyle.nLeft  >= WIDTH_THICK ) )    // dick
    {
        theStyle = THICK_DOUBLE_LINE;
        theState = SVX_FRMLINESTATE_SHOW;
    }
}

#undef WIDTH_THICK

//------------------------------------------------------------------------

void SvxFrameLine::SetState( SvxFrameLineState eState )
{
    theState = eState;

    if ( SVX_FRMLINESTATE_DONT_CARE == eState )
    {
        theStyle     = THICK_LINE;
        theCoreStyle = NO_LINE;
    }
}

// class SvxFrameSelector ------------------------------------------------

SvxFrameSelector::SvxFrameSelector( Window* pParent,
                                    SvxFrameSelectorType eType,
                                    BOOL bDontCare )
    :   Window          ( pParent, (WinBits)0x0000 ),
        eSel            ( eType ),
        eShadow         ( SVX_FRMSHADOW_NONE ),
        theCurLineStyle ( SvxFrameLine::NO_LINE ),
        theCurLineCol   ( COL_BLACK ),
        theShadowCol    ( COL_BLACK ),
        bIsDontCare     ( bDontCare ),
        bIsClicked      ( FALSE )
{
    Size aSzBmp( ( eSel == SVX_FRMSELTYPE_TABLE )
                    ? Size( 60, 42 )
                    : Size( 50, 50 ) );
    aSzBmp = pParent->LogicToPixel(aSzBmp, MAP_APPFONT);
    Size aSzParent( pParent->GetSizePixel() );
    long nXCenter = ( aSzParent.Width()  - aSzBmp.Width()  ) / 2;
    long nYCenter = ( aSzParent.Height() - aSzBmp.Height() ) / 2;
    Point atPos( ( nXCenter < 0 ) ? 0 : nXCenter,
                 ( nYCenter < 0 ) ? 0 : nYCenter );

    InitBitmap_Impl( aSzBmp );
    SetPosSizePixel( atPos, theBmp.GetSizePixel() );
    Show();
}

// -----------------------------------------------------------------------

void SvxFrameSelector::InitBitmap_Impl( const Size& rSize )
{
    VirtualDevice   aVirDev;
    long            nX  = rSize.Width();
    long            nY  = rSize.Height();
    long            nXMid = nX / 2;
    long            nYMid = nY / 2;

    /*
     * Berechnen des Frame-Rects und der Linie-Mittelpunkte
     */
    theBoundingRect = aRectFrame = Rectangle( 21, 21, nX-21, nY-21 );

    theLeftLine.aStartPos   = Point( 21, 21 );
    theLeftLine.aEndPos     = Point( 21, nY - 21 );
    theLeftLine.aMidPos     = Point( 21, nYMid );

    theRightLine.aStartPos  = Point( nX - 21, 21 );
    theRightLine.aEndPos    = Point( nX - 21, nY - 21 );
    theRightLine.aMidPos    = Point( nX - 21, nYMid );

    theVerLine.aStartPos    = Point( nXMid, 21 );
    theVerLine.aEndPos      = Point( nXMid, nY - 21 );
    theVerLine.aMidPos      = Point( nXMid, nYMid );

    theTopLine.aStartPos    = theLeftLine.aStartPos;
    theTopLine.aEndPos      = theRightLine.aStartPos;
    theTopLine.aMidPos      = theVerLine.aStartPos;

    theBottomLine.aStartPos = theLeftLine.aEndPos;
    theBottomLine.aEndPos   = theRightLine.aEndPos;
    theBottomLine.aMidPos   = theVerLine.aEndPos;

    theHorLine.aStartPos    = theLeftLine.aMidPos;
    theHorLine.aEndPos      = theRightLine.aMidPos;
    theHorLine.aMidPos      = theVerLine.aMidPos;

    // HotSpot-Rectangles:
    aSpotLeft   = Rectangle( Point( 0, 0 ),
                             Size( theLeftLine.aStartPos.X() + 3,
                                   rSize.Height() ) );
    aSpotRight  = Rectangle( Point( theRightLine.aStartPos.X() - 3, 0 ),
                             aSpotLeft.GetSize() );
    aSpotTop        = Rectangle( Point( theTopLine.aStartPos.X(), 0 ),
                             Size( aRectFrame.GetWidth(),
                                   theTopLine.aStartPos.Y() + 3 ) );
    aSpotBottom = Rectangle( theBottomLine.aStartPos - Point( 0, 3 ),
                             aSpotTop.GetSize() );
    aSpotHor        = ( eSel == SVX_FRMSELTYPE_TABLE )
                  ? Rectangle( theHorLine.aStartPos + Point( 0, -2 ),
                               Size( aRectFrame.GetWidth(), 5 ) )
                  : Rectangle( Point( -1, -1 ), Size(- 1, -1 ) );
    aSpotVer        = ( eSel == SVX_FRMSELTYPE_TABLE )
                  ? Rectangle( theVerLine.aStartPos + Point( -2, 0 ),
                               Size( 5, aRectFrame.GetHeight() ) )
                  : Rectangle( Point( -1, -1 ), Size( -1, -1 ) );

    /*
     * Initialisieren der Bitmap:
     */
    aVirDev.SetOutputSizePixel( rSize );
    aVirDev.SetLineColor( Color( COL_BLACK ) );
    aVirDev.SetFillColor( Color( COL_WHITE ) );
    aVirDev.DrawRect( Rectangle( Point( 0, 0 ), rSize ) );

    DrawContents_Impl( aVirDev );

    /*
     * Malen der vier Eck-Winkel:
     */
    aVirDev.SetLineColor( Color( COL_GRAY ) );

    // links/rechts oben:
    aVirDev.DrawLine( Point( 10, 15 ), Point( 15, 15 ) );
    aVirDev.DrawLine( Point( 15, 15 ), Point( 15, 10 ) );
    aVirDev.DrawLine( Point( nX - 10, 15 ), Point( nX - 15, 15 ) );
    aVirDev.DrawLine( Point( nX - 15, 15 ), Point( nX - 15, 10 ) );

    // links/rechts unten:
    aVirDev.DrawLine( Point( 10, nY - 15 ), Point( 15, nY - 15 ) );
    aVirDev.DrawLine( Point( 15, nY - 15 ), Point( 15, nY - 10 ) );
    aVirDev.DrawLine( Point( nX -10, nY - 15 ), Point( nX - 15, nY - 15 ) );
    aVirDev.DrawLine( Point( nX -15, nY - 15 ), Point( nX - 15, nY - 10 ) );

    /*
     * Tabelle: Malen der vier Mitte-Winkel:
     */
    if ( eSel == SVX_FRMSELTYPE_TABLE )
    {
        // links/rechts
        aVirDev.DrawLine( Point( 10, nYMid ),       Point( 15, nYMid ) );
        aVirDev.DrawLine( Point( 15, nYMid-2 ),     Point( 15, nYMid+2 ) );
        aVirDev.DrawLine( Point( nX-10, nYMid ),    Point( nX-15, nYMid ) );
        aVirDev.DrawLine( Point( nX-15, nYMid-2 ),  Point( nX-15, nYMid+2 ) );

        // horizontal/vertikal
        aVirDev.DrawLine( Point( nXMid, 10 ),       Point( nXMid, 15 ) );
        aVirDev.DrawLine( Point( nXMid-2, 15 ),     Point( nXMid+2, 15 ) );
        aVirDev.DrawLine( Point( nXMid, nY-10 ),    Point( nXMid, nY-15 ) );
        aVirDev.DrawLine( Point( nXMid-2, nY-15 ),  Point( nXMid+2, nY-15 ) );
    }
    theBmp = aVirDev.GetBitmap( Point( 0, 0 ), rSize );
}

// -----------------------------------------------------------------------

void SvxFrameSelector::DrawFrameLine_Impl( OutputDevice& rVirDev, Point from,
    Point to, BOOL bHor, SvxLineStruct& dline, SvxLineStruct& lt,
    SvxLineStruct& lLine, SvxLineStruct& lb, SvxLineStruct& rt,
    SvxLineStruct& rLine, SvxLineStruct& rb )
{
    short diffArr[4];       // Kreuzungspunkt-Differenzen
    short nHeight = 0;
    Point from2( from );
    Point to2( to );

    if ( bHor ) // horizontale Linien
    {
        // Berechnung der Differenzen der Linienlaenge
        // in den Kreuzungspunkten:
        LinkLine( dline, lt, lLine, lb, rt, rLine, rb, diffArr );
        nHeight = dline.nLeft + dline.nMiddle + dline.nRight;

        from.Y() -= nHeight/2;
        to.Y()   -= nHeight/2;
        to.Y()   += ( dline.nLeft - 1 );

        from.X() += diffArr[0];
        to.X()   += diffArr[2];

        rVirDev.DrawRect( Rectangle( from, to ) );

        // noch eine zweite Linie zu malen?
        if ( dline.nRight != 0 )
        {
            from2.Y() -= nHeight / 2;
            from2.Y() += dline.nLeft + dline.nMiddle;
            to2.Y()   -= nHeight / 2;
            to2.Y()   += dline.nMiddle + dline.nLeft;
            to2.Y()   += ( dline.nRight - 1 );
            from2.X() += diffArr[1];
            to2.X()   += diffArr[3];
            rVirDev.DrawRect( Rectangle( from2, to2 ) );
        }
    }
    else // vertikale Linien
    {
        // Berechnung der Differenzen der Linienlaenge
        // in den Kreuzungspunkten:
        LinkLine( dline, lt, lLine, lb, rt, rLine, rb, diffArr );
        nHeight = dline.nLeft + dline.nMiddle + dline.nRight;

        from.X() -= nHeight / 2;
        to.X()   -= nHeight / 2;
        to.X()   += ( dline.nLeft - 1 );
        from.Y() -= diffArr[2];
        to.Y()   -= diffArr[0];
        rVirDev.DrawRect( Rectangle( from, to ) );

        // noch eine zweite Linie zu malen?
        if ( dline.nRight != 0 )
        {
            from2.X() -= nHeight / 2;
            from2.X() += dline.nLeft + dline.nMiddle;
            to2.X()   -= nHeight / 2;
            to2.X()   += dline.nMiddle + dline.nLeft;
            to2.X()   += ( dline.nRight - 1 );
            from2.Y() -= diffArr[3];
            to2.Y()   -= diffArr[1];
            rVirDev.DrawRect( Rectangle( from2, to2 ) );
        }
    }

    /*
     * Anpassen des umgebenden Rectangles:
     */
    long x1 = Min( from.X(), from2.X() );
    long x2 = Max( to.X(),   to2.X()   );
    long y1 = Min( from.Y(), from2.Y() );
    long y2 = Max( to.Y(),   to2.Y()   );

    if ( x1 < theBoundingRect.Left() )
        theBoundingRect.Left() = x1;
    else if ( x1 > theBoundingRect.Right() )
        theBoundingRect.Right() = x1;

    if ( x2 < theBoundingRect.Left() )
        theBoundingRect.Left() = x2;
    else if ( x2 > theBoundingRect.Right() )
        theBoundingRect.Right() = x2;

    if ( y1 < theBoundingRect.Top() )
        theBoundingRect.Top() = y1;
    else if ( y1 > theBoundingRect.Bottom() )
        theBoundingRect.Bottom() = y1;

    if ( y2 < theBoundingRect.Top() )
        theBoundingRect.Top() = y2;
    else if ( y2 > theBoundingRect.Bottom() )
        theBoundingRect.Bottom() = y2;
}

// class SvxFrameSelector ------------------------------------------------

void SvxFrameSelector::HideLines()
{
    theLeftLine.theState    =
    theRightLine.theState   =
    theTopLine.theState     =
    theBottomLine.theState  =
    theHorLine.theState     =
    theVerLine.theState     = SVX_FRMLINESTATE_HIDE;
    theLeftLine.theStyle    =
    theRightLine.theStyle   =
    theTopLine.theStyle     =
    theBottomLine.theStyle  =
    theHorLine.theStyle     =
    theHorLine.theStyle     =
    theVerLine.theStyle     = SvxFrameLine::NO_LINE;

    ShowLines();
}

// -----------------------------------------------------------------------

void SvxFrameSelector::ShowLines()
{
    VirtualDevice   aVirDev;
    Color           aFillColor;

    aVirDev.SetOutputSizePixel( theBmp.GetSizePixel() );
    aVirDev.DrawBitmap( Point( 0, 0 ), theBmp );

    // gesamten Rahmen loeschen und Inhalt malen
    aVirDev.SetLineColor();
    aVirDev.SetFillColor( Color( COL_WHITE ) );
    aVirDev.DrawRect( theBoundingRect );
    DrawContents_Impl( aVirDev );
    theBoundingRect = aRectFrame;

    /*
     * Zeichnen aller Linien:
     * ----------------------
     * Eine Linie wird in zwei Abschnitten gemalt, um die Kreuzungs-
     * punkte mit anderen Linien beruecksichtigen zu koennen.
     * Diese Aufgabe uebernimmt die Methode DrawFrameLine_Impl().
     */

    // innere Linien: ---------------------------------------------

    if ( eSel == SVX_FRMSELTYPE_TABLE &&
         ( theVerLine.theState == SVX_FRMLINESTATE_SHOW ||
           theVerLine.theState == SVX_FRMLINESTATE_DONT_CARE ) )
    {
        if ( theVerLine.theState == SVX_FRMLINESTATE_SHOW )
            aFillColor = theVerLine.aColor;
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            theVerLine.aStartPos,
            theVerLine.aMidPos,
            FALSE,
            theVerLine.theStyle,    // dline
            theHorLine.theStyle,    // rb
            theVerLine.theStyle,    // lLine
            theHorLine.theStyle,    // rt
            theTopLine.theStyle,    // lb
            SvxFrameLine::NO_LINE,  // rLine
            theTopLine.theStyle     // lt
        );

        DrawFrameLine_Impl( aVirDev,
            theVerLine.aMidPos,
            theVerLine.aEndPos,
            FALSE,
            theVerLine.theStyle,    // dline
            theBottomLine.theStyle, // rb
            SvxFrameLine::NO_LINE,  // lLine
            theBottomLine.theStyle, // rb
            theHorLine.theStyle,    // lb
            theVerLine.theStyle,    // rLine
            theHorLine.theStyle     // lt
        );
    }

    if ( eSel == SVX_FRMSELTYPE_TABLE &&
         ( theHorLine.theState == SVX_FRMLINESTATE_SHOW ||
           theHorLine.theState == SVX_FRMLINESTATE_DONT_CARE ) )
    {
        if ( theHorLine.theState == SVX_FRMLINESTATE_SHOW )
            aFillColor = theHorLine.aColor;
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            theHorLine.aStartPos,
            theHorLine.aMidPos,
            TRUE,
            theHorLine.theStyle,    // dline
            theLeftLine.theStyle,   // lt
            SvxFrameLine::NO_LINE,  // lLine
            theLeftLine.theStyle,   // lb
            theVerLine.theStyle,    // rt
            theHorLine.theStyle,    // rLine
            theVerLine.theStyle     // rb
        );

        DrawFrameLine_Impl( aVirDev,
            theHorLine.aMidPos,
            theHorLine.aEndPos,
            TRUE,
            theHorLine.theStyle,    // dline
            theVerLine.theStyle,    // lt
            theHorLine.theStyle,    // lLine
            theVerLine.theStyle,    // lb
            theRightLine.theStyle,  // rt
            SvxFrameLine::NO_LINE,  // rLine
            theRightLine.theStyle   // rb
        );
    }

    // aeussere Linien: -------------------------------------------

    if ( theLeftLine.theState == SVX_FRMLINESTATE_SHOW ||
         theLeftLine.theState == SVX_FRMLINESTATE_DONT_CARE )
    {
        if ( theLeftLine.theState == SVX_FRMLINESTATE_SHOW )
            aFillColor = theLeftLine.aColor;
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            theLeftLine.aStartPos,
            theLeftLine.aMidPos,
            FALSE,
            theLeftLine.theStyle,   // dline
            SvxFrameLine::NO_LINE,  // lb
            theLeftLine.theStyle,   // lLine
            theHorLine.theStyle,    // lt
            SvxFrameLine::NO_LINE,  // rb
            SvxFrameLine::NO_LINE,  // rLine
            theTopLine.theStyle     // rt
        );

        DrawFrameLine_Impl( aVirDev,
            theLeftLine.aMidPos,
            theLeftLine.aEndPos,
            FALSE,
            theLeftLine.theStyle,   // dline
            SvxFrameLine::NO_LINE,  // lb
            SvxFrameLine::NO_LINE,  // lLine
            theBottomLine.theStyle, // lt
            SvxFrameLine::NO_LINE,  // rb
            theLeftLine.theStyle,   // rLine
            theHorLine.theStyle     // rt
        );
    }

    if ( theRightLine.theState == SVX_FRMLINESTATE_SHOW ||
         theRightLine.theState == SVX_FRMLINESTATE_DONT_CARE )
    {
        if ( theRightLine.theState == SVX_FRMLINESTATE_SHOW )
            aFillColor = theRightLine.aColor;
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            theRightLine.aStartPos,
            theRightLine.aMidPos,
            FALSE,
            theRightLine.theStyle,  // dline
            theHorLine.theStyle,    // rb
            theRightLine.theStyle,  // lLine
            SvxFrameLine::NO_LINE,  // rt
            theTopLine.theStyle,    // lb
            SvxFrameLine::NO_LINE,  // eLine
            SvxFrameLine::NO_LINE   // lt
        );

        DrawFrameLine_Impl( aVirDev,
            theRightLine.aMidPos,
            theRightLine.aEndPos,
            FALSE,
            theRightLine.theStyle,  // dline
            theBottomLine.theStyle, // rb
            SvxFrameLine::NO_LINE,  // lLine
            SvxFrameLine::NO_LINE,  // lt
            theHorLine.theStyle,    // lb
            theRightLine.theStyle,  // lLine
            SvxFrameLine::NO_LINE   // rt
        );
    }

    if ( theTopLine.theState == SVX_FRMLINESTATE_SHOW ||
         theTopLine.theState == SVX_FRMLINESTATE_DONT_CARE )
    {
        if ( theTopLine.theState == SVX_FRMLINESTATE_SHOW )
            aFillColor = theTopLine.aColor;
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            theTopLine.aStartPos,
            theTopLine.aMidPos,
            TRUE,
            theTopLine.theStyle,    // dline
            SvxFrameLine::NO_LINE,  // lt
            SvxFrameLine::NO_LINE,  // lLine
            theLeftLine.theStyle,   // lb
            SvxFrameLine::NO_LINE,  // rt
            theTopLine.theStyle,    // rLine
            theVerLine.theStyle     // rb
        );

        DrawFrameLine_Impl( aVirDev,
            theTopLine.aMidPos,
            theTopLine.aEndPos,
            TRUE,
            theTopLine.theStyle,    // dline
            SvxFrameLine::NO_LINE,  // lt
            theTopLine.theStyle,    // lLine
            theVerLine.theStyle,    // lb
            SvxFrameLine::NO_LINE,  // rt
            SvxFrameLine::NO_LINE,  // rLine
            theRightLine.theStyle   // rb
        );
    }

    if ( theBottomLine.theState == SVX_FRMLINESTATE_SHOW ||
         theBottomLine.theState == SVX_FRMLINESTATE_DONT_CARE )
    {
        if ( theBottomLine.theState == SVX_FRMLINESTATE_SHOW )
            aFillColor = theBottomLine.aColor;
        else
            aFillColor = Color( COL_LIGHTGRAY );

        aVirDev.SetLineColor();
        aVirDev.SetFillColor( aFillColor );

        DrawFrameLine_Impl( aVirDev,
            theBottomLine.aStartPos,
            theBottomLine.aMidPos,
            TRUE,
            theBottomLine.theStyle, // dline
            theLeftLine.theStyle,   // lt
            SvxFrameLine::NO_LINE,  // lLine
            SvxFrameLine::NO_LINE,  // lb
            theVerLine.theStyle,    // rt
            theBottomLine.theStyle, // rLine
            SvxFrameLine::NO_LINE   // rb
        );

        DrawFrameLine_Impl( aVirDev,
            theBottomLine.aMidPos,
            theBottomLine.aEndPos,
            TRUE,
            theBottomLine.theStyle, // dline
            theVerLine.theStyle,    // lt
            theBottomLine.theStyle, // lLine
            SvxFrameLine::NO_LINE,  // lb
            theRightLine.theStyle,  // rt
            SvxFrameLine::NO_LINE,  // rLine
            SvxFrameLine::NO_LINE   // rb
        );
    }

    theBmp = aVirDev.GetBitmap( Point(0,0), theBmp.GetSizePixel() );

    if ( eShadow != SVX_FRMSHADOW_NONE )
        ShowShadow();
    else
        Invalidate( INVALIDATE_NOERASE );
}

// -----------------------------------------------------------------------

void SvxFrameSelector::SetCurLineStyle( const SvxLineStruct& aStyle )
{
    theCurLineStyle = aStyle;

    if ( theLeftLine.bIsSelected )      theLeftLine.SetStyle( aStyle );
    if ( theRightLine.bIsSelected )     theRightLine.SetStyle( aStyle );
    if ( theTopLine.bIsSelected )       theTopLine.SetStyle( aStyle );
    if ( theBottomLine.bIsSelected )    theBottomLine.SetStyle( aStyle );
    if ( theHorLine.bIsSelected )       theHorLine.SetStyle( aStyle );
    if ( theVerLine.bIsSelected )       theVerLine.SetStyle( aStyle );
}

//------------------------------------------------------------------------

void SvxFrameSelector::SetCurLineColor( const Color& aColor )
{
    theCurLineCol = aColor;

    if ( theLeftLine.bIsSelected )      theLeftLine.aColor   = aColor;
    if ( theRightLine.bIsSelected )     theRightLine.aColor  = aColor;
    if ( theTopLine.bIsSelected )       theTopLine.aColor    = aColor;
    if ( theBottomLine.bIsSelected )    theBottomLine.aColor = aColor;
    if ( theHorLine.bIsSelected )       theHorLine.aColor    = aColor;
    if ( theVerLine.bIsSelected )       theVerLine.aColor    = aColor;
}

//------------------------------------------------------------------------

void SvxFrameSelector::DrawContents_Impl( OutputDevice& rVirDev )
{
    /*
     * Malen des Inhaltes:
     */
    if ( eSel == SVX_FRMSELTYPE_TABLE )
    {
        Color aPrevLineColor = rVirDev.GetLineColor();
        rVirDev.SetLineColor( Color( COL_LIGHTGRAY ) );
        Color aPrevFillColor = rVirDev.GetFillColor();
        rVirDev.SetFillColor( Color( COL_LIGHTGRAY ) );

        Size aContentsSize =
            Size( theVerLine.aStartPos.X() - theLeftLine.aStartPos.X() - 8,
                  theHorLine.aStartPos.Y() - theTopLine.aStartPos.Y() - 8 );

        // links-oben
        rVirDev.DrawRect( Rectangle( theLeftLine.aStartPos + Point( 4, 4 ),
                                     aContentsSize ) );
        // rechts-oben
        rVirDev.DrawRect( Rectangle( theVerLine.aStartPos + Point( 4, 4 ),
                                     aContentsSize ) );
        // links-unten
        rVirDev.DrawRect( Rectangle( theHorLine.aStartPos + Point( 4, 4 ),
                                     aContentsSize ) );
        // rechts-unten
        rVirDev.DrawRect( Rectangle( theHorLine.aMidPos + Point( 4, 4 ),
                                     aContentsSize ) );

        rVirDev.SetLineColor( aPrevLineColor );
        rVirDev.SetFillColor( aPrevFillColor);
    }
    else
    {
        Color aPrevLineColor = rVirDev.GetLineColor();
        rVirDev.SetLineColor( Color( COL_LIGHTGRAY ) );
        Color aPrevFillColor = rVirDev.GetFillColor();
        rVirDev.SetFillColor( Color( COL_LIGHTGRAY ) );
        Size aContentsSize =
            Size( theRightLine.aStartPos.X() - theLeftLine.aStartPos.X() - 8 ,
                  theBottomLine.aStartPos.Y() - theLeftLine.aStartPos.Y() - 8 );

        rVirDev.DrawRect( Rectangle( theLeftLine.aStartPos + Point( 4, 4 ),
                                     aContentsSize ) );

    /* symbolisierter Paragraph ist nicht gewuenscht - vielleicht spaeter mal wieder...
        Size aContentsSize =
            Size( theRightLine.aStartPos.X() - theLeftLine.aStartPos.X() - 8,
                  theBottomLine.aStartPos.Y() - theLeftLine.aStartPos.Y() - 13 );

        rVirDev.DrawRect( Rectangle( theLeftLine.aStartPos + Point( 4, 4 ),
                                     aContentsSize ) );
        rVirDev.DrawRect( Rectangle( theLeftLine.aEndPos + Point( 4, -13 ),
                                     Size( 20, 8 ) ) );
    */
        /* -------------------------------------------------------------------
            kann StarWriter noch nicht - vielleicht spaeter mal:
            Size    aContentsSize =
                    Size(  theRightLine.aStartPos.X()
                         - theLeftLine.aStartPos.X()
                         - 8,
                           theHorLine.aStartPos.Y()
                         - theLeftLine.aStartPos.Y()
                         - 13 );

            // oben
            rVirDev.DrawRect( Rectangle( theLeftLine.aStartPos + Point(4,4),
                                         aContentsSize ) );
            rVirDev.DrawRect( Rectangle( theLeftLine.aMidPos + Point(4,-13),
                                         Size(20,8) ) );
            // unten
            rVirDev.DrawRect( Rectangle( theLeftLine.aMidPos + Point(4,4),
                                         aContentsSize ) );
            rVirDev.DrawRect( Rectangle( theLeftLine.aEndPos + Point(4,-13),
                                         Size(20,8) ) );
        --------------------------------------------------------------------*/
    }
}

//------------------------------------------------------------------------

void SvxFrameSelector::SelectLine( SvxFrameSelectorLine eNewLine, BOOL bSet )
{
    VirtualDevice aVirDev;
    Color aDrawColor(
        ( bSet && ( eNewLine != SVX_FRMSELLINE_NONE ) ) ? COL_BLACK : COL_WHITE );
    long nX    = theBmp.GetSizePixel().Width();
    long nY    = theBmp.GetSizePixel().Height();
    long nXMid = theVerLine.aStartPos.X();
    long nYMid = theHorLine.aStartPos.Y();

    aVirDev.SetOutputSizePixel( theBmp.GetSizePixel() );
    aVirDev.DrawBitmap( Point( 0, 0 ), theBmp );

    switch ( eNewLine )
    {
        case SVX_FRMSELLINE_LEFT:
        {
            DrawSelArrow_Impl( aVirDev, Point( 13, 3 ), aDrawColor,
                          SVX_SELARROWTYPE_DOWN );
            DrawSelArrow_Impl( aVirDev, Point( 13, nY - 8 ), aDrawColor,
                          SVX_SELARROWTYPE_UP );
            theLeftLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_RIGHT:
        {
            DrawSelArrow_Impl( aVirDev, Point( nX - 17, 3 ), aDrawColor,
                          SVX_SELARROWTYPE_DOWN );
            DrawSelArrow_Impl( aVirDev, Point( nX - 17, nY - 8 ), aDrawColor,
                          SVX_SELARROWTYPE_UP );
            theRightLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_TOP:
        {
            DrawSelArrow_Impl( aVirDev, Point( 3, 13 ), aDrawColor,
                          SVX_SELARROWTYPE_RIGHT );
            DrawSelArrow_Impl( aVirDev, Point( nX - 8, 13 ), aDrawColor,
                          SVX_SELARROWTYPE_LEFT );
            theTopLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_BOTTOM:
        {
            DrawSelArrow_Impl( aVirDev, Point( 3, nY - 17 ), aDrawColor,
                          SVX_SELARROWTYPE_RIGHT );
            DrawSelArrow_Impl( aVirDev, Point( nX - 8, nY - 17 ), aDrawColor,
                          SVX_SELARROWTYPE_LEFT );
            theBottomLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_HOR:
        if ( eSel == SVX_FRMSELTYPE_TABLE )
        {
            DrawSelArrow_Impl( aVirDev, Point( 3, nYMid - 2 ), aDrawColor,
                          SVX_SELARROWTYPE_RIGHT );
            DrawSelArrow_Impl( aVirDev, Point( nX - 8, nYMid - 2 ), aDrawColor,
                          SVX_SELARROWTYPE_LEFT );
            theHorLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_VER:
        if ( eSel == SVX_FRMSELTYPE_TABLE )
        {
            DrawSelArrow_Impl( aVirDev, Point( nXMid - 2, 3 ), aDrawColor,
                          SVX_SELARROWTYPE_DOWN);
            DrawSelArrow_Impl( aVirDev, Point( nXMid - 2, nY - 8 ), aDrawColor,
                          SVX_SELARROWTYPE_UP );
            theVerLine.bIsSelected = bSet;
        }
        break;

        case SVX_FRMSELLINE_NONE:
        {
            // links
            DrawSelArrow_Impl( aVirDev, Point( 13, 3 ), aDrawColor,
                          SVX_SELARROWTYPE_DOWN );
            DrawSelArrow_Impl( aVirDev, Point( 13, nY - 8 ), aDrawColor,
                          SVX_SELARROWTYPE_UP );
            // rechts
            DrawSelArrow_Impl( aVirDev, Point( nX - 17, 3 ), aDrawColor,
                          SVX_SELARROWTYPE_DOWN );
            DrawSelArrow_Impl( aVirDev, Point( nX - 17, nY - 8 ), aDrawColor,
                          SVX_SELARROWTYPE_UP );
            // oben
            DrawSelArrow_Impl( aVirDev, Point( 3, 13 ), aDrawColor,
                          SVX_SELARROWTYPE_RIGHT );
            DrawSelArrow_Impl( aVirDev, Point( nX - 8, 13 ), aDrawColor,
                          SVX_SELARROWTYPE_LEFT );
            // unten
            DrawSelArrow_Impl( aVirDev, Point( 3, nY - 17 ), aDrawColor,
                          SVX_SELARROWTYPE_RIGHT );
            DrawSelArrow_Impl( aVirDev, Point( nX - 8, nY - 17 ), aDrawColor,
                          SVX_SELARROWTYPE_LEFT );

            if ( eSel == SVX_FRMSELTYPE_TABLE )
            {
                // horizontal
                DrawSelArrow_Impl( aVirDev, Point( 3, nYMid-2 ), aDrawColor,
                              SVX_SELARROWTYPE_RIGHT );
                DrawSelArrow_Impl( aVirDev, Point( nX - 8, nYMid - 2 ), aDrawColor,
                              SVX_SELARROWTYPE_LEFT );

                // vertikal
                DrawSelArrow_Impl( aVirDev, Point( nXMid - 2, 3 ),aDrawColor,
                              SVX_SELARROWTYPE_DOWN );
                DrawSelArrow_Impl( aVirDev, Point( nXMid - 2, nY - 8 ), aDrawColor,
                              SVX_SELARROWTYPE_UP );
            }

            theLeftLine.bIsSelected     =
            theRightLine.bIsSelected    =
            theTopLine.bIsSelected      =
            theBottomLine.bIsSelected   =
            theHorLine.bIsSelected      =
            theVerLine.bIsSelected      = FALSE;
        }
    }
    theBmp = aVirDev.GetBitmap( Point( 0, 0 ), theBmp.GetSizePixel() );
    Invalidate( INVALIDATE_NOERASE );
    if(aSelectLink.IsSet())
        aSelectLink.Call(0);
}

//------------------------------------------------------------------------

void SvxFrameSelector::DrawSelArrow_Impl(

    OutputDevice& rDev, Point aPos, Color aCol, SvxSelArrowType eType )
{
    Point   aPtArray[5];
    Color   aPrevLineColor = rDev.GetLineColor();
    Color   aPrevFillColor = rDev.GetFillColor();
    rDev.SetLineColor( aCol );
    rDev.SetFillColor( aCol );

    switch ( eType )
    {
        case SVX_SELARROWTYPE_LEFT:
            aPtArray[0] = aPos + Point( 3, 0 );
            aPtArray[1] = aPos + Point( 4, 4 );
            aPtArray[2] = aPos + Point( 1, 1 );
            aPtArray[3] = aPos + Point( 2, 3 );
            aPtArray[4] = aPos + Point( 0, 2 );
            break;
        case SVX_SELARROWTYPE_RIGHT:
            aPtArray[0] = aPos;
            aPtArray[1] = aPos + Point( 1, 4 );
            aPtArray[2] = aPos + Point( 2, 1 );
            aPtArray[3] = aPos + Point( 3, 3 );
            aPtArray[4] = aPos + Point( 4, 2 );
            break;
        case SVX_SELARROWTYPE_UP:
            aPtArray[0] = aPos + Point( 0, 3 );
            aPtArray[1] = aPos + Point( 4, 4 );
            aPtArray[2] = aPos + Point( 1, 1 );
            aPtArray[3] = aPos + Point( 3, 2 );
            aPtArray[4] = aPos + Point( 2, 0 );
            break;
        case SVX_SELARROWTYPE_DOWN:
            aPtArray[0] = aPos;
            aPtArray[1] = aPos + Point( 4, 1 );
            aPtArray[2] = aPos + Point( 1, 2 );
            aPtArray[3] = aPos + Point( 3, 3 );
            aPtArray[4] = aPos + Point( 2, 4 );
            break;
    }
    rDev.DrawRect( Rectangle( aPtArray[0], aPtArray[1] ) );
    rDev.DrawRect( Rectangle( aPtArray[2], aPtArray[3] ) );
    rDev.DrawLine( aPtArray[4], aPtArray[4] );
    rDev.SetLineColor( aPrevLineColor );
    rDev.SetFillColor( aPrevFillColor );
}

//------------------------------------------------------------------------

void SvxFrameSelector::SetShadowPos( SvxFrameShadow eShadowPos )
{
    eShadow = eShadowPos;
}

//------------------------------------------------------------------------

void SvxFrameSelector::ShowShadow()
{
    VirtualDevice aVirDev;
    Bitmap theFrameBmp;
    Color aWhiteCol( COL_WHITE );
    Color theDrawCol =
        eShadow != SVX_FRMSHADOW_NONE ? theShadowCol : aWhiteCol;
    long nX = theBmp.GetSizePixel().Width();
    long nY = theBmp.GetSizePixel().Height();
    Point theOldPos;
    Rectangle theFrameRect = theBoundingRect;
    Rectangle theEraseRect( Point( 16, 16 ), Size( nX - 31, nY - 31 ) );

    aVirDev.SetOutputSizePixel( theBmp.GetSizePixel() );
    aVirDev.DrawBitmap( Point( 0, 0 ), theBmp );

    // Rahmen-Bitmap sichern
    theFrameBmp = aVirDev.GetBitmap( theFrameRect.TopLeft(),
                                     theFrameRect.GetSize() );
    // aktuellen Schatten entfernen:
    aVirDev.SetLineColor( aWhiteCol );
    aVirDev.SetFillColor( aWhiteCol );
    aVirDev.DrawRect( theEraseRect );
    aVirDev.SetLineColor( theDrawCol );
    aVirDev.SetFillColor( theDrawCol );

    if ( eShadow != SVX_FRMSHADOW_NONE )
    {
        Point aDeltaPnt( 3, 3 );

        switch ( eShadow )
        {
            case SVX_FRMSHADOW_TOP_RIGHT:
                aDeltaPnt.Y() *= -1; break;

            case SVX_FRMSHADOW_BOT_LEFT:
                aDeltaPnt.X() *= -1;
                break;

            case SVX_FRMSHADOW_TOP_LEFT:
                aDeltaPnt.X() *= -1; aDeltaPnt.Y() *= -1;
                break;
        }
        Rectangle aTempRect( Point( theFrameRect.Left()+aDeltaPnt.X(),
                                    theFrameRect.Top()+aDeltaPnt.Y() ),
                             theFrameRect.GetSize() );
        aVirDev.DrawRect( aTempRect );
    }
    aVirDev.DrawBitmap( theFrameRect.TopLeft(), theFrameBmp );
    theBmp = aVirDev.GetBitmap( Point( 0, 0 ), theBmp.GetSizePixel() );
    Invalidate( INVALIDATE_NOERASE );
}

//------------------------------------------------------------------------

void SvxFrameSelector::SetShadowColor( const Color& aColor )
{
    theShadowCol = aColor;
}

// -----------------------------------------------------------------------

Color SvxFrameSelector::GetShadowColor() const
{
    return theShadowCol;
}

// -----------------------------------------------------------------------

SvxFrameShadow SvxFrameSelector::GetShadowPos() const
{
    return eShadow;
}

// -----------------------------------------------------------------------

void SvxFrameSelector::Paint( const Rectangle& )
{
    DrawBitmap( Point( 0, 0 ), theBmp );
}

// -----------------------------------------------------------------------

void SvxFrameSelector::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() && ( rMEvt.GetClicks() == 1 ) )
    {
        CaptureMouse();
    }
}

// -----------------------------------------------------------------------

void SvxFrameSelector::MouseButtonUp( const MouseEvent& rMEvt )
{
    /* Verhalten bei Mausclicks:
     * -------------------------
     * o nur Einfachclicks
     * o Shift-Einfachclick -> Mehrfachselektion
     * o einzelne Linien:
     *   - ist die Linie bereits selektiert -> an/aus-Toggle
     *   - sonst -> an
     * o Kreuzungspunkt vertikal/horizontal (Mitte)
     *   - ist eine oder keine Linie selektiert -> an
     *   - sonst -> an/aus-Toggle beide
     * o Click auf die gleiche Linie -> Toggle SHOW/HIDE/DONT_CARE
     */

    if ( rMEvt.IsLeft() )
    {
        Point aBtnUpPos( rMEvt.GetPosPixel() );

        if ( !bIsClicked && !bIsDontCare )
        {
            bIsClicked = TRUE;

            // wenn Linien auf DontCare sind, muessen diese auf HIDE
            // gesetzt werden (ausser der aktuellen Linie)

            if ( theLeftLine.theState == SVX_FRMLINESTATE_DONT_CARE &&
                 !aSpotLeft.IsInside( aBtnUpPos ) )
            {
                theLeftLine.SetStyle( SvxFrameLine::NO_LINE );
            }
            if ( theRightLine.theState == SVX_FRMLINESTATE_DONT_CARE &&
                 !aSpotRight.IsInside( aBtnUpPos ) )
            {
                theRightLine.SetStyle( SvxFrameLine::NO_LINE );
            }
            if ( theTopLine.theState == SVX_FRMLINESTATE_DONT_CARE &&
                 !aSpotTop.IsInside( aBtnUpPos ) )
            {
                theTopLine.SetStyle( SvxFrameLine::NO_LINE );
            }
            if ( theBottomLine.theState == SVX_FRMLINESTATE_DONT_CARE &&
                 !aSpotBottom.IsInside( aBtnUpPos ) )
            {
                theBottomLine.SetStyle( SvxFrameLine::NO_LINE );
            }
            if ( theVerLine.theState == SVX_FRMLINESTATE_DONT_CARE &&
                 !aSpotVer.IsInside( aBtnUpPos ) )
            {
                theVerLine.SetStyle( SvxFrameLine::NO_LINE );
            }
            if ( theHorLine.theState == SVX_FRMLINESTATE_DONT_CARE &&
                 !aSpotHor.IsInside( aBtnUpPos ) )
            {
                theHorLine.SetStyle( SvxFrameLine::NO_LINE );
            }
        }

        if ( aSpotLeft.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( theLeftLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_LEFT );
        }
        else if ( aSpotRight.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( theRightLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_RIGHT );
        }
        else if ( aSpotTop.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( theTopLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_TOP );
        }
        else if ( aSpotBottom.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( theBottomLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_BOTTOM );
        }
        else if ( aSpotVer.IsInside( aBtnUpPos ) &&
                  aSpotHor.IsInside( aBtnUpPos ) )
        {
            if ( !theHorLine.bIsSelected || !theVerLine.bIsSelected )
            {
                theVerLine.aColor   =
                theHorLine.aColor   = theCurLineCol;
                theVerLine.SetStyle( theCurLineStyle );
                theHorLine.SetStyle( theCurLineStyle );

                if ( !rMEvt.IsShift() )
                    SelectLine( SVX_FRMSELLINE_NONE );

                SelectLine( SVX_FRMSELLINE_VER );
                SelectLine( SVX_FRMSELLINE_HOR );
            }
            else if ( theHorLine.bIsSelected && theVerLine.bIsSelected )
            {
                if ( !rMEvt.IsShift() )
                    SelectLine( SVX_FRMSELLINE_NONE );
                SelectLine( SVX_FRMSELLINE_VER );
                SelectLine( SVX_FRMSELLINE_HOR );
                LineClicked_Impl( theVerLine, TRUE, ( rMEvt.GetClicks() != 1 ) );
                LineClicked_Impl( theHorLine, TRUE, ( rMEvt.GetClicks() != 1 ) );
            }
        }
        else if ( aSpotVer.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( theVerLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_VER );
        }
        else if ( aSpotHor.IsInside( aBtnUpPos ) )
        {
            LineClicked_Impl( theHorLine, rMEvt.IsShift(),
                         ( rMEvt.GetClicks() != 1 ) );
            SelectLine( SVX_FRMSELLINE_HOR );
        }
        ShowLines();
        ReleaseMouse();
    }
}

// -----------------------------------------------------------------------

void SvxFrameSelector::LineClicked_Impl( SvxFrameLine& aLine,
                                 BOOL bShiftPressed,
                                 BOOL bDoubleClick )
{
    if ( aLine.bIsSelected )
    {
        int nMod = bIsDontCare ? 3 : 2;
        aLine.theState = SvxFrameLineState(
            ( ( (int)aLine.theState ) + ( ( !bDoubleClick ) ? 1 : 2 ) ) % nMod );

        switch ( aLine.theState )
        {
            case SVX_FRMLINESTATE_SHOW:
                aLine.SetStyle( theCurLineStyle );
                aLine.aColor = theCurLineCol;
                break;
            case SVX_FRMLINESTATE_HIDE:
                aLine.SetStyle( SvxFrameLine::NO_LINE );
                break;
            case SVX_FRMLINESTATE_DONT_CARE:
                aLine.theStyle = SvxFrameLine::THICK_LINE;
                break;
        }
    }
    else
    {
        aLine.theState = SVX_FRMLINESTATE_SHOW;
        aLine.aColor    = theCurLineCol;
        aLine.SetStyle( theCurLineStyle );
    }

    if ( !bShiftPressed )
        SelectLine( SVX_FRMSELLINE_NONE );
}

// -----------------------------------------------------------------------

BOOL    SvxFrameSelector::IsAnyLineSet() const
{
    if( theLeftLine.theState    == SVX_FRMLINESTATE_SHOW ||
        theRightLine.theState   == SVX_FRMLINESTATE_SHOW ||
        theTopLine.theState     == SVX_FRMLINESTATE_SHOW ||
        theBottomLine.theState  == SVX_FRMLINESTATE_SHOW ||
        theHorLine.theState     == SVX_FRMLINESTATE_SHOW ||
        theVerLine.theState     == SVX_FRMLINESTATE_SHOW )
        return TRUE;
    else
        return FALSE;
}


