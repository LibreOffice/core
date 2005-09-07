/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: view.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:24:33 $
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

#ifndef _SLIDESHOW_VIEW_HXX
#define _SLIDESHOW_VIEW_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _CPPCANVAS_SPRITECANVAS_HXX
#include <cppcanvas/spritecanvas.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_VECTOR_B2DSIZE_HXX
#include <basegfx/vector/b2dsize.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include <vector>

#include <viewlayer.hxx>


/* Definition of View interface */

namespace presentation
{
    namespace internal
    {
        class View : public ViewLayer
        {
        public:
            /** Create a new view layer for this view
             */
            virtual ViewLayerSharedPtr createViewLayer() const = 0;

            /** Clear the view layer area
             */
            virtual void clear() const = 0;

            /** Query whether view content is still valid.

                This method returns false, if the view content has
                been destroyed until the last update. That might
                happen e.g. during window resizes, or when volatile
                bitmaps become reclaimed by the system. If this method
                returns false, the slideshow must assume that the
                whole view area needs a repaint. A call to clear()
                sets the content to valid again.
             */
            virtual bool isContentDestroyed() const = 0;

            /** Update screen representation from backbuffer
             */
            virtual bool updateScreen() const = 0;

            /** Get the overall view transformation.

                This method should <em>not</em> simply return the
                underlying canvas' transformation, but rather provide
                a layer above that. This enables clients of the
                slideshow to set their own user space transformation
                at the canvas, whilst the slideshow adds their
                transformation on top of that. Concretely, this method
                returns the user transform (implicitely calculated
                from the setViewSize() method), combined with the view
                transformation.
            */
            virtual ::basegfx::B2DHomMatrix getTransformation() const = 0;

            /** Set the size of the user view coordinate system.

                This method sets the width and height of the view in
                document coordinates (i.e. 'logical' coordinates). The
                resulting transformation is then concatenated with the
                underlying view transformation, returned by the
                getTransformation() method.
            */
            virtual void setViewSize( const ::basegfx::B2DSize& ) = 0;

            /** Set clipping on this view.

                @param rClip
                Clip poly-polygon to set. The polygon is interpreted
                in the user coordinate system, i.e. the view has the
                size as given by setViewSize().
             */
            virtual void setClip( const ::basegfx::B2DPolyPolygon& rClip ) = 0;

            /** Change the view's mouse cursor.

                @param nPointerShape
                One of the ::com::sun::star::awt::SystemPointer
                constant group members.
             */
            virtual void setMouseCursor( sal_Int16 nPointerShape ) = 0;
        };

        typedef ::boost::shared_ptr< View >     ViewSharedPtr;
        typedef ::std::vector< ViewSharedPtr >  ViewVector;
    }
}

#endif /* _SLIDESHOW_VIEW_HXX */
