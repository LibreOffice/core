/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implbitmapcanvas.hxx,v $
 * $Revision: 1.7 $
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
#include <basegfx/vector/b2dsize.hxx>
#include <cppcanvas/bitmapcanvas.hxx>

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
