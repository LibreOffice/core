/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: helper.hxx,v $
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
#include <bridges/remote/bridgeimpl.hxx>

typedef void ( SAL_CALL * ReleaseRemoteCallbackFunc ) (
    remote_Interface *ppRemoteI,
    rtl_uString *pOid,
    typelib_TypeDescriptionReference *pTypeRef,
    uno_Environment *pEnvRemote
    );

typedef void ( SAL_CALL * remote_createStubFunc ) (
    remote_Interface **ppRemoteI,
    rtl_uString *pOid ,
    typelib_TypeDescriptionReference *pTypeRef,
    uno_Environment *pEnvRemote,
    ReleaseRemoteCallbackFunc callback
    );
namespace bridges_remote
{

    /** @param callback If the bridge implementation wants to handle the remote release call,
                        it can do it giving this callback. If callback == 0, the releaseRemote
                        method of the stub is called.
    */
    void SAL_CALL remote_createStub (
        remote_Interface **ppRemoteI,
        rtl_uString *pOid ,
        typelib_TypeDescriptionReference *pType,
        uno_Environment *pEnvRemote,
        ReleaseRemoteCallbackFunc callback );

    void SAL_CALL remote_retrieveOidFromProxy(
        remote_Interface *pRemtoeI,
        rtl_uString **ppOid );

    void SAL_CALL remote_sendQueryInterface(
        uno_Environment *pEnvRemote,
        remote_Interface **ppRemoteI,
        rtl_uString *pOid ,
        typelib_TypeDescriptionReference *pType,
        uno_Any **ppException
        );
}
