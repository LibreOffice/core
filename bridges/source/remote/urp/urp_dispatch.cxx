/*************************************************************************
 *
 *  $RCSfile: urp_dispatch.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:50 $
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
#ifdef SOLARIS
#include <alloca.h>
#else
#include <malloc.h>
#endif

#include <osl/mutex.hxx>
#include <osl/diagnose.h>

#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>

#include <vos/timer.hxx>

#include <uno/mapping.hxx>
#include <uno/threadpool.h>

#include <bridges/remote/remote.h>
#include <bridges/remote/stub.hxx>
#include <bridges/remote/proxy.hxx>
#include <bridges/remote/remote.hxx>

#include "urp_bridgeimpl.hxx"
#include "urp_marshal.hxx"
#include "urp_dispatch.hxx"
#include "urp_job.hxx"
#include "urp_writer.hxx"
#include "urp_log.hxx"

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

namespace bridges_urp
{

void prepareRuntimeExceptionClientSide( uno_Any **ppException , const OUString &s)
{

    // TODO : add string to runtimeexception
    Type type = ::getCppuType( ( ::com::sun::star::uno::RuntimeException *) 0 );
    uno_type_any_construct( *ppException , 0 , type.getTypeLibType() , 0 );
}

void SAL_CALL urp_sendCloseConnection( uno_Environment *pEnvRemote )
{
    remote_Context *pContext = (remote_Context *) pEnvRemote->pContext;
    urp_BridgeImpl *pImpl = (urp_BridgeImpl*) ( pContext->m_pBridgeImpl );

    {
        MutexGuard guard( pImpl->m_marshalingMutex );
        sal_uInt8 nBitfield = 0;

        // send immeadiatly
        if( ! pImpl->m_blockMarshaler.empty() )
        {
            pImpl->m_pWriter->touch( sal_True );
        }

        pImpl->m_pWriter->sendEmptyMessage();
    }
}

void SAL_CALL urp_sendRequest(
    uno_Environment *pEnvRemote,
    typelib_TypeDescription * pMemberType,
    rtl_uString *pOid,
    typelib_InterfaceTypeDescription *pInterfaceType,
    void *pReturn,
    void *ppArgs[],
    uno_Any **ppException
    )
{
    remote_Context *pContext = (remote_Context *) pEnvRemote->pContext;
    urp_BridgeImpl *pImpl = (urp_BridgeImpl*) ( pContext->m_pBridgeImpl );

    uno_threadpool_Handle *pThreadpoolHandle = 0;
    sal_Bool bOneway = typelib_TypeClass_INTERFACE_METHOD == pMemberType->eTypeClass ?
        (( typelib_InterfaceMethodTypeDescription * ) pMemberType)->bOneWay :
        sal_False;
    sal_Bool bReleaseForTypeDescriptionNecessary = sal_False;

    // get thread id
    sal_Sequence *pThreadId = 0;
    uno_getIdOfCurrentThread( &pThreadId );
    ByteSequence aThreadId = ByteSequence( pThreadId , BYTESEQ_NOACQUIRE );

    ClientJob clientJob = ClientJob( pEnvRemote, pImpl );
    {
        MutexGuard guard( pImpl->m_marshalingMutex );

        if( pImpl->m_bDisposed )
        {
            prepareRuntimeExceptionClientSide(
                ppException , OUString( RTL_CONSTASCII_USTRINGPARAM( "URP-Bridge: disposed" )) );
            return;
        }

        clientJob.m_ppArgs = ppArgs;
        clientJob.m_pReturn = pReturn;
        clientJob.m_ppException = ppException;

        if( typelib_TypeClass_INTERFACE_METHOD == pMemberType->eTypeClass )
        {
            clientJob.m_pMethodType = ( typelib_InterfaceMethodTypeDescription * ) pMemberType;
        }

        if( typelib_TypeClass_INTERFACE_ATTRIBUTE == pMemberType->eTypeClass )
        {
            clientJob.m_pAttributeType =
                ( typelib_InterfaceAttributeTypeDescription * ) pMemberType;
        }

        // calculate method index
        sal_Int32 nMethodIndex = 0;
        if( ! pInterfaceType->aBase.bComplete )
        {
            // must be acquired because typedescription may be exchanged
            typelib_typedescription_acquire((typelib_TypeDescription*) pInterfaceType );
            bReleaseForTypeDescriptionNecessary = sal_True;
            typelib_typedescription_complete( (typelib_TypeDescription ** ) &pInterfaceType );
        }
        nMethodIndex = pInterfaceType->pMapMemberIndexToFunctionIndex[
            ((typelib_InterfaceMemberTypeDescription*)pMemberType)->nPosition ];

        if( clientJob.m_pAttributeType && clientJob.m_ppArgs )
        {
            // setter
            nMethodIndex ++;
        }

        // build up the flag byte
        sal_Bool bType = sal_False, bOid = sal_False, bTid = sal_False;
        sal_uInt8 nFlags = 0;
        if( pImpl->m_lastOutType.getTypeLibType() != pInterfaceType->aBase.pWeakRef &&
            ! typelib_typedescriptionreference_equals(
                pImpl->m_lastOutType.getTypeLibType(), pInterfaceType->aBase.pWeakRef ) )
        {
            //new type
            nFlags = nFlags | HDRFLAG_NEWTYPE;
            bType = sal_True;
        }
        if( pImpl->m_lastOutOid.pData != pOid &&
            rtl_ustr_compare_WithLength( pImpl->m_lastOutOid.pData->buffer,
                                         pImpl->m_lastOutOid.pData->length,
                                         pOid->buffer,
                                         pOid->length ) )
        {
            //new object id
            nFlags = nFlags | HDRFLAG_NEWOID;
            bOid = sal_True;
        }
        if( pImpl->m_lastOutTid.getHandle() != aThreadId.getHandle() &&
            !(pImpl->m_lastOutTid == aThreadId) )
        {
            // new threadid
            nFlags = nFlags | HDRFLAG_NEWTID;
            bTid = sal_True;
        }
#ifdef BRIDGES_URP_PROT
        sal_Int32 nLogStart = pImpl->m_blockMarshaler.getPos();
#endif
        if( nFlags )
        {
            // the flag byte is needed + request
            nFlags = nFlags | HDRFLAG_LONG | HDRFLAG_REQUEST; //
            if( nMethodIndex >= 0x100 )
            {
                nFlags = nFlags | HDRFLAG_LONGMETHODID;
            }
            if( clientJob.m_pMethodType && clientJob.m_pMethodType->bOneWay )
            {
                // oneway
                nFlags = nFlags | HDRFLAG_ONEWAY;
            }
            pImpl->m_blockMarshaler.packInt8( &nFlags  );

            if( nFlags & HDRFLAG_LONGMETHODID )
            {
                sal_uInt16 nMethod = (sal_uInt16 ) nMethodIndex;
                pImpl->m_blockMarshaler.packInt16( &nMethod  );
            }
            else
            {
                sal_uInt8 nMethod = (sal_uInt8) nMethodIndex;
                pImpl->m_blockMarshaler.packInt8( &nMethod );
            }
        }
        else
        {
            // no flag byte needed, simply marshal the method index
            if( nMethodIndex >= 64 )
            {
                sal_uInt16 nMethod = ( sal_uInt16 ) nMethodIndex;
                nMethod = nMethod | 0x4000;
                pImpl->m_blockMarshaler.packInt16( &nMethod );
            }
            else
            {
                sal_uInt8 nMethod = (sal_uInt8 ) nMethodIndex;
                pImpl->m_blockMarshaler.packInt8( &nMethod );
            }
        }

        // marshal type,oid,tid
        if( bType )
        {
            pImpl->m_lastOutType = pInterfaceType->aBase.pWeakRef;
            pImpl->m_blockMarshaler.packType( &(pImpl->m_lastOutType) );
        }
        if( bOid )
        {
            pImpl->m_lastOutOid = pOid;
            pImpl->m_blockMarshaler.packOid( pImpl->m_lastOutOid );
        }
        if( bTid )
        {
            pImpl->m_lastOutTid = aThreadId;
            pImpl->m_blockMarshaler.packTid( pImpl->m_lastOutTid );
        }


        // marshal arguments !
#ifdef BRIDGES_URP_PROT
        sal_Int32 nLogHeader = pImpl->m_blockMarshaler.getPos();
#endif
        if( clientJob.m_pMethodType )
        {
            sal_Int32 i;
            for( i = 0 ; i < clientJob.m_pMethodType->nParams ; i ++ )
            {
                if( clientJob.m_pMethodType->pParams[i].bIn )
                {
                      typelib_TypeDescription *pType = 0;
                      TYPELIB_DANGER_GET( &pType , clientJob.m_pMethodType->pParams[i].pTypeRef );
                    pImpl->m_blockMarshaler.pack( ppArgs[i] , pType );
                      TYPELIB_DANGER_RELEASE( pType );
                }
            }
        }
        else if( clientJob.m_pAttributeType && clientJob.m_pReturn )
        {
            // nothing to do !
        }
        else if( clientJob.m_pAttributeType && clientJob.m_ppArgs )
        {
              typelib_TypeDescription *pType = 0;
              TYPELIB_DANGER_GET( &pType , clientJob.m_pAttributeType->pAttributeTypeRef );
            pImpl->m_blockMarshaler.pack( clientJob.m_ppArgs[0] , pType );
              TYPELIB_DANGER_RELEASE( pType );
        }
        else
        {
            OSL_ASSERT( 0 );
        }

#ifdef BRIDGES_URP_PROT
        urp_logCall( pImpl, pImpl->m_blockMarshaler.getPos() - nLogStart,
                     pImpl->m_blockMarshaler.getPos() - nLogHeader, ! bOneway,
                     ((typelib_InterfaceMemberTypeDescription*)pMemberType)->pMemberName );
#endif

        if( ! bOneway )
        {
            pThreadpoolHandle = uno_threadpool_createHandle( (sal_Int64 ) pEnvRemote );
            pImpl->m_clientJobContainer.add( aThreadId , &clientJob );
        }

        //---------------------------
        // Inform the writer thread, that there is some work to do
        //---------------------------
        pImpl->m_pWriter->touch( !bOneway );

        // release the guard
    }

    if(! bOneway)
    {
        //---------------------------
        // Wait for the reply
        //---------------------------
        ClientJob * pData = 0;
        uno_threadpool_enter( pThreadpoolHandle, (void **) &pData );

        if( ! pData )
        {
            OSL_VERIFY( &clientJob ==
                        pImpl->m_clientJobContainer.remove( aThreadId ) );

            prepareRuntimeExceptionClientSide(
                ppException, OUString( RTL_CONSTASCII_USTRINGPARAM( "URP_Bridge : disposed" ) ) );
        }
    }
    else
    {
        //----------------
        // One way call
        //----------------
        *ppException = 0;
    }

    uno_releaseIdFromCurrentThread();

    if( bReleaseForTypeDescriptionNecessary )
        typelib_typedescription_release( (typelib_TypeDescription *) pInterfaceType );
}

}


