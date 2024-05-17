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

// This code strongly inspired by Miguel / Federico's Gnome Canvas demo code.

#include <sal/config.h>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <o3tl/deleter.hxx>
#include <o3tl/safeint.hxx>
#include <utility>
#include <vcl/canvastools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclmain.hxx>
#include <vcl/wrkwin.hxx>

using namespace ::com::sun::star;

static void PrintHelp()
{
    fprintf( stdout, "canvasdemo - Exercise the new canvas impl\n" );
}

namespace {

class TestWindow : public WorkWindow
{
    public:
        TestWindow() : WorkWindow(nullptr, WB_APP | WB_STDWORK)
        {
            SetText(u"Canvas test"_ustr);
            SetSizePixel( Size( 600, 450 ) );
            EnablePaint( true );
            Show();
        }
        virtual ~TestWindow() override {}
        virtual void MouseButtonUp( const MouseEvent& /*rMEvt*/ ) override
        {
            //TODO: do something cool
            Application::Quit();
        }
        virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
};

class DemoRenderer
{
    public:
        Size maSize;
        Size maBox;
        rendering::ViewState   maViewState;
        rendering::RenderState maRenderState;
        uno::Sequence< double > maColorBlack;
        uno::Sequence< double > maColorRed;
        uno::Reference< rendering::XCanvas > mxCanvas;
        uno::Reference< rendering::XCanvasFont > mxDefaultFont;
        uno::Reference< rendering::XGraphicDevice > mxDevice;

        DemoRenderer( uno::Reference< rendering::XGraphicDevice > xDevice,
                      uno::Reference< rendering::XCanvas > xCanvas,
                      Size aSize ) :
            maSize(aSize),
            maColorBlack( vcl::unotools::colorToStdColorSpaceSequence( COL_BLACK) ),
            maColorRed( vcl::unotools::colorToStdColorSpaceSequence( COL_RED) ),
            mxCanvas(xCanvas),
            mxDevice(std::move( xDevice ))
        {
            // Geometry init
            geometry::AffineMatrix2D aUnit( 1,0, 0,
                                            0,1, 0 );
            maViewState.AffineTransform = aUnit;
            maRenderState.AffineTransform = aUnit;
            maRenderState.DeviceColor = maColorBlack;

            //I can't figure out what the compositeoperation stuff does
            //it doesn't seem to do anything in either VCL or cairocanvas
            //I was hoping that CLEAR would clear the canvas before we paint,
            //but nothing changes
            maRenderState.CompositeOperation = rendering::CompositeOperation::OVER;

            maBox.setWidth(aSize.Width() / 3);
            maBox.setHeight(aSize.Height() / 3);

            lang::Locale aLocale;
            rendering::FontInfo aFontInfo;
            aFontInfo.FamilyName = "Swiss";
            aFontInfo.StyleName = "SansSerif";
            geometry::Matrix2D aFontMatrix( 1, 0,
                                            0, 1 );
            rendering::FontRequest aFontRequest( aFontInfo, 12.0, 0.0, aLocale );
            uno::Sequence< beans::PropertyValue > aExtraFontProperties;
            mxDefaultFont = xCanvas->createFont( aFontRequest, aExtraFontProperties, aFontMatrix );
            if( !mxDefaultFont.is() )
                fprintf( stderr, "Failed to create font\n" );
        }

        void drawGrid()
        {
            tools::Long d, dIncr = maSize.Width() / 3;
            for ( d = 0; d <= maSize.Width(); d += dIncr )
                mxCanvas->drawLine( geometry::RealPoint2D( d, 0 ),
                                    geometry::RealPoint2D( d, maSize.Height() ),
                                    maViewState, maRenderState );
            dIncr = maSize.Height() / 3;
            for ( d = 0; d <= maSize.Height(); d += dIncr )
                mxCanvas->drawLine( geometry::RealPoint2D( 0, d ),
                                    geometry::RealPoint2D( maSize.Width(), d ),
                                    maViewState, maRenderState );
        }

