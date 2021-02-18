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
#include <tools/urlobj.hxx>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;

namespace avmedia::macavf {

Manager::Manager()
{
}


Manager::~Manager()
{}


uno::Reference< media::XPlayer > SAL_CALL Manager::createPlayer( const OUString& rURL )
{
    rtl::Reference<Player>              xPlayer( new Player() );
    INetURLObject                       aURL( rURL );

    if( !xPlayer->create( aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ) )  )
        return {};

    return uno::Reference<media::XPlayer>(xPlayer);
}


OUString SAL_CALL Manager::getImplementationName(  )
{
    return "com.sun.star.comp.avmedia.Manager_MacAVF";
}


sal_Bool SAL_CALL Manager::supportsService( const OUString& ServiceName )
{
    return ServiceName == "com.sun.star.media.Manager_MacAVF";
}


uno::Sequence< OUString > SAL_CALL Manager::getSupportedServiceNames(  )
{
    return { "com.sun.star.media.Manager_MacAVF" };
}

} // namespace avmedia::macavf

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_avmedia_Manager_MacAVF_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::avmedia::macavf::Manager());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
