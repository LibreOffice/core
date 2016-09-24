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

#include "oox/helper/zipstorage.hxx"

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/diagnose.h>
#include <comphelper/storagehelper.hxx>
#include "oox/helper/helper.hxx"

namespace oox {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

ZipStorage::ZipStorage( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStream ) :
    StorageBase( rxInStream, false )
{
    OSL_ENSURE( rxContext.is(), "ZipStorage::ZipStorage - missing component context" );
    // create base storage object
    if( rxContext.is() ) try
    {
        /*  #i105325# ::comphelper::OStorageHelper::GetStorageFromInputStream()
            cannot be used here as it will open a storage with format type
            'PackageFormat' that will not work with OOXML packages.

            #161971# The MS-document storages should always be opened in repair
            mode to ignore the format errors and get so much info as possible.
            I hate this solution, but it seems to be the only consistent way to
            handle the MS documents.

            TODO: #i105410# switch to 'OFOPXMLFormat' and use its
            implementation of relations handling.
         */
        mxStorage = ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(
            ZIP_STORAGE_FORMAT_STRING, rxInStream, rxContext );
    }
    catch (Exception const& e)
    {
        SAL_WARN("oox.storage", "ZipStorage::ZipStorage "
                "exception opening input storage: " << e.Message);
    }
}

ZipStorage::ZipStorage( const Reference< XComponentContext >& rxContext, const Reference< XStream >& rxStream ) :
    StorageBase( rxStream, false )
{
    OSL_ENSURE( rxContext.is(), "ZipStorage::ZipStorage - missing component context" );
    // create base storage object
    if( rxContext.is() ) try
    {
        const sal_Int32 nOpenMode = ElementModes::READWRITE | ElementModes::TRUNCATE;
        mxStorage = ::comphelper::OStorageHelper::GetStorageOfFormatFromStream(
            OFOPXML_STORAGE_FORMAT_STRING, rxStream, nOpenMode, rxContext, true );
    }
    catch (Exception const& e)
    {
        SAL_WARN("oox.storage", "ZipStorage::ZipStorage "
                "exception opening output storage: " << e.Message);
    }
}

ZipStorage::ZipStorage( const ZipStorage& rParentStorage, const Reference< XStorage >& rxStorage, const OUString& rElementName ) :
    StorageBase( rParentStorage, rElementName, rParentStorage.isReadOnly() ),
    mxStorage( rxStorage )
{
    SAL_WARN_IF(!mxStorage.is(), "oox.storage", "ZipStorage::ZipStorage "
            " - missing storage" );
}

ZipStorage::~ZipStorage()
{
}

bool ZipStorage::implIsStorage() const
{
    return mxStorage.is();
}

Reference< XStorage > ZipStorage::implGetXStorage() const
{
    return mxStorage;
}

void ZipStorage::implGetElementNames( ::std::vector< OUString >& orElementNames ) const
{
    Sequence< OUString > aNames;
    if( mxStorage.is() ) try
    {
        aNames = mxStorage->getElementNames();
        if( aNames.getLength() > 0 )
            orElementNames.insert( orElementNames.end(), aNames.getConstArray(), aNames.getConstArray() + aNames.getLength() );
    }
    catch (Exception const& e)
    {
        SAL_INFO("oox.storage", "getElementNames: exception: " << e.Message);
    }
}

StorageRef ZipStorage::implOpenSubStorage( const OUString& rElementName, bool bCreateMissing )
{
    Reference< XStorage > xSubXStorage;
    bool bMissing = false;
    if( mxStorage.is() ) try
    {
        // XStorage::isStorageElement may throw various exceptions...
        if( mxStorage->isStorageElement( rElementName ) )
            xSubXStorage = mxStorage->openStorageElement(
                rElementName, css::embed::ElementModes::READ );
    }
    catch( NoSuchElementException& )
    {
        bMissing = true;
    }
    catch (Exception const& e)
    {
        SAL_INFO("oox.storage", "openStorageElement: exception: " << e.Message);
    }

    if( bMissing && bCreateMissing ) try
    {
        xSubXStorage = mxStorage->openStorageElement(
            rElementName, css::embed::ElementModes::READWRITE );
    }
    catch (Exception const& e)
    {
        SAL_INFO("oox.storage", "openStorageElement: exception: " << e.Message);
    }

    StorageRef xSubStorage;
    if( xSubXStorage.is() )
        xSubStorage.reset( new ZipStorage( *this, xSubXStorage, rElementName ) );
    return xSubStorage;
}

Reference< XInputStream > ZipStorage::implOpenInputStream( const OUString& rElementName )
{
    Reference< XInputStream > xInStream;
    if( mxStorage.is() ) try
    {
        xInStream.set( mxStorage->openStreamElement( rElementName, css::embed::ElementModes::READ ), UNO_QUERY );
    }
    catch (Exception const& e)
    {
        SAL_INFO("oox.storage", "openStreamElement: exception: " << e.Message);
    }
    return xInStream;
}

Reference< XOutputStream > ZipStorage::implOpenOutputStream( const OUString& rElementName )
{
    Reference< XOutputStream > xOutStream;
    if( mxStorage.is() ) try
    {
        xOutStream.set( mxStorage->openStreamElement( rElementName, css::embed::ElementModes::READWRITE ), UNO_QUERY );
    }
    catch (Exception const& e)
    {
        SAL_INFO("oox.storage", "openStreamElement: exception: " << e.Message);
    }
    return xOutStream;
}

void ZipStorage::implCommit() const
{
    try
    {
        Reference< XTransactedObject >( mxStorage, UNO_QUERY_THROW )->commit();
    }
    catch (Exception const& e)
    {
        SAL_WARN("oox.storage", "commit: exception: " << e.Message);
    }
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
