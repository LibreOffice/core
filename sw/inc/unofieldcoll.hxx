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

#ifndef INCLUDED_SW_INC_UNOFIELDCOLL_HXX
#define INCLUDED_SW_INC_UNOFIELDCOLL_HXX

#include <memory>

#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/container/XUniqueIDAccess.hpp>

#include "unocoll.hxx"

class SwFieldType;

typedef ::cppu::WeakImplHelper
<   css::container::XNameAccess
,   css::lang::XServiceInfo
> SwXTextFieldMasters_Base;

class SwXTextFieldMasters final
    : public SwXTextFieldMasters_Base
    , public SwUnoCollection
{
    virtual ~SwXTextFieldMasters() override;

public:
    SwXTextFieldMasters(SwDoc* pDoc);

    static bool getInstanceName(const SwFieldType& rFieldType, OUString& rName);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName(
            const OUString& rName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& rName) override;

};

typedef ::cppu::WeakImplHelper
<   css::container::XEnumerationAccess
,   css::lang::XServiceInfo
,   css::util::XRefreshable
,   css::container::XUniqueIDAccess
> SwXTextFieldTypes_Base;

class SwXTextFieldTypes final
    : public SwXTextFieldTypes_Base
    , public SwUnoCollection
{
private:
    class Impl;
    std::unique_ptr<Impl> m_pImpl; // currently does not need UnoImplPtr

    virtual ~SwXTextFieldTypes() override;

public:
    SwXTextFieldTypes(SwDoc* pDoc);

    // SwUnoCollection
    virtual void    Invalidate() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XEnumerationAccess
    virtual css::uno::Reference<
            css::container::XEnumeration >  SAL_CALL
        createEnumeration() override;

    // XRefreshable
    virtual void SAL_CALL refresh() override;
    virtual void SAL_CALL addRefreshListener(
            const css::uno::Reference<
                css::util::XRefreshListener>& xListener) override;
    virtual void SAL_CALL removeRefreshListener(
            const css::uno::Reference<
                css::util::XRefreshListener>& xListener) override;

    // container::XUniqueIDAccess
    virtual css::uno::Any SAL_CALL getByUniqueID( const OUString& ID ) override;
    virtual void SAL_CALL removeByUniqueID( const OUString& ID ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
