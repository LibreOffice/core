/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: remote.hxx,v $
 * $Revision: 1.7 $
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

extern "C" typedef void SAL_CALL AcquireRemote2RemoteStub(
    remote_Interface * pThis);
AcquireRemote2RemoteStub acquireRemote2RemoteStub;

extern "C" typedef void SAL_CALL FreeRemote2RemoteStub(
    uno_ExtEnvironment * environment, void * stub);
FreeRemote2RemoteStub freeRemote2RemoteStub;

class Remote2RemoteStub :
    public remote_Interface
{
public:
    Remote2RemoteStub(rtl_uString *pOid,
                      typelib_InterfaceTypeDescription *pType,
                      uno_Environment *pEnvRemote,
                      requestClientSideDispatcher dispatch );
    ~Remote2RemoteStub();

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
