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

using ::rtl::OUString;
using ::rtl::OUStringToOString;

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

    excompRdb += OUString(RTL_CONSTASCII_USTRINGPARAM("excomp.rdb"));

    Reference< XMultiServiceFactory > xSMgr  = ::cppu::createRegistryServiceFactory( excompRdb );
    TEST_ENSHURE( xSMgr.is(), "excomp error 0" );

    typelib_TypeDescription* pTypeDesc = NULL;
    OUString sType(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.XTextDocument"));
    typelib_typedescription_getByName( &pTypeDesc, sType.pData);
//  typelib_InterfaceTypeDescription* pInterDesc = (typelib_InterfaceTypeDescription*)pTypeDesc;

    Reference< XInterface > xIFace = xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.ImplementationRegistration")));
    Reference< XImplementationRegistration > xImpReg( xIFace, UNO_QUERY);
    TEST_ENSHURE( xImpReg.is(), "excomp error 1" );
    try
    {
        xImpReg->registerImplementation(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")),
                                        compName1,
                                        Reference< XSimpleRegistry >() );
        xImpReg->registerImplementation(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")),
                                        compName2,
                                        Reference< XSimpleRegistry >() );
    }
    catch( CannotRegisterImplementationException& e)
    {
        TEST_ENSHURE( e.Message.getLength(), OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
    }

    Reference< XTest > xTest1( xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("example.ExampleComponent1"))),
                               UNO_QUERY);
    TEST_ENSHURE( xTest1.is(), "excomp error 2" );
    Reference< XTest > xTest2( xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("example.ExampleComponent2"))),
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
