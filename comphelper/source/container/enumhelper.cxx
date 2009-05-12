/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: enumhelper.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
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
    ,m_bListening(sal_False)
{
    impl_startDisposeListening();
}

//------------------------------------------------------------------------------
OEnumerationByName::OEnumerationByName(const staruno::Reference<starcontainer::XNameAccess>& _rxAccess,
                                       const staruno::Sequence< ::rtl::OUString >&           _aNames  )
    :m_aNames(_aNames)
    ,m_nPos(0)
    ,m_xAccess(_rxAccess)
    ,m_bListening(sal_False)
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

    if (!aRes.hasValue())       // es gibt kein Element mehr
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
        m_bListening = sal_True;
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
        m_bListening = sal_False;
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
    ,m_bListening(sal_False)
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

    if (!aRes.hasValue())       // es gibt kein Element mehr
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
        m_bListening = sal_True;
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
        m_bListening = sal_False;
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


