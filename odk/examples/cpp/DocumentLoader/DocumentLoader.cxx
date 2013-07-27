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



/*****************************************************************************
 *****************************************************************************
 *
 * Simple client application using the UnoUrlResolver service.
 *
 *****************************************************************************
 *****************************************************************************/
#include <stdio.h>
#include <wchar.h>

#include <sal/main.h>

#include <cppuhelper/bootstrap.hxx>

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/process.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>

#include <string.h>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::bridge;
using namespace com::sun::star::frame;
using namespace com::sun::star::registry;



//============================================================================
SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    OUString sConnectionString(RTL_CONSTASCII_USTRINGPARAM("uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager"));

    sal_Int32 nCount = (sal_Int32)rtl_getAppCommandArgCount();

    if (nCount < 1)
    {
        printf("using: DocumentLoader -env:URE_MORE_TYPES=<office_types_rdb_url> <file_url> [<uno_connection_url>]\n\n"
               "example: DocumentLoader -env:URE_MORE_TYPES=\"file:///.../OpenOffice%204/program/types.rdb\" \"file:///e:/temp/test.odt\" \"uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager\"\n");
        exit(1);
    }
     if (nCount == 2)
    {
        rtl_getAppCommandArg(1, &sConnectionString.pData);
    }

    Reference< XComponentContext > xComponentContext(::cppu::defaultBootstrap_InitialComponentContext());

    /* Gets the service manager instance to be used (or null). This method has
       been added for convenience, because the service manager is a often used
       object.
    */
    Reference< XMultiComponentFactory > xMultiComponentFactoryClient(
        xComponentContext->getServiceManager() );

    /* Creates an instance of a component which supports the services specified
       by the factory.
    */
    Reference< XInterface > xInterface =
        xMultiComponentFactoryClient->createInstanceWithContext(
            OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ),
            xComponentContext );

    Reference< XUnoUrlResolver > resolver( xInterface, UNO_QUERY );

    // Resolves the component context from the office, on the uno URL given by argv[1].
    try
    {
        xInterface = Reference< XInterface >(
            resolver->resolve( sConnectionString ), UNO_QUERY );
    }
    catch ( Exception& e )
    {
        printf("Error: cannot establish a connection using '%s':\n       %s\n",
               OUStringToOString(sConnectionString, RTL_TEXTENCODING_ASCII_US).getStr(),
               OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        exit(1);
    }

    // gets the server component context as property of the office component factory
    Reference< XPropertySet > xPropSet( xInterface, UNO_QUERY );
    xPropSet->getPropertyValue( OUString::createFromAscii("DefaultContext") ) >>= xComponentContext;

    // gets the service manager from the office
    Reference< XMultiComponentFactory > xMultiComponentFactoryServer(
        xComponentContext->getServiceManager() );

    /* Creates an instance of a component which supports the services specified
       by the factory. Important: using the office component context.
    */
    Reference < XComponentLoader > xComponentLoader(
        xMultiComponentFactoryServer->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop" ) ),
            xComponentContext ), UNO_QUERY );

    /* Loads a component specified by an URL into the specified new or existing
       frame.
    */
    OUString sAbsoluteDocUrl, sWorkingDir, sDocPathUrl, sArgDocUrl;
    rtl_getAppCommandArg(0, &sArgDocUrl.pData);

    osl_getProcessWorkingDir(&sWorkingDir.pData);
    osl::FileBase::getFileURLFromSystemPath( sArgDocUrl, sDocPathUrl);
    osl::FileBase::getAbsoluteFileURL( sWorkingDir, sDocPathUrl, sAbsoluteDocUrl);

    Reference< XComponent > xComponent = xComponentLoader->loadComponentFromURL(
        sAbsoluteDocUrl, OUString( RTL_CONSTASCII_USTRINGPARAM("_blank") ), 0,
        Sequence < ::com::sun::star::beans::PropertyValue >() );

    // dispose the local service manager
    Reference< XComponent >::query( xMultiComponentFactoryClient )->dispose();

    return 0;
}