        void drawStringAt( OString aString, double x, double y )
        {
            rendering::StringContext aText;
            aText.Text = OStringToOUString( aString, RTL_TEXTENCODING_UTF8 );
            aText.StartPosition = 0;
            aText.Length = aString.getLength();
            rendering::RenderState aRenderState( maRenderState );
            aRenderState.AffineTransform.m02 += x;
            aRenderState.AffineTransform.m12 += y;

            mxCanvas->drawText( aText, mxDefaultFont, maViewState, aRenderState, 0);
        }

        void drawRect( tools::Rectangle rRect, const uno::Sequence< double > &aColor, int /*nWidth*/ )
        {
            uno::Sequence< uno::Sequence< geometry::RealPoint2D > > aPolys
            {
                {
                    { o3tl::narrowing<double>(rRect.Left()),  o3tl::narrowing<double>(rRect.Top()) },
                    { o3tl::narrowing<double>(rRect.Left()),  o3tl::narrowing<double>(rRect.Bottom()) },
                    { o3tl::narrowing<double>(rRect.Right()), o3tl::narrowing<double>(rRect.Bottom()) },
                    { o3tl::narrowing<double>(rRect.Right()), o3tl::narrowing<double>(rRect.Top()) }
                }
            };
            auto xPoly = mxDevice->createCompatibleLinePolyPolygon( aPolys );
            xPoly->setClosed( 0, true );

            rendering::RenderState aRenderState( maRenderState );
            aRenderState.DeviceColor = aColor;
            mxCanvas->drawPolyPolygon( xPoly, maViewState, aRenderState );
        }

        void translate( double x, double y)
        {
            maRenderState.AffineTransform.m02 += x;
            maRenderState.AffineTransform.m12 += y;
        }

        void drawPolishDiamond( double center_x, double center_y)
        {
            const int    VERTICES = 10;
            const double RADIUS = 60.0;
            int i, j;

            rendering::RenderState aOldRenderState = maRenderState; // push
            translate( center_x, center_y );

            for (i = 0; i < VERTICES; i++)
            {
                double a = 2.0 * M_PI * i / VERTICES;
                geometry::RealPoint2D aSrc( RADIUS * cos (a), RADIUS * sin (a) );

                for (j = i + 1; j < VERTICES; j++)
                {
                    a = 2.0 * M_PI * j / VERTICES;

//                  FIXME: set cap_style to 'ROUND'
                    mxCanvas->drawLine( aSrc,
                                        geometry::RealPoint2D( RADIUS * cos (a),
                                                               RADIUS * sin (a) ),
                                        maViewState, maRenderState );
                }
            }

            maRenderState = std::move(aOldRenderState); // pop
        }

        void drawHilbert( double anchor_x, double anchor_y )
        {
            const double SCALE=7.0;
            const char hilbert[] = "urdrrulurulldluuruluurdrurddldrrruluurdrurddldrddlulldrdldrrurd";
            int nLength = std::size( hilbert );

            uno::Sequence< geometry::RealPoint2D > aPoints( nLength );
            auto pPoints = aPoints.getArray();
            uno::Reference< rendering::XLinePolyPolygon2D > xPoly;

            pPoints[0] = geometry::RealPoint2D( anchor_x, anchor_y );
            for (int i = 0; i < nLength; i++ )
            {
                switch( hilbert[i] )
                {
                    case 'u':
                        pPoints[i+1] = geometry::RealPoint2D( aPoints[i].X,
                                                            aPoints[i].Y - SCALE );
                        break;
                    case 'd':
                        pPoints[i+1] = geometry::RealPoint2D( aPoints[i].X,
                                                            aPoints[i].Y + SCALE );
                        break;
                    case 'l':
                        pPoints[i+1] = geometry::RealPoint2D( aPoints[i].X - SCALE,
                                                            aPoints[i].Y );
                        break;
                    case 'r':
                        pPoints[i+1] = geometry::RealPoint2D( aPoints[i].X + SCALE,
                                                            aPoints[i].Y );
                        break;
                }
            }

            uno::Sequence< uno::Sequence< geometry::RealPoint2D > > aPolys { aPoints };

            xPoly = mxDevice->createCompatibleLinePolyPolygon( aPolys );
            xPoly->setClosed( 0, false );

            rendering::RenderState aRenderState( maRenderState );
            aRenderState.DeviceColor = maColorRed;
//          aRenderState.DeviceColor[3] = 0.5;
            rendering::StrokeAttributes aStrokeAttrs;
            aStrokeAttrs.StrokeWidth = 4.0;
            aStrokeAttrs.MiterLimit = 2.0; // ?
            aStrokeAttrs.StartCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.EndCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.JoinType = rendering::PathJoinType::MITER;
            //fprintf( stderr, "FIXME: stroking a tools::PolyPolygon doesn't show up\n" );
            //yes it does
            mxCanvas->strokePolyPolygon( xPoly, maViewState, aRenderState, aStrokeAttrs );
            // FIXME: do this instead:
            //mxCanvas->drawPolyPolygon( xPoly, maViewState, aRenderState );
        }

