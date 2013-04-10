/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

namespace io_stm {

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
