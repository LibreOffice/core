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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "osl/file.h"
#include "osl/process.h"
#include "osl/security.h"

#include "vcl/svapp.hxx"

#include "unx/salunx.h"
#include <X11/Xatom.h>
#ifdef USE_CDE
#include "unx/cdeint.hxx"
#endif
#include "unx/dtint.hxx"
#include "unx/saldisp.hxx"
#include "unx/saldata.hxx"
#include "unx/wmadaptor.hxx"

#include "dtsetenum.hxx"

#include <set>
#include <stdio.h>

// NETBSD has no RTLD_GLOBAL
#ifndef RTLD_GLOBAL
#define DLOPEN_MODE (RTLD_LAZY)
#else
#define DLOPEN_MODE (RTLD_GLOBAL | RTLD_LAZY)
#endif


using namespace rtl;
using namespace vcl_sal;

String DtIntegrator::aHomeDir;

DtIntegrator::DtIntegrator() :
        meType( DtGeneric ),
        mnSystemLookCommandProcess( -1 )
{
    mpSalDisplay = GetX11SalData()->GetDisplay();
    mpDisplay = mpSalDisplay->GetDisplay();
    OUString aDir;
    oslSecurity aCur = osl_getCurrentSecurity();
    if( aCur )
    {
        osl_getHomeDir( aCur, &aDir.pData );
        osl_freeSecurityHandle( aCur );
        OUString aSysDir;
        osl_getSystemPathFromFileURL( aDir.pData, &aSysDir.pData );
        aHomeDir = aSysDir;
    }
}

DtIntegrator::~DtIntegrator()
{
}

DtIntegrator* DtIntegrator::CreateDtIntegrator()
{
    /*
     *  #i22061# override desktop detection
     *  if environment variable OOO_FORCE_DESKTOP is set
     *  to one of "cde" "kde" "gnome" then autodetection
     *  is overridden.
     */
    static const char* pOverride = getenv( "OOO_FORCE_DESKTOP" );
    if( pOverride && *pOverride )
    {
        OString aOver( pOverride );

#if USE_CDE
        if( aOver.equalsIgnoreAsciiCase( "cde" ) )
            return new CDEIntegrator();
#endif
        if( aOver.equalsIgnoreAsciiCase( "none" ) )
            return new DtIntegrator();
    }

#ifdef USE_CDE
    void* pLibrary = NULL;

    // check dt type
    // CDE
    SalDisplay* pSalDisplay = GetX11SalData()->GetDisplay();
    Display* pDisplay = pSalDisplay->GetDisplay();
    Atom nDtAtom = XInternAtom( pDisplay, "_DT_WM_READY", True );
    if( nDtAtom && ( pLibrary = dlopen( "/usr/dt/lib/libDtSvc.so", DLOPEN_MODE ) ) )
    {
        dlclose( pLibrary );
        return new CDEIntegrator();
    }
#endif

    // default: generic implementation
    return new DtIntegrator();
}

void DtIntegrator::GetSystemLook( AllSettings& rSettings )
{
    // #i48001# set a default blink rate
    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    aStyleSettings.SetCursorBlinkTime( 500 );
    rSettings.SetStyleSettings( aStyleSettings );
}
