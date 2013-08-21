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

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/EmbeddedObjectCreator.hpp>
#include <com/sun/star/embed/XLinkCreator.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>

#include <comphelper/seqstream.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/weakref.hxx>
#include <boost/unordered_map.hpp>
#include <algorithm>


using namespace ::com::sun::star;

namespace comphelper
{

struct hashObjectName_Impl
{
    size_t operator()(const OUString Str) const
    {
        return (size_t)Str.hashCode();
    }
};

struct eqObjectName_Impl
{
    sal_Bool operator()(const OUString Str1, const OUString Str2) const
    {
        return ( Str1 == Str2 );
    }
};

typedef boost::unordered_map
<
    OUString,
    ::com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >,
    hashObjectName_Impl,
    eqObjectName_Impl
>
EmbeddedObjectContainerNameMap;

struct EmbedImpl
{
    // TODO/LATER: remove objects from temp. Container storage when object is disposed
    EmbeddedObjectContainerNameMap maObjectContainer;
    uno::Reference < embed::XStorage > mxStorage;
    EmbeddedObjectContainer* mpTempObjectContainer;
    uno::Reference < embed::XStorage > mxImageStorage;
    uno::WeakReference < uno::XInterface > m_xModel;
    //EmbeddedObjectContainerNameMap maTempObjectContainer;
    //uno::Reference < embed::XStorage > mxTempStorage;
    bool bOwnsStorage;

    const uno::Reference < embed::XStorage >& GetReplacements();
};

const uno::Reference < embed::XStorage >& EmbedImpl::GetReplacements()
{
    if ( !mxImageStorage.is() )
    {
        try
        {
            mxImageStorage = mxStorage->openStorageElement(
                OUString("ObjectReplacements"), embed::ElementModes::READWRITE );
        }
        catch (const uno::Exception&)
        {
            mxImageStorage = mxStorage->openStorageElement(
                OUString("ObjectReplacements"), embed::ElementModes::READ );
        }
    }

    if ( !mxImageStorage.is() )
        throw io::IOException();

    return mxImageStorage;
}

EmbeddedObjectContainer::EmbeddedObjectContainer()
{
    pImpl = new EmbedImpl;
    pImpl->mxStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
    pImpl->bOwnsStorage = true;
    pImpl->mpTempObjectContainer = 0;
}

EmbeddedObjectContainer::EmbeddedObjectContainer( const uno::Reference < embed::XStorage >& rStor )
{
    pImpl = new EmbedImpl;
    pImpl->mxStorage = rStor;
    pImpl->bOwnsStorage = false;
    pImpl->mpTempObjectContainer = 0;
}

EmbeddedObjectContainer::EmbeddedObjectContainer( const uno::Reference < embed::XStorage >& rStor, const uno::Reference < uno::XInterface >& xModel )
{
    pImpl = new EmbedImpl;
    pImpl->mxStorage = rStor;
    pImpl->bOwnsStorage = false;
    pImpl->mpTempObjectContainer = 0;
    pImpl->m_xModel = xModel;
}

void EmbeddedObjectContainer::SwitchPersistence( const uno::Reference < embed::XStorage >& rStor )
{
    ReleaseImageSubStorage();

    if ( pImpl->bOwnsStorage )
        pImpl->mxStorage->dispose();

    pImpl->mxStorage = rStor;
    pImpl->bOwnsStorage = false;
}

sal_Bool EmbeddedObjectContainer::CommitImageSubStorage()
{
    if ( pImpl->mxImageStorage.is() )
    {
        try
        {
            sal_Bool bReadOnlyMode = sal_True;
            uno::Reference < beans::XPropertySet > xSet(pImpl->mxImageStorage,uno::UNO_QUERY);
            if ( xSet.is() )
            {
                // get the open mode from the parent storage
                sal_Int32 nMode = 0;
                uno::Any aAny = xSet->getPropertyValue("OpenMode");
                if ( aAny >>= nMode )
                    bReadOnlyMode = !(nMode & embed::ElementModes::WRITE );
            } // if ( xSet.is() )
            if ( !bReadOnlyMode )
            {
                uno::Reference< embed::XTransactedObject > xTransact( pImpl->mxImageStorage, uno::UNO_QUERY_THROW );
                xTransact->commit();
            }
        }
        catch (const uno::Exception&)
        {
            return sal_False;
        }
    }

    return sal_True;
}

void EmbeddedObjectContainer::ReleaseImageSubStorage()
{
    CommitImageSubStorage();

    if ( pImpl->mxImageStorage.is() )
    {
        try
        {
            pImpl->mxImageStorage->dispose();
            pImpl->mxImageStorage = uno::Reference< embed::XStorage >();
        }
        catch (const uno::Exception&)
        {
            SAL_WARN( "comphelper.container", "Problems releasing image substorage!\n" );
        }
    }
}

EmbeddedObjectContainer::~EmbeddedObjectContainer()
{
    ReleaseImageSubStorage();

    if ( pImpl->bOwnsStorage )
        pImpl->mxStorage->dispose();

    delete pImpl->mpTempObjectContainer;
    delete pImpl;
}

void EmbeddedObjectContainer::CloseEmbeddedObjects()
{
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        uno::Reference < util::XCloseable > xClose( (*aIt).second, uno::UNO_QUERY );
        if ( xClose.is() )
        {
            try
            {
                xClose->close( sal_True );
            }
            catch (const uno::Exception&)
            {
            }
        }

        ++aIt;
    }
}

OUString EmbeddedObjectContainer::CreateUniqueObjectName()
{
    OUString aPersistName("Object ");
    OUString aStr;
    sal_Int32 i=1;
    do
    {
        aStr = aPersistName;
        aStr += OUString::number( i++ );
    }
    while( HasEmbeddedObject( aStr ) );
    // TODO/LATER: should we consider deleted objects?

    return aStr;
}

uno::Sequence < OUString > EmbeddedObjectContainer::GetObjectNames()
{
    uno::Sequence < OUString > aSeq( pImpl->maObjectContainer.size() );
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    sal_Int32 nIdx=0;
    while ( aIt != pImpl->maObjectContainer.end() )
        aSeq[nIdx++] = (*aIt++).first;
    return aSeq;
}

sal_Bool EmbeddedObjectContainer::HasEmbeddedObjects()
{
    return pImpl->maObjectContainer.size() != 0;
}

sal_Bool EmbeddedObjectContainer::HasEmbeddedObject( const OUString& rName )
{
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.find( rName );
    if ( aIt == pImpl->maObjectContainer.end() )
    {
        uno::Reference < container::XNameAccess > xAccess( pImpl->mxStorage, uno::UNO_QUERY );
        return xAccess->hasByName(rName);
    }
    else
        return sal_True;
}

sal_Bool EmbeddedObjectContainer::HasEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj )
{
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        if ( (*aIt).second == xObj )
            return sal_True;
        else
            ++aIt;
    }

    return sal_False;
}

