/*************************************************************************
 *
 *  $RCSfile: colctrl.cxx,v $
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


#ifndef _SV_SALBTYPE_HXX //autogen
#include <vcl/salbtype.hxx>
#endif

#ifndef _SV_BMPACC_HXX //autogen
#include <vcl/bmpacc.hxx>
#endif

#include <colctrl.hxx>

// ----------------
// - ColorControl -
// ----------------

// -----------------------------------------------------------------------
SvColorControl::SvColorControl( Window* pParent, WinBits nStyle ) :
    Control         ( pParent, nStyle ),
    mpBitmap        ( NULL ),
    mpReadAccess    ( NULL ),
    mnLuminance     ( 50 )
{
    Initialize();
}

// -----------------------------------------------------------------------
SvColorControl::SvColorControl( Window* pParent, const ResId& rResId ) :
    Control         ( pParent, rResId ),
    mpBitmap        ( NULL ),
    mpReadAccess    ( NULL ),
    mnLuminance     ( 50 )
{
    Initialize();
}


// -----------------------------------------------------------------------
SvColorControl::~SvColorControl()
{
    if( mpReadAccess )
        mpBitmap->ReleaseAccess( mpReadAccess );

    delete mpBitmap;

}

// -----------------------------------------------------------------------
void SvColorControl::Initialize()
{
    SetLineColor( Color( COL_BLACK ) );
}

// -----------------------------------------------------------------------
void SvColorControl::CreateBitmap()
{
    Size aSize = GetOutputSizePixel();
    Size aBmpSize;
    if( mpBitmap && mpBitmap->GetSizePixel() != aSize )
    {
        delete mpBitmap;
        mpBitmap = NULL;
    }
    if( !mpBitmap )
        mpBitmap = new Bitmap( aSize, 24 );

    BitmapWriteAccess* pWriteAccess = mpBitmap->AcquireWriteAccess();

    USHORT nX = (USHORT) aSize.Width();
    USHORT nY = (USHORT) aSize.Height();

    UINT16 nHue, nSat;
    UINT16 nBri = mnLuminance;
    ColorHSB aColHSB( 0, 0, mnLuminance );

    for( USHORT i = 0; i < nY; i++ )
    {
        nSat = 100 - ( 100.0 * i + 0.5 ) / nY;

        for( USHORT j = 0; j < nX; j++ )
        {
            nHue = ( 360.0 * j + 0.5 ) / nX;

            aColHSB.SetHue( nHue );
            aColHSB.SetSat( nSat );
            Color aCol( aColHSB.GetRGB() );

            pWriteAccess->SetPixel( i, j, BitmapColor( aCol ) );

        }
    }

    /*
     erster Ansatz nur mit RGB
    USHORT n1_6 = (USHORT) ((nX+3) / 6);
    USHORT n2_6 = (USHORT) ((nX*2+3) / 6);
    USHORT n3_6 = (USHORT) ((nX*3+3) / 6);
    USHORT n4_6 = (USHORT) ((nX*4+3) / 6);
    USHORT n5_6 = (USHORT) ((nX*5+3) / 6);
    USHORT n6_6 = nX;

    BitmapColor aBmpColor;
    BYTE cR, cG, cB;
    */
    /*
    for( USHORT i = 0; i < nY; i++ )
    {
        USHORT nK = 100 * i / nY;

        for( USHORT j = 0; j < nX; j++ )
        {
            if( j < n1_6 )
            {
                cR = 255;
                cG = 255 * j / n1_6;
                cB = 0;
            }
            else if( j < n2_6 )
            {
                cR = 255 - 255 * ( j - n1_6 ) / n1_6;
                cG = 255;
                cB = 0;
            }
            else if( j < n3_6 )
            {
                cR = 0;
                cG = 255;
                cB = 255 * (j - n2_6 ) / n1_6;
            }
            else if( j < n4_6 )
            {
                cR = 0;
                cG = 255 - 255 * ( j - n3_6 ) / n1_6;
                cB = 255;
            }
            else if( j < n5_6 )
            {
                cR = 255 * (j - n4_6 ) / n1_6;
                cG = 0;
                cB = 255;
            }
            else if( j < nX )
            {
                cR = 255;
                cG = 0;
                cB = 255 - 255 * ( j - n5_6 ) / n1_6;;
            }

            //cR -= ( cR - mnLuminance * 256 / 100 ) * nK / 100;
            //cG -= ( cG - mnLuminance * 256 / 100 ) * nK / 100;
            //cB -= ( cB - mnLuminance * 256 / 100 ) * nK / 100;

            //aBmpColor.SetRed( cR );
            //aBmpColor.SetGreen( cG );
            //aBmpColor.SetBlue( cB );

            //pWriteAccess->SetPixel( i, j, aBmpColor );

            ColorHSB aColHSB( Color( cR, cG, cB ) );
            aColHSB.SetSat( nK );
            Color aCol( aColHSB.GetRGB() );

            pWriteAccess->SetPixel( i, j, BitmapColor( aCol ) );

        }
    }
    */
    mpBitmap->ReleaseAccess( pWriteAccess );

    mpReadAccess = mpBitmap->AcquireReadAccess();

    SetColor( maColor ); // Anzeige der Position im Control
}

