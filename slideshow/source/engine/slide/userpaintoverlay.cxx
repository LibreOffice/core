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
                
                mbIsEraseAllModeActivated( false ),
                
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
                
                
            }

            virtual void viewsChanged()
            {
                
                
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
                    
                    for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
                        aIter!=aEnd;
                        ++aIter )
                    {
                        
                        

                        
                        SlideBitmapSharedPtr         pBitmap( mrSlide.getCurrentSlideBitmap( (*aIter) ) );
                        ::cppcanvas::CanvasSharedPtr pCanvas( (*aIter)->getCanvas() );

                        const ::basegfx::B2DHomMatrix   aViewTransform( (*aIter)->getTransformation() );
                        const ::basegfx::B2DPoint       aOutPosPixel( aViewTransform * ::basegfx::B2DPoint() );

                        
                        
                        ::cppcanvas::CanvasSharedPtr pDevicePixelCanvas( pCanvas->clone() );

                        pDevicePixelCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

                        
                        pBitmap->move( aOutPosPixel );

                        
                        
                        pBitmap->clip( ::basegfx::B2DPolyPolygon() );
                        pBitmap->draw( pDevicePixelCanvas );

                        mrScreenUpdater.notifyUpdate(*aIter,true);
                    }
            }

            bool eraseAllInkChanged( bool const& rEraseAllInk )
            {
                this->mbIsEraseAllModeActivated= rEraseAllInk;
                
                
                if(mbIsEraseAllModeActivated)
                {
                    
                    mbIsEraseModeActivated = false;
                    repaintWithoutPolygons();
                    maPolygons.clear();
                }
            mbIsEraseAllModeActivated=false;
            return true;
            }

            bool eraseInkWidthChanged( sal_Int32 rEraseInkSize )
            {
                
                this->mnSize=rEraseInkSize;
                
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

            
            void drawPolygons()
            {
                for( PolyPolygonVector::iterator aIter=maPolygons.begin(), aEnd=maPolygons.end();
                                     aIter!=aEnd;
                                     ++aIter )
                {
                    (*aIter)->draw();
                }
                
                mrScreenUpdater.notifyUpdate();
            }

            
            PolyPolygonVector getPolygons()
            {
                return maPolygons;
            }

            
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

                
                
                
                
                
                
                if( mbIsLastMouseDownPosValid &&
                    ::basegfx::B2DPoint( e.X,
                                         e.Y ) == maLastMouseDownPos )
                {
                    mbIsLastMouseDownPosValid = false;
                    return false;
                }

                
                
                mbIsLastPointValid = false;

                
                
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

                    

                    
                    
            for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
                        aIter!=aEnd;
                        ++aIter )
                    {

                        
                        SlideBitmapSharedPtr         pBitmap( mrSlide.getCurrentSlideBitmap( (*aIter) ) );
                        ::cppcanvas::CanvasSharedPtr pCanvas( (*aIter)->getCanvas() );

                        ::basegfx::B2DHomMatrix     aViewTransform( (*aIter)->getTransformation() );
                        const ::basegfx::B2DPoint       aOutPosPixel( aViewTransform * ::basegfx::B2DPoint() );

                        
                        
                        ::cppcanvas::CanvasSharedPtr pDevicePixelCanvas( pCanvas->clone() );

                        pDevicePixelCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

                        
                        pBitmap->move( aOutPosPixel );

                        ::basegfx::B2DPolyPolygon aPolyPoly=::basegfx::B2DPolyPolygon(aPoly);
                        aViewTransform.translate(-aOutPosPixel.getX(), -aOutPosPixel.getY());
                        aPolyPoly.transform(aViewTransform);
                        
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

                        
                        mrScreenUpdater.notifyUpdate();
                    }
                }
                
                return true;
            }

            virtual bool handleMouseMoved( const awt::MouseEvent& /*e*/ )
            {
                
                return false; 
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
                OSL_FAIL( OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
