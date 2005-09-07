/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: remote.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:13:28 $
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
#ifndef _BRIDGES_REMOTE_REMOTE_HXX_
#define _BRIDGES_REMOTE_REMOTE_HXX_
#include <list>

#include <osl/mutex.hxx>
#include <osl/conditn.h>
#include <osl/interlck.h>

#include <uno/environment.h>

#include <bridges/remote/remote.h>
#include <bridges/remote/connection.h>

#include <com/sun/star/uno/Sequence.hxx>

namespace bridges_remote {

class Remote2RemoteStub :
    public remote_Interface
{
public:
    Remote2RemoteStub(rtl_uString *pOid,
                      typelib_InterfaceTypeDescription *pType,
                      uno_Environment *pEnvRemote,
                      requestClientSideDispatcher dispatch );
    ~Remote2RemoteStub();

    static void SAL_CALL thisAcquire( remote_Interface *pThis );
    static void SAL_CALL thisRelease( remote_Interface *pThis );
    static void SAL_CALL thisDispatch( remote_Interface * pUnoI,
                                       typelib_TypeDescription * pMemberType,
                                       void * pReturn,
                                       void * pArgs[],
                                       uno_Any ** ppException );
    static void SAL_CALL thisFree( uno_ExtEnvironment *pEnvRemote , void * );

    void releaseRemote();
public:
    ::rtl::OUString                   m_sOid;
    typelib_InterfaceTypeDescription  *m_pType;
    oslInterlockedCount               m_nRef;
    uno_Environment                   *m_pEnvRemote;
    requestClientSideDispatcher       m_dispatch;
    oslInterlockedCount m_nReleaseRemote;
};

}
#endif
