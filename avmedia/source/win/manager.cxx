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

#include "manager.hxx"
#include "player.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;

namespace avmedia::win {

Manager::Manager()
{
}


Manager::~Manager()
{
}


uno::Reference< media::XPlayer > SAL_CALL Manager::createPlayer( const OUString& rURL )
{
    rtl::Reference<Player>              pPlayer( new Player() );
    const INetURLObject                 aURL( rURL );

    if( !pPlayer->create( aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ) )  )
        pPlayer.clear();

    return pPlayer;
}


OUString SAL_CALL Manager::getImplementationName(  )
{
    return "com.sun.star.comp.avmedia.Manager_DirectX";
}


sal_Bool SAL_CALL Manager::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


uno::Sequence< OUString > SAL_CALL Manager::getSupportedServiceNames(  )
{
    return { "com.sun.star.media.Manager" };
}

} // namespace avmedia::win

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
avmedia_Manager_DirectX_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new avmedia::win::Manager());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