sal_Bool EmbeddedObjectContainer::HasInstantiatedEmbeddedObject( const OUString& rName )
{
    // allows to detect whether the object was already instantiated
    // currently the filter instantiate it on loading, so this method allows
    // to avoid objects pointing to the same persistence
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.find( rName );
    return ( aIt != pImpl->maObjectContainer.end() );
}

OUString EmbeddedObjectContainer::GetEmbeddedObjectName( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xObj )
{
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        if ( (*aIt).second == xObj )
            return (*aIt).first;
        else
            ++aIt;
    }

    SAL_WARN( "comphelper.container", "Unknown object!" );
    return OUString();
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::GetEmbeddedObject( const OUString& rName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::GetEmbeddedObject" );

    SAL_WARN_IF( rName.isEmpty(), "comphelper.container", "Empty object name!");

    uno::Reference < embed::XEmbeddedObject > xObj;
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.find( rName );

#if OSL_DEBUG_LEVEL > 1
    uno::Reference < container::XNameAccess > xAccess( pImpl->mxStorage, uno::UNO_QUERY );
    uno::Sequence< OUString> aSeq = xAccess->getElementNames();
    const OUString* pIter = aSeq.getConstArray();
    const OUString* pEnd   = pIter + aSeq.getLength();
    for(;pIter != pEnd;++pIter)
    {
        (void)*pIter;
    }
    OSL_ENSURE( aIt != pImpl->maObjectContainer.end() || xAccess->hasByName(rName), "Could not return object!" );
#endif

    // check if object was already created
    if ( aIt != pImpl->maObjectContainer.end() )
        xObj = (*aIt).second;
    else
        xObj = Get_Impl( rName, uno::Reference < embed::XEmbeddedObject >() );

    return xObj;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::Get_Impl( const OUString& rName, const uno::Reference < embed::XEmbeddedObject >& xCopy )
{
    uno::Reference < embed::XEmbeddedObject > xObj;
    try
    {
        // create the object from the storage
        uno::Reference < beans::XPropertySet > xSet( pImpl->mxStorage, uno::UNO_QUERY );
        sal_Bool bReadOnlyMode = sal_True;
        if ( xSet.is() )
        {
            // get the open mode from the parent storage
            sal_Int32 nMode = 0;
            uno::Any aAny = xSet->getPropertyValue("OpenMode");
            if ( aAny >>= nMode )
                bReadOnlyMode = !(nMode & embed::ElementModes::WRITE );
        }

        // object was not added until now - should happen only by calling this method from "inside"
        //TODO/LATER: it would be good to detect an error when an object should be created already, but isn't (not an "inside" call)
        uno::Reference < embed::XEmbeddedObjectCreator > xFactory = embed::EmbeddedObjectCreator::create( ::comphelper::getProcessComponentContext() );
        uno::Sequence< beans::PropertyValue > aObjDescr( xCopy.is() ? 2 : 1 );
        aObjDescr[0].Name = "Parent";
        aObjDescr[0].Value <<= pImpl->m_xModel.get();
        if ( xCopy.is() )
        {
            aObjDescr[1].Name = "CloneFrom";
            aObjDescr[1].Value <<= xCopy;
        }

        uno::Sequence< beans::PropertyValue > aMediaDescr( 1 );
        aMediaDescr[0].Name = "ReadOnly";
        aMediaDescr[0].Value <<= bReadOnlyMode;
        xObj = uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceInitFromEntry(
                pImpl->mxStorage, rName,
                aMediaDescr, aObjDescr ), uno::UNO_QUERY );

        // insert object into my list
        AddEmbeddedObject( xObj, rName );
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("comphelper.container", "EmbeddedObjectContainer::Get_Impl: exception caught: " << e.Message);
    }

    return xObj;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::CreateEmbeddedObject( const uno::Sequence < sal_Int8 >& rClassId,
            const uno::Sequence < beans::PropertyValue >& rArgs, OUString& rNewName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::CreateEmbeddedObject" );

    if ( rNewName.isEmpty() )
        rNewName = CreateUniqueObjectName();

    SAL_WARN_IF( HasEmbeddedObject(rNewName), "comphelper.container", "Object to create already exists!");

    // create object from classid by inserting it into storage
    uno::Reference < embed::XEmbeddedObject > xObj;
    try
    {
        uno::Reference < embed::XEmbeddedObjectCreator > xFactory = embed::EmbeddedObjectCreator::create( ::comphelper::getProcessComponentContext() );

        uno::Sequence< beans::PropertyValue > aObjDescr( rArgs.getLength() + 1 );
        aObjDescr[0].Name = "Parent";
        aObjDescr[0].Value <<= pImpl->m_xModel.get();
        ::std::copy( rArgs.getConstArray(), rArgs.getConstArray() + rArgs.getLength(), aObjDescr.getArray() + 1 );
        xObj = uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceInitNew(
                    rClassId, OUString(), pImpl->mxStorage, rNewName,
                    aObjDescr ), uno::UNO_QUERY );

        AddEmbeddedObject( xObj, rNewName );

        OSL_ENSURE( !xObj.is() || xObj->getCurrentState() != embed::EmbedStates::LOADED,
                    "A freshly create object should be running always!\n" );
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("comphelper.container", "EmbeddedObjectContainer::CreateEmbeddedObject: exception caught: " << e.Message);
    }

    return xObj;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::CreateEmbeddedObject( const uno::Sequence < sal_Int8 >& rClassId, OUString& rNewName )
{
    return CreateEmbeddedObject( rClassId, uno::Sequence < beans::PropertyValue >(), rNewName );
}

void EmbeddedObjectContainer::AddEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xObj, const OUString& rName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::AddEmbeddedObject" );

#if OSL_DEBUG_LEVEL > 1
    SAL_WARN_IF( rName.isEmpty(), "comphelper.container", "Added object doesn't have a name!");
    uno::Reference < container::XNameAccess > xAccess( pImpl->mxStorage, uno::UNO_QUERY );
    uno::Reference < embed::XEmbedPersist > xEmb( xObj, uno::UNO_QUERY );
    uno::Reference < embed::XLinkageSupport > xLink( xEmb, uno::UNO_QUERY );
    // if the object has a persistance and the object is not a link than it must have persistence entry in the storage
    OSL_ENSURE( !( xEmb.is() && ( !xLink.is() || !xLink->isLink() ) ) || xAccess->hasByName(rName),
                    "Added element not in storage!" );
#endif

    // remember object - it needs to be in storage already
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.find( rName );
    OSL_ENSURE( aIt == pImpl->maObjectContainer.end(), "Element already inserted!" );
    pImpl->maObjectContainer[ rName ] = xObj;
    uno::Reference < container::XChild > xChild( xObj, uno::UNO_QUERY );
    if ( xChild.is() && xChild->getParent() != pImpl->m_xModel.get() )
        xChild->setParent( pImpl->m_xModel.get() );

    // look for object in temorary container
    if ( pImpl->mpTempObjectContainer )
    {
        aIt = pImpl->mpTempObjectContainer->pImpl->maObjectContainer.begin();
        while ( aIt != pImpl->mpTempObjectContainer->pImpl->maObjectContainer.end() )
        {
            if ( (*aIt).second == xObj )
            {
                // copy replacement image from temporary container (if there is any)
                OUString aTempName = (*aIt).first;
                OUString aMediaType;
                uno::Reference < io::XInputStream > xStream = pImpl->mpTempObjectContainer->GetGraphicStream( xObj, &aMediaType );
                if ( xStream.is() )
                {
                    InsertGraphicStream( xStream, rName, aMediaType );
                    xStream = 0;
                    pImpl->mpTempObjectContainer->RemoveGraphicStream( aTempName );
                }

                // remove object from storage of temporary container
                uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
                if ( xPersist.is() )
                {
                    try
                    {
                        pImpl->mpTempObjectContainer->pImpl->mxStorage->removeElement( aTempName );
                    }
                    catch (const uno::Exception&)
                    {
                    }
                }

                // temp. container needs to forget the object
                pImpl->mpTempObjectContainer->pImpl->maObjectContainer.erase( aIt );
                break;
            }
            else
                ++aIt;
        }
    }
}

