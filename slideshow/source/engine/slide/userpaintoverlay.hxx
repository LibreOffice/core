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

#ifndef INCLUDED_SLIDESHOW_USERPAINTOVERLAY_HXX
#define INCLUDED_SLIDESHOW_USERPAINTOVERLAY_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>

#include "unoview.hxx"
#include "rgbcolor.hxx"

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

/* Definition of UserPaintOverlay class */

namespace slideshow
{
    namespace internal
    {
        class EventMultiplexer;
        struct SlideShowContext;

        class PaintOverlayHandler;
        typedef ::boost::shared_ptr< class UserPaintOverlay > UserPaintOverlaySharedPtr;
        typedef ::std::vector< ::cppcanvas::PolyPolygonSharedPtr> PolyPolygonVector;
        /** Slide overlay, which can be painted into by the user.

            This class registers itself at the EventMultiplexer,
            listening for mouse clicks and moves. When the mouse is
            dragged, a hand sketching in the selected color is shown.
        */
        class UserPaintOverlay : private boost::noncopyable
        {
        public:
            /** Create a UserPaintOverlay

                @param rStrokeColor
                Color to use for drawing

                @param nStrokeWidth
                Width of the stroked path
             */
            static UserPaintOverlaySharedPtr create( const RGBColor&          rStrokeColor,
                                                     double                   nStrokeWidth,
                                                     const SlideShowContext&  rContext,
                                                     const PolyPolygonVector& rPolygons,
                                                     bool                     bActive);
            ~UserPaintOverlay();
            PolyPolygonVector getPolygons();
            void drawPolygons();

            void update_settings( bool bUserPaintEnabled, RGBColor const& aUserPaintColor, double dUserPaintStrokeWidth );


        private:
            UserPaintOverlay( const RGBColor&          rStrokeColor,
                              double                   nStrokeWidth,
                              const SlideShowContext&  rContext,
                              const PolyPolygonVector& rPolygons,
                              bool                    bActive );

            ::boost::shared_ptr<PaintOverlayHandler>    mpHandler;
            EventMultiplexer&                           mrMultiplexer;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_USERPAINTOVERLAY_HXX */
