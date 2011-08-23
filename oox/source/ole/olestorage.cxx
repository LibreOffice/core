/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "oox/ole/olestorage.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase2.hxx>
#include "oox/helper/helper.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"

using ::rtl::OUString;
using ::com::sun::star::container::XNameContainer;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::embed::XStorage;
using ::com::sun::star::embed::XTransactedObject;
using ::com::sun::star::io::BufferSizeExceededException;
using ::com::sun::star::io::IOException;
using ::com::sun::star::io::NotConnectedException;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::XSeekable;
using ::com::sun::star::io::XStream;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;

namespace oox {
namespace ole {

// ============================================================================

namespace {

typedef ::cppu::WeakImplHelper2< XSeekable, XOutputStream > OleOutputStreamBase;
    
/** Implementation of an OLE storage output stream that inserts itself into the
    storage when it is closed.
 */
class OleOutputStream : public OleOutputStreamBase
{
public:
    explicit            OleOutputStream(
                            const Reference< XMultiServiceFactory >& rxFactory,
                            const Reference< XNameContainer >& rxStorage,
                            const OUString& rElementName );
    virtual             ~OleOutputStream();

    virtual void SAL_CALL seek( sal_Int64 nPos ) throw( IllegalArgumentException, IOException, RuntimeException );
    virtual sal_Int64 SAL_CALL getPosition() throw( IOException, RuntimeException );
    virtual sal_Int64 SAL_CALL getLength() throw( IOException, RuntimeException );

    virtual void SAL_CALL writeBytes( const Sequence< sal_Int8 >& rData ) throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException );
    virtual void SAL_CALL flush() throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException );
    virtual void SAL_CALL closeOutput() throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException );

private:
    void                ensureSeekable() const throw( IOException );
    void                ensureConnected() const throw( NotConnectedException );

private:
    Reference< XNameContainer > mxStorage;
    Reference< XStream > mxTempFile;
    Reference< XOutputStream > mxOutStrm;
    Reference< XSeekable > mxSeekable;
    OUString            maElementName;
};

// ----------------------------------------------------------------------------

OleOutputStream::OleOutputStream( const Reference< XMultiServiceFactory >& rxFactory,
        const Reference< XNameContainer >& rxStorage, const OUString& rElementName ) :
    mxStorage( rxStorage ),
    maElementName( rElementName )
{
    try
    {
        mxTempFile.set( rxFactory->createInstance( CREATE_OUSTRING( "com.sun.star.io.TempFile" ) ), UNO_QUERY_THROW );
        mxOutStrm = mxTempFile->getOutputStream();
        mxSeekable.set( mxOutStrm, UNO_QUERY );
    }
    catch( Exception& )
    {
    }
}

OleOutputStream::~OleOutputStream()
{
}

void SAL_CALL OleOutputStream::seek( sal_Int64 nPos ) throw( IllegalArgumentException, IOException, RuntimeException )
{
    ensureSeekable();
    mxSeekable->seek( nPos );
}

sal_Int64 SAL_CALL OleOutputStream::getPosition() throw( IOException, RuntimeException )
{
    ensureSeekable();
    return mxSeekable->getPosition();
}

sal_Int64 SAL_CALL OleOutputStream::getLength() throw( IOException, RuntimeException )
{
    ensureSeekable();
    return mxSeekable->getLength();
}

void SAL_CALL OleOutputStream::writeBytes( const Sequence< sal_Int8 >& rData ) throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
{
    ensureConnected();
    mxOutStrm->writeBytes( rData );
}

void SAL_CALL OleOutputStream::flush() throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
{
    ensureConnected();
    mxOutStrm->flush();
}

void SAL_CALL OleOutputStream::closeOutput() throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
{
    ensureConnected();
    ensureSeekable();
    // remember the class members
    Reference< XOutputStream > xOutStrm = mxOutStrm;
    Reference< XSeekable > xSeekable = mxSeekable;
    // reset all class members
    mxOutStrm.clear();
    mxSeekable.clear();
    // close stream (and let it throw something if needed)
    xOutStrm->closeOutput();
    // on success, insert the stream into the OLE storage (must be seeked back before)
    xSeekable->seek( 0 );
    if( !ContainerHelper::insertByName( mxStorage, maElementName, Any( mxTempFile ) ) )
        throw IOException();
}

