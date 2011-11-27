/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#define Region QtXRegion
#include <QApplication>
#undef Region

#include "KDEData.hxx"
#include "KDESalInstance.hxx"

#include <vclpluginapi.h>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include <rtl/string.hxx>

/// entry point for the KDE4 VCL plugin
extern "C" {
    VCLPLUG_KDE4_PUBLIC SalInstance* create_SalInstance( oslModule )
    {
        /* #i92121# workaround deadlocks in the X11 implementation
        */
        static const char* pNoXInitThreads = getenv( "SAL_NO_XINITTHREADS" );
        /* #i90094#
           from now on we know that an X connection will be
           established, so protect X against itself
        */
        if( ! ( pNoXInitThreads && *pNoXInitThreads ) )
        {
#if QT_VERSION >= 0x040800
            // let Qt call XInitThreads(), so that also Qt knows it's been used
            // (otherwise QPixmap may warn about threads not being initialized)
            QApplication::setAttribute( Qt::AA_X11InitThreads );
#else
            XInitThreads();
            // just in case somebody builds with old version and then upgrades Qt,
            // otherwise this is a no-op
            QApplication::setAttribute( static_cast< Qt::ApplicationAttribute >( 10 ));
#endif
        }

#if QT_VERSION < 0x050000
        // Qt 4.x support needs >= 4.1.0
        rtl::OString aVersion( qVersion() );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "qt version string is \"%s\"\n", aVersion.getStr() );
#endif
        sal_Int32 nIndex = 0, nMajor = 0, nMinor = 0;
        nMajor = aVersion.getToken( 0, '.', nIndex ).toInt32();
        if( nIndex > 0 )
            nMinor = aVersion.getToken( 0, '.', nIndex ).toInt32();
        if( nMajor != 4 || nMinor < 1 )
        {
#if OSL_DEBUG_LEVEL > 1
            sal_Int32 nMicro = 0;
            if( nIndex > 0 )
                nMicro = aVersion.getToken( 0, '.', nIndex ).toInt32();
            fprintf( stderr, "unsuitable qt version %d.%d.%d\n", nMajor, nMinor, nMicro );
#endif
            return NULL;
        }
#endif

        KDESalInstance* pInstance = new KDESalInstance( new SalYieldMutex() );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "created KDESalInstance 0x%p\n", pInstance );
#endif

        // initialize SalData
        KDEData *salData = new KDEData( pInstance );
        salData->Init();
        salData->initNWF();
        pInstance->SetLib(salData->GetLib());

        return pInstance;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
