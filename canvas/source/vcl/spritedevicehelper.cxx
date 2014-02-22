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
#include <canvas/canvastools.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <vcl/dibtools.hxx>

#include "spritedevicehelper.hxx"
#include "spritecanvas.hxx"
#include "spritecanvashelper.hxx"
#include "canvasbitmap.hxx"

using namespace ::com::sun::star;

namespace vclcanvas
{
    SpriteDeviceHelper::SpriteDeviceHelper() :
        mpBackBuffer()
    {
    }

    void SpriteDeviceHelper::init( const OutDevProviderSharedPtr& pOutDev )
    {
        DeviceHelper::init(pOutDev);

        
        OutputDevice& rOutDev( pOutDev->getOutDev() );
        mpBackBuffer.reset( new BackBuffer( rOutDev ));
        mpBackBuffer->setSize( rOutDev.GetOutputSizePixel() );

        
#if defined( MACOSX )
        
        mpBackBuffer->getOutDev().SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW | mpBackBuffer->getOutDev().GetAntialiasing() );
#else
        
        
        
        mpBackBuffer->getOutDev().SetAntialiasing(mpBackBuffer->getOutDev().GetAntialiasing() & ~ANTIALIASING_ENABLE_B2DDRAW);
#endif
    }

    ::sal_Int32 SpriteDeviceHelper::createBuffers( ::sal_Int32 nBuffers )
    {
        (void)nBuffers;

        
        
        return 1;
    }

    void SpriteDeviceHelper::destroyBuffers()
    {
        
        
    }

    ::sal_Bool SpriteDeviceHelper::showBuffer( bool, ::sal_Bool )
    {
        OSL_FAIL("Not supposed to be called, handled by SpriteCanvas");
        return sal_False;
    }

    ::sal_Bool SpriteDeviceHelper::switchBuffer( bool, ::sal_Bool )
    {
        OSL_FAIL("Not supposed to be called, handled by SpriteCanvas");
        return sal_False;
    }

    void SpriteDeviceHelper::disposing()
    {
        
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

        return uno::makeAny(
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

            SvFileStream aStream( aFilename, STREAM_STD_READWRITE );

            const ::Point aEmptyPoint;
            mpBackBuffer->getOutDev().EnableMapMode( false );
            mpBackBuffer->getOutDev().SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
            WriteDIB(mpBackBuffer->getOutDev().GetBitmap(aEmptyPoint, mpBackBuffer->getOutDev().GetOutputSizePixel()), aStream, false, true);
        }

        ++nFilePostfixCount;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
