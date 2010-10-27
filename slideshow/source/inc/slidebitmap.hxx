/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_SLIDESHOW_SLIDEBITMAP_HXX
#define INCLUDED_SLIDESHOW_SLIDEBITMAP_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <cppcanvas/canvas.hxx>
#include <cppcanvas/bitmap.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace com { namespace sun { namespace star { namespace rendering
{
    class XBitmap;
} } } }


/* Definition of SlideBitmap class */

namespace slideshow
{
    namespace internal
    {

        /** Little wrapper encapsulating an XBitmap

            This is to insulate us from changes to the preferred
            transport format for bitmaps (using a sole XBitmap here is
            a hack, since it is not guaranteed to work, or to work
            without data loss, across different canvases). And since
            we don't want to revert to a VCL Bitmap here, have to wait
            until basegfx bitmap tooling is ready.

            TODO(F2): Add support for Canvas-independent bitmaps
            here. Then, Slide::getInitialSlideBitmap and
            Slide::getFinalSlideBitmap must also be adapted (they no
            longer need a Canvas ptr, which is actually a hack now).
         */
        class SlideBitmap : private boost::noncopyable
        {
        public:
            SlideBitmap( const ::cppcanvas::BitmapSharedPtr& rBitmap );

            bool                draw( const ::cppcanvas::CanvasSharedPtr& rCanvas ) const;
            ::basegfx::B2ISize  getSize() const;
            ::basegfx::B2DPoint getOutputPos() const{return maOutputPos;}
            void                move( const ::basegfx::B2DPoint& rNewPos );
            void                clip( const ::basegfx::B2DPolyPolygon& rClipPoly );

            ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XBitmap >    getXBitmap();

        private:
            ::basegfx::B2DPoint                                     maOutputPos;
            ::basegfx::B2DPolyPolygon                               maClipPoly;

            // TODO(Q2): Remove UNO bitmap as the transport medium
            ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XBitmap >      mxBitmap;
        };

        typedef ::boost::shared_ptr< SlideBitmap > SlideBitmapSharedPtr;
    }
}

#endif /* INCLUDED_SLIDESHOW_SLIDEBITMAP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
