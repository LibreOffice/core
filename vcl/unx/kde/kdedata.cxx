/*************************************************************************
 *
 *  $RCSfile: kdedata.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-01-07 09:25:01 $
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
 * class KDEXLib                                                           *
 ***************************************************************************/

class KDEXLib : public SalXLib
{
public:
    KDEXLib() : SalXLib() {}
    virtual ~KDEXLib() {}
    virtual void Init();
};

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

    int nFakeArgc = 1;
    char** pFakeArgv = NULL;
    USHORT nIdx;
    vos::OExtCommandLine aCommandLine;
    int nParams = aCommandLine.getCommandArgCount();
    rtl::OString aDisplay;
    rtl::OUString aParam, aBin;

    for ( nIdx = 0; nIdx < nParams; ++nIdx )
    {
        aCommandLine.getCommandArg( nIdx, aParam );
        if ( !pFakeArgv && aParam.equalsAscii( "-display" ) && nIdx + 1 < nParams )
        {
            aCommandLine.getCommandArg( nIdx + 1, aParam );
            aDisplay = rtl::OUStringToOString( aParam, osl_getThreadTextEncoding() );

            nFakeArgc = 3;
            pFakeArgv = new char*[ nFakeArgc ];
            pFakeArgv[ 1 ] = strdup( "-display" );
            pFakeArgv[ 2 ] = strdup( aDisplay.getStr() );
        }
    }
    if ( !pFakeArgv )
        pFakeArgv = new char*[ nFakeArgc ];

    osl_getExecutableFile( &aParam.pData );
    osl_getSystemPathFromFileURL( aParam.pData, &aBin.pData );
    pFakeArgv[0] = strdup( rtl::OUStringToOString( aBin, osl_getThreadTextEncoding() ).getStr() );

    KCmdLineArgs::init( nFakeArgc, pFakeArgv, kAboutData );

    KApplication::disableAutoDcopRegistration();
    new KApplication();

    Display* pDisp = QPaintDevice::x11AppDisplay();
    XVisualInfo aVI;
    Colormap    aColMap;
    int         nScreen = DefaultScreen( pDisp );
    if( SalDisplay::BestVisual( pDisp, nScreen, aVI ) ) // DefaultVisual
        aColMap = DefaultColormap( pDisp, nScreen );
    else
        aColMap = XCreateColormap( pDisp,
                                   RootWindow( pDisp, nScreen ),
                                   aVI.visual,
                                   AllocNone );

    SalDisplay *pSalDisplay = new SalX11Display( pDisp, aVI.visual, aColMap );

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
    SalInstance* create_SalInstance( oslModule pModule )
    {
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
