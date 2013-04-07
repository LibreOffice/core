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

#include "cppuhelper/bootstrap.hxx"

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>

#include <tools/urlobj.hxx>
#include <osl/file.hxx>

#include <memory>
#include <iostream>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sheet;

using ::com::sun::star::beans::Property;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::lang::XMultiComponentFactory;
using ::com::sun::star::frame::XComponentLoader;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::ucb::XSimpleFileAccess3;
using ::com::sun::star::document::XTypeDetection;

using ::std::auto_ptr;

const OUString EXTN(".xls");

OUString convertToURL( const OUString& rPath )
{
        OUString aURL;
        INetURLObject aObj;
        aObj.SetURL( rPath );
        bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
        if ( bIsURL )
                aURL = rPath;
        else
        {
                osl::FileBase::getFileURLFromSystemPath( rPath, aURL );
                if ( aURL.equals( rPath ) )
                    throw uno::RuntimeException( OUString( "could'nt convert " ).concat( rPath ).concat( OUString( " to a URL, is it a fully qualified path name? " ) ), Reference< uno::XInterface >() );
        }
        return aURL;
}

OUString ascii(const sal_Char* cstr)
{
    return OUString::createFromAscii(cstr);
}

const sal_Char* getStr(const OUString& ou)
{
    return OUStringToOString(ou, RTL_TEXTENCODING_UTF8).getStr();
}


int usage( const char* pName )
{
    std::cerr << "usage: " << pName << "<path to testdocument dir> <output_directory>" << std::endl;
        return 1;

}

class TestVBA
{
private:
    Reference< XComponentContext >  mxContext;
    Reference< XMultiComponentFactory > mxMCF;
    Reference< XComponentLoader > mxCompLoader;
    Reference< XSimpleFileAccess3 > mxSFA;
    OUString msOutDirPath;
protected:
public:
    TestVBA( const Reference< XComponentContext >&  _xContext,
        const Reference< XMultiComponentFactory >& _xMCF,
        const Reference< XComponentLoader >& _xCompLoader,
        const OUString& _outDirPath ) : mxContext( _xContext ), mxMCF( _xMCF ),
mxCompLoader( _xCompLoader ), msOutDirPath( convertToURL( _outDirPath  ) )
    {
        mxSFA.set( ucb::SimpleFileAccess::create(_xContext) );
    }

    OUString getLogLocation() throw (  beans::UnknownPropertyException,  lang::IllegalArgumentException, lang::WrappedTargetException,  uno::Exception )
    {
        OUString sLogLocation;
        Reference< XPropertySet > pathSettings( mxMCF->createInstanceWithContext( OUString( "com.sun.star.comp.framework.PathSettings" ), mxContext), uno::UNO_QUERY_THROW );
        pathSettings->getPropertyValue( OUString( "Work" ) ) >>= sLogLocation;
        sLogLocation = sLogLocation.concat( OUString( "/" ) ).concat( OUString( "HelperAPI-test.log" ) );
        return sLogLocation;
    }
    OUString getLogLocationWithName( OUString fileName ) throw (  beans::UnknownPropertyException,  lang::IllegalArgumentException, lang::WrappedTargetException,  uno::Exception )
    {
        printf("%s\n", getenv("HOME") );
        printf("file name %s\n", OUStringToOString( fileName, RTL_TEXTENCODING_UTF8 ).getStr() );
        OUString sLogLocation;
        Reference< XPropertySet > pathSettings( mxMCF->createInstanceWithContext( OUString( "com.sun.star.comp.framework.PathSettings" ), mxContext), uno::UNO_QUERY_THROW );
        pathSettings->getPropertyValue( OUString( "Work" ) ) >>= sLogLocation;
        sLogLocation = sLogLocation.concat( OUString( "/" ) ).concat( fileName.copy ( 0, fileName.lastIndexOf( EXTN )  ) + OUString( ".log" ) );
        return sLogLocation;
    }

