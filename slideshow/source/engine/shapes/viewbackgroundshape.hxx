/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewbackgroundshape.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 14:55:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
