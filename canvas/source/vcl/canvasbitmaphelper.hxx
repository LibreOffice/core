/*************************************************************************
 *
 *  $RCSfile: canvasbitmaphelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 17:11:01 $
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

        ::drafts::com::sun::star::geometry::IntegerSize2D SAL_CALL getSize();

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmapCanvas > SAL_CALL queryBitmapCanvas();

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmap > SAL_CALL
            getScaledBitmap( const ::drafts::com::sun::star::geometry::RealSize2D&  newSize,
                             sal_Bool                                               beFast );

        ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
            getData( const ::drafts::com::sun::star::geometry::IntegerRectangle2D& rect );

        void SAL_CALL setData( const ::com::sun::star::uno::Sequence< sal_Int8 >&               data,
                               const ::drafts::com::sun::star::geometry::IntegerRectangle2D&    rect );

        void SAL_CALL setPixel( const ::com::sun::star::uno::Sequence< sal_Int8 >&          color,
                                const ::drafts::com::sun::star::geometry::IntegerPoint2D&   pos );

        ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
            getPixel( const ::drafts::com::sun::star::geometry::IntegerPoint2D& pos );

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmapPalette > SAL_CALL getPalette();

        ::drafts::com::sun::star::rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout();

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
