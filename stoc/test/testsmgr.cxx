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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"
#include <stdio.h>

#include <sal/main.h>
#include <osl/process.h>
#include <registry/registry.hxx>
#include <uno/mapping.hxx>

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

extern "C" void SAL_CALL test_ServiceManager();

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#if OSL_DEBUG_LEVEL > 0
#define TEST_ENSHURE(c, m)   OSL_ENSURE(c, m)
#else
#define TEST_ENSHURE(c, m)   OSL_VERIFY(c)
#endif


using namespace ::rtl;
OString userRegEnv("STAR_USER_REGISTRY=");

OUString getExePath()
{
    OUString        exe;

    OSL_VERIFY( osl_getExecutableFile( &exe.pData) == osl_Process_E_None);

#if defined(WIN32) || defined(__OS2__) || defined(WNT)
    exe = exe.copy(0, exe.getLength() - 16);
#else
    exe = exe.copy(0, exe.getLength() - 12);
#endif
    return exe;
}

void setStarUserRegistry()
{
    Registry *myRegistry = new Registry();

    RegistryKey rootKey, rKey, rKey2;

    OUString userReg = getExePath();
    userReg += OUString::createFromAscii("user.rdb");
    if(myRegistry->open(userReg, REG_READWRITE))
    {
        TEST_ENSHURE(!myRegistry->create(userReg), "setStarUserRegistry error 1");
    }

    TEST_ENSHURE(!myRegistry->close(), "setStarUserRegistry error 9");
    delete myRegistry;

    userRegEnv += OUStringToOString(userReg, RTL_TEXTENCODING_ASCII_US);
    putenv((char *)userRegEnv.getStr());
}


SAL_IMPLEMENT_MAIN()
{
    printf( "ServiceManagerTest : \r");
    setStarUserRegistry();
    fflush( stdout );
    test_ServiceManager();

    printf( "ServiceManagerTest : OK\n" );
    return 0;
}
