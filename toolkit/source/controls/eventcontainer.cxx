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

#include <cppuhelper/factory.hxx>

#include <controls/eventcontainer.hxx>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::script;
using namespace cppu;

namespace toolkit
{

// Methods XElementAccess
Type ScriptEventContainer::getElementType()
{
    return mType;
}

sal_Bool ScriptEventContainer::hasElements()
{
    return !mHashMap.empty();
}

// Methods XNameAccess
Any ScriptEventContainer::getByName( const OUString& aName )
{
    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    if( aIt == mHashMap.end() )
    {
        throw NoSuchElementException();
    }
    sal_Int32 iHashResult = (*aIt).second;
    Any aRetAny = mValues[ iHashResult ];
    return aRetAny;
}

Sequence< OUString > ScriptEventContainer::getElementNames()
{
    return mNames;
}

sal_Bool ScriptEventContainer::hasByName( const OUString& aName )
{
    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    bool bRet = ( aIt != mHashMap.end() );
    return bRet;
}


// Methods XNameReplace
void ScriptEventContainer::replaceByName( const OUString& aName, const Any& aElement )
{
    const Type& aAnyType = aElement.getValueType();
    if( mType != aAnyType )
        throw IllegalArgumentException();

    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    if( aIt == mHashMap.end() )
    {
        throw NoSuchElementException();
    }

    ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Accessor <<= aName;
    aEvent.Element = aElement;

    sal_Int32 iHashResult = (*aIt).second;
    aEvent.ReplacedElement = mValues[ iHashResult ];
    mValues[ iHashResult ] = aElement;

    // Fire event
    maContainerListeners.elementReplaced( aEvent );
}

// Methods XNameContainer
void ScriptEventContainer::insertByName( const OUString& aName, const Any& aElement )
{
    const Type& aAnyType = aElement.getValueType();
    if( mType != aAnyType )
        throw IllegalArgumentException();

    NameContainerNameMap::iterator aIt = mHashMap.find( aName );
    if( aIt != mHashMap.end() )
    {
        throw ElementExistException();
    }

    sal_Int32 nCount = mNames.getLength();
    mNames.realloc( nCount + 1 );
    mValues.resize( nCount + 1 );
    mNames.getArray()[ nCount ] = aName;
    mValues[ nCount ] = aElement;
    mHashMap[ aName ] = nCount;

    // Fire event
    ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Element = aElement;
    aEvent.Accessor <<= aName;
    maContainerListeners.elementInserted( aEvent );
}

void ScriptEventContainer::removeByName( const OUString& Name )
{
    NameContainerNameMap::iterator aIt = mHashMap.find( Name );
    if( aIt == mHashMap.end() )
    {
        throw NoSuchElementException();
    }

    sal_Int32 iHashResult = (*aIt).second;

    // Fire event
    ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Element = mValues[iHashResult];
    aEvent.Accessor <<= Name;
    maContainerListeners.elementRemoved( aEvent );

    mHashMap.erase( aIt );
    sal_Int32 iLast = mNames.getLength() - 1;
    if( iLast != iHashResult )
    {
        OUString* pNames = mNames.getArray();
        pNames[ iHashResult ] = pNames[ iLast ];
        mValues[ iHashResult ] = mValues[ iLast ];
        mHashMap[ pNames[ iHashResult ] ] = iHashResult;
    }
    mNames.realloc( iLast );
    mValues.resize( iLast );
}

// Methods XContainer
void ScriptEventContainer::addContainerListener( const css::uno::Reference< css::container::XContainerListener >& l )
{
    maContainerListeners.addInterface( l );
}

void ScriptEventContainer::removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& l )
{
    maContainerListeners.removeInterface( l );
}


ScriptEventContainer::ScriptEventContainer()
    : mType( cppu::UnoType<ScriptEventDescriptor>::get() ),
      maContainerListeners( *this )
{
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
