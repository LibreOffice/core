/*************************************************************************
 *
 *  $RCSfile: urp_writer.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 09:25:39 $
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
#include <assert.h>
#include <stdio.h>

//#include <vos/thread.hxx>
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#include <osl/mutex.hxx>
#include <osl/conditn.h>

#include <typelib/typedescription.h>

#include <bridges/remote/connection.h>
#include <bridges/remote/remote.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <bridges/remote/counter.hxx>

#include "urp_writer.hxx"
#include "urp_bridgeimpl.hxx"
#include "urp_marshal.hxx"
#include "urp_dispatch.hxx"

#ifdef DEBUG
static MyCounter thisCounter( "DEBUG : WriterThread" );
#endif

using namespace ::osl;

namespace bridges_urp {

OWriterThread::OWriterThread( remote_Connection *pConnection, urp_BridgeImpl *pBridgeImpl,
                              uno_Environment *pEnvRemote) :
    m_pConnection( pConnection ),
    m_bAbort( sal_False ),
    m_pBridgeImpl( pBridgeImpl ),
    m_pEnvRemote( pEnvRemote )
{
    m_oslCondition = osl_createCondition();
    osl_resetCondition( m_oslCondition );
    m_pConnection->acquire( m_pConnection );

#ifdef DEBUG
    thisCounter.acquire();
#endif
}

OWriterThread::~OWriterThread()
{
    osl_destroyCondition( m_oslCondition );
    m_pConnection->release( m_pConnection );
#ifdef DEBUG
    thisCounter.release();
#endif
}


// touch is called with locked m_marshalingMutex
void OWriterThread::touch( sal_Bool bImmediately )
{
      if( bImmediately || m_pBridgeImpl->m_blockMarshaler.getPos() > m_pBridgeImpl->m_properties.nFlushBlockSize )
      {
        write();
      }
}


void OWriterThread::abort()
{
    m_bAbort = sal_True;
    osl_setCondition( m_oslCondition );
}


// must be called with locked marshaling mutex
void OWriterThread::write()
{
     if( ! m_pBridgeImpl->m_blockMarshaler.empty() && ! m_bAbort )
     {
        m_pBridgeImpl->m_blockMarshaler.finish( m_pBridgeImpl->m_nMarshaledMessages);
        m_pBridgeImpl->m_nMarshaledMessages = 0;

        sal_Int32 nLength = m_pBridgeImpl->m_blockMarshaler.getSize();
        sal_Int8 *pBuf = m_pBridgeImpl->m_blockMarshaler.getBuffer();

        if( nLength != m_pConnection->write( m_pConnection, pBuf, nLength ))
        {
            m_pBridgeImpl->m_blockMarshaler.restart();
            return;
        }
        m_pConnection->flush( m_pConnection );
        m_pBridgeImpl->m_blockMarshaler.restart();
    }
}

void OWriterThread::sendEmptyMessage()
{
    // must be called with locked marshaling mutex
    sal_Int32 a[2] = {0,0};
    if( m_pConnection )
    {
        m_pConnection->write( m_pConnection , (sal_Int8*) a , sizeof( sal_Int32) *2 );
    }
}

void OWriterThread::insertReleaseRemoteCall(
    rtl_uString *pOid,typelib_TypeDescriptionReference *pTypeRef)
{
    ::osl::MutexGuard guard( m_releaseCallMutex );

    struct RemoteReleaseCall call;
    call.sOid = pOid;
    call.typeInterface = pTypeRef;
    m_lstReleaseCalls.push_back( call );
}

/* The release calls for doubled interfaces
 *
 *
 ***/
void OWriterThread::executeReleaseRemoteCalls()
{
    sal_Bool bFound = sal_True;
    while( bFound )
    {
        struct RemoteReleaseCall call;
        {
            ::osl::MutexGuard guard( m_releaseCallMutex );
            if( m_lstReleaseCalls.size() )
            {
                call = m_lstReleaseCalls.front();
                m_lstReleaseCalls.pop_front();
            }
            else
            {
                bFound = sal_False;
            }
        }
        if( bFound )
        {
            sal_Bool bNeedsRelease = sal_False;

            typelib_TypeDescription *pInterfaceTypeDesc = 0;
            typelib_TypeDescription *pReleaseMethod = 0;

            call.typeInterface.getDescription( &pInterfaceTypeDesc );
            if( ! pInterfaceTypeDesc->bComplete )
            {
                typelib_typedescription_complete( &pInterfaceTypeDesc );
            }

            uno_Any any;
            uno_Any *pAny = &any;

            typelib_typedescriptionreference_getDescription(
                &pReleaseMethod ,
                ((typelib_InterfaceTypeDescription*)pInterfaceTypeDesc)->ppAllMembers[REMOTE_RELEASE_METHOD_INDEX] );

            urp_sendRequest( m_pEnvRemote , pReleaseMethod, call.sOid.pData,
                             (typelib_InterfaceTypeDescription*) pInterfaceTypeDesc,
                             0, 0 , &pAny );

            typelib_typedescription_release( pReleaseMethod );
            typelib_typedescription_release( pInterfaceTypeDesc );
        }
    }
}


void OWriterThread::run()
{
    while( sal_True )
    {
        // Wait for some work to do
        TimeValue value = { 0 , 1000 * m_pBridgeImpl->m_properties.nOnewayTimeoutMUSEC };
        osl_resetCondition( m_oslCondition );
        osl_waitCondition( m_oslCondition , &value );

        {
            // check if there are some release calls to be sent ....
              executeReleaseRemoteCalls();

            // write to the socket
            MutexGuard guard( m_pBridgeImpl->m_marshalingMutex );
            m_bWaitForTimeout = sal_False;
            if( ! m_pBridgeImpl->m_blockMarshaler.empty() )
            {
                write();
            }
            osl_resetCondition( m_oslCondition );
        }
        if( m_bAbort )
        {
            break;
        }

    }
}


}

