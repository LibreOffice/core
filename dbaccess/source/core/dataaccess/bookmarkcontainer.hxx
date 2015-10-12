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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_BOOKMARKCONTAINER_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_BOOKMARKCONTAINER_HXX

#include <sal/config.h>

#include <map>
#include <vector>

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

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

class OBookmarkContainer
            :public OBookmarkContainer_Base
{
protected:
    typedef std::map<OUString, OUString> MapString2String;
    typedef std::vector<MapString2String::iterator> MapIteratorVector;

    MapString2String        m_aBookmarks;           // the bookmarks itself
    MapIteratorVector       m_aBookmarksIndexed;    // for index access to the

protected:
    ::cppu::OWeakObject&    m_rParent;      // for the ref counting
    ::cppu::OInterfaceContainerHelper
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
    virtual ~OBookmarkContainer();

// css::uno::XInterface
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

// css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 _nIndex ) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

// css::container::XNameContainer
    virtual void SAL_CALL insertByName( const OUString& _rName, const css::uno::Any& aElement ) throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& _rName ) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

// css::container::XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& _rName, const css::uno::Any& aElement ) throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

// css::container::XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XContainer
    virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

protected:
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

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_BOOKMARKCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
