/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basegfxfactory.hxx,v $
 * $Revision: 1.5 $
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


namespace basegfx
{
    class B2DPolygon;
    class B2DPolyPolygon;
}

namespace rtl
{
    class OUString;
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
    class BaseGfxFactory
    {
    public:
        static BaseGfxFactory& getInstance();

        /** Create a polygon from a ::basegfx::B2DPolygon

            The created polygon initially has the same size in user
            coordinate space as the source polygon
         */
        PolyPolygonSharedPtr    createPolyPolygon( const CanvasSharedPtr&, const ::basegfx::B2DPolygon& rPoly ) const;
        PolyPolygonSharedPtr    createPolyPolygon( const CanvasSharedPtr&, const ::basegfx::B2DPolyPolygon& rPoly ) const;

        /** Create an uninitialized bitmap with the given size
         */
        BitmapSharedPtr         createBitmap( const CanvasSharedPtr&, const ::basegfx::B2ISize& rSize ) const;

        /** Create an uninitialized alpha bitmap with the given size
         */
        BitmapSharedPtr         createAlphaBitmap( const CanvasSharedPtr&, const ::basegfx::B2ISize& rSize ) const;

        /** Create a text portion with the given content string
         */
        TextSharedPtr           createText( const CanvasSharedPtr&, const ::rtl::OUString& ) const;

    private:
        friend struct InitInstance2;

        // singleton
        BaseGfxFactory();
        ~BaseGfxFactory();

        // default: disabled copy/assignment
        BaseGfxFactory(const BaseGfxFactory&);
        BaseGfxFactory& operator=( const BaseGfxFactory& );
    };

}

#endif /* _CPPCANVAS_BASEGFXFACTORY_HXX */
