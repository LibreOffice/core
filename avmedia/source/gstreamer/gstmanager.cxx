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

#include <cppuhelper/supportsservice.hxx>

#include "gstmanager.hxx"
#include "gstplayer.hxx"

#include <tools/urlobj.hxx>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;

namespace avmedia::gstreamer {

Manager::Manager()
{
}

Manager::~Manager()
{
}

uno::Reference< media::XPlayer > SAL_CALL Manager::createPlayer( const OUString& rURL )
{
    rtl::Reference<Player> pPlayer( new Player );
    const INetURLObject                 aURL( rURL );

    if( !pPlayer->create( aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ) )  )
        pPlayer.clear();

    return pPlayer;
}

OUString SAL_CALL Manager::getImplementationName(  )
{
    return "com.sun.star.comp.avmedia.Manager_GStreamer";
}

sal_Bool SAL_CALL Manager::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL Manager::getSupportedServiceNames(  )
{
    return { "com.sun.star.media.Manager" };
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_media_Manager_GStreamer_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new avmedia::gstreamer::Manager());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
