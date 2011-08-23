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

#include "oox/helper/containerhelper.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include "oox/helper/helper.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::container::XNameContainer;

namespace oox {

// ============================================================================

Reference< XIndexContainer > ContainerHelper::createIndexContainer( const Reference< XMultiServiceFactory >& rxFactory )
{
    Reference< XIndexContainer > xContainer;
    if( rxFactory.is() ) try
    {
        xContainer.set( rxFactory->createInstance( CREATE_OUSTRING( "com.sun.star.document.IndexedPropertyValues" ) ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xContainer.is(), "ContainerHelper::createIndexContainer - cannot create container" );
    return xContainer;
}

bool ContainerHelper::insertByIndex(
        const Reference< XIndexContainer >& rxIndexContainer,
        sal_Int32 nIndex, const Any& rObject )
{
    OSL_ENSURE( rxIndexContainer.is(), "ContainerHelper::insertByIndex - missing XIndexContainer interface" );
    bool bRet = false;
    try
    {
        rxIndexContainer->insertByIndex( nIndex, rObject );
        bRet = true;
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( bRet, "ContainerHelper::insertByIndex - cannot insert object" );
    return bRet;
}

Reference< XNameContainer > ContainerHelper::createNameContainer( const Reference< XMultiServiceFactory >& rxFactory )
{
    Reference< XNameContainer > xContainer;
    if( rxFactory.is() ) try
    {
        xContainer.set( rxFactory->createInstance( CREATE_OUSTRING( "com.sun.star.document.NamedPropertyValues" ) ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xContainer.is(), "ContainerHelper::createNameContainer - cannot create container" );
    return xContainer;
}

OUString ContainerHelper::getUnusedName(
        const Reference< XNameAccess >& rxNameAccess, const OUString& rSuggestedName,
        sal_Unicode cSeparator, sal_Int32 nFirstIndexToAppend )
{
    OSL_ENSURE( rxNameAccess.is(), "ContainerHelper::getUnusedName - missing XNameAccess interface" );

    OUString aNewName = rSuggestedName;
    sal_Int32 nIndex = nFirstIndexToAppend;
    while( rxNameAccess->hasByName( aNewName ) )
        aNewName = OUStringBuffer( rSuggestedName ).append( cSeparator ).append( nIndex++ ).makeStringAndClear();
    return aNewName;
}

bool ContainerHelper::insertByName(
        const Reference< XNameContainer >& rxNameContainer,
        const OUString& rName, const Any& rObject, bool bReplaceOldExisting )
{
    OSL_ENSURE( rxNameContainer.is(), "ContainerHelper::insertByName - missing XNameContainer interface" );
    bool bRet = false;
    try
    {
        if( bReplaceOldExisting && rxNameContainer->hasByName( rName ) )
            rxNameContainer->replaceByName( rName, rObject );
        else
            rxNameContainer->insertByName( rName, rObject );
        bRet = true;
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( bRet, "ContainerHelper::insertByName - cannot insert object" );
    return bRet;
}

OUString ContainerHelper::insertByUnusedName(
        const Reference< XNameContainer >& rxNameContainer,
        const OUString& rSuggestedName, sal_Unicode cSeparator,
        const Any& rObject, bool bRenameOldExisting )
{
    OSL_ENSURE( rxNameContainer.is(), "ContainerHelper::insertByUnusedName - missing XNameContainer interface" );

    // find an unused name
    Reference< XNameAccess > xNameAccess( rxNameContainer, UNO_QUERY );
    OUString aNewName = getUnusedName( xNameAccess, rSuggestedName, cSeparator );

    // rename existing object
    if( bRenameOldExisting && rxNameContainer->hasByName( rSuggestedName ) )
    {
        try
        {
            Any aOldObject = rxNameContainer->getByName( rSuggestedName );
            rxNameContainer->removeByName( rSuggestedName );
            rxNameContainer->insertByName( aNewName, aOldObject );
            aNewName = rSuggestedName;
        }
        catch( Exception& )
        {
            OSL_ENSURE( false, "ContainerHelper::insertByUnusedName - cannot rename old object" );
        }
    }

    // insert the new object and return its resulting name
    insertByName( rxNameContainer, aNewName, rObject );
    return aNewName;
}

// ============================================================================

ObjectContainer::ObjectContainer( const Reference< XMultiServiceFactory >& rxFactory, const OUString& rServiceName ) :
    mxFactory( rxFactory ),
    maServiceName( rServiceName ),
    mnIndex( 0 )
{
    OSL_ENSURE( mxFactory.is(), "ObjectContainer::ObjectContainer - missing service factory" );
}

ObjectContainer::~ObjectContainer()
{
}

bool ObjectContainer::hasObject( const OUString& rObjName ) const
{
    createContainer();
    return mxContainer.is() && mxContainer->hasByName( rObjName );
}

Any ObjectContainer::getObject( const OUString& rObjName ) const
{
    createContainer();
    if( mxContainer.is() ) try
    {
        return mxContainer->getByName( rObjName );
    }
    catch( Exception& )
    {
    }
    return Any();
}

OUString ObjectContainer::insertObject( const OUString& rObjName, const Any& rObj, bool bInsertByUnusedName )
{
    createContainer();
    if( mxContainer.is() )
    {
        if( bInsertByUnusedName )
            return ContainerHelper::insertByUnusedName( mxContainer, rObjName + OUString::valueOf( ++mnIndex ), ' ', rObj );
        if( ContainerHelper::insertByName( mxContainer, rObjName, rObj ) )
            return rObjName;
    }
    return OUString();
}

void ObjectContainer::createContainer() const
{
    if( !mxContainer.is() && mxFactory.is() ) try
    {
        mxContainer.set( mxFactory->createInstance( maServiceName ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( mxContainer.is(), "ObjectContainer::createContainer - container not found" );
}

// ============================================================================

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
