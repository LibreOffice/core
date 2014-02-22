/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "oox/ole/olestorage.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/helper.hxx"

namespace oox {
namespace ole {



using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;



namespace {

typedef ::cppu::WeakImplHelper2< XSeekable, XOutputStream > OleOutputStreamBase;

/** Implementation of an OLE storage output stream that inserts itself into the
    storage when it is closed.
 */
class OleOutputStream : public OleOutputStreamBase
{
public:
    explicit            OleOutputStream(
                            const Reference< XComponentContext >& rxContext,
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



OleOutputStream::OleOutputStream( const Reference< XComponentContext >& rxContext,
        const Reference< XNameContainer >& rxStorage, const OUString& rElementName ) :
    mxStorage( rxStorage ),
    maElementName( rElementName )
{
    try
    {
        mxTempFile.set( TempFile::create(rxContext), UNO_QUERY_THROW );
        mxOutStrm = mxTempFile->getOutputStream();
        mxSeekable.set( mxOutStrm, UNO_QUERY );
    }
    catch(const Exception& )
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
    
    Reference< XOutputStream > xOutStrm = mxOutStrm;
    Reference< XSeekable > xSeekable = mxSeekable;
    
    mxOutStrm.clear();
    mxSeekable.clear();
    
    xOutStrm->closeOutput();
    
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

} 



OleStorage::OleStorage( const Reference< XComponentContext >& rxContext,
        const Reference< XInputStream >& rxInStream, bool bBaseStreamAccess ) :
    StorageBase( rxInStream, bBaseStreamAccess ),
    mxContext( rxContext ),
    mpParentStorage( 0 )
{
    OSL_ENSURE( mxContext.is(), "OleStorage::OleStorage - missing component context" );
    initStorage( rxInStream );
}

OleStorage::OleStorage( const Reference< XComponentContext >& rxContext,
        const Reference< XStream >& rxOutStream, bool bBaseStreamAccess ) :
    StorageBase( rxOutStream, bBaseStreamAccess ),
    mxContext( rxContext ),
    mpParentStorage( 0 )
{
    OSL_ENSURE( mxContext.is(), "OleStorage::OleStorage - missing component context" );
    initStorage( rxOutStream );
}

OleStorage::OleStorage( const OleStorage& rParentStorage,
        const Reference< XNameContainer >& rxStorage, const OUString& rElementName, bool bReadOnly ) :
    StorageBase( rParentStorage, rElementName, bReadOnly ),
    mxContext( rParentStorage.mxContext ),
    mxStorage( rxStorage ),
    mpParentStorage( &rParentStorage )
{
    OSL_ENSURE( mxStorage.is(), "OleStorage::OleStorage - missing substorage elements" );
}

OleStorage::OleStorage( const OleStorage& rParentStorage,
        const Reference< XStream >& rxOutStream, const OUString& rElementName ) :
    StorageBase( rParentStorage, rElementName, false ),
    mxContext( rParentStorage.mxContext ),
    mpParentStorage( &rParentStorage )
{
    initStorage( rxOutStream );
}

OleStorage::~OleStorage()
{
}



void OleStorage::initStorage( const Reference< XInputStream >& rxInStream )
{
    
    Reference< XInputStream > xInStrm = rxInStream;
    if( !Reference< XSeekable >( xInStrm, UNO_QUERY ).is() ) try
    {
        Reference< XStream > xTempFile( TempFile::create(mxContext), UNO_QUERY_THROW );
        {
            Reference< XOutputStream > xOutStrm( xTempFile->getOutputStream(), UNO_SET_THROW );
            /*  Pass false to both binary stream objects to keep the UNO
                streams alive. Life time of these streams is controlled by the
                tempfile implementation. */
            BinaryXOutputStream aOutStrm( xOutStrm, false );
            BinaryXInputStream aInStrm( xInStrm, false );
            aInStrm.copyToStream( aOutStrm );
        } 
        xInStrm = xTempFile->getInputStream();
    }
    catch(const Exception& )
    {
        OSL_FAIL( "OleStorage::initStorage - cannot create temporary copy of input stream" );
    }

    
    if( xInStrm.is() ) try
    {
        Reference< XMultiServiceFactory > xFactory( mxContext->getServiceManager(), UNO_QUERY_THROW );
        Sequence< Any > aArgs( 2 );
        aArgs[ 0 ] <<= xInStrm;
        aArgs[ 1 ] <<= true;        
        mxStorage.set( xFactory->createInstanceWithArguments("com.sun.star.embed.OLESimpleStorage", aArgs ), UNO_QUERY_THROW );
    }
    catch(const Exception& )
    {
    }
}

void OleStorage::initStorage( const Reference< XStream >& rxOutStream )
{
    
    if( rxOutStream.is() ) try
    {
        Reference< XMultiServiceFactory > xFactory( mxContext->getServiceManager(), UNO_QUERY_THROW );
        Sequence< Any > aArgs( 2 );
        aArgs[ 0 ] <<= rxOutStream;
        aArgs[ 1 ] <<= true;        
        mxStorage.set( xFactory->createInstanceWithArguments("com.sun.star.embed.OLESimpleStorage", aArgs ), UNO_QUERY_THROW );
    }
    catch(const Exception& )
    {
    }
}



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
    catch(const Exception& )
    {
    }
    return false;
}

Reference< XStorage > OleStorage::implGetXStorage() const
{
    OSL_FAIL( "OleStorage::getXStorage - not implemented" );
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
    catch(const Exception& )
    {
    }
}

StorageRef OleStorage::implOpenSubStorage( const OUString& rElementName, bool bCreateMissing )
{
    StorageRef xSubStorage;
    if( mxStorage.is() && !rElementName.isEmpty() )
    {
        try
        {
            Reference< XNameContainer > xSubElements( mxStorage->getByName( rElementName ), UNO_QUERY_THROW );
            xSubStorage.reset( new OleStorage( *this, xSubElements, rElementName, true ) );
        }
        catch(const Exception& )
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
            
            Reference< XStream > xTempFile( TempFile::create(mxContext), UNO_QUERY_THROW );
            StorageRef xTempStorage( new OleStorage( *this, xTempFile, rElementName ) );
            
            if( xSubStorage.get() )
                xSubStorage->copyStorageToStorage( *xTempStorage );
            
            xSubStorage = xTempStorage;
        }
        catch(const Exception& )
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
    catch(const Exception& )
    {
    }
    return xInStream;
}

Reference< XOutputStream > OleStorage::implOpenOutputStream( const OUString& rElementName )
{
    Reference< XOutputStream > xOutStream;
    if( mxStorage.is() && !rElementName.isEmpty() )
        xOutStream.set( new OleOutputStream( mxContext, mxStorage, rElementName ) );
    return xOutStream;
}

void OleStorage::implCommit() const
{
    try
    {
        
        Reference< XTransactedObject >( mxStorage, UNO_QUERY_THROW )->commit();
        
        if( mpParentStorage )
        {
            if( mpParentStorage->mxStorage->hasByName( getName() ) )
            {
                
                mpParentStorage->mxStorage->removeByName( getName() );
                Reference< XTransactedObject >( mpParentStorage->mxStorage, UNO_QUERY_THROW )->commit();
            }
            mpParentStorage->mxStorage->insertByName( getName(), Any( mxStorage ) );
            
        }
    }
    catch(const Exception& )
    {
    }
}



} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