// -----------------------------------------------------------------------
void SvColorControl::ShowPosition( const Point& rPos )
{
    // Explizites Abfragen des Bereichs, da schon mal ein Wert < 0 vorhanden ist
    if( mpBitmap )
    {
        long nX = rPos.X();
        long nY = rPos.Y();
        if( nX < 0L )
            nX = 0L;
        else if( nX >= mpBitmap->GetSizePixel().Width() )
            nX = mpBitmap->GetSizePixel().Width() - 1L;

        if( nY < 0L )
            nY= 0L;
        else if( nY >= mpBitmap->GetSizePixel().Height() )
            nY = mpBitmap->GetSizePixel().Height() - 1L;

        Point aPos = maPosition;
        maPosition.X() = nX - 2;
        maPosition.Y() = nY - 2;
        Invalidate( Rectangle( aPos, Size( 5, 5) ) );
        Invalidate( Rectangle( maPosition, Size( 5, 5) ) );

        maColor = mpReadAccess->GetPixel( nY, nX );
    }
}
// -----------------------------------------------------------------------
void SvColorControl::MouseMove( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        ShowPosition( rMEvt.GetPosPixel() );
        Modify();
    }
}

// -----------------------------------------------------------------------
void SvColorControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() && !rMEvt.IsShift() )
    {
        //ShowPointer( FALSE );
        CaptureMouse();
        ShowPosition( rMEvt.GetPosPixel() );
        Modify();
    }
}

// -----------------------------------------------------------------------
void SvColorControl::MouseButtonUp( const MouseEvent& rMEvt )
{
    //ShowPointer( TRUE );
    if( IsMouseCaptured() )
        ReleaseMouse();
}

// -----------------------------------------------------------------------
void SvColorControl::Paint( const Rectangle& rRect )
{
    if( !mpBitmap )
        CreateBitmap();

    // dither bitmap if there only 256 colors available !
    if( GetBitCount() <= 8 )
        mpBitmap->Dither();

    DrawBitmap( rRect.TopLeft(), rRect.GetSize(),
                rRect.TopLeft(), rRect.GetSize(), *mpBitmap );

    // Positions-Control (Fadenkreuz oder Aehnliches)
    Point aPos1( maPosition );
    Point aPos2( maPosition );
    aPos2.X() += 4;
    DrawLine( aPos1, aPos2 );
    aPos2.X() -= 4;
    aPos2.Y() += 4;
    DrawLine( aPos1, aPos2 );
    aPos1.Y() += 4;
    aPos2.X() += 4;
    DrawLine( aPos1, aPos2 );
    aPos1.X() += 4;
    aPos2.Y() -= 4;
    DrawLine( aPos1, aPos2 );
}

// -----------------------------------------------------------------------
void SvColorControl::Resize()
{
    CreateBitmap();

    Control::Resize();
}

// -----------------------------------------------------------------------
void SvColorControl::Modify()
{
    maModifyHdl.Call( this );
}

// -----------------------------------------------------------------------
void SvColorControl::SetColor( const ColorHSB& rCol, BOOL bSetColor )
{
    if( bSetColor )
        maColor = rCol.GetRGB();

    if( mpBitmap )
    {
        USHORT nX = (USHORT) mpBitmap->GetSizePixel().Width();
        USHORT nY = (USHORT) mpBitmap->GetSizePixel().Height();

        INT16 nZ = rCol.GetBri();
        SetLuminance( nZ );

        nX = rCol.GetHue() * nX / 360; // Farbe
        nY = nY - rCol.GetSat() * nY / 100; // Saettigung

        ShowPosition( Point( nX, nY ) );
    }
}

