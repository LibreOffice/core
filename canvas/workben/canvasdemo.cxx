/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */




#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/canvastools.hxx>

#include <rtl/bootstrap.hxx>
#include <sal/macros.h>

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/FillRule.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>

#include <stdio.h>
#include <unistd.h>



using namespace ::com::sun::star;


class DemoApp : public Application
{
public:
    virtual void Main();
    virtual USHORT  Exception( USHORT nError );
};

static void PrintHelp()
{
    fprintf( stdout, "canvasdemo - Exercise the new canvas impl\n" );
}

class TestWindow : public Dialog
{
    public:
        TestWindow() : Dialog( (Window *) NULL )
        {
            SetText( OUString( "Canvas test" ) );
            SetSizePixel( Size( 600, 450 ) );
            EnablePaint( true );
            Show();
        }
        virtual ~TestWindow() {}
        virtual void MouseButtonUp( const MouseEvent& /*rMEvt*/ )
        {
            
            EndDialog();
        }
        virtual void Paint( const Rectangle& rRect );
};

class DemoRenderer
{
    public:
        Size maSize;
        Size maBox;
        rendering::ViewState   maViewState;
        rendering::RenderState maRenderState;
        uno::Sequence< double > maColorBlack;
        uno::Sequence< double > maColorWhite;
        uno::Sequence< double > maColorRed;
        uno::Reference< rendering::XCanvas > mxCanvas;
        uno::Reference< rendering::XCanvasFont > mxDefaultFont;
        uno::Reference< rendering::XGraphicDevice > mxDevice;

