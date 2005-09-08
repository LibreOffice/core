/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testsmgr.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:24:35 $
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
#include <stdio.h>

#include <sal/main.h>
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#ifndef _REGISTRY_REGISTRY_HXX_
#include <registry/registry.hxx>
#endif

#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

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
    RegistryLoader* pLoader = new RegistryLoader();

    if (!pLoader->isLoaded())
    {
        delete pLoader;
        return;
    }

    Registry *myRegistry = new Registry(*pLoader);
    delete pLoader;

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
