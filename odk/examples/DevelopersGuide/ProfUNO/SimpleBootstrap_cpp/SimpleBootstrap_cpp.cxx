/*************************************************************************
 *
 *  $RCSfile: SimpleBootstrap_cpp.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 15:04:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
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

#include <iostream>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>

using namespace std;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;


int SAL_CALL main( int argc, char **argv )
{
    try
    {
        // get the remote office component context
        Reference< XComponentContext > xContext( ::cppu::bootstrap() );
        if ( !xContext.is() )
        {
            cerr << "no component context!\n";
            return 1;
        }

        // get the remote office service manager
        Reference< XMultiComponentFactory > xServiceManager(
            xContext->getServiceManager() );
        if ( !xServiceManager.is() )
        {
            cerr << "no service manager!\n";
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
            cerr << "opening spreadsheet document failed!\n";
            return 1;
        }
    }
    catch ( ::cppu::BootstrapException & e )
    {
        cerr << "caught BootstrapException: "
             << OUStringToOString( e.getMessage(), RTL_TEXTENCODING_ASCII_US ).getStr()
             << '\n';
        return 1;
    }
    catch ( Exception & e )
    {
        cerr << "caught UNO exception: "
             << OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr()
             << '\n';
        return 1;
    }

    return 0;
}
