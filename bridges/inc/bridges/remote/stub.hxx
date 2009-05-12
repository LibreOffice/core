/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: stub.hxx,v $
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
#include <bridges/remote/remote.hxx>

#include <uno/dispatcher.h>
#include <uno/mapping.hxx>

namespace bridges_remote {

extern "C" typedef void SAL_CALL AcquireUno2RemoteStub( remote_Interface *pThis );
AcquireUno2RemoteStub acquireUno2RemoteStub;

extern "C" typedef void SAL_CALL FreeUno2RemoteStub(
    uno_ExtEnvironment * environment, void * stub);
FreeUno2RemoteStub freeUno2RemoteStub;

class Uno2RemoteStub :
    public remote_Interface
{
public:
    Uno2RemoteStub( uno_Interface *pUnoI,
                    rtl_uString *pOid,
                    typelib_InterfaceTypeDescription *pType,
                    uno_Environment *pEnvUno,
                    uno_Environment *pEnvRemote );
    ~Uno2RemoteStub();

public:
    ::rtl::OUString m_sOid;
    typelib_InterfaceTypeDescription *m_pType;
    uno_Interface *m_pUnoI;
    oslInterlockedCount m_nRef;

    uno_Environment *m_pEnvUno;
    uno_Environment *m_pEnvRemote;
    ::com::sun::star::uno::Mapping m_mapRemote2Uno;
    ::com::sun::star::uno::Mapping m_mapUno2Remote;
};

}
