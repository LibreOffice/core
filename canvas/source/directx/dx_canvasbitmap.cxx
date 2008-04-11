/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_canvasbitmap.cxx,v $
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

#include <ctype.h> // don't ask. msdev breaks otherwise...
#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>

#include "dx_canvasbitmap.hxx"
#include "dx_impltools.hxx"


using namespace ::com::sun::star;

namespace dxcanvas
{
    CanvasBitmap::CanvasBitmap( const DXBitmapSharedPtr& rSurface,
                                const DeviceRef&         rDevice ) :
        mpDevice( rDevice ),
        mpSurface( rSurface )
    {
        ENSURE_AND_THROW( mpDevice.is() && rSurface,
                          "CanvasBitmap::CanvasBitmap(): Invalid surface or device" );

        maCanvasHelper.setDevice( *mpDevice.get() );
        maCanvasHelper.setTarget( rSurface,
                                   ::basegfx::B2ISize() );
    }

    void SAL_CALL CanvasBitmap::disposing()
    {
        mpSurface.reset();
        mpDevice.clear();

        // forward to parent
        CanvasBitmap_Base::disposing();
    }

#define IMPLEMENTATION_NAME "DXCanvas.CanvasBitmap"
#define SERVICE_NAME "com.sun.star.rendering.CanvasBitmap"

    ::rtl::OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

}
