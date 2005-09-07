/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvasbitmaphelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:18:10 $
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

#ifndef _VCLCANVAS_CANVASBITMAPHELPER_HXX_
#define _VCLCANVAS_CANVASBITMAPHELPER_HXX_

#include <canvashelper.hxx>

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#include <canvas/vclwrapper.hxx>

#include "bitmapbackbuffer.hxx"


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

            This method force-sets a new bitmap. The internally stored
            bitmap representation is updated from the given bitmap,
            including any size changes.
         */
        void setBitmap( const BitmapEx&                     rBitmap,
                        const WindowGraphicDevice::ImplRef& rDevice );


        // Overridden CanvasHelper functionality
        // =====================================

        void disposing();

        ::com::sun::star::geometry::IntegerSize2D SAL_CALL getSize();

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > SAL_CALL queryBitmapCanvas();

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > SAL_CALL
            getScaledBitmap( const ::com::sun::star::geometry::RealSize2D&  newSize,
                             sal_Bool                                               beFast );

        ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
            getData( const ::com::sun::star::geometry::IntegerRectangle2D& rect );

        void SAL_CALL setData( const ::com::sun::star::uno::Sequence< sal_Int8 >&               data,
                               const ::com::sun::star::geometry::IntegerRectangle2D&    rect );

        void SAL_CALL setPixel( const ::com::sun::star::uno::Sequence< sal_Int8 >&          color,
                                const ::com::sun::star::geometry::IntegerPoint2D&   pos );

        ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
            getPixel( const ::com::sun::star::geometry::IntegerPoint2D& pos );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapPalette > SAL_CALL getPalette();

        ::com::sun::star::rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout();

        /// @internal
        BitmapEx getBitmap() const;

    private:
        // default: disabled copy/assignment
        CanvasBitmapHelper(const CanvasBitmapHelper&);
        CanvasBitmapHelper& operator=( const CanvasBitmapHelper& );

        BitmapBackBufferSharedPtr   mpBackBuffer;
    };
}

#endif /* _VCLCANVAS_CANVASBITMAPHELPER_HXX_ */
