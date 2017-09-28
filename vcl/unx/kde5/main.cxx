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

#include <QtGui/QApplication>

#include "KDE5Data.hxx"
#include "KDE5SalInstance.hxx"

#include <vclpluginapi.h>

#include <rtl/string.hxx>

/// entry point for the KDE4 VCL plugin
extern "C" {
    VCLPLUG_KDE4_PUBLIC SalInstance* create_SalInstance()
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
        OString aVersion( qVersion() );
        SAL_INFO( "vcl.kde5", "qt version string is " << aVersion );

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
            SAL_INFO( "vcl.kde5", "unsuitable qt version " << nMajor << "." << nMinor << "." << nMicro );
#endif
            return nullptr;
        }
#endif

        KDE5SalInstance* pInstance = new KDE5SalInstance( new SalYieldMutex() );
        SAL_INFO( "vcl.kde5", "created KDE5SalInstance " << &pInstance );

        // initialize SalData
        KDE5Data *salData = new KDE5Data( pInstance );
        salData->Init();
        salData->initNWF();
        //pInstance->SetLib(salData->GetLib());

        return pInstance;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
