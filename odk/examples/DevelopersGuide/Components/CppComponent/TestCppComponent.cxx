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



#include <stdio.h>
#include <sal/main.h>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <my_module/MyService1.hpp>
#include <my_module/MyService2.hpp>

using namespace rtl;
using namespace com::sun::star::uno;
//namespace cssuno = ::com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;

SAL_IMPLEMENT_MAIN()
{
    try
    {
        // get the remote office component context
        Reference< XComponentContext > xContext( ::cppu::bootstrap() );
        fprintf(stdout, "\nconnected to a running office...\n");

        // create a new instance of MyService1
        Reference<my_module::XSomething> xSomething =
            my_module::MyService1::create(xContext);

        // call methodOne and print the return value on stdout
        OUString s = xSomething->methodOne(OUString(RTL_CONSTASCII_USTRINGPARAM("Hello World!")));
        fprintf(stdout,"\nCreate new instance of MyService1\nCall of XSomething.methOne at MyService1 = %s", OUStringToOString( s, RTL_TEXTENCODING_ASCII_US ).getStr());

        // create a new instance of MyService2 with the specified string argument
        xSomething = my_module::MyService2::create(xContext, OUString(RTL_CONSTASCII_USTRINGPARAM("Hello My World!")));

        // call methodTwo and print the return value of methodTwo
        s = xSomething->methodTwo();
        fprintf(stdout, "\n\nCreate new instance of MyService2 with argument\nCall of XSomething.methTwo at MyService2 = %s", OUStringToOString( s, RTL_TEXTENCODING_ASCII_US ).getStr());

        fprintf(stdout, "\n\nPlease press 'return' to finish the example!\n");
        getchar();
    }
    catch ( ::cppu::BootstrapException & e )
    {
        fprintf(stderr, "\ncaught BootstrapException: %s\n",
                OUStringToOString( e.getMessage(), RTL_TEXTENCODING_ASCII_US ).getStr());
        return 1;
    }
    catch ( Exception & e )
    {
        fprintf(stderr, "\ncaught UNO exception: %s\n",
                OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
        return 1;
    }

    return 0;
}
