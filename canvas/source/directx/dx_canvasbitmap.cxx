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

#include <memory>

#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/bitmap.hxx>

#include <canvas/canvastools.hxx>

#include "dx_canvasbitmap.hxx"
#include "dx_impltools.hxx"


using namespace ::com::sun::star;

namespace dxcanvas
{
    CanvasBitmap::CanvasBitmap( const IBitmapSharedPtr& rBitmap,
                                const DeviceRef&        rDevice ) :
        mpDevice( rDevice ),
        mpBitmap( rBitmap )
    {
        ENSURE_OR_THROW( mpDevice.is() && mpBitmap,
                         "CanvasBitmap::CanvasBitmap(): Invalid surface or device" );

        maCanvasHelper.setDevice( *mpDevice );
        maCanvasHelper.setTarget( mpBitmap );
    }

    void CanvasBitmap::disposeThis()
    {
        mpBitmap.reset();
        mpDevice.clear();

        // forward to parent
        CanvasBitmap_Base::disposeThis();
    }

    namespace {

    struct AlphaDIB
    {
        BITMAPINFOHEADER bmiHeader;
        RGBQUAD          bmiColors[256];
        AlphaDIB()
            : bmiHeader({0,0,0,1,8,BI_RGB,0,0,0,0,0})
        {
            for (size_t i = 0; i < 256; ++i)
            {
                // this here fills palette with grey level colors, starting
                // from 0,0,0 up to 255,255,255
                BYTE const b(i);
                bmiColors[i] = { b,b,b,b };
            }
        }
    };

    }

    uno::Any SAL_CALL CanvasBitmap::getFastPropertyValue( sal_Int32 nHandle )
    {
        uno::Any aRes;
        // 0 ... get Bitmap
        // 1 ... get Pixbuf with bitmap RGB content
        // 2 ... get Pixbuf with bitmap alpha mask
        switch( nHandle )
        {
            // sorry, no Bitmap here...
            case 0:
                aRes <<= reinterpret_cast<sal_Int64>( nullptr );
                break;

            case 1:
            {
                HBITMAP aHBmp;
                mpBitmap->getBitmap()->GetHBITMAP(Gdiplus::Color(), &aHBmp );

                uno::Sequence< uno::Any > args{ uno::Any(reinterpret_cast<sal_Int64>(aHBmp)) };
                aRes <<= args;
            }
            break;

            case 2:
            {
                assert(false && "should not be calling this anymore since the relevant code in vcl was removed");
            }
            break;
        }

        return aRes;
    }

    OUString SAL_CALL CanvasBitmap::getImplementationName(  )
    {
        return "DXCanvas.CanvasBitmap";
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  )
    {
        return { "com.sun.star.rendering.CanvasBitmap" };
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
