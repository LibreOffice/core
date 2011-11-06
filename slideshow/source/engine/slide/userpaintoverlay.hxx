/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
