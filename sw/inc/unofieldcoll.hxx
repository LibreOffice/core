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

#ifndef SW_UNOFIELDCOLL_HXX
#define SW_UNOFIELDCOLL_HXX

#include <boost/scoped_ptr.hpp>

#include <com/sun/star/util/XRefreshable.hpp>

#include <unocoll.hxx>

class SwFieldType;

typedef ::cppu::WeakImplHelper2
<   ::com::sun::star::container::XNameAccess
,   ::com::sun::star::lang::XServiceInfo
> SwXTextFieldMasters_Base;

class SwXTextFieldMasters
    : public SwXTextFieldMasters_Base
    , public SwUnoCollection
{
protected:
    virtual ~SwXTextFieldMasters();

public:
    SwXTextFieldMasters(SwDoc* pDoc);

    static sal_Bool getInstanceName(const SwFieldType& rFldType, OUString& rName);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(
            const OUString& rName)
        throw (::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getElementNames() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName(const OUString& rName)
        throw (::com::sun::star::uno::RuntimeException);

};

typedef ::cppu::WeakImplHelper3
<   ::com::sun::star::container::XEnumerationAccess
,   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::util::XRefreshable
> SwXTextFieldTypes_Base;

class SwXTextFieldTypes
    : public SwXTextFieldTypes_Base
    , public SwUnoCollection
{
private:
    class Impl;
    ::boost::scoped_ptr<Impl> m_pImpl; // currently does not need UnoImplPtr

protected:
    virtual ~SwXTextFieldTypes();

public:
    SwXTextFieldTypes(SwDoc* pDoc);

    // SwUnoCollection
    virtual void    Invalidate();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (::com::sun::star::uno::RuntimeException);

    // XRefreshable
    virtual void SAL_CALL refresh()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRefreshListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::util::XRefreshListener>& xListener)
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRefreshListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::util::XRefreshListener>& xListener)
        throw (::com::sun::star::uno::RuntimeException);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
