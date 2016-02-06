/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/bitmapaccess.hxx>

#include "canvasbitmap.hxx"


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
        const BitmapPalette*    pPalette = nullptr;

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


    OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException, std::exception)
    {
        return OUString( "VCLCanvas.CanvasBitmap" );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const OUString& ServiceName ) throw (uno::RuntimeException, std::exception)
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException, std::exception)
    {
        uno::Sequence< OUString > aRet { "com.sun.star.rendering.CanvasBitmap" };

        return aRet;
    }

    BitmapEx CanvasBitmap::getBitmap() const
    {
        SolarMutexGuard aGuard;

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
        SolarMutexGuard aGuard;

        mbSurfaceDirty = true;

        return maCanvasHelper.repaint( rGrf, viewState, renderState, rPt, rSz, rAttr );
    }

    uno::Any SAL_CALL CanvasBitmap::getFastPropertyValue( sal_Int32 nHandle ) throw (uno::RuntimeException, std::exception)
    {
        if( nHandle == 0 ) {
            BitmapEx* pBitmapEx = new BitmapEx( getBitmap() );

            return uno::Any( reinterpret_cast<sal_Int64>( pBitmapEx ) );
        }

        return uno::Any( sal_Int64(0) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
