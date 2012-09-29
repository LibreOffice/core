/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <stdio.h>
#include "osl/file.h"

#include <rtl/ustrbuf.hxx>

#include <xmlscript/xmldlg_imexp.hxx>
#include <xmlscript/xml_helper.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/implbase2.hxx>

#include <comphelper/processfactory.hxx>

#include <vcl/svapp.hxx>

#include <com/sun/star/io/XActiveDataSource.hpp>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDialog.hpp>

#include <com/sun/star/container/XNameContainer.hpp>


using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;




Reference< XComponentContext > createInitialComponentContext(
    OUString const & inst_dir )
{
    Reference< XComponentContext > xContext;

    try
    {
        OUString file_url;
        oslFileError rc = osl_getFileURLFromSystemPath(
            inst_dir.pData, &file_url.pData );
        OSL_ASSERT( osl_File_E_None == rc );

        OUString unorc = file_url + OUString("/program/" SAL_CONFIGFILE("uno") );

        return defaultBootstrap_InitialComponentContext( unorc );
    }

    catch( const Exception& rExc )
    {
        OString aStr( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( aStr.getStr() );
    }

    return xContext;
}


// -----------------------------------------------------------------------

Reference< container::XNameContainer > importFile(
    char const * fname,
    Reference< XComponentContext > const & xContext )
{
    // create the input stream
    FILE *f = ::fopen( fname, "rb" );
    if (f)
    {
        ::fseek( f, 0 ,SEEK_END );
        int nLength = ::ftell( f );
        ::fseek( f, 0, SEEK_SET );

        ByteSequence bytes( nLength );
        ::fread( bytes.getArray(), nLength, 1, f );
        ::fclose( f );

        Reference< container::XNameContainer > xModel( xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.awt.UnoControlDialogModel", xContext ), UNO_QUERY );
        ::xmlscript::importDialogModel( ::xmlscript::createInputStream( bytes ), xModel, xContext );

        return xModel;
    }
    else
    {
        throw Exception( "### Cannot read file!", Reference< XInterface >() );
    }
}

void exportToFile(
    char const * fname,
    Reference< container::XNameContainer > const & xModel,
    Reference< XComponentContext > const & xContext )
{
    Reference< io::XInputStreamProvider > xProvider( ::xmlscript::exportDialogModel( xModel, xContext ) );
    Reference< io::XInputStream > xStream( xProvider->createInputStream() );

    Sequence< sal_Int8 > bytes;
    sal_Int32 nRead = xStream->readBytes( bytes, xStream->available() );
    for (;;)
    {
        Sequence< sal_Int8 > readBytes;
        nRead = xStream->readBytes( readBytes, 1024 );
        if (! nRead)
            break;
        OSL_ASSERT( readBytes.getLength() >= nRead );

        sal_Int32 nPos = bytes.getLength();
        bytes.realloc( nPos + nRead );
        memcpy( bytes.getArray() + nPos, readBytes.getConstArray(), (sal_uInt32)nRead );
    }

    FILE * f = ::fopen( fname, "w" );
    ::fwrite( bytes.getConstArray(), 1, bytes.getLength(), f );
    ::fflush( f );
    ::fclose( f );
}



class MyApp : public Application
{
public:
    void Main();
};

MyApp aMyApp;

// -----------------------------------------------------------------------

void MyApp::Main()
{
    if (GetCommandLineParamCount() < 2)
    {
        OSL_FAIL( "usage: imexp inst_dir inputfile [outputfile]\n" );
        return;
    }

    Reference< XComponentContext > xContext(
        createInitialComponentContext( OUString( GetCommandLineParam( 0 ) ) ) );
    Reference< lang::XMultiServiceFactory > xMSF(
        xContext->getServiceManager(), UNO_QUERY );

    try
    {
        ::comphelper::setProcessServiceFactory( xMSF );

        Reference< awt::XToolkit> xToolkit( xMSF->createInstance( "com.sun.star.awt.ExtToolkit" ), UNO_QUERY );

        // import dialogs
        OString aParam1( OUStringToOString(
                             OUString( GetCommandLineParam( 1 ) ),
                             RTL_TEXTENCODING_ASCII_US ) );
        Reference< container::XNameContainer > xModel(
            importFile( aParam1.getStr(), xContext ) );
        OSL_ASSERT( xModel.is() );

        Reference< awt::XControl > xDlg( xMSF->createInstance( "com.sun.star.awt.UnoControlDialog" ), UNO_QUERY );
        xDlg->setModel( Reference< awt::XControlModel >::query( xModel ) );
        xDlg->createPeer( xToolkit, 0 );
        Reference< awt::XDialog > xD( xDlg, UNO_QUERY );
        xD->execute();

        if (GetCommandLineParamCount() == 3)
        {
            // write modified dialogs
            OString aParam2( OUStringToOString(
                                 OUString( GetCommandLineParam( 2 ) ), RTL_TEXTENCODING_ASCII_US ) );
            exportToFile( aParam2.getStr(), xModel, xContext );
        }
    }
    catch (const xml::sax::SAXException & rExc)
    {
        OString aStr( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        uno::Exception exc;
        if (rExc.WrappedException >>= exc)
        {
            aStr += OString( " >>> " );
            aStr += OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US );
        }
        OSL_FAIL( aStr.getStr() );
    }
    catch (const uno::Exception & rExc)
    {
        OString aStr( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( aStr.getStr() );
    }

    Reference< lang::XComponent > xComp( xContext, UNO_QUERY );
    if (xComp.is())
    {
        xComp->dispose();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
