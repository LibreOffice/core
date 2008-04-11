/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_vcltools.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _DXCANVAS_VCLTOOLS_HXX
#define _DXCANVAS_VCLTOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/TriState.hpp>

#include <boost/shared_ptr.hpp>


namespace com { namespace sun { namespace star { namespace lang
{
    class XUnoTunnel;
} } } }


namespace dxcanvas
{
    class SurfaceGraphics;

    namespace tools
    {
        /** Raw RGBA bitmap data,
            contiguous in memory
         */
        struct RawRGBABitmap
        {
            sal_Int32                           mnWidth;
            sal_Int32                           mnHeight;
            ::boost::shared_ptr< sal_uInt8 >    mpBitmapData;
        };

        bool drawVCLBitmapFromUnoTunnel( const ::boost::shared_ptr< SurfaceGraphics >&  rGraphics,
                                         const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::lang::XUnoTunnel >&      xTunnel );

        ::com::sun::star::util::TriState isAlphaVCLBitmapFromUnoTunnel( const ::com::sun::star::uno::Reference<
                                                                            ::com::sun::star::lang::XUnoTunnel >& xTunnel );
    }
}

#endif /* _DXCANVAS_VCLTOOLS_HXX */
