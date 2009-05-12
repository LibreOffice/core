/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urp_dispatch.hxx,v $
 * $Revision: 1.6 $
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

#include <rtl/ustring.hxx>

#include <typelib/typedescription.h>

#include <uno/any2.h>


typedef struct _uno_Environment uno_Environment;
struct remote_Interface;



namespace bridges_urp {

    const sal_uInt8 HDRFLAG_LONGHEADER       = 0x80;
    const sal_uInt8 HDRFLAG_REQUEST          = 0x40;
    const sal_uInt8 HDRFLAG_NEWTYPE          = 0x20;
    const sal_uInt8 HDRFLAG_NEWOID           = 0x10;
    const sal_uInt8 HDRFLAG_NEWTID           = 0x08;
    const sal_uInt8 HDRFLAG_LONGMETHODID     = 0x04;
    const sal_uInt8 HDRFLAG_IGNORECACHE      = 0x02;
    const sal_uInt8 HDRFLAG_MOREFLAGS        = 0x01;
    const sal_uInt8 HDRFLAG_MUSTREPLY        = 0x80;
    const sal_uInt8 HDRFLAG_SYNCHRONOUS      = 0x40;

    const sal_uInt8 HDRFLAG_EXCEPTION        = 0x20;

    void SAL_CALL urp_sendCloseConnection( uno_Environment *pEnvRemote );

    extern "C" void SAL_CALL urp_sendRequest(
        uno_Environment *pEnvRemote,
        typelib_TypeDescription const * pMemberType,
        rtl_uString *pOid,
        typelib_InterfaceTypeDescription *pInterfaceType,
        void *pReturn,
        void *ppArgs[],
        uno_Any **ppException
        );

    void SAL_CALL urp_sendRequest_internal(
        uno_Environment *pEnvRemote,
        typelib_TypeDescription const * pMemberType,
        rtl_uString *pOid,
        typelib_InterfaceTypeDescription *pInterfaceType,
        void *pReturn,
        void *ppArgs[],
        uno_Any **ppException
        );

}

