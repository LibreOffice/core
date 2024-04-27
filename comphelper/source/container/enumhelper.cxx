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

#include <comphelper/enumhelper.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <utility>

namespace comphelper
{

OEnumerationByName::OEnumerationByName(css::uno::Reference<css::container::XNameAccess> _xAccess)
    :m_aNames(_xAccess->getElementNames())
    ,m_xAccess(_xAccess)
    ,m_nPos(0)
    ,m_bListening(false)
{
    impl_startDisposeListening();
}


OEnumerationByName::OEnumerationByName(css::uno::Reference<css::container::XNameAccess> _xAccess,
                                       std::vector<OUString> _aNames  )
    :m_aNames(std::move(_aNames))
    ,m_xAccess(std::move(_xAccess))
    ,m_nPos(0)
    ,m_bListening(false)
{
    impl_startDisposeListening();
}

OEnumerationByName::~OEnumerationByName()
{
    std::lock_guard aLock(m_aLock);

    impl_stopDisposeListening();
}


sal_Bool SAL_CALL OEnumerationByName::hasMoreElements(  )
{
    std::lock_guard aLock(m_aLock);

    if (m_xAccess.is() && getLength() > m_nPos)
        return true;

    if (m_xAccess.is())
    {
        impl_stopDisposeListening();
        m_xAccess.clear();
    }

    return false;
}


css::uno::Any SAL_CALL OEnumerationByName::nextElement(  )
{
    std::lock_guard aLock(m_aLock);

    css::uno::Any aRes;
    if (m_xAccess.is() && m_nPos < getLength())
        aRes = m_xAccess->getByName(getElement(m_nPos++));

    if (m_xAccess.is() && m_nPos >= getLength())
    {
        impl_stopDisposeListening();
        m_xAccess.clear();
    }

    if (!aRes.hasValue())       //There are no more elements
        throw css::container::NoSuchElementException();

    return aRes;
}

void SAL_CALL OEnumerationByName::disposing(const css::lang::EventObject& aEvent)
{
    std::lock_guard aLock(m_aLock);

    if (aEvent.Source == m_xAccess)
        m_xAccess.clear();
}


void OEnumerationByName::impl_startDisposeListening()
{
    if (m_bListening)
        return;

    osl_atomic_increment(&m_refCount);
    css::uno::Reference< css::lang::XComponent > xDisposable(m_xAccess, css::uno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->addEventListener(this);
        m_bListening = true;
    }
    osl_atomic_decrement(&m_refCount);
}


void OEnumerationByName::impl_stopDisposeListening()
{
    if (!m_bListening)
        return;

    osl_atomic_increment(&m_refCount);
    css::uno::Reference< css::lang::XComponent > xDisposable(m_xAccess, css::uno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->removeEventListener(this);
        m_bListening = false;
    }
    osl_atomic_decrement(&m_refCount);
}

sal_Int32 OEnumerationByName::getLength() const
{
    if (m_aNames.index() == 0)
        return std::get<css::uno::Sequence<OUString>>(m_aNames).getLength();
    else
        return std::get<std::vector<OUString>>(m_aNames).size();
}

const OUString& OEnumerationByName::getElement(sal_Int32 nIndex) const
{
    if (m_aNames.index() == 0)
        return std::get<css::uno::Sequence<OUString>>(m_aNames)[nIndex];
    else
        return std::get<std::vector<OUString>>(m_aNames)[nIndex];
}


OEnumerationByIndex::OEnumerationByIndex(css::uno::Reference< css::container::XIndexAccess > _xAccess)
    :m_xAccess(std::move(_xAccess))
    ,m_nPos(0)
    ,m_bListening(false)
{
    impl_startDisposeListening();
}


OEnumerationByIndex::~OEnumerationByIndex()
{
    std::lock_guard aLock(m_aLock);

    impl_stopDisposeListening();
}


sal_Bool SAL_CALL OEnumerationByIndex::hasMoreElements(  )
{
    std::lock_guard aLock(m_aLock);

    if (m_xAccess.is() && m_xAccess->getCount() > m_nPos)
        return true;

    if (m_xAccess.is())
    {
        impl_stopDisposeListening();
        m_xAccess.clear();
    }

    return false;
}


css::uno::Any SAL_CALL OEnumerationByIndex::nextElement(  )
{
    std::lock_guard aLock(m_aLock);

    css::uno::Any aRes;
    if (m_xAccess.is() && m_nPos < m_xAccess->getCount())
        aRes = m_xAccess->getByIndex(m_nPos++);

    if (m_xAccess.is() && m_nPos >= m_xAccess->getCount())
    {
        impl_stopDisposeListening();
        m_xAccess.clear();
    }

    if (!aRes.hasValue())
        throw css::container::NoSuchElementException();
    return aRes;
}


void SAL_CALL OEnumerationByIndex::disposing(const css::lang::EventObject& aEvent)
{
    std::lock_guard aLock(m_aLock);

    if (aEvent.Source == m_xAccess)
        m_xAccess.clear();
}


void OEnumerationByIndex::impl_startDisposeListening()
{
    if (m_bListening)
        return;

    osl_atomic_increment(&m_refCount);
    css::uno::Reference< css::lang::XComponent > xDisposable(m_xAccess, css::uno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->addEventListener(this);
        m_bListening = true;
    }
    osl_atomic_decrement(&m_refCount);
}


void OEnumerationByIndex::impl_stopDisposeListening()
{
    if (!m_bListening)
        return;

    osl_atomic_increment(&m_refCount);
    css::uno::Reference< css::lang::XComponent > xDisposable(m_xAccess, css::uno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->removeEventListener(this);
        m_bListening = false;
    }
    osl_atomic_decrement(&m_refCount);
}

OAnyEnumeration::OAnyEnumeration(const css::uno::Sequence< css::uno::Any >& lItems)
    :m_nPos(0)
    ,m_lItems(lItems)
{
}


OAnyEnumeration::~OAnyEnumeration()
{
}


sal_Bool SAL_CALL OAnyEnumeration::hasMoreElements(  )
{
    std::lock_guard aLock(m_aLock);

    return (m_lItems.getLength() > m_nPos);
}


css::uno::Any SAL_CALL OAnyEnumeration::nextElement(  )
{
    if ( ! hasMoreElements())
        throw css::container::NoSuchElementException();

    std::lock_guard aLock(m_aLock);
    sal_Int32 nPos = m_nPos;
    ++m_nPos;
    return m_lItems[nPos];
}


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
