/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: userpaintoverlay.cxx,v $
 * $Revision: 1.3 $
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
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <cppcanvas/basegfxfactory.hxx>

#include "activity.hxx"
#include "activitiesqueue.hxx"
#include "slideshowcontext.hxx"
#include "userpaintoverlay.hxx"
#include "mouseeventhandler.hxx"
#include "eventmultiplexer.hxx"
#include "screenupdater.hxx"
#include "vieweventhandler.hxx"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        class PaintOverlayHandler : public MouseEventHandler,
                                    public ViewEventHandler
        {
        public:
            PaintOverlayHandler( const RGBColor&         rStrokeColor,
                                 double                  nStrokeWidth,
                                 ActivitiesQueue&        rActivitiesQueue,
                                 ScreenUpdater&          rScreenUpdater,
                                 const UnoViewContainer& rViews ) :
                mrActivitiesQueue( rActivitiesQueue ),
                mrScreenUpdater( rScreenUpdater ),
                maViews(),
                maStrokeColor( rStrokeColor ),
                mnStrokeWidth( nStrokeWidth ),
                maLastPoint(),
                maLastMouseDownPos(),
                mbIsLastPointValid( false ),
                mbIsLastMouseDownPosValid( false )
            {
                std::for_each( rViews.begin(),
                               rViews.end(),
                               boost::bind( &PaintOverlayHandler::viewAdded,
                                            this,
                                            _1 ));
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

            // MouseEventHandler methods
            virtual bool handleMousePressed( const awt::MouseEvent& e )
            {
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
                mbIsLastPointValid = true;
                maLastPoint.setX( e.X );
                maLastPoint.setY( e.Y );

                return true;
            }

            virtual bool handleMouseExited( const awt::MouseEvent& )
            {
                mbIsLastPointValid = false;
                mbIsLastMouseDownPosValid = false;

                return true;
            }

            virtual bool handleMouseDragged( const awt::MouseEvent& e )
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
                            pPolyPoly->setRGBALineColor( maStrokeColor.getIntegerColor() );
                            pPolyPoly->draw();
                        }
                    }

                    // screen update necessary to show painting
                    mrScreenUpdater.notifyUpdate();
                }

                // mouse events captured
                return true;
            }

            virtual bool handleMouseMoved( const awt::MouseEvent& /*e*/ )
            {
                // not used here
                return false; // did not handle the event
            }

        private:
            ActivitiesQueue&        mrActivitiesQueue;
            ScreenUpdater&          mrScreenUpdater;
            UnoViewVector           maViews;
            RGBColor                maStrokeColor;
            double                  mnStrokeWidth;
            basegfx::B2DPoint       maLastPoint;
            basegfx::B2DPoint       maLastMouseDownPos;
            bool                    mbIsLastPointValid;
            bool                    mbIsLastMouseDownPosValid;
        };

        UserPaintOverlaySharedPtr UserPaintOverlay::create( const RGBColor&         rStrokeColor,
                                                            double                  nStrokeWidth,
                                                            const SlideShowContext& rContext )
        {
            UserPaintOverlaySharedPtr pRet( new UserPaintOverlay( rStrokeColor,
                                                                  nStrokeWidth,
                                                                  rContext ));

            return pRet;
        }

        UserPaintOverlay::UserPaintOverlay( const RGBColor&         rStrokeColor,
                                            double                  nStrokeWidth,
                                            const SlideShowContext& rContext ) :
            mpHandler( new PaintOverlayHandler( rStrokeColor,
                                                nStrokeWidth,
                                                rContext.mrActivitiesQueue,
                                                rContext.mrScreenUpdater,
                                                rContext.mrViewContainer )),
            mrMultiplexer( rContext.mrEventMultiplexer )
        {
            mrMultiplexer.addClickHandler( mpHandler, 3.0 );
            mrMultiplexer.addMouseMoveHandler( mpHandler, 3.0 );
            mrMultiplexer.addViewHandler( mpHandler );
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
            catch (uno::Exception &) {
                OSL_ENSURE( false, rtl::OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }
    }
}
