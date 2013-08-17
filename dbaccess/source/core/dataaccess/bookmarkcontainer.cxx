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

#include "bookmarkcontainer.hxx"
#include "dbastrings.hrc"
#include "apitools.hxx"
#include "core_resource.hxx"
#include "core_resource.hrc"

#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <comphelper/types.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

namespace dbaccess
{

// OBookmarkContainer
DBG_NAME(OBookmarkContainer)

OBookmarkContainer::OBookmarkContainer(OWeakObject& _rParent, Mutex& _rMutex)
    :m_rParent(_rParent)
    ,m_aContainerListeners(_rMutex)
    ,m_rMutex(_rMutex)
{
    DBG_CTOR(OBookmarkContainer, NULL);
}

void OBookmarkContainer::dispose()
{
    MutexGuard aGuard(m_rMutex);

    // say goodbye to our listeners
    EventObject aEvt(*this);
    m_aContainerListeners.disposeAndClear(aEvt);

    // remove our elements
    m_aBookmarksIndexed.clear();
    m_aBookmarks.clear();
}

void SAL_CALL OBookmarkContainer::acquire(  ) throw()
{
    m_rParent.acquire();
}

void SAL_CALL OBookmarkContainer::release(  ) throw()
{
    m_rParent.release();
}

OBookmarkContainer::~OBookmarkContainer()
{
    DBG_DTOR(OBookmarkContainer, NULL);
}

// XServiceInfo
OUString SAL_CALL OBookmarkContainer::getImplementationName(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.dba.OBookmarkContainer");
}

sal_Bool SAL_CALL OBookmarkContainer::supportsService( const OUString& _rServiceName ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

Sequence< OUString > SAL_CALL OBookmarkContainer::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< OUString > aReturn(1);
    aReturn.getArray()[0] = "com.sun.star.sdb.DefinitionContainer";
    return aReturn;
}

// XNameContainer
void SAL_CALL OBookmarkContainer::insertByName( const OUString& _rName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_True);

    if (checkExistence(_rName))
        throw ElementExistException();

    if (_rName.isEmpty())
        throw IllegalArgumentException();

    // approve the new object
    OUString sNewLink;
    if (!(aElement >>= sNewLink))
        throw IllegalArgumentException();

    implAppend(_rName, sNewLink);

    // notify the listeners
    if (m_aContainerListeners.getLength())
    {
        ContainerEvent aEvent(*this, makeAny(_rName), makeAny(sNewLink), Any());
        OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementInserted(aEvent);
    }
}

void SAL_CALL OBookmarkContainer::removeByName( const OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    OUString sOldBookmark;
    {
        MutexGuard aGuard(m_rMutex);
        checkValid(sal_True);

        // check the arguments
        if (_rName.isEmpty())
            throw IllegalArgumentException();

        if (!checkExistence(_rName))
            throw NoSuchElementException();

        // the old element (for the notifications)
        sOldBookmark = m_aBookmarks[_rName];

        // do the removal
        implRemove(_rName);
    }

    // notify the listeners
    if (m_aContainerListeners.getLength())
    {
        ContainerEvent aEvent(*this, makeAny(_rName), makeAny(sOldBookmark), Any());
        OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementRemoved(aEvent);
    }
}

// XNameReplace
void SAL_CALL OBookmarkContainer::replaceByName( const OUString& _rName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ClearableMutexGuard aGuard(m_rMutex);
    checkValid(sal_True);

    // check the arguments
    if (_rName.isEmpty())
        throw IllegalArgumentException();

    // do we have such an element?
    if (!checkExistence(_rName))
        throw NoSuchElementException();

    // approve the new object
    OUString sNewLink;
    if (!(aElement >>= sNewLink))
        throw IllegalArgumentException();

    // the old element (for the notifications)
    OUString sOldLink = m_aBookmarks[_rName];

    // do the replace
    implReplace(_rName, sNewLink);

    // notify the listeners
    aGuard.clear();
    if (m_aContainerListeners.getLength())
    {
        ContainerEvent aEvent(*this, makeAny(_rName), makeAny(sNewLink), makeAny(sOldLink));
        OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementReplaced(aEvent);
    }
}

