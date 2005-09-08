/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implbitmapcanvas.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:25:56 $
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

#ifndef _CPPCANVAS_IMPLBITMAPCANVAS_HXX
#define _CPPCANVAS_IMPLBITMAPCANVAS_HXX

#ifndef _COM_SUN_STAR_RENDERING_XBITMAPCANVAS_HPP__
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XBITMAP_HPP__
#include <com/sun/star/rendering/XBitmap.hpp>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _BGFX_VECTOR_B2DSIZE_HXX
#include <basegfx/vector/b2dsize.hxx>
#endif

#ifndef _CPPCANVAS_BITMAPCANVAS_HXX
#include <cppcanvas/bitmapcanvas.hxx>
#endif

#include <implcanvas.hxx>


namespace cppcanvas
{
    namespace internal
    {
        // share Canvas implementation from ImplCanvas
        class ImplBitmapCanvas : public virtual BitmapCanvas, protected virtual ImplCanvas
        {
        public:
            ImplBitmapCanvas( const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::rendering::XBitmapCanvas >& rCanvas );
            virtual ~ImplBitmapCanvas();

            virtual ::basegfx::B2ISize      getSize() const;

            virtual CanvasSharedPtr         clone() const;

            // take compiler-provided default copy constructor
            //ImplBitmapCanvas(const ImplBitmapCanvas&);

        private:
            // default: disabled assignment
            ImplBitmapCanvas& operator=( const ImplBitmapCanvas& );

            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas >    mxBitmapCanvas;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >          mxBitmap;
        };
    }
}

#endif /* _CPPCANVAS_IMPLBITMAPCANVAS_HXX */
