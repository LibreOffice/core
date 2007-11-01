/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_canvasbitmap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:51:48 $
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