        void drawTitle( OString aTitle )
        {
            // FIXME: text anchoring to be done
            double nStringWidth = aTitle.getLength() * 8.0;
            drawStringAt ( aTitle, (maBox.Width() - nStringWidth) / 2, 15 );
        }

        void drawRectangles()
        {
            rendering::RenderState aOldRenderState = maRenderState; // push

            drawTitle( "Rectangles"_ostr );

            drawRect( tools::Rectangle( 20, 30, 70, 60 ), maColorRed, 8 );
            // color mediumseagreen, stipple fill, outline black
            drawRect( tools::Rectangle( 90, 40, 180, 100 ), maColorBlack, 4 );
            // color steelblue, filled, no outline
            drawRect( tools::Rectangle( 10, 80, 80, 140 ), maColorBlack, 1 );

            maRenderState = std::move(aOldRenderState); // pop
        }

        void drawEllipses()
        {
            rendering::RenderState aOldRenderState = maRenderState; // push
            translate( maBox.Width(), 0.0 );

            drawTitle( "Ellipses"_ostr );

            const basegfx::B2DPoint aCenter( maBox.Width()*.5,
                                             maBox.Height()*.5 );
            const basegfx::B2DPoint aRadii( maBox.Width()*.3,
                                            maBox.Height()*.3 );
            const basegfx::B2DPolygon& rEllipse(
                basegfx::utils::createPolygonFromEllipse( aCenter,
                                                          aRadii.getX(),
                                                          aRadii.getY() ));

            uno::Reference< rendering::XPolyPolygon2D > xPoly(
                basegfx::unotools::xPolyPolygonFromB2DPolygon(mxDevice,
                                                              rEllipse) );

            rendering::StrokeAttributes aStrokeAttrs;
            aStrokeAttrs.StrokeWidth = 4.0;
            aStrokeAttrs.MiterLimit = 2.0; // ?
            aStrokeAttrs.StartCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.EndCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.JoinType = rendering::PathJoinType::MITER;
            mxCanvas->strokePolyPolygon( xPoly, maViewState, maRenderState, aStrokeAttrs );

            maRenderState = std::move(aOldRenderState); // pop
        }

        void drawText()
        {
            rendering::RenderState aOldRenderState = maRenderState; // push
            translate( maBox.Width() * 2.0, 0.0 );

            drawTitle( "Text"_ostr );

            translate( 0.0,
                       maBox.Height() * .5 );
            drawTitle( "This is lame"_ostr );

            maRenderState = std::move(aOldRenderState); // pop
        }

