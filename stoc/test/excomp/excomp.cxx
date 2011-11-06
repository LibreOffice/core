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
#include <stdlib.h>
#include <string.h>

#include <sal/main.h>
#include <osl/diagnose.h>
#include <osl/process.h>

#include <example/XTest.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>


using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace example;
using namespace cppu;
using namespace rtl;

#if OSL_DEBUG_LEVEL > 0
#define TEST_ENSHURE(c, m)   OSL_ENSURE(c, m)
#else
#define TEST_ENSHURE(c, m)   OSL_VERIFY(c)
#endif

OUString getExePath()
{
    OUString        exe;

    OSL_VERIFY(osl_getExecutableFile( &exe.pData) == osl_Process_E_None);

#if defined(WIN32) || defined(__OS2__) || defined(WNT)
    exe = exe.copy(0, exe.getLength() - 10);
#else
    exe = exe.copy(0, exe.getLength() - 6);
#endif
    return exe;
}

SAL_IMPLEMENT_MAIN()
{
#ifdef UNX
    OUString compName1(RTL_CONSTASCII_USTRINGPARAM("libexcomp1.so"));
    OUString compName2(RTL_CONSTASCII_USTRINGPARAM("libexcomp2.so"));
#else
    OUString compName1(RTL_CONSTASCII_USTRINGPARAM("excomp1"));
    OUString compName2(RTL_CONSTASCII_USTRINGPARAM("excomp2"));
#endif

    OUString exePath( getExePath() );
    OUString excompRdb(exePath);

    excompRdb += OUString::createFromAscii("excomp.rdb");

    Reference< XMultiServiceFactory > xSMgr  = ::cppu::createRegistryServiceFactory( excompRdb );
    TEST_ENSHURE( xSMgr.is(), "excomp error 0" );

    typelib_TypeDescription* pTypeDesc = NULL;
    OUString sType = OUString::createFromAscii("com.sun.star.text.XTextDocument");
    typelib_typedescription_getByName( &pTypeDesc, sType.pData);
//  typelib_InterfaceTypeDescription* pInterDesc = (typelib_InterfaceTypeDescription*)pTypeDesc;

    Reference< XInterface > xIFace = xSMgr->createInstance(OUString::createFromAscii("com.sun.star.registry.ImplementationRegistration"));
    Reference< XImplementationRegistration > xImpReg( xIFace, UNO_QUERY);
    TEST_ENSHURE( xImpReg.is(), "excomp error 1" );
    try
    {
        xImpReg->registerImplementation(OUString::createFromAscii("com.sun.star.loader.SharedLibrary"),
                                        compName1,
                                        Reference< XSimpleRegistry >() );
        xImpReg->registerImplementation(OUString::createFromAscii("com.sun.star.loader.SharedLibrary"),
                                        compName2,
                                        Reference< XSimpleRegistry >() );
    }
    catch( CannotRegisterImplementationException& e)
    {
        TEST_ENSHURE( e.Message.getLength(), OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
    }

    Reference< XTest > xTest1( xSMgr->createInstance(OUString::createFromAscii("example.ExampleComponent1")),
                               UNO_QUERY);
    TEST_ENSHURE( xTest1.is(), "excomp error 2" );
    Reference< XTest > xTest2( xSMgr->createInstance(OUString::createFromAscii("example.ExampleComponent2")),
                               UNO_QUERY);
    TEST_ENSHURE( xTest2.is(), "excomp error 3" );

    OUString m1 = xTest1->getMessage();
    OUString m2 = xTest2->getMessage();

    fprintf(stdout, "ExampleComponent1, Message = \"%s\"\n", OUStringToOString(m1, RTL_TEXTENCODING_ASCII_US).getStr());
    fprintf(stdout, "ExampleComponent2, Message = \"%s\"\n", OUStringToOString(m2, RTL_TEXTENCODING_ASCII_US).getStr());

    xImpReg->revokeImplementation(compName1, Reference< XSimpleRegistry >() );
    xImpReg->revokeImplementation(compName2, Reference< XSimpleRegistry >() );

    Reference< XComponent >( xSMgr, UNO_QUERY )->dispose();

    return(0);
}


