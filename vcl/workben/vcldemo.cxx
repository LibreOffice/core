/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include <vcl/vclmain.hxx>

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

using namespace css;

class DemoWin : public WorkWindow
{
public:
    DemoWin() : WorkWindow( NULL, WB_APP | WB_STDWORK)
    {
    }

    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;

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

std::vector<Rectangle> DemoWin::partitionAndClear(int nX, int nY)
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

void DemoWin::Paint( const Rectangle& rRect )
{
    fprintf(stderr, "DemoWin::Paint(%ld,%ld,%ld,%ld)\n", rRect.getX(), rRect.getY(), rRect.getWidth(), rRect.getHeight());

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

class DemoApp : public Application
{
public:
    DemoApp() {}

    virtual int Main() SAL_OVERRIDE
    {
        DemoWin aMainWin;
        aMainWin.SetText( "Interactive VCL demo" );
        aMainWin.Show();
        Application::Execute();
        return 0;
    }

protected:
    uno::Reference<lang::XMultiServiceFactory> xMSF;
    void Init() SAL_OVERRIDE
    {
        try
        {
            uno::Reference<uno::XComponentContext> xComponentContext
                = ::cppu::defaultBootstrap_InitialComponentContext();
            xMSF = uno::Reference<lang::XMultiServiceFactory>
                ( xComponentContext->getServiceManager(), uno::UNO_QUERY );
            if( !xMSF.is() )
                Application::Abort("Bootstrap failure - no service manager");

            ::comphelper::setProcessServiceFactory( xMSF );
        }
        catch (const uno::Exception &e)
        {
            Application::Abort("Bootstrap exception " + e.Message);
        }
    }
    void DeInit() SAL_OVERRIDE
    {
        uno::Reference< lang::XComponent >(
            comphelper::getProcessComponentContext(),
        uno::UNO_QUERY_THROW )-> dispose();
        ::comphelper::setProcessServiceFactory( NULL );
    }
};

void vclmain::createApplication()
{
    static DemoApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
