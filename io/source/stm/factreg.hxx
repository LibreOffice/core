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
#include <rtl/unload.h>

namespace io_stm {

extern rtl_StandardModuleCount g_moduleCount;

// OPipeImpl
Reference< XInterface > SAL_CALL OPipeImpl_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString    OPipeImpl_getImplementationName();
Sequence<OUString> OPipeImpl_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL ODataInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString    ODataInputStream_getImplementationName();
Sequence<OUString> ODataInputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL ODataOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString ODataOutputStream_getImplementationName();
Sequence<OUString> ODataOutputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL OMarkableOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString OMarkableOutputStream_getImplementationName();
Sequence<OUString> OMarkableOutputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL OMarkableInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString    OMarkableInputStream_getImplementationName() ;
Sequence<OUString> OMarkableInputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL OObjectOutputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw(Exception);
OUString OObjectOutputStream_getImplementationName();
Sequence<OUString> OObjectOutputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL OObjectInputStream_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw(Exception);
OUString    OObjectInputStream_getImplementationName() ;
Sequence<OUString> OObjectInputStream_getSupportedServiceNames(void);

Reference< XInterface > SAL_CALL OPumpImpl_CreateInstance( const Reference< XComponentContext > & rSMgr ) throw (Exception);
OUString OPumpImpl_getImplementationName();
Sequence<OUString> OPumpImpl_getSupportedServiceNames(void);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
