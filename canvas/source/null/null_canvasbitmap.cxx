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


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/canvastools.hxx>

#include "null_canvasbitmap.hxx"


using namespace ::com::sun::star;

namespace nullcanvas
{
    CanvasBitmap::CanvasBitmap( const ::basegfx::B2ISize& rSize,
                                const DeviceRef&          rDevice,
                                bool                      bHasAlpha ) :
        mpDevice( rDevice )
    {
        ENSURE_OR_THROW( mpDevice.is(),
                          "CanvasBitmap::CanvasBitmap(): Invalid surface or device" );

        maCanvasHelper.init( rSize,
                             *mpDevice.get(),
                             bHasAlpha );
    }

    void CanvasBitmap::disposeThis()
    {
        mpDevice.clear();

        // forward to parent
        CanvasBitmap_Base::disposeThis();
    }

#define IMPLEMENTATION_NAME "NullCanvas.CanvasBitmap"
#define SERVICE_NAME "com.sun.star.rendering.CanvasBitmap"

    ::rtl::OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return ServiceName == SERVICE_NAME;
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
