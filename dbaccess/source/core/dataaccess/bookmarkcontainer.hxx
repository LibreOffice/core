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

#ifndef _DBA_CORE_BOOKMARKCONTAINER_HXX_
#define _DBA_CORE_BOOKMARKCONTAINER_HXX_

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase6.hxx>
#include <comphelper/stl_types.hxx>
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
typedef ::cppu::WeakImplHelper6 <
                                    ::com::sun::star::container::XIndexAccess
                                ,   ::com::sun::star::container::XNameContainer
                                ,   ::com::sun::star::container::XEnumerationAccess
                                ,   ::com::sun::star::container::XContainer
                                ,   ::com::sun::star::lang::XServiceInfo
                                ,   ::com::sun::star::container::XChild
                                >   OBookmarkContainer_Base;

class OBookmarkContainer
            :public OBookmarkContainer_Base
{
protected:
    DECLARE_STL_USTRINGACCESS_MAP(OUString, MapString2String);
    DECLARE_STL_VECTOR(MapString2StringIterator, MapIteratorVector);

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

// ::com::sun::star::uno::XInterface
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

// ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 _nIndex ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameContainer
    virtual void SAL_CALL insertByName( const OUString& _rName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByName( const OUString& _rName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& _rName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::container::XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

// helper
    /** tell the container to free all resources. After that it's in a state like after the construction, i.e.
        you may call <code>initialize</code> again (maybe with another configuration node).
    */
    virtual void    dispose();

protected:
    /** checks whether the object is basically alive, i.e. it has been fully initialized (@see initialize) and
        not disposed (@see dispose)
        @param      _bIntendWriteAccess     determines whether or not the caller intends to modify the configuration.
                                            if sal_True and the configuration is readonly, a runtime exception with
                                            a description string is thrown.
    */
    void        checkValid(sal_Bool _bIntendWriteAccess) const throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::DisposedException);

    /** quickly checks if there already is an element with a given name. No access to the configuration occures, i.e.
        if there is such an object which is not already loaded, it won't be loaded now.
        @param      _rName      the object name to check
        @return                 sal_True if there already exists such an object
    */
    inline  sal_Bool    checkExistence(const OUString& _rName);

    void    implAppend(
        const OUString& _rName,
        const OUString& _rDocumentLocation
        );

    void implRemove(const OUString& _rName);

    void implReplace(
        const OUString& _rName,
        const OUString& _rNewLink);

};

inline  sal_Bool OBookmarkContainer::checkExistence(const OUString& _rName)
{
    return m_aBookmarks.find(_rName) != m_aBookmarks.end();
}

}   // namespace dbaccess

#endif // _DBA_CORE_BOOKMARKCONTAINER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
