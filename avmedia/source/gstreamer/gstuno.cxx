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

#include "mediamisc.hxx"
#include "gstmanager.hxx"

using namespace ::com::sun::star;


// - factory methods -


#ifdef AVMEDIA_GST_0_10
#  define IMPL_NAME    "com.sun.star.comp.media.Manager_GStreamer_0_10"
#  define SERVICE_NAME AVMEDIA_MANAGER_SERVICE_NAME_OLD // "com.sun.star.comp.avmedia.Manager_GStreamer_0_10"
#else
#  define IMPL_NAME    "com.sun.star.comp.media.Manager_GStreamer"
#  define SERVICE_NAME AVMEDIA_MANAGER_SERVICE_NAME // "com.sun.star.comp.avmedia.Manager_GStreamer"
#endif

static uno::Reference< uno::XInterface > SAL_CALL create_MediaPlayer( const uno::Reference< lang::XMultiServiceFactory >&  )
{
    return uno::Reference< uno::XInterface >( *new ::avmedia::gstreamer::Manager );
}

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL avmediagst_component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
{
    uno::Reference< lang::XSingleServiceFactory > xFactory;
    void*                                   pRet = nullptr;

    if( rtl_str_compare( pImplName, IMPL_NAME ) == 0 )
    {
        xFactory.set( ::cppu::createSingleFactory(
                        static_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                        IMPL_NAME, create_MediaPlayer, uno::Sequence< OUString > { SERVICE_NAME } ) );
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
