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
#include "Functions.hxx"
#include "Function.hxx"
#include <tools/debug.hxx>
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <com/sun/star/lang/NoSupportException.hpp>
#include <comphelper/property.hxx>
#include <algorithm>

namespace reportdesign
{

    using namespace com::sun::star;

OFunctions::OFunctions(const uno::Reference< report::XFunctionsSupplier >& _xParent,const uno::Reference< uno::XComponentContext >& context)
:FunctionsBase(m_aMutex)
,m_aContainerListeners(m_aMutex)
,m_xContext(context)
,m_xParent(_xParent)
{
}

// TODO: VirtualFunctionFinder: This is virtual function!

OFunctions::~OFunctions()
{
}

void SAL_CALL OFunctions::dispose()
{
    cppu::WeakComponentImplHelperBase::dispose();
}

// TODO: VirtualFunctionFinder: This is virtual function!

void SAL_CALL OFunctions::disposing()
{
    for (auto& rFunction : m_aFunctions)
        rFunction->dispose();
    m_aFunctions.clear();
    lang::EventObject aDisposeEvent( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aContainerListeners.disposeAndClear( aDisposeEvent );
    m_xContext.clear();
}

// XFunctionsSupplier

uno::Reference< report::XFunction > SAL_CALL OFunctions::createFunction(  )
{
    return new OFunction(m_xContext);
}

// XIndexContainer
void SAL_CALL OFunctions::insertByIndex( ::sal_Int32 Index, const uno::Any& aElement )
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        bool bAdd = (Index == static_cast<sal_Int32>(m_aFunctions.size()));
        if ( !bAdd )
            checkIndex(Index);
        uno::Reference< report::XFunction > xFunction(aElement,uno::UNO_QUERY);
        if ( !xFunction.is() )
            throw lang::IllegalArgumentException(RPT_RESSTRING(RID_STR_ARGUMENT_IS_NULL),*this,2);

        if ( bAdd )
            m_aFunctions.push_back(xFunction);
        else
        {
            TFunctions::iterator aPos = m_aFunctions.begin();
            ::std::advance(aPos,Index);
            m_aFunctions.insert(aPos, xFunction);
        }
        xFunction->setParent(*this);
    }
    // notify our container listeners
    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), aElement, uno::Any());
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementInserted,aEvent);
}


void SAL_CALL OFunctions::removeByIndex( ::sal_Int32 Index )
{
    uno::Reference< report::XFunction > xFunction;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        checkIndex(Index);
        TFunctions::iterator aPos = m_aFunctions.begin();
        ::std::advance(aPos,Index);
        xFunction = *aPos;
        m_aFunctions.erase(aPos);
        xFunction->setParent(nullptr);
    }
    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), uno::makeAny(xFunction), uno::Any());
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementRemoved,aEvent);
}

// XIndexReplace
void SAL_CALL OFunctions::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element )
{
    uno::Any aOldElement;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        checkIndex(Index);
        uno::Reference< report::XFunction > xFunction(Element,uno::UNO_QUERY);
        if ( !xFunction.is() )
            throw lang::IllegalArgumentException(RPT_RESSTRING(RID_STR_ARGUMENT_IS_NULL),*this,2);
        TFunctions::iterator aPos = m_aFunctions.begin();
        ::std::advance(aPos,Index);
        aOldElement <<= *aPos;
        *aPos = xFunction;
    }

    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), Element, aOldElement);
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementReplaced,aEvent);
}

// XIndexAccess
::sal_Int32 SAL_CALL OFunctions::getCount(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aFunctions.size();
}

uno::Any SAL_CALL OFunctions::getByIndex( ::sal_Int32 Index )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkIndex(Index);
    TFunctions::const_iterator aPos = m_aFunctions.begin();
    ::std::advance(aPos,Index);
    return uno::makeAny(*aPos);
}

// XElementAccess
uno::Type SAL_CALL OFunctions::getElementType(  )
{
    return cppu::UnoType<report::XFunction>::get();
}

sal_Bool SAL_CALL OFunctions::hasElements(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return !m_aFunctions.empty();
}

// XChild
uno::Reference< uno::XInterface > SAL_CALL OFunctions::getParent(  )
{
    return m_xParent;
}

void SAL_CALL OFunctions::setParent( const uno::Reference< uno::XInterface >& /*Parent*/ )
{
    throw lang::NoSupportException();
}

// XContainer
void SAL_CALL OFunctions::addContainerListener( const uno::Reference< container::XContainerListener >& xListener )
{
    m_aContainerListeners.addInterface(xListener);
}

void SAL_CALL OFunctions::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener )
{
    m_aContainerListeners.removeInterface(xListener);
}

void OFunctions::checkIndex(sal_Int32 _nIndex)
{
    if ( _nIndex < 0 || static_cast<sal_Int32>(m_aFunctions.size()) <= _nIndex )
        throw lang::IndexOutOfBoundsException();
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
