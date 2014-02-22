/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <canvas/debug.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>
#include "canvasbitmap.hxx"

#include <vcl/bmpacc.hxx>

using namespace ::com::sun::star;


namespace vclcanvas
{
    
    
    
    CanvasBitmap::CanvasBitmap( const ::Size&                  rSize,
                                bool                           bAlphaBitmap,
                                rendering::XGraphicDevice&     rDevice,
                                const OutDevProviderSharedPtr& rOutDevProvider )
    {
        
        
        const sal_uInt16 nBitCount( (sal_uInt16)24U );
        const BitmapPalette*    pPalette = NULL;

        Bitmap aBitmap( rSize, nBitCount, pPalette );

        
        
        
        
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


    OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return OUString( "VCLCanvas.CanvasBitmap" );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< OUString > aRet(1);
        aRet[0] = "com.sun.star.rendering.CanvasBitmap";

        return aRet;
    }

    BitmapEx CanvasBitmap::getBitmap() const
    {
        SolarMutexGuard aGuard;

        
        
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

    uno::Any SAL_CALL CanvasBitmap::getFastPropertyValue( sal_Int32 nHandle ) throw (uno::RuntimeException)
    {
        if( nHandle == 0 ) {
            BitmapEx* pBitmapEx = new BitmapEx( getBitmap() );

            return uno::Any( reinterpret_cast<sal_Int64>( pBitmapEx ) );
        }

        return uno::Any( sal_Int64(0) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
