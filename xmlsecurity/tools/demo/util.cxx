/*************************************************************************
 *
 *  $RCSfile: util.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-05-18 10:02:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
