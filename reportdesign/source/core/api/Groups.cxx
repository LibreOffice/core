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
#include "Groups.hxx"
#include "Group.hxx"
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <tools/debug.hxx>
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <algorithm>

namespace reportdesign
{

    using namespace com::sun::star;

OGroups::OGroups(const uno::Reference< report::XReportDefinition >& _xParent,const uno::Reference< uno::XComponentContext >& context)
:GroupsBase(m_aMutex)
,m_aContainerListeners(m_aMutex)
,m_xContext(context)
,m_xParent(_xParent)
{
}

// TODO: VirtualFunctionFinder: This is virtual function!

OGroups::~OGroups()
{
}

void SAL_CALL OGroups::dispose()
{
    cppu::WeakComponentImplHelperBase::dispose();
}

// TODO: VirtualFunctionFinder: This is virtual function!

void SAL_CALL OGroups::disposing()
{
    for(auto& rGroup : m_aGroups)
        rGroup->dispose();
    m_aGroups.clear();
    lang::EventObject aDisposeEvent( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aContainerListeners.disposeAndClear( aDisposeEvent );
    m_xContext.clear();
}

// XGroups
uno::Reference< report::XReportDefinition > SAL_CALL OGroups::getReportDefinition()
{
    return m_xParent;
}

uno::Reference< report::XGroup > SAL_CALL OGroups::createGroup(  )
{
    return new OGroup(this,m_xContext);
}

// XIndexContainer
void SAL_CALL OGroups::insertByIndex( ::sal_Int32 Index, const uno::Any& aElement )
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        bool bAdd = (Index == static_cast<sal_Int32>(m_aGroups.size()));
        if ( !bAdd )
            checkIndex(Index);
        uno::Reference< report::XGroup > xGroup(aElement,uno::UNO_QUERY);
        if ( !xGroup.is() )
            throw lang::IllegalArgumentException(RPT_RESSTRING(RID_STR_ARGUMENT_IS_NULL),*this,2);

        if ( bAdd )
            m_aGroups.push_back(xGroup);
        else
        {
            TGroups::iterator aPos = m_aGroups.begin();
            ::std::advance(aPos,Index);
            m_aGroups.insert(aPos, xGroup);
        }
    }
    // notify our container listeners
    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), aElement, uno::Any());
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementInserted,aEvent);
}


void SAL_CALL OGroups::removeByIndex( ::sal_Int32 Index )
{
    uno::Reference< report::XGroup > xGroup;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        checkIndex(Index);
        TGroups::iterator aPos = m_aGroups.begin();
        ::std::advance(aPos,Index);
        xGroup = *aPos;
        m_aGroups.erase(aPos);
    }
    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), uno::makeAny(xGroup), uno::Any());
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementRemoved,aEvent);
}

// XIndexReplace
void SAL_CALL OGroups::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element )
{
    uno::Any aOldElement;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        checkIndex(Index);
        uno::Reference< report::XGroup > xGroup(Element,uno::UNO_QUERY);
        if ( !xGroup.is() )
            throw lang::IllegalArgumentException(RPT_RESSTRING(RID_STR_ARGUMENT_IS_NULL),*this,2);
        TGroups::iterator aPos = m_aGroups.begin();
        ::std::advance(aPos,Index);
        aOldElement <<= *aPos;
        *aPos = xGroup;
    }

    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), Element, aOldElement);
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementReplaced,aEvent);
}

// XIndexAccess
::sal_Int32 SAL_CALL OGroups::getCount(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aGroups.size();
}

uno::Any SAL_CALL OGroups::getByIndex( ::sal_Int32 Index )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkIndex(Index);
    TGroups::const_iterator aPos = m_aGroups.begin();
    ::std::advance(aPos,Index);
    return uno::makeAny(*aPos);
}

// XElementAccess
uno::Type SAL_CALL OGroups::getElementType(  )
{
    return cppu::UnoType<report::XGroup>::get();
}

sal_Bool SAL_CALL OGroups::hasElements(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return !m_aGroups.empty();
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL OGroups::getParent(  )
{
    return m_xParent;
}

void SAL_CALL OGroups::setParent( const uno::Reference< uno::XInterface >& /*Parent*/ )
{
    throw lang::NoSupportException();
}

// XContainer
void SAL_CALL OGroups::addContainerListener( const uno::Reference< container::XContainerListener >& xListener )
{
    m_aContainerListeners.addInterface(xListener);
}

void SAL_CALL OGroups::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener )
{
    m_aContainerListeners.removeInterface(xListener);
}

void OGroups::checkIndex(sal_Int32 _nIndex)
{
    if ( _nIndex < 0 || static_cast<sal_Int32>(m_aGroups.size()) <= _nIndex )
        throw lang::IndexOutOfBoundsException();
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
