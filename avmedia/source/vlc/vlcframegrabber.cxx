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

#include <chrono>
#include <iostream>
#include <osl/conditn.hxx>
#include <osl/file.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/pngread.hxx>
#include <avmedia/mediawindow.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/stream.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include "vlcframegrabber.hxx"
#include "vlcplayer.hxx"
#include <wrapper/Player.hxx>
#include <wrapper/EventManager.hxx>

using namespace ::com::sun::star;

namespace avmedia::vlc {

namespace
{
    constexpr OUStringLiteral AVMEDIA_VLC_GRABBER_IMPLEMENTATIONNAME = u"com.sun.star.comp.avmedia.VLCFrameGrabber_VLC";
    constexpr OUStringLiteral AVMEDIA_VLC_GRABBER_SERVICENAME = u"com.sun.star.media.VLCFrameGrabber_VLC";
    const int MSEC_IN_SEC = 1000;

    const char * const VLC_ARGS[] = {
        "-Vdummy",
        "--demux",
        "ffmpeg",
        "--snapshot-format=png",
        "--ffmpeg-threads", /* Is deprecated in 2.1.0 */
        "--verbose=-1",
        "--no-audio"
    };
}

VLCFrameGrabber::VLCFrameGrabber( wrapper::EventHandler& eh, const OUString& url )
    : FrameGrabber_BASE()
    , mInstance( SAL_N_ELEMENTS(VLC_ARGS), VLC_ARGS )
    , mMedia( url, mInstance )
    , mPlayer( mMedia )
    , mEventHandler( eh )
{
}

::uno::Reference< css::graphic::XGraphic > SAL_CALL VLCFrameGrabber::grabFrame( double fMediaTime )
{
    osl::Condition condition;

    const OUString& fileName = utl::TempFile::CreateTempName();
    {
        wrapper::EventManager manager( mPlayer, mEventHandler );
        manager.onPaused([&condition](){ condition.set(); });

        if ( !mPlayer.play() )
        {
            SAL_WARN("avmedia", "Couldn't play when trying to grab frame");
            return ::uno::Reference< css::graphic::XGraphic >();
        }

        mPlayer.setTime( std::max(fMediaTime, 0.0) * MSEC_IN_SEC );
        mPlayer.pause();

        condition.wait(std::chrono::seconds(2));

        if ( !mPlayer.hasVout() )
        {
            SAL_WARN("avmedia", "Couldn't grab frame");
            manager.onPaused();
            return ::uno::Reference< css::graphic::XGraphic >();
        }

        mPlayer.takeSnapshot( fileName );
        mPlayer.stop();

        manager.onPaused();
    }

    OUString url;
    osl::FileBase::getFileURLFromSystemPath( fileName, url );
    std::unique_ptr<SvStream> stream( utl::UcbStreamHelper::CreateStream( url,
                                                                            StreamMode::STD_READ ) );

    vcl::PNGReader reader( *stream );

    const BitmapEx& bitmap = reader.Read();

    return Graphic( bitmap ).GetXGraphic();
}

OUString SAL_CALL VLCFrameGrabber::getImplementationName()
{
    return AVMEDIA_VLC_GRABBER_IMPLEMENTATIONNAME;
}

sal_Bool SAL_CALL VLCFrameGrabber::supportsService( const OUString& serviceName )
{
    return cppu::supportsService(this, serviceName);
}

::uno::Sequence< OUString > SAL_CALL VLCFrameGrabber::getSupportedServiceNames()
{
    return { AVMEDIA_VLC_GRABBER_SERVICENAME };
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
