/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: remote.h,v $
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
#ifndef _BRIDGES_REMOTE_REMOTE_H
#define _BRIDGES_REMOTE_REMOTE_H
#include <uno/mapping.h>
#include <uno/any2.h>

#include <typelib/typedescription.h>
#include <uno/environment.h>

#define CORBA_STRING8_NAME "com.sun.star.corba.CorbaString8"
#define CORBA_STRING8_NAME_LENGTH (sizeof(CORBA_STRING8_NAME)-1)

#define CORBA_UNION_NAME   "com.sun.star.corba.CorbaUnion"
#define CORBA_UNION_NAME_LENGTH   (sizeof(CORBA_UNION_NAME)-1)

#define REMOTE_MARSHALED_MSGHDR_SIZE 12
#define REMOTE_RELEASE_METHOD_INDEX 2
#define REMOTE_RELEASE_METHOD_NAME "release"

#define CURRENT_IIOP_PROTOCOL_MAJOR 1
#define CURRENT_IIOP_PROTOCOL_MINOR 2

extern "C" {

struct remote_Interface;

/** @internal
 */
typedef void (SAL_CALL * remote_DispatchMethod)(
    remote_Interface * pRemoteI, typelib_TypeDescription const * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException );

/**
   @internal
 */
typedef void ( SAL_CALL * requestClientSideDispatcher ) (
    uno_Environment *pEnvRemote,
    typelib_TypeDescription const * pMemberType,
    rtl_uString *pOid,
    typelib_InterfaceTypeDescription *pInterfaceType,
    void *pReturn,
    void *ppArgs[],
    uno_Any **ppException );

/** The base class of an UNO interface in a remote environment.
 */
struct remote_Interface
{
    void (SAL_CALL * acquire)( remote_Interface * pInterface );

    void (SAL_CALL * release)( remote_Interface * pInterface );

    remote_DispatchMethod pDispatcher;
};

/** The mapping between an binary-c-uno and a remote environment.
    @internal
 */
struct remote_Mapping
{
    uno_Mapping aBase;
    uno_Environment *pEnvRemote;
    uno_Environment *pEnvUno;
};

}

#endif