// -----------------------------------------------------------------------
void SvColorControl::SetColor( const Color& rCol )
{
    maColor = rCol;

    if( mpBitmap )
    {
        ColorHSB aColHsb( rCol );
        SetColor( aColHsb, FALSE );
        /*
        BOOL bDone = FALSE;
        USHORT nX = (USHORT) mpBitmap->GetSizePixel().Width();
        USHORT nY = (USHORT) mpBitmap->GetSizePixel().Height();
        USHORT i, j;

        BitmapColor aBmpColor;

        for( i = 0; i < nY && !bDone; i++ )
        {
            for( j = 0; j < nX && !bDone; j++ )
            {
                aBmpColor = mpReadAccess->GetPixel( i, j );

                if( maColor == (Color)aBmpColor )
                    bDone = TRUE;
            }
        }

        if( bDone )
            ShowPosition( Point( j, i ) );
        */
    }
}

// -----------------------------------------------------------------------
void SvColorControl::SetLuminance( short nLum )
{
    if( nLum != mnLuminance &&
        nLum >= 0 && nLum <= 100 )
    {
        mnLuminance = nLum;

        if( mnLuminance < 40 )
            SetLineColor( Color( COL_WHITE ) );
        else
            SetLineColor( Color( COL_BLACK ) );

        CreateBitmap();

        USHORT nX = maPosition.X() + 2;
        USHORT nY = maPosition.Y() + 2;
        maColor = mpReadAccess->GetPixel( nY, nX );

        Invalidate();
    }
}


// -----------------------
// - ColorPreviewControl -
// -----------------------


// -----------------------------------------------------------------------
ColorPreviewControl::ColorPreviewControl( Window* pParent, WinBits nStyle ) :
    Control     ( pParent, nStyle )
{
    SetFillColor( maColor );
    SetLineColor( maColor );
}

// -----------------------------------------------------------------------
ColorPreviewControl::ColorPreviewControl( Window* pParent, const ResId& rResId ) :
    Control     ( pParent, rResId )
{
    SetFillColor( maColor );
    SetLineColor( maColor );
}


// -----------------------------------------------------------------------
ColorPreviewControl::~ColorPreviewControl()
{
}

// -----------------------------------------------------------------------
void ColorPreviewControl::Paint( const Rectangle& rRect )
{
    DrawRect( rRect );
}

// -----------------------------------------------------------------------
void ColorPreviewControl::SetColor( const Color& rCol )
{
    if( rCol != maColor )
    {
        maColor = rCol;
        SetFillColor( maColor );
        SetLineColor( maColor );
        Invalidate();
    }
}


// -----------------------
// - ColorMixingControl -
// -----------------------


// -----------------------------------------------------------------------
ColorMixingControl::ColorMixingControl( Window* pParent, WinBits nStyle,
                                        USHORT nRows, USHORT nColumns ) :
    ValueSet    ( pParent, nStyle ),
    mnRows      ( nRows ),
    mnColumns   ( nColumns )
{
    Initialize();
}

// -----------------------------------------------------------------------
ColorMixingControl::ColorMixingControl( Window* pParent, const ResId& rResId,
                                        USHORT nRows, USHORT nColumns ) :
    ValueSet    ( pParent, rResId ),
    mnRows      ( nRows ),
    mnColumns   ( nColumns )
{
    Initialize();
}


// -----------------------------------------------------------------------
ColorMixingControl::~ColorMixingControl()
{
}

// -----------------------------------------------------------------------
void ColorMixingControl::Initialize()
{
    SetColCount( mnColumns );

    Color aColor;
    String aStr;
    for( USHORT i = 1; i <= mnRows * mnColumns; i++ )
    {
        InsertItem( i, aColor, aStr );
    }

    /*maColor[ 0 ] = Color( COL_LIGHTRED );
    maColor[ 1 ] = Color( COL_LIGHTGREEN );
    maColor[ 2 ] = Color( COL_YELLOW );
    maColor[ 3 ] = Color( COL_LIGHTBLUE );*/

    SetColor( CMC_TOPLEFT, Color( COL_LIGHTRED ) );
    SetColor( CMC_BOTTOMRIGHT, Color( COL_LIGHTBLUE ) );

    SetColor( CMC_TOPRIGHT, Color( COL_LIGHTGREEN ) );
    SetColor( CMC_BOTTOMLEFT, Color( COL_YELLOW ) );

    /*FillColumn( 0 );
    FillColumn( mnColumns - 1 );
    for( i = 0; i < mnRows; i++ )
        FillRow( i );*/
}

