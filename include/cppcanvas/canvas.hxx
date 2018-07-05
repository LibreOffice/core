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

#ifndef INCLUDED_CPPCANVAS_CANVAS_HXX
#define INCLUDED_CPPCANVAS_CANVAS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <cppcanvas/font.hxx>
#include <cppcanvas/color.hxx>
#include <memory>

namespace basegfx
{
    class B2DHomMatrix;
    class B2DPolyPolygon;
}

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvas;
    struct ViewState;
} } } }


/* Definition of BitmapCanvas */

namespace cppcanvas
{
    class PolyPolygon;
    class Canvas;

    // forward declaration, since tools::PolyPolygon also references Canvas
    typedef std::shared_ptr< PolyPolygon > PolyPolygonSharedPtr;

    // forward declaration, since cloneCanvas() also references Canvas
    typedef std::shared_ptr< Canvas > CanvasSharedPtr;

    /** Canvas interface
     */
    class Canvas
    {
    public:
        enum
        {
            /** Extra pixel used when canvas anti-aliases.

                Enlarge the bounding box of drawing primitives by this
                amount in both dimensions, and on both sides of the
                bounds, to account for extra pixel touched outside the
                actual primitive bounding box, when the canvas
                performs anti-aliasing.
             */
            ANTIALIASING_EXTRA_SIZE=2
        };

        Canvas() = default;
        Canvas(Canvas const &) = default;
        Canvas(Canvas &&) = default;
        Canvas & operator =(Canvas const &) = default;
        Canvas & operator =(Canvas &&) = default;

        virtual ~Canvas() {}

        virtual void                             setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) = 0;
        virtual ::basegfx::B2DHomMatrix          getTransformation() const = 0;

        virtual void                             setClip( const ::basegfx::B2DPolyPolygon& rClipPoly ) = 0;
        virtual void                             setClip() = 0;

        /** Get current clip

            @return NULL, if no clip is set, otherwise the current clip poly-polygon
         */
        virtual ::basegfx::B2DPolyPolygon const* getClip() const = 0;

        virtual CanvasSharedPtr                  clone() const = 0;
        virtual void                             clear() const = 0;

        // this should be considered private. if RTTI gets enabled
        // someday, remove that to a separate interface
        virtual css::uno::Reference<
            css::rendering::XCanvas >           getUNOCanvas() const = 0;
        virtual css::rendering::ViewState       getViewState() const = 0;
    };

}

#endif // INCLUDED_CPPCANVAS_CANVAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
