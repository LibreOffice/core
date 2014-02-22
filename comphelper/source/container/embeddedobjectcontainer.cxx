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
    bool operator()(const OUString Str1, const OUString Str2) const
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
    
    EmbeddedObjectContainerNameMap maObjectContainer;
    uno::Reference < embed::XStorage > mxStorage;
    EmbeddedObjectContainer* mpTempObjectContainer;
    uno::Reference < embed::XStorage > mxImageStorage;
    uno::WeakReference < uno::XInterface > m_xModel;
    
    
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

bool EmbeddedObjectContainer::CommitImageSubStorage()
{
    if ( pImpl->mxImageStorage.is() )
    {
        try
        {
            bool bReadOnlyMode = true;
            uno::Reference < beans::XPropertySet > xSet(pImpl->mxImageStorage,uno::UNO_QUERY);
            if ( xSet.is() )
            {
                
                sal_Int32 nMode = 0;
                uno::Any aAny = xSet->getPropertyValue("OpenMode");
                if ( aAny >>= nMode )
                    bReadOnlyMode = !(nMode & embed::ElementModes::WRITE );
            } 
            if ( !bReadOnlyMode )
            {
                uno::Reference< embed::XTransactedObject > xTransact( pImpl->mxImageStorage, uno::UNO_QUERY_THROW );
                xTransact->commit();
            }
        }
        catch (const uno::Exception&)
        {
            return false;
        }
    }

    return true;
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

bool EmbeddedObjectContainer::HasEmbeddedObjects()
{
    return pImpl->maObjectContainer.size() != 0;
}

bool EmbeddedObjectContainer::HasEmbeddedObject( const OUString& rName )
{
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.find( rName );
    if ( aIt == pImpl->maObjectContainer.end() )
    {
        uno::Reference < container::XNameAccess > xAccess( pImpl->mxStorage, uno::UNO_QUERY );
        return xAccess->hasByName(rName);
    }
    else
        return true;
}

bool EmbeddedObjectContainer::HasEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj )
{
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        if ( (*aIt).second == xObj )
            return true;
        else
            ++aIt;
    }

    return false;
}

bool EmbeddedObjectContainer::HasInstantiatedEmbeddedObject( const OUString& rName )
{
    
    
    
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
        
        uno::Reference < beans::XPropertySet > xSet( pImpl->mxStorage, uno::UNO_QUERY );
        bool bReadOnlyMode = true;
        if ( xSet.is() )
        {
            
            sal_Int32 nMode = 0;
            uno::Any aAny = xSet->getPropertyValue("OpenMode");
            if ( aAny >>= nMode )
                bReadOnlyMode = !(nMode & embed::ElementModes::WRITE );
        }

        
        
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
    
    OSL_ENSURE( !( xEmb.is() && ( !xLink.is() || !xLink->isLink() ) ) || xAccess->hasByName(rName),
                    "Added element not in storage!" );
#endif

    
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.find( rName );
    OSL_ENSURE( aIt == pImpl->maObjectContainer.end(), "Element already inserted!" );
    pImpl->maObjectContainer[ rName ] = xObj;
    uno::Reference < container::XChild > xChild( xObj, uno::UNO_QUERY );
    if ( xChild.is() && xChild->getParent() != pImpl->m_xModel.get() )
        xChild->setParent( pImpl->m_xModel.get() );

    
    if ( pImpl->mpTempObjectContainer )
    {
        aIt = pImpl->mpTempObjectContainer->pImpl->maObjectContainer.begin();
        while ( aIt != pImpl->mpTempObjectContainer->pImpl->maObjectContainer.end() )
        {
            if ( (*aIt).second == xObj )
            {
                
                OUString aTempName = (*aIt).first;
                OUString aMediaType;
                uno::Reference < io::XInputStream > xStream = pImpl->mpTempObjectContainer->GetGraphicStream( xObj, &aMediaType );
                if ( xStream.is() )
                {
                    InsertGraphicStream( xStream, rName, aMediaType );
                    xStream = 0;
                    pImpl->mpTempObjectContainer->RemoveGraphicStream( aTempName );
                }

                
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

                
                pImpl->mpTempObjectContainer->pImpl->maObjectContainer.erase( aIt );
                break;
            }
            else
                ++aIt;
        }
    }
}

