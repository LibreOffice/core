/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/tools/canvastools.hxx>

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

        // setup back buffer
        OutputDevice& rOutDev( pOutDev->getOutDev() );
        mpBackBuffer.reset( new BackBuffer( rOutDev ));
        mpBackBuffer->setSize( rOutDev.GetOutputSizePixel() );

        // #i95645#
#if defined( QUARTZ )
        // use AA on VCLCanvas for Mac
        mpBackBuffer->getOutDev().SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW | mpBackBuffer->getOutDev().GetAntialiasing() );
#else
        // switch off AA for WIN32 and UNIX, the VCLCanvas does not look good with it and
        // is not required to do AA. It would need to be adapted to use it correctly
        // (especially gradient painting). This will need extra work.
        mpBackBuffer->getOutDev().SetAntialiasing(mpBackBuffer->getOutDev().GetAntialiasing() & ~ANTIALIASING_ENABLE_B2DDRAW);
#endif
    }

    ::sal_Int32 SpriteDeviceHelper::createBuffers( ::sal_Int32 nBuffers )
    {
        (void)nBuffers;

        // TODO(F3): implement XBufferStrategy interface. For now, we
        // _always_ will have exactly one backbuffer
        return 1;
    }

    void SpriteDeviceHelper::destroyBuffers()
    {
        // TODO(F3): implement XBufferStrategy interface. For now, we
        // _always_ will have exactly one backbuffer
    }

    ::sal_Bool SpriteDeviceHelper::showBuffer( bool, ::sal_Bool )
    {
        OSL_ENSURE(false,"Not supposed to be called, handled by SpriteCanvas");
        return sal_False;
    }

    ::sal_Bool SpriteDeviceHelper::switchBuffer( bool, ::sal_Bool )
    {
        OSL_ENSURE(false,"Not supposed to be called, handled by SpriteCanvas");
        return sal_False;
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

        static sal_uInt32 nFilePostfixCount(0);

        if( mpBackBuffer )
        {
            String aFilename( String::CreateFromAscii("dbg_backbuffer") );
            aFilename += String::CreateFromInt32(nFilePostfixCount);
            aFilename += String::CreateFromAscii(".bmp");

            SvFileStream aStream( aFilename, STREAM_STD_READWRITE );

            const ::Point aEmptyPoint;
            mpBackBuffer->getOutDev().EnableMapMode( sal_False );
            aStream << mpBackBuffer->getOutDev().GetBitmap(aEmptyPoint,
                                                            mpBackBuffer->getOutDev().GetOutputSizePixel());
        }

        ++nFilePostfixCount;
    }

}
