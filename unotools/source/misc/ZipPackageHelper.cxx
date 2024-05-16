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

#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <unotools/ZipPackageHelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <osl/file.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>

#include <rtl/uri.hxx>

namespace com::sun::star::io { class XInputStream; }

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
: mxContext( rxContext )
{
    // create the package zip file
    Sequence< Any > aArguments{
        Any(sPackageURL),
        // let ZipPackage be used
        Any(beans::NamedValue(u"StorageFormat"_ustr, Any(ZIP_STORAGE_FORMAT_STRING)))
    };

    mxHNameAccess.set(
        mxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            u"com.sun.star.packages.comp.ZipPackage"_ustr,
            aArguments, mxContext ), UNO_QUERY);

    if( !mxHNameAccess.is() )
        return;

    mxFactory.set(mxHNameAccess, UNO_QUERY);

    // get root zip folder
    mxHNameAccess->getByHierarchicalName( u"/"_ustr ) >>= mxRootFolder;
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

    Reference< XInterface > xFolder( mxFactory->createInstanceWithArguments({ Any(true) } ));
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
                                const OUString& rSourceFileURL )
{
    INetURLObject aURL( rSourceFileURL );
    OUString aName( aURL.getName() );

    SvFileStream* pStream = new SvFileStream(rSourceFileURL, StreamMode::READ );
    Reference< XInputStream > xInput(  new utl::OSeekableInputStreamWrapper( pStream, true ) );
    Reference< XActiveDataSink > xSink( mxFactory->createInstance(), UNO_QUERY );
    assert(xSink); // this should never fail
    if( !xSink.is() )
        return;

    Reference< XNameContainer > xNameContainer(xRootFolder, UNO_QUERY );
    xNameContainer->insertByName(encodeZipUri( aName ), Any(xSink));
    xSink->setInputStream( xInput );
}

void ZipPackageHelper::savePackage()
{
    Reference< XChangesBatch > xBatch( mxHNameAccess, UNO_QUERY );
    if( xBatch.is() )
        xBatch->commitChanges();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
