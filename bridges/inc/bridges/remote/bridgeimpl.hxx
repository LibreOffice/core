/*************************************************************************
 *
 *  $RCSfile: bridgeimpl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:47 $
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
        RemoteThreadCounter( uno_Environment *pEnvRemote, RemoteThreadCounter_HoldEnvWeak value )
            : m_pEnvRemote( pEnvRemote )
            , m_bReleaseEnvironment( sal_False )
        {
              remote_Context *pContext = ((remote_Context *) m_pEnvRemote->pContext );
            osl_incrementInterlockedCount( &( pContext->m_pBridgeImpl->m_nRemoteThreads ) );
        }

        RemoteThreadCounter( uno_Environment *pEnvRemote )
            : m_pEnvRemote( pEnvRemote )
            , m_bReleaseEnvironment( sal_True )
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
