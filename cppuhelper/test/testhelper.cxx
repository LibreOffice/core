/*************************************************************************
 *
 *  $RCSfile: testhelper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:26:10 $
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
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include "testhelper.hxx"

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int __cdecl main( int argc, char * argv[] )
#endif
{
    Reference< XMultiServiceFactory > xMgr( createRegistryServiceFactory(
        OUString( RTL_CONSTASCII_USTRINGPARAM("cpputest.rdb") ) ) );

    sal_Bool bSucc = sal_False;
    try
    {
        Reference< XImplementationRegistration > xImplReg(
            xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.ImplementationRegistration") ) ),
            UNO_QUERY );
        OSL_ENSHURE( xImplReg.is(), "### no impl reg!" );

#ifdef UNX
#define REG_PREFIX      "lib"
#define DLL_POSTFIX     ".so"
#else
#define REG_PREFIX      ""
#define DLL_POSTFIX     ".dll"
#endif
        OString aLibName( REG_PREFIX );
        aLibName += "corefl";
#ifndef OS2
        aLibName += DLL_POSTFIX;
#endif
        xImplReg->registerImplementation(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary") ),
            OUString::createFromAscii( aLibName.getStr() ),
            Reference< XSimpleRegistry >() );

        testPropertyTypeHelper();
        testidlclass( xMgr );
        test_PropertySetHelper();
        test_ImplHelper( xMgr );
    }
    catch (Exception & rExc)
    {
        OSL_ENSHURE( sal_False, "### exception occured!" );
        OString aMsg( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### exception occured: " );
        OSL_TRACE( aMsg.getStr() );
        OSL_TRACE( "\n" );
    }

    Reference< XComponent >( xMgr, UNO_QUERY )->dispose();

    printf( "Test finished\n" );
    return 0;
}
