/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bridgeimpl.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _BRIDGES_REMOTE_BRIDGEIMPL_HXX_
#define _BRIDGES_REMOTE_BRIDGEIMPL_HXX_
#include <osl/interlck.h>

#include <uno/environment.h>

#include <bridges/remote/context.h>
#include <bridges/remote/remote.h>


struct remote_BridgeImpl
{
    void (SAL_CALL * m_allThreadsAreGone ) ( uno_Environment * );
    requestClientSideDispatcher m_sendRequest;
    oslInterlockedCount m_nRemoteThreads;
    sal_Bool  m_bDisposed;
    sal_Bool  m_bReleaseStubsCalled;
};

namespace bridges_remote {

    enum RemoteThreadCounter_HoldEnvWeak
    {
        RTC_HOLDENVWEAK = 0x1
    };

    class RemoteThreadCounter
    {
    public:
        // performance optimization. In some cases, it is not necessary to acquire the
        // environment.
        RemoteThreadCounter( uno_Environment *pEnvRemote, RemoteThreadCounter_HoldEnvWeak )
            : m_bReleaseEnvironment( sal_False )
            , m_pEnvRemote( pEnvRemote )
        {
              remote_Context *pContext = ((remote_Context *) m_pEnvRemote->pContext );
            osl_incrementInterlockedCount( &( pContext->m_pBridgeImpl->m_nRemoteThreads ) );
        }

        RemoteThreadCounter( uno_Environment *pEnvRemote )
            : m_bReleaseEnvironment( sal_True )
            , m_pEnvRemote( pEnvRemote )
        {
            m_pEnvRemote->acquire( m_pEnvRemote );

              remote_Context *pContext = ((remote_Context *) m_pEnvRemote->pContext );
            osl_incrementInterlockedCount( &( pContext->m_pBridgeImpl->m_nRemoteThreads ) );
        }

    ~RemoteThreadCounter( )
        {
              remote_Context *pContext = ((remote_Context *) m_pEnvRemote->pContext );
            if( 0 == osl_decrementInterlockedCount( &( pContext->m_pBridgeImpl->m_nRemoteThreads)) &&
                pContext->m_pBridgeImpl->m_bDisposed &&
                ! pContext->m_pBridgeImpl->m_bReleaseStubsCalled )
            {
                pContext->m_pBridgeImpl->m_allThreadsAreGone( m_pEnvRemote );
            }
            if( m_bReleaseEnvironment )
                m_pEnvRemote->release( m_pEnvRemote );
        }

        sal_Bool m_bReleaseEnvironment;
        uno_Environment *m_pEnvRemote;
    };
}

#endif
