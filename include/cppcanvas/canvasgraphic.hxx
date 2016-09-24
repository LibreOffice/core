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

#ifndef INCLUDED_CPPCANVAS_CANVASGRAPHIC_HXX
#define INCLUDED_CPPCANVAS_CANVASGRAPHIC_HXX

#include <sal/types.h>
#include <cppcanvas/color.hxx>
#include <cppcanvas/canvas.hxx>
#include <memory>

namespace basegfx
{
    class B2DHomMatrix;
    class B2DPolyPolygon;
}


/* Definition of CanvasGraphic interface */

namespace cppcanvas
{
    // forward declaration, since tools::PolyPolygon also derives from CanvasGraphic
    typedef std::shared_ptr< class PolyPolygon > PolyPolygonSharedPtr;


    /** This interface defines basic properties of
        objects that can be painted on a Canvas
     */
    class CanvasGraphic
    {
    public:

        /** These enums determine how the primitive color is combined
            with the background. When performing this calculations, it
            is assumed that all color values are premultiplied with
            the corresponding alpha values (if no alpha is specified,
            1.0 is assumed). Then, the following general compositing
            operation is performed:

            C = Ca * Fa + Cb * Fb

            where C is the result color, Ca and Cb are the input
            colors, premultiplied with alpha, and Fa and Fb are
            described for the different composite modes (wherein Aa
            and Ab denote source and destination alpha, respectively).
        */
        enum CompositeOp
        {
            /// Clear destination. Fa = Fb = 0.
            CLEAR,

            /// Copy source as-is to destination. Fa = 1, Fb = 0.
            SOURCE,

            /// Leave destination as-is.  Fa = 0, Fb = 1.
            DESTINATION,

            /// Copy source over destination. Fa = 1, Fb = 1-Aa.
            OVER,

            /// Copy source under destination. Fa = 1-Ab, Fb = 1.
            UNDER,

            /// Copy source to destination, but limited to where the destination is. Fa = Ab, Fb = 0.
            INSIDE,

            /// Leave destination as is, but only where source was. Fa = 0, Fb = Aa.
            INSIDE_REVERSE,

            /// Copy source to destination, but limited to where destination is not. Fa = 1-Ab, Fb = 0.
            OUTSIDE,

            /// Leave destination as is, but only where source has not been. Fa = 0, Fb = 1-Aa.
            OUTSIDE_REVERSE,

            /// Copy source over destination, but only where destination is. Keep destination. Fa = Ab, Fb = 1-Aa.
            ATOP,

            /// Copy destination over source, but only where source is. Keep source. Fa = 1-Ab, Fb = Aa.
            ATOP_REVERSE,

            /// Take only parts where either source or destination, but not both are. Fa = 1-Ab, Fb = 1-Aa.
            XOR,

            /** simply add contributions of both source and destination. The
                resulting color values are limited to the permissible color
                range, and clipped to the maximal value, if exceeded. Fa = 1, Fb = 1.
            */
            ADD,

            /// Fa = min(1,(1-Ab)/Aa), Fb = 1
            SATURATE
        };

        virtual ~CanvasGraphic() {}

        /** Set object transformation matrix
         */
        virtual void                             setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) = 0;

        /** Set object clipping polygon
         */
        virtual void                             setClip( const ::basegfx::B2DPolyPolygon& rClipPoly ) = 0;
        /** Clear object clipping polygon
         */
        virtual void                             setClip() = 0;

        /** Set object composite mode
         */
        virtual void                             setCompositeOp( CompositeOp aOp ) = 0;

        /** Render to parent canvas

            This method renders the content to the parent canvas,
            i.e. the canvas this object was constructed for.

            @return whether the rendering finished successfully.
         */
        virtual bool draw() const = 0;

    };

    typedef std::shared_ptr< ::cppcanvas::CanvasGraphic > CanvasGraphicSharedPtr;
}

#endif // INCLUDED_CPPCANVAS_CANVASGRAPHIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
