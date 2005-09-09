/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: util.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:38:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "util.hxx"

#include <stdio.h>

#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamhelper.hxx>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

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
    ULONG nBytes = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    SvLockBytesRef xLockBytes = new SvLockBytes( pStream, TRUE );
    uno::Reference< io::XInputStream > xInputStream = new utl::OInputStreamHelper( xLockBytes, nBytes );

    return xInputStream;

}

::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > OpenOutputStream( const ::rtl::OUString& rStreamName )
{
    SvFileStream* pStream = new SvFileStream( rStreamName, STREAM_WRITE );
    SvLockBytesRef xLockBytes = new SvLockBytes( pStream, TRUE );
    uno::Reference< io::XOutputStream > xOutputStream = new utl::OOutputStreamHelper( xLockBytes );

    return xOutputStream;
}