sal_Bool EmbeddedObjectContainer::StoreEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj, OUString& rName, sal_Bool bCopy )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::StoreEmbeddedObject" );

    uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
    if ( rName.isEmpty() )
        rName = CreateUniqueObjectName();

#if OSL_DEBUG_LEVEL > 1
    uno::Reference < container::XNameAccess > xAccess( pImpl->mxStorage, uno::UNO_QUERY );
    OSL_ENSURE( !xPersist.is() || !xAccess->hasByName(rName), "Inserting element already present in storage!" );
    OSL_ENSURE( xPersist.is() || xObj->getCurrentState() == embed::EmbedStates::RUNNING, "Non persistent object inserted!");
#endif

    // insert objects' storage into the container storage (if object has one)
    try
    {
        if ( xPersist.is() )
        {
            uno::Sequence < beans::PropertyValue > aSeq;
            if ( bCopy )
                xPersist->storeToEntry( pImpl->mxStorage, rName, aSeq, aSeq );
            else
            {
                //TODO/LATER: possible optimisation, don't store immediately
                //xPersist->setPersistentEntry( pImpl->mxStorage, rName, embed::EntryInitModes::ENTRY_NO_INIT, aSeq, aSeq );
                xPersist->storeAsEntry( pImpl->mxStorage, rName, aSeq, aSeq );
                xPersist->saveCompleted( sal_True );
            }
        }
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("comphelper.container", "EmbeddedObjectContainer::StoreEmbeddedObject: exception caught: " << e.Message);
        // TODO/LATER: better error recovery should keep storage intact
        return sal_False;
    }

    return sal_True;
}

sal_Bool EmbeddedObjectContainer::InsertEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj, OUString& rName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::InsertEmbeddedObject( Object )" );
    // store it into the container storage
    if ( StoreEmbeddedObject( xObj, rName, sal_False ) )
    {
        // remember object
        AddEmbeddedObject( xObj, rName );
        return sal_True;
    }
    else
        return sal_False;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::InsertEmbeddedObject( const uno::Reference < io::XInputStream >& xStm, OUString& rNewName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::InsertEmbeddedObject( InputStream )" );

    if ( rNewName.isEmpty() )
        rNewName = CreateUniqueObjectName();

    // store it into the container storage
    sal_Bool bIsStorage = sal_False;
    try
    {
        // first try storage persistence
        uno::Reference < embed::XStorage > xStore = ::comphelper::OStorageHelper::GetStorageFromInputStream( xStm );

        // storage was created from stream successfully
        bIsStorage = sal_True;

        uno::Reference < embed::XStorage > xNewStore = pImpl->mxStorage->openStorageElement( rNewName, embed::ElementModes::READWRITE );
        xStore->copyToStorage( xNewStore );
    }
    catch (const uno::Exception&)
    {
        if ( bIsStorage )
            // it is storage persistence, but opening of new substorage or copying to it failed
            return uno::Reference < embed::XEmbeddedObject >();

        // stream didn't contain a storage, now try stream persistence
        try
        {
            uno::Reference < io::XStream > xNewStream = pImpl->mxStorage->openStreamElement( rNewName, embed::ElementModes::READWRITE );
            ::comphelper::OStorageHelper::CopyInputToOutput( xStm, xNewStream->getOutputStream() );

            // No mediatype is provided so the default for OLE objects value is used
            // it is correct so for now, but what if somebody introduces a new stream based embedded object?
            // Probably introducing of such an object must be restricted ( a storage must be used! ).
            uno::Reference< beans::XPropertySet > xProps( xNewStream, uno::UNO_QUERY_THROW );
            xProps->setPropertyValue("MediaType",
                    uno::makeAny( OUString( "application/vnd.sun.star.oleobject" ) ) );
        }
        catch (uno::Exception const& e)
        {
            // complete disaster!
            SAL_WARN("comphelper.container", "EmbeddedObjectContainer::InsertEmbeddedObject: exception caught: " << e.Message);
            return uno::Reference < embed::XEmbeddedObject >();
        }
    }

    // stream was copied into the container storage in either way, now try to open something form it
    uno::Reference < embed::XEmbeddedObject > xRet = GetEmbeddedObject( rNewName );
    try
    {
        if ( !xRet.is() )
            // no object could be created, so withdraw insertion
            pImpl->mxStorage->removeElement( rNewName );
    }
    catch (const uno::Exception&)
    {
    }

    return xRet;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::InsertEmbeddedObject( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& aMedium, OUString& rNewName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::InsertEmbeddedObject( MediaDescriptor )" );

    if ( rNewName.isEmpty() )
        rNewName = CreateUniqueObjectName();

    uno::Reference < embed::XEmbeddedObject > xObj;
    try
    {
        uno::Reference < embed::XEmbeddedObjectCreator > xFactory = embed::EmbeddedObjectCreator::create( ::comphelper::getProcessComponentContext() );
        uno::Sequence< beans::PropertyValue > aObjDescr( 1 );
        aObjDescr[0].Name = "Parent";
        aObjDescr[0].Value <<= pImpl->m_xModel.get();
        xObj = uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceInitFromMediaDescriptor(
                pImpl->mxStorage, rNewName, aMedium, aObjDescr ), uno::UNO_QUERY );
        uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );

           OSL_ENSURE( !xObj.is() || xObj->getCurrentState() != embed::EmbedStates::LOADED,
                    "A freshly create object should be running always!\n" );

        // possible optimization: store later!
        if ( xPersist.is())
            xPersist->storeOwn();

        AddEmbeddedObject( xObj, rNewName );
    }
    catch (const uno::Exception&)
    {
    }

    return xObj;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::InsertEmbeddedLink( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& aMedium, OUString& rNewName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::InsertEmbeddedLink" );

    if ( rNewName.isEmpty() )
        rNewName = CreateUniqueObjectName();

    uno::Reference < embed::XEmbeddedObject > xObj;
    try
    {
        uno::Reference < embed::XEmbeddedObjectCreator > xFactory = embed::EmbeddedObjectCreator::create(::comphelper::getProcessComponentContext());
        uno::Sequence< beans::PropertyValue > aObjDescr( 1 );
        aObjDescr[0].Name = "Parent";
        aObjDescr[0].Value <<= pImpl->m_xModel.get();
        xObj = uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceLink(
                pImpl->mxStorage, rNewName, aMedium, aObjDescr ), uno::UNO_QUERY );

        uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );

           OSL_ENSURE( !xObj.is() || xObj->getCurrentState() != embed::EmbedStates::LOADED,
                    "A freshly create object should be running always!\n" );

        // possible optimization: store later!
        if ( xPersist.is())
            xPersist->storeOwn();

        AddEmbeddedObject( xObj, rNewName );
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("comphelper", "EmbeddedObjectContainer::InsertEmbeddedLink: "
                "exception caught: " << e.Message);
    }

    return xObj;
}

