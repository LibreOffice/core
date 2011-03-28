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

#include "VCLKDEApplication.hxx"

#define Region QtXRegion

#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kstartupinfo.h>

#undef Region

#include "KDEXLib.hxx"

#include <i18n_im.hxx>
#include <i18n_xkb.hxx>

#include <saldata.hxx>
#include <vos/process.hxx>

#include "KDESalDisplay.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

KDEXLib::KDEXLib() :
    SalXLib(),  m_bStartupDone(false), m_pApplication(0),
    m_pFreeCmdLineArgs(0), m_pAppCmdLineArgs(0), m_nFakeCmdLineArgs( 0 )
{
}

KDEXLib::~KDEXLib()
{
    delete (VCLKDEApplication*)m_pApplication;

    // free the faked cmdline arguments no longer needed by KApplication
    for( int i = 0; i < m_nFakeCmdLineArgs; i++ )
    {
        free( m_pFreeCmdLineArgs[i] );
    }

    delete [] m_pFreeCmdLineArgs;
    delete [] m_pAppCmdLineArgs;
}

void KDEXLib::Init()
{
    SalI18N_InputMethod* pInputMethod = new SalI18N_InputMethod;
    pInputMethod->SetLocale();
    XrmInitialize();

    KAboutData *kAboutData = new KAboutData("OpenOffice.org",
            "kdelibs4",
            ki18n( "OpenOffice.org" ),
            "3.0.0",
            ki18n( "OpenOffice.org with KDE Native Widget Support." ),
            KAboutData::License_LGPL,
            ki18n( "Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Novell, Inc"),
            ki18n( "OpenOffice.org is an office suite.\n" ),
            "http://kde.openoffice.org/index.html",
            "dev@kde.openoffice.org" );

    kAboutData->addAuthor( ki18n( "Jan Holesovsky" ),
            ki18n( "Original author and maintainer of the KDE NWF." ),
            "kendy@artax.karlin.mff.cuni.cz",
            "http://artax.karlin.mff.cuni.cz/~kendy" );
    kAboutData->addAuthor( ki18n("Roman Shtylman"),
            ki18n( "Porting to KDE 4." ),
            "shtylman@gmail.com", "http://shtylman.com" );
    kAboutData->addAuthor( ki18n("Eric Bischoff"),
            ki18n( "Accessibility fixes, porting to KDE 4." ),
            "bischoff@kde.org" );

    //kAboutData->setProgramIconName("OpenOffice");

    m_nFakeCmdLineArgs = 1;
    int nIdx;
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

    m_pApplication = new VCLKDEApplication();
    kapp->disableSessionManagement();
    KApplication::setQuitOnLastWindowClosed(false);

    Display* pDisp = QX11Info::display();
    SalKDEDisplay *pSalDisplay = new SalKDEDisplay(pDisp);

    ((VCLKDEApplication*)m_pApplication)->disp = pSalDisplay;

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
