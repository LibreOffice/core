/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Novell, Inc.
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

#include "mediamisc.hxx"
#include "gstmanager.hxx"

using namespace ::com::sun::star;

// -------------------
// - factory methods -
// -------------------

#ifdef AVMEDIA_GST_0_10
#  define IMPL_NAME    "com.sun.star.comp.media.Manager_GStreamer_0_10"
#  define SERVICE_NAME AVMEDIA_MANAGER_SERVICE_NAME_OLD // "com.sun.star.comp.avmedia.Manager_GStreamer_0_10"
#else
#  define IMPL_NAME    "com.sun.star.comp.media.Manager_GStreamer"
#  define SERVICE_NAME AVMEDIA_MANAGER_SERVICE_NAME // "com.sun.star.comp.avmedia.Manager_GStreamer"
#endif

static uno::Reference< uno::XInterface > SAL_CALL create_MediaPlayer( const uno::Reference< lang::XMultiServiceFactory >& rxFact )
{
    return uno::Reference< uno::XInterface >( *new ::avmedia::gstreamer::Manager( rxFact ) );
}

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
{
    uno::Reference< lang::XSingleServiceFactory > xFactory;
    void*                                   pRet = 0;

    if( rtl_str_compare( pImplName, IMPL_NAME ) == 0 )
    {
        const OUString aServiceName( SERVICE_NAME );
        xFactory = uno::Reference< lang::XSingleServiceFactory >( ::cppu::createSingleFactory(
                        reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
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
