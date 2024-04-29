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

#include <algorithm>
#include <comphelper/numberedcollection.hxx>
#include <com/sun/star/frame/UntitledNumbersConst.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace comphelper{

constexpr OUString ERRMSG_INVALID_COMPONENT_PARAM = u"NULL as component reference not allowed."_ustr;


NumberedCollection::NumberedCollection()
{
}


NumberedCollection::~NumberedCollection()
{
}


void NumberedCollection::setOwner(const css::uno::Reference< css::uno::XInterface >& xOwner)
{
    // SYNCHRONIZED ->
    std::scoped_lock aLock(m_aMutex);

    m_xOwner = xOwner;

    // <- SYNCHRONIZED
}


void NumberedCollection::setUntitledPrefix(const OUString& sPrefix)
{
    // SYNCHRONIZED ->
    std::scoped_lock aLock(m_aMutex);

    m_sUntitledPrefix = sPrefix;

    // <- SYNCHRONIZED
}


::sal_Int32 SAL_CALL NumberedCollection::leaseNumber(const css::uno::Reference< css::uno::XInterface >& xComponent)
{
    // SYNCHRONIZED ->
    std::scoped_lock aLock(m_aMutex);

    if ( ! xComponent.is ())
        throw css::lang::IllegalArgumentException(ERRMSG_INVALID_COMPONENT_PARAM, m_xOwner.get(), 1);

    sal_IntPtr pComponent = reinterpret_cast<sal_IntPtr>( xComponent.get() );
    TNumberedItemHash::const_iterator pIt = m_lComponents.find (pComponent);

    // a) component already exists - return its number directly
    if (pIt != m_lComponents.end())
        return pIt->second.nNumber;

    // b) component must be added new to this container

    // b1) collection is full - no further components possible
    //     -> return INVALID_NUMBER
    ::sal_Int32 nFreeNumber = impl_searchFreeNumber();
    if (nFreeNumber == css::frame::UntitledNumbersConst::INVALID_NUMBER)
        return css::frame::UntitledNumbersConst::INVALID_NUMBER;

    // b2) add component to collection and return its number
    TNumberedItem aItem;
    aItem.xItem   = css::uno::WeakReference< css::uno::XInterface >(xComponent);
    aItem.nNumber = nFreeNumber;
    m_lComponents[pComponent] = aItem;

    return nFreeNumber;

    // <- SYNCHRONIZED
}


void SAL_CALL NumberedCollection::releaseNumber(::sal_Int32 nNumber)
{
    // SYNCHRONIZED ->
    std::scoped_lock aLock(m_aMutex);

    if (nNumber == css::frame::UntitledNumbersConst::INVALID_NUMBER)
        throw css::lang::IllegalArgumentException (u"Special value INVALID_NUMBER not allowed as input parameter."_ustr, m_xOwner.get(), 1);

    TDeadItemList               lDeadItems;
    TNumberedItemHash::iterator pComponent;

    for (  pComponent  = m_lComponents.begin ();
           pComponent != m_lComponents.end   ();
         ++pComponent                          )
    {
        const TNumberedItem&                              rItem = pComponent->second;
        const css::uno::Reference< css::uno::XInterface > xItem = rItem.xItem.get();

        if ( ! xItem.is ())
        {
            lDeadItems.push_back(pComponent->first);
            continue;
        }

        if (rItem.nNumber == nNumber)
        {
            m_lComponents.erase (pComponent);
            break;
        }
    }

    impl_cleanUpDeadItems(m_lComponents, lDeadItems);

    // <- SYNCHRONIZED
}


void SAL_CALL NumberedCollection::releaseNumberForComponent(const css::uno::Reference< css::uno::XInterface >& xComponent)
{
    // SYNCHRONIZED ->
    std::scoped_lock aLock(m_aMutex);

    if ( ! xComponent.is ())
        throw css::lang::IllegalArgumentException(ERRMSG_INVALID_COMPONENT_PARAM, m_xOwner.get(), 1);

    sal_IntPtr pComponent = reinterpret_cast<sal_IntPtr>( xComponent.get() );
    TNumberedItemHash::iterator pIt = m_lComponents.find (pComponent);

    // a) component exists and will be removed
    if (pIt != m_lComponents.end())
        m_lComponents.erase(pIt);

    // else
    // b) component does not exists - nothing todo here (ignore request!)

    // <- SYNCHRONIZED
}


OUString SAL_CALL NumberedCollection::getUntitledPrefix()
{
    // SYNCHRONIZED ->
    std::scoped_lock aLock(m_aMutex);

    return m_sUntitledPrefix;

    // <- SYNCHRONIZED
}


/** create an ordered list of all possible numbers ...
    e.g. {1,2,3,...,N} Max size of these list will be
    current size of component list + 1 .

    "+1" ... because in case all numbers in range 1..n
    are in use we need a new number n+1 :-)

    Every item which is already used as unique number
    will be removed. At the end a list of e.g. {3,6,...,M}
    exists where the first item represent the lowest free
    number (in this example 3).
 */
::sal_Int32 NumberedCollection::impl_searchFreeNumber ()
{
    // create bitset, where each position represents one possible number.
    std::vector<bool> aUsedNumbers((m_lComponents.size() * 2) + 1, false);

    for (const auto& rPair : m_lComponents)
    {
        // numbers start at 1
        sal_Int32 pos = rPair.second.nNumber - 1;
        if (pos >= static_cast<sal_Int32>(aUsedNumbers.size()))
            aUsedNumbers.resize(pos * 2, false); //  should be rare
        aUsedNumbers[pos] = true;
    }

    // a) non free numbers ... return INVALID_NUMBER
    auto it = std::find(aUsedNumbers.begin(), aUsedNumbers.end(), false);
    if (it == aUsedNumbers.end())
        return css::frame::UntitledNumbersConst::INVALID_NUMBER;

    // b) return first free number
    return it - aUsedNumbers.begin() + 1;
}

void NumberedCollection::impl_cleanUpDeadItems (      TNumberedItemHash& lItems    ,
                                                const TDeadItemList&     lDeadItems)
{
    for (const sal_IntPtr& rDeadItem : lDeadItems)
    {
        lItems.erase(rDeadItem);
    }
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