void OleOutputStream::ensureSeekable() const throw( IOException )
{
    if( !mxSeekable.is() )
        throw IOException();
}

void OleOutputStream::ensureConnected() const throw( NotConnectedException )
{
    if( !mxOutStrm.is() )
        throw NotConnectedException();
}

} // namespace

// ============================================================================

OleStorage::OleStorage(
        const Reference< XMultiServiceFactory >& rxFactory,
        const Reference< XInputStream >& rxInStream,
        bool bBaseStreamAccess ) :
    StorageBase( rxInStream, bBaseStreamAccess ),
    mxFactory( rxFactory ),
    mpParentStorage( 0 )
{
    OSL_ENSURE( mxFactory.is(), "OleStorage::OleStorage - missing service factory" );
    initStorage( rxInStream );
}

OleStorage::OleStorage(
        const Reference< XMultiServiceFactory >& rxFactory,
        const Reference< XStream >& rxOutStream,
        bool bBaseStreamAccess ) :
    StorageBase( rxOutStream, bBaseStreamAccess ),
    mxFactory( rxFactory ),
    mpParentStorage( 0 )
{
    OSL_ENSURE( mxFactory.is(), "OleStorage::OleStorage - missing service factory" );
    initStorage( rxOutStream );
}

OleStorage::OleStorage(
        const OleStorage& rParentStorage,
        const Reference< XNameContainer >& rxStorage,
        const OUString& rElementName,
        bool bReadOnly ) :
    StorageBase( rParentStorage, rElementName, bReadOnly ),
    mxFactory( rParentStorage.mxFactory ),
    mxStorage( rxStorage ),
    mpParentStorage( &rParentStorage )
{
    OSL_ENSURE( mxStorage.is(), "OleStorage::OleStorage - missing substorage elements" );
}

OleStorage::OleStorage(
        const OleStorage& rParentStorage,
        const Reference< XStream >& rxOutStream,
        const OUString& rElementName ) :
    StorageBase( rParentStorage, rElementName, false ),
    mxFactory( rParentStorage.mxFactory ),
    mpParentStorage( &rParentStorage )
{
    initStorage( rxOutStream );
}

OleStorage::~OleStorage()
{
}

// ----------------------------------------------------------------------------

