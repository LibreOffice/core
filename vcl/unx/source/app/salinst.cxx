/*************************************************************************
 *
 *  $RCSfile: salinst.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cp $ $Date: 2000-11-17 18:35:58 $
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

#define _SV_SALINST_CXX

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <salunx.h>

#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif
#ifndef _SV_SALATYPE_HXX
#include <salatype.hxx>
#endif
#ifndef _SV_DTINT_HXX
#include <dtint.hxx>
#endif
#ifndef _SV_SALPRN_H
#include <salprn.h>
#endif
#ifndef USE_PSPRINT
#ifndef _VCL_SALCONFIG_HXX
#include <salconfig.hxx>
#endif
#endif

// -=-= C++ globals =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalAbort( const XubString& rErrorText )
{
    if( !rErrorText.Len() )
        fprintf( stderr, "Application Error" );
    else
        fprintf( stderr, ByteString( rErrorText, gsl_getSystemTextEncoding() ).GetBuffer() );
    abort();
}


// -------------------------------------------------------------------------
//
// SalYieldMutex
//
// -------------------------------------------------------------------------

SalYieldMutex::SalYieldMutex()
{
    mnCount     = 0;
    mnThreadId  = 0;
}

void SalYieldMutex::acquire()
{
    OMutex::acquire();
    mnThreadId = NAMESPACE_VOS(OThread)::getCurrentIdentifier();
    mnCount++;
}

void SalYieldMutex::release()
{
    if ( mnThreadId == NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
    {
        if ( mnCount == 1 )
            mnThreadId = 0;
        mnCount--;
    }
    OMutex::release();
}

sal_Bool SalYieldMutex::tryToAcquire()
{
    if ( OMutex::tryToAcquire() )
    {
        mnThreadId = NAMESPACE_VOS(OThread)::getCurrentIdentifier();
        mnCount++;
        return True;
    }
    else
        return False;
}

//----------------------------------------------------------------------------

final void InitSalData()
{
    SalData *pSalData = new SalData;
    SetSalData( pSalData );
}

final void DeInitSalData()
{
    SalData *pSalData = GetSalData();
    delete pSalData;
    SetSalData( NULL );
}

final void SetFilterCallback( void* pCallback, void* pInst )
{
    SalData* pSalData = GetSalData();

    pSalData->pFirstInstance_->maInstData.mpFilterCallback = pCallback;
    pSalData->pFirstInstance_->maInstData.mpFilterInst = pInst;
}

final SalInstance *CreateSalInstance()
{
    SalData     *pSalData = GetSalData();
    SalInstance *pInst    = new SalInstance;

    // init instance (only one instance in this version !!!)
    pSalData->pFirstInstance_ = pInst;

#ifndef USE_PSPRINT
    StartPrinterListening();
#endif

    return pInst;
}

final void DestroySalInstance( SalInstance *pInst )
{
    SalData *pSalData = GetSalData();

    // reset instance (only one instance in this version !!!)
    if( pSalData->pFirstInstance_ == pInst )
    {
#ifndef USE_PSPRINT
        StopPrinterListening();
#endif
        pSalData->pFirstInstance_ = NULL;
#ifndef USE_PSPRINT
        ::vcl_sal::XpDefaults::release();
#endif
    }

    delete pInst;
}

// -=-= SalInstance =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
final SalInstance::SalInstance()
{
    maInstData.mpFilterCallback = NULL;
    maInstData.mpFilterInst     = NULL;
    maInstData.mpSalYieldMutex  = new SalYieldMutex;
    maInstData.mpSalYieldMutex->acquire();
}

final SalInstance::~SalInstance()
{
// #75711# - java is running
      maInstData.mpSalYieldMutex->release();
      delete maInstData.mpSalYieldMutex;
}


// --------------------------------------------------------
// AnyInput from sv/mow/source/app/svapp.cxx

struct PredicateReturn
{
    USHORT  nType;
    BOOL    bRet;
};

Bool ImplPredicateEvent( Display *, XEvent *pEvent, char *pData )
{
    PredicateReturn *pPre = (PredicateReturn *)pData;

    if ( pPre->bRet )
        return False;

    USHORT nType;

    switch( pEvent->type )
    {
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
        case EnterNotify:
        case LeaveNotify:
            nType = INPUT_MOUSE;
            break;

        case XLIB_KeyPress:
        //case KeyRelease:
            nType = INPUT_KEYBOARD;
            break;
        case Expose:
        case GraphicsExpose:
        case NoExpose:
            nType = INPUT_PAINT;
            break;
        default:
            nType = 0;
    }

    if ( nType & pPre->nType || ( ! nType && pPre->nType & INPUT_OTHER ) )
        pPre->bRet = TRUE;

    return False;
}


alpha BOOL SalInstance::AnyInput(USHORT nType)
{
    SalData *pSalData = GetSalData();
    Display *pDisplay  = pSalData->GetDefDisp()->GetDisplay();

    // XtInputMask nMask = XtAppPending( SVData().GetAppContext() );
    // if( nMask )
    if (XPending(pDisplay) )
    {
        // if ( INPUT_TIMER & nType && XtIMTimer & nMask )
        //  return TRUE;
        // else
        {
            PredicateReturn aInput;
            XEvent          aEvent;

            aInput.bRet     = FALSE;
            aInput.nType    = nType;

            XCheckIfEvent(pDisplay, &aEvent, ImplPredicateEvent,
                          (char *)&aInput );

            return aInput.bRet;
        }
    }
    return FALSE ;
}

#ifdef _VOS_NO_NAMESPACE
IMutex* SalInstance::GetYieldMutex()
#else
vos::IMutex* SalInstance::GetYieldMutex()
#endif
{
    return maInstData.mpSalYieldMutex;
}

// -----------------------------------------------------------------------

ULONG SalInstance::ReleaseYieldMutex()
{
    SalYieldMutex* pYieldMutex = maInstData.mpSalYieldMutex;
    if ( pYieldMutex->GetThreadId() ==
         NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
    {
        ULONG nCount = pYieldMutex->GetAcquireCount();
        ULONG n = nCount;
        while ( n )
        {
            pYieldMutex->release();
            n--;
        }

        return nCount;
    }
    else
        return 0;
}

// -----------------------------------------------------------------------

void SalInstance::AcquireYieldMutex( ULONG nCount )
{
    SalYieldMutex* pYieldMutex = maInstData.mpSalYieldMutex;
    while ( nCount )
    {
        pYieldMutex->acquire();
        nCount--;
    }
}

final void SalInstance::Yield( BOOL bWait )
{ GetSalData()->GetLib()->Yield( bWait ); }

