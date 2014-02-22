/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "ogl_canvasbitmap.hxx"

#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>
#include <tools/diagnose_ex.h>


using namespace ::com::sun::star;

namespace oglcanvas
{
    CanvasBitmap::CanvasBitmap( const geometry::IntegerSize2D& rSize,
                                const SpriteCanvasRef&         rDevice,
                                SpriteDeviceHelper&            rDeviceHelper,
                                bool                           bHasAlpha ) :
        mpDevice( rDevice ),
        mbHasAlpha( bHasAlpha )
    {
        ENSURE_OR_THROW( mpDevice.is(),
                         "CanvasBitmap::CanvasBitmap(): Invalid surface or device" );

        maCanvasHelper.init( *mpDevice.get(), rDeviceHelper, rSize );
    }

    CanvasBitmap::CanvasBitmap( const CanvasBitmap& rSrc ) :
        mpDevice( rSrc.mpDevice ),
        mbHasAlpha( rSrc.mbHasAlpha )
    {
        maCanvasHelper = rSrc.maCanvasHelper;
    }

    void SAL_CALL CanvasBitmap::disposeThis()
    {
        mpDevice.clear();

        
        CanvasBitmapBaseT::disposeThis();
    }

    bool CanvasBitmap::renderRecordedActions() const
    {
        return maCanvasHelper.renderRecordedActions();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
