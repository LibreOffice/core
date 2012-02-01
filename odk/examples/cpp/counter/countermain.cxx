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



/*************************************************************************
 *************************************************************************
 *
 * simple client application registering and using the counter component.
 *
 *************************************************************************
 *************************************************************************/

#include <stdio.h>

#include <sal/main.h>
#include <rtl/ustring.hxx>

#include <osl/diagnose.h>

#include <cppuhelper/bootstrap.hxx>

// generated c++ interfaces
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <foo/XCountable.hpp>


using namespace foo;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

using namespace ::rtl;


//=======================================================================
SAL_IMPLEMENT_MAIN()
{
    try {

        Reference< XComponentContext > xContext(::cppu::defaultBootstrap_InitialComponentContext());
        OSL_ENSURE( xContext.is(), "### bootstrap failed!\n" );

        Reference< XMultiComponentFactory > xMgr = xContext->getServiceManager();
        OSL_ENSURE( xMgr.is(), "### cannot get initial service manager!" );

        Reference< XInterface > xx = xMgr->createInstanceWithContext(
            OUString::createFromAscii("foo.Counter"), xContext);

        OSL_ENSURE( xx.is(), "### cannot get service instance of \"foo.Counter\"!" );

        Reference< XCountable > xCount( xx, UNO_QUERY );
        OSL_ENSURE( xCount.is(), "### cannot query XCountable interface of service instance \"foo.Counter\"!" );

        if (xCount.is())
        {
             xCount->setCount( 42 );
             fprintf( stdout , "%d," , (int)xCount->getCount() );
             fprintf( stdout , "%d," , (int)xCount->increment() );
             fprintf( stdout , "%d\n" , (int)xCount->decrement() );
        }

        Reference< XComponent >::query( xContext )->dispose();

    } catch( Exception& e) {
        printf("Error: caught exception:\n       %s\n",
               OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        exit(1);
    }

    return 0;
}
