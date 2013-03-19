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

#if !defined(OSL_DEBUG_LEVEL) || OSL_DEBUG_LEVEL == 0
# undef OSL_DEBUG_LEVEL
# define OSL_DEBUG_LEVEL 2
#endif


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

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringToOString;

SAL_IMPLEMENT_MAIN()
{
    try
    {
        Reference< XMultiComponentFactory > xMgr( createRegistryServiceFactory(
                                                      OUString("cpputest.rdb") ), UNO_QUERY );
        Reference< XComponentContext > xInitialContext;
        OSL_VERIFY( Reference< beans::XPropertySet >( xMgr, UNO_QUERY )->getPropertyValue(
                        OUString("DefaultContext") ) >>= xInitialContext );

        ContextEntry_Init aEntry;
        aEntry.bLateInitService = false;
        aEntry.name = OUString("bla, bla");
        aEntry.value = makeAny( (sal_Int32)5 );
        Reference< XComponentContext > xContext( createComponentContext( &aEntry, 1, xInitialContext ) );
        OSL_ASSERT( xContext->getServiceManager() != xMgr ); // must be wrapped one
        OSL_ASSERT(
            Reference< beans::XPropertySet >(
                xContext->getServiceManager(), UNO_QUERY )->getPropertyValue(
                    OUString("DefaultContext") ) != xInitialContext );

        Reference< XMultiServiceFactory > x( xMgr, UNO_QUERY );
        test_ImplHelper( x );
        testPropertyTypeHelper();
        testidlclass( x );
         test_PropertySetHelper();
        test_interfacecontainer();

        OSL_VERIFY( xContext->getValueByName(
                        OUString("bla, bla") ) == (sal_Int32)5 );
        OSL_VERIFY( ! xInitialContext->getValueByName(
                        OUString("bla, bla") ).hasValue() );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
