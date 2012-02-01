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
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;


SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    try
    {
        // get the remote office component context
        Reference< XComponentContext > xContext( ::cppu::bootstrap() );
        if ( !xContext.is() )
        {
            fprintf(stderr, "no component context!\n");
            return 1;
        }

        // get the remote office service manager
        Reference< XMultiComponentFactory > xServiceManager(
            xContext->getServiceManager() );
        if ( !xServiceManager.is() )
        {
            fprintf(stderr, "no service manager!\n");
            return 1;
        }

        // get an instance of the remote office desktop UNO service
        // and query the XComponentLoader interface
        Reference < XComponentLoader > xComponentLoader(
            xServiceManager->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ),
            xContext ), UNO_QUERY_THROW );

        // open a spreadsheet document
        Reference< XComponent > xComponent( xComponentLoader->loadComponentFromURL(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "private:factory/scalc" ) ),
            OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" ) ), 0,
            Sequence < ::com::sun::star::beans::PropertyValue >() ) );
        if ( !xComponent.is() )
        {
            fprintf(stderr, "opening spreadsheet document failed!\n");
            return 1;
        }
    }
    catch ( ::cppu::BootstrapException & e )
    {
        fprintf(stderr, "caught BootstrapException: %s\n",
                OUStringToOString( e.getMessage(), RTL_TEXTENCODING_ASCII_US ).getStr());
        return 1;
    }
    catch ( Exception & e )
    {
        fprintf(stderr, "caught UNO exception: %s\n",
                OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr());
        return 1;
    }

    return 0;
}