sal_Bool EmbeddedObjectContainer::TryToCopyGraphReplacement( EmbeddedObjectContainer& rSrc,
                                                            const OUString& aOrigName,
                                                            const OUString& aTargetName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::TryToCopyGraphReplacement" );

    sal_Bool bResult = sal_False;

    if ( ( &rSrc != this || !aOrigName.equals( aTargetName ) ) && !aOrigName.isEmpty() && !aTargetName.isEmpty() )
    {
        OUString aMediaType;
        uno::Reference < io::XInputStream > xGrStream = rSrc.GetGraphicStream( aOrigName, &aMediaType );
        if ( xGrStream.is() )
            bResult = InsertGraphicStream( xGrStream, aTargetName, aMediaType );
    }

    return bResult;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::CopyAndGetEmbeddedObject( EmbeddedObjectContainer& rSrc, const uno::Reference < embed::XEmbeddedObject >& xObj, OUString& rName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::CopyAndGetEmbeddedObject" );

    uno::Reference< embed::XEmbeddedObject > xResult;

    // TODO/LATER: For now only objects that implement XEmbedPersist have a replacement image, it might change in future
    // do an incompatible change so that object name is provided in all the move and copy methods
    OUString aOrigName;
    try
    {
        uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY_THROW );
        aOrigName = xPersist->getEntryName();
    }
    catch (const uno::Exception&)
    {
    }

    if ( rName.isEmpty() )
        rName = CreateUniqueObjectName();

    // objects without persistance are not really stored by the method
    if ( xObj.is() && StoreEmbeddedObject( xObj, rName, sal_True ) )
    {
        xResult = Get_Impl( rName, xObj);
        if ( !xResult.is() )
        {
            // this is a case when object has no real persistence
            // in such cases a new object should be explicitly created and initialized with the data of the old one
            try
            {
                uno::Reference< embed::XLinkageSupport > xOrigLinkage( xObj, uno::UNO_QUERY );
                if ( xOrigLinkage.is() && xOrigLinkage->isLink() )
                {
                    // this is a OOo link, it has no persistence
                    OUString aURL = xOrigLinkage->getLinkURL();
                    if ( aURL.isEmpty() )
                        throw uno::RuntimeException();

                    // create new linked object from the URL the link is based on
                    uno::Reference < embed::XEmbeddedObjectCreator > xCreator =
                        embed::EmbeddedObjectCreator::create( ::comphelper::getProcessComponentContext() );

                    uno::Sequence< beans::PropertyValue > aMediaDescr( 1 );
                    aMediaDescr[0].Name = "URL";
                    aMediaDescr[0].Value <<= aURL;
                    uno::Sequence< beans::PropertyValue > aObjDescr( 1 );
                    aObjDescr[0].Name = "Parent";
                    aObjDescr[0].Value <<= pImpl->m_xModel.get();
                    xResult = uno::Reference < embed::XEmbeddedObject >(
                                xCreator->createInstanceLink(
                                    pImpl->mxStorage,
                                    rName,
                                    aMediaDescr,
                                    aObjDescr ),
                                uno::UNO_QUERY_THROW );
                }
                else
                {
                    // the component is required for copying of this object
                    if ( xObj->getCurrentState() == embed::EmbedStates::LOADED )
                        xObj->changeState( embed::EmbedStates::RUNNING );

                    // this must be an object based on properties, otherwise we can not copy it currently
                    uno::Reference< beans::XPropertySet > xOrigProps( xObj->getComponent(), uno::UNO_QUERY_THROW );

                    // use object class ID to create a new one and tranfer all the properties
                    uno::Reference < embed::XEmbeddedObjectCreator > xCreator =
                        embed::EmbeddedObjectCreator::create( ::comphelper::getProcessComponentContext() );

                    uno::Sequence< beans::PropertyValue > aObjDescr( 1 );
                    aObjDescr[0].Name = "Parent";
                    aObjDescr[0].Value <<= pImpl->m_xModel.get();
                    xResult = uno::Reference < embed::XEmbeddedObject >(
                                xCreator->createInstanceInitNew(
                                    xObj->getClassID(),
                                    xObj->getClassName(),
                                    pImpl->mxStorage,
                                    rName,
                                    aObjDescr ),
                                uno::UNO_QUERY_THROW );

                    if ( xResult->getCurrentState() == embed::EmbedStates::LOADED )
                        xResult->changeState( embed::EmbedStates::RUNNING );

                    uno::Reference< beans::XPropertySet > xTargetProps( xResult->getComponent(), uno::UNO_QUERY_THROW );

                    // copy all the properties from xOrigProps to xTargetProps
                    uno::Reference< beans::XPropertySetInfo > xOrigInfo = xOrigProps->getPropertySetInfo();
                    if ( !xOrigInfo.is() )
                        throw uno::RuntimeException();

                    uno::Sequence< beans::Property > aPropertiesList = xOrigInfo->getProperties();
                    for ( sal_Int32 nInd = 0; nInd < aPropertiesList.getLength(); nInd++ )
                    {
                        try
                        {
                            xTargetProps->setPropertyValue(
                                aPropertiesList[nInd].Name,
                                xOrigProps->getPropertyValue( aPropertiesList[nInd].Name ) );
                        }
                        catch (const beans::PropertyVetoException&)
                        {
                            // impossibility to copy readonly property is not treated as an error for now
                            // but the assertion is helpful to detect such scenarios and review them
                            SAL_WARN( "comphelper.container", "Could not copy readonly property!\n" );
                        }
                    }
                }

                   if ( xResult.is() )
                    AddEmbeddedObject( xResult, rName );
            }
            catch (const uno::Exception&)
            {
                if ( xResult.is() )
                {
                    try
                    {
                        xResult->close( sal_True );
                    }
                    catch (const uno::Exception&)
                    {
                    }
                    xResult = uno::Reference< embed::XEmbeddedObject >();
                }
            }
        }
    }

    SAL_WARN_IF( !xResult.is(), "comphelper.container", "Can not copy embedded object that has no persistance!\n" );

    if ( xResult.is() )
    {
        // the object is successfully copied, try to copy graphical replacement
        if ( !aOrigName.isEmpty() )
            TryToCopyGraphReplacement( rSrc, aOrigName, rName );

        // the object might need the size to be set
        try
        {
            if ( xResult->getStatus( embed::Aspects::MSOLE_CONTENT ) & embed::EmbedMisc::EMBED_NEEDSSIZEONLOAD )
                xResult->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT,
                                            xObj->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT ) );
        }
        catch (const uno::Exception&)
        {
        }
    }

    return xResult;
}

