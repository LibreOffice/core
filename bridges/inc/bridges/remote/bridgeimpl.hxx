/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bridgeimpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:38:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
