/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "oox/helper/storagebase.hxx"

#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"

namespace oox {

// ============================================================================

using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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
    return implIsStorage() && (maStorageName.getLength() == 0);
}

bool StorageBase::isReadOnly() const
{
    return mbReadOnly;
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
    OUStringBuffer aBuffer( maParentPath );
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

StorageRef StorageBase::openSubStorage( const OUString& rStorageName, bool bCreateMissing )
{
    StorageRef xSubStorage;
    OSL_ENSURE( !bCreateMissing || !mbReadOnly, "StorageBase::openSubStorage - cannot create substorage in read-only mode" );
    if( !bCreateMissing || !mbReadOnly )
    {
        OUString aElement, aRemainder;
        lclSplitFirstElement( aElement, aRemainder, rStorageName );
        if( aElement.getLength() > 0 )
            xSubStorage = getSubStorage( aElement, bCreateMissing );
        if( xSubStorage.get() && (aRemainder.getLength() > 0) )
            xSubStorage = xSubStorage->openSubStorage( aRemainder, bCreateMissing );
    }
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
    OSL_ENSURE( !mbReadOnly, "StorageBase::openOutputStream - cannot create output stream in read-only mode" );
    if( !mbReadOnly )
    {
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
    }
    return xOutStream;
}

void StorageBase::copyToStorage( StorageBase& rDestStrg, const OUString& rElementName )
{
    OSL_ENSURE( rDestStrg.isStorage() && !rDestStrg.isReadOnly(), "StorageBase::copyToStorage - invalid destination" );
    OSL_ENSURE( rElementName.getLength() > 0, "StorageBase::copyToStorage - invalid element name" );
    if( rDestStrg.isStorage() && !rDestStrg.isReadOnly() && (rElementName.getLength() > 0) )
    {
        StorageRef xSubStrg = openSubStorage( rElementName, false );
        if( xSubStrg.get() )
        {
            StorageRef xDestSubStrg = rDestStrg.openSubStorage( rElementName, true );
            if( xDestSubStrg.get() )
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
}

void StorageBase::copyStorageToStorage( StorageBase& rDestStrg )
{
    OSL_ENSURE( rDestStrg.isStorage() && !rDestStrg.isReadOnly(), "StorageBase::copyToStorage - invalid destination" );
    if( rDestStrg.isStorage() && !rDestStrg.isReadOnly() )
    {
        ::std::vector< OUString > aElements;
        getElementNames( aElements );
        for( ::std::vector< OUString >::iterator aIt = aElements.begin(), aEnd = aElements.end(); aIt != aEnd; ++aIt )
            copyToStorage( rDestStrg, *aIt );
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

// ============================================================================

} // namespace oox
