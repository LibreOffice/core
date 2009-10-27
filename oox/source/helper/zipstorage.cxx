/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: zipstorage.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/helper/zipstorage.hxx"
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <comphelper/storagehelper.hxx>
#include "oox/helper/helper.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::embed::XStorage;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::XStream;

namespace oox {

// ============================================================================

ZipStorage::ZipStorage(
        const Reference< XMultiServiceFactory >& rxFactory,
        const Reference< XInputStream >& rxInStream ) :
    StorageBase( rxInStream, false )
{
    OSL_ENSURE( rxFactory.is(), "ZipStorage::ZipStorage - missing service factory" );
    // create base storage object
    try
    {
        /*  #i105325# ::comphelper::OStorageHelper::GetStorageFromInputStream()
            cannot be used here as it will open a storage with format type
            'PackageFormat' that will not work with OOXML packages.
            TODO: #i105410# switch to 'OFOPXMLFormat' and use its
            implementation of relations handling. */
        mxStorage = ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(
            ZIP_STORAGE_FORMAT_STRING, rxInStream, rxFactory );
    }
    catch( Exception& )
    {
    }
}

ZipStorage::ZipStorage(
        const Reference< XMultiServiceFactory >& rxFactory,
        const Reference< XStream >& rxStream ) :
    StorageBase( rxStream, false )
{
    OSL_ENSURE( rxFactory.is(), "ZipStorage::ZipStorage - missing service factory" );
    // create base storage object
    try
    {
        using namespace ::com::sun::star::embed::ElementModes;
        mxStorage = ::comphelper::OStorageHelper::GetStorageOfFormatFromStream(
            OFOPXML_STORAGE_FORMAT_STRING, rxStream, READWRITE | TRUNCATE, rxFactory );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "ZipStorage::ZipStorage - cannot open output storage" );
    }
}

ZipStorage::ZipStorage( const ZipStorage& rParentStorage, const Reference< XStorage >& rxStorage, const OUString& rElementName ) :
    StorageBase( rParentStorage, rElementName ),
    mxStorage( rxStorage )
{
    OSL_ENSURE( mxStorage.is(), "ZipStorage::ZipStorage - missing storage" );
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
    catch( Exception& )
    {
    }
}

StorageRef ZipStorage::implOpenSubStorage( const OUString& rElementName, bool bCreate )
{
    Reference< XStorage > xSubXStorage;
    bool bMissing = false;
    if( mxStorage.is() ) try
    {
        // XStorage::isStorageElement may throw various exceptions...
        if( mxStorage->isStorageElement( rElementName ) )
            xSubXStorage = mxStorage->openStorageElement(
                rElementName, ::com::sun::star::embed::ElementModes::READ );
    }
    catch( ::com::sun::star::container::NoSuchElementException& )
    {
        bMissing = true;
    }
    catch( Exception& )
    {
    }

    if( bMissing && bCreate )
        try
        {
            xSubXStorage = mxStorage->openStorageElement(
                rElementName, ::com::sun::star::embed::ElementModes::READWRITE );
        }
        catch( Exception& )
        {
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
        xInStream.set( mxStorage->openStreamElement( rElementName, ::com::sun::star::embed::ElementModes::READ ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    return xInStream;
}

Reference< XOutputStream > ZipStorage::implOpenOutputStream( const OUString& rElementName )
{
    Reference< XOutputStream > xOutStream;
    if( mxStorage.is() ) try
    {
        xOutStream.set( mxStorage->openStreamElement( rElementName, ::com::sun::star::embed::ElementModes::READWRITE ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    return xOutStream;
}

// ============================================================================

} // namespace oox

