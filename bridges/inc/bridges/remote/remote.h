/*************************************************************************
 *
 *  $RCSfile: remote.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:48 $
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
#ifndef _BRIDGES_REMOTE_REMOTE_H
#define _BRIDGES_REMOTE_REMOTE_H
#include <uno/mapping.h>
#include <uno/any2.h>

#include <typelib/typedescription.h>

#define UNO_LB_REMOTE "remote"

#define CORBA_STRING8_NAME "com.sun.star.corba.CorbaString8"
#define CORBA_STRING8_NAME_LENGTH (sizeof(CORBA_STRING8_NAME)-1)

#define CORBA_UNION_NAME   "com.sun.star.corba.CorbaUnion"
#define CORBA_UNION_NAME_LENGTH   (sizeof(CORBA_UNION_NAME)-1)

#define REMOTE_MARSHALED_MSGHDR_SIZE 12
#define REMOTE_RELEASE_METHOD_INDEX 2
#define REMOTE_RELEASE_METHOD_NAME "release"

#define CURRENT_IIOP_PROTOCOL_MAJOR 1
#define CURRENT_IIOP_PROTOCOL_MINOR 2

struct remote_Interface;

typedef void (SAL_CALL * remote_DispatchMethod)( remote_Interface * pRemoteI,
                                                 typelib_TypeDescription * pMemberType,
                                                 void * pReturn,
                                                 void * pArgs[],
                                                 uno_Any ** ppException );


typedef void ( SAL_CALL * requestClientSideDispatcher ) (
    uno_Environment *pEnvRemote,
    typelib_TypeDescription * pMemberType,
    rtl_uString *pOid,
    typelib_InterfaceTypeDescription *pInterfaceType,
    void *pReturn,
    void *ppArgs[],
    uno_Any **ppException );

struct remote_Interface
{
    void (SAL_CALL * acquire)( remote_Interface * pInterface );

    void (SAL_CALL * release)( remote_Interface * pInterface );

    remote_DispatchMethod pDispatcher;
};


struct remote_Mapping
{
    uno_Mapping aBase;
    uno_Environment *pEnvRemote;
    uno_Environment *pEnvUno;
};

#endif
