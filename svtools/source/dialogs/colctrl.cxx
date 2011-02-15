/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"


#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>

#include <svtools/colctrl.hxx>

// ----------------
// - ColorControl -
// ----------------

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
    const Size aSize( GetOutputSizePixel() );

    if( mpBitmap && mpBitmap->GetSizePixel() != aSize )
        delete mpBitmap, mpBitmap = NULL;

    if( !mpBitmap )
        mpBitmap = new Bitmap( aSize, 24 );

    BitmapWriteAccess* pWriteAccess = mpBitmap->AcquireWriteAccess();

    if( pWriteAccess )
    {
        sal_uInt16 nX = (sal_uInt16) aSize.Width();
        sal_uInt16 nY = (sal_uInt16) aSize.Height();

        sal_uInt16      nHue, nSat;
        ColorHSB    aColHSB( 0, 0, mnLuminance );

        for( sal_uInt16 i = 0; i < nY; i++ )
        {
            nSat = (sal_uInt16) FRound( 100 - ( 100.0 * i + 0.5 ) / nY );

            for( sal_uInt16 j = 0; j < nX; j++ )
            {
                nHue = (sal_uInt16) FRound( ( 360.0 * j + 0.5 ) / nX );

                aColHSB.SetHue( nHue );
                aColHSB.SetSat( nSat );

                // mpBitmap always has a bit count of 24 => use of SetPixel(...) is safe
                pWriteAccess->SetPixel( i, j, BitmapColor( aColHSB.GetRGB() ) );
            }
        }

        mpBitmap->ReleaseAccess( pWriteAccess );
    }

       SetColor( maColor );
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

        if( ( mpReadAccess = mpBitmap->AcquireReadAccess() ) != NULL )
        {
            // mpBitmap always has a bit count of 24 => use of GetPixel(...) is safe
            maColor = mpReadAccess->GetPixel( nY, nX );
            mpBitmap->ReleaseAccess( mpReadAccess );
            mpReadAccess = NULL;
        }
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
        //ShowPointer( sal_False );
        CaptureMouse();
        ShowPosition( rMEvt.GetPosPixel() );
        Modify();
    }
}

// -----------------------------------------------------------------------
void SvColorControl::MouseButtonUp( const MouseEvent& )
{
    //ShowPointer( sal_True );
    if( IsMouseCaptured() )
        ReleaseMouse();
}

