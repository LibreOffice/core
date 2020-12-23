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
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <tools/extendapplicationenvironment.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/metric.hxx>
#include <vcl/vclptr.hxx>
#include <bitmapwriteaccess.hxx>

#include <rtl/ustrbuf.hxx>

#include <math.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

// Forward declaration
static void Main();

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
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("vcl.app", "Fatal");
        return 1;
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", e.what());
        return 1;
    }

    return 0;
}

namespace {

class MyWin : public WorkWindow
{
    Bitmap      m_aBitmap;
public:
                 MyWin( vcl::Window* pParent, WinBits nWinStyle );

    virtual void Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& rRect ) override;
};

}

void Main()
{
    ScopedVclPtrInstance< MyWin > aMainWin( nullptr, WB_APP | WB_STDWORK );
    aMainWin->SetText( "VCL - Workbench" );
    aMainWin->Show();

    Application::Execute();
}

MyWin::MyWin( vcl::Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle ),
    m_aBitmap( Size( 256, 256 ), 32 )
{
    // prepare an alpha mask
    BitmapWriteAccess* pAcc = m_aBitmap.AcquireWriteAccess();
    for( int nX = 0; nX < 256; nX++ )
    {
        for( int nY = 0; nY < 256; nY++ )
        {
            double fRed = 255.0-1.5*sqrt(static_cast<double>(nX*nX+nY*nY));
            if( fRed < 0.0 )
                fRed = 0.0;
            double fGreen = 255.0-1.5*sqrt(static_cast<double>((255-nX)*(255-nX)+nY*nY));
            if( fGreen < 0.0 )
                fGreen = 0.0;
            double fBlue = 255.0-1.5*sqrt(static_cast<double>((128-nX)*(128-nX)+(255-nY)*(255-nY)));
            if( fBlue < 0.0 )
                fBlue = 0.0;
            pAcc->SetPixel( nY, nX, BitmapColor( sal_uInt8(fRed), sal_uInt8(fGreen), sal_uInt8(fBlue) ) );
        }
    }
    Bitmap::ReleaseAccess( pAcc );
}

static Point project( const Point& rPoint )
{
    const double angle_x = M_PI / 6.0;
    const double angle_z = M_PI / 6.0;

    // transform planar coordinates to 3d
    double x = rPoint.X();
    double y = rPoint.Y();

    // rotate around X axis
    double x1 = x;
    double y1 = y * cos( angle_x );
    double z1 = y * sin( angle_x );

    // rotate around Z axis
    double x2 = x1 * cos( angle_z ) + y1 * sin( angle_z );
    //double y2 = y1 * cos( angle_z ) - x1 * sin( angle_z );
    double z2 = z1;

    return Point( static_cast<sal_Int32>(x2), static_cast<sal_Int32>(z2) );
}

static Color approachColor( const Color& rFrom, const Color& rTo )
{
    Color aColor;
    sal_uInt8 nDiff;
    // approach red
    if( rFrom.GetRed() < rTo.GetRed() )
    {
        nDiff = rTo.GetRed() - rFrom.GetRed();
        aColor.SetRed( rFrom.GetRed() + std::min<sal_uInt8>( nDiff, 10 ) );
    }
    else if( rFrom.GetRed() > rTo.GetRed() )
    {
        nDiff = rFrom.GetRed() - rTo.GetRed();
        aColor.SetRed( rFrom.GetRed() - std::min<sal_uInt8>( nDiff, 10 ) );
    }
    else
        aColor.SetRed( rFrom.GetRed() );

    // approach Green
    if( rFrom.GetGreen() < rTo.GetGreen() )
    {
        nDiff = rTo.GetGreen() - rFrom.GetGreen();
        aColor.SetGreen( rFrom.GetGreen() + std::min<sal_uInt8>( nDiff, 10 ) );
    }
    else if( rFrom.GetGreen() > rTo.GetGreen() )
    {
        nDiff = rFrom.GetGreen() - rTo.GetGreen();
        aColor.SetGreen( rFrom.GetGreen() - std::min<sal_uInt8>( nDiff, 10 ) );
    }
    else
        aColor.SetGreen( rFrom.GetGreen() );

    // approach blue
    if( rFrom.GetBlue() < rTo.GetBlue() )
    {
        nDiff = rTo.GetBlue() - rFrom.GetBlue();
        aColor.SetBlue( rFrom.GetBlue() + std::min<sal_uInt8>( nDiff, 10 ) );
    }
    else if( rFrom.GetBlue() > rTo.GetBlue() )
    {
        nDiff = rFrom.GetBlue() - rTo.GetBlue();
        aColor.SetBlue( rFrom.GetBlue() - std::min<sal_uInt8>( nDiff, 10 ) );
    }
    else
        aColor.SetBlue( rFrom.GetBlue() );

    return aColor;
}

