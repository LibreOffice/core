/*************************************************************************
 *
 *  $RCSfile: slidebitmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:20:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SLIDESHOW_SLIDEBITMAP_HXX
#define _SLIDESHOW_SLIDEBITMAP_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _CPPCANVAS_CANVAS_HXX
#include <cppcanvas/canvas.hxx>
#endif
#ifndef _CPPCANVAS_BITMAP_HXX
#include <cppcanvas/bitmap.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

namespace drafts { namespace com { namespace sun { namespace star { namespace rendering
{
    class XBitmap;
} } } } }


/* Definition of SlideBitmap class */

namespace presentation
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
        class SlideBitmap
        {
        public:
            SlideBitmap( const ::cppcanvas::BitmapSharedPtr& rBitmap );

            bool                draw( const ::cppcanvas::CanvasSharedPtr& rCanvas ) const;
            ::basegfx::B2ISize  getSize() const;
            void                move( const ::basegfx::B2DPoint& rNewPos );
            void                clip( const ::basegfx::B2DPolyPolygon& rClipPoly );

        private:
            // default: disabled copy/assignment
            SlideBitmap(const SlideBitmap&);
            SlideBitmap& operator=( const SlideBitmap& );

            ::basegfx::B2DPoint                                     maOutputPos;
            ::basegfx::B2DPolyPolygon                               maClipPoly;

            // TODO(Q2): Remove UNO bitmap as the transport medium
            ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::rendering::XBitmap >      mxBitmap;
        };

        typedef ::boost::shared_ptr< ::presentation::internal::SlideBitmap > SlideBitmapSharedPtr;

    }
}

#endif /* _SLIDESHOW_SLIDEBITMAP_HXX */
