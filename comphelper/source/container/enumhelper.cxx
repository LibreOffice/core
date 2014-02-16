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

//.........................................................................
namespace comphelper
{
//.........................................................................

//==================================================================
//= OEnumerationByName
//==================================================================
//------------------------------------------------------------------------------
OEnumerationByName::OEnumerationByName(const staruno::Reference<starcontainer::XNameAccess>& _rxAccess)
    :m_aNames(_rxAccess->getElementNames())
    ,m_nPos(0)
    ,m_xAccess(_rxAccess)
    ,m_bListening(false)
{
    impl_startDisposeListening();
}

//------------------------------------------------------------------------------
OEnumerationByName::OEnumerationByName(const staruno::Reference<starcontainer::XNameAccess>& _rxAccess,
                                       const staruno::Sequence< OUString >&           _aNames  )
    :m_aNames(_aNames)
    ,m_nPos(0)
    ,m_xAccess(_rxAccess)
    ,m_bListening(false)
{
    impl_startDisposeListening();
}

//------------------------------------------------------------------------------
OEnumerationByName::~OEnumerationByName()
{
    impl_stopDisposeListening();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OEnumerationByName::hasMoreElements(  ) throw(staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (m_xAccess.is() && m_aNames.getLength() > m_nPos)
        return sal_True;

    if (m_xAccess.is())
    {
        impl_stopDisposeListening();
        m_xAccess.clear();
    }

    return sal_False;
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OEnumerationByName::nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    staruno::Any aRes;
    if (m_xAccess.is() && m_nPos < m_aNames.getLength())
        aRes = m_xAccess->getByName(m_aNames.getConstArray()[m_nPos++]);

    if (m_xAccess.is() && m_nPos >= m_aNames.getLength())
    {
        impl_stopDisposeListening();
        m_xAccess.clear();
    }

    if (!aRes.hasValue())       //There are no more elements
        throw starcontainer::NoSuchElementException();

    return aRes;
}

//------------------------------------------------------------------------------
void SAL_CALL OEnumerationByName::disposing(const starlang::EventObject& aEvent)
        throw(staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (aEvent.Source == m_xAccess)
        m_xAccess.clear();
}

//------------------------------------------------------------------------------
void OEnumerationByName::impl_startDisposeListening()
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (m_bListening)
        return;

    ++m_refCount;
    staruno::Reference< starlang::XComponent > xDisposable(m_xAccess, staruno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->addEventListener(this);
        m_bListening = true;
    }
    --m_refCount;
}

//------------------------------------------------------------------------------
void OEnumerationByName::impl_stopDisposeListening()
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (!m_bListening)
        return;

    ++m_refCount;
    staruno::Reference< starlang::XComponent > xDisposable(m_xAccess, staruno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->removeEventListener(this);
        m_bListening = false;
    }
    --m_refCount;
}

//==================================================================
//= OEnumerationByIndex
//==================================================================
//------------------------------------------------------------------------------
OEnumerationByIndex::OEnumerationByIndex(const staruno::Reference< starcontainer::XIndexAccess >& _rxAccess)
    :m_nPos(0)
    ,m_xAccess(_rxAccess)
    ,m_bListening(false)
{
    impl_startDisposeListening();
}

//------------------------------------------------------------------------------
OEnumerationByIndex::~OEnumerationByIndex()
{
    impl_stopDisposeListening();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OEnumerationByIndex::hasMoreElements(  ) throw(staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (m_xAccess.is() && m_xAccess->getCount() > m_nPos)
        return sal_True;

    if (m_xAccess.is())
    {
        impl_stopDisposeListening();
        m_xAccess.clear();
    }

    return sal_False;
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OEnumerationByIndex::nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    staruno::Any aRes;
    if (m_xAccess.is())
    {
        aRes = m_xAccess->getByIndex(m_nPos++);
        if (m_nPos >= m_xAccess->getCount())
        {
            impl_stopDisposeListening();
            m_xAccess.clear();
        }
    }

    if (!aRes.hasValue())
        throw starcontainer::NoSuchElementException();
    return aRes;
}

//------------------------------------------------------------------------------
void SAL_CALL OEnumerationByIndex::disposing(const starlang::EventObject& aEvent)
        throw(staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (aEvent.Source == m_xAccess)
        m_xAccess.clear();
}

//------------------------------------------------------------------------------
void OEnumerationByIndex::impl_startDisposeListening()
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (m_bListening)
        return;

    ++m_refCount;
    staruno::Reference< starlang::XComponent > xDisposable(m_xAccess, staruno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->addEventListener(this);
        m_bListening = true;
    }
    --m_refCount;
}

//------------------------------------------------------------------------------
void OEnumerationByIndex::impl_stopDisposeListening()
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (!m_bListening)
        return;

    ++m_refCount;
    staruno::Reference< starlang::XComponent > xDisposable(m_xAccess, staruno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->removeEventListener(this);
        m_bListening = false;
    }
    --m_refCount;
}

//==================================================================
//= OAnyEnumeration
//==================================================================

//------------------------------------------------------------------------------
OAnyEnumeration::OAnyEnumeration(const staruno::Sequence< staruno::Any >& lItems)
    :m_nPos(0)
    ,m_lItems(lItems)
{
}

//------------------------------------------------------------------------------
OAnyEnumeration::~OAnyEnumeration()
{
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OAnyEnumeration::hasMoreElements(  ) throw(staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    return (m_lItems.getLength() > m_nPos);
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OAnyEnumeration::nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    if ( ! hasMoreElements())
        throw starcontainer::NoSuchElementException();

    ::osl::ResettableMutexGuard aLock(m_aLock);
    sal_Int32 nPos = m_nPos;
    ++m_nPos;
    return m_lItems[nPos];
}

//.........................................................................
}   // namespace comphelper
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
