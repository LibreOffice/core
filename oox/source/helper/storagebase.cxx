/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: storagebase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/helper/storagebase.hxx"
#include <rtl/ustrbuf.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::embed::XStorage;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;

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

StorageBase::StorageBase( const Reference< XOutputStream >& rxOutStream, bool bBaseStreamAccess ) :
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
        xOutStream = mxOutStream;
    }
    return xOutStream;
}

StorageRef StorageBase::getSubStorage( const OUString& rElementName, bool bCreate )
{
    SubStorageMap::iterator aIt = maSubStorages.find( rElementName );
    return (aIt == maSubStorages.end()) ?
        (maSubStorages[ rElementName ] = implOpenSubStorage( rElementName, bCreate )) : aIt->second;
}

// ============================================================================

} // namespace oox

