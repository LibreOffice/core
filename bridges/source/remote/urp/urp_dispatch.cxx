/*************************************************************************
 *
 *  $RCSfile: urp_dispatch.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-16 08:47:31 $
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
#elif MACOSX
#include <sys/types.h>
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif

#include <osl/mutex.hxx>
#include <osl/diagnose.h>

#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>

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
        // no more data via this connection !
        pImpl->m_pWriter->abort();
    }
}
void SAL_CALL urp_sendRequest(
    uno_Environment *pEnvRemote,
    typelib_TypeDescription * pMemberType,
    rtl_uString *pOid,
    typelib_InterfaceTypeDescription *pInterfaceType,
    void *pReturn,
    void *ppArgs[],
    uno_Any **ppException )
{
    remote_Context *pContext = (remote_Context *) pEnvRemote->pContext;
    urp_BridgeImpl *pImpl = (urp_BridgeImpl*) ( pContext->m_pBridgeImpl );

    ClientJob job(pEnvRemote, pImpl, pOid, pMemberType, pInterfaceType, pReturn, ppArgs, ppException);

    if( pImpl->m_properties.bForceSynchronous &&
        REMOTE_RELEASE_METHOD_INDEX ==
        ((typelib_InterfaceMemberTypeDescription*)pMemberType)->nPosition &&
        pImpl->m_pWriter->getIdentifier() != ::osl::Thread::getCurrentIdentifier() )
    {
        // all release calls in the FORCE SYNCHRONOUS case are delegated to the writer thread to avoid
        // multiple synchron calls with the same thread id.
        pImpl->m_pWriter->insertReleaseRemoteCall( pOid, pInterfaceType->aBase.pWeakRef );
    }
    else
    {
        if( job.pack() && ! job.isOneway() )
        {
            job.wait();
        }
    }
}

}


