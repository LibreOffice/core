/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <tools/diagnose_ex.h>

#include <canvas/canvastools.hxx>

#include "ogl_bitmapcanvashelper.hxx"


using namespace ::com::sun::star;

namespace oglcanvas
{
    BitmapCanvasHelper::BitmapCanvasHelper()
    {}

    void BitmapCanvasHelper::disposing()
    {
        CanvasHelper::disposing();
    }

    void BitmapCanvasHelper::init( rendering::XGraphicDevice&     rDevice,
                                   SpriteDeviceHelper&            rDeviceHelper,
                                   const geometry::IntegerSize2D& rSize )
    {
        maSize = rSize;
        CanvasHelper::init(rDevice,rDeviceHelper);
    }

    uno::Reference< rendering::XBitmap > BitmapCanvasHelper::getScaledBitmap( const geometry::RealSize2D& /*newSize*/,
                                                                              bool                    /*beFast*/ )
    {
        // TODO(F1):
        return uno::Reference< rendering::XBitmap >();
    }

    uno::Sequence< sal_Int8 > BitmapCanvasHelper::getData( rendering::IntegerBitmapLayout&     /*bitmapLayout*/,
                                                           const geometry::IntegerRectangle2D& /*rect*/ )
    {
        // TODO(F2): NYI - and improbable to ever be
        return uno::Sequence< sal_Int8 >();
    }

    uno::Sequence< sal_Int8 > BitmapCanvasHelper::getPixel( rendering::IntegerBitmapLayout&   /*bitmapLayout*/,
                                                            const geometry::IntegerPoint2D&   /*pos*/ )
    {
        // TODO(F2): NYI - and improbable to ever be
        return uno::Sequence< sal_Int8 >();
    }

    rendering::IntegerBitmapLayout BitmapCanvasHelper::getMemoryLayout() const
    {
        return ::canvas::tools::getStdMemoryLayout(getSize());
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
