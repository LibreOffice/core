/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implbitmap.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:25:16 $
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

#ifndef _CPPCANVAS_IMPLBITMAP_HXX
#define _CPPCANVAS_IMPLBITMAP_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
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
