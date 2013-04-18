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

#ifndef _CPPCANVAS_BASEGFXFACTORY_HXX
#define _CPPCANVAS_BASEGFXFACTORY_HXX

#include <cppcanvas/canvas.hxx>
#include <cppcanvas/bitmapcanvas.hxx>
#include <cppcanvas/spritecanvas.hxx>
#include <cppcanvas/polypolygon.hxx>
#include <cppcanvas/bitmap.hxx>
#include <cppcanvas/renderer.hxx>
#include <cppcanvas/text.hxx>
#include <cppcanvas/sprite.hxx>
#include <basegfx/vector/b2isize.hxx>

#include <cppcanvas/cppcanvasdllapi.h>

namespace basegfx
{
    class B2DPolygon;
    class B2DPolyPolygon;
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
        static BaseGfxFactory& getInstance();

        /** Create a polygon from a ::basegfx::B2DPolygon

            The created polygon initially has the same size in user
            coordinate space as the source polygon
         */
        PolyPolygonSharedPtr    createPolyPolygon( const CanvasSharedPtr&, const ::basegfx::B2DPolygon& rPoly ) const;

        /** Create an uninitialized bitmap with the given size
         */
        BitmapSharedPtr         createBitmap( const CanvasSharedPtr&, const ::basegfx::B2ISize& rSize ) const;

        /** Create an uninitialized alpha bitmap with the given size
         */
        BitmapSharedPtr         createAlphaBitmap( const CanvasSharedPtr&, const ::basegfx::B2ISize& rSize ) const;

    private:
        friend struct InitInstance2;

        // singleton
        BaseGfxFactory();

        // default: disabled copy/assignment
        BaseGfxFactory(const BaseGfxFactory&);
        BaseGfxFactory& operator=( const BaseGfxFactory& );
    };

}

#endif /* _CPPCANVAS_BASEGFXFACTORY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
