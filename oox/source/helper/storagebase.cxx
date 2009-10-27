/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: storagebase.cxx,v $
 * $Revision: 1.3 $
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

#include "oox/helper/storagebase.hxx"
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <rtl/ustrbuf.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::embed::XStorage;
using ::com::sun::star::embed::XTransactedObject;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::XStream;

namespace oox {

// ============================================================================

namespace {

void lclSplitFirstElement( OUString& orElement, OUString& orRemainder, const OUString& rFullName )
{
    sal_Int32 nSlashPos = rFullName.indexOf( '/' );
    if( (0 <= nSlashPos) && (nSlashPos < rFullName.getLength()) )
    {
        orElement = rFullName.copy( 0, nSlashPos );
        orRemainder = rFullName.copy( nSlashPos + 1 );
    }
    else
    {
        orElement = rFullName;
    }
}

} // namespace

// ----------------------------------------------------------------------------

StorageBase::StorageBase( const Reference< XInputStream >& rxInStream, bool bBaseStreamAccess ) :
    mxInStream( rxInStream ),
    mpParentStorage( 0 ),
    mbBaseStreamAccess( bBaseStreamAccess )
{
    OSL_ENSURE( mxInStream.is(), "StorageBase::StorageBase - missing base input stream" );
}

StorageBase::StorageBase( const Reference< XStream >& rxOutStream, bool bBaseStreamAccess ) :
    mxOutStream( rxOutStream ),
    mpParentStorage( 0 ),
    mbBaseStreamAccess( bBaseStreamAccess )
{
    OSL_ENSURE( mxOutStream.is(), "StorageBase::StorageBase - missing base output stream" );
}

StorageBase::StorageBase( const StorageBase& rParentStorage, const OUString& rStorageName ) :
    maStorageName( rStorageName ),
    mpParentStorage( &rParentStorage ),
    mbBaseStreamAccess( false )
{
}

StorageBase::~StorageBase()
{
}

bool StorageBase::isStorage() const
{
    return implIsStorage();
}

Reference< XStorage > StorageBase::getXStorage() const
{
    return implGetXStorage();
}

const OUString& StorageBase::getName() const
{
    return maStorageName;
}

OUString StorageBase::getPath() const
{
    OUStringBuffer aBuffer;
    if( mpParentStorage )
        aBuffer.append( mpParentStorage->getPath() );
    if( aBuffer.getLength() > 0 )
        aBuffer.append( sal_Unicode( '/' ) );
    aBuffer.append( maStorageName );
    return aBuffer.makeStringAndClear();
}

void StorageBase::getElementNames( ::std::vector< OUString >& orElementNames ) const
{
    orElementNames.clear();
    implGetElementNames( orElementNames );
}

StorageRef StorageBase::openSubStorage( const OUString& rStorageName, bool bCreate )
{
    StorageRef xSubStorage;
    OUString aElement, aRemainder;
    lclSplitFirstElement( aElement, aRemainder, rStorageName );
    if( aElement.getLength() > 0 )
        xSubStorage = getSubStorage( aElement, bCreate );
    if( xSubStorage.get() && (aRemainder.getLength() > 0) )
        xSubStorage = xSubStorage->openSubStorage( aRemainder, bCreate );
    return xSubStorage;
}

Reference< XInputStream > StorageBase::openInputStream( const OUString& rStreamName )
{
    Reference< XInputStream > xInStream;
    OUString aElement, aRemainder;
    lclSplitFirstElement( aElement, aRemainder, rStreamName );
    if( aElement.getLength() > 0 )
    {
        if( aRemainder.getLength() > 0 )
        {
            StorageRef xSubStorage = getSubStorage( aElement, false );
            if( xSubStorage.get() )
                xInStream = xSubStorage->openInputStream( aRemainder );
        }
        else
        {
            xInStream = implOpenInputStream( aElement );
        }
    }
    else if( mbBaseStreamAccess )
    {
        xInStream = mxInStream;
    }
    return xInStream;
}

Reference< XOutputStream > StorageBase::openOutputStream( const OUString& rStreamName )
{
    Reference< XOutputStream > xOutStream;
    OUString aElement, aRemainder;
    lclSplitFirstElement( aElement, aRemainder, rStreamName );
    if( aElement.getLength() > 0 )
    {
        if( aRemainder.getLength() > 0 )
        {
            StorageRef xSubStorage = getSubStorage( aElement, true );
            if( xSubStorage.get() )
                xOutStream = xSubStorage->openOutputStream( aRemainder );
        }
        else
        {
            xOutStream = implOpenOutputStream( aElement );
        }
    }
    else if( mbBaseStreamAccess )
    {
        xOutStream = mxOutStream->getOutputStream();
    }
    return xOutStream;
}

StorageRef StorageBase::getSubStorage( const OUString& rElementName, bool bCreate )
{
    SubStorageMap::iterator aIt = maSubStorages.find( rElementName );
    return (aIt == maSubStorages.end()) ?
        (maSubStorages[ rElementName ] = implOpenSubStorage( rElementName, bCreate )) : aIt->second;
}

void StorageBase::commit()
{
    for( SubStorageMap::iterator aIt = maSubStorages.begin(); aIt != maSubStorages.end(); aIt ++ )
        aIt->second->commit();

    Reference< XTransactedObject > xTransactedObj( getXStorage(), UNO_QUERY );

    if( xTransactedObj.is() )
        xTransactedObj->commit();
}

// ============================================================================

} // namespace oox

