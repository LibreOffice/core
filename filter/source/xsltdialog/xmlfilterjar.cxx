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
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


#include <comphelper/oslfile2streamwrap.hxx>
#include <comphelper/storagehelper.hxx>
#include <osl/file.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <svl/urihelper.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>

#include <rtl/uri.hxx>

#include "xmlfiltercommon.hxx"
#include "xmlfilterjar.hxx"
#include "typedetectionexport.hxx"
#include "typedetectionimport.hxx"

using namespace osl;
using namespace comphelper;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;

using ::rtl::Uri;

constexpr OUStringLiteral sVndSunStarPackage(u"vnd.sun.star.Package:");

XMLFilterJarHelper::XMLFilterJarHelper( const Reference< XComponentContext >& rxContext )
: mxContext( rxContext ),
    sXSLTPath( "$(user)/xslt/" ),
    sTemplatePath( "$(user)/template/" ),
    sProgPath( "$(prog)/" )
{
    SvtPathOptions aOptions;
    sProgPath = aOptions.SubstituteVariable( sProgPath );
    sXSLTPath = aOptions.SubstituteVariable( sXSLTPath );
    sTemplatePath = aOptions.SubstituteVariable( sTemplatePath );
}

static OUString encodeZipUri( const OUString& rURI )
{
    return Uri::encode( rURI, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8 );
}

