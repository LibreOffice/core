/*************************************************************************
 *
 *  $RCSfile: kdedata.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2005-07-25 14:22:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Jan Holesovsky <kendy@artax.karlin.mff.cuni.cz>
 *                  Michael Meeks <michael@ximian.com>
 *
 ************************************************************************/

#define _SV_SALDATA_CXX

#define Region QtXRegion
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kstartupinfo.h>
#include <qpaintdevice.h>
#undef Region

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

/***************************************************************************
 * class SalKDEDisplay                                                     *
 ***************************************************************************/

SalKDEDisplay::SalKDEDisplay( Display* pDisp, Visual* pVisual, Colormap aColMap )
    : SalX11Display( pDisp, pVisual, aColMap, false )
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
    delete (KApplication*)m_pApplication;
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
    m_pApplication = new KApplication();
    kapp->disableSessionManagement();

    Display* pDisp = QPaintDevice::x11AppDisplay();

    SalDisplay *pSalDisplay = new SalKDEDisplay( pDisp,
            static_cast< Visual * >( QPaintDevice::x11AppVisual() ),
            QPaintDevice::x11AppColormap() );

    pInputMethod->CreateMethod( pDisp );
    pInputMethod->AddConnectionWatch( pDisp, (void*)this );
    pSalDisplay->SetInputMethod( pInputMethod );

    sal_Bool bOldErrorSetting = GetIgnoreXErrors();
    SetIgnoreXErrors( True );
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp );
    XSync( pDisp, False );

    pKbdExtension->UseExtension( ! WasXError() );
    SetIgnoreXErrors( bOldErrorSetting );

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
    VCL_DLLPUBLIC SalInstance* create_SalInstance( oslModule pModule )
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
        SalData *pSalData = new KDEData();
        SetSalData( pSalData );
        pSalData->pInstance_ = pInstance;
        pSalData->Init();
        pSalData->initNWF();

        return pInstance;
    }
}
