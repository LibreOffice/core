/*************************************************************************
 *
 *  $RCSfile: salinst.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: pluby $ $Date: 2000-11-19 02:37:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *   - GNU Lesser General Public License Version 2.1
 *   - Sun Industry Standards Source License Version 1.1
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

#include <stdio.h>

#define _SV_SALINST_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif
#ifndef _SV_SALSYS_H
#include <salsys.hxx>
#endif
#ifndef _SV_VCLAPPLICATION_H
#include <VCLApplication.h>
#endif

// =======================================================================

void SalAbort( const XubString& rErrorText )
{
    if( !rErrorText.Len() )
        fprintf( stderr, "Application Error" );
    else
        fprintf( stderr,
            ByteString( rErrorText, gsl_getSystemTextEncoding() ).GetBuffer() );
    abort();
}

// -----------------------------------------------------------------------

void InitSalData()
{
    SalData *pSalData = new SalData;
    SetSalData( pSalData );
}

// -----------------------------------------------------------------------

void DeInitSalData()
{
    SalData *pSalData = GetSalData();
    delete pSalData;
    SetSalData( NULL );
}

// -----------------------------------------------------------------------

void SetFilterCallback( void* pCallback, void* pInst )
{
    SalData *pSalData = GetSalData();
    pSalData->mpFirstInstance->maInstData.mpFilterCallback = pCallback;
    pSalData->mpFirstInstance->maInstData.mpFilterInst = pInst;
}

// =======================================================================

SalInstance* CreateSalInstance()
{
    SalData* pSalData = GetSalData();
    SalInstance* pInst = new SalInstance;

    // init instance (only one instance in this version !!!)
    pSalData->mpFirstInstance = pInst;

    return pInst;
}

// -----------------------------------------------------------------------

void DestroySalInstance( SalInstance* pInst )
{
    delete pInst;
}

// -----------------------------------------------------------------------

SalInstance::SalInstance()
{
}

// -----------------------------------------------------------------------

SalInstance::~SalInstance()
{
}

// -----------------------------------------------------------------------

vos::IMutex* SalInstance::GetYieldMutex()
{
    return NULL;
}

// -----------------------------------------------------------------------

ULONG SalInstance::ReleaseYieldMutex()
{
    return 0;
}

// -----------------------------------------------------------------------

void SalInstance::AcquireYieldMutex( ULONG nCount )
{
}

// -----------------------------------------------------------------------

void SalInstance::Yield( BOOL bWait )
{
    // Start the event queue. Note that VCLApplication_run() will not return
    // until the application shuts down. On other platforms, this function
    // returns after each event is pulled off the event queue and dispatched.
    // Instead, we have enter this method only once and let VCLApplication_run
    // do all of the pulling and dispatching of events.
    VCLApplication_run();
}

// -----------------------------------------------------------------------

BOOL SalInstance::AnyInput( USHORT nType )
{
    return FALSE;
}

// -----------------------------------------------------------------------

SalFrame* SalInstance::CreateChildFrame( SystemParentData* pSystemParentData, ULONG nSalFrameStyle )
{
    return NULL;
}

// -----------------------------------------------------------------------

SalFrame* SalInstance::CreateFrame( SalFrame* pParent, ULONG nSalFrameStyle )
{
    SalFrame *pFrame = new SalFrame;

    pFrame->maFrameData.mpParent = pParent;

    // Create the native window
    pFrame->maFrameData.mhWnd = VCLWindow_new( nSalFrameStyle, NULL,
        pFrame, &(pFrame->maFrameData) );

    return pFrame;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyFrame( SalFrame* pFrame )
{
    delete ( pFrame );
}

// -----------------------------------------------------------------------

SalObject* SalInstance::CreateObject( SalFrame* pParent )
{
    SalObject *pObject = new SalObject;

    pObject->maObjectData.mhWnd = pParent->maFrameData.mhWnd;

    return pObject;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyObject( SalObject* pObject )
{
    delete ( pObject );
}

void SalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
}

// -----------------------------------------------------------------------

void SalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
}

// -----------------------------------------------------------------------

void SalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
}

// -----------------------------------------------------------------------

XubString SalInstance::GetDefaultPrinter()
{
    return XubString();
}

// -----------------------------------------------------------------------

SalInfoPrinter* SalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                ImplJobSetup* pSetupData )
{
    return NULL;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
}

// -----------------------------------------------------------------------

SalSystem* SalInstance::CreateSystem()
{
    return new SalSystem();
}

// -----------------------------------------------------------------------

void SalInstance::DestroySystem( SalSystem* pSystem )
{
    delete pSystem;
}
