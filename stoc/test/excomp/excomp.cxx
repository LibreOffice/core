/*************************************************************************
 *
 *  $RCSfile: excomp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 17:19:50 $
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
#include <stdlib.h>
#include <string.h>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#include <example/XTest.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif


using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace example;
using namespace cppu;
using namespace vos;
using namespace rtl;

#ifdef _DEBUG
#define TEST_ENSHURE(c, m)   OSL_ENSURE(c, m)
#else
#define TEST_ENSHURE(c, m)   OSL_VERIFY(c)
#endif

OUString getExePath()
{
    OStartupInfo    startupInfo;
    OUString        exe;

    OSL_VERIFY(startupInfo.getExecutableFile(exe) == OStartupInfo::E_None);

#if defined(WIN32) || defined(__OS2__) || defined(WNT)
    exe = exe.copy(0, exe.getLength() - 10);
#else
    exe = exe.copy(0, exe.getLength() - 6);
#endif
    return exe;
}

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
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
    typelib_InterfaceTypeDescription* pInterDesc = (typelib_InterfaceTypeDescription*)pTypeDesc;

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


