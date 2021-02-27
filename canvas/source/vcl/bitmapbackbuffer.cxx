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

#include <osl/diagnose.h>
#include <vcl/bitmapex.hxx>
#include <vcl/svapp.hxx>

#include "bitmapbackbuffer.hxx"
#include "impltools.hxx"

namespace vclcanvas
{
    BitmapBackBuffer::BitmapBackBuffer( const BitmapEx&         rBitmap,
                                        const OutputDevice&     rRefDevice ) :
        maBitmap( rBitmap ),
        mpVDev( nullptr ),
        mrRefDevice( rRefDevice ),
        mbBitmapContentIsCurrent( false ),
        mbVDevContentIsCurrent( false )
    {
    }

    BitmapBackBuffer::~BitmapBackBuffer()
    {
        // make sure solar mutex is held on deletion (other methods
        // are supposed to be called with already locked solar mutex)
        SolarMutexGuard aGuard;

        mpVDev.disposeAndClear();
    }

    OutputDevice& BitmapBackBuffer::getOutDev()
    {
        createVDev();
        updateVDev();
        return *mpVDev;
    }

    const OutputDevice& BitmapBackBuffer::getOutDev() const
    {
        createVDev();
        updateVDev();
        return *mpVDev;
    }

    void BitmapBackBuffer::clear()
    {
        // force current content to bitmap, make all transparent white
        getBitmapReference().Erase(COL_TRANSPARENT);
    }

    BitmapEx& BitmapBackBuffer::getBitmapReference()
    {
        OSL_ENSURE( !mbBitmapContentIsCurrent || !mbVDevContentIsCurrent,
                    "BitmapBackBuffer::getBitmapReference(): Both bitmap and VDev are valid?!" );

        if( mbVDevContentIsCurrent && mpVDev )
        {
            // VDev content is more current than bitmap - copy contents before!
            mpVDev->EnableMapMode( false );
            mpVDev->SetAntialiasing( AntialiasingFlags::Enable );
            const Point aEmptyPoint;
            *maBitmap = mpVDev->GetBitmapEx( aEmptyPoint,
                                             mpVDev->GetOutputSizePixel() );
        }

        // client queries bitmap, and will possibly alter content -
        // next time, VDev needs to be updated
        mbBitmapContentIsCurrent = true;
        mbVDevContentIsCurrent   = false;

        return *maBitmap;
    }

    Size BitmapBackBuffer::getBitmapSizePixel() const
    {
        Size aSize = maBitmap->GetSizePixel();

        if( mbVDevContentIsCurrent && mpVDev )
        {
            mpVDev->EnableMapMode( false );
            mpVDev->SetAntialiasing( AntialiasingFlags::Enable );
            aSize = mpVDev->GetOutputSizePixel();
        }

        return aSize;
    }

    void BitmapBackBuffer::createVDev() const
    {
        if( mpVDev )
            return;

        // VDev not yet created, do it now. Create an alpha-VDev,
        // if bitmap has transparency.
        mpVDev = maBitmap->IsAlpha() ?
            VclPtr<VirtualDevice>::Create( mrRefDevice, DeviceFormat::DEFAULT, DeviceFormat::DEFAULT ) :
            VclPtr<VirtualDevice>::Create( mrRefDevice );

        OSL_ENSURE( mpVDev,
                    "BitmapBackBuffer::createVDev(): Unable to create VirtualDevice" );

        mpVDev->SetOutputSizePixel( maBitmap->GetSizePixel() );

        tools::SetDefaultDeviceAntiAliasing( mpVDev );
    }

    void BitmapBackBuffer::updateVDev() const
    {
        OSL_ENSURE( !mbBitmapContentIsCurrent || !mbVDevContentIsCurrent,
                    "BitmapBackBuffer::updateVDev(): Both bitmap and VDev are valid?!" );

        if( mpVDev && mbBitmapContentIsCurrent )
        {
            // fill with bitmap content
            mpVDev->EnableMapMode( false );
            mpVDev->SetAntialiasing( AntialiasingFlags::Enable );
            const Point aEmptyPoint;
            mpVDev->DrawBitmapEx( aEmptyPoint, *maBitmap );
        }

        // canvas queried the VDev, and will possibly paint into
        // it. Next time, bitmap must be updated
        mbBitmapContentIsCurrent = false;
        mbVDevContentIsCurrent   = true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
