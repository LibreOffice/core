/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: proxy.hxx,v $
 * $Revision: 1.4 $
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
#include <osl/interlck.h>

#include <uno/environment.h>
#include <uno/dispatcher.h>
#include <uno/mapping.hxx>

#include <bridges/remote/remote.h>

namespace bridges_remote {

extern "C" typedef void SAL_CALL FreeRemote2UnoProxy(
    uno_ExtEnvironment * environment, void * proxy);
FreeRemote2UnoProxy freeRemote2UnoProxy;

// private:
extern "C" typedef void SAL_CALL AcquireRemote2UnoProxy(uno_Interface *);
AcquireRemote2UnoProxy acquireRemote2UnoProxy;

// private:
extern "C" typedef void SAL_CALL ReleaseRemote2UnoProxy(uno_Interface *);
ReleaseRemote2UnoProxy releaseRemote2UnoProxy;

// private:
extern "C" typedef void SAL_CALL DispatchRemote2UnoProxy(
    uno_Interface *, typelib_TypeDescription const *, void *, void **,
    uno_Any **);
DispatchRemote2UnoProxy dispatchRemote2UnoProxy;

extern "C" void SAL_CALL remote_release( void * );

class Remote2UnoProxy :
    public uno_Interface
{
public:
    Remote2UnoProxy(
        remote_Interface *pRemoteI,
        rtl_uString *pOid,
        typelib_InterfaceTypeDescription *pType ,
        uno_Environment *pEnvUno,
        uno_Environment *pEnvRemote
        );

    ~Remote2UnoProxy();

private:
    ::rtl::OUString m_sOid;
    typelib_InterfaceTypeDescription *m_pType;
    remote_Interface *m_pRemoteI;
    uno_Environment *m_pEnvUno;
    uno_Environment *m_pEnvRemote;
    ::com::sun::star::uno::Mapping m_mapRemote2Uno;
    ::com::sun::star::uno::Mapping m_mapUno2Remote;

    oslInterlockedCount m_nRef;

    friend void SAL_CALL acquireRemote2UnoProxy(uno_Interface *);

    friend void SAL_CALL releaseRemote2UnoProxy(uno_Interface *);

    friend void SAL_CALL dispatchRemote2UnoProxy(
        uno_Interface *, typelib_TypeDescription const *, void *, void **,
        uno_Any **);
};

}