        DemoRenderer( uno::Reference< rendering::XGraphicDevice > xDevice,
                      uno::Reference< rendering::XCanvas > xCanvas,
                      Size aSize ) :
            maSize(aSize),
            maBox(),
            maViewState(),
            maRenderState(),
            maColorBlack( vcl::unotools::colorToStdColorSpaceSequence( Color(COL_BLACK)) ),
            maColorWhite( vcl::unotools::colorToStdColorSpaceSequence( Color(COL_WHITE)) ),
            maColorRed( vcl::unotools::colorToStdColorSpaceSequence( Color(COL_RED)) ),
            mxCanvas(xCanvas),
            mxDefaultFont(),
            mxDevice( xDevice )
        {
            
            geometry::AffineMatrix2D aUnit( 1,0, 0,
                                            0,1, 0 );
            maViewState.AffineTransform = aUnit;
            maRenderState.AffineTransform = aUnit;
            maRenderState.DeviceColor = maColorBlack;

            
            
            
            
            maRenderState.CompositeOperation = rendering::CompositeOperation::OVER;

            maBox.Width() = aSize.Width() / 3;
            maBox.Height() = aSize.Height() / 3;

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
            double d, dIncr = maSize.Width() / 3;
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

        void drawRect( Rectangle rRect, uno::Sequence< double > &aColor, int /*nWidth*/ )
        {
            uno::Sequence< geometry::RealPoint2D > aPoints(4);
            uno::Reference< rendering::XLinePolyPolygon2D > xPoly;

            aPoints[0] = geometry::RealPoint2D( rRect.Left(),  rRect.Top() );
            aPoints[1] = geometry::RealPoint2D( rRect.Left(),  rRect.Bottom() );
            aPoints[2] = geometry::RealPoint2D( rRect.Right(), rRect.Bottom() );
            aPoints[3] = geometry::RealPoint2D( rRect.Right(), rRect.Top() );

            uno::Sequence< uno::Sequence< geometry::RealPoint2D > > aPolys(1);
            aPolys[0] = aPoints;
            xPoly = mxDevice->createCompatibleLinePolyPolygon( aPolys );
            xPoly->setClosed( 0, true );
            uno::Reference< rendering::XPolyPolygon2D> xPP( xPoly, uno::UNO_QUERY );

            rendering::RenderState aRenderState( maRenderState );
            aRenderState.DeviceColor = aColor;
            mxCanvas->drawPolyPolygon( xPP, maViewState, aRenderState );
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
            double a;

            rendering::RenderState maOldRenderState = maRenderState; 
            translate( center_x, center_y );

            for (i = 0; i < VERTICES; i++)
            {
                a = 2.0 * M_PI * i / VERTICES;
                geometry::RealPoint2D aSrc( RADIUS * cos (a), RADIUS * sin (a) );

                for (j = i + 1; j < VERTICES; j++)
                {
                    a = 2.0 * M_PI * j / VERTICES;


                    mxCanvas->drawLine( aSrc,
                                        geometry::RealPoint2D( RADIUS * cos (a),
                                                               RADIUS * sin (a) ),
                                        maViewState, maRenderState );
                }
            }

            maRenderState = maOldRenderState; 
        }

        void drawHilbert( double anchor_x, double anchor_y )
        {
            const double SCALE=7.0;
            const char hilbert[] = "urdrrulurulldluuruluurdrurddldrrruluurdrurddldrddlulldrdldrrurd";
            int nLength = SAL_N_ELEMENTS( hilbert );

            uno::Sequence< geometry::RealPoint2D > aPoints( nLength );
            uno::Reference< rendering::XLinePolyPolygon2D > xPoly;

            aPoints[0] = geometry::RealPoint2D( anchor_x, anchor_y );
            for (int i = 0; i < nLength; i++ )
            {
                switch( hilbert[i] )
                {
                    case 'u':
                        aPoints[i+1] = geometry::RealPoint2D( aPoints[i].X,
                                                            aPoints[i].Y - SCALE );
                        break;
                    case 'd':
                        aPoints[i+1] = geometry::RealPoint2D( aPoints[i].X,
                                                            aPoints[i].Y + SCALE );
                        break;
                    case 'l':
                        aPoints[i+1] = geometry::RealPoint2D( aPoints[i].X - SCALE,
                                                            aPoints[i].Y );
                        break;
                    case 'r':
                        aPoints[i+1] = geometry::RealPoint2D( aPoints[i].X + SCALE,
                                                            aPoints[i].Y );
                        break;
                }
            }

            uno::Sequence< uno::Sequence< geometry::RealPoint2D > > aPolys(1);
            aPolys[0] = aPoints;

            xPoly = mxDevice->createCompatibleLinePolyPolygon( aPolys );
            xPoly->setClosed( 0, false );
            uno::Reference< rendering::XPolyPolygon2D> xPP( xPoly, uno::UNO_QUERY );

            rendering::RenderState aRenderState( maRenderState );
            aRenderState.DeviceColor = maColorRed;

            rendering::StrokeAttributes aStrokeAttrs;
            aStrokeAttrs.StrokeWidth = 4.0;
            aStrokeAttrs.MiterLimit = 2.0; 
            aStrokeAttrs.StartCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.EndCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.JoinType = rendering::PathJoinType::MITER;
            
            
            mxCanvas->strokePolyPolygon( xPP, maViewState, aRenderState, aStrokeAttrs );
            
            
        }

        void drawTitle( OString aTitle )
        {
            
            double nStringWidth = aTitle.getLength() * 8.0;
            drawStringAt ( aTitle, (maBox.Width() - nStringWidth) / 2, 15 );
        }

        void drawRectangles()
        {
            rendering::RenderState maOldRenderState = maRenderState; 

            drawTitle( OString( "Rectangles" ) );

            drawRect( Rectangle( 20, 30, 70, 60 ), maColorRed, 8 );
            
            drawRect( Rectangle( 90, 40, 180, 100 ), maColorBlack, 4 );
            
            drawRect( Rectangle( 10, 80, 80, 140 ), maColorBlack, 1 );

            maRenderState = maOldRenderState; 
        }

        void drawEllipses()
        {
            rendering::RenderState maOldRenderState = maRenderState; 
            translate( maBox.Width(), 0.0 );

            drawTitle( OString( "Ellipses" ) );

            const basegfx::B2DPoint aCenter( maBox.Width()*.5,
                                             maBox.Height()*.5 );
            const basegfx::B2DPoint aRadii( maBox.Width()*.3,
                                            maBox.Height()*.3 );
            const basegfx::B2DPolygon& rEllipse(
                basegfx::tools::createPolygonFromEllipse( aCenter,
                                                          aRadii.getX(),
                                                          aRadii.getY() ));

            uno::Reference< rendering::XPolyPolygon2D > xPoly(
                basegfx::unotools::xPolyPolygonFromB2DPolygon(mxDevice,
                                                              rEllipse) );

            rendering::StrokeAttributes aStrokeAttrs;
            aStrokeAttrs.StrokeWidth = 4.0;
            aStrokeAttrs.MiterLimit = 2.0; 
            aStrokeAttrs.StartCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.EndCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.JoinType = rendering::PathJoinType::MITER;
            mxCanvas->strokePolyPolygon( xPoly, maViewState, maRenderState, aStrokeAttrs );

            maRenderState = maOldRenderState; 
        }

        void drawText()
        {
            rendering::RenderState maOldRenderState = maRenderState; 
            translate( maBox.Width() * 2.0, 0.0 );

            drawTitle( OString( "Text" ) );

            translate( 0.0,
                       maBox.Height() * .5 );
            drawTitle( OString( "This is lame" ) );

            maRenderState = maOldRenderState; 
        }

        void drawImages()
        {
            rendering::RenderState maOldRenderState = maRenderState; 
            translate( 0.0, maBox.Height() );

            drawTitle( OString( "Images" ) );

            uno::Reference< rendering::XBitmap > xBitmap(mxCanvas, uno::UNO_QUERY);

            if( !xBitmap.is() )
                return;

            translate( maBox.Width()*0.1, maBox.Height()*0.2 );
            maRenderState.AffineTransform.m00 *= 4.0/15;
            maRenderState.AffineTransform.m11 *= 3.0/15;

            mxCanvas->drawBitmap(xBitmap, maViewState, maRenderState);

            
            
            
            
            
            
            //
            
            
            
            

            maRenderState = maOldRenderState; 
        }

        void drawLines()
        {
            rendering::RenderState maOldRenderState = maRenderState; 
            translate( maBox.Width(), maBox.Height() );

            drawTitle( OString( "Lines" ) );

            drawPolishDiamond( 70.0, 80.0 );
            drawHilbert( 140.0, 140.0 );

            maRenderState = maOldRenderState; 
        }

        void drawCurves()
        {
            rendering::RenderState maOldRenderState = maRenderState; 
            translate( maBox.Width() * 2.0, maBox.Height() );

            drawTitle( OString( "Curves" ) );

            translate( maBox.Width() * .5, maBox.Height() * .5 );

            const double r= 30.0;
            const int num_curves = 3;

            
            uno::Sequence< geometry::RealBezierSegment2D > aBeziers (num_curves);
            uno::Reference< rendering::XBezierPolyPolygon2D > xPoly;

            for (int i= 0; i < num_curves; i++)
                aBeziers[i]= geometry::RealBezierSegment2D( r * cos(i*2*M_PI/num_curves), 
                                                            r * sin(i*2*M_PI/num_curves), 
                                                            r * 2 * cos((i*2*M_PI + 2*M_PI)/num_curves),  
                                                            r * 2 * sin((i*2*M_PI + 2*M_PI)/num_curves),  
                                                            r * 2 * cos((i*2*M_PI + 2*M_PI)/num_curves),  
                                                            r * 2 * sin((i*2*M_PI + 2*M_PI)/num_curves)); 
            uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > > aPolys(1);
            aPolys[0] = aBeziers;
            xPoly = mxDevice->createCompatibleBezierPolyPolygon(aPolys);
            xPoly->setClosed( 0, true );
            
            
            uno::Reference< rendering::XPolyPolygon2D> xPP( xPoly, uno::UNO_QUERY );

            rendering::StrokeAttributes aStrokeAttrs;
            aStrokeAttrs.StrokeWidth = 4.0;
            aStrokeAttrs.MiterLimit = 2.0; 
            aStrokeAttrs.StartCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.EndCapType = rendering::PathCapType::BUTT;
            aStrokeAttrs.JoinType = rendering::PathJoinType::MITER;
            mxCanvas->strokePolyPolygon( xPP, maViewState, maRenderState, aStrokeAttrs );
            
            

            maRenderState = maOldRenderState; 
        }

    double gimmerand()
        {
            return (double)(rand()) / RAND_MAX * 100 + 50;
        }

        void drawArcs()
        {
            rendering::RenderState maOldRenderState = maRenderState; 
            translate( 0.0, maBox.Height() * 2.0 );

            drawTitle( OString( "Arcs" ) );


            
            
            
            
            double ax;
            double ay;
            double bx;
            double by;
            bx= gimmerand();
            by= gimmerand();

            for (int i= 0; i < 1; i++)
            {
                
                ax= bx;
                ay= by;
                
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
                 
                geometry::RealBezierSegment2D aBezierSegment(
                    ax, 
                    ay, 
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
            maRenderState = maOldRenderState; 
        }


    void drawRegularPolygon(double centerx, double centery, int sides, double r)
        {
            
            uno::Sequence< geometry::RealPoint2D > aPoints (sides);
            uno::Reference< rendering::XLinePolyPolygon2D > xPoly;

            for (int i= 0; i < sides; i++)
            {
                aPoints[i]= geometry::RealPoint2D( centerx + r * cos(i*2 * M_PI/sides),
                                                   centery + r * sin(i*2 * M_PI/sides));
            }
            uno::Sequence< uno::Sequence< geometry::RealPoint2D > > aPolys(1);
            aPolys[0] = aPoints;
            xPoly = mxDevice->createCompatibleLinePolyPolygon( aPolys );
            xPoly->setClosed( 0, true );
            rendering::RenderState aRenderState( maRenderState );
            aRenderState.DeviceColor = maColorRed;
            uno::Reference< rendering::XPolyPolygon2D> xPP( xPoly, uno::UNO_QUERY );
            mxCanvas->drawPolyPolygon( xPP, maViewState, aRenderState);
            mxCanvas->fillPolyPolygon( xPP,
                                       maViewState,
                                       aRenderState );
        }

        void drawPolygons()
        {
            rendering::RenderState maOldRenderState = maRenderState; 
            translate( maBox.Width() * 1.0, maBox.Height() * 2.0 );

            drawTitle( OString( "Polgyons" ) );

            int sides= 3;
            for (int i= 1; i <= 4; i++)
            {
                drawRegularPolygon(35*i, 35, sides, 15);
                sides++;
            }

            maRenderState = maOldRenderState; 
        }

        void drawWidgets() 
        {
            rendering::RenderState maOldRenderState = maRenderState; 
            translate( maBox.Width() * 2.0, maBox.Height() * 2.0 );

            drawTitle( OString( "Widgets" ) );

            maRenderState = maOldRenderState; 
        }
};


void TestWindow::Paint( const Rectangle& /*rRect*/ )
{
    try
    {
        const Size aVDevSize(300,300);
        VirtualDevice aVDev(*this);
        aVDev.SetOutputSizePixel(aVDevSize);
        uno::Reference< rendering::XCanvas > xVDevCanvas( aVDev.GetCanvas(),
                                                          uno::UNO_QUERY_THROW );
        uno::Reference< rendering::XGraphicDevice > xVDevDevice( xVDevCanvas->getDevice(),
                                                                 uno::UNO_QUERY_THROW );
        DemoRenderer aVDevRenderer( xVDevDevice, xVDevCanvas, aVDevSize);
        xVDevCanvas->clear();
        aVDevRenderer.drawGrid();
        aVDevRenderer.drawRectangles();
        aVDevRenderer.drawEllipses();
        aVDevRenderer.drawText();
        aVDevRenderer.drawLines();
        aVDevRenderer.drawCurves();
        aVDevRenderer.drawArcs();
        aVDevRenderer.drawPolygons();

        uno::Reference< rendering::XCanvas > xCanvas( GetSpriteCanvas(),
                                                          uno::UNO_QUERY_THROW );
        uno::Reference< rendering::XGraphicDevice > xDevice( xCanvas->getDevice(),
                                                             uno::UNO_QUERY_THROW );

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

        
        uno::Reference< rendering::XBitmap > xBitmap(xVDevCanvas, uno::UNO_QUERY);
        if( !xBitmap.is() )
            return;

        aRenderer.maRenderState.AffineTransform.m02 += 100;
        aRenderer.maRenderState.AffineTransform.m12 += 100;
        xCanvas->drawBitmap(xBitmap, aRenderer.maViewState, aRenderer.maRenderState);

        uno::Reference< rendering::XSpriteCanvas > xSpriteCanvas( xCanvas,
                                                                  uno::UNO_QUERY );
        if( xSpriteCanvas.is() )
            xSpriteCanvas->updateScreen( sal_True ); 
                                                     
                                                     
                                                     
    }
    catch (const uno::Exception &e)
    {
        fprintf( stderr, "Exception '%s' thrown\n" ,
                 OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
    }
}

USHORT DemoApp::Exception( USHORT nError )
{
    switch( nError & EXC_MAJORTYPE )
    {
        case EXC_RSCNOTLOADED:
            Abort( "Error: could not load language resources.\nPlease check your installation.\n" );
            break;
    }
    return 0;
}

void DemoApp::Main()
{
    bool bHelp = false;

    for( USHORT i = 0; i < GetCommandLineParamCount(); i++ )
    {
        OUString aParam = GetCommandLineParam( i );

        if( aParam == "--help" || aParam == "-h" )
            bHelp = true;
    }

    if( bHelp )
    {
        PrintHelp();
        return;
    }

    
    
    
    uno::Reference< lang::XMultiServiceFactory > xFactory;
    try
    {
        uno::Reference< uno::XComponentContext > xCtx = ::cppu::defaultBootstrap_InitialComponentContext();
        xFactory = uno::Reference< lang::XMultiServiceFactory >(  xCtx->getServiceManager(),
                                                                  uno::UNO_QUERY );
        if( xFactory.is() )
            ::comphelper::setProcessServiceFactory( xFactory );
    }
    catch( const uno::Exception& )
    {
    }

    if( !xFactory.is() )
    {
        fprintf( stderr, "Could not bootstrap UNO, installation must be in disorder. Exiting.\n" );
        exit( 1 );
    }

    
    
    ::ucb::UniversalContentBroker::create(
        comphelper::getProcessComponentContext() );

    InitVCL();
    TestWindow pWindow;
    pWindow.Execute();
    DeInitVCL();
}

DemoApp aDemoApp;






/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
