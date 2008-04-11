/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvasbitmaphelper.hxx,v $
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

#ifndef _VCLCANVAS_CANVASBITMAPHELPER_HXX_
#define _VCLCANVAS_CANVASBITMAPHELPER_HXX_

#include <canvashelper.hxx>
#include <canvas/vclwrapper.hxx>

#include <vcl/bitmapex.hxx>

#include "bitmapbackbuffer.hxx"
#include "spritecanvas.hxx"


namespace vclcanvas
{
    /** Helper class for basic canvasbitmap functionality. Extends
        CanvasHelper with some CanvasBitmap specialities, such as alpha
        support.

        Note that a plain CanvasHelper, although it does support the
        XBitmap interface, has no provision for alpha channel on VCL
        (at least no efficient one. If the alpha VDev one day gets
        part of SAL, we might change that).
     */
    class CanvasBitmapHelper : public CanvasHelper
    {
    public:
        CanvasBitmapHelper();

        /** Set a new bitmap on this helper.

            This method late-initializes the bitmap canvas helper,
            providing it with the necessary device and output
            objects. The internally stored bitmap representation is
            updated from the given bitmap, including any size
            changes. Note that the CanvasHelper does <em>not</em> take
            ownership of the SpriteCanvas object, nor does it perform
            any reference counting. Thus, to prevent reference counted
            objects from deletion, the user of this class is
            responsible for holding ref-counted references to those
            objects!

            @param rBitmap
            Content of this bitmap is used as our new content (our
            internal size is adapted to the size of the bitmap given)

            @param rDevice
            Reference device for this canvas bitmap
         */
        void init( const BitmapEx&  rBitmap,
                   SpriteCanvas&    rDevice );


        // Overridden CanvasHelper functionality
        // =====================================

        void disposing();

        ::com::sun::star::geometry::IntegerSize2D getSize();

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > queryBitmapCanvas();

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >
            getScaledBitmap( const ::com::sun::star::geometry::RealSize2D&  newSize,
                             sal_Bool                                       beFast );

        ::com::sun::star::uno::Sequence< sal_Int8 >
            getData( ::com::sun::star::rendering::IntegerBitmapLayout&      bitmapLayout,
                     const ::com::sun::star::geometry::IntegerRectangle2D&  rect );

        void setData( const ::com::sun::star::uno::Sequence< sal_Int8 >&        data,
                      const ::com::sun::star::rendering::IntegerBitmapLayout&   bitmapLayout,
                      const ::com::sun::star::geometry::IntegerRectangle2D&     rect );

        void setPixel( const ::com::sun::star::uno::Sequence< sal_Int8 >&       color,
                       const ::com::sun::star::rendering::IntegerBitmapLayout&  bitmapLayout,
                       const ::com::sun::star::geometry::IntegerPoint2D&        pos );

        ::com::sun::star::uno::Sequence< sal_Int8 >
            getPixel( ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
                      const ::com::sun::star::geometry::IntegerPoint2D& pos );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapPalette > getPalette();

        ::com::sun::star::rendering::IntegerBitmapLayout getMemoryLayout();

        /// @internal
        BitmapEx getBitmap() const;

    private:

        void setBitmap( const BitmapEx& rBitmap );

        BitmapBackBufferSharedPtr   mpBackBuffer;
    };
}

#endif /* _VCLCANVAS_CANVASBITMAPHELPER_HXX_ */
