/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <sal/main.h>
#include <tools/extendapplicationenvironment.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/metric.hxx>

#include <rtl/ustrbuf.hxx>

#include <math.h>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
// -----------------------------------------------------------------------

// Forward declaration
void Main();

// -----------------------------------------------------------------------

SAL_IMPLEMENT_MAIN()
{
    tools::extendApplicationEnvironment();

    Reference< XMultiServiceFactory > xMS;
    xMS = cppu::createRegistryServiceFactory( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "types.rdb" ) ), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "applicat.rdb" ) ), sal_True );

    comphelper::setProcessServiceFactory( xMS );

    InitVCL();
    ::Main();
    DeInitVCL();

    return 0;
}

// -----------------------------------------------------------------------

class MyWin : public WorkWindow
{
    Bitmap      m_aBitmap;
public:
                MyWin( Window* pParent, WinBits nWinStyle );

    void        MouseMove( const MouseEvent& rMEvt );
    void        MouseButtonDown( const MouseEvent& rMEvt );
    void        MouseButtonUp( const MouseEvent& rMEvt );
    void        KeyInput( const KeyEvent& rKEvt );
    void        KeyUp( const KeyEvent& rKEvt );
    void        Paint( const Rectangle& rRect );
    void        Resize();
};

// -----------------------------------------------------------------------

void Main()
{
    MyWin aMainWin( NULL, WB_APP | WB_STDWORK );
    aMainWin.SetText( rtl::OUString( "VCL - Workbench" ) );
    aMainWin.Show();

    Application::Execute();
}

// -----------------------------------------------------------------------

MyWin::MyWin( Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle ),
    m_aBitmap( Size( 256, 256 ), 32 )
{
    // prepare an alpha mask
    BitmapWriteAccess* pAcc = m_aBitmap.AcquireWriteAccess();
    for( int nX = 0; nX < 256; nX++ )
    {
        for( int nY = 0; nY < 256; nY++ )
        {
            double fRed = 255.0-1.5*sqrt((double)(nX*nX+nY*nY));
            if( fRed < 0.0 )
                fRed = 0.0;
            double fGreen = 255.0-1.5*sqrt((double)(((255-nX)*(255-nX)+nY*nY)));
            if( fGreen < 0.0 )
                fGreen = 0.0;
            double fBlue = 255.0-1.5*sqrt((double)((128-nX)*(128-nX)+(255-nY)*(255-nY)));
            if( fBlue < 0.0 )
                fBlue = 0.0;
            pAcc->SetPixel( nX, nY, BitmapColor( sal_uInt8(fRed), sal_uInt8(fGreen), sal_uInt8(fBlue) ) );
        }
    }
    m_aBitmap.ReleaseAccess( pAcc );
}

// -----------------------------------------------------------------------

