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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#define _SV_SALDATA_CXX
#include <shell/kde_headers.h>

#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <poll.h>
#ifdef FREEBSD
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif
#include <plugins/kde/kdedata.hxx>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/module.h>
#include <osl/mutex.hxx>

#include <tools/debug.hxx>
#include "i18n_im.hxx"
#include "i18n_xkb.hxx"

/* #i59042# override KApplications method for session management
 * since it will interfere badly with our own.
 */
class VCLKDEApplication : public KApplication
{
    public:
    VCLKDEApplication() : KApplication() {}

    virtual void commitData(QSessionManager &sm);
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
    // no known work around, therefor currently leak. Hopefully
    // this does not make problems since we're shutting down anyway
    // should we ever get a real kde plugin that uses the KDE event loop
    // we should use kde's method to signal screen changes similar
    // to the gtk plugin
    #if ! defined USE_RANDR || ! (defined LINUX && defined X86_64)
    // properly deinitialize KApplication
    delete (VCLKDEApplication*)m_pApplication;
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
            "1.1.0",
            I18N_NOOP( "LibreOffice with KDE Native Widget Support." ),
            KAboutData::License_LGPL,
            "(c) 2003, 2004 Novell, Inc",
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
    rtl::OString aDisplay;
    rtl::OUString aParam, aBin;

    for ( nIdx = 0; nIdx < nParams; ++nIdx )
    {
        osl_getCommandArg( nIdx, &aParam.pData );
        if ( !m_pFreeCmdLineArgs && aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-display" ) ) && nIdx + 1 < nParams )
        {
            osl_getCommandArg( nIdx + 1, &aParam.pData );
            aDisplay = rtl::OUStringToOString( aParam, osl_getThreadTextEncoding() );

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
    rtl::OString aExec = rtl::OUStringToOString( aBin, osl_getThreadTextEncoding() );
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

    SalDisplay *pSalDisplay = new SalKDEDisplay( pDisp );

    pInputMethod->CreateMethod( pDisp );
    pInputMethod->AddConnectionWatch( pDisp, (void*)this );
    pSalDisplay->SetInputMethod( pInputMethod );

    PushXErrorLevel( true );
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp );
    XSync( pDisp, False );

    pKbdExtension->UseExtension( ! HasXErrorOccurred() );
    PopXErrorLevel();

    pSalDisplay->SetKbdExtension( pKbdExtension );
}

void KDEXLib::doStartup()
{
    if( ! m_bStartupDone )
    {
        KStartupInfo::appStarted();
        m_bStartupDone = true;
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "called KStartupInfo::appStarted()\n" );
        #endif
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
    VCL_DLLPUBLIC SalInstance* create_SalInstance( oslModule )
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
        if( nMajor != 3 || nMinor < 2 || (nMinor == 2 && nMicro < 2) )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "unsuitable qt version %d.%d.%d\n", (int)nMajor, (int)nMinor, (int)nMicro );
#endif
            return NULL;
        }

        KDESalInstance* pInstance = new KDESalInstance( new SalYieldMutex() );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "created KDESalInstance 0x%p\n", pInstance );
#endif

        // initialize SalData
        KDEData *pSalData = new KDEData();
        SetSalData( pSalData );
        pSalData->m_pInstance = pInstance;
        pSalData->Init();
        pSalData->initNWF();

        return pInstance;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
