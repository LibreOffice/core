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

#include <bookmarkcontainer.hxx>
#include <stringconstants.hxx>
#include <apitools.hxx>

#include <osl/diagnose.h>
#include <comphelper/enumhelper.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>

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

OBookmarkContainer::OBookmarkContainer(OWeakObject& _rParent, Mutex& _rMutex)
    :m_rParent(_rParent)
    ,m_aContainerListeners(_rMutex)
    ,m_rMutex(_rMutex)
{
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
}

// XServiceInfo
OUString SAL_CALL OBookmarkContainer::getImplementationName(  )
{
    return OUString("com.sun.star.comp.dba.OBookmarkContainer");
}

sal_Bool SAL_CALL OBookmarkContainer::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL OBookmarkContainer::getSupportedServiceNames(  )
{
    Sequence< OUString > aReturn { "com.sun.star.sdb.DefinitionContainer" };
    return aReturn;
}

// XNameContainer
void SAL_CALL OBookmarkContainer::insertByName( const OUString& _rName, const Any& aElement )
{
    MutexGuard aGuard(m_rMutex);

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
        OInterfaceIteratorHelper2 aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementInserted(aEvent);
    }
}

void SAL_CALL OBookmarkContainer::removeByName( const OUString& _rName )
{
    OUString sOldBookmark;
    {
        MutexGuard aGuard(m_rMutex);

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
        OInterfaceIteratorHelper2 aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementRemoved(aEvent);
    }
}

// XNameReplace
void SAL_CALL OBookmarkContainer::replaceByName( const OUString& _rName, const Any& aElement )
{
    ClearableMutexGuard aGuard(m_rMutex);

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
        OInterfaceIteratorHelper2 aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementReplaced(aEvent);
    }
}

void SAL_CALL OBookmarkContainer::addContainerListener( const Reference< XContainerListener >& _rxListener )
{
    MutexGuard aGuard(m_rMutex);
    if (_rxListener.is())
        m_aContainerListeners.addInterface(_rxListener);
}

void SAL_CALL OBookmarkContainer::removeContainerListener( const Reference< XContainerListener >& _rxListener )
{
    MutexGuard aGuard(m_rMutex);
    if (_rxListener.is())
        m_aContainerListeners.removeInterface(_rxListener);
}

// XElementAccess
Type SAL_CALL OBookmarkContainer::getElementType( )
{
    MutexGuard aGuard(m_rMutex);
    return ::cppu::UnoType<OUString>::get();
}

sal_Bool SAL_CALL OBookmarkContainer::hasElements( )
{
    MutexGuard aGuard(m_rMutex);
    return !m_aBookmarks.empty();
}

// XEnumerationAccess
Reference< XEnumeration > SAL_CALL OBookmarkContainer::createEnumeration(  )
{
    MutexGuard aGuard(m_rMutex);
    return new ::comphelper::OEnumerationByIndex(static_cast<XIndexAccess*>(this));
}

// XIndexAccess
sal_Int32 SAL_CALL OBookmarkContainer::getCount(  )
{
    MutexGuard aGuard(m_rMutex);
    return m_aBookmarks.size();
}

Any SAL_CALL OBookmarkContainer::getByIndex( sal_Int32 _nIndex )
{
    MutexGuard aGuard(m_rMutex);

    if ((_nIndex < 0) || (_nIndex >= static_cast<sal_Int32>(m_aBookmarksIndexed.size())))
        throw IndexOutOfBoundsException();

    return makeAny(m_aBookmarksIndexed[_nIndex]->second);
}

Any SAL_CALL OBookmarkContainer::getByName( const OUString& _rName )
{
    MutexGuard aGuard(m_rMutex);

    if (!checkExistence(_rName))
        throw NoSuchElementException();

    return makeAny(m_aBookmarks[_rName]);
}

Sequence< OUString > SAL_CALL OBookmarkContainer::getElementNames(  )
{
    MutexGuard aGuard(m_rMutex);

    Sequence< OUString > aNames(m_aBookmarks.size());
    OUString* pNames = aNames.getArray();

    for (auto const& bookmarkIndexed : m_aBookmarksIndexed)
    {
        *pNames = bookmarkIndexed->first;
        ++pNames;
    }

    return aNames;
}

sal_Bool SAL_CALL OBookmarkContainer::hasByName( const OUString& _rName )
{
    MutexGuard aGuard(m_rMutex);

    return checkExistence(_rName);
}

void OBookmarkContainer::implRemove(const OUString& _rName)
{
    MutexGuard aGuard(m_rMutex);

    // look for the name in the index access vector
    MapString2String::const_iterator aMapPos = m_aBookmarks.end();
    auto aSearch = std::find_if(m_aBookmarksIndexed.begin(), m_aBookmarksIndexed.end(),
        [&_rName](const MapString2String::iterator& rIter) { return rIter->first == _rName; });
    if (aSearch != m_aBookmarksIndexed.end())
    {
        aMapPos = *aSearch;
        m_aBookmarksIndexed.erase(aSearch);
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
    m_aBookmarksIndexed.push_back(m_aBookmarks.emplace( _rName,_rDocumentLocation).first);
}

void OBookmarkContainer::implReplace(const OUString& _rName, const OUString& _rNewLink)
{
    MutexGuard aGuard(m_rMutex);
    OSL_ENSURE(checkExistence(_rName), "OBookmarkContainer::implReplace : invalid name !");

    m_aBookmarks[_rName] = _rNewLink;
}

Reference< XInterface > SAL_CALL OBookmarkContainer::getParent(  )
{
    return m_rParent;
}

void SAL_CALL OBookmarkContainer::setParent( const Reference< XInterface >& /*Parent*/ )
{
    throw NoSupportException();
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
