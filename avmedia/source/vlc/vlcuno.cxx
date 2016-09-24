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

#include <comphelper/processfactory.hxx>
#include <comphelper/configuration.hxx>
#include <officecfg/Office/Common.hxx>

#include "vlccommon.hxx"
#include "vlcmanager.hxx"

using namespace ::com::sun::star;

#define IMPL_NAME    "com.sun.star.comp.media.Manager_VLC"
#define SERVICE_NAME "com.sun.star.comp.avmedia.Manager_VLC"

static uno::Reference< uno::XInterface > SAL_CALL create_MediaPlayer( const uno::Reference< lang::XMultiServiceFactory >& rxFact )
{
    SAL_INFO("avmedia", "create VLC Media player !\n");

    // Experimental for now - code is neither elegant nor well tested.
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    if (!xContext.is() || !officecfg::Office::Common::Misc::ExperimentalMode::get(xContext))
        return nullptr;

    static uno::Reference< uno::XInterface > manager( *new ::avmedia::vlc::Manager( rxFact ) );
    return manager;
}

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL avmediavlc_component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
{
    uno::Reference< lang::XSingleServiceFactory > xFactory;
    void*                                   pRet = nullptr;

    // Experimental for now - code is neither elegant nor well tested.
    uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    if (!xContext.is() || !officecfg::Office::Common::Misc::ExperimentalMode::get(xContext))
        return nullptr;

    SAL_INFO("avmedia", "Create VLC Media component: '" << pImplName << "'\n");
    if( rtl_str_compare( pImplName, IMPL_NAME ) == 0 )
    {
        const OUString aServiceName( SERVICE_NAME );
        xFactory.set( ::cppu::createSingleFactory(
                        static_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                        IMPL_NAME, create_MediaPlayer, uno::Sequence< OUString >( &aServiceName, 1 ) ) );
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
