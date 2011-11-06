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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

#include "util.hxx"

#include <stdio.h>

#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamhelper.hxx>
#include <tools/string.hxx>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssi = com::sun::star::io;

using namespace ::com::sun::star;

cssu::Reference< cssl::XMultiServiceFactory > CreateDemoServiceFactory()
{
    cssu::Reference< cssl::XMultiServiceFactory > xMSF;

    try
    {
        cssu::Reference< cssl::XMultiComponentFactory > xLocalServiceManager = NULL ;
        cssu::Reference< cssu::XComponentContext > xLocalComponentContext = NULL ;

        cssu::Reference< ::com::sun::star::registry::XSimpleRegistry > xSimpleRegistry
            = ::cppu::createSimpleRegistry();
        OSL_ENSURE( xSimpleRegistry.is(),
            "serviceManager - "
            "Cannot create simple registry" ) ;

        xSimpleRegistry->open(rtl::OUString::createFromAscii( "demo.rdb" ), sal_True, sal_False);
        OSL_ENSURE( xSimpleRegistry->isValid() ,
            "serviceManager - "
            "Cannot open xml security registry rdb" ) ;

        xLocalComponentContext = ::cppu::bootstrap_InitialComponentContext( xSimpleRegistry ) ;
        OSL_ENSURE( xLocalComponentContext.is() ,
            "serviceManager - "
            "Cannot create intial component context" ) ;

        xLocalServiceManager = xLocalComponentContext->getServiceManager() ;
        OSL_ENSURE( xLocalServiceManager.is() ,
            "serviceManager - "
            "Cannot create intial service manager" ) ;

        xMSF = cssu::Reference< cssl::XMultiServiceFactory >(xLocalServiceManager, cssu::UNO_QUERY) ;

        ::comphelper::setProcessServiceFactory( xMSF );
    }
    catch( cssu::Exception& e )
    {
        fprintf( stderr , "Error creating ServiceManager, Exception is %s\n" , rtl::OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).getStr() ) ;
        exit (-1);
    }

    return xMSF;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > OpenInputStream( const ::rtl::OUString& rStreamName )
{
    SvFileStream* pStream = new SvFileStream( rStreamName, STREAM_READ );
    pStream->Seek( STREAM_SEEK_TO_END );
    sal_uLong nBytes = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    SvLockBytesRef xLockBytes = new SvLockBytes( pStream, sal_True );
    uno::Reference< io::XInputStream > xInputStream = new utl::OInputStreamHelper( xLockBytes, nBytes );

    return xInputStream;

}

::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > OpenOutputStream( const ::rtl::OUString& rStreamName )
{
    SvFileStream* pStream = new SvFileStream( rStreamName, STREAM_WRITE );
    SvLockBytesRef xLockBytes = new SvLockBytes( pStream, sal_True );
    uno::Reference< io::XOutputStream > xOutputStream = new utl::OOutputStreamHelper( xLockBytes );

    return xOutputStream;
}