#define DELTA 5.0
void MyWin::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    WorkWindow::Paint(rRenderContext, rRect);

    rRenderContext.Push();
    MapMode aMapMode(MapUnit::Map100thMM);

    rRenderContext.SetMapMode(aMapMode);

    Size aPaperSize = rRenderContext.GetOutputSize();
    Point aCenter(aPaperSize.Width() / 2 - 300,
                  (aPaperSize.Height() - 8400) / 2 + 8400);
    Point aP1(aPaperSize.Width() / 48, 0), aP2(aPaperSize.Width() / 40, 0);
    Point aPoint;

    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), aPaperSize));
    rRenderContext.DrawRect(tools::Rectangle(Point(100, 100),
                         Size(aPaperSize.Width() - 200,
                              aPaperSize.Height() - 200)));
    rRenderContext.DrawRect(tools::Rectangle(Point(200, 200),
                         Size(aPaperSize.Width() - 400,
                              aPaperSize.Height() - 400)));
    rRenderContext.DrawRect(tools::Rectangle(Point(300, 300),
                         Size(aPaperSize.Width() - 600,
                              aPaperSize.Height() - 600)));

    const int nFontCount = rRenderContext.GetDevFontCount();
    const int nFontSamples = (nFontCount < 15) ? nFontCount : 15;
    for (int i = 0; i < nFontSamples; ++i)
    {

        FontMetric aFont = rRenderContext.GetDevFont((i * nFontCount) / nFontSamples);
        aFont.SetFontHeight(400 + (i % 7) * 100);
        aFont.SetOrientation(Degree10(i * (3600 / nFontSamples)));
        rRenderContext.SetFont(aFont);

        sal_uInt8 nRed   = (i << 6) & 0xC0;
        sal_uInt8 nGreen = (i << 4) & 0xC0;
        sal_uInt8 nBlue  = (i << 2) & 0xC0;
        rRenderContext.SetTextColor(Color(nRed, nGreen, nBlue));

        rRenderContext.DrawText(tools::Rectangle(Point((aPaperSize.Width() - 4000) / 2, 2000),
                                Size(aPaperSize.Width() - 2100, aPaperSize.Height() - 4000)),
                                "SVP test program",
                                DrawTextFlags::MultiLine);
    }

    rRenderContext.SetFillColor();
    DrawRect(tools::Rectangle(Point(aPaperSize.Width() - 4000, 1000),
                       Size(3000, 3000)));
    rRenderContext.DrawBitmap(Point(aPaperSize.Width() - 4000, 1000),
                              Size( 3000,3000 ),
                              m_aBitmap);

    Color const aWhite(0xff, 0xff, 0xff);
    Color const aBlack(0, 0, 0);
    Color const aLightRed(0xff, 0, 0);
    Color const aDarkRed(0x40, 0, 0);
    Color const aLightBlue(0, 0, 0xff);
    Color const aDarkBlue(0,0,0x40);
    Color const aLightGreen(0, 0xff, 0);
    Color const aDarkGreen(0, 0x40, 0);

    Gradient aGradient(GradientStyle::Linear, aBlack, aWhite);
    aGradient.SetAngle(900_deg10);
    rRenderContext.DrawGradient(tools::Rectangle(Point(1000, 4500),
                                Size(aPaperSize.Width() - 2000, 500)),
                                aGradient);
    aGradient.SetStartColor(aDarkRed);
    aGradient.SetEndColor(aLightBlue);
    rRenderContext.DrawGradient(tools::Rectangle(Point(1000, 5300),
                                Size(aPaperSize.Width() - 2000, 500)),
                                aGradient);
    aGradient.SetStartColor(aDarkBlue);
    aGradient.SetEndColor(aLightGreen);
    rRenderContext.DrawGradient(tools::Rectangle(Point(1000, 6100),
                                Size(aPaperSize.Width() - 2000, 500)),
                                aGradient);
    aGradient.SetStartColor(aDarkGreen);
    aGradient.SetEndColor(aLightRed);
    rRenderContext.DrawGradient(tools::Rectangle(Point(1000, 6900),
                                Size(aPaperSize.Width() - 2000, 500)),
                                aGradient);

    LineInfo aLineInfo(LineStyle::Solid, 200);
    const double sind = sin(basegfx::deg2rad(DELTA));
    const double cosd = cos(basegfx::deg2rad(DELTA));
    const double factor = 1 + (DELTA / 1000.0);
    int n = 0;
    Color aLineColor(0, 0, 0);
    Color aApproachColor(0, 0, 200);

    while (aP2.X() < aCenter.X() && n++ < 680)
    {
        aLineInfo.SetWidth(n / 3);
        aLineColor = approachColor(aLineColor, aApproachColor);
        rRenderContext.SetLineColor(aLineColor);

        // switch approach color
        if (aApproachColor.IsRGBEqual(aLineColor))
        {
            if (aApproachColor.GetRed())
               aApproachColor = Color(0, 0, 200);
            else if (aApproachColor.GetGreen())
               aApproachColor = Color(200, 0, 0);
            else
                aApproachColor = Color(0, 200, 0);
        }

        rRenderContext.DrawLine(project(aP1) + aCenter,
                                project(aP2) + aCenter,
                                aLineInfo);
        aPoint.setX( static_cast<int>((static_cast<double>(aP1.X())*cosd - static_cast<double>(aP1.Y())*sind)*factor) );
        aPoint.setY( static_cast<int>((static_cast<double>(aP1.Y())*cosd + static_cast<double>(aP1.X())*sind)*factor) );
        aP1 = aPoint;
        aPoint.setX( static_cast<int>((static_cast<double>(aP2.X())*cosd - static_cast<double>(aP2.Y())*sind)*factor) );
        aPoint.setY( static_cast<int>((static_cast<double>(aP2.Y())*cosd + static_cast<double>(aP2.X())*sind)*factor) );
        aP2 = aPoint;
    }
    rRenderContext.Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