    void proccessDocument( const OUString& sUrl )
    {
            if ( !mxSFA->isFolder( sUrl ) && sUrl.endsWithIgnoreAsciiCaseAsciiL( ".xls", 4 ) )

            {
                try
                {
                    OSL_TRACE( "processing %s",  OUStringToOString( sUrl, RTL_TEXTENCODING_UTF8 ).getStr() );
                    printf( "processing %s\n",  OUStringToOString( sUrl, RTL_TEXTENCODING_UTF8 ).getStr() );
                    // Loading the wanted document
                    Sequence< PropertyValue > propertyValues(1);
                    propertyValues[0].Name = OUString( "Hidden" );
                    propertyValues[0].Value <<= false;

                    OUString sfileUrl = convertToURL( sUrl );
                    printf( "try to get xDoc %s\n", OUStringToOString( sfileUrl, RTL_TEXTENCODING_UTF8 ).getStr() );
                    Reference< uno::XInterface > xDoc =
                        mxCompLoader->loadComponentFromURL( sfileUrl, OUString( "_blank" ), 0, propertyValues);
                    printf( "got xDoc\n" );

                    OUString logFileURL = convertToURL( getLogLocation() );
                    try
                    {
                        Reference< script::provider::XScriptProviderSupplier > xSupplier( xDoc, uno::UNO_QUERY_THROW ) ;
                        if ( mxSFA->exists( logFileURL ) )
                            mxSFA->kill( logFileURL );

                        printf("try to get the ScriptProvider\n");
                        Reference< script::provider::XScriptProvider > xProv = xSupplier->getScriptProvider();
                        printf("get the ScriptProvider\n");
                        printf("try to get the Script\n");
                        Reference< script::provider::XScript > xScript;
                        try
                        {
                            xScript = xProv->getScript( OUString( "vnd.sun.star.script:VBAProject.TestMacros.Main?language=Basic&location=document"));
                        } catch ( uno::Exception& e )
                        {
                            try
                            {
                                xScript = xProv->getScript( OUString( "vnd.sun.star.script:VBAProject.testMacro.Main?language=Basic&location=document" ));
                            } catch ( uno::Exception& e2 )
                            {
                                xScript = xProv->getScript( OUString( "vnd.sun.star.script:VBAProject.testMain.Main?language=Basic&location=document" ));
                            }
                        }
                        OSL_TRACE("Got script for doc %s", OUStringToOString( sUrl, RTL_TEXTENCODING_UTF8 ).getStr() );
                        printf("get the Script\n");
                        Sequence< uno::Any > aArgs;
                        Sequence< sal_Int16 > aOutArgsIndex;
                        Sequence< uno::Any > aOutArgs;

                        xScript->invoke(aArgs, aOutArgsIndex, aOutArgs);

                        OUString fileName = sUrl.copy ( sUrl.lastIndexOf( '/' ) );
                        OUString newLocation = msOutDirPath + fileName.copy ( 0, fileName.lastIndexOf( EXTN )  ) + OUString( ".log" );
                        try
                        {
                            printf("move log file\n");
                            mxSFA->move( logFileURL, newLocation );
                            OSL_TRACE("new logfile location is %s ", OUStringToOString( newLocation, RTL_TEXTENCODING_UTF8 ).getStr() );
                            printf("moved to new location\n");
                        }
                        catch ( uno::Exception& e )
                        {
                            logFileURL = convertToURL( getLogLocationWithName( fileName ) );
                            printf("move log file from %s\n", OUStringToOString( logFileURL, RTL_TEXTENCODING_UTF8 ).getStr() );
                            mxSFA->move( logFileURL, newLocation );
                            OSL_TRACE("new logfile location is %s ", OUStringToOString( newLocation, RTL_TEXTENCODING_UTF8 ).getStr() );
                            printf("moved to new location\n");
                        }

                    }
                    catch ( uno::Exception& e )
                    {
                        std::cerr << "Caught exception " << OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
                    }

                    // interface is supported, otherwise use XComponent.dispose
                    Reference< util::XCloseable > xCloseable ( xDoc, uno::UNO_QUERY );

                    if ( xCloseable.is() )
                    {
                        printf("try to close\n");
                        // will close application. and only run a test case for 3.0
                        // maybe it is a bug. yes, it is a bug
                        // if only one frame and model, click a button which related will colse.
                        // will make a crash. It related with window listener.
                        // so, for run all test cases, it should not close the document at this moment.
                        xCloseable->close(false);
                        printf("closed\n");
                    }
                    else
                    {
                        printf("try to dispose\n");
                        Reference< XComponent > xComp( xDoc, uno::UNO_QUERY_THROW );
                        // same as close.
                        xComp->dispose();
                        printf("disposed\n");
                    }
                }
                catch( uno::Exception& e )
                {
                    std::cerr << "Caught exception " << OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
                }

            }
        printf("complete processing %s\n", OUStringToOString( sUrl, RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    void traverse( const OUString& sFileDirectory )
    {
        OUString sFileDirectoryURL = convertToURL( sFileDirectory );
        if ( !mxSFA->isFolder( sFileDirectoryURL) )
        {
            throw lang::IllegalArgumentException( OUString( "not a directory: ").concat( sFileDirectoryURL ), Reference<uno::XInterface>(), 1 );
        }
        // Getting all files and directories in the current directory
        Sequence<OUString> entries = mxSFA->getFolderContents( sFileDirectoryURL, false );

        // Iterating for each file and directory
        printf( "Entries %d\n", (int)entries.getLength() );
        for ( sal_Int32 i = 0; i < entries.getLength(); ++i )
        {
            proccessDocument( entries[ i ] );
        }
    }
};

void tryDispose( Reference< uno::XInterface > xIF, const char* sComp )
{
    Reference< lang::XComponent > xComponent( xIF, uno::UNO_QUERY );
    if ( xComponent.is() )
    {
        try
        {
            xComponent->dispose();
        }
        catch( uno::Exception& e )
        {
            std::cerr << "tryDispose caught exception " <<OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() << " while disposing " <<  sComp << std::endl;
        }
    }
}
int main( int argv, char** argc )
{
    if ( !( argv > 2 ) )
        return usage( argc[0] );
    try
    {

        OSL_TRACE("Attempting to bootstrap normal");
        Reference<XComponentContext> xCC = ::cppu::bootstrap();
        Reference<XMultiComponentFactory> xFactory = xCC->getServiceManager();
        OSL_TRACE("got servicemanager");
        std::cout << "got servicemanager" << std::endl;
        Reference<XDesktop2> desktop = Desktop::create(xCC);
        OSL_TRACE("got desktop");
        std::cout << "got desktop" << std::endl;
        Reference<frame::XComponentLoader> xLoader(desktop, UNO_QUERY_THROW);
        TestVBA* dTest = new TestVBA( xCC, xFactory, xLoader, ascii( argc[ 2 ] ) );
        if ( argv == 4 )
        {
            std::cout << "before process" << std::endl;
            dTest->proccessDocument( ascii( argc[ 3 ] ) );
            std::cout << "after process" << std::endl;
        }
        else
        {
            dTest->traverse( ascii( argc[ 1 ] ) );
        }
        delete dTest;

    }
    catch( uno::Exception& e )
    {
        std::cerr << "Caught Exception " << OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() << std::endl;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