sal_Bool EmbeddedObjectContainer::MoveEmbeddedObject( EmbeddedObjectContainer& rSrc, const uno::Reference < embed::XEmbeddedObject >& xObj, OUString& rName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::MoveEmbeddedObject( Object )" );

    // get the object name before(!) it is assigned to a new storage
    uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
    OUString aName;
    if ( xPersist.is() )
        aName = xPersist->getEntryName();

    // now move the object to the new container; the returned name is the new persist name in this container
    sal_Bool bRet;

    try
    {
        bRet = InsertEmbeddedObject( xObj, rName );
        if ( bRet )
            TryToCopyGraphReplacement( rSrc, aName, rName );
    }
    catch (const uno::Exception&)
    {
        SAL_WARN( "comphelper.container", "Failed to insert embedded object into storage!" );
        bRet = sal_False;
    }

    if ( bRet )
    {
        // now remove the object from the former container
        bRet = sal_False;
        EmbeddedObjectContainerNameMap::iterator aIt = rSrc.pImpl->maObjectContainer.begin();
        while ( aIt != rSrc.pImpl->maObjectContainer.end() )
        {
            if ( (*aIt).second == xObj )
            {
                rSrc.pImpl->maObjectContainer.erase( aIt );
                bRet = sal_True;
                break;
            }

            ++aIt;
        }

        SAL_WARN_IF( !bRet, "comphelper.container", "Object not found for removal!" );
        if ( xPersist.is() )
        {
            // now it's time to remove the storage from the container storage
            try
            {
                if ( xPersist.is() )
                    rSrc.pImpl->mxStorage->removeElement( aName );
            }
            catch (const uno::Exception&)
            {
                SAL_WARN( "comphelper.container", "Failed to remove object from storage!" );
                bRet = sal_False;
            }
        }

        // rSrc.RemoveGraphicStream( aName );
    }

    return bRet;
}

// #i119941, bKeepToTempStorage: use to specify whether store the removed object to temporary storage+
sal_Bool EmbeddedObjectContainer::RemoveEmbeddedObject( const OUString& rName, sal_Bool bClose, sal_Bool bKeepToTempStorage )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::RemoveEmbeddedObject( Name )" );

    uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObject( rName );
    if ( xObj.is() )
        //return RemoveEmbeddedObject( xObj, bClose );
        return RemoveEmbeddedObject( xObj, bClose, bKeepToTempStorage );
    else
        return sal_False;
}

sal_Bool EmbeddedObjectContainer::MoveEmbeddedObject( const OUString& rName, EmbeddedObjectContainer& rCnt )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::MoveEmbeddedObject( Name )" );

    // find object entry
    EmbeddedObjectContainerNameMap::iterator aIt2 = rCnt.pImpl->maObjectContainer.find( rName );
    OSL_ENSURE( aIt2 == rCnt.pImpl->maObjectContainer.end(), "Object does already exist in target container!" );

    if ( aIt2 != rCnt.pImpl->maObjectContainer.end() )
        return sal_False;

    uno::Reference < embed::XEmbeddedObject > xObj;
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.find( rName );
    if ( aIt != pImpl->maObjectContainer.end() )
    {
        xObj = (*aIt).second;
        try
        {
            if ( xObj.is() )
            {
                // move object
                OUString aName( rName );
                rCnt.InsertEmbeddedObject( xObj, aName );
                pImpl->maObjectContainer.erase( aIt );
                uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
                if ( xPersist.is() )
                    pImpl->mxStorage->removeElement( rName );
            }
            else
            {
                // copy storages; object *must* have persistence!
                uno::Reference < embed::XStorage > xOld = pImpl->mxStorage->openStorageElement( rName, embed::ElementModes::READ );
                uno::Reference < embed::XStorage > xNew = rCnt.pImpl->mxStorage->openStorageElement( rName, embed::ElementModes::READWRITE );
                xOld->copyToStorage( xNew );
            }

            rCnt.TryToCopyGraphReplacement( *this, rName, rName );
            // RemoveGraphicStream( rName );

            return sal_True;
        }
        catch (const uno::Exception&)
        {
            SAL_WARN( "comphelper.container", "Could not move object!");
            return sal_False;
        }

    }
    else
        SAL_WARN( "comphelper.container", "Unknown object!");
    return sal_False;
}

//sal_Bool EmbeddedObjectContainer::RemoveEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj, sal_Bool bClose )
// #i119941, bKeepToTempStorage: use to specify whether store the removed object to temporary storage+
sal_Bool EmbeddedObjectContainer::RemoveEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj, sal_Bool bClose, sal_Bool bKeepToTempStorage )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::RemoveEmbeddedObject( Object )" );

    uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
    OUString aName;
    if ( xPersist.is() )
        aName = xPersist->getEntryName();

#if OSL_DEBUG_LEVEL > 1
    uno::Reference < container::XNameAccess > xAccess( pImpl->mxStorage, uno::UNO_QUERY );
    uno::Reference < embed::XLinkageSupport > xLink( xPersist, uno::UNO_QUERY );
    sal_Bool bIsNotEmbedded = !xPersist.is() || ( xLink.is() && xLink->isLink() );

    // if the object has a persistance and the object is not a link than it must have persistence entry in the storage
    OSL_ENSURE( bIsNotEmbedded || xAccess->hasByName(aName), "Removing element not present in storage!" );
