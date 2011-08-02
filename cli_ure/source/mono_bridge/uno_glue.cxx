/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "rtl/ustring.h"
#include "uno/dispatcher.h"
#include "uno/environment.h"
#include <stdio.h>

extern "C" {

/* uno_Interface */

void SAL_CALL cli_uno_interface_acquire( uno_Interface *pInterface )
    SAL_THROW_EXTERN_C()
{
    (*pInterface->acquire)( pInterface );
}

void SAL_CALL cli_uno_interface_release( uno_Interface *pInterface )
    SAL_THROW_EXTERN_C()
{
    (*pInterface->release)( pInterface );
}

void SAL_CALL cli_uno_interface_dispatch(
    uno_Interface *pInterface, const struct _typelib_TypeDescription *pMemberType,
    void *pReturn, void *pArgs[], uno_Any **ppException )
    SAL_THROW_EXTERN_C()
{
    (*pInterface->pDispatcher)( pInterface, pMemberType, pReturn, pArgs, ppException );
}

/* uno_ExtEnvironment */

void SAL_CALL cli_uno_environment_getObjectIdentifier(
    uno_ExtEnvironment *pUnoEnv, rtl_uString **ppOId, uno_Interface *pUnoI )
    SAL_THROW_EXTERN_C()
{
    (*pUnoEnv->getObjectIdentifier)( pUnoEnv, ppOId, pUnoI );
}

void SAL_CALL cli_uno_environment_registerInterface(
    uno_ExtEnvironment *pUnoEnv, void **ppInterface, rtl_uString *pOId,
    struct _typelib_InterfaceTypeDescription *pTypeDescr )
    SAL_THROW_EXTERN_C()
{
    (*pUnoEnv->registerInterface)( pUnoEnv, ppInterface, pOId, pTypeDescr );
}

void SAL_CALL cli_uno_environment_getRegisteredInterface(
    uno_ExtEnvironment *pUnoEnv, void **ppInterface, rtl_uString *pOId,
    struct _typelib_InterfaceTypeDescription *pTypeDescr )
    SAL_THROW_EXTERN_C()
{
    (*pUnoEnv->getRegisteredInterface)( pUnoEnv, ppInterface, pOId, pTypeDescr );
}


}