void OleStorage::initStorage( const Reference< XInputStream >& rxInStream )
{
    // if stream is not seekable, create temporary copy
    Reference< XInputStream > xInStrm = rxInStream;
    if( !Reference< XSeekable >( xInStrm, UNO_QUERY ).is() ) try
    {
        Reference< XStream > xTempFile( mxFactory->createInstance( CREATE_OUSTRING( "com.sun.star.io.TempFile" ) ), UNO_QUERY_THROW );
        {
            Reference< XOutputStream > xOutStrm( xTempFile->getOutputStream(), UNO_SET_THROW );
            /*  Pass false to both binary stream objects to keep the UNO
                streams alive. Life time of these streams is controlled by the
                tempfile implementation. */
            BinaryXOutputStream aOutStrm( xOutStrm, false );
            BinaryXInputStream aInStrm( xInStrm, false );
            aInStrm.copyToStream( aOutStrm );
        } // scope closes output stream of tempfile
        xInStrm = xTempFile->getInputStream();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "OleStorage::initStorage - cannot create temporary copy of input stream" );
    }

    // create base storage object
    if( xInStrm.is() ) try
    {
        Sequence< Any > aArgs( 2 );
        aArgs[ 0 ] <<= xInStrm;
        aArgs[ 1 ] <<= true;        // true = do not create a copy of the input stream
        mxStorage.set( mxFactory->createInstanceWithArguments(
            CREATE_OUSTRING( "com.sun.star.embed.OLESimpleStorage" ), aArgs ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
}

void OleStorage::initStorage( const Reference< XStream >& rxOutStream )
{
    // create base storage object
    if( rxOutStream.is() ) try
    {
        Sequence< Any > aArgs( 2 );
        aArgs[ 0 ] <<= rxOutStream;
        aArgs[ 1 ] <<= true;        // true = do not create a copy of the stream
        mxStorage.set( mxFactory->createInstanceWithArguments(
            CREATE_OUSTRING( "com.sun.star.embed.OLESimpleStorage" ), aArgs ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
}

// StorageBase interface ------------------------------------------------------

bool OleStorage::implIsStorage() const
{
    if( mxStorage.is() ) try
    {
        /*  If this is not an OLE storage, hasElements() of the OLESimpleStorage
            implementation throws an exception. But we do not return the result
            of hasElements(), because an empty storage is a valid storage too. */
        mxStorage->hasElements();
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
        aNames = mxStorage->getElementNames();
        if( aNames.getLength() > 0 )
            orElementNames.insert( orElementNames.end(), aNames.getConstArray(), aNames.getConstArray() + aNames.getLength() );
    }
    catch( Exception& )
    {
    }
}

StorageRef OleStorage::implOpenSubStorage( const OUString& rElementName, bool bCreateMissing )
{
    StorageRef xSubStorage;
    if( mxStorage.is() && (rElementName.getLength() > 0) )
    {
        try
        {
            Reference< XNameContainer > xSubElements( mxStorage->getByName( rElementName ), UNO_QUERY_THROW );
            xSubStorage.reset( new OleStorage( *this, xSubElements, rElementName, true ) );
        }
        catch( Exception& )
        {
        }

        /*  The OLESimpleStorage API implementation seems to be buggy in the
            area of writable inplace substorage (sometimes it overwrites other
            unrelated streams with zero bytes). We go the save way and create a
            new OLE storage based on a temporary file. All operations are
            performed on this clean storage. On committing, the storage will be
            completely re-inserted into the parent storage. */
        if( !isReadOnly() && (bCreateMissing || xSubStorage.get()) ) try
        {
            // create new storage based on a temp file
            Reference< XStream > xTempFile( mxFactory->createInstance( CREATE_OUSTRING( "com.sun.star.io.TempFile" ) ), UNO_QUERY_THROW );
            StorageRef xTempStorage( new OleStorage( *this, xTempFile, rElementName ) );
            // copy existing substorage into temp storage
            if( xSubStorage.get() )
                xSubStorage->copyStorageToStorage( *xTempStorage );
            // return the temp storage to caller
            xSubStorage = xTempStorage;
        }
        catch( Exception& )
        {
        }
    }
    return xSubStorage;
}

Reference< XInputStream > OleStorage::implOpenInputStream( const OUString& rElementName )
{
    Reference< XInputStream > xInStream;
    if( mxStorage.is() ) try
    {
        xInStream.set( mxStorage->getByName( rElementName ), UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    return xInStream;
}

Reference< XOutputStream > OleStorage::implOpenOutputStream( const OUString& rElementName )
{
    Reference< XOutputStream > xOutStream;
    if( mxStorage.is() && (rElementName.getLength() > 0) )
        xOutStream.set( new OleOutputStream( mxFactory, mxStorage, rElementName ) );
    return xOutStream;
}

void OleStorage::implCommit() const
{
    try
    {
        // commit this storage (finalizes the file this storage is based on)
        Reference< XTransactedObject >( mxStorage, UNO_QUERY_THROW )->commit();
        // re-insert this storage into the parent storage
        if( mpParentStorage )
        {
            if( mpParentStorage->mxStorage->hasByName( getName() ) )
            {
                // replaceByName() does not work (#i109539#)
                mpParentStorage->mxStorage->removeByName( getName() );
                Reference< XTransactedObject >( mpParentStorage->mxStorage, UNO_QUERY_THROW )->commit();
            }
            mpParentStorage->mxStorage->insertByName( getName(), Any( mxStorage ) );
            // this requires another commit(), which will be performed by the parent storage
        }
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
