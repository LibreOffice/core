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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