// -----------------------------------------------------------------------
void SvColorControl::Paint( const Rectangle& rRect )
{
    if( !mpBitmap )
        CreateBitmap();

    Bitmap aOutputBitmap( *mpBitmap );

    if( GetBitCount() <= 8 )
        aOutputBitmap.Dither();

    DrawBitmap( rRect.TopLeft(), rRect.GetSize(), rRect.TopLeft(), rRect.GetSize(), aOutputBitmap );

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
void SvColorControl::SetColor( const ColorHSB& rCol, sal_Bool bSetColor )
{
    if( bSetColor )
        maColor = rCol.GetRGB();

    if( mpBitmap )
    {
        sal_uInt16  nX = (sal_uInt16) mpBitmap->GetSizePixel().Width();
        sal_uInt16  nY = (sal_uInt16) mpBitmap->GetSizePixel().Height();
        sal_Int16   nZ = rCol.GetBri();

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
        SetColor( aColHsb, sal_False );
    }
}

// -----------------------------------------------------------------------
void SvColorControl::SetLuminance( short nLum )
{
    if( nLum != mnLuminance && nLum >= 0 && nLum <= 100 )
    {
        mnLuminance = nLum;

        if( mnLuminance < 40 )
            SetLineColor( Color( COL_WHITE ) );
        else
            SetLineColor( Color( COL_BLACK ) );

        CreateBitmap();

        long nX = maPosition.X() + 2;
        long nY = maPosition.Y() + 2;

        if( mpBitmap && ( ( mpReadAccess = mpBitmap->AcquireReadAccess() ) != NULL ) )
        {
            // mpBitmap always has a bit count of 24 => use of GetPixel(...) is safe
            maColor = mpReadAccess->GetPixel( nY, nX );
            mpBitmap->ReleaseAccess( mpReadAccess );
            mpReadAccess = NULL;
        }

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
                                        sal_uInt16 nRows, sal_uInt16 nColumns ) :
    ValueSet    ( pParent, nStyle ),
    mnRows      ( nRows ),
    mnColumns   ( nColumns )
{
    Initialize();
}

// -----------------------------------------------------------------------
ColorMixingControl::ColorMixingControl( Window* pParent, const ResId& rResId,
                                        sal_uInt16 nRows, sal_uInt16 nColumns ) :
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
    for( sal_uInt16 i = 1; i <= mnRows * mnColumns; i++ )
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
Color ColorMixingControl::CalcDifferenceColor( sal_uInt16 nCol1, sal_uInt16 nCol2,
                                                sal_uInt16 nSteps )
{
    // Die Berechnung ist noch etwas ungenau, daher sollte besser mit floats
    // gearbeitet werden...  (muss !!!)
    Color aColor( GetItemColor( nCol1 ) );
    Color aColor2( GetItemColor( nCol2 ) );

    aColor.SetRed( (sal_uInt8) ( ( aColor2.GetRed() - aColor.GetRed() ) / nSteps ) );
    aColor.SetGreen( (sal_uInt8) ( ( aColor2.GetGreen() - aColor.GetGreen() ) / nSteps ) );
    aColor.SetBlue( (sal_uInt8) ( ( aColor2.GetBlue() - aColor.GetBlue() ) / nSteps ) );

    return( aColor );
}

// -----------------------------------------------------------------------
void ColorMixingControl::FillRow( sal_uInt16 nRow )
{
    sal_uInt16 nCol1 = nRow * mnColumns + 1;
    sal_uInt16 nCol2 = ( nRow + 1 ) * mnColumns;
    Color aColor( GetItemColor( nCol1 ) );
    Color aDiffColor( CalcDifferenceColor( nCol1, nCol2, mnColumns - 1 ) );

    for( sal_uInt16 i = nCol1 + 1; i < nCol2; i++ )
    {
        aColor.SetRed( aColor.GetRed() + aDiffColor.GetRed() );
        aColor.SetGreen( aColor.GetGreen() + aDiffColor.GetGreen() );
        aColor.SetBlue( aColor.GetBlue() + aDiffColor.GetBlue() );

        SetItemColor( i, aColor );
        SetItemText( i, GetRGBString( aColor ) );
    }
}

// -----------------------------------------------------------------------
void ColorMixingControl::FillColumn( sal_uInt16 nColumn )
{
    sal_uInt16 nCol1 = nColumn + 1;
    sal_uInt16 nCol2 = nColumn + ( mnRows - 1 ) * mnColumns + 1;
    Color aColor( GetItemColor( nCol1 ) );
    Color aDiffColor( CalcDifferenceColor( nCol1, nCol2, mnRows - 1 ) );

    for( sal_uInt16 i = nCol1 + mnColumns; i < nCol2; i = i + mnColumns )
    {
        aColor.SetRed( aColor.GetRed() + aDiffColor.GetRed() );
        aColor.SetGreen( aColor.GetGreen() + aDiffColor.GetGreen() );
        aColor.SetBlue( aColor.GetBlue() + aDiffColor.GetBlue() );

        SetItemColor( i, aColor );
        SetItemText( i, GetRGBString( aColor ) );
    }
}

// -----------------------------------------------------------------------
void ColorMixingControl::SetRows( sal_uInt16 nRows )
{
    mnRows = nRows;
}

// -----------------------------------------------------------------------
void ColorMixingControl::SetColumns( sal_uInt16 nColumns )
{
    mnColumns = nColumns;
}

// -----------------------------------------------------------------------
void ColorMixingControl::SetColor( CMCPosition ePos, const Color& rCol )
{
    if( rCol != maColor[ ePos ] )
    {
        maColor[ ePos ] = rCol;
        sal_uInt16 nPos = 0;
        sal_uInt16 nColumn = 0;
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
            case CMC_OTHER:
            break;  // -Wall not handled.
        }
        SetItemColor( nPos, rCol );
        SetItemText( nPos, aStr );
        FillColumn( nColumn );

        for( sal_uInt16 i = 0; i < mnRows; i++ )
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
    sal_uInt16 nPos = GetSelectItemId();

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
    sal_uInt8 c[3];
    sal_uInt8 cMax, cMin;

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

    sal_uInt8 cDelta = cMax - cMin;

    // Saturation = max - min / max
    if( mnBri > 0 )
        mnSat = cDelta * 100 / cMax;
    else
        mnSat = 0;

    if( mnSat == 0 )
        mnHue = 0; // Default = undefined
    else
    {
        double dHue = 0;

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
        // else dHue = ???   -Wall   FIXME
        dHue *= 60.0;

        if( dHue < 0.0 )
            dHue += 360.0;

        mnHue = (sal_uInt16) dHue;
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
    sal_uInt8 cR,cG,cB;
    sal_uInt8 nB = (sal_uInt8) ( mnBri * 255 / 100 );

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
        sal_uInt16 n;
        if( dH == 360.0 )
            dH = 0.0;

        dH /= 60.0;
        n = (sal_uInt16) dH;
        f = dH - n;

        // #107375# Doing the calculation completely in floating
        // point, the former optimization gave sometimes negative
        // results for c and was pointless anyway
        sal_uInt8 a = static_cast<sal_uInt8>( nB * ( 100.0 - mnSat ) / 100.0 );
        sal_uInt8 b = static_cast<sal_uInt8>( nB * ( 100.0 - mnSat * f ) / 100.0 );
        sal_uInt8 c = static_cast<sal_uInt8>( nB * ( 100.0 - mnSat * ( 1.0 - f ) ) / 100.0 );

        switch( n )
        {
            case 0: cR = nB;    cG = c;     cB = a;     break;
            case 1: cR = b;     cG = nB;    cB = a;     break;
            case 2: cR = a;     cG = nB;    cB = c;     break;
            case 3: cR = a;     cG = b;     cB = nB;    break;
            case 4: cR = c;     cG = a;     cB = nB;    break;
            case 5: cR = nB;    cG = a;     cB = b;     break;
            default: cR = 0;    cG = 0;     cB = 0;     break;  // -Wall ????
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
    int nTmp = Max( 0, 255 - ( mnCyan + mnKey ) );
    sal_uInt8 cR = (sal_uInt8) nTmp;
          nTmp = Max( 0, 255 - ( mnMagenta + mnKey ) );
    sal_uInt8 cG = (sal_uInt8) nTmp;
          nTmp = Max( 0, 255 - ( mnYellow + mnKey ) );
    sal_uInt8 cB = (sal_uInt8) nTmp;

    return( Color( cR, cG, cB ) );
}


