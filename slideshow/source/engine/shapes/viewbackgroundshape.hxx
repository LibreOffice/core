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

#ifndef INCLUDED_SLIDESHOW_VIEWBACKGROUNDSHAPE_HXX
#define INCLUDED_SLIDESHOW_VIEWBACKGROUNDSHAPE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XBitmap.hpp>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <cppcanvas/spritecanvas.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

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
            mutable ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XBitmap >  mxBitmap;

            /// The last metafile a render object was generated for
            mutable GDIMetaFileSharedPtr                        mpLastMtf;

            /// The canvas, mpRenderer is associated with
            mutable ::basegfx::B2DHomMatrix                     maLastTransformation;

            const ::basegfx::B2DRectangle                       maBounds;
        };

        typedef ::boost::shared_ptr< ViewBackgroundShape > ViewBackgroundShapeSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_VIEWBACKGROUNDSHAPE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
