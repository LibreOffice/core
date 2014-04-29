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



#if !defined(OSL_DEBUG_LEVEL) || OSL_DEBUG_LEVEL == 0
# undef OSL_DEBUG_LEVEL
# define OSL_DEBUG_LEVEL 2
#endif

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"

#include <sal/main.h>

#include <stdio.h>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include <cppuhelper/component_context.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include "testhelper.hxx"

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

SAL_IMPLEMENT_MAIN()
{
    try
    {
        Reference< XMultiComponentFactory > xMgr( createRegistryServiceFactory(
                                                      OUString( RTL_CONSTASCII_USTRINGPARAM("cpputest.rdb") ) ), UNO_QUERY );
        Reference< XComponentContext > xInitialContext;
        OSL_VERIFY( Reference< beans::XPropertySet >( xMgr, UNO_QUERY )->getPropertyValue(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xInitialContext );

        ContextEntry_Init aEntry;
        aEntry.bLateInitService = false;
        aEntry.name = OUString( RTL_CONSTASCII_USTRINGPARAM("bla, bla") );
        aEntry.value = makeAny( (sal_Int32)5 );
        Reference< XComponentContext > xContext( createComponentContext( &aEntry, 1, xInitialContext ) );
        OSL_ASSERT( xContext->getServiceManager() != xMgr ); // must be wrapped one
        OSL_ASSERT(
            Reference< beans::XPropertySet >(
                xContext->getServiceManager(), UNO_QUERY )->getPropertyValue(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) != xInitialContext );

        Reference< XMultiServiceFactory > x( xMgr, UNO_QUERY );
        test_ImplHelper( x );
        testPropertyTypeHelper();
        testidlclass( x );
         test_PropertySetHelper();
        test_interfacecontainer();

        OSL_VERIFY( xContext->getValueByName(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("bla, bla") ) ) == (sal_Int32)5 );
        OSL_VERIFY( ! xInitialContext->getValueByName(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("bla, bla") ) ).hasValue() );
        Reference< XComponent >( xInitialContext, UNO_QUERY )->dispose();
        xMgr.clear();
        xContext.clear();
        xInitialContext.clear();
    }
    catch (Exception & exc)
    {
        OString cstr_msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( ! "exception occurred: ", cstr_msg.getStr() );
    }

    printf( "Test finished\n" );
    return 0;
}
