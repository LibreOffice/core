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

#include <sal/main.h>
#include <tools/extendapplicationenvironment.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>

#include <unistd.h>
#include <stdio.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

// Forward declaration
void Main();

SAL_IMPLEMENT_MAIN()
{
    try
    {
        tools::extendApplicationEnvironment();

        Reference< XComponentContext > xContext = defaultBootstrap_InitialComponentContext();
        Reference< XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), UNO_QUERY );

        if( !xServiceManager.is() )
            Application::Abort( "Failed to bootstrap" );

        comphelper::setProcessServiceFactory( xServiceManager );

        InitVCL();
        ::Main();
        DeInitVCL();
    }
    catch (const Exception& e)
    {
        SAL_WARN("vcl.app", "Fatal exception: " << e.Message);
        return 1;
    }

    return 0;
}

class MyWin : public WorkWindow
{
public:
                 MyWin( vcl::Window* pParent, WinBits nWinStyle );

    virtual void MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void KeyUp( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;

    std::vector<Rectangle> partitionAndClear(int nX, int nY);

    void drawBackground()
    {
        Rectangle r(Point(0,0), GetSizePixel());
        Gradient aGradient;
        aGradient.SetStartColor(COL_BLUE);
        aGradient.SetEndColor(COL_GREEN);
        aGradient.SetStyle(GradientStyle_LINEAR);
//        aGradient.SetBorder(r.GetSize().Width()/20);
        DrawGradient(r, aGradient);
    }

    void drawRadialLines(Rectangle r)
    {
        SetFillColor(Color(COL_LIGHTRED));
        SetLineColor(Color(COL_LIGHTGREEN));
        DrawRect( r );

        for(int i=0; i<r.GetHeight(); i+=15)
            DrawLine( Point(r.Left(), r.Top()+i), Point(r.Right(), r.Bottom()-i) );
        for(int i=0; i<r.GetWidth(); i+=15)
            DrawLine( Point(r.Left()+i, r.Bottom()), Point(r.Right()-i, r.Top()) );
    }

    void drawText(Rectangle r)
    {
        SetTextColor( Color( COL_BLACK ) );
        vcl::Font aFont( OUString( "Times" ), Size( 0, 25 ) );
        SetFont( aFont );
        DrawText( r, OUString( "Just a simple text" ) );
    }

    void drawPoly(Rectangle r) // pretty
    {
        Polygon aPoly(r, r.TopLeft(), r.TopRight());
        SetLineColor(Color(COL_RED));
//        DrawPolyLine(aPoly);
    }

    void drawPolyPoly(Rectangle r)
    {
        (void)r;
    }

    void drawCheckered(Rectangle r)
    {
        DrawCheckered(r.TopLeft(), r.GetSize());
    }
    void drawGradient(Rectangle r)
    {
        Gradient aGradient;
        aGradient.SetStartColor(COL_BLUE);
        aGradient.SetEndColor(COL_GREEN);
//        aGradient.SetAngle(45);
        aGradient.SetStyle(GradientStyle_LINEAR);
        aGradient.SetBorder(r.GetSize().Width()/20);
        DrawGradient(r, aGradient);
    }
};

void Main()
{
    MyWin aMainWin( NULL, WB_APP | WB_STDWORK );
    aMainWin.SetText( OUString( "VCLDemo - VCL Workbench" ) );
    aMainWin.Show();

    Application::Execute();
}

MyWin::MyWin( vcl::Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle )
{
}

void MyWin::MouseMove( const MouseEvent& rMEvt )
{
    WorkWindow::MouseMove( rMEvt );
}

void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    Rectangle aRect(0,0,4,4);
    aRect.SetPos( rMEvt.GetPosPixel() );
    SetFillColor(Color(COL_RED));
    DrawRect( aRect );
}

void MyWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonUp( rMEvt );
}

void MyWin::KeyInput( const KeyEvent& rKEvt )
{
    WorkWindow::KeyInput( rKEvt );
}

void MyWin::KeyUp( const KeyEvent& rKEvt )
{
    WorkWindow::KeyUp( rKEvt );
}

std::vector<Rectangle> MyWin::partitionAndClear(int nX, int nY)
{
    Rectangle r;
    std::vector<Rectangle> aRegions;

    // Make small cleared area for these guys
    Size aSize(GetSizePixel());
    long nBorderSize = aSize.Width() / 32;
    long nBoxWidth = (aSize.Width() - nBorderSize*(nX+1)) / nX;
    long nBoxHeight = (aSize.Height() - nBorderSize*(nX+1)) / nY;
//    SL_DEBUG("Size " << aSize << " boxes " << nBoxWidth << "x" << nBoxHeight << " border " << nBorderSize);
    for (int y = 0; y < nY; y++ )
    {
        for (int x = 0; x < nX; x++ )
        {
            r.SetPos(Point(nBorderSize + (nBorderSize + nBoxWidth) * x,
                           nBorderSize + (nBorderSize + nBoxHeight) * y));
            r.SetSize(Size(nBoxWidth, nBoxHeight));

            // knock up a nice little border
            SetLineColor(COL_GRAY);
            SetFillColor(COL_LIGHTGRAY);
            if ((x + y) % 2)
                DrawRect(r);
            else
                DrawRect(r);
//              DrawRect(r, nBorderSize, nBorderSize); FIXME - lfrb

            aRegions.push_back(r);
        }
    }

    return aRegions;
}

void MyWin::Paint( const Rectangle& rRect )
{
    fprintf(stderr, "MyWin::Paint(%ld,%ld,%ld,%ld)\n", rRect.getX(), rRect.getY(), rRect.getWidth(), rRect.getHeight());

    drawBackground();

    std::vector<Rectangle> aRegions(partitionAndClear(4,3));

    int i = 0;
    drawRadialLines(aRegions[i++]);
    drawText(aRegions[i++]);
    drawPoly(aRegions[i++]);
    drawPolyPoly(aRegions[i++]);
    drawCheckered(aRegions[i++]);
    drawGradient(aRegions[i++]);
}

void MyWin::Resize()
{
    WorkWindow::Resize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
