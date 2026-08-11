/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <osl/mutex.hxx>

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <com/sun/star/uno/Reference.hxx>


class Point;

namespace com::sun::star::geometry
{
    struct RealPoint2D;
    struct Matrix2D;
}

namespace vclcanvas
{
    struct RenderState;
    struct ViewState;
}

namespace vclcanvastools
{
        /** Setup VCL font and output position

            @returns false, if no text output should happen
         */
        bool setupFontTransform( ::Point&                              o_rPoint,
                                 vcl::Font&                            io_rVCLFont,
                                 const ::vclcanvas::ViewState&      viewState,
                                 const ::vclcanvas::RenderState&    renderState,
                                 ::OutputDevice const &                rOutDev );

        void setupFontWidth(const css::geometry::Matrix2D& rFontMatrix,
                            vcl::Font&                     rFont,
                            OutputDevice&                  rOutDev);

        /** Predicate, to determine whether polygon is actually an axis-aligned rectangle

            @return true, if the polygon is a rectangle.
         */
        bool isRectangle( const ::tools::PolyPolygon& rPolyPoly );


        class OutDevStateKeeper
        {
        public:
            explicit OutDevStateKeeper( OutputDevice& rOutDev ) :
                mrOutDev( rOutDev ),
                mbMappingWasEnabled( mrOutDev.IsMapModeEnabled() ),
                mnAntiAliasing( mrOutDev.GetAntialiasing() )
            {
                mrOutDev.Push();
                mrOutDev.EnableMapMode(false);
                mrOutDev.SetAntialiasing( AntialiasingFlags::Enable );
            }

            ~OutDevStateKeeper()
            {
                mrOutDev.EnableMapMode( mbMappingWasEnabled );
                mrOutDev.SetAntialiasing( mnAntiAliasing );
                mrOutDev.Pop();
            }

        private:
            OutputDevice&           mrOutDev;
            const bool              mbMappingWasEnabled;
            const AntialiasingFlags mnAntiAliasing;
        };

        ::Point mapB2DPoint( const ::basegfx::B2DPoint&  rPoint,
                             const ::vclcanvas::ViewState&   rViewState,
                             const ::vclcanvas::RenderState& rRenderState );

        ::tools::PolyPolygon mapPolyPolygon( const ::basegfx::B2DPolyPolygon&                          rPoly,
                                      const ::vclcanvas::ViewState&     rViewState,
                                      const ::vclcanvas::RenderState&   rRenderState );

        ::Bitmap transformBitmap( const ::Bitmap&                                   rBitmap,
                                  const ::basegfx::B2DHomMatrix&                    rTransform );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
