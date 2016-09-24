/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include <stdio.h>

#include <sal/main.h>

#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge;
using namespace cppu;

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringToOString;

SAL_IMPLEMENT_MAIN()
{
    // create the initial component context
    Reference< XComponentContext > rComponentContext =
        defaultBootstrap_InitialComponentContext();

    // retrieve the servicemanager from the context
    Reference< XMultiComponentFactory > rServiceManager =
        rComponentContext->getServiceManager();

    // instantiate a sample service with the servicemanager.
    Reference< XInterface > rInstance =
        rServiceManager->createInstanceWithContext(
            "com.sun.star.bridge.UnoUrlResolver",
            rComponentContext );

    // Query for the XUnoUrlResolver interface
    Reference< XUnoUrlResolver > rResolver( rInstance, UNO_QUERY );

    if( ! rResolver.is() )
    {
        printf( "Error: Couldn't instantiate com.sun.star.bridge.UnoUrlResolver service\n" );
        return 1;
    }
    try
    {
        // resolve the uno-url
        rInstance = rResolver->resolve( OUString(
            "uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager" ) );

        if( ! rInstance.is() )
        {
            printf( "StarOffice.ServiceManager is not exported from remote counterpart\n" );
            return 1;
        }

        // query for the simpler XMultiServiceFactory interface, sufficient for scripting
        Reference< XMultiServiceFactory > rOfficeServiceManager (rInstance, UNO_QUERY);

        if( ! rInstance.is() )
        {
            printf( "XMultiServiceFactory interface is not exported for StarOffice.ServiceManager\n" );
            return 1;
        }

        printf( "Connected successfully to the office\n" );
    }
    catch( Exception &e )
    {
        OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
        printf( "Error: %s\n", o.pData->buffer );
        return 1;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
