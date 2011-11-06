/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include "canvasbitmap.hxx"

#include <vcl/bmpacc.hxx>

using namespace ::com::sun::star;


namespace vclcanvas
{
    // Currently, the only way to generate an XBitmap is from
    // XGraphicDevice.getCompatibleBitmap(). Therefore, we don't even
    // take a bitmap here, but a VDev directly.
    CanvasBitmap::CanvasBitmap( const ::Size&                  rSize,
                                bool                           bAlphaBitmap,
                                rendering::XGraphicDevice&     rDevice,
                                const OutDevProviderSharedPtr& rOutDevProvider )
    {
        // create bitmap for given reference device
        // ========================================
        const sal_uInt16 nBitCount( (sal_uInt16)24U );
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
                                 rDevice,
                                 rOutDevProvider );
        }
        else
        {
            maCanvasHelper.init( BitmapEx( aBitmap ),
                                 rDevice,
                                 rOutDevProvider );
        }
    }

    CanvasBitmap::CanvasBitmap( const BitmapEx&                rBitmap,
                                rendering::XGraphicDevice&     rDevice,
                                const OutDevProviderSharedPtr& rOutDevProvider )
    {
        maCanvasHelper.init( rBitmap, rDevice, rOutDevProvider );
    }

    void SAL_CALL CanvasBitmap::disposing()
    {
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

    bool CanvasBitmap::repaint( const GraphicObjectSharedPtr& rGrf,
                                const rendering::ViewState&   viewState,
                                const rendering::RenderState& renderState,
                                const ::Point&                rPt,
                                const ::Size&                 rSz,
                                const GraphicAttr&            rAttr ) const
    {
        tools::LocalGuard aGuard;

        mbSurfaceDirty = true;

        return maCanvasHelper.repaint( rGrf, viewState, renderState, rPt, rSz, rAttr );
    }

    uno::Any SAL_CALL CanvasBitmap::getFastPropertyValue( sal_Int32 nHandle ) throw (uno::RuntimeException)
    {
        if( nHandle == 0 ) {
            BitmapEx* pBitmapEx = new BitmapEx( getBitmap() );

            return uno::Any( reinterpret_cast<sal_Int64>( pBitmapEx ) );
        }

        return uno::Any( sal_Int64(0) );
    }
}
