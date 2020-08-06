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

#include <oox/helper/storagebase.hxx>

#include <com/sun/star/io/XStream.hpp>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>

namespace oox {

using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

namespace {

void lclSplitFirstElement( OUString& orElement, OUString& orRemainder, const OUString& _aFullName )
{
    OUString aFullName = _aFullName;
    sal_Int32  nSlashPos = aFullName.indexOf( '/' );

    // strip leading slashes
    while( nSlashPos == 0 )
    {
        aFullName = aFullName.copy(1);
        nSlashPos = aFullName.indexOf( '/' );
    }

    if( (0 <= nSlashPos) && (nSlashPos < aFullName.getLength()) )
    {
        orElement = aFullName.copy( 0, nSlashPos );
        orRemainder = aFullName.copy( nSlashPos + 1 );
    }
    else
    {
        orElement = aFullName;
    }
}

} // namespace

StorageBase::StorageBase( const Reference< XInputStream >& rxInStream, bool bBaseStreamAccess ) :
    mxInStream( rxInStream ),
    mbBaseStreamAccess( bBaseStreamAccess ),
    mbReadOnly( true )
{
    OSL_ENSURE( mxInStream.is(), "StorageBase::StorageBase - missing base input stream" );
}

StorageBase::StorageBase( const Reference< XStream >& rxOutStream, bool bBaseStreamAccess ) :
    mxOutStream( rxOutStream ),
    mbBaseStreamAccess( bBaseStreamAccess ),
    mbReadOnly( false )
{
    OSL_ENSURE( mxOutStream.is(), "StorageBase::StorageBase - missing base output stream" );
}

StorageBase::StorageBase( const StorageBase& rParentStorage, const OUString& rStorageName, bool bReadOnly ) :
    maParentPath( rParentStorage.getPath() ),
    maStorageName( rStorageName ),
    mbBaseStreamAccess( false ),
    mbReadOnly( bReadOnly )
{
}

StorageBase::~StorageBase()
{
}

bool StorageBase::isStorage() const
{
    return implIsStorage();
}

bool StorageBase::isRootStorage() const
{
    return implIsStorage() && maStorageName.isEmpty();
}

Reference< XStorage > StorageBase::getXStorage() const
{
    return implGetXStorage();
}

OUString StorageBase::getPath() const
{
    OUStringBuffer aBuffer( maParentPath );
    if( !aBuffer.isEmpty() )
        aBuffer.append( '/' );
    aBuffer.append( maStorageName );
    return aBuffer.makeStringAndClear();
}

void StorageBase::getElementNames( ::std::vector< OUString >& orElementNames ) const
{
    orElementNames.clear();
    implGetElementNames( orElementNames );
}

StorageRef StorageBase::openSubStorage( const OUString& rStorageName, bool bCreateMissing )
{
    StorageRef xSubStorage;
    OSL_ENSURE( !bCreateMissing || !mbReadOnly, "StorageBase::openSubStorage - cannot create substorage in read-only mode" );
    if( !bCreateMissing || !mbReadOnly )
    {
        OUString aElement, aRemainder;
        lclSplitFirstElement( aElement, aRemainder, rStorageName );
        if( !aElement.isEmpty() )
            xSubStorage = getSubStorage( aElement, bCreateMissing );
        if( xSubStorage && !aRemainder.isEmpty() )
            xSubStorage = xSubStorage->openSubStorage( aRemainder, bCreateMissing );
    }
    return xSubStorage;
}

Reference< XInputStream > StorageBase::openInputStream( const OUString& rStreamName )
{
    Reference< XInputStream > xInStream;
    OUString aElement, aRemainder;
    lclSplitFirstElement( aElement, aRemainder, rStreamName );
    if( !aElement.isEmpty() )
    {
        if( !aRemainder.isEmpty() )
        {
            StorageRef xSubStorage = getSubStorage( aElement, false );
            if( xSubStorage )
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
    OSL_ENSURE( !mbReadOnly, "StorageBase::openOutputStream - cannot create output stream in read-only mode" );
    if( !mbReadOnly )
    {
        OUString aElement, aRemainder;
        lclSplitFirstElement( aElement, aRemainder, rStreamName );
        if( !aElement.isEmpty() )
        {
            if( !aRemainder.isEmpty() )
            {
                StorageRef xSubStorage = getSubStorage( aElement, true );
                if( xSubStorage )
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
    }
    return xOutStream;
}

void StorageBase::copyToStorage( StorageBase& rDestStrg, const OUString& rElementName )
{
    OSL_ENSURE( rDestStrg.isStorage() && !rDestStrg.isReadOnly(), "StorageBase::copyToStorage - invalid destination" );
    OSL_ENSURE( !rElementName.isEmpty(), "StorageBase::copyToStorage - invalid element name" );
    if( !rDestStrg.isStorage() || rDestStrg.isReadOnly() || rElementName.isEmpty() )
        return;

    StorageRef xSubStrg = openSubStorage( rElementName, false );
    if( xSubStrg )
    {
        StorageRef xDestSubStrg = rDestStrg.openSubStorage( rElementName, true );
        if( xDestSubStrg )
            xSubStrg->copyStorageToStorage( *xDestSubStrg );
    }
    else
    {
        Reference< XInputStream > xInStrm = openInputStream( rElementName );
        if( xInStrm.is() )
        {
            Reference< XOutputStream > xOutStrm = rDestStrg.openOutputStream( rElementName );
            if( xOutStrm.is() )
            {
                BinaryXInputStream aInStrm( xInStrm, true );
                BinaryXOutputStream aOutStrm( xOutStrm, true );
                aInStrm.copyToStream( aOutStrm );
            }
        }
    }
}

void StorageBase::copyStorageToStorage( StorageBase& rDestStrg )
{
    OSL_ENSURE( rDestStrg.isStorage() && !rDestStrg.isReadOnly(), "StorageBase::copyToStorage - invalid destination" );
    if( rDestStrg.isStorage() && !rDestStrg.isReadOnly() )
    {
        ::std::vector< OUString > aElements;
        getElementNames( aElements );
        for (auto const& elem : aElements)
            copyToStorage(rDestStrg, elem);
    }
}

void StorageBase::commit()
{
    OSL_ENSURE( !mbReadOnly, "StorageBase::commit - cannot commit in read-only mode" );
    if( !mbReadOnly )
    {
        // commit all open substorages
        maSubStorages.forEachMem( &StorageBase::commit );
        // commit this storage
        implCommit();
    }
}

// private --------------------------------------------------------------------

StorageRef StorageBase::getSubStorage( const OUString& rElementName, bool bCreateMissing )
{
    StorageRef& rxSubStrg = maSubStorages[ rElementName ];
    if( !rxSubStrg )
        rxSubStrg = implOpenSubStorage( rElementName, bCreateMissing );
    return rxSubStrg;
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