// -----------------------------------------------------------------------
Color ColorMixingControl::CalcDifferenceColor( USHORT nCol1, USHORT nCol2,
                                                USHORT nSteps )
{
    // Die Berechnung ist noch etwas ungenau, daher sollte besser mit floats
    // gearbeitet werden...  (muss !!!)
    Color aColor( GetItemColor( nCol1 ) );
    Color aColor2( GetItemColor( nCol2 ) );

    aColor.SetRed( (UINT8) ( ( aColor2.GetRed() - aColor.GetRed() ) / nSteps ) );
    aColor.SetGreen( (UINT8) ( ( aColor2.GetGreen() - aColor.GetGreen() ) / nSteps ) );
    aColor.SetBlue( (UINT8) ( ( aColor2.GetBlue() - aColor.GetBlue() ) / nSteps ) );

    return( aColor );
}

// -----------------------------------------------------------------------
void ColorMixingControl::FillRow( USHORT nRow )
{
    USHORT nCol1 = nRow * mnColumns + 1;
    USHORT nCol2 = ( nRow + 1 ) * mnColumns;
    Color aColor( GetItemColor( nCol1 ) );
    Color aDiffColor( CalcDifferenceColor( nCol1, nCol2, mnColumns - 1 ) );

    for( USHORT i = nCol1 + 1; i < nCol2; i++ )
    {
        aColor.SetRed( aColor.GetRed() + aDiffColor.GetRed() );
        aColor.SetGreen( aColor.GetGreen() + aDiffColor.GetGreen() );
        aColor.SetBlue( aColor.GetBlue() + aDiffColor.GetBlue() );

        SetItemColor( i, aColor );
        SetItemText( i, GetRGBString( aColor ) );
    }
}

// -----------------------------------------------------------------------
void ColorMixingControl::FillColumn( USHORT nColumn )
{
    USHORT nCol1 = nColumn + 1;
    USHORT nCol2 = nColumn + ( mnRows - 1 ) * mnColumns + 1;
    Color aColor( GetItemColor( nCol1 ) );
    Color aDiffColor( CalcDifferenceColor( nCol1, nCol2, mnRows - 1 ) );

    for( USHORT i = nCol1 + mnColumns; i < nCol2; i += mnColumns )
    {
        aColor.SetRed( aColor.GetRed() + aDiffColor.GetRed() );
        aColor.SetGreen( aColor.GetGreen() + aDiffColor.GetGreen() );
        aColor.SetBlue( aColor.GetBlue() + aDiffColor.GetBlue() );

        SetItemColor( i, aColor );
        SetItemText( i, GetRGBString( aColor ) );
    }
}

// -----------------------------------------------------------------------
void ColorMixingControl::SetRows( USHORT nRows )
{
    mnRows = nRows;
}

// -----------------------------------------------------------------------
void ColorMixingControl::SetColumns( USHORT nColumns )
{
    mnColumns = nColumns;
}

// -----------------------------------------------------------------------
void ColorMixingControl::SetColor( CMCPosition ePos, const Color& rCol )
{
    if( rCol != maColor[ ePos ] )
    {
        maColor[ ePos ] = rCol;
        USHORT nPos, nColumn;
        String aStr( GetRGBString( rCol ) );

        switch( ePos )
        {
            case CMC_TOPLEFT:
                nPos = 1;
                nColumn = 0;
            break;

            case CMC_TOPRIGHT:
                nPos = mnColumns;
                nColumn = mnColumns - 1;
            break;

            case CMC_BOTTOMLEFT:
                nPos = ( mnRows - 1 ) * mnColumns + 1;
                nColumn = 0;
            break;

            case CMC_BOTTOMRIGHT:
                nPos = mnRows * mnColumns;
                nColumn = mnColumns - 1;
            break;
        }
        SetItemColor( nPos, rCol );
        SetItemText( nPos, aStr );
        FillColumn( nColumn );

        for( USHORT i = 0; i < mnRows; i++ )
            FillRow( i );
    }
}

// -----------------------------------------------------------------------
String ColorMixingControl::GetRGBString( const Color& rColor )
{
    String  aStr( String::CreateFromInt32(rColor.GetRed()) );
            aStr += ' ';
            aStr += String::CreateFromInt32(rColor.GetGreen());
            aStr += ' ';
            aStr += String::CreateFromInt32(rColor.GetBlue());

    return aStr;
}
// -----------------------------------------------------------------------
CMCPosition ColorMixingControl::GetCMCPosition() const
{
    CMCPosition ePos = CMC_OTHER;
    USHORT nPos = GetSelectItemId();

    if( nPos == 1 )
        ePos = CMC_TOPLEFT;
    else if( nPos == mnColumns )
        ePos = CMC_TOPRIGHT;
    else if( nPos == ( mnRows - 1 ) * mnColumns + 1 )
        ePos = CMC_BOTTOMLEFT;
    else if( nPos == mnRows * mnColumns )
        ePos = CMC_BOTTOMRIGHT;

    return( ePos );
}


