/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidebitmap.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 10:19:19 $
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
