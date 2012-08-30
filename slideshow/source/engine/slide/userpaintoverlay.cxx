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


#include <canvas/debug.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <cppcanvas/basegfxfactory.hxx>

#include "activity.hxx"
#include "slideshowcontext.hxx"
#include "userpaintoverlay.hxx"
#include "mouseeventhandler.hxx"
#include "eventmultiplexer.hxx"
#include "screenupdater.hxx"
#include "vieweventhandler.hxx"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include "slide.hxx"
#include "cursormanager.hxx"

using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        class PaintOverlayHandler : public MouseEventHandler,
                                    public ViewEventHandler,
                    public UserPaintEventHandler
        {
        public:
            PaintOverlayHandler( const RGBColor&          rStrokeColor,
                                 double                   nStrokeWidth,
                                 ScreenUpdater&           rScreenUpdater,
                                 const UnoViewContainer&  rViews,
                                 Slide&                   rSlide,
                                 const PolyPolygonVector& rPolygons,
                                 bool                     bActive ) :
                mrScreenUpdater( rScreenUpdater ),
                maViews(),
                maPolygons( rPolygons ),
                maStrokeColor( rStrokeColor ),
                mnStrokeWidth( nStrokeWidth ),
                maLastPoint(),
                maLastMouseDownPos(),
                mbIsLastPointValid( false ),
                mbIsLastMouseDownPosValid( false ),
                //handle the "remove all ink from slide" mode of erasing
                mbIsEraseAllModeActivated( false ),
                //handle the "remove stroke by stroke" mode of erasing
                mbIsEraseModeActivated( false ),
                mrSlide(rSlide),
                mnSize(100),
                mbActive( bActive )
            {
                std::for_each( rViews.begin(),
                               rViews.end(),
                               boost::bind( &PaintOverlayHandler::viewAdded,
                                            this,
                                            _1 ));
                drawPolygons();
            }

            virtual void dispose()
            {
                maViews.clear();
            }

            // ViewEventHandler methods
            virtual void viewAdded( const UnoViewSharedPtr& rView )
            {
                maViews.push_back( rView );
            }

            virtual void viewRemoved( const UnoViewSharedPtr& rView )
            {
                maViews.erase( ::std::remove( maViews.begin(),
                                              maViews.end(),
                                              rView ) );
            }

            virtual void viewChanged( const UnoViewSharedPtr& /*rView*/ )
            {
                // TODO(F2): for persistent drawings, need to store
                // polygon and repaint here.
            }

            virtual void viewsChanged()
            {
                // TODO(F2): for persistent drawings, need to store
                // polygon and repaint here.
            }

            bool colorChanged( RGBColor const& rUserColor )
            {
                mbIsLastPointValid = false;
                mbActive = true;
                this->maStrokeColor = rUserColor;
                this->mbIsEraseModeActivated = false;
                return true;
            }

            bool widthChanged( double nUserStrokeWidth )
            {
                this->mnStrokeWidth = nUserStrokeWidth;
                mbIsEraseModeActivated = false;
                return true;
            }

            void repaintWithoutPolygons()
            {
                    // must get access to the instance to erase all polygon
                    for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
                        aIter!=aEnd;
                        ++aIter )
                    {
                        // fully clear view content to background color
                        //(*aIter)->getCanvas()->clear();

                        //get via SlideImpl instance the bitmap of the slide unmodified to redraw it
                        SlideBitmapSharedPtr         pBitmap( mrSlide.getCurrentSlideBitmap( (*aIter) ) );
                        ::cppcanvas::CanvasSharedPtr pCanvas( (*aIter)->getCanvas() );

                        const ::basegfx::B2DHomMatrix   aViewTransform( (*aIter)->getTransformation() );
                        const ::basegfx::B2DPoint       aOutPosPixel( aViewTransform * ::basegfx::B2DPoint() );

                        // setup a canvas with device coordinate space, the slide
                        // bitmap already has the correct dimension.
                        ::cppcanvas::CanvasSharedPtr pDevicePixelCanvas( pCanvas->clone() );

                        pDevicePixelCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

                        // render at given output position
                        pBitmap->move( aOutPosPixel );

                        // clear clip (might have been changed, e.g. from comb
                        // transition)
                        pBitmap->clip( ::basegfx::B2DPolyPolygon() );
                        pBitmap->draw( pDevicePixelCanvas );

                        mrScreenUpdater.notifyUpdate(*aIter,true);
                    }
            }

            bool eraseAllInkChanged( bool const& rEraseAllInk )
            {
                this->mbIsEraseAllModeActivated= rEraseAllInk;
                // if the erase all mode is activated it will remove all ink from slide,
                // therefor destroy all the polygons stored
                if(mbIsEraseAllModeActivated)
                {
                    // The Erase Mode should be desactivated
                    mbIsEraseModeActivated = false;
                    repaintWithoutPolygons();
                    maPolygons.clear();
                }
            mbIsEraseAllModeActivated=false;
            return true;
            }

            bool eraseInkWidthChanged( sal_Int32 rEraseInkSize )
            {
                // Change the size
                this->mnSize=rEraseInkSize;
                // Changed to mode Erase
                this->mbIsEraseModeActivated = true;
                return true;
            }

            bool switchPenMode()
            {
                mbIsLastPointValid = false;
                mbActive = true;
                this->mbIsEraseModeActivated = false;
                return true;
            }

            bool switchEraserMode()
            {
                mbIsLastPointValid = false;
                mbActive = true;
                this->mbIsEraseModeActivated = true;
                return true;
            }

            bool disable()
            {
                mbIsLastPointValid = false;
                mbIsLastMouseDownPosValid = false;
                mbActive = false;
                return true;
            }

            //Draw all registered polygons.
            void drawPolygons()
            {
                for( PolyPolygonVector::iterator aIter=maPolygons.begin(), aEnd=maPolygons.end();
                                     aIter!=aEnd;
                                     ++aIter )
                {
                    (*aIter)->draw();
                }
                // screen update necessary to show painting
                mrScreenUpdater.notifyUpdate();
            }

            //Retrieve all registered polygons.
            PolyPolygonVector getPolygons()
            {
                return maPolygons;
            }

            // MouseEventHandler methods
            virtual bool handleMousePressed( const awt::MouseEvent& e )
            {
                if( !mbActive )
                    return false;

                if (e.Buttons == awt::MouseButton::RIGHT)
                {
                    mbIsLastPointValid = false;
                    return false;
                }

                if (e.Buttons != awt::MouseButton::LEFT)
                    return false;

                maLastMouseDownPos.setX( e.X );
                maLastMouseDownPos.setY( e.Y );
                mbIsLastMouseDownPosValid = true;

                // eat mouse click (though we don't process it
                // _directly_, it enables the drag mode
                return true;
            }

            virtual bool handleMouseReleased( const awt::MouseEvent& e )
            {
                if( !mbActive )
                    return false;

                if (e.Buttons == awt::MouseButton::RIGHT)
                {
                    mbIsLastPointValid = false;
                    return false;
                }

                if (e.Buttons != awt::MouseButton::LEFT)
                    return false;

                // check, whether up- and down press are on exactly
                // the same pixel. If that's the case, ignore the
                // click, and pass on the event to low-prio
                // handlers. This effectively permits effect
                // advancements via clicks also when user paint is
                // enabled.
                if( mbIsLastMouseDownPosValid &&
                    ::basegfx::B2DPoint( e.X,
                                         e.Y ) == maLastMouseDownPos )
                {
                    mbIsLastMouseDownPosValid = false;
                    return false;
                }

                // invalidate, next downpress will have to start a new
                // polygon.
                mbIsLastPointValid = false;

                // eat mouse click (though we don't process it
                // _directly_, it enables the drag mode
                return true;
            }

            virtual bool handleMouseEntered( const awt::MouseEvent& e )
            {
                if( !mbActive )
                    return false;

                mbIsLastPointValid = true;
                maLastPoint.setX( e.X );
                maLastPoint.setY( e.Y );

                return true;
            }

            virtual bool handleMouseExited( const awt::MouseEvent& )
            {
                if( !mbActive )
                    return false;

                mbIsLastPointValid = false;
                mbIsLastMouseDownPosValid = false;

                return true;
            }

            virtual bool handleMouseDragged( const awt::MouseEvent& e )
            {
                if( !mbActive )
                    return false;

                if (e.Buttons == awt::MouseButton::RIGHT)
                {
                    mbIsLastPointValid = false;
                    return false;
                }

                if(mbIsEraseModeActivated)
                {
                    //define the last point as an object
                    //we suppose that there's no way this point could be valid
                    ::basegfx::B2DPolygon aPoly;

                    maLastPoint.setX( e.X-mnSize );
                    maLastPoint.setY( e.Y-mnSize );

                    aPoly.append( maLastPoint );

                    maLastPoint.setX( e.X-mnSize );
                    maLastPoint.setY( e.Y+mnSize );

                    aPoly.append( maLastPoint );
                    maLastPoint.setX( e.X+mnSize );
                    maLastPoint.setY( e.Y+mnSize );

                    aPoly.append( maLastPoint );
                    maLastPoint.setX( e.X+mnSize );
                    maLastPoint.setY( e.Y-mnSize );

                    aPoly.append( maLastPoint );
                    maLastPoint.setX( e.X-mnSize );
                    maLastPoint.setY( e.Y-mnSize );

                    aPoly.append( maLastPoint );

                    //now we have defined a Polygon that is closed

                    //The point is to redraw the LastPoint the way it was originally on the bitmap,
                    //of the slide
            for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
                        aIter!=aEnd;
                        ++aIter )
                    {

                        //get via SlideImpl instance the bitmap of the slide unmodified to redraw it
                        SlideBitmapSharedPtr         pBitmap( mrSlide.getCurrentSlideBitmap( (*aIter) ) );
                        ::cppcanvas::CanvasSharedPtr pCanvas( (*aIter)->getCanvas() );

                        ::basegfx::B2DHomMatrix     aViewTransform( (*aIter)->getTransformation() );
                        const ::basegfx::B2DPoint       aOutPosPixel( aViewTransform * ::basegfx::B2DPoint() );

                        // setup a canvas with device coordinate space, the slide
                        // bitmap already has the correct dimension.
                        ::cppcanvas::CanvasSharedPtr pDevicePixelCanvas( pCanvas->clone() );

                        pDevicePixelCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

                        // render at given output position
                        pBitmap->move( aOutPosPixel );

                        ::basegfx::B2DPolyPolygon aPolyPoly=::basegfx::B2DPolyPolygon(aPoly);
                        aViewTransform.translate(-aOutPosPixel.getX(), -aOutPosPixel.getY());
                        aPolyPoly.transform(aViewTransform);
                        // set clip so that we just redraw a part of the canvas
                        pBitmap->clip(aPolyPoly);
                        pBitmap->draw( pDevicePixelCanvas );

                        mrScreenUpdater.notifyUpdate(*aIter,true);
                    }

        }
                else
                {
                    if( !mbIsLastPointValid )
                    {
                        mbIsLastPointValid = true;
                        maLastPoint.setX( e.X );
                        maLastPoint.setY( e.Y );
                    }
                    else
                    {
                        ::basegfx::B2DPolygon aPoly;
                        aPoly.append( maLastPoint );

                        maLastPoint.setX( e.X );
                        maLastPoint.setY( e.Y );

                        aPoly.append( maLastPoint );

                        // paint to all views
                        for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
                             aIter!=aEnd;
                             ++aIter )
                        {
                            ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
                                ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon( (*aIter)->getCanvas(),
                                                                                              aPoly ) );

                            if( pPolyPoly )
                            {
                                pPolyPoly->setStrokeWidth(mnStrokeWidth);
                                pPolyPoly->setRGBALineColor( maStrokeColor.getIntegerColor() );
                                pPolyPoly->draw();
                                maPolygons.push_back(pPolyPoly);
                            }
                        }

                        // screen update necessary to show painting
                        mrScreenUpdater.notifyUpdate();
                    }
                }
                // mouse events captured
                return true;
            }

            virtual bool handleMouseMoved( const awt::MouseEvent& /*e*/ )
            {
                // not used here
                return false; // did not handle the event
            }


            void update_settings( bool bUserPaintEnabled, RGBColor const& aUserPaintColor, double dUserPaintStrokeWidth )
            {
                maStrokeColor = aUserPaintColor;
                mnStrokeWidth = dUserPaintStrokeWidth;
                mbActive = bUserPaintEnabled;
                if( !mbActive )
                    disable();
            }

        private:
            ScreenUpdater&          mrScreenUpdater;
            UnoViewVector           maViews;
            PolyPolygonVector       maPolygons;
            RGBColor                maStrokeColor;
            double                  mnStrokeWidth;
            basegfx::B2DPoint       maLastPoint;
            basegfx::B2DPoint       maLastMouseDownPos;
            bool                    mbIsLastPointValid;
            bool                    mbIsLastMouseDownPosValid;
            // added bool for erasing purpose :
            bool                    mbIsEraseAllModeActivated;
            bool                    mbIsEraseModeActivated;
            Slide&                  mrSlide;
            sal_Int32               mnSize;
            bool                    mbActive;
        };

        UserPaintOverlaySharedPtr UserPaintOverlay::create( const RGBColor&          rStrokeColor,
                                                            double                   nStrokeWidth,
                                                            const SlideShowContext&  rContext,
                                                            const PolyPolygonVector& rPolygons,
                                                            bool                     bActive )
        {
            UserPaintOverlaySharedPtr pRet( new UserPaintOverlay( rStrokeColor,
                                                                  nStrokeWidth,
                                                                  rContext,
                                                                  rPolygons,
                                                                  bActive));

            return pRet;
        }

        UserPaintOverlay::UserPaintOverlay( const RGBColor&          rStrokeColor,
                                            double                   nStrokeWidth,
                                            const SlideShowContext&  rContext,
                                            const PolyPolygonVector& rPolygons,
                                            bool                     bActive ) :
            mpHandler( new PaintOverlayHandler( rStrokeColor,
                                                nStrokeWidth,
                                                rContext.mrScreenUpdater,
                                                rContext.mrViewContainer,
                                                //adding a link to Slide
                                                dynamic_cast<Slide&>(rContext.mrCursorManager),
                                                rPolygons, bActive )),
            mrMultiplexer( rContext.mrEventMultiplexer )
        {
            mrMultiplexer.addClickHandler( mpHandler, 3.0 );
            mrMultiplexer.addMouseMoveHandler( mpHandler, 3.0 );
            mrMultiplexer.addViewHandler( mpHandler );
            mrMultiplexer.addUserPaintHandler(mpHandler);
        }

        PolyPolygonVector UserPaintOverlay::getPolygons()
        {
            return mpHandler->getPolygons();
        }

        void UserPaintOverlay::drawPolygons()
        {
            mpHandler->drawPolygons();
        }

        UserPaintOverlay::~UserPaintOverlay()
        {
            try
            {
                mrMultiplexer.removeMouseMoveHandler( mpHandler );
                mrMultiplexer.removeClickHandler( mpHandler );
                mrMultiplexer.removeViewHandler( mpHandler );
                mpHandler->dispose();
            }
            catch (uno::Exception &)
            {
                OSL_FAIL( rtl::OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