#endif

    // try to close it if permitted
    if ( bClose )
    {
        uno::Reference < ::util::XCloseable > xClose( xObj, uno::UNO_QUERY );
        try
        {
            xClose->close( sal_True );
        }
        catch (const util::CloseVetoException&)
        {
            bClose = sal_False;
        }
    }

    if ( !bClose )
    {
        // somebody still needs the object, so we must assign a temporary persistence
        try
        {
        //    if ( xPersist.is() )
             if ( xPersist.is() && bKeepToTempStorage ) // #i119941
            {
                /*
                //TODO/LATER: needs storage handling!  Why not letting the object do it?!
                if ( !pImpl->mxTempStorage.is() )
                    pImpl->mxTempStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
                uno::Sequence < beans::PropertyValue > aSeq;

                OUString aTmpPersistName = "Object ";
                aTmpPersistName += OUString::valueOf( (sal_Int32) pImpl->maTempObjectContainer.size() );

                xPersist->storeAsEntry( pImpl->mxTempStorage, aTmpPersistName, aSeq, aSeq );
                xPersist->saveCompleted( sal_True );

                pImpl->maTempObjectContainer[ aTmpPersistName ] = uno::Reference < embed::XEmbeddedObject >();
                */

                if ( !pImpl->mpTempObjectContainer )
                {
                    pImpl->mpTempObjectContainer = new EmbeddedObjectContainer();
                    try
                    {
                        // TODO/LATER: in future probably the temporary container will have two storages ( of two formats )
                        //             the media type will be provided with object insertion
                        OUString aOrigStorMediaType;
                        uno::Reference< beans::XPropertySet > xStorProps( pImpl->mxStorage, uno::UNO_QUERY_THROW );
                        static const OUString s_sMediaType("MediaType");
                        xStorProps->getPropertyValue( s_sMediaType ) >>= aOrigStorMediaType;

                        SAL_WARN_IF( aOrigStorMediaType.isEmpty(), "comphelper.container", "No valuable media type in the storage!\n" );

                        uno::Reference< beans::XPropertySet > xTargetStorProps(
                                                                    pImpl->mpTempObjectContainer->pImpl->mxStorage,
                                                                    uno::UNO_QUERY_THROW );
                        xTargetStorProps->setPropertyValue( s_sMediaType,uno::makeAny( aOrigStorMediaType ) );
                    }
                    catch (const uno::Exception&)
                    {
                        SAL_WARN( "comphelper.container", "Can not set the new media type to a storage!\n" );
                    }
                }

                OUString aTempName, aMediaType;
                pImpl->mpTempObjectContainer->InsertEmbeddedObject( xObj, aTempName );

                uno::Reference < io::XInputStream > xStream = GetGraphicStream( xObj, &aMediaType );
                if ( xStream.is() )
                    pImpl->mpTempObjectContainer->InsertGraphicStream( xStream, aTempName, aMediaType );

                // object is stored, so at least it can be set to loaded state
                xObj->changeState( embed::EmbedStates::LOADED );
            }
            else
                // objects without persistence need to stay in running state if they shall not be closed
                xObj->changeState( embed::EmbedStates::RUNNING );
        }
        catch (const uno::Exception&)
        {
            return sal_False;
        }
    }

    sal_Bool bFound = sal_False;
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        if ( (*aIt).second == xObj )
        {
            pImpl->maObjectContainer.erase( aIt );
            bFound = sal_True;
            uno::Reference < container::XChild > xChild( xObj, uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( uno::Reference < uno::XInterface >() );
            break;
        }

        ++aIt;
    }

    SAL_WARN_IF( !bFound,"comphelper.container", "Object not found for removal!" );
    (void)bFound;
    if ( xPersist.is() && bKeepToTempStorage )  // #i119941#
    {
        // remove replacement image (if there is one)
        RemoveGraphicStream( aName );

        // now it's time to remove the storage from the container storage
        try
        {
#if OSL_DEBUG_LEVEL > 1
            // if the object has a persistance and the object is not a link than it must have persistence entry in storage
            OSL_ENSURE( bIsNotEmbedded || pImpl->mxStorage->hasByName( aName ), "The object has no persistence entry in the storage!" );
#endif
            if ( xPersist.is() && pImpl->mxStorage->hasByName( aName ) )
                pImpl->mxStorage->removeElement( aName );
        }
        catch (const uno::Exception&)
        {
            SAL_WARN( "comphelper.container", "Failed to remove object from storage!" );
            return sal_False;
        }
    }

    return sal_True;
}

sal_Bool EmbeddedObjectContainer::CloseEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::CloseEmbeddedObject" );

    // disconnect the object from the container and close it if possible

    sal_Bool bFound = sal_False;
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        if ( (*aIt).second == xObj )
        {
            pImpl->maObjectContainer.erase( aIt );
            bFound = sal_True;
            break;
        }

        ++aIt;
    }

    if ( bFound )
    {
        uno::Reference < ::util::XCloseable > xClose( xObj, uno::UNO_QUERY );
        try
        {
            xClose->close( sal_True );
        }
        catch (const uno::Exception&)
        {
            // it is no problem if the object is already closed
            // TODO/LATER: what if the object can not be closed?
        }
    }

    return bFound;
}

uno::Reference < io::XInputStream > EmbeddedObjectContainer::GetGraphicStream( const OUString& aName, OUString* pMediaType )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::GetGraphicStream( Name )" );

    uno::Reference < io::XInputStream > xStream;

    SAL_WARN_IF( aName.isEmpty(), "comphelper.container", "Retrieving graphic for unknown object!" );
    if ( !aName.isEmpty() )
    {
        try
        {
            uno::Reference < embed::XStorage > xReplacements = pImpl->GetReplacements();
            uno::Reference < io::XStream > xGraphicStream = xReplacements->openStreamElement( aName, embed::ElementModes::READ );
            xStream = xGraphicStream->getInputStream();
            if ( pMediaType )
            {
                uno::Reference < beans::XPropertySet > xSet( xStream, uno::UNO_QUERY );
                if ( xSet.is() )
                {
                    uno::Any aAny = xSet->getPropertyValue("MediaType");
                    aAny >>= *pMediaType;
                }
            }
        }
        catch (const uno::Exception&)
        {
        }
    }

    return xStream;
}

uno::Reference < io::XInputStream > EmbeddedObjectContainer::GetGraphicStream( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xObj, OUString* pMediaType )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::GetGraphicStream( Object )" );

    // get the object name
    OUString aName;
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        if ( (*aIt).second == xObj )
        {
            aName = (*aIt).first;
            break;
        }

        ++aIt;
    }

    // try to load it from the container storage
    return GetGraphicStream( aName, pMediaType );
}

sal_Bool EmbeddedObjectContainer::InsertGraphicStream( const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& rStream, const OUString& rObjectName, const OUString& rMediaType )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::InsertGraphicStream" );

    try
    {
        uno::Reference < embed::XStorage > xReplacements = pImpl->GetReplacements();

        // store it into the subfolder
        uno::Reference < io::XOutputStream > xOutStream;
        uno::Reference < io::XStream > xGraphicStream = xReplacements->openStreamElement( rObjectName,
                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
        xOutStream = xGraphicStream->getOutputStream();
        ::comphelper::OStorageHelper::CopyInputToOutput( rStream, xOutStream );
        xOutStream->flush();

        uno::Reference< beans::XPropertySet > xPropSet( xGraphicStream, uno::UNO_QUERY );
        if ( !xPropSet.is() )
            throw uno::RuntimeException();

        xPropSet->setPropertyValue("UseCommonStoragePasswordEncryption",
                                    uno::makeAny( (sal_Bool)sal_True ) );
        uno::Any aAny;
        aAny <<= rMediaType;
        xPropSet->setPropertyValue("MediaType", aAny );

        xPropSet->setPropertyValue("Compressed",
                                    uno::makeAny( (sal_Bool)sal_True ) );
    }
    catch (const uno::Exception&)
    {
        return sal_False;
    }

    return sal_True;
}

