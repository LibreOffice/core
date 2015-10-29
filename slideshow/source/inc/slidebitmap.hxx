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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SLIDEBITMAP_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SLIDEBITMAP_HXX

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
            void                move( const ::basegfx::B2DPoint& rNewPos );
            void                clip( const ::basegfx::B2DPolyPolygon& rClipPoly );

            css::uno::Reference< css::rendering::XBitmap >    getXBitmap();

        private:
            ::basegfx::B2DPoint                                     maOutputPos;
            ::basegfx::B2DPolyPolygon                               maClipPoly;

            // TODO(Q2): Remove UNO bitmap as the transport medium
            css::uno::Reference< css::rendering::XBitmap >          mxBitmap;
        };

        typedef ::boost::shared_ptr< SlideBitmap > SlideBitmapSharedPtr;
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SLIDEBITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
