/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
