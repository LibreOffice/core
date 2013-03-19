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


using namespace ::rtl;
OString userRegEnv("STAR_USER_REGISTRY=");

OUString getExePath()
{
    OUString        exe;

    OSL_VERIFY( osl_getExecutableFile( &exe.pData) == osl_Process_E_None);

#if defined(WIN32) || defined(WNT)
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
    userReg += "user.rdb";
    if(myRegistry->open(userReg, REG_READWRITE))
    {
        OSL_VERIFY(!myRegistry->create(userReg));
    }

    OSL_VERIFY(!myRegistry->close());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
