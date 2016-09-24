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

#include <shell/kde_headers.h>

#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <poll.h>
#ifdef FREEBSD
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <osl/thread.h>
#include <osl/process.h>
#include <osl/mutex.hxx>

#include "unx/kde/kdedata.hxx"
#include "unx/i18n_im.hxx"
#include "unx/i18n_xkb.hxx"

#include "vclpluginapi.h"

#include <config_vclplug.h>

/* #i59042# override KApplications method for session management
 * since it will interfere badly with our own.
 */
class VCLKDEApplication : public KApplication
{
    public:
    VCLKDEApplication() : KApplication() {}

    virtual void commitData(QSessionManager &sm) override;
};

void VCLKDEApplication::commitData(QSessionManager&)
{
}

/***************************************************************************
 * class SalKDEDisplay                                                     *
 ***************************************************************************/

SalKDEDisplay::SalKDEDisplay( Display* pDisp )
    : SalX11Display( pDisp )
{
}

SalKDEDisplay::~SalKDEDisplay()
{
    // in case never a frame opened
    static_cast<KDEXLib*>(GetXLib())->doStartup();
    // clean up own members
    doDestruct();
    // prevent SalDisplay from closing KApplication's display
    pDisp_ = NULL;
}

/***************************************************************************
 * class KDEXLib                                                           *
 ***************************************************************************/

KDEXLib::~KDEXLib()
{
    // on 64 bit linux using libXRandr.so.2 will crash in
    // XCloseDisplay when freeing extension data
    // no known work around, therefore currently leak. Hopefully
    // this does not make problems since we're shutting down anyway
    // should we ever get a real kde plugin that uses the KDE event loop
    // we should use kde's method to signal screen changes similar
    // to the gtk plugin
    #if ! defined USE_RANDR || ! (defined LINUX && defined X86_64)
    // properly deinitialize KApplication
    delete static_cast<VCLKDEApplication*>(m_pApplication);
    #endif
    // free the faked cmdline arguments no longer needed by KApplication
    for( int i = 0; i < m_nFakeCmdLineArgs; i++ )
        free( m_pFreeCmdLineArgs[i] );
    delete [] m_pFreeCmdLineArgs;
    delete [] m_pAppCmdLineArgs;
}

void KDEXLib::Init()
{
    SalI18N_InputMethod* pInputMethod = new SalI18N_InputMethod;
    pInputMethod->SetLocale();
    XrmInitialize();

    KAboutData *kAboutData = new KAboutData( "LibreOffice",
            I18N_NOOP( "LibreOffice" ),
            "3.4.0",
            I18N_NOOP( "LibreOffice with KDE Native Widget Support." ),
            KAboutData::License_File,
            I18N_NOOP( "Copyright (C) 2000, 2014 LibreOffice contributors" ),
            I18N_NOOP( "LibreOffice is an office suite.\n" ),
            "http://libreoffice.org",
            "libreoffice@lists.freedesktop.org");
    kAboutData->addAuthor( "Jan Holesovsky",
            I18N_NOOP( "Original author and maintainer of the KDE NWF." ),
            "kendy@artax.karlin.mff.cuni.cz",
            "http://artax.karlin.mff.cuni.cz/~kendy" );

    m_nFakeCmdLineArgs = 1;
    sal_uInt16 nIdx;
    int nParams = osl_getCommandArgCount();
    OString aDisplay;
    OUString aParam, aBin;

    for ( nIdx = 0; nIdx < nParams; ++nIdx )
    {
        osl_getCommandArg( nIdx, &aParam.pData );
        if ( !m_pFreeCmdLineArgs && aParam == "-display" && nIdx + 1 < nParams )
        {
            osl_getCommandArg( nIdx + 1, &aParam.pData );
            aDisplay = OUStringToOString( aParam, osl_getThreadTextEncoding() );

            m_nFakeCmdLineArgs = 3;
            m_pFreeCmdLineArgs = new char*[ m_nFakeCmdLineArgs ];
            m_pFreeCmdLineArgs[ 1 ] = strdup( "-display" );
            m_pFreeCmdLineArgs[ 2 ] = strdup( aDisplay.getStr() );
        }
    }
    if ( !m_pFreeCmdLineArgs )
        m_pFreeCmdLineArgs = new char*[ m_nFakeCmdLineArgs ];

    osl_getExecutableFile( &aParam.pData );
    osl_getSystemPathFromFileURL( aParam.pData, &aBin.pData );
    OString aExec = OUStringToOString( aBin, osl_getThreadTextEncoding() );
    m_pFreeCmdLineArgs[0] = strdup( aExec.getStr() );

    // make a copy of the string list for freeing it since
    // KApplication manipulates the pointers inside the argument vector
    // note: KApplication bad !
    m_pAppCmdLineArgs = new char*[ m_nFakeCmdLineArgs ];
    for( int i = 0; i < m_nFakeCmdLineArgs; i++ )
        m_pAppCmdLineArgs[i] = m_pFreeCmdLineArgs[i];

    KCmdLineArgs::init( m_nFakeCmdLineArgs, m_pAppCmdLineArgs, kAboutData );

    KApplication::disableAutoDcopRegistration();
    m_pApplication = new VCLKDEApplication();
    kapp->disableSessionManagement();

    Display* pDisp = QPaintDevice::x11AppDisplay();

    SalX11Display *pSalDisplay = new SalKDEDisplay( pDisp );

    pInputMethod->CreateMethod( pDisp );
    pSalDisplay->SetupInput( pInputMethod );
}

void KDEXLib::doStartup()
{
    if( ! m_bStartupDone )
    {
        KStartupInfo::appStarted();
        m_bStartupDone = true;
        SAL_INFO( "vcl.kde", "called KStartupInfo::appStarted()" );
    }
}

/**********************************************************************
 * class KDEData                                                      *
 **********************************************************************/

KDEData::~KDEData()
{
}

void KDEData::Init()
{
    pXLib_ = new KDEXLib();
    pXLib_->Init();
}

/**********************************************************************
 * plugin entry point                                                 *
 **********************************************************************/

extern "C" {
    VCLPLUG_KDE_PUBLIC SalInstance* create_SalInstance()
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

        OString aVersion( qVersion() );
        SAL_INFO( "vcl.kde", "qt version string is \"" << aVersion << "\"" );

        sal_Int32 nIndex = 0, nMajor = 0, nMinor = 0, nMicro = 0;
        nMajor = aVersion.getToken( 0, '.', nIndex ).toInt32();
        if( nIndex > 0 )
            nMinor = aVersion.getToken( 0, '.', nIndex ).toInt32();
        if( nIndex > 0 )
            nMicro = aVersion.getToken( 0, '.', nIndex ).toInt32();
        if( nMajor != 3 || nMinor < 2 || (nMinor == 2 && nMicro < 2) )
        {
            SAL_INFO( "vcl.kde", "unsuitable qt version " << nMajor << "." << nMinor << "." << nMicro );
            return NULL;
        }

        KDESalInstance* pInstance = new KDESalInstance( new SalYieldMutex() );
        SAL_INFO( "vcl.kde", "created KDESalInstance " << &pInstance );

        // initialize SalData
        KDEData *pSalData = new KDEData( pInstance );
        pSalData->Init();
        pInstance->SetLib( pSalData->GetLib() );
        pSalData->initNWF();

        return pInstance;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
