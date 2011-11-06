/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
