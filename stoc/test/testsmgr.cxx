/*************************************************************************
 *
 *  $RCSfile: testsmgr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-19 11:08:05 $
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
#include <stdio.h>

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

#include "testsmgr_cpnt/testsmgr_cpnt.hxx"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifdef _DEBUG
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



void SAL_CALL main()
{
    printf( "ServiceManagerTest : \r");
    setStarUserRegistry();
    fflush( stdout );
    test_ServiceManager();

    printf( "ServiceManagerTest : OK\n" );
}
