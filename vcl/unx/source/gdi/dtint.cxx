/*************************************************************************
 *
 *  $RCSfile: dtint.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pl $ $Date: 2001-08-20 11:05:09 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <prex.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <postx.h>

#ifdef USE_CDE
#include <cdeint.hxx>
#endif
#include <kdeint.hxx>
#include <soicon.hxx>
#include <saldisp.hxx>
#include <saldata.hxx>
#include <salbmp.hxx>
#include <salframe.hxx>

#include <strhelper.hxx>
#include <svapp.hxx>

#include <unistd.h>

#ifndef _VOS_PROCESS_HXX
#include <vos/process.hxx>
#endif

#include <tools/urlobj.hxx>

#ifdef SOLARIS
// Solaris 2.5.1 misses it in unistd.h
extern "C" int usleep(unsigned int);
#endif

#define MAX_TRY_CONVERTSELECTION 40
#define EVENTMASK_WHILE_DRAGGING ButtonPressMask | ButtonReleaseMask | PointerMotionMask

BOOL bSymbolLoadFailed = FALSE;

DtIntegratorList DtIntegrator::aIntegratorList;
String DtIntegrator::aHomeDir;

DtIntegrator::DtIntegrator( SalFrame* pFrame ) :
        mpSalFrame( pFrame ),
        mpSalDisplay( pFrame->maFrameData.GetDisplay() ),
        meType( DtGeneric ),
        mnRefCount( 0 )
{
    mpDisplay = mpSalDisplay->GetDisplay();
    aIntegratorList.Insert( this, LIST_APPEND );
    aHomeDir = String( getenv( "HOME" ), gsl_getSystemTextEncoding() );
}

DtIntegrator::~DtIntegrator()
{
}

DtIntegrator* DtIntegrator::CreateDtIntegrator( SalFrame* pFrame )
{
    // hack for sclient
    if( ! pFrame && aIntegratorList.Count() )
        return aIntegratorList.GetObject( 0 );

    for( int i = 0; i < aIntegratorList.Count(); i++ )
    {
        DtIntegrator* pIntegrator = aIntegratorList.GetObject( i );
        if( pIntegrator->mpDisplay == pFrame->maFrameData.GetXDisplay() )
            return pIntegrator;
    }

    if( ! pFrame )
        pFrame = GetSalData()->pFirstFrame_;

#ifndef REMOTE_APPSERVER
    Display* pDisplay = pFrame->maFrameData.GetXDisplay();
    Atom nDtAtom;
    void* pLibrary = NULL;

#ifdef USE_CDE
    // check dt type
    // CDE
    nDtAtom = XInternAtom( pDisplay, "_DT_WM_READY", True );
    if( nDtAtom && ( pLibrary = _LoadLibrary( "libDtSvc.so" ) ) )
    {
        // performance: do not dlopen DtSvc twice
        CDEIntegrator::pDtSvcLib = pLibrary;
        return new CDEIntegrator( pFrame );
    }
#endif

    nDtAtom = XInternAtom( pDisplay, "KWM_RUNNING", True );
    if( nDtAtom ) // perhaps should check getenv( "KDEDIR" )
        return new KDEIntegrator( pFrame );
#endif
    // default: generic implementation
    return new DtIntegrator( pFrame );
}

BOOL DtIntegrator::GetSystemLook( SystemLookInfo& rInfo )
{
    return FALSE;
}
