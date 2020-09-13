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

#pragma once

#include <sal/config.h>

#include <map>
#include <vector>

#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace dbaccess
{

// OBookmarkContainer -    base class of collections of database definition
//                         documents
typedef ::cppu::WeakImplHelper<
                                  css::container::XIndexAccess
                              ,   css::container::XNameContainer
                              ,   css::container::XEnumerationAccess
                              ,   css::container::XContainer
                              ,   css::lang::XServiceInfo
                              ,   css::container::XChild
                              >   OBookmarkContainer_Base;

class OBookmarkContainer final
            :public OBookmarkContainer_Base
{
    typedef std::map<OUString, OUString> MapString2String;
    typedef std::vector<MapString2String::iterator> MapIteratorVector;

    MapString2String        m_aBookmarks;           // the bookmarks itself
    MapIteratorVector       m_aBookmarksIndexed;    // for index access to the

    ::cppu::OWeakObject&    m_rParent;      // for the ref counting
    ::comphelper::OInterfaceContainerHelper2
                            m_aContainerListeners;
    ::osl::Mutex&           m_rMutex;

public:
    /** constructs the container.<BR>
        after the construction of the object the creator has to call <code>initialize</code>.
        @param      _rParent                the parent object which is used for ref counting
        @param      _rMutex                 the parent's mutex object for access safety
    */
    OBookmarkContainer(
        ::cppu::OWeakObject& _rParent,
        ::osl::Mutex& _rMutex
        );

    /** looks like the dtor ...
    */
    virtual ~OBookmarkContainer() override;

// css::uno::XInterface
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

// css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

// css::container::XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

// css::container::XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) override;

// css::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 _nIndex ) override;

// css::container::XNameContainer
    virtual void SAL_CALL insertByName( const OUString& _rName, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& _rName ) override;

// css::container::XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& _rName, const css::uno::Any& aElement ) override;

// css::container::XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

// css::container::XContainer
    virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

// css::container::XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

private:
    /** quickly checks if there already is an element with a given name. No access to the configuration occurs, i.e.
        if there is such an object which is not already loaded, it won't be loaded now.
        @param      _rName      the object name to check
        @return                 sal_True if there already exists such an object
    */
    inline  bool    checkExistence(const OUString& _rName);

    void    implAppend(
        const OUString& _rName,
        const OUString& _rDocumentLocation
        );

    void implRemove(const OUString& _rName);

    void implReplace(
        const OUString& _rName,
        const OUString& _rNewLink);

};

inline  bool OBookmarkContainer::checkExistence(const OUString& _rName)
{
    return m_aBookmarks.find(_rName) != m_aBookmarks.end();
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
