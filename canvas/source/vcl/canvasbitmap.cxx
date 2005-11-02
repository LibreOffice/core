/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvasbitmap.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:58:16 $
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

#include <canvas/debug.hxx>
#include "canvasbitmap.hxx"

#include <vcl/bmpacc.hxx>

using namespace ::com::sun::star;


namespace vclcanvas
{
    // Currently, the only way to generate an XBitmap is from
    // XGraphicDevice.getCompatibleBitmap(). Therefore, we don't even
    // take a bitmap here, but a VDev directly.
    CanvasBitmap::CanvasBitmap( const ::Size&       rSize,
                                bool                bAlphaBitmap,
                                const DeviceRef&    rDevice ) :
        mpDevice( rDevice )
    {
        ENSURE_AND_THROW( rDevice->getOutDev(),
                          "CanvasBitmap::CanvasBitmap(): Invalid reference outdev" );

        OutputDevice&            rOutDev( *rDevice->getOutDev() );
        tools::OutDevStateKeeper aStateKeeper( rOutDev );

        // create bitmap for given reference device
        // ========================================
        const USHORT nBitCount( (USHORT)24U );
        const BitmapPalette*    pPalette = NULL;

        Bitmap aBitmap( rSize, nBitCount, pPalette );

        // only create alpha channel bitmap, if factory requested
        // that. Providing alpha-channeled bitmaps by default has,
        // especially under VCL, a huge performance penalty (have to
        // use alpha VDev, then).
        if( bAlphaBitmap )
        {
            AlphaMask   aAlpha ( rSize );

            maCanvasHelper.init( BitmapEx( aBitmap, aAlpha ),
                                 *rDevice.get() );
        }
        else
        {
            maCanvasHelper.init( BitmapEx( aBitmap ),
                                 *rDevice.get() );
        }
    }

    CanvasBitmap::CanvasBitmap( const BitmapEx&     rBitmap,
                                const DeviceRef&    rDevice ) :
        mpDevice( rDevice )
    {
        ENSURE_AND_THROW( rDevice->getOutDev(),
                          "CanvasBitmap::CanvasBitmap(): Invalid reference outdev" );

        maCanvasHelper.init( rBitmap,
                             *rDevice.get() );
    }

    void SAL_CALL CanvasBitmap::disposing()
    {
        mpDevice.clear();

        // forward to base
        CanvasBitmap_Base::disposing();
    }

#define IMPLEMENTATION_NAME "VCLCanvas.CanvasBitmap"
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

    BitmapEx CanvasBitmap::getBitmap() const
    {
        tools::LocalGuard aGuard;

        // TODO(T3): Rework to use shared_ptr all over the place for
        // BmpEx. This is highly un-threadsafe
        return maCanvasHelper.getBitmap();
    }

    bool CanvasBitmap::repaint( const GraphicObjectSharedPtr&   rGrf,
                                const ::Point&                  rPt,
                                const ::Size&                   rSz,
                                const GraphicAttr&              rAttr ) const
    {
        tools::LocalGuard aGuard;

        mbSurfaceDirty = true;

        return maCanvasHelper.repaint( rGrf, rPt, rSz, rAttr );
    }
}