bool EmbeddedObjectContainer::StoreEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj, OUString& rName, bool bCopy )
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

    
    try
    {
        if ( xPersist.is() )
        {
            uno::Sequence < beans::PropertyValue > aSeq;
            if ( bCopy )
                xPersist->storeToEntry( pImpl->mxStorage, rName, aSeq, aSeq );
            else
            {
                
                
                xPersist->storeAsEntry( pImpl->mxStorage, rName, aSeq, aSeq );
                xPersist->saveCompleted( sal_True );
            }
        }
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("comphelper.container", "EmbeddedObjectContainer::StoreEmbeddedObject: exception caught: " << e.Message);
        
        return false;
    }

    return true;
}

bool EmbeddedObjectContainer::InsertEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj, OUString& rName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::InsertEmbeddedObject( Object )" );
    
    if ( StoreEmbeddedObject( xObj, rName, false ) )
    {
        
        AddEmbeddedObject( xObj, rName );
        return true;
    }
    else
        return false;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::InsertEmbeddedObject( const uno::Reference < io::XInputStream >& xStm, OUString& rNewName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::InsertEmbeddedObject( InputStream )" );

    if ( rNewName.isEmpty() )
        rNewName = CreateUniqueObjectName();

    
    bool bIsStorage = false;
    try
    {
        
        uno::Reference < embed::XStorage > xStore = ::comphelper::OStorageHelper::GetStorageFromInputStream( xStm );

        
        bIsStorage = true;

        uno::Reference < embed::XStorage > xNewStore = pImpl->mxStorage->openStorageElement( rNewName, embed::ElementModes::READWRITE );
        xStore->copyToStorage( xNewStore );
    }
    catch (const uno::Exception&)
    {
        if ( bIsStorage )
            
            return uno::Reference < embed::XEmbeddedObject >();

        
        try
        {
            uno::Reference < io::XStream > xNewStream = pImpl->mxStorage->openStreamElement( rNewName, embed::ElementModes::READWRITE );
            ::comphelper::OStorageHelper::CopyInputToOutput( xStm, xNewStream->getOutputStream() );

            
            
            
            uno::Reference< beans::XPropertySet > xProps( xNewStream, uno::UNO_QUERY_THROW );
            xProps->setPropertyValue("MediaType",
                    uno::makeAny( OUString( "application/vnd.sun.star.oleobject" ) ) );
        }
        catch (uno::Exception const& e)
        {
            
            SAL_WARN("comphelper.container", "EmbeddedObjectContainer::InsertEmbeddedObject: exception caught: " << e.Message);
            return uno::Reference < embed::XEmbeddedObject >();
        }
    }

    
    uno::Reference < embed::XEmbeddedObject > xRet = GetEmbeddedObject( rNewName );
    try
    {
        if ( !xRet.is() )
            
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

bool EmbeddedObjectContainer::TryToCopyGraphReplacement( EmbeddedObjectContainer& rSrc,
                                                            const OUString& aOrigName,
                                                            const OUString& aTargetName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::TryToCopyGraphReplacement" );

    bool bResult = false;

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

    
    if ( xObj.is() && StoreEmbeddedObject( xObj, rName, true ) )
    {
        xResult = Get_Impl( rName, xObj);
        if ( !xResult.is() )
        {
            
            
            try
            {
                uno::Reference< embed::XLinkageSupport > xOrigLinkage( xObj, uno::UNO_QUERY );
                if ( xOrigLinkage.is() && xOrigLinkage->isLink() )
                {
                    
                    OUString aURL = xOrigLinkage->getLinkURL();
                    if ( aURL.isEmpty() )
                        throw uno::RuntimeException();

                    
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
                    
                    if ( xObj->getCurrentState() == embed::EmbedStates::LOADED )
                        xObj->changeState( embed::EmbedStates::RUNNING );

                    
                    uno::Reference< beans::XPropertySet > xOrigProps( xObj->getComponent(), uno::UNO_QUERY_THROW );

                    
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
        
        if ( !aOrigName.isEmpty() )
            TryToCopyGraphReplacement( rSrc, aOrigName, rName );

        
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

bool EmbeddedObjectContainer::MoveEmbeddedObject( EmbeddedObjectContainer& rSrc, const uno::Reference < embed::XEmbeddedObject >& xObj, OUString& rName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::MoveEmbeddedObject( Object )" );

    
    uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
    OUString aName;
    if ( xPersist.is() )
        aName = xPersist->getEntryName();

    
    bool bRet;

    try
    {
        bRet = InsertEmbeddedObject( xObj, rName );
        if ( bRet )
            TryToCopyGraphReplacement( rSrc, aName, rName );
    }
    catch (const uno::Exception&)
    {
        SAL_WARN( "comphelper.container", "Failed to insert embedded object into storage!" );
        bRet = false;
    }

    if ( bRet )
    {
        
        bRet = false;
        EmbeddedObjectContainerNameMap::iterator aIt = rSrc.pImpl->maObjectContainer.begin();
        while ( aIt != rSrc.pImpl->maObjectContainer.end() )
        {
            if ( (*aIt).second == xObj )
            {
                rSrc.pImpl->maObjectContainer.erase( aIt );
                bRet = true;
                break;
            }

            ++aIt;
        }

        SAL_WARN_IF( !bRet, "comphelper.container", "Object not found for removal!" );
        if ( xPersist.is() )
        {
            
            try
            {
                if ( xPersist.is() )
                    rSrc.pImpl->mxStorage->removeElement( aName );
            }
            catch (const uno::Exception&)
            {
                SAL_WARN( "comphelper.container", "Failed to remove object from storage!" );
                bRet = false;
            }
        }

        
    }

    return bRet;
}


bool EmbeddedObjectContainer::RemoveEmbeddedObject( const OUString& rName, bool bClose, bool bKeepToTempStorage )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::RemoveEmbeddedObject( Name )" );

    uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObject( rName );
    if ( xObj.is() )
        
        return RemoveEmbeddedObject( xObj, bClose, bKeepToTempStorage );
    else
        return false;
}

bool EmbeddedObjectContainer::MoveEmbeddedObject( const OUString& rName, EmbeddedObjectContainer& rCnt )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::MoveEmbeddedObject( Name )" );

    
    EmbeddedObjectContainerNameMap::iterator aIt2 = rCnt.pImpl->maObjectContainer.find( rName );
    OSL_ENSURE( aIt2 == rCnt.pImpl->maObjectContainer.end(), "Object does already exist in target container!" );

    if ( aIt2 != rCnt.pImpl->maObjectContainer.end() )
        return false;

    uno::Reference < embed::XEmbeddedObject > xObj;
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.find( rName );
    if ( aIt != pImpl->maObjectContainer.end() )
    {
        xObj = (*aIt).second;
        try
        {
            if ( xObj.is() )
            {
                
                OUString aName( rName );
                rCnt.InsertEmbeddedObject( xObj, aName );
                pImpl->maObjectContainer.erase( aIt );
                uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
                if ( xPersist.is() )
                    pImpl->mxStorage->removeElement( rName );
            }
            else
            {
                
                uno::Reference < embed::XStorage > xOld = pImpl->mxStorage->openStorageElement( rName, embed::ElementModes::READ );
                uno::Reference < embed::XStorage > xNew = rCnt.pImpl->mxStorage->openStorageElement( rName, embed::ElementModes::READWRITE );
                xOld->copyToStorage( xNew );
            }

            rCnt.TryToCopyGraphReplacement( *this, rName, rName );
            

            return true;
        }
        catch (const uno::Exception&)
        {
            SAL_WARN( "comphelper.container", "Could not move object!");
            return false;
        }

    }
    else
        SAL_WARN( "comphelper.container", "Unknown object!");
    return false;
}



bool EmbeddedObjectContainer::RemoveEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj, bool bClose, bool bKeepToTempStorage )
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

    
    OSL_ENSURE( bIsNotEmbedded || xAccess->hasByName(aName), "Removing element not present in storage!" );
#endif

    
    if ( bClose )
    {
        uno::Reference < ::util::XCloseable > xClose( xObj, uno::UNO_QUERY );
        try
        {
            xClose->close( sal_True );
        }
        catch (const util::CloseVetoException&)
        {
            bClose = false;
        }
    }

    if ( !bClose )
    {
        
        try
        {
        
             if ( xPersist.is() && bKeepToTempStorage ) 
            {
                /*
                
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

                
                xObj->changeState( embed::EmbedStates::LOADED );
            }
            else
                
                xObj->changeState( embed::EmbedStates::RUNNING );
        }
        catch (const uno::Exception&)
        {
            return false;
        }
    }

    bool bFound = false;
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        if ( (*aIt).second == xObj )
        {
            pImpl->maObjectContainer.erase( aIt );
            bFound = true;
            uno::Reference < container::XChild > xChild( xObj, uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( uno::Reference < uno::XInterface >() );
            break;
        }

        ++aIt;
    }

    SAL_WARN_IF( !bFound,"comphelper.container", "Object not found for removal!" );
    (void)bFound;
    if ( xPersist.is() && bKeepToTempStorage )  
    {
        
        RemoveGraphicStream( aName );

        
        try
        {
#if OSL_DEBUG_LEVEL > 1
            
            OSL_ENSURE( bIsNotEmbedded || pImpl->mxStorage->hasByName( aName ), "The object has no persistence entry in the storage!" );
#endif
            if ( xPersist.is() && pImpl->mxStorage->hasByName( aName ) )
                pImpl->mxStorage->removeElement( aName );
        }
        catch (const uno::Exception&)
        {
            SAL_WARN( "comphelper.container", "Failed to remove object from storage!" );
            return false;
        }
    }

    return true;
}

bool EmbeddedObjectContainer::CloseEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::CloseEmbeddedObject" );

    

    bool bFound = false;
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        if ( (*aIt).second == xObj )
        {
            pImpl->maObjectContainer.erase( aIt );
            bFound = true;
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

    
    return GetGraphicStream( aName, pMediaType );
}

bool EmbeddedObjectContainer::InsertGraphicStream( const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& rStream, const OUString& rObjectName, const OUString& rMediaType )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::InsertGraphicStream" );

    try
    {
        uno::Reference < embed::XStorage > xReplacements = pImpl->GetReplacements();

        
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
                                    uno::makeAny( true ) );
        uno::Any aAny;
        aAny <<= rMediaType;
        xPropSet->setPropertyValue("MediaType", aAny );

        xPropSet->setPropertyValue("Compressed",
                                    uno::makeAny( true ) );
    }
    catch (const uno::Exception&)
    {
        return false;
    }

    return true;
}

bool EmbeddedObjectContainer::InsertGraphicStreamDirectly( const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& rStream, const OUString& rObjectName, const OUString& rMediaType )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::InsertGraphicStreamDirectly" );

    try
    {
        uno::Reference < embed::XStorage > xReplacement = pImpl->GetReplacements();
        uno::Reference < embed::XOptimizedStorage > xOptRepl( xReplacement, uno::UNO_QUERY_THROW );

        
        uno::Sequence< beans::PropertyValue > aProps( 3 );
        aProps[0].Name = "MediaType";
        aProps[0].Value <<= rMediaType;
        aProps[1].Name = "UseCommonStoragePasswordEncryption";
        aProps[1].Value <<= true;
        aProps[2].Name = "Compressed";
        aProps[2].Value <<= true;

        if ( xReplacement->hasByName( rObjectName ) )
            xReplacement->removeElement( rObjectName );

        xOptRepl->insertStreamElementDirect( rObjectName, rStream, aProps );
    }
    catch (const uno::Exception&)
    {
        return false;
    }

    return true;
}


bool EmbeddedObjectContainer::RemoveGraphicStream( const OUString& rObjectName )
{
    SAL_INFO( "comphelper.container", "comphelper (mv76033) comphelper::EmbeddedObjectContainer::RemoveGraphicStream" );

    try
    {
        uno::Reference < embed::XStorage > xReplacements = pImpl->GetReplacements();
        xReplacements->removeElement( rObjectName );
    }
    catch (const uno::Exception&)
    {
        return false;
    }

    return true;
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

bool EmbeddedObjectContainer::StoreAsChildren(bool _bOasisFormat,bool _bCreateEmbedded,const uno::Reference < embed::XStorage >& _xStorage)
{
    bool bResult = false;
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
                bool bSwitchBackToLoaded = false;
                uno::Reference< embed::XLinkageSupport > xLink( xObj, uno::UNO_QUERY );

                uno::Reference < io::XInputStream > xStream;
                OUString aMediaType;

                sal_Int32 nCurState = xObj->getCurrentState();
                if ( nCurState == embed::EmbedStates::LOADED || nCurState == embed::EmbedStates::RUNNING )
                {
                    
                    
                    xStream = GetGraphicStream( xObj, &aMediaType );
                }

                if ( !xStream.is() )
                {
                    
                    
                    if ( xObj->getCurrentState() == embed::EmbedStates::LOADED )
                            bSwitchBackToLoaded = true;

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
                            
                            if ( _bCreateEmbedded
                                || !aCnt.InsertGraphicStreamDirectly( xStream, *pIter, aMediaType ) )
                                aCnt.InsertGraphicStream( xStream, *pIter, aMediaType );
                        }
                        else
                        {
                            
                            InsertStreamIntoPicturesStorage_Impl( _xStorage, xStream, *pIter );
                        }
                    }
                }

                uno::Reference< embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
                if ( xPersist.is() )
                {
                    uno::Sequence< beans::PropertyValue > aArgs( _bOasisFormat ? 2 : 3 );
                    aArgs[0].Name = "StoreVisualReplacement";
                    aArgs[0].Value <<= !_bOasisFormat;

                    
                    aArgs[1].Name = "CanTryOptimization";
                    aArgs[1].Value <<= !_bCreateEmbedded;
                    if ( !_bOasisFormat )
                    {
                        
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
                    
                    xObj->changeState( embed::EmbedStates::LOADED );
            }
        }

        bResult = aCnt.CommitImageSubStorage();

    }
    catch (const uno::Exception& e)
    {
        
        bResult = false;
        SAL_WARN("comphelper.container", "failed. Message: " << e.Message);
    }

    
    if ( !_bOasisFormat && bResult )
    {
        try
        {
            
            OUString aObjReplElement( "ObjectReplacements" );
            if ( _xStorage->hasByName( aObjReplElement ) && _xStorage->isStorageElement( aObjReplElement ) )
                _xStorage->removeElement( aObjReplElement );
        }
        catch (const uno::Exception&)
        {
            
            bResult = false;
        }
    }
    return bResult;
}

bool EmbeddedObjectContainer::StoreChildren(bool _bOasisFormat,bool _bObjectsOnly)
{
    bool bResult = true;
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
                
                
                OUString aMediaType;

                
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

            
            
            
            
            
            
            

            uno::Reference< embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
            if ( xPersist.is() )
            {
                try
                {
                    
                    

            
                    
                    
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
                            
                        }
                    }
                    else 
                    {
                        xPersist->storeOwn();
                    }
                    
                }
                catch (const uno::Exception&)
                {
                    
                    bResult = false;
                    break;
                }
            }

            if ( !_bOasisFormat && !_bObjectsOnly )
            {
                
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
            
            bResult = false;
        }
    }
    return bResult;
}

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

bool EmbeddedObjectContainer::SetPersistentEntries(const uno::Reference< embed::XStorage >& _xStorage,bool _bClearModifedFlag)
{
    bool bError = false;
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
                    
                    bError = true;
                    break;
                }
            }
            if ( _bClearModifedFlag )
            {
                
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
