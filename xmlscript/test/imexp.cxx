/*************************************************************************
 *
 *  $RCSfile: imexp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-02-20 14:05:25 $
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

#include <stdio.h>

#include <xmlscript/xmldlg_imexp.hxx>
#include <xmlscript/xml_helper.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase2.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/regpathhelper.hxx>

#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <svtools/unoiface.hxx> // InitExtToolkit

#include <com/sun/star/io/XActiveDataSource.hpp>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XControlModel.hpp>

#include <com/sun/star/container/XNameContainer.hpp>


using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;



//-----------------------------------------
// The document handler, which is needed for the saxparser
// The Documenthandler for reading sax
//-----------------------------------------
class ErrorHandler :
    public WeakImplHelper2< xml::sax::XEntityResolver , xml::sax::XErrorHandler >
{
public:
    ErrorHandler()
        {}

public: // Error handler
    virtual void SAL_CALL error(const Any& aSAXParseException) throw (xml::sax::SAXException, RuntimeException)
    {
        OSL_ENSURE( 0, "Error !\n" );
        Exception exc;
        OSL_VERIFY( aSAXParseException >>= exc );
        throw exc;
    }
    virtual void SAL_CALL fatalError(const Any& aSAXParseException) throw (xml::sax::SAXException, RuntimeException)
    {
        OSL_ENSURE( 0, "Fatal Error !\n" );
        Exception exc;
        OSL_VERIFY( aSAXParseException >>= exc );
        throw exc;
    }
    virtual void SAL_CALL warning(const Any& aSAXParseException) throw (xml::sax::SAXException, RuntimeException)
    {
        OSL_ENSURE( 0, "Warning !\n" );
        Exception exc;
        OSL_VERIFY( aSAXParseException >>= exc );
        throw exc;
    }

    virtual xml::sax::InputSource SAL_CALL resolveEntity(
        const OUString& sPublicId,
        const OUString& sSystemId)
        throw (xml::sax::SAXException,RuntimeException)
    {
        OSL_ENSURE( 0, "not impl !\n" );
        return xml::sax::InputSource();
    }
};



Reference< lang::XMultiServiceFactory > createApplicationServiceManager()
{
    Reference< lang::XMultiServiceFactory > xReturn = createServiceFactory();

    try
    {

    if ( xReturn.is() )
    {
        Reference< lang::XInitialization > xInit ( xReturn, UNO_QUERY ) ;
        if ( xInit.is() )
        {
            OUString localRegistry = ::comphelper::getPathToUserRegistry();
            OUString systemRegistry = ::comphelper::getPathToSystemRegistry();

            Reference< registry::XSimpleRegistry > xLocalRegistry(
                xReturn->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.registry.SimpleRegistry") ) ), UNO_QUERY );
            Reference< registry::XSimpleRegistry > xSystemRegistry(
                xReturn->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.SimpleRegistry") ) ), UNO_QUERY );
            if ( xLocalRegistry.is() && (localRegistry.getLength() > 0) )
            {
                try
                {
                    xLocalRegistry->open( localRegistry, sal_False, sal_True);
                }
                catch ( registry::InvalidRegistryException& )
                {
                }

                if ( !xLocalRegistry->isValid() )
                    xLocalRegistry->open(localRegistry, sal_True, sal_True);
            }

            if ( xSystemRegistry.is() && (systemRegistry.getLength() > 0) )
                xSystemRegistry->open( systemRegistry, sal_True, sal_False);

            if ( (xLocalRegistry.is() && xLocalRegistry->isValid()) &&
                 (xSystemRegistry.is() && xSystemRegistry->isValid()) )
            {
                Sequence< Any > seqAnys(2);
                seqAnys[0] <<= xLocalRegistry ;
                seqAnys[1] <<= xSystemRegistry ;

                Reference < registry::XSimpleRegistry > xReg(
                    xReturn->createInstanceWithArguments(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.NestedRegistry")), seqAnys ), UNO_QUERY );

                seqAnys = Sequence< Any >( 1 );
                seqAnys[0] <<= xReg;
                if ( xReg.is() )
                    xInit->initialize( seqAnys );
            }
        }
        else
        {
            xReturn = Reference< lang::XMultiServiceFactory >();
        }
    }

    Reference < registry::XImplementationRegistration > xReg(
        xReturn->createInstance( OUString::createFromAscii( "com.sun.star.registry.ImplementationRegistration" ) ),
        UNO_QUERY );

#ifdef SAL_W32
    OUString aDllName = OUString::createFromAscii( "sax.dll" );
#else
    OUString aDllName = OUString::createFromAscii( "libsax.so" );
#endif
    xReg->registerImplementation(
        OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
        aDllName, Reference< registry::XSimpleRegistry > () );
#ifdef SAL_W32
    aDllName = OUString::createFromAscii( "tk" );
    aDllName += OUString::valueOf( (sal_Int32)SUPD );
    aDllName += OUString::createFromAscii( "mi.dll" );
#else
    aDllName = OUString::createFromAscii( "libtk" );
    aDllName += OUString::valueOf( (sal_Int32)SUPD );
    aDllName += OUString::createFromAscii( ".so" );
#endif
    xReg->registerImplementation(
        OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
        aDllName, Reference< registry::XSimpleRegistry > () );
#ifdef SAL_W32
    aDllName = OUString::createFromAscii( "svt" );
    aDllName += OUString::valueOf( (sal_Int32)SUPD );
    aDllName += OUString::createFromAscii( "mi.dll" );
#else
    aDllName = OUString::createFromAscii( "libsvt" );
    aDllName += OUString::valueOf( (sal_Int32)SUPD );
    aDllName += OUString::createFromAscii( ".so" );
#endif
    xReg->registerImplementation(
        OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
        aDllName, Reference< registry::XSimpleRegistry > () );

    }

    catch( Exception& rExc )
    {
        OString aStr( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( 0, aStr.getStr() );
    }

    return xReturn ;
}


// -----------------------------------------------------------------------

Reference< container::XNameContainer > importFile(
    char const * fname,
    Reference< lang::XMultiServiceFactory > const & xSMgr )
{
    Reference< xml::sax::XParser > xParser( xSMgr->createInstance(
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser") ) ), UNO_QUERY );
    if (xParser.is())
    {
        ErrorHandler * pHandler = new ErrorHandler();
        xParser->setErrorHandler( (xml::sax::XErrorHandler *)pHandler );
        xParser->setEntityResolver( (xml::sax::XEntityResolver *)pHandler );

        Reference< container::XNameContainer > xModel( xSMgr->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel") ) ), UNO_QUERY );
        OSL_ASSERT( xModel.is() );
        xParser->setDocumentHandler( ::xmlscript::importDialogModel( xModel ) );

        // create the input stream
        FILE *f = ::fopen( fname, "rb" );
        if (f)
        {
            ::fseek( f, 0 ,SEEK_END );
            int nLength = ::ftell( f );
            ::fseek( f, 0, SEEK_SET );

            ByteSequence seqIn( nLength );
            ::fread( seqIn.getArray(), nLength, 1, f );
            ::fclose( f );

            xml::sax::InputSource source;
            source.aInputStream = ::xmlscript::createInputStream( seqIn );
            source.sSystemId    = OUString::createFromAscii( fname );

            // start parsing
            xParser->parseStream( source );
            return xModel;
        }
        else
        {
            OSL_ENSURE( 0, "### Cannot read file!\n" );
        }
    }
    else
    {
        OSL_ENSURE( 0, "### couln't create sax-parser component\n" );
    }
    return Reference< container::XNameContainer >();
}

bool exportToFile(
    char const * fname,
    Reference< container::XNameContainer > const & xModel,
    Reference< lang::XMultiServiceFactory > const & xSMgr )
{
    Reference< xml::sax::XExtendedDocumentHandler > xHandler(
        xSMgr->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer") ) ), UNO_QUERY );
    OSL_ASSERT( xHandler.is() );
    if (! xHandler.is())
    {
        OSL_ENSURE( 0, "### couln't create sax-writer component\n" );
        return false;
    }

    ByteSequence seq;
    Reference< io::XActiveDataSource > xSource( xHandler, UNO_QUERY );
    xSource->setOutputStream( ::xmlscript::createOutputStream( &seq ) );

    xHandler->startDocument();

    ::xmlscript::exportDialogModel(
        xModel,
        Reference< xml::sax::XExtendedDocumentHandler >::query( xHandler ) );

    xHandler->endDocument();

    FILE * f = ::fopen( fname, "w" );
    ::fwrite( seq.getConstArray(), 1 ,seq.getLength(), f );
    ::fflush( f );
    ::fclose( f );
    return true;
}



class MyApp : public Application
{
public:
    void        Main();
};

MyApp aMyApp;

// -----------------------------------------------------------------------

void MyApp::Main()
{
    if( GetCommandLineParamCount() < 1)
    {
        OSL_ENSURE( 0, "usage: imexp inputfile [outputfile]\n" );
        return;
    }

    Reference< lang::XMultiServiceFactory >  xMSF = createApplicationServiceManager();

    try
    {
        ::comphelper::setProcessServiceFactory( xMSF );
        InitExtVclToolkit();
        Application::RegisterUnoServices();

        Reference< awt::XToolkit> xToolkit( xMSF->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.ExtToolkit" ) ) ), UNO_QUERY );

        // import dialog
        OString aParam1( OUStringToOString( OUString( GetCommandLineParam( 0 ) ), RTL_TEXTENCODING_ASCII_US ) );
        Reference< container::XNameContainer > xImport( importFile( aParam1.getStr(), xMSF ) );

        if (GetCommandLineParamCount() == 2)
        {
            // write and read again dialogs
            OString aParam2( OUStringToOString( OUString( GetCommandLineParam( 1 ) ), RTL_TEXTENCODING_ASCII_US ) );
            exportToFile( aParam2.getStr(), xImport, xMSF );
            // re-import
            xImport = importFile( aParam2.getStr(), xMSF );
        }

        Reference< awt::XControl > xDlg( xMSF->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialog" ) ) ), UNO_QUERY );
        xDlg->setModel( Reference< awt::XControlModel >::query( xImport ) );
        xDlg->createPeer( xToolkit, NULL );
        Reference< awt::XDialog > xD( xDlg, UNO_QUERY );
        xD->execute();
    }
    catch (uno::Exception & rExc)
    {
        OString aStr( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( 0, aStr.getStr() );
    }

    Reference< lang::XComponent > xComp( xMSF, UNO_QUERY );
    if (xComp.is())
    {
        xComp->dispose();
    }
}

