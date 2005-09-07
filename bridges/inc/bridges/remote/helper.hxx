/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:12:09 $
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