        void drawImages()
        {
            rendering::RenderState aOldRenderState = maRenderState; // push
            translate( 0.0, maBox.Height() );

            drawTitle( "Images"_ostr );

            uno::Reference< rendering::XBitmap > xBitmap(mxCanvas, uno::UNO_QUERY);

            if( !xBitmap.is() )
                return;

            translate( maBox.Width()*0.1, maBox.Height()*0.2 );
            maRenderState.AffineTransform.m00 *= 4.0/15;
            maRenderState.AffineTransform.m11 *= 3.0/15;

            mxCanvas->drawBitmap(xBitmap, maViewState, maRenderState);

            // uno::Reference< rendering::XBitmap > xBitmap2( xBitmap->getScaledBitmap(geometry::RealSize2D(48, 48), false) );
            // mxCanvas->drawBitmap(xBitmap2, maViewState, maRenderState); //yes, but where?
            //cairo-canvas says:
            //called CanvasHelper::getScaledBitmap, we return NULL, TODO
            //Exception 'BitmapEx vclcanvas::tools::bitmapExFromXBitmap(const css::uno::Reference<css::rendering::XBitmap>&),
            //bitmapExFromXBitmap(): could not extract BitmapEx' thrown

            //vcl-canvas says:
            //Exception 'BitmapEx vclcanvas::tools::bitmapExFromXBitmap(const css::uno::Reference<css::rendering::XBitmap>&),
            //bitmapExFromXBitmap(): could not extract bitmap' thrown
            //  Thorsten says that this is a bug, and Thorsten never lies.

            maRenderState = std::move(aOldRenderState); // pop
        }

        void drawLines()
        {
            rendering::RenderState aOldRenderState = maRenderState; // push
            translate( maBox.Width(), maBox.Height() );

            drawTitle( "Lines"_ostr );

            drawPolishDiamond( 70.0, 80.0 );
            drawHilbert( 140.0, 140.0 );

            maRenderState = std::move(aOldRenderState); // pop
        }

        void drawCurves()
        {
            rendering::RenderState aOldRenderState = maRenderState; // push
            translate( maBox.Width() * 2.0, maBox.Height() );

            drawTitle( "Curves"_ostr );

            translate( maBox.Width() * .5, maBox.Height() * .5 );

            const double r= 30.0;
            const int num_curves = 3;

            //hacky hack hack
            uno::Sequence< geometry::RealBezierSegment2D > aBeziers (num_curves);
            auto pBeziers = aBeziers.getArray();
            uno::Reference< rendering::XBezierPolyPolygon2D > xPoly;

            for (int i= 0; i < num_curves; i++)
                pBeziers[i]= geometry::RealBezierSegment2D( r * cos(i*2*M_PI/num_curves), //Px
                                                            r * sin(i*2*M_PI/num_curves), //py
                                                            r * 2 * cos((i*2*M_PI + 2*M_PI)/num_curves),  //C1x
                                                            r * 2 * sin((i*2*M_PI + 2*M_PI)/num_curves),  //C1y
                                                            r * 2 * cos((i*2*M_PI + 2*M_PI)/num_curves),  //C2x
                                                            r * 2 * sin((i*2*M_PI + 2*M_PI)/num_curves)); //C2y
            uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > > aPolys { aBeziers };
            xPoly = mxDevice->createCompatibleBezierPolyPolygon(aPolys);
            xPoly->setClosed( 0, true );
            //uno::Reference< rendering::XBezierPolyPolygon2D> xPP( xPoly, uno::UNO_QUERY );
            //compiles, but totally screws up.  I think it is interpreting the bezier as a line

            rendering::StrokeAttributes aStrokeAttrs;
            aStrokeAttrs.StrokeWidth = 4.0;
            aStrokeAttrs.MiterLimit = 2.0; // ?
            aStrokeAttrs.StartCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.EndCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.JoinType = rendering::PathJoinType::MITER;
            mxCanvas->strokePolyPolygon( xPoly, maViewState, maRenderState, aStrokeAttrs );
            //you can't draw a BezierPolyPolygon2D with this, even though it is derived from it
            //mxCanvas->drawPolyPolygon( xPoly, maViewState, maRenderState );

            maRenderState = std::move(aOldRenderState); // pop
        }

        double gimmerand()
        {
            return comphelper::rng::uniform_real_distribution(0, 100);
        }

