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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWBACKGROUNDSHAPE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWBACKGROUNDSHAPE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XBitmap.hpp>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <cppcanvas/spritecanvas.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "gdimtftools.hxx"
#include "viewlayer.hxx"


namespace slideshow
{
    namespace internal
    {
        /** This class is the viewable representation of a draw
            document's background, associated to a specific View

            The class is able to render the associated background on
            View implementations.
         */
        class ViewBackgroundShape : private boost::noncopyable
        {
        public:
            /** Create a ViewBackgroundShape for the given View

                @param rView
                The associated View object.

                @param rShapeBounds
                Bounds of the background shape, in document coordinate
                system.
             */
            ViewBackgroundShape( const ViewLayerSharedPtr&      rViewLayer,
                                 const ::basegfx::B2DRectangle& rShapeBounds );

            /** Query the associated view layer of this shape
             */
            ViewLayerSharedPtr getViewLayer() const;

            bool render( const GDIMetaFileSharedPtr& rMtf ) const;

        private:
            /** Prefetch bitmap for given canvas
             */
            bool prefetch( const ::cppcanvas::CanvasSharedPtr&  rDestinationCanvas,
                           const GDIMetaFileSharedPtr&          rMtf ) const;

            /** The view layer this object is part of.
             */
            ViewLayerSharedPtr                                  mpViewLayer;

            /// Generated content bitmap, already with correct output size
            mutable css::uno::Reference< css::rendering::XBitmap >  mxBitmap;

            /// The last metafile a render object was generated for
            mutable GDIMetaFileSharedPtr                        mpLastMtf;

            /// The canvas, mpRenderer is associated with
            mutable ::basegfx::B2DHomMatrix                     maLastTransformation;

            const ::basegfx::B2DRectangle                       maBounds;
        };

        typedef ::boost::shared_ptr< ViewBackgroundShape > ViewBackgroundShapeSharedPtr;

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_VIEWBACKGROUNDSHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
