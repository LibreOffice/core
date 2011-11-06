/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#define Region QtXRegion
#include <QApplication>
#undef Region

#include "KDEData.hxx"
#include "KDESalInstance.hxx"

#include "vclpluginapi.h"

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
            XInitThreads();

#if QT_VERSION < 0x050000
        // Qt 4.x support needs >= 4.1.0
        rtl::OString aVersion( qVersion() );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "qt version string is \"%s\"\n", aVersion.getStr() );
#endif
        sal_Int32 nIndex = 0, nMajor = 0, nMinor = 0, nMicro = 0;
        nMajor = aVersion.getToken( 0, '.', nIndex ).toInt32();
        if( nIndex > 0 )
            nMinor = aVersion.getToken( 0, '.', nIndex ).toInt32();
        if( nIndex > 0 )
            nMicro = aVersion.getToken( 0, '.', nIndex ).toInt32();
        if( nMajor != 4 || nMinor < 1 )
        {
#if OSL_DEBUG_LEVEL > 1
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
        KDEData *salData = new KDEData();
        SetSalData(salData);
        salData->m_pInstance = pInstance;
        salData->Init();
        salData->initNWF();

        return pInstance;
    }
}
