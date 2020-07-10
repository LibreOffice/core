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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_USERPAINTOVERLAY_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_USERPAINTOVERLAY_HXX

#include <cppcanvas/canvasgraphic.hxx>

#include <rgbcolor.hxx>

#include <memory>
#include <vector>

/* Definition of UserPaintOverlay class */

namespace slideshow::internal
    {
        class EventMultiplexer;
        struct SlideShowContext;

        class PaintOverlayHandler;
        typedef ::std::shared_ptr< class UserPaintOverlay > UserPaintOverlaySharedPtr;
        typedef ::std::vector< ::cppcanvas::PolyPolygonSharedPtr> PolyPolygonVector;
        /** Slide overlay, which can be painted into by the user.

            This class registers itself at the EventMultiplexer,
            listening for mouse clicks and moves. When the mouse is
            dragged, a hand sketching in the selected color is shown.
        */
        class UserPaintOverlay
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
            UserPaintOverlay(const UserPaintOverlay&) = delete;
            UserPaintOverlay& operator=(const UserPaintOverlay&) = delete;
            PolyPolygonVector const & getPolygons() const;
            void drawPolygons();

        private:
            UserPaintOverlay( const RGBColor&          rStrokeColor,
                              double                   nStrokeWidth,
                              const SlideShowContext&  rContext,
                              const PolyPolygonVector& rPolygons,
                              bool                    bActive );

            ::std::shared_ptr<PaintOverlayHandler>    mpHandler;
            EventMultiplexer&                           mrMultiplexer;
        };

}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_USERPAINTOVERLAY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
