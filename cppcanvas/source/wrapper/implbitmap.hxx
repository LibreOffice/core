/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implbitmap.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _CPPCANVAS_IMPLBITMAP_HXX
#define _CPPCANVAS_IMPLBITMAP_HXX

#include <com/sun/star/uno/Reference.hxx>
#ifndef _COM_SUN_STAR_RENDERING_XBITMAP_HPP__
#include <com/sun/star/rendering/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP__
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#endif

#include <cppcanvas/bitmap.hxx>
#include <canvasgraphichelper.hxx>


/*Definition of ImplBitmap */

namespace cppcanvas
{

    namespace internal
    {
        // share partial CanvasGraphic implementation from CanvasGraphicHelper
        class ImplBitmap : public virtual ::cppcanvas::Bitmap, protected CanvasGraphicHelper
        {
        public:

            ImplBitmap( const CanvasSharedPtr& rParentCanvas,
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::rendering::XBitmap >& rBitmap );

            virtual ~ImplBitmap();

            // CanvasGraphic implementation (that was not already implemented by CanvasGraphicHelper)
            virtual bool draw() const;
            virtual bool drawAlphaModulated( double nAlphaModulation ) const;

            virtual BitmapCanvasSharedPtr                       getBitmapCanvas() const;

            // Bitmap implementation
            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XBitmap >  getUNOBitmap() const;

        private:
            // default: disabled copy/assignment
            ImplBitmap(const ImplBitmap&);
            ImplBitmap& operator=( const ImplBitmap& );

            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >  mxBitmap;
            BitmapCanvasSharedPtr                                                                   mpBitmapCanvas;
         };
    }
}

#endif /* _CPPCANVAS_IMPLBITMAP_HXX */