        void drawArcs()
        {
            rendering::RenderState aOldRenderState = maRenderState; // push
            translate( 0.0, maBox.Height() * 2.0 );

            drawTitle( "Arcs"_ostr );


            //begin hacks
            //This stuff doesn't belong here, but probably in curves
            //This stuff doesn't work in VCL b/c vcl doesn't do beziers
            //Hah!  Every time the window redraws, we do this
            double bx;
            double by;
            bx= gimmerand();
            by= gimmerand();

            for (int i= 0; i < 1; i++)
            {
                double ax;
                double ay;
                //point a= point b;
                ax= bx;
                ay= by;
                //point b= rand;
                bx= gimmerand();
                by= gimmerand();
                double c1x= gimmerand();
                double c1y= gimmerand();
                double c2x= gimmerand();
                double c2y= gimmerand();
                maRenderState.DeviceColor = maColorRed;
                mxCanvas->drawLine(geometry::RealPoint2D(ax, ay), geometry::RealPoint2D(c1x, c1y), maViewState, maRenderState);
                mxCanvas->drawLine(geometry::RealPoint2D(c1x, c1y), geometry::RealPoint2D(c2x, c2y), maViewState, maRenderState);
                mxCanvas->drawLine(geometry::RealPoint2D(bx, by), geometry::RealPoint2D(c2x, c2y), maViewState, maRenderState);
                 //draw from a to b
                geometry::RealBezierSegment2D aBezierSegment(
                    ax, //Px
                    ay, //Py
                    c1x,
                    c1x,
                    c2x,
                    c2y
                    );
                geometry::RealPoint2D aEndPoint(bx, by);
                maRenderState.DeviceColor = maColorBlack;
                mxCanvas->drawBezier(
                    aBezierSegment,
                    aEndPoint,
                    maViewState, maRenderState );
            }
            maRenderState = std::move(aOldRenderState); // pop
        }


    void drawRegularPolygon(double centerx, double centery, int sides, double r)
        {
            //hacky hack hack
            uno::Sequence< geometry::RealPoint2D > aPoints (sides);
            auto pPoints = aPoints.getArray();
            uno::Reference< rendering::XLinePolyPolygon2D > xPoly;

            for (int i= 0; i < sides; i++)
            {
                pPoints[i]= geometry::RealPoint2D( centerx + r * cos(i*2 * M_PI/sides),
                                                   centery + r * sin(i*2 * M_PI/sides));
            }
            uno::Sequence< uno::Sequence< geometry::RealPoint2D > > aPolys { aPoints };
            xPoly = mxDevice->createCompatibleLinePolyPolygon( aPolys );
            xPoly->setClosed( 0, true );
            rendering::RenderState aRenderState( maRenderState );
            aRenderState.DeviceColor = maColorRed;
            mxCanvas->drawPolyPolygon( xPoly, maViewState, aRenderState);
            mxCanvas->fillPolyPolygon( xPoly,
                                       maViewState,
                                       aRenderState );
        }

        void drawPolygons()
        {
            rendering::RenderState aOldRenderState = maRenderState; // push
            translate( maBox.Width() * 1.0, maBox.Height() * 2.0 );

            drawTitle( "Polygons"_ostr );

            int sides= 3;
            for (int i= 1; i <= 4; i++)
            {
                drawRegularPolygon(35*i, 35, sides, 15);
                sides++;
            }

            maRenderState = std::move(aOldRenderState); // pop
        }

        void drawWidgets() // FIXME: prolly makes no sense
        {
            rendering::RenderState aOldRenderState = maRenderState; // push
            translate( maBox.Width() * 2.0, maBox.Height() * 2.0 );

            drawTitle( "Widgets"_ostr );

            maRenderState = std::move(aOldRenderState); // pop
        }
};

}