void MyWin::MouseMove( const MouseEvent& rMEvt )
{
    WorkWindow::MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::KeyInput( const KeyEvent& rKEvt )
{
    WorkWindow::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void MyWin::KeyUp( const KeyEvent& rKEvt )
{
    WorkWindow::KeyUp( rKEvt );
}

// -----------------------------------------------------------------------

static Point project( const Point& rPoint )
{
    const double angle_x = M_PI / 6.0;
    const double angle_z = M_PI / 6.0;

    // transform planar coordinates to 3d
    double x = rPoint.X();
    double y = rPoint.Y();
    //double z = 0;

    // rotate around X axis
    double x1 = x;
    double y1 = y * cos( angle_x );
    double z1 = y * sin( angle_x );

    // rotate around Z axis
    double x2 = x1 * cos( angle_z ) + y1 * sin( angle_z );
    //double y2 = y1 * cos( angle_z ) - x1 * sin( angle_z );
    double z2 = z1;

    return Point( (sal_Int32)x2, (sal_Int32)z2 );
}

static Color approachColor( const Color& rFrom, const Color& rTo )
{
    Color aColor;
    sal_uInt8 nDiff;
    // approach red
    if( rFrom.GetRed() < rTo.GetRed() )
    {
        nDiff = rTo.GetRed() - rFrom.GetRed();
        aColor.SetRed( rFrom.GetRed() + ( nDiff < 10 ? nDiff : 10 ) );
    }
    else if( rFrom.GetRed() > rTo.GetRed() )
    {
        nDiff = rFrom.GetRed() - rTo.GetRed();
        aColor.SetRed( rFrom.GetRed() - ( nDiff < 10 ? nDiff : 10 ) );
    }
    else
        aColor.SetRed( rFrom.GetRed() );

    // approach Green
    if( rFrom.GetGreen() < rTo.GetGreen() )
    {
        nDiff = rTo.GetGreen() - rFrom.GetGreen();
        aColor.SetGreen( rFrom.GetGreen() + ( nDiff < 10 ? nDiff : 10 ) );
    }
    else if( rFrom.GetGreen() > rTo.GetGreen() )
    {
        nDiff = rFrom.GetGreen() - rTo.GetGreen();
        aColor.SetGreen( rFrom.GetGreen() - ( nDiff < 10 ? nDiff : 10 ) );
    }
    else
        aColor.SetGreen( rFrom.GetGreen() );

    // approach blue
    if( rFrom.GetBlue() < rTo.GetBlue() )
    {
        nDiff = rTo.GetBlue() - rFrom.GetBlue();
        aColor.SetBlue( rFrom.GetBlue() + ( nDiff < 10 ? nDiff : 10 ) );
    }
    else if( rFrom.GetBlue() > rTo.GetBlue() )
    {
        nDiff = rFrom.GetBlue() - rTo.GetBlue();
        aColor.SetBlue( rFrom.GetBlue() - ( nDiff < 10 ? nDiff : 10 ) );
    }
    else
        aColor.SetBlue( rFrom.GetBlue() );

    return aColor;
}

#define DELTA 5.0
void MyWin::Paint( const Rectangle& rRect )
{
    WorkWindow::Paint( rRect );

    Push( PUSH_ALL );
    MapMode aMapMode( MAP_100TH_MM );

    SetMapMode( aMapMode );

    Size aPaperSize = GetOutputSize();
    Point aCenter( aPaperSize.Width()/2-300,
                   (aPaperSize.Height() - 8400)/2 + 8400 );
    Point aP1( aPaperSize.Width()/48, 0), aP2( aPaperSize.Width()/40, 0 ), aPoint;

    DrawRect( Rectangle( Point( 0,0 ), aPaperSize ) );
    DrawRect( Rectangle( Point( 100,100 ),
                         Size( aPaperSize.Width()-200,
                               aPaperSize.Height()-200 ) ) );
    DrawRect( Rectangle( Point( 200,200 ),
                         Size( aPaperSize.Width()-400,
                               aPaperSize.Height()-400 ) ) );
    DrawRect( Rectangle( Point( 300,300 ),
                         Size( aPaperSize.Width()-600,
                               aPaperSize.Height()-600 ) ) );

    // AllSettings aSettings( Application::GetSettings() );

    const int nFontCount = GetDevFontCount();
    const int nFontSamples = (nFontCount<15) ? nFontCount : 15;
    for( int i = 0; i < nFontSamples; ++i )
    {

        FontInfo aFont = GetDevFont( (i*nFontCount) / nFontSamples );
        aFont.SetHeight( 400 + (i%7) * 100 );
        aFont.SetOrientation( i * (3600 / nFontSamples) );
        SetFont( aFont );

        sal_uInt8 nRed   = (i << 6) & 0xC0;
        sal_uInt8 nGreen = (i << 4) & 0xC0;
        sal_uInt8 nBlue  = (i << 2) & 0xC0;
        SetTextColor( Color( nRed, nGreen, nBlue ) );

        OUStringBuffer aPrintText(1024);
        long nMaxWidth = 0;

        aPrintText.appendAscii( "SVP test program" );

        DrawText( Rectangle( Point( (aPaperSize.Width() - 4000) / 2, 2000 ),
                            Size( aPaperSize.Width() - 2100 - nMaxWidth,
                                aPaperSize.Height() - 4000 ) ),
                aPrintText.makeStringAndClear(),
                TEXT_DRAW_MULTILINE );
    }

    SetFillColor();
    DrawRect( Rectangle( Point( aPaperSize.Width() - 4000, 1000 ),
                         Size( 3000,3000 ) ) );
    DrawBitmap( Point( aPaperSize.Width() - 4000, 1000 ),
                Size( 3000,3000 ),
                m_aBitmap );

    Color aWhite( 0xff, 0xff, 0xff );
    Color aBlack( 0, 0, 0 );
    Color aLightRed( 0xff, 0, 0 );
    Color aDarkRed( 0x40, 0, 0 );
    Color aLightBlue( 0, 0, 0xff );
    Color aDarkBlue( 0,0,0x40 );
    Color aLightGreen( 0, 0xff, 0 );
    Color aDarkGreen( 0, 0x40, 0 );

    Gradient aGradient( GradientStyle_LINEAR, aBlack, aWhite );
    aGradient.SetAngle( 900 );
    DrawGradient( Rectangle( Point( 1000, 4500 ),
                             Size( aPaperSize.Width() - 2000,
                                   500 ) ), aGradient );
    aGradient.SetStartColor( aDarkRed );
    aGradient.SetEndColor( aLightBlue );
    DrawGradient( Rectangle( Point( 1000, 5300 ),
                             Size( aPaperSize.Width() - 2000,
                                   500 ) ), aGradient );
    aGradient.SetStartColor( aDarkBlue );
    aGradient.SetEndColor( aLightGreen );
    DrawGradient( Rectangle( Point( 1000, 6100 ),
                             Size( aPaperSize.Width() - 2000,
                                   500 ) ), aGradient );
    aGradient.SetStartColor( aDarkGreen );
    aGradient.SetEndColor( aLightRed );
    DrawGradient( Rectangle( Point( 1000, 6900 ),
                             Size( aPaperSize.Width() - 2000,
                                   500 ) ), aGradient );



    LineInfo aLineInfo( LINE_SOLID, 200 );
    double sind = sin( DELTA*M_PI/180.0 );
    double cosd = cos( DELTA*M_PI/180.0 );
    double factor = 1 + (DELTA/1000.0);
    int n=0;
    Color aLineColor( 0, 0, 0 );
    Color aApproachColor( 0, 0, 200 );
    while ( aP2.X() < aCenter.X() && n++ < 680 )
    {
        aLineInfo.SetWidth( n/3 );
        aLineColor = approachColor( aLineColor, aApproachColor );
        SetLineColor( aLineColor );

        // switch aproach color
        if( aApproachColor.IsRGBEqual( aLineColor ) )
        {
            if( aApproachColor.GetRed() )
                aApproachColor = Color( 0, 0, 200 );
            else if( aApproachColor.GetGreen() )
                aApproachColor = Color( 200, 0, 0 );
            else
                aApproachColor = Color( 0, 200, 0 );
        }

        DrawLine( project( aP1 ) + aCenter,
                  project( aP2 ) + aCenter,
                  aLineInfo );
        aPoint.X() = (int)((((double)aP1.X())*cosd - ((double)aP1.Y())*sind)*factor);
        aPoint.Y() = (int)((((double)aP1.Y())*cosd + ((double)aP1.X())*sind)*factor);
        aP1 = aPoint;
        aPoint.X() = (int)((((double)aP2.X())*cosd - ((double)aP2.Y())*sind)*factor);
        aPoint.Y() = (int)((((double)aP2.Y())*cosd + ((double)aP2.X())*sind)*factor);
        aP2 = aPoint;
    }
    Pop();
}

// -----------------------------------------------------------------------

void MyWin::Resize()
{
    WorkWindow::Resize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
