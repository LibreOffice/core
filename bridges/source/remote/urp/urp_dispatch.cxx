/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urp_dispatch.cxx,v $
 * $Revision: 1.17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"
#include <sal/alloca.h>
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

        // send immediately
        if( ! pImpl->m_blockMarshaler.empty() )
        {
            pImpl->m_pWriter->touch( sal_True );
        }

        pImpl->m_pWriter->sendEmptyMessage();
        // no more data via this connection !
        pImpl->m_pWriter->abort();
    }
}
extern "C" void SAL_CALL urp_sendRequest(
    uno_Environment *pEnvRemote,
    typelib_TypeDescription const * pMemberType,
    rtl_uString *pOid,
    typelib_InterfaceTypeDescription *pInterfaceType,
    void *pReturn,
    void *ppArgs[],
    uno_Any **ppException )
{
    remote_Context *pContext = (remote_Context *) pEnvRemote->pContext;
    urp_BridgeImpl *pImpl = (urp_BridgeImpl*) ( pContext->m_pBridgeImpl );
    pImpl->m_initialized.wait();
    urp_sendRequest_internal(
        pEnvRemote, pMemberType, pOid, pInterfaceType, pReturn, ppArgs,
        ppException );
}
void SAL_CALL urp_sendRequest_internal(
    uno_Environment *pEnvRemote,
    typelib_TypeDescription const * pMemberType,
    rtl_uString *pOid,
    typelib_InterfaceTypeDescription *pInterfaceType,
    void *pReturn,
    void *ppArgs[],
    uno_Any **ppException )
{
    remote_Context *pContext = (remote_Context *) pEnvRemote->pContext;
    urp_BridgeImpl *pImpl = (urp_BridgeImpl*) ( pContext->m_pBridgeImpl );

    ClientJob job(
        pEnvRemote, pContext, pImpl, pOid, pMemberType, pInterfaceType, pReturn,
        ppArgs, ppException);

    if( job.pack() && ! job.isOneway() )
    {
        job.wait();
    }
}

}