void TestWindow::Paint(vcl::RenderContext&, const tools::Rectangle&)
{
    try
    {
        uno::Reference< rendering::XCanvas > xVDevCanvas( GetOutDev()->GetCanvas(),
                                                          uno::UNO_SET_THROW );
        uno::Reference< rendering::XGraphicDevice > xVDevDevice( xVDevCanvas->getDevice(),
                                                                 uno::UNO_SET_THROW );
        DemoRenderer aVDevRenderer( xVDevDevice, xVDevCanvas, GetSizePixel());
        xVDevCanvas->clear();
        aVDevRenderer.drawGrid();
        aVDevRenderer.drawRectangles();
        aVDevRenderer.drawEllipses();
        aVDevRenderer.drawText();
        aVDevRenderer.drawLines();
        aVDevRenderer.drawCurves();
        aVDevRenderer.drawArcs();
        aVDevRenderer.drawPolygons();

        uno::Reference< rendering::XCanvas > xCanvas( GetOutDev()->GetSpriteCanvas(),
                                                          uno::UNO_QUERY_THROW );
        uno::Reference< rendering::XGraphicDevice > xDevice( xCanvas->getDevice(),
                                                             uno::UNO_SET_THROW );

        DemoRenderer aRenderer( xDevice, xCanvas, GetSizePixel() );
        xCanvas->clear();
        aRenderer.drawGrid();
        aRenderer.drawRectangles();
        aRenderer.drawEllipses();
        aRenderer.drawText();
        aRenderer.drawLines();
        aRenderer.drawCurves();
        aRenderer.drawArcs();
        aRenderer.drawPolygons();
        aRenderer.drawWidgets();
        aRenderer.drawImages();

        // check whether virdev actually contained something
        uno::Reference< rendering::XBitmap > xBitmap(xVDevCanvas, uno::UNO_QUERY);
        if( !xBitmap.is() )
            return;

        aRenderer.maRenderState.AffineTransform.m02 += 100;
        aRenderer.maRenderState.AffineTransform.m12 += 100;
        xCanvas->drawBitmap(xBitmap, aRenderer.maViewState, aRenderer.maRenderState);

        uno::Reference< rendering::XSpriteCanvas > xSpriteCanvas( xCanvas,
                                                                  uno::UNO_QUERY );
        if( xSpriteCanvas.is() )
            xSpriteCanvas->updateScreen( true ); // without
                                                     // updateScreen(),
                                                     // nothing is
                                                     // visible
    }
    catch (const uno::Exception &e)
    {
        fprintf( stderr, "Exception '%s' thrown\n" ,
                 OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
    }
}

namespace {

class DemoApp : public Application
{
public:
    virtual int Main() override;
    virtual void Exception(ExceptionCategory nCategory) override;

protected:
    void Init() override;
    void DeInit() override;
};

}

int DemoApp::Main()
{
    bool bHelp = false;

    for( unsigned int i = 0; i < GetCommandLineParamCount(); i++ )
    {
        OUString aParam = GetCommandLineParam( i );

        if( aParam == "--help" || aParam == "-h" )
            bHelp = true;
    }

    if( bHelp )
    {
        PrintHelp();
        return 1;
    }

    ScopedVclPtr<TestWindow> aWindow = VclPtr<TestWindow>::Create();
    suppress_fun_call_w_exception(aWindow->Show());

    Application::Execute();
    return 0;
}

void DemoApp::Exception( ExceptionCategory )
{
}

void DemoApp::Init()
{
    try
    {
        uno::Reference<uno::XComponentContext> xComponentContext
            = ::cppu::defaultBootstrap_InitialComponentContext();
        uno::Reference<lang::XMultiServiceFactory> xMSF;
        xMSF.set(xComponentContext->getServiceManager(), uno::UNO_QUERY);
        if(!xMSF.is())
            Application::Abort(u"Bootstrap failure - no service manager"_ustr);

        ::comphelper::setProcessServiceFactory(xMSF);
    }
    catch (const uno::Exception &e)
    {
        Application::Abort("Bootstrap exception " + e.Message);
    }
}

void DemoApp::DeInit()
{
    uno::Reference< lang::XComponent >(
        comphelper::getProcessComponentContext(),
        uno::UNO_QUERY_THROW)-> dispose();
    ::comphelper::setProcessServiceFactory(nullptr);
}

void vclmain::createApplication()
{
    static DemoApp aApp;
}

// TODO
//   - bouncing clip-rectangle mode - bounce a clip-rect around the window...
//   - complete all of pre-existing canvas bits
//   - affine transform tweakage...

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
