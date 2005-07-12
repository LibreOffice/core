/*************************************************************************
 *
 *  $RCSfile: embeddedobjectcontainer.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 12:27:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDOBJECTCREATOR_HPP_
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XLINKCREATOR_HPP_
#include <com/sun/star/embed/XLinkCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XLINKAGESUPPORT_HPP_
#include <com/sun/star/embed/XLinkageSupport.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif

#include <comphelper/seqstream.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <hash_map>

using namespace ::com::sun::star;

namespace comphelper
{

struct hashObjectName_Impl
{
    size_t operator()(const ::rtl::OUString Str) const
    {
        return (size_t)Str.hashCode();
    }
};

struct eqObjectName_Impl
{
    sal_Bool operator()(const ::rtl::OUString Str1, const ::rtl::OUString Str2) const
    {
        return ( Str1 == Str2 );
    }
};

typedef std::hash_map
<
    ::rtl::OUString,
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
    //EmbeddedObjectContainerNameMap maTempObjectContainer;
    //uno::Reference < embed::XStorage > mxTempStorage;
    sal_Bool bOwnsStorage;

    const uno::Reference < embed::XStorage >& GetReplacements();
};

const uno::Reference < embed::XStorage >& EmbedImpl::GetReplacements()
{
    if ( !mxImageStorage.is() )
    {
        try
        {
            mxImageStorage = mxStorage->openStorageElement(
                ::rtl::OUString::createFromAscii( "ObjectReplacements" ), embed::ElementModes::READWRITE );
        }
        catch ( uno::Exception& )
        {
            mxImageStorage = mxStorage->openStorageElement(
                ::rtl::OUString::createFromAscii( "ObjectReplacements" ), embed::ElementModes::READ );
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
    pImpl->bOwnsStorage = sal_True;
    pImpl->mpTempObjectContainer = 0;
}

EmbeddedObjectContainer::EmbeddedObjectContainer( const uno::Reference < embed::XStorage >& rStor )
{
    pImpl = new EmbedImpl;
    pImpl->mxStorage = rStor;
    pImpl->bOwnsStorage = sal_False;
    pImpl->mpTempObjectContainer = 0;
}

void EmbeddedObjectContainer::SwitchPersistence( const uno::Reference < embed::XStorage >& rStor )
{
    ReleaseImageSubStorage();

    if ( pImpl->bOwnsStorage )
        pImpl->mxStorage->dispose();

    pImpl->mxStorage = rStor;
    pImpl->bOwnsStorage = sal_False;
}

sal_Bool EmbeddedObjectContainer::CommitImageSubStorage()
{
    if ( pImpl->mxImageStorage.is() )
    {
        try
        {
            uno::Reference< embed::XTransactedObject > xTransact( pImpl->mxImageStorage, uno::UNO_QUERY_THROW );
            xTransact->commit();
        }
        catch( uno::Exception& )
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
        catch( uno::Exception& )
        {
            OSL_ASSERT( "Problems releasing image substorage!\n" );
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
            catch ( uno::Exception& )
            {
            }
        }

        aIt++;
    }
}

::rtl::OUString EmbeddedObjectContainer::CreateUniqueObjectName()
{
    ::rtl::OUString aPersistName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Object ") );
    ::rtl::OUString aStr;
    sal_Int32 i=1;
    do
    {
        aStr = aPersistName;
        aStr += ::rtl::OUString::valueOf( i++ );
    }
    while( HasEmbeddedObject( aStr ) );
    // TODO/LATER: should we consider deleted objects?

    return aStr;
}

uno::Sequence < ::rtl::OUString > EmbeddedObjectContainer::GetObjectNames()
{
    uno::Sequence < ::rtl::OUString > aSeq( pImpl->maObjectContainer.size() );
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

sal_Bool EmbeddedObjectContainer::HasEmbeddedObject( const ::rtl::OUString& rName )
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
            aIt++;
    }

    return sal_False;
}

::rtl::OUString EmbeddedObjectContainer::GetEmbeddedObjectName( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xObj )
{
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        if ( (*aIt).second == xObj )
            return (*aIt).first;
        else
            aIt++;
    }

    OSL_ENSURE( 0, "Unknown object!" );
    return ::rtl::OUString();
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::GetEmbeddedObject( const ::rtl::OUString& rName )
{
    OSL_ENSURE( rName.getLength(), "Empty object name!");

    uno::Reference < embed::XEmbeddedObject > xObj;
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.find( rName );

#if OSL_DEBUG_LEVEL > 1
    uno::Reference < container::XNameAccess > xAccess( pImpl->mxStorage, uno::UNO_QUERY );
    OSL_ENSURE( aIt != pImpl->maObjectContainer.end() || xAccess->hasByName(rName), "Could not return object!" );
#endif

    try
    {
        // check if object was already created
        if ( aIt != pImpl->maObjectContainer.end() )
            xObj = (*aIt).second;
        else
        {
            // create the object from the storage
            uno::Reference < beans::XPropertySet > xSet( pImpl->mxStorage, uno::UNO_QUERY );
            sal_Bool bReadOnlyMode = sal_True;
            if ( xSet.is() )
            {
                // get the open mode from the parent storage
                sal_Int32 nMode;
                uno::Any aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("OpenMode") );
                if ( aAny >>= nMode )
                    bReadOnlyMode = !(nMode & embed::ElementModes::WRITE );
            }

            // object was not added until now - should happen only by calling this method from "inside"
            //TODO/LATER: it would be good to detect an error when an object should be created already, but isn't (not an "inside" call)
            uno::Reference < embed::XEmbedObjectCreator > xFactory( ::comphelper::getProcessServiceFactory()->createInstance(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.EmbeddedObjectCreator")) ), uno::UNO_QUERY );
            xObj = uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceInitFromEntry(
                    pImpl->mxStorage, rName,
                    bReadOnlyMode, uno::Sequence < beans::PropertyValue >() ), uno::UNO_QUERY );

            // insert object into my list
            AddEmbeddedObject( xObj, rName );
        }
    }
    catch ( uno::Exception& )
    {
    }

    return xObj;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::CreateEmbeddedObject( const uno::Sequence < sal_Int8 >& rClassId, ::rtl::OUString& rNewName )
{
    if ( !rNewName.getLength() )
        rNewName = CreateUniqueObjectName();

    OSL_ENSURE( !HasEmbeddedObject(rNewName), "Object to create already exists!");

    // create object from classid by inserting it into storage
    uno::Reference < embed::XEmbeddedObject > xObj;
    try
    {
        uno::Reference < embed::XEmbedObjectCreator > xFactory( ::comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.EmbeddedObjectCreator")) ), uno::UNO_QUERY );

        xObj = uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceInitNew(
                    rClassId, ::rtl::OUString(), pImpl->mxStorage, rNewName,
                    uno::Sequence < beans::PropertyValue >() ), uno::UNO_QUERY );

        AddEmbeddedObject( xObj, rNewName );

        // a freshly created object should be running always
        if ( xObj.is() && xObj->getCurrentState() == embed::EmbedStates::LOADED )
            xObj->changeState( embed::EmbedStates::RUNNING );
    }
    catch ( uno::Exception& )
    {
    }

    return xObj;
}

void EmbeddedObjectContainer::AddEmbeddedObject( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xObj, const ::rtl::OUString& rName )
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE( rName.getLength(), "Added object doesn't have a name!");
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

    // look for object in temorary container
    if ( pImpl->mpTempObjectContainer )
    {
        aIt = pImpl->mpTempObjectContainer->pImpl->maObjectContainer.begin();
        while ( aIt != pImpl->mpTempObjectContainer->pImpl->maObjectContainer.end() )
        {
            if ( (*aIt).second == xObj )
            {
                // copy replacement image from temporary container (if there is any)
                ::rtl::OUString aTempName = (*aIt).first;
                ::rtl::OUString aMediaType;
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
                    catch ( uno::Exception& )
                    {
                    }
                }

                // temp. container needs to forget the object
                pImpl->mpTempObjectContainer->pImpl->maObjectContainer.erase( aIt );
                break;
            }
            else
                aIt++;
        }
    }

    //TODO/LATER: set parent of either object or component through XChild
}

sal_Bool EmbeddedObjectContainer::StoreEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj, ::rtl::OUString& rName, sal_Bool bCopy )
{
    uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
    if ( !rName.getLength() )
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
    catch ( uno::Exception& )
    {
        // TODO/LATER: better error recovery should keep storage intact
        return sal_False;
    }

    return sal_True;
}

sal_Bool EmbeddedObjectContainer::InsertEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj, ::rtl::OUString& rName )
{
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

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::InsertEmbeddedObject( const uno::Reference < io::XInputStream >& xStm, ::rtl::OUString& rNewName )
{
    if ( !rNewName.getLength() )
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
    catch ( uno::Exception& )
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
            xProps->setPropertyValue(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                    uno::makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.star.oleobject" ) ) ) );
        }
        catch ( uno::Exception& )
        {
            // complete disaster!
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
        else
            // freshly created objects always should be running!
            xRet->changeState( embed::EmbedStates::RUNNING );
    }
    catch ( uno::Exception& )
    {
    }

    return xRet;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::InsertEmbeddedObject( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& aMedium, ::rtl::OUString& rNewName )
{
    if ( !rNewName.getLength() )
        rNewName = CreateUniqueObjectName();

    uno::Reference < embed::XEmbeddedObject > xObj;
    try
    {
        uno::Reference < embed::XEmbedObjectCreator > xFactory( ::comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.EmbeddedObjectCreator")) ), uno::UNO_QUERY );
        xObj = uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceInitFromMediaDescriptor(
                pImpl->mxStorage, rNewName, aMedium, uno::Sequence < beans::PropertyValue >() ), uno::UNO_QUERY );
        uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );

        // freshly created objects always should be running!
        xObj->changeState( embed::EmbedStates::RUNNING );

        // possible optimization: store later!
        if ( xPersist.is())
            xPersist->storeOwn();

        AddEmbeddedObject( xObj, rNewName );
    }
    catch ( uno::Exception& )
    {
    }

    return xObj;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::InsertEmbeddedLink( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& aMedium, ::rtl::OUString& rNewName )
{
    if ( !rNewName.getLength() )
        rNewName = CreateUniqueObjectName();

    uno::Reference < embed::XEmbeddedObject > xObj;
    try
    {
        uno::Reference < embed::XLinkCreator > xFactory( ::comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.EmbeddedObjectCreator")) ), uno::UNO_QUERY );
        xObj = uno::Reference < embed::XEmbeddedObject >( xFactory->createInstanceLink(
                pImpl->mxStorage, rNewName, aMedium, uno::Sequence < beans::PropertyValue >() ), uno::UNO_QUERY );

        // a freshly created object should be in running state
        if ( xObj.is() && xObj->getCurrentState() == embed::EmbedStates::LOADED )
        {
            uno::Sequence< sal_Int32 > aSupportedStates = xObj->getReachableStates();
            for ( sal_Int32 nInd = 0; nInd < aSupportedStates.getLength(); nInd++ )
                if ( aSupportedStates[nInd] == embed::EmbedStates::RUNNING )
                {
                    xObj->changeState( embed::EmbedStates::RUNNING );
                    break;
                }
        }

        AddEmbeddedObject( xObj, rNewName );
    }
    catch ( uno::Exception& )
    {
    }

    return xObj;
}

sal_Bool EmbeddedObjectContainer::TryToCopyGraphReplacement( EmbeddedObjectContainer& rSrc,
                                                            const ::rtl::OUString& aOrigName,
                                                            const ::rtl::OUString& aTargetName )
{
    sal_Bool bResult = sal_False;

    if ( &rSrc != this && aOrigName.getLength() && aTargetName.getLength() )
    {
        ::rtl::OUString aMediaType;
        uno::Reference < io::XInputStream > xGrStream = rSrc.GetGraphicStream( aOrigName, &aMediaType );
        if ( xGrStream.is() )
            bResult = InsertGraphicStream( xGrStream, aTargetName, aMediaType );
    }

    return bResult;
}

sal_Bool EmbeddedObjectContainer::CopyEmbeddedObject( EmbeddedObjectContainer& rSrc, const uno::Reference < embed::XEmbeddedObject >& xObj, ::rtl::OUString& rName )
{
    // get the object name before(!) it is assigned to a new storage
    ::rtl::OUString aOrigName;
    uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
    if ( xPersist.is() )
        aOrigName = xPersist->getEntryName();

    if ( !rName.getLength() )
        rName = CreateUniqueObjectName();

    if ( StoreEmbeddedObject( xObj, rName, sal_True ) )
    {
        TryToCopyGraphReplacement( rSrc, aOrigName, rName );
        return sal_True;
    }

    return sal_False;
}

uno::Reference < embed::XEmbeddedObject > EmbeddedObjectContainer::CopyAndGetEmbeddedObject( EmbeddedObjectContainer& rSrc, const uno::Reference < embed::XEmbeddedObject >& xObj, ::rtl::OUString& rName )
{
    uno::Reference< embed::XEmbeddedObject > xResult;

    // TODO/LATER: For now only objects that implement XEmbedPersist have a replacement image, it might change in future
    // do an incompatible change so that object name is provided in all the move and copy methods
    ::rtl::OUString aOrigName;
    try
    {
        uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY_THROW );
        aOrigName = xPersist->getEntryName();
    }
    catch( uno::Exception& )
    {}

    if ( !rName.getLength() )
        rName = CreateUniqueObjectName();

    // objects without persistance are not really stored by the method
       if ( xObj.is() && StoreEmbeddedObject( xObj, rName, sal_True ) )
    {
        xResult = GetEmbeddedObject( rName );

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
                    ::rtl::OUString aURL = xOrigLinkage->getLinkURL();
                    if ( !aURL.getLength() )
                        throw uno::RuntimeException();

                    // create new linked object from the URL the link is based on
                    uno::Reference < embed::XLinkCreator > xCreator(
                        ::comphelper::getProcessServiceFactory()->createInstance(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.EmbeddedObjectCreator") ) ),
                        uno::UNO_QUERY_THROW );

                    uno::Sequence< beans::PropertyValue > aMediaDescr( 1 );
                    aMediaDescr[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
                    aMediaDescr[0].Value <<= aURL;
                    xResult = uno::Reference < embed::XEmbeddedObject >(
                                xCreator->createInstanceLink(
                                    pImpl->mxStorage,
                                    rName,
                                    aMediaDescr,
                                    uno::Sequence < beans::PropertyValue >() ),
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
                    uno::Reference < embed::XEmbedObjectCreator > xCreator(
                        ::comphelper::getProcessServiceFactory()->createInstance(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.EmbeddedObjectCreator") ) ),
                        uno::UNO_QUERY_THROW );

                    xResult = uno::Reference < embed::XEmbeddedObject >(
                                xCreator->createInstanceInitNew(
                                    xObj->getClassID(),
                                    xObj->getClassName(),
                                    pImpl->mxStorage,
                                    rName,
                                    uno::Sequence < beans::PropertyValue >() ),
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
                        catch( beans::PropertyVetoException& )
                        {
                            // impossibility to copy readonly property is not treated as an error for now
                            // but the assertion is helpful to detect such scenarios and review them
                            OSL_ENSURE( sal_False, "Could not copy readonly property!\n" );
                        }
                    }
                }

                   if ( xResult.is() )
                    AddEmbeddedObject( xResult, rName );
            }
            catch( uno::Exception& )
            {
                if ( xResult.is() )
                {
                    try
                    {
                        xResult->close( sal_True );
                    }
                    catch( uno::Exception& )
                    {}
                    xResult = uno::Reference< embed::XEmbeddedObject >();
                }
            }
        }
    }

    OSL_ENSURE( xResult.is(), "Can not copy embedded object that has no persistance!\n" );

    // the object is successfully copied, try to copy graphical replacement
    if ( xResult.is() && aOrigName.getLength() )
        TryToCopyGraphReplacement( rSrc, aOrigName, rName );

    return xResult;
}

sal_Bool EmbeddedObjectContainer::MoveEmbeddedObject( EmbeddedObjectContainer& rSrc, const uno::Reference < embed::XEmbeddedObject >& xObj, ::rtl::OUString& rName )
{
    // get the object name before(!) it is assigned to a new storage
    uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
    ::rtl::OUString aName;
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
    catch ( uno::Exception& e )
    {
        (void)e;
        OSL_ENSURE( sal_False, "Failed to insert embedded object into storage!" );
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

            aIt++;
        }

        OSL_ENSURE( bRet, "Object not found for removal!" );
        if ( xPersist.is() )
        {
            // now it's time to remove the storage from the container storage
            try
            {
                if ( xPersist.is() )
                    rSrc.pImpl->mxStorage->removeElement( aName );
            }
            catch ( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "Failed to remove object from storage!" );
                bRet = sal_False;
            }
        }

        // rSrc.RemoveGraphicStream( aName );
    }

    return bRet;
}

sal_Bool EmbeddedObjectContainer::RemoveEmbeddedObject( const ::rtl::OUString& rName, sal_Bool bClose )
{
    uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObject( rName );
    if ( xObj.is() )
        return RemoveEmbeddedObject( xObj, bClose );
    else
        return sal_False;
}

sal_Bool EmbeddedObjectContainer::MoveEmbeddedObject( const ::rtl::OUString& rName, EmbeddedObjectContainer& rCnt )
{
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
                ::rtl::OUString aName( rName );
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
        catch ( uno::Exception& )
        {
            OSL_ENSURE(0,"Could not move object!");
            return sal_False;
        }

    }
    else
        OSL_ENSURE(0,"Unknown object!");
    return sal_False;
}

sal_Bool EmbeddedObjectContainer::RemoveEmbeddedObject( const uno::Reference < embed::XEmbeddedObject >& xObj, sal_Bool bClose )
{
    uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
    ::rtl::OUString aName;
    if ( xPersist.is() )
        aName = xPersist->getEntryName();

#if OSL_DEBUG_LEVEL > 1
    uno::Reference < container::XNameAccess > xAccess( pImpl->mxStorage, uno::UNO_QUERY );
    uno::Reference < embed::XLinkageSupport > xLink( xPersist, uno::UNO_QUERY );
    sal_Bool bIsNotEmbedded = !xPersist.is() || xLink.is() && xLink->isLink();

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
        catch ( util::CloseVetoException& )
        {
            bClose = sal_False;
        }
    }

    if ( !bClose )
    {
        // somebody still needs the object, so we must assign a temporary persistence
        try
        {
            if ( xPersist.is() )
            {
                /*
                //TODO/LATER: needs storage handling!  Why not letting the object do it?!
                if ( !pImpl->mxTempStorage.is() )
                    pImpl->mxTempStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
                uno::Sequence < beans::PropertyValue > aSeq;

                ::rtl::OUString aTmpPersistName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Object ") );
                aTmpPersistName += ::rtl::OUString::valueOf( (sal_Int32) pImpl->maTempObjectContainer.size() );

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
                        ::rtl::OUString aOrigStorMediaType;
                        uno::Reference< beans::XPropertySet > xStorProps( pImpl->mxStorage, uno::UNO_QUERY_THROW );
                        xStorProps->getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) )
                                        >>= aOrigStorMediaType;

                        OSL_ENSURE( aOrigStorMediaType.getLength(), "No valuable media type in the storage!\n" );

                        uno::Reference< beans::XPropertySet > xTargetStorProps(
                                                                    pImpl->mpTempObjectContainer->pImpl->mxStorage,
                                                                    uno::UNO_QUERY_THROW );
                        xTargetStorProps->setPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ),
                                                            uno::makeAny( aOrigStorMediaType ) );
                    }
                    catch( uno::Exception& )
                    {
                        OSL_ENSURE( sal_False, "Can not set the new media type to a storage!\n" );
                    }
                }

                ::rtl::OUString aTempName, aMediaType;
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
        catch ( uno::Exception& )
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
            break;
        }

        aIt++;
    }

    OSL_ENSURE( bFound, "Object not found for removal!" );
    if ( xPersist.is() )
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
        catch ( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "Failed to remove object from storage!" );
            return sal_False;
        }
    }

    return sal_True;
}

uno::Reference < io::XInputStream > EmbeddedObjectContainer::GetGraphicStream( const ::rtl::OUString& aName, rtl::OUString* pMediaType )
{
    uno::Reference < io::XInputStream > xStream;

    OSL_ENSURE( aName.getLength(), "Retrieving graphic for unknown object!" );
    if ( aName.getLength() )
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
                    uno::Any aAny = xSet->getPropertyValue( ::rtl::OUString::createFromAscii("MediaType") );
                    aAny >>= *pMediaType;
                }
            }
        }
        catch ( uno::Exception& )
        {
        }
    }

    return xStream;
}

uno::Reference < io::XInputStream > EmbeddedObjectContainer::GetGraphicStream( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XEmbeddedObject >& xObj, rtl::OUString* pMediaType )
{
    // get the object name
    ::rtl::OUString aName;
    EmbeddedObjectContainerNameMap::iterator aIt = pImpl->maObjectContainer.begin();
    while ( aIt != pImpl->maObjectContainer.end() )
    {
        if ( (*aIt).second == xObj )
        {
            aName = (*aIt).first;
            break;
        }

        aIt++;
    }

    // try to load it from the container storage
    return GetGraphicStream( aName, pMediaType );
}

sal_Bool EmbeddedObjectContainer::InsertGraphicStream( const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& rStream, const ::rtl::OUString& rObjectName, const rtl::OUString& rMediaType )
{
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

        xPropSet->setPropertyValue( ::rtl::OUString::createFromAscii( "UseCommonStoragePasswordEncryption" ),
                                    uno::makeAny( (sal_Bool)sal_True ) );
        uno::Any aAny;
        aAny <<= rMediaType;
        xPropSet->setPropertyValue( ::rtl::OUString::createFromAscii("MediaType"), aAny );
    }
    catch( uno::Exception& )
    {
        return sal_False;
    }

    return sal_True;
}

sal_Bool EmbeddedObjectContainer::RemoveGraphicStream( const ::rtl::OUString& rObjectName )
{
    try
    {
        uno::Reference < embed::XStorage > xReplacements = pImpl->GetReplacements();
        xReplacements->removeElement( rObjectName );
    }
    catch( uno::Exception& )
    {
        return sal_False;
    }

    return sal_True;
}

}
