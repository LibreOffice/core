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
#include <vcl/dibtools.hxx>
#include <tools/stream.hxx>

#include "spritedevicehelper.hxx"
#include "impltools.hxx"

using namespace ::com::sun::star;

namespace vclcanvas
{
    SpriteDeviceHelper::SpriteDeviceHelper()
    {
    }

    void SpriteDeviceHelper::init( const OutDevProviderSharedPtr& pOutDev )
    {
        DeviceHelper::init(pOutDev);

        // setup back buffer
        OutputDevice& rOutDev( pOutDev->getOutDev() );
        mpBackBuffer = std::make_shared<BackBuffer>( rOutDev );
        mpBackBuffer->setSize( rOutDev.GetOutputSizePixel() );

        tools::SetDefaultDeviceAntiAliasing( &mpBackBuffer->getOutDev());
    }

    bool SpriteDeviceHelper::showBuffer( bool, bool )
    {
        OSL_FAIL("Not supposed to be called, handled by SpriteCanvas");
        return false;
    }

    bool SpriteDeviceHelper::switchBuffer( bool, bool )
    {
        OSL_FAIL("Not supposed to be called, handled by SpriteCanvas");
        return false;
    }

    void SpriteDeviceHelper::disposing()
    {
        // release all references
        mpBackBuffer.reset();

        DeviceHelper::disposing();
    }

    uno::Any SpriteDeviceHelper::isAccelerated() const
    {
        return DeviceHelper::isAccelerated();
    }

    uno::Any SpriteDeviceHelper::getDeviceHandle() const
    {
        return DeviceHelper::getDeviceHandle();
    }

    uno::Any SpriteDeviceHelper::getSurfaceHandle() const
    {
        if( !mpBackBuffer )
            return uno::Any();

        return uno::Any(
            reinterpret_cast< sal_Int64 >(&mpBackBuffer->getOutDev()) );
    }

    void SpriteDeviceHelper::notifySizeUpdate( const awt::Rectangle& rBounds )
    {
        if( mpBackBuffer )
            mpBackBuffer->setSize( ::Size(rBounds.Width,
                                          rBounds.Height) );
    }

    void SpriteDeviceHelper::dumpScreenContent() const
    {
        DeviceHelper::dumpScreenContent();

        static sal_Int32 nFilePostfixCount(0);

        if( mpBackBuffer )
        {
            OUString aFilename = "dbg_backbuffer" + OUString::number(nFilePostfixCount) + ".bmp";

            SvFileStream aStream( aFilename, StreamMode::STD_READWRITE );

            const ::Point aEmptyPoint;
            mpBackBuffer->getOutDev().EnableMapMode( false );
            mpBackBuffer->getOutDev().SetAntialiasing( AntialiasingFlags::Enable );
            WriteDIB(BitmapEx(mpBackBuffer->getOutDev().GetBitmap(aEmptyPoint, mpBackBuffer->getOutDev().GetOutputSizePixel())), aStream, false);
        }

        ++nFilePostfixCount;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
