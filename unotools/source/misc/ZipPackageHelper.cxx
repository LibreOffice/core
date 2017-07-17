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
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <unotools/ZipPackageHelper.hxx>
#include <comphelper/processfactory.hxx>
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

using namespace utl;
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

ZipPackageHelper::ZipPackageHelper(
    const Reference< XComponentContext >& rxContext,
    const OUString& sPackageURL)
: mxContext( rxContext ),
  mxHNameAccess(),
  mxFactory(),
  mxRootFolder()
{
    // create the package zip file
    Sequence< Any > aArguments( 2 );
    aArguments[ 0 ] <<= sPackageURL;

    // let ZipPackage be used
    beans::NamedValue aArg;
    aArg.Name = "StorageFormat";
    aArg.Value <<= OUString(ZIP_STORAGE_FORMAT_STRING);
    aArguments[ 1 ] <<= aArg;

    Reference< XHierarchicalNameAccess > xHNameAccess(
        mxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.packages.comp.ZipPackage",
            aArguments, mxContext ), UNO_QUERY);
    mxHNameAccess = xHNameAccess;

    if( !mxHNameAccess.is() )
        return;

    Reference<XSingleServiceFactory> xFactory(mxHNameAccess, UNO_QUERY);
    mxFactory = xFactory;

    // get root zip folder
    mxHNameAccess->getByHierarchicalName( "/" ) >>= mxRootFolder;
}

static OUString encodeZipUri( const OUString& rURI )
{
    return Uri::encode( rURI, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8 );
}

Reference< XInterface >& ZipPackageHelper::getRootFolder()
{
    return mxRootFolder;
}

Reference< XInterface > ZipPackageHelper::addFolder( Reference< XInterface > const & xRootFolder,
                                                     const OUString& rName )
{
    if ( rName == ".." || rName == "." )
        throw lang::IllegalArgumentException();

    Sequence< Any > aArgs(1);
    aArgs[0] <<= true;

    Reference< XInterface > xFolder( mxFactory->createInstanceWithArguments(aArgs) );
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

void ZipPackageHelper::addFolderWithContent( Reference< XInterface > const & xRootFolder, const OUString& rDirURL )
{
    if (rDirURL.isEmpty())
        return;

    osl::Directory aDirectory(rDirURL);

    if (aDirectory.open() != osl::FileBase::E_None)
        return;

    osl::DirectoryItem aDirectoryItem;

    while (osl::FileBase::E_None == aDirectory.getNextItem(aDirectoryItem))
    {
        osl::FileStatus aFileStatus(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL | osl_FileStatus_Mask_FileName);

        if (osl::FileBase::E_None == aDirectoryItem.getFileStatus(aFileStatus))
        {
            if (aFileStatus.isDirectory())
            {
                const OUString aFileName(aFileStatus.getFileName());

                if (!aFileName.isEmpty())
                {
                    Reference<XInterface> folder(addFolder(xRootFolder, aFileName));
                    addFolderWithContent(folder, aFileStatus.getFileURL());
                }
            }
            else if (aFileStatus.isRegular())
            {
                addFile(xRootFolder, aFileStatus.getFileURL());
            }
        }
    }
}

void ZipPackageHelper::addFile( css::uno::Reference< css::uno::XInterface > const & xRootFolder,
                                const OUString& rSourceFile )
{
    OUString aFileURL( rSourceFile );

    /*if( !aFileURL.matchIgnoreAsciiCase("file://") )
    {
        aFileURL = URIHelper::SmartRel2Abs( aFileURL, Link<OUString *, bool>(), false );
    }*/

    INetURLObject aURL( aFileURL );
    OUString aName( aURL.getName() );

    SvFileStream* pStream = new SvFileStream(aFileURL, StreamMode::READ );
    Reference< XInputStream > xInput(  new utl::OSeekableInputStreamWrapper( pStream, true ) );
    Reference< XActiveDataSink > xSink( mxFactory->createInstance(), UNO_QUERY );
    Reference< XUnoTunnel > xTunnel( xSink, UNO_QUERY );
    if( !xSink.is() || !xTunnel.is())
        return;

    Reference< XNameContainer > xNameContainer(xRootFolder, UNO_QUERY );
    xNameContainer->insertByName(encodeZipUri( aName ), makeAny(xTunnel));
    xSink->setInputStream( xInput );
}

void ZipPackageHelper::savePackage()
{
    Reference< XChangesBatch > xBatch( mxHNameAccess, UNO_QUERY );
    if( xBatch.is() )
        xBatch->commitChanges();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
