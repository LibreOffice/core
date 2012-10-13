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

#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>


#include <comphelper/oslfile2streamwrap.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <svl/urihelper.hxx>
#include <osl/file.hxx>

#include <rtl/uri.hxx>

#include "xmlfilterjar.hxx"
#include "xmlfilterdialogstrings.hrc"
#include "xmlfiltersettingsdialog.hxx"
#include "typedetectionexport.hxx"
#include "typedetectionimport.hxx"

using namespace osl;
using namespace comphelper;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;

using ::rtl::OUString;
using ::rtl::Uri;

XMLFilterJarHelper::XMLFilterJarHelper( Reference< XMultiServiceFactory >& xMSF )
: mxMSF( xMSF ),
    sVndSunStarPackage( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package:" ) ),
    sXSLTPath( RTL_CONSTASCII_USTRINGPARAM( "$(user)/xslt/" ) ),
    sDTDPath( RTL_CONSTASCII_USTRINGPARAM( "$(user)/dtd/" ) ),
    sTemplatePath( RTL_CONSTASCII_USTRINGPARAM( "$(user)/template/") ),
    sSpecialConfigManager( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.config.SpecialConfigManager" ) ),
    sPump( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.Pump" ) ),
    sProgPath( RTL_CONSTASCII_USTRINGPARAM( "$(prog)/" ) )
{
    try
    {
        Reference< XConfigManager > xCfgMgr( xMSF->createInstance(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.config.SpecialConfigManager" ))), UNO_QUERY );
        if( xCfgMgr.is() )
        {
            sProgPath = xCfgMgr->substituteVariables( sProgPath );
            sXSLTPath = xCfgMgr->substituteVariables( sXSLTPath );
            sDTDPath = xCfgMgr->substituteVariables( sDTDPath );
            sTemplatePath = xCfgMgr->substituteVariables( sTemplatePath );
        }
    }
    catch(const Exception&)
    {
    }
}

static OUString encodeZipUri( const OUString& rURI )
{
    return Uri::encode( rURI, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8 );
}

static Reference< XInterface > addFolder( Reference< XInterface >& xRootFolder, Reference< XSingleServiceFactory >& xFactory, const OUString& rName ) throw( Exception )
{
    if ( rName == ".." || rName == "." )
        throw lang::IllegalArgumentException();

    Sequence< Any > aArgs(1);
    aArgs[0] <<= (sal_Bool)sal_True;

    Reference< XInterface > xFolder( xFactory->createInstanceWithArguments(aArgs) );
    Reference< XNamed > xNamed( xFolder, UNO_QUERY );
    Reference< XChild > xChild( xFolder, UNO_QUERY );

    if( xNamed.is() && xChild.is() )
    {
        OUString aName( encodeZipUri( rName ) );
        xNamed->setName( aName );
        xChild->setParent( xRootFolder );
    }

    return xFolder;
}

static void _addFile( Reference< XInterface >& xRootFolder, Reference< XSingleServiceFactory >& xFactory, Reference< XInputStream >& xInput, OUString aName ) throw( Exception )
{

    Reference< XActiveDataSink > xSink( xFactory->createInstance(), UNO_QUERY );
    Reference< XUnoTunnel > xTunnel( xSink, UNO_QUERY );
    if( xSink.is() && xTunnel.is())
    {
        Reference< XNameContainer > xNameContainer(xRootFolder, UNO_QUERY );
        xNameContainer->insertByName(aName = encodeZipUri( aName ), makeAny(xTunnel));
        xSink->setInputStream( xInput );
    }
}

void XMLFilterJarHelper::addFile( Reference< XInterface > xRootFolder, Reference< XSingleServiceFactory > xFactory, const OUString& rSourceFile ) throw( Exception )
{
    if( !rSourceFile.isEmpty() &&
        (rSourceFile.compareToAscii( RTL_CONSTASCII_STRINGPARAM("http:") ) != 0) &&
        (rSourceFile.compareToAscii( RTL_CONSTASCII_STRINGPARAM("shttp:") ) != 0) &&
        (rSourceFile.compareToAscii( RTL_CONSTASCII_STRINGPARAM("jar:") ) != 0) &&
        (rSourceFile.compareToAscii( RTL_CONSTASCII_STRINGPARAM("ftp:") ) != 0))
    {
        OUString aFileURL( rSourceFile );

        if( !aFileURL.matchIgnoreAsciiCase( OUString( RTL_CONSTASCII_USTRINGPARAM("file://") ) ) )
        {
            aFileURL = URIHelper::SmartRel2Abs( sProgPath, aFileURL, Link(), false );
        }

        INetURLObject aURL( aFileURL );
        OUString aName( aURL.getName() );

        SvFileStream* pStream = new SvFileStream(aFileURL, STREAM_READ );
        Reference< XInputStream > xInput(  new utl::OSeekableInputStreamWrapper( pStream, true ) );
        _addFile( xRootFolder, xFactory, xInput, aName );
    }
}

bool XMLFilterJarHelper::savePackage( const OUString& rPackageURL, const XMLFilterVector& rFilters )
{
    try
    {
        osl::File::remove( rPackageURL );

        // create the package jar file

        Sequence< Any > aArguments( 2 );
        aArguments[ 0 ] <<= rPackageURL;

        // let ZipPackage be used ( no manifest.xml is required )
        beans::NamedValue aArg;
        aArg.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StorageFormat" ) );
        aArg.Value <<= ZIP_STORAGE_FORMAT_STRING;
        aArguments[ 1 ] <<= aArg;

        Reference< XHierarchicalNameAccess > xIfc(
            mxMSF->createInstanceWithArguments(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.packages.comp.ZipPackage" )),
                aArguments ), UNO_QUERY );

        if( xIfc.is() )
        {
            Reference< XSingleServiceFactory > xFactory( xIfc, UNO_QUERY );

            // get root zip folder
            Reference< XInterface > xRootFolder;
            OUString szRootFolder( RTL_CONSTASCII_USTRINGPARAM("/") );
            xIfc->getByHierarchicalName( szRootFolder ) >>= xRootFolder;

            // export filters files
            XMLFilterVector::const_iterator aIter( rFilters.begin() );
            while( aIter != rFilters.end() )
            {
                const filter_info_impl* pFilter = (*aIter);

                Reference< XInterface > xFilterRoot( addFolder( xRootFolder, xFactory, pFilter->maFilterName ) );

                if( xFilterRoot.is() )
                {
                    if( !pFilter->maDTD.isEmpty() )
                        addFile( xFilterRoot, xFactory, pFilter->maDTD );

                    if( !pFilter->maExportXSLT.isEmpty() )
                        addFile( xFilterRoot, xFactory, pFilter->maExportXSLT );
                    try
                    {
                        if( !pFilter->maImportXSLT.isEmpty() )
                            addFile( xFilterRoot, xFactory, pFilter->maImportXSLT );
                    }
                    catch(const com::sun::star::container::ElementExistException&)
                    {
                    // in case of same named import / export XSLT the latter
                    // is ignored
                        OSL_FAIL( "XMLFilterJarHelper::same named xslt filter exception!" );
                    }

                    if( !pFilter->maImportTemplate.isEmpty() )
                        addFile( xFilterRoot, xFactory, pFilter->maImportTemplate );
                }

                ++aIter;
            }

            // create TypeDetection.xcu
            utl::TempFile aTempFile;
            aTempFile.EnableKillingFile();
            OUString aTempFileURL( aTempFile.GetURL() );

            {
                osl::File aOutputFile( aTempFileURL );
                /* osl::File::RC rc = */ aOutputFile.open( osl_File_OpenFlag_Write );
                Reference< XOutputStream > xOS( new OSLOutputStreamWrapper( aOutputFile ) );

                TypeDetectionExporter aExporter( mxMSF );
                aExporter.doExport(xOS,rFilters);
            }

            Reference< XInputStream > XIS(  new utl::OSeekableInputStreamWrapper( new SvFileStream(aTempFileURL, STREAM_READ ), true ) );
            OUString szTypeDetection( RTL_CONSTASCII_USTRINGPARAM( "TypeDetection.xcu" ) );
            _addFile( xRootFolder, xFactory,  XIS, szTypeDetection );

            Reference< XChangesBatch > xBatch( xIfc, UNO_QUERY );
            if( xBatch.is() )
                xBatch->commitChanges();

            return true;
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterJarHelper::savePackage exception catched!" );
    }

    osl::File::remove( rPackageURL );

    return false;
}



//

void XMLFilterJarHelper::openPackage( const OUString& rPackageURL, XMLFilterVector& rFilters )
{
    try
    {
        // create the package jar file

        Sequence< Any > aArguments( 2 );
        aArguments[ 0 ] <<= rPackageURL;

        // let ZipPackage be used ( no manifest.xml is required )
        beans::NamedValue aArg;
        aArg.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StorageFormat" ) );
        aArg.Value <<= ZIP_STORAGE_FORMAT_STRING;
        aArguments[ 1 ] <<= aArg;

        Reference< XHierarchicalNameAccess > xIfc(
            mxMSF->createInstanceWithArguments(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.packages.comp.ZipPackage" )),
                aArguments ), UNO_QUERY );

        if( xIfc.is() )
        {
            Reference< XSingleServiceFactory > xFactory( xIfc, UNO_QUERY );

            // get root zip folder
            Reference< XInterface > xRootFolder;
            OUString szRootFolder( RTL_CONSTASCII_USTRINGPARAM("/") );
            xIfc->getByHierarchicalName( szRootFolder ) >>= xRootFolder;

            OUString szTypeDetection( RTL_CONSTASCII_USTRINGPARAM("TypeDetection.xcu") );
            if( xIfc->hasByHierarchicalName( szTypeDetection ) )
            {
                Reference< XActiveDataSink > xTypeDetection;
                xIfc->getByHierarchicalName( szTypeDetection ) >>= xTypeDetection;

                if( xTypeDetection.is() )
                {
                    Reference< XInputStream > xIS( xTypeDetection->getInputStream() );

                    XMLFilterVector aFilters;
                    TypeDetectionImporter::doImport( mxMSF, xIS, aFilters );

                    // copy all files used by the filters imported from the
                    // typedetection to office/user/xslt
                    XMLFilterVector::iterator aIter( aFilters.begin() );
                    while( aIter != aFilters.end() )
                    {
                        if( copyFiles( xIfc, (*aIter) ) )
                        {
                            rFilters.push_back( (*aIter) );
                        }
                        else
                        {
                            // failed to copy all files
                            delete (*aIter);
                        }
                        ++aIter;
                    }
                }
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterJarHelper::savePackage exception catched!" );
    }
}

bool XMLFilterJarHelper::copyFiles( Reference< XHierarchicalNameAccess > xIfc, filter_info_impl* pFilter )
{
    bool bOk = copyFile( xIfc, pFilter->maDTD, sDTDPath );

    if( bOk )
        bOk = copyFile( xIfc, pFilter->maExportXSLT, sXSLTPath );

    if( bOk )
        bOk = copyFile( xIfc, pFilter->maImportXSLT, sXSLTPath );

    if( bOk )
        bOk = copyFile( xIfc, pFilter->maImportTemplate, sTemplatePath );

    return bOk;
}

bool XMLFilterJarHelper::copyFile( Reference< XHierarchicalNameAccess > xIfc, OUString& rURL, const OUString& rTargetURL )
{
    if( !rURL.matchIgnoreAsciiCase( sVndSunStarPackage ) )
        return true;

    try
    {
        OUString szPackagePath( encodeZipUri( rURL.copy( sVndSunStarPackage.getLength() ) ) );

        if ( ::comphelper::OStorageHelper::PathHasSegment( szPackagePath, OUString( RTL_CONSTASCII_USTRINGPARAM( ".." ) ) )
          || ::comphelper::OStorageHelper::PathHasSegment( szPackagePath, OUString( RTL_CONSTASCII_USTRINGPARAM( "." ) ) ) )
            throw lang::IllegalArgumentException();

        if( xIfc->hasByHierarchicalName( szPackagePath ) )
        {
            Reference< XActiveDataSink > xFileEntry;
            xIfc->getByHierarchicalName( szPackagePath ) >>= xFileEntry;

            if( xFileEntry.is() )
            {
                Reference< XInputStream > xIS( xFileEntry->getInputStream() );

                INetURLObject aBaseURL( rTargetURL );

                rURL = URIHelper::SmartRel2Abs( aBaseURL, szPackagePath, Link(), false );

                if( !rURL.isEmpty() )
                {
                    // create output directory if needed
                    if( !createDirectory( rURL ) )
                        return false;

                    ::osl::File file(rURL);
                    ::osl::FileBase::RC rc =
                        file.open(osl_File_OpenFlag_Write|osl_File_OpenFlag_Create);
                    if (::osl::FileBase::E_EXIST == rc) {
                        rc = file.open(osl_File_OpenFlag_Write);
                        if (::osl::FileBase::E_None == rc) {
                            file.setSize(0); // #i97170# truncate
                        }
                    }
                    if (::osl::FileBase::E_None != rc) {
                        throw RuntimeException();
                    }
                    Reference< XOutputStream > const xOS(
                            new comphelper::OSLOutputStreamWrapper(file));

                    return copyStreams( xIS, xOS );
                }
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterJarHelper::copyFile exception catched" );
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
