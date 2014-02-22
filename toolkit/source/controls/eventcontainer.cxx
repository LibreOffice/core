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

#include <osl/mutex.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include "toolkit/controls/eventcontainer.hxx"
#include <com/sun/star/script/ScriptEventDescriptor.hpp>


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::registry;
using namespace com::sun::star::script;
using namespace cppu;
using namespace osl;
using namespace std;


namespace toolkit
{


Type NameContainer_Impl::getElementType()
    throw(RuntimeException)
{
    return mType;
}

sal_Bool NameContainer_Impl::hasElements()
    throw(RuntimeException)
{
    sal_Bool bRet = (mnElementCount > 0);
    return bRet;
}


Any NameContainer_Impl::getByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    if( aIt == mHashMap.end() )
    {
        throw NoSuchElementException();
    }
    sal_Int32 iHashResult = (*aIt).second;
    Any aRetAny = mValues.getConstArray()[ iHashResult ];
    return aRetAny;
}

Sequence< OUString > NameContainer_Impl::getElementNames()
    throw(RuntimeException)
{
    return mNames;
}

sal_Bool NameContainer_Impl::hasByName( const OUString& aName )
    throw(RuntimeException)
{
    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    sal_Bool bRet = ( aIt != mHashMap.end() );
    return bRet;
}



void NameContainer_Impl::replaceByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    Type aAnyType = aElement.getValueType();
    if( mType != aAnyType )
        throw IllegalArgumentException();

    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    if( aIt == mHashMap.end() )
    {
        throw NoSuchElementException();
    }
    sal_Int32 iHashResult = (*aIt).second;
    Any aOldElement = mValues.getConstArray()[ iHashResult ];
    mValues.getArray()[ iHashResult ] = aElement;

    
    ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Element <<= aElement;
    aEvent.ReplacedElement = aOldElement;
    aEvent.Accessor <<= aName;
    maContainerListeners.elementReplaced( aEvent );
}



void NameContainer_Impl::insertByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    Type aAnyType = aElement.getValueType();
    if( mType != aAnyType )
        throw IllegalArgumentException();

    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    if( aIt != mHashMap.end() )
    {
        throw ElementExistException();
    }

    sal_Int32 nCount = mNames.getLength();
    mNames.realloc( nCount + 1 );
    mValues.realloc( nCount + 1 );
    mNames.getArray()[ nCount ] = aName;
    mValues.getArray()[ nCount ] = aElement;
    mHashMap[ aName ] = nCount;

    
    ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Element <<= aElement;
    aEvent.Accessor <<= aName;
    maContainerListeners.elementInserted( aEvent );
}

void NameContainer_Impl::removeByName( const OUString& Name )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    NameContainerNameMap::iterator aIt = mHashMap.find( Name );
    if( aIt == mHashMap.end() )
    {
        throw NoSuchElementException();
    }

    sal_Int32 iHashResult = (*aIt).second;
    Any aOldElement = mValues.getConstArray()[ iHashResult ];

    
    ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Element = aOldElement;
    aEvent.Accessor <<= Name;
    maContainerListeners.elementRemoved( aEvent );

    mHashMap.erase( aIt );
    sal_Int32 iLast = mNames.getLength() - 1;
    if( iLast != iHashResult )
    {
        OUString* pNames = mNames.getArray();
        Any* pValues = mValues.getArray();
        pNames[ iHashResult ] = pNames[ iLast ];
        pValues[ iHashResult ] = pValues[ iLast ];
        mHashMap[ pNames[ iHashResult ] ] = iHashResult;
    }
    mNames.realloc( iLast );
    mValues.realloc( iLast );

}


void NameContainer_Impl::addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l ) throw(::com::sun::star::uno::RuntimeException)
{
    maContainerListeners.addInterface( l );
}

void NameContainer_Impl::removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l ) throw(::com::sun::star::uno::RuntimeException)
{
    maContainerListeners.removeInterface( l );
}




ScriptEventContainer::ScriptEventContainer( void )
    : NameContainer_Impl( getCppuType( (ScriptEventDescriptor*) NULL ) )
{
}

}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
