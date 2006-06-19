/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: remote.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:39:21 $
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
#ifndef _BRIDGES_REMOTE_REMOTE_H
#define _BRIDGES_REMOTE_REMOTE_H
#include <uno/mapping.h>
#include <uno/any2.h>

#include <typelib/typedescription.h>

#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif

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