// ------------
// - ColorHSB -
// ------------

// Erste Ansaetze gingen auf die Berechnung von Sven Hannover zurueck
// Der jetzige Algorithmus stammt im weitesten Sinne aus dem Foley/VanDam


/**************************************************************************
|*
|*    ColorHSB::ColorHSB()
|*
|*    Beschreibung       RGB nach HSB
|*    Ersterstellung     SOH 02.10.97
|*
**************************************************************************/

ColorHSB::ColorHSB( const Color& rColor )
{
    UINT8 c[3];
    UINT8 cMax, cMin;

    c[0] = rColor.GetRed();
    c[1] = rColor.GetGreen();
    c[2] = rColor.GetBlue();

    cMax = c[0];
    if( c[1] > cMax )
        cMax = c[1];
    if( c[2] > cMax )
        cMax = c[2];

    // Brightness = max(R, G, B);
    mnBri = cMax * 100 / 255;

    cMin = c[0];
    if( c[1] < cMin )
        cMin = c[1];
    if( c[2] < cMin )
        cMin = c[2];

    UINT8 cDelta = cMax - cMin;

    // Saturation = max - min / max
    if( mnBri > 0 )
        mnSat = cDelta * 100 / cMax;
    else
        mnSat = 0;

    if( mnSat == 0 )
        mnHue = 0; // Default = undefined
    else
    {
        double dHue;

        if( c[0] == cMax )
        {
            dHue = (double)( c[1] - c[2] ) / (double)cDelta;
        }
        else if( c[1] == cMax )
        {
            dHue = 2.0 + (double)( c[2] - c[0] ) / (double)cDelta;
        }
        else if ( c[2] == cMax )
        {
            dHue = 4.0 + (double)( c[0] - c[1] ) / (double)cDelta;
        }
        dHue *= 60.0;

        if( dHue < 0.0 )
            dHue += 360.0;

        mnHue = (UINT16) dHue;
    }
}

/**************************************************************************
|*
|*    ColorHSB::GetRGB()
|*
|*    Beschreibung       HSB nach RGB
|*    Ersterstellung     SOH 02.10.97
|*
**************************************************************************/

Color ColorHSB::GetRGB() const
{
    UINT8 cR,cG,cB;
    UINT8 nB = (UINT8) ( mnBri * 255 / 100 );

    if( mnSat == 0 )
    {
        cR = nB;
        cG = nB;
        cB = nB;
    }
    else
    {
        double dH = mnHue;
        double f;
        UINT16 n;
        if( dH == 360.0 )
            dH = 0.0;

        dH /= 60.0;
        n = (UINT16) dH;
        f = dH - n;

        UINT8 a = (UINT8) ( nB * ( 100 - mnSat ) / 100 );
        UINT8 b = (UINT8) ( nB * ( 100 - ( (double)mnSat * f + 0.5 ) ) / 100 );
        UINT8 c = (UINT8) ( nB * ( 100 - ( (double)mnSat * ( 1.0 - f ) + 0.5 ) ) / 100 );

        switch( n )
        {
            case 0: cR = nB;    cG = c;     cB = a;     break;
            case 1: cR = b;     cG = nB;    cB = a;     break;
            case 2: cR = a;     cG = nB;    cB = c;     break;
            case 3: cR = a;     cG = b;     cB = nB;    break;
            case 4: cR = c;     cG = a;     cB = nB;    break;
            case 5: cR = nB;    cG = a;     cB = b;     break;
        }
    }

    return( Color( cR, cG, cB ) );
}

// ------------
// - ColorCMYK -
// ------------


// -----------------------------------------------------------------------
ColorCMYK::ColorCMYK( const Color& rColor )
{
    mnCyan    = 255 - rColor.GetRed();
    mnMagenta = 255 - rColor.GetGreen();
    mnYellow  = 255 - rColor.GetBlue();

    mnKey = Min( Min( mnCyan, mnMagenta ), mnYellow );

    mnCyan    = mnCyan - mnKey;
    mnMagenta = mnMagenta - mnKey;
    mnYellow  = mnYellow - mnKey;
}

// -----------------------------------------------------------------------
Color ColorCMYK::GetRGB() const
{
    INT16 nTmp = Max( 0, 255 - ( mnCyan + mnKey ) );
    UINT8 cR = (UINT8) nTmp;
          nTmp = Max( 0, 255 - ( mnMagenta + mnKey ) );
    UINT8 cG = (UINT8) nTmp;
          nTmp = Max( 0, 255 - ( mnYellow + mnKey ) );
    UINT8 cB = (UINT8) nTmp;

    return( Color( cR, cG, cB ) );
}