void SAL_CALL OBookmarkContainer::addContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    if (_rxListener.is())
        m_aContainerListeners.addInterface(_rxListener);
}

void SAL_CALL OBookmarkContainer::removeContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    if (_rxListener.is())
        m_aContainerListeners.removeInterface(_rxListener);
}

// XElementAccess
Type SAL_CALL OBookmarkContainer::getElementType( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return ::getCppuType( static_cast< OUString* >(NULL) );
}

sal_Bool SAL_CALL OBookmarkContainer::hasElements( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return !m_aBookmarks.empty();
}

// XEnumerationAccess
Reference< XEnumeration > SAL_CALL OBookmarkContainer::createEnumeration(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return new ::comphelper::OEnumerationByIndex(static_cast<XIndexAccess*>(this));
}

// XIndexAccess
sal_Int32 SAL_CALL OBookmarkContainer::getCount(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return m_aBookmarks.size();
}

Any SAL_CALL OBookmarkContainer::getByIndex( sal_Int32 _nIndex ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    if ((_nIndex < 0) || (_nIndex >= (sal_Int32)m_aBookmarksIndexed.size()))
        throw IndexOutOfBoundsException();

    return makeAny(m_aBookmarksIndexed[_nIndex]->second);
}

Any SAL_CALL OBookmarkContainer::getByName( const OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    if (!checkExistence(_rName))
        throw NoSuchElementException();

    return makeAny(m_aBookmarks[_rName]);
}

Sequence< OUString > SAL_CALL OBookmarkContainer::getElementNames(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    Sequence< OUString > aNames(m_aBookmarks.size());
    OUString* pNames = aNames.getArray();
    ;
    for (   ConstMapIteratorVectorIterator aNameIter = m_aBookmarksIndexed.begin();
            aNameIter != m_aBookmarksIndexed.end();
            ++pNames, ++aNameIter
        )
    {
        *pNames = (*aNameIter)->first;
    }

    return aNames;
}

sal_Bool SAL_CALL OBookmarkContainer::hasByName( const OUString& _rName ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    return checkExistence(_rName);
}

void OBookmarkContainer::implRemove(const OUString& _rName)
{
    MutexGuard aGuard(m_rMutex);

    // look for the name in the index access vector
    MapString2StringIterator aMapPos = m_aBookmarks.end();
    for (   MapIteratorVectorIterator aSearch = m_aBookmarksIndexed.begin();
            aSearch != m_aBookmarksIndexed.end();
            ++aSearch
        )
    {
        if ((*aSearch)->first == _rName)
        {
            aMapPos = *aSearch;
            m_aBookmarksIndexed.erase(aSearch);
            break;
        }
    }

    if (m_aBookmarks.end() == aMapPos)
    {
        OSL_FAIL("OBookmarkContainer::implRemove: inconsistence!");
        return;
    }

    // remove the map entries
    m_aBookmarks.erase(aMapPos);
}

void OBookmarkContainer::implAppend(const OUString& _rName, const OUString& _rDocumentLocation)
{
    MutexGuard aGuard(m_rMutex);

    OSL_ENSURE(m_aBookmarks.find(_rName) == m_aBookmarks.end(),"Bookmark already known!");
    m_aBookmarksIndexed.push_back(m_aBookmarks.insert(  MapString2String::value_type(_rName,_rDocumentLocation)).first);
}

void OBookmarkContainer::implReplace(const OUString& _rName, const OUString& _rNewLink)
{
    MutexGuard aGuard(m_rMutex);
    OSL_ENSURE(checkExistence(_rName), "OBookmarkContainer::implReplace : invalid name !");

    m_aBookmarks[_rName] = _rNewLink;
}

void OBookmarkContainer::checkValid(sal_Bool /*_bIntendWriteAccess*/) const throw (RuntimeException, DisposedException)
{
}

Reference< XInterface > SAL_CALL OBookmarkContainer::getParent(  ) throw (RuntimeException)
{
    return m_rParent;
}

void SAL_CALL OBookmarkContainer::setParent( const Reference< XInterface >& /*Parent*/ ) throw (NoSupportException, RuntimeException)
{
    throw NoSupportException();
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
