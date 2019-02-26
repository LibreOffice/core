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

#ifndef INCLUDED_CPPCANVAS_BASEGFXFACTORY_HXX
#define INCLUDED_CPPCANVAS_BASEGFXFACTORY_HXX

#include <cppcanvas/canvas.hxx>
#include <cppcanvas/polypolygon.hxx>
#include <cppcanvas/bitmap.hxx>
#include <basegfx/vector/b2isize.hxx>

#include <cppcanvas/cppcanvasdllapi.h>

namespace basegfx
{
    class B2DPolygon;
}


/* Definition of BaseGfxFactory class */

namespace cppcanvas
{
    /** The BaseGfxFactory creates Canvas objects for various basegfx
        primitives, such as polygons and bitmaps (not yet
        implemented).

        Please note that the objects created for a specific Canvas can
        only be drawn on exactly that canvas. You have to regenerate
        them for different canvases.
     */
    class CPPCANVAS_DLLPUBLIC BaseGfxFactory
    {
    public:
        /** Create a polygon from a ::basegfx::B2DPolygon

            The created polygon initially has the same size in user
            coordinate space as the source polygon
         */
        static PolyPolygonSharedPtr    createPolyPolygon( const CanvasSharedPtr&, const ::basegfx::B2DPolygon& rPoly );

        /** Create an uninitialized bitmap with the given size
         */
        static BitmapSharedPtr         createBitmap( const CanvasSharedPtr&, const ::basegfx::B2ISize& rSize );

        /** Create an uninitialized alpha bitmap with the given size
         */
        static BitmapSharedPtr         createAlphaBitmap( const CanvasSharedPtr&, const ::basegfx::B2ISize& rSize );

    private:
        BaseGfxFactory() = delete;
        BaseGfxFactory(const BaseGfxFactory&) = delete;
        BaseGfxFactory& operator=( const BaseGfxFactory& ) = delete;
    };

}

#endif // INCLUDED_CPPCANVAS_BASEGFXFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
