/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvasbitmap.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:17:31 $
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

#ifndef _VCLCANVAS_CANVASBITMAP_HXX
#define _VCLCANVAS_CANVASBITMAP_HXX

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XBITMAPCANVAS_HPP_
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XINTEGERBITMAP_HPP_
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

#include <canvas/vclwrapper.hxx>

#include <canvas/bitmapcanvasbase.hxx>
#include <canvasbitmaphelper.hxx>

#include "impltools.hxx"
#include "repainttarget.hxx"


#define CANVASBITMAP_IMPLEMENTATION_NAME "VCLCanvas::CanvasBitmap"

/* Definition of CanvasBitmap class */

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper3< ::com::sun::star::rendering::XBitmapCanvas,
                                               ::com::sun::star::rendering::XIntegerBitmap,
                                                ::com::sun::star::lang::XServiceInfo >                                  CanvasBitmapBase_Base;
    typedef ::canvas::internal::BitmapCanvasBase< CanvasBitmapBase_Base, CanvasBitmapHelper, tools::LocalGuard >    CanvasBitmap_Base;

    class CanvasBitmap : public CanvasBitmap_Base,
                         public RepaintTarget
    {
    public:
        /** Must be called with locked Solar mutex

            @param rSize
            Size in pixel of the bitmap to generate

            @param bAlphaBitmap
            When true, bitmap will have an alpha channel

            @param rDevice
            Reference device, with which bitmap should be compatible
         */
        CanvasBitmap( const ::Size&                         rSize,
                      bool                                  bAlphaBitmap,
                      const WindowGraphicDevice::ImplRef&   rDevice );

        /// Must be called with locked Solar mutex
        CanvasBitmap( const BitmapEx&                       rBitmap,
                      const WindowGraphicDevice::ImplRef&   rDevice );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // RepaintTarget interface
        virtual bool repaint( const GraphicObjectSharedPtr& rGrf,
                              const ::Point&                rPt,
                              const ::Size&                 rSz,
                              const GraphicAttr&            rAttr ) const;

        BitmapEx getBitmap() const;

    protected:
        ~CanvasBitmap(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        // default: disabled copy/assignment
        CanvasBitmap(const CanvasBitmap&);
        CanvasBitmap& operator=( const CanvasBitmap& );
    };
}

#endif /* _VCLCANVAS_CANVASBITMAP_HXX */
