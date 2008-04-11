/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvasfont.hxx,v $
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

#ifndef _VCLCANVAS_CANVASFONT_HXX
#define _VCLCANVAS_CANVASFONT_HXX

#include <comphelper/implementationreference.hxx>

#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/rendering/FontRequest.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>

#include <vcl/font.hxx>

#include <canvas/vclwrapper.hxx>

#include "spritecanvas.hxx"
#include "impltools.hxx"

#include <boost/utility.hpp>


/* Definition of CanvasFont class */

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::rendering::XCanvasFont,
                                               ::com::sun::star::lang::XServiceInfo > CanvasFont_Base;

    class CanvasFont : public ::comphelper::OBaseMutex,
                       public CanvasFont_Base,
                       private ::boost::noncopyable
    {
    public:
        typedef ::comphelper::ImplementationReference<
            CanvasFont,
            ::com::sun::star::rendering::XCanvasFont > Reference;

        CanvasFont( const ::com::sun::star::rendering::FontRequest&                                     fontRequest,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&    extraFontProperties,
                    const ::com::sun::star::geometry::Matrix2D&                                         rFontMatrix,
                    const DeviceRef&                                                                    rDevice );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XCanvasFont
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XTextLayout > SAL_CALL createTextLayout( const ::com::sun::star::rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::FontRequest SAL_CALL getFontRequest(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::FontMetrics SAL_CALL getFontMetrics(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getAvailableSizes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getExtraFontProperties(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException );

        ::Font getVCLFont() const;

    private:
        ::canvas::vcltools::VCLObject<Font>         maFont;
        ::com::sun::star::rendering::FontRequest    maFontRequest;
        DeviceRef                                   mpRefDevice;
    };

}

#endif /* _VCLCANVAS_CANVASFONT_HXX */
