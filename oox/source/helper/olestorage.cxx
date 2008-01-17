/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: olestorage.cxx,v $
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

#include "oox/helper/olestorage.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include "oox/helper/helper.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::embed::XStorage;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;

namespace oox {

// ============================================================================

OleStorage::OleStorage(
        const Reference< XMultiServiceFactory >& rxFactory,
        const Reference< XInputStream >& rxInStream,
        bool bBaseStreamAccess ) :
    StorageBase( rxInStream, bBaseStreamAccess )
{
    OSL_ENSURE( rxFactory.is(), "OleStorage::OleStorage - missing service factory" );
    // create base storage object
    Sequence< Any > aArgs( 2 );
    aArgs[ 0 ] <<= rxInStream;
    aArgs[ 1 ] <<= true;        // true = do not create a copy of the input stream
    mxStorage.set( rxFactory->createInstanceWithArguments(
        CREATE_OUSTRING( "com.sun.star.embed.OLESimpleStorage" ), aArgs ), UNO_QUERY );
    mxElements.set( mxStorage, UNO_QUERY );
}

OleStorage::OleStorage(
        const Reference< XMultiServiceFactory >& rxFactory,
        const Reference< XOutputStream >& rxOutStream,
        bool bBaseStreamAccess ) :
    StorageBase( rxOutStream, bBaseStreamAccess )
{
    OSL_ENSURE( rxFactory.is(), "OleStorage::OleStorage - missing service factory" );
    (void)rxFactory;    // prevent compiler warning
    OSL_ENSURE( false, "OleStorage::OleStorage - not implemented" );
    mxElements.set( mxStorage, UNO_QUERY );
}

OleStorage::OleStorage( const OleStorage& rParentStorage, const Reference< XNameAccess >& rxElementsAccess, const OUString& rElementName ) :
    StorageBase( rParentStorage, rElementName ),
    mxStorage( rParentStorage.mxStorage ),
    mxElements( rxElementsAccess )
{
    OSL_ENSURE( mxElements.is(), "OleStorage::OleStorage - missing elements access" );
}

OleStorage::~OleStorage()
{
}

// StorageBase interface ------------------------------------------------------

bool OleStorage::implIsStorage() const
{
    if( mxStorage.is() && mxElements.is() ) try
    {
        /*  If this is not a storage, hasElements() throws an exception. But we
            do not return the result of hasElements(), because an empty storage
            is a valid storage too. */
        mxElements->hasElements();
        return true;
    }
    catch( Exception& )
    {
    }
    return false;
}

Reference< XStorage > OleStorage::implGetXStorage() const
{
    OSL_ENSURE( false, "OleStorage::getXStorage - not implemented" );
    return Reference< XStorage >();
}

void OleStorage::implGetElementNames( ::std::vector< OUString >& orElementNames ) const
{
    Sequence< OUString > aNames;
    if( mxStorage.is() ) try
    {
        aNames = mxElements->getElementNames();
        if( aNames.getLength() > 0 )
            orElementNames.insert( orElementNames.end(), aNames.getConstArray(), aNames.getConstArray() + aNames.getLength() );
    }
    catch( Exception& )
    {
    }
}

StorageRef OleStorage::implOpenSubStorage( const OUString& rElementName, bool bCreate )
{
    OSL_ENSURE( !bCreate, "OleStorage::implOpenSubStorage - creating substorages not implemented" );
    (void)bCreate;  // prevent compiler warning
    StorageRef xSubStorage;
    if( mxElements.is() ) try
    {
        Reference< XNameAccess > xSubElements( mxElements->getByName( rElementName ), UNO_QUERY_THROW );
        xSubStorage.reset( new OleStorage( *this, xSubElements, rElementName ) );
    }
    catch( Exception& )
    {
    }
    return xSubStorage;
}

Reference< XInputStream > OleStorage::implOpenInputStream( const OUString& rElementName )
{
    Reference< XInputStream > xInStream;
    if( mxElements.is() ) try
    {
        xInStream.set( mxElements->getByName( rElementName ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    return xInStream;
}

Reference< XOutputStream > OleStorage::implOpenOutputStream( const OUString& rElementName )
{
    Reference< XOutputStream > xOutStream;
    if( mxElements.is() && (rElementName.getLength() > 0) ) try
    {
        (void)rElementName;     // prevent compiler warning
        OSL_ENSURE( false, "OleStorage::implOpenOutputStream - not implemented" );
    }
    catch( Exception& )
    {
    }
    return xOutStream;
}

// ============================================================================

} // namespace oox