sal_Bool EmbeddedObjectContainer::InsertGraphicStreamDirectly( const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& rStream, const OUString& rObjectName, const OUString& rMediaType )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::InsertGraphicStreamDirectly" );

    try
    {
        uno::Reference < embed::XStorage > xReplacement = pImpl->GetReplacements();
        uno::Reference < embed::XOptimizedStorage > xOptRepl( xReplacement, uno::UNO_QUERY_THROW );

        // store it into the subfolder
        uno::Sequence< beans::PropertyValue > aProps( 3 );
        aProps[0].Name = "MediaType";
        aProps[0].Value <<= rMediaType;
        aProps[1].Name = "UseCommonStoragePasswordEncryption";
        aProps[1].Value <<= (sal_Bool)sal_True;
        aProps[2].Name = "Compressed";
        aProps[2].Value <<= (sal_Bool)sal_True;

        if ( xReplacement->hasByName( rObjectName ) )
            xReplacement->removeElement( rObjectName );

        xOptRepl->insertStreamElementDirect( rObjectName, rStream, aProps );
    }
    catch (const uno::Exception&)
    {
        return sal_False;
    }

    return sal_True;
}


sal_Bool EmbeddedObjectContainer::RemoveGraphicStream( const OUString& rObjectName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::RemoveGraphicStream" );

    try
    {
        uno::Reference < embed::XStorage > xReplacements = pImpl->GetReplacements();
        xReplacements->removeElement( rObjectName );
    }
    catch (const uno::Exception&)
    {
        return sal_False;
    }

    return sal_True;
}
namespace {
    void InsertStreamIntoPicturesStorage_Impl( const uno::Reference< embed::XStorage >& xDocStor,
                                            const uno::Reference< io::XInputStream >& xInStream,
                                            const OUString& aStreamName )
    {
        OSL_ENSURE( !aStreamName.isEmpty() && xInStream.is() && xDocStor.is(), "Misuse of the method!\n" );

        try
        {
            uno::Reference< embed::XStorage > xPictures = xDocStor->openStorageElement(
                                        OUString( "Pictures" ),
                                        embed::ElementModes::READWRITE );
            uno::Reference< io::XStream > xObjReplStr = xPictures->openStreamElement(
                                        aStreamName,
                                        embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
            uno::Reference< io::XOutputStream > xOutStream(
                                        xObjReplStr->getInputStream(), uno::UNO_QUERY_THROW );

            ::comphelper::OStorageHelper::CopyInputToOutput( xInStream, xOutStream );
            xOutStream->closeOutput();

            uno::Reference< embed::XTransactedObject > xTransact( xPictures, uno::UNO_QUERY );
            if ( xTransact.is() )
                xTransact->commit();
        }
        catch (const uno::Exception&)
        {
            SAL_WARN( "comphelper.container", "The pictures storage is not available!\n" );
        }
    }

}
// -----------------------------------------------------------------------------
sal_Bool EmbeddedObjectContainer::StoreAsChildren(sal_Bool _bOasisFormat,sal_Bool _bCreateEmbedded,const uno::Reference < embed::XStorage >& _xStorage)
{
    sal_Bool bResult = sal_False;
    try
    {
        comphelper::EmbeddedObjectContainer aCnt( _xStorage );
        const uno::Sequence < OUString > aNames = GetObjectNames();
        const OUString* pIter = aNames.getConstArray();
        const OUString* pEnd   = pIter + aNames.getLength();
        for(;pIter != pEnd;++pIter)
        {
            uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObject( *pIter );
            SAL_WARN_IF( !xObj.is(), "comphelper.container", "An empty entry in the embedded objects list!\n" );
            if ( xObj.is() )
            {
                sal_Bool bSwitchBackToLoaded = sal_False;
                uno::Reference< embed::XLinkageSupport > xLink( xObj, uno::UNO_QUERY );

                uno::Reference < io::XInputStream > xStream;
                OUString aMediaType;

                sal_Int32 nCurState = xObj->getCurrentState();
                if ( nCurState == embed::EmbedStates::LOADED || nCurState == embed::EmbedStates::RUNNING )
                {
                    // means that the object is not active
                    // copy replacement image from old to new container
                    xStream = GetGraphicStream( xObj, &aMediaType );
                }

                if ( !xStream.is() )
                {
                    // the image must be regenerated
                    // TODO/LATER: another aspect could be used
                    if ( xObj->getCurrentState() == embed::EmbedStates::LOADED )
                            bSwitchBackToLoaded = sal_True;

                    xStream = GetGraphicReplacementStream(
                                                            embed::Aspects::MSOLE_CONTENT,
                                                            xObj,
                                                            &aMediaType );
                }

                if ( _bOasisFormat || (xLink.is() && xLink->isLink()) )
                {
                    if ( xStream.is() )
                    {
                        if ( _bOasisFormat )
                        {
                            // if it is an embedded object or the optimized inserting fails the normal inserting should be done
                            if ( _bCreateEmbedded
                                || !aCnt.InsertGraphicStreamDirectly( xStream, *pIter, aMediaType ) )
                                aCnt.InsertGraphicStream( xStream, *pIter, aMediaType );
                        }
                        else
                        {
                            // it is a linked object exported into SO7 format
                            InsertStreamIntoPicturesStorage_Impl( _xStorage, xStream, *pIter );
                        }
                    }
                }

                uno::Reference< embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
                if ( xPersist.is() )
                {
                    uno::Sequence< beans::PropertyValue > aArgs( _bOasisFormat ? 2 : 3 );
                    aArgs[0].Name = "StoreVisualReplacement";
                    aArgs[0].Value <<= (sal_Bool)( !_bOasisFormat );

                    // if it is an embedded object or the optimized inserting fails the normal inserting should be done
                    aArgs[1].Name = "CanTryOptimization";
                    aArgs[1].Value <<= !_bCreateEmbedded;
                    if ( !_bOasisFormat )
                    {
                        // if object has no cached replacement it will use this one
                        aArgs[2].Name = "VisualReplacement";
                        aArgs[2].Value <<= xStream;
                    }

                    try
                    {
                        xPersist->storeAsEntry( _xStorage, xPersist->getEntryName(), uno::Sequence< beans::PropertyValue >(), aArgs );
                    }
                    catch (const embed::WrongStateException&)
                    {
                        SAL_WARN("comphelper.container", "failed to store '" << *pIter << "'");
                    }
                }

                if ( bSwitchBackToLoaded )
                    // switch back to loaded state; that way we have a minimum cache confusion
                    xObj->changeState( embed::EmbedStates::LOADED );
            }
        }

        bResult = aCnt.CommitImageSubStorage();

    }
    catch (const uno::Exception& e)
    {
        // TODO/LATER: error handling
        bResult = sal_False;
        SAL_WARN("comphelper.container", "failed. Message: " << e.Message);
    }

    // the old SO6 format does not store graphical replacements
    if ( !_bOasisFormat && bResult )
    {
        try
        {
            // the substorage still can not be locked by the embedded object conteiner
            OUString aObjReplElement( "ObjectReplacements" );
            if ( _xStorage->hasByName( aObjReplElement ) && _xStorage->isStorageElement( aObjReplElement ) )
                _xStorage->removeElement( aObjReplElement );
        }
        catch (const uno::Exception&)
        {
            // TODO/LATER: error handling;
            bResult = sal_False;
        }
    }
    return bResult;
}
// -----------------------------------------------------------------------------
sal_Bool EmbeddedObjectContainer::StoreChildren(sal_Bool _bOasisFormat,sal_Bool _bObjectsOnly)
{
    sal_Bool bResult = sal_True;
    const uno::Sequence < OUString > aNames = GetObjectNames();
    const OUString* pIter = aNames.getConstArray();
    const OUString* pEnd   = pIter + aNames.getLength();
    for(;pIter != pEnd;++pIter)
    {
        uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObject( *pIter );
        SAL_WARN_IF( !xObj.is(), "comphelper.container", "An empty entry in the embedded objects list!\n" );
        if ( xObj.is() )
        {
            sal_Int32 nCurState = xObj->getCurrentState();
            if ( _bOasisFormat && nCurState != embed::EmbedStates::LOADED && nCurState != embed::EmbedStates::RUNNING )
            {
                // means that the object is active
                // the image must be regenerated
                OUString aMediaType;

                // TODO/LATER: another aspect could be used
                uno::Reference < io::XInputStream > xStream =
                            GetGraphicReplacementStream(
                                                        embed::Aspects::MSOLE_CONTENT,
                                                        xObj,
                                                        &aMediaType );
                if ( xStream.is() )
                {
                    if ( !InsertGraphicStreamDirectly( xStream, *pIter, aMediaType ) )
                        InsertGraphicStream( xStream, *pIter, aMediaType );
                }
            }

            // TODO/LATER: currently the object by default does not cache replacement image
            // that means that if somebody loads SO7 document and store its objects using
            // this method the images might be lost.
            // Currently this method is only used on storing to alien formats, that means
            // that SO7 documents storing does not use it, and all other filters are
            // based on OASIS format. But if it changes the method must be fixed. The fix
            // must be done only on demand since it can affect performance.

            uno::Reference< embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
            if ( xPersist.is() )
            {
                try
                {
                    //TODO/LATER: only storing if changed!
                    //xPersist->storeOwn(); //commented, i120168

            // begin:all charts will be persited as xml format on disk when saving, which is time consuming.
                    // '_bObjectsOnly' mean we are storing to alien formats.
                    //  'isStorageElement' mean current object is NOT an MS OLE format. (may also include in future), i120168
                    if (_bObjectsOnly && (nCurState == embed::EmbedStates::LOADED || nCurState == embed::EmbedStates::RUNNING)
                        && (pImpl->mxStorage->isStorageElement( *pIter ) ))
                    {
                        uno::Reference< util::XModifiable > xModifiable( xObj->getComponent(), uno::UNO_QUERY );
                        if ( xModifiable.is() && xModifiable->isModified())
                        {
                            xPersist->storeOwn();
                        }
                        else
                        {
                            //do nothing.embeded model is not modified, no need to persist.
                        }
                    }
                    else //the embeded object is in active status, always store back it.
                    {
                        xPersist->storeOwn();
                    }
                    //end i120168
                }
                catch (const uno::Exception&)
                {
                    // TODO/LATER: error handling
                    bResult = sal_False;
                    break;
                }
            }

            if ( !_bOasisFormat && !_bObjectsOnly )
            {
                // copy replacement images for linked objects
                try
                {
                    uno::Reference< embed::XLinkageSupport > xLink( xObj, uno::UNO_QUERY );
                    if ( xLink.is() && xLink->isLink() )
                    {
                        OUString aMediaType;
                        uno::Reference < io::XInputStream > xInStream = GetGraphicStream( xObj, &aMediaType );
                        if ( xInStream.is() )
                            InsertStreamIntoPicturesStorage_Impl( pImpl->mxStorage, xInStream, *pIter );
                    }
                }
                catch (const uno::Exception&)
                {
                }
            }
        }
    }

    if ( bResult && _bOasisFormat )
        bResult = CommitImageSubStorage();

    if ( bResult && !_bObjectsOnly )
    {
        try
        {
            ReleaseImageSubStorage();
            OUString aObjReplElement( "ObjectReplacements" );
            if ( !_bOasisFormat && pImpl->mxStorage->hasByName( aObjReplElement ) && pImpl->mxStorage->isStorageElement( aObjReplElement ) )
                pImpl->mxStorage->removeElement( aObjReplElement );
        }
        catch (const uno::Exception&)
        {
            // TODO/LATER: error handling
            bResult = sal_False;
        }
    }
    return bResult;
}
// -----------------------------------------------------------------------------
uno::Reference< io::XInputStream > EmbeddedObjectContainer::GetGraphicReplacementStream(
                                                                sal_Int64 nViewAspect,
                                                                const uno::Reference< embed::XEmbeddedObject >& xObj,
                                                                OUString* pMediaType )
{
    uno::Reference< io::XInputStream > xInStream;
    if ( xObj.is() )
    {
        try
        {
            // retrieving of the visual representation can switch object to running state
            embed::VisualRepresentation aRep = xObj->getPreferredVisualRepresentation( nViewAspect );
            if ( pMediaType )
                *pMediaType = aRep.Flavor.MimeType;

            uno::Sequence < sal_Int8 > aSeq;
            aRep.Data >>= aSeq;
            xInStream = new ::comphelper::SequenceInputStream( aSeq );
        }
        catch (const uno::Exception&)
        {
        }
    }

    return xInStream;
}
// -----------------------------------------------------------------------------
sal_Bool EmbeddedObjectContainer::SetPersistentEntries(const uno::Reference< embed::XStorage >& _xStorage,bool _bClearModifedFlag)
{
    sal_Bool bError = sal_False;
    const uno::Sequence < OUString > aNames = GetObjectNames();
    const OUString* pIter = aNames.getConstArray();
    const OUString* pEnd   = pIter + aNames.getLength();
    for(;pIter != pEnd;++pIter)
    {
        uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObject( *pIter );
        SAL_WARN_IF( !xObj.is(), "comphelper.container", "An empty entry in the embedded objects list!\n" );
        if ( xObj.is() )
        {
            uno::Reference< embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
            if ( xPersist.is() )
            {
                try
                {
                    xPersist->setPersistentEntry( _xStorage,
                                                *pIter,
                                                embed::EntryInitModes::NO_INIT,
                                                uno::Sequence< beans::PropertyValue >(),
                                                uno::Sequence< beans::PropertyValue >() );

                }
                catch (const uno::Exception&)
                {
                    // TODO/LATER: error handling
                    bError = sal_True;
                    break;
                }
            }
            if ( _bClearModifedFlag )
            {
                // if this method is used as part of SaveCompleted the object must stay unmodified after execution
                try
                {
                    uno::Reference< util::XModifiable > xModif( xObj->getComponent(), uno::UNO_QUERY_THROW );
                    if ( xModif->isModified() )
                        xModif->setModified( sal_False );
                }
                catch (const uno::Exception&)
                {
                }
            }
        }
    }
    return bError;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