/// @throws Exception
static Reference< XInterface > addFolder( Reference< XInterface > const & xRootFolder, Reference< XSingleServiceFactory > const & xFactory, const OUString& rName )
{
    if ( rName == ".." || rName == "." )
        throw lang::IllegalArgumentException();

    Sequence< Any > aArgs(1);
    aArgs[0] <<= true;

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

/// @throws Exception
static void addFile_( Reference< XInterface > const & xRootFolder, Reference< XSingleServiceFactory > const & xFactory, Reference< XInputStream > const & xInput, const OUString& aName )
{
    Reference< XActiveDataSink > xSink( xFactory->createInstance(), UNO_QUERY );
    Reference< XUnoTunnel > xTunnel( xSink, UNO_QUERY );
    if( xSink.is() && xTunnel.is())
    {
        Reference< XNameContainer > xNameContainer(xRootFolder, UNO_QUERY );
        xNameContainer->insertByName(encodeZipUri( aName ), makeAny(xTunnel));
        xSink->setInputStream( xInput );
    }
}

void XMLFilterJarHelper::addFile( Reference< XInterface > const & xRootFolder, Reference< XSingleServiceFactory > const & xFactory, const OUString& rSourceFile )
{
    if( rSourceFile.isEmpty() ||
        rSourceFile.startsWith("http:") ||
        rSourceFile.startsWith("https:") ||
        rSourceFile.startsWith("jar:") ||
        rSourceFile.startsWith("ftp:") )
        return;

    OUString aFileURL( rSourceFile );

    if( !aFileURL.matchIgnoreAsciiCase("file://") )
    {
        aFileURL = URIHelper::SmartRel2Abs( INetURLObject(sProgPath), aFileURL, Link<OUString *, bool>(), false );
    }

    INetURLObject aURL( aFileURL );
    OUString aName( aURL.getName() );

    SvFileStream* pStream = new SvFileStream(aFileURL, StreamMode::READ );
    Reference< XInputStream > xInput(  new utl::OSeekableInputStreamWrapper( pStream, true ) );
    addFile_( xRootFolder, xFactory, xInput, aName );
}

bool XMLFilterJarHelper::savePackage( const OUString& rPackageURL, const std::vector<filter_info_impl*>& rFilters )
{
    try
    {
        osl::File::remove( rPackageURL );

        // create the package jar file

        Sequence< Any > aArguments( 2 );
        aArguments[ 0 ] <<= rPackageURL;

        // let ZipPackage be used ( no manifest.xml is required )
        beans::NamedValue aArg;
        aArg.Name = "StorageFormat";
        aArg.Value <<= OUString(ZIP_STORAGE_FORMAT_STRING);
        aArguments[ 1 ] <<= aArg;

        Reference< XHierarchicalNameAccess > xIfc(
            mxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                "com.sun.star.packages.comp.ZipPackage",
                aArguments, mxContext ), UNO_QUERY );

        if( xIfc.is() )
        {
            Reference< XSingleServiceFactory > xFactory( xIfc, UNO_QUERY );

            // get root zip folder
            Reference< XInterface > xRootFolder;
            xIfc->getByHierarchicalName( "/" ) >>= xRootFolder;

            // export filters files
            for (auto const& filter : rFilters)
            {
                Reference< XInterface > xFilterRoot( addFolder( xRootFolder, xFactory, filter->maFilterName ) );

                if( xFilterRoot.is() )
                {
                    if( !filter->maExportXSLT.isEmpty() )
                        addFile( xFilterRoot, xFactory, filter->maExportXSLT );
                    try
                    {
                        if( !filter->maImportXSLT.isEmpty() )
                            addFile( xFilterRoot, xFactory, filter->maImportXSLT );
                    }
                    catch(const css::container::ElementExistException&)
                    {
                    // in case of same named import / export XSLT the latter
                    // is ignored
                        OSL_FAIL( "XMLFilterJarHelper::same named xslt filter exception!" );
                    }

                    if( !filter->maImportTemplate.isEmpty() )
                        addFile( xFilterRoot, xFactory, filter->maImportTemplate );
                }
            }

            // create TypeDetection.xcu
            utl::TempFile aTempFile;
            aTempFile.EnableKillingFile();
            OUString aTempFileURL( aTempFile.GetURL() );

            {
                osl::File aOutputFile( aTempFileURL );
                (void)aOutputFile.open(osl_File_OpenFlag_Write);
                Reference< XOutputStream > xOS( new OSLOutputStreamWrapper( aOutputFile ) );

                TypeDetectionExporter aExporter( mxContext );
                aExporter.doExport(xOS,rFilters);
            }

            Reference< XInputStream > XIS(  new utl::OSeekableInputStreamWrapper( new SvFileStream(aTempFileURL, StreamMode::READ ), true ) );
            addFile_( xRootFolder, xFactory,  XIS, "TypeDetection.xcu" );

            Reference< XChangesBatch > xBatch( xIfc, UNO_QUERY );
            if( xBatch.is() )
                xBatch->commitChanges();

            return true;
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterJarHelper::savePackage exception caught!" );
    }

    osl::File::remove( rPackageURL );

    return false;
}


void XMLFilterJarHelper::openPackage( const OUString& rPackageURL,
                                      std::vector< std::unique_ptr<filter_info_impl> >& rFilters )
{
    try
    {
        // create the package jar file

        Sequence< Any > aArguments( 2 );
        aArguments[ 0 ] <<= rPackageURL;

        // let ZipPackage be used ( no manifest.xml is required )
        beans::NamedValue aArg;
        aArg.Name = "StorageFormat";
        aArg.Value <<= OUString(ZIP_STORAGE_FORMAT_STRING);
        aArguments[ 1 ] <<= aArg;

        Reference< XHierarchicalNameAccess > xIfc(
            mxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                "com.sun.star.packages.comp.ZipPackage",
                aArguments, mxContext ), UNO_QUERY );

        if( xIfc.is() )
        {
            // get root zip folder
            Reference< XInterface > xRootFolder;
            xIfc->getByHierarchicalName( "/" ) >>= xRootFolder;

            OUString szTypeDetection("TypeDetection.xcu");
            if( xIfc->hasByHierarchicalName( szTypeDetection ) )
            {
                Reference< XActiveDataSink > xTypeDetection;
                xIfc->getByHierarchicalName( szTypeDetection ) >>= xTypeDetection;

                if( xTypeDetection.is() )
                {
                    Reference< XInputStream > xIS( xTypeDetection->getInputStream() );

                    std::vector< std::unique_ptr<filter_info_impl> > aFilters;
                    TypeDetectionImporter::doImport( mxContext, xIS, aFilters );

                    // copy all files used by the filters imported from the
                    // typedetection to office/user/xslt
                    for (auto& filter : aFilters)
                    {
                        if( copyFiles( xIfc, filter.get() ) )
                        {
                            rFilters.push_back(std::move(filter));
                        }
                        else
                        {
                            // failed to copy all files
                            filter.reset();
                        }
                    }
                }
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "XMLFilterJarHelper::savePackage exception caught!" );
    }
}

bool XMLFilterJarHelper::copyFiles( const Reference< XHierarchicalNameAccess >& xIfc, filter_info_impl* pFilter )
{
    bool bOk = copyFile( xIfc, pFilter->maExportXSLT, sXSLTPath );

    if( bOk )
        bOk = copyFile( xIfc, pFilter->maImportXSLT, sXSLTPath );

    if( bOk )
        bOk = copyFile( xIfc, pFilter->maImportTemplate, sTemplatePath );

    return bOk;
}

bool XMLFilterJarHelper::copyFile( const Reference< XHierarchicalNameAccess >& xIfc, OUString& rURL, const OUString& rTargetURL )
{
    if( !rURL.matchIgnoreAsciiCase( sVndSunStarPackage ) )
        return true;

    try
    {
        OUString szPackagePath( encodeZipUri( rURL.copy( sVndSunStarPackage.getLength() ) ) );

        if ( ::comphelper::OStorageHelper::PathHasSegment( szPackagePath, ".." )
          || ::comphelper::OStorageHelper::PathHasSegment( szPackagePath, "." ) )
            throw lang::IllegalArgumentException();

        if( xIfc->hasByHierarchicalName( szPackagePath ) )
        {
            Reference< XActiveDataSink > xFileEntry;
            xIfc->getByHierarchicalName( szPackagePath ) >>= xFileEntry;

            if( xFileEntry.is() )
            {
                Reference< XInputStream > xIS( xFileEntry->getInputStream() );

                INetURLObject aBaseURL( rTargetURL );

                rURL = URIHelper::SmartRel2Abs( aBaseURL, szPackagePath, Link<OUString *, bool>(), false );

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
        OSL_FAIL( "XMLFilterJarHelper::copyFile exception caught" );
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
