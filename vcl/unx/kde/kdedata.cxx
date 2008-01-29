/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kdedata.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:21:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#define _SV_SALDATA_CXX

#ifndef INCLUDED_VCL_KDE_HEADERS_H
#include "kde_headers.h"
#endif

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

#ifndef _VCL_KDEDATA_HXX
#include <plugins/kde/kdedata.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#include <osl/module.h>

#include <tools/debug.hxx>

#ifndef _SAL_I18N_INPUTMETHOD_HXX
#include "i18n_im.hxx"
#endif
#ifndef _SAL_I18N_XKBDEXTENSION_HXX
#include "i18n_xkb.hxx"
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

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
    // properly deinitialize KApplication
    delete (VCLKDEApplication*)m_pApplication;
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

    KAboutData *kAboutData = new KAboutData( "OpenOffice.org",
            I18N_NOOP( "OpenOffice.org" ),
            "1.1.0",
            I18N_NOOP( "OpenOffice.org with KDE Native Widget Support." ),
            KAboutData::License_LGPL,
            "(c) 2003, 2004 Novell, Inc",
            I18N_NOOP( "OpenOffice.org is an office suite.\n" ),
            "http://kde.openoffice.org/index.html",
            "dev@kde.openoffice.org");
    kAboutData->addAuthor( "Jan Holesovsky",
            I18N_NOOP( "Original author and maintainer of the KDE NWF." ),
            "kendy@artax.karlin.mff.cuni.cz",
            "http://artax.karlin.mff.cuni.cz/~kendy" );

    m_nFakeCmdLineArgs = 1;
    USHORT nIdx;
    vos::OExtCommandLine aCommandLine;
    int nParams = aCommandLine.getCommandArgCount();
    rtl::OString aDisplay;
    rtl::OUString aParam, aBin;

    for ( nIdx = 0; nIdx < nParams; ++nIdx )
    {
        aCommandLine.getCommandArg( nIdx, aParam );
        if ( !m_pFreeCmdLineArgs && aParam.equalsAscii( "-display" ) && nIdx + 1 < nParams )
        {
            aCommandLine.getCommandArg( nIdx + 1, aParam );
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

    XSetIOErrorHandler    ( (XIOErrorHandler)X11SalData::XIOErrorHdl );
    XSetErrorHandler      ( (XErrorHandler)X11SalData::XErrorHdl );

    pInputMethod->CreateMethod( pDisp );
    pInputMethod->AddConnectionWatch( pDisp, (void*)this );
    pSalDisplay->SetInputMethod( pInputMethod );

    PushXErrorLevel( true );
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp );
    XSync( pDisp, False );

    pKbdExtension->UseExtension( ! HasXErrorOccured() );
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
            fprintf( stderr, "unsuitable qt version %d.%d.%d\n", nMajor, nMinor, nMicro );
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
