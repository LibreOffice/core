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

#include <svl/lstner.hxx>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase.hxx>

namespace com::sun::star::beans { class XPropertyChangeListener; }
namespace com::sun::star::beans { class XPropertySetInfo; }
namespace com::sun::star::beans { class XVetoableChangeListener; }

class ScDocShell;

#define SC_LINKTARGETTYPE_SHEET     0
#define SC_LINKTARGETTYPE_RANGENAME 1
#define SC_LINKTARGETTYPE_DBAREA    2

#define SC_LINKTARGETTYPE_COUNT     3

#define SCLINKTARGET_SERVICE        "com.sun.star.document.LinkTarget"

//! Graphic / OleObject (need separate collections!)

class ScLinkTargetTypesObj final : public ::cppu::WeakImplHelper<
                                css::container::XNameAccess,
                                css::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    OUString                aNames[SC_LINKTARGETTYPE_COUNT];

public:
                            ScLinkTargetTypesObj(ScDocShell* pDocSh);
    virtual                 ~ScLinkTargetTypesObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // css::container::XNameAccess
    virtual css::uno::Any SAL_CALL     getByName(const OUString& aName) override;
    virtual css::uno::Sequence< OUString> SAL_CALL      getElementNames() override;
    virtual sal_Bool SAL_CALL           hasByName(const OUString& aName) override;

                            // css::container::XElementAccess
    virtual css::uno::Type SAL_CALL        getElementType() override;
    virtual sal_Bool SAL_CALL           hasElements() override;

                            // css::lang::XServiceInfo
    virtual OUString SAL_CALL            getImplementationName() override;
    virtual sal_Bool SAL_CALL           supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString> SAL_CALL      getSupportedServiceNames() override;
};

class ScLinkTargetTypeObj final : public ::cppu::WeakImplHelper<
                                css::beans::XPropertySet,
                                css::document::XLinkTargetSupplier,
                                css::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    sal_uInt16              nType;
    OUString                aName;

public:
                            ScLinkTargetTypeObj(ScDocShell* pDocSh, sal_uInt16 nT);
    virtual                 ~ScLinkTargetTypeObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    static void             SetLinkTargetBitmap( css::uno::Any& rRet, sal_uInt16 nType );

                            // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL  getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue(const OUString& aPropertyName,
                                    const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL     getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL           addPropertyChangeListener(const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener > & xListener) override;
    virtual void SAL_CALL           removePropertyChangeListener(const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL           addVetoableChangeListener(const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
    virtual void SAL_CALL           removeVetoableChangeListener(const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;

                            // css::document::XLinkTargetSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL  getLinks() override;

                            // css::lang::XServiceInfo
    virtual OUString SAL_CALL            getImplementationName() override;
    virtual sal_Bool SAL_CALL           supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;
};

class ScLinkTargetsObj final : public ::cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::lang::XServiceInfo >
{
private:
    css::uno::Reference< css::container::XNameAccess >            xCollection;

public:
                            ScLinkTargetsObj( const css::uno::Reference< css::container::XNameAccess > & rColl );
    virtual                 ~ScLinkTargetsObj() override;

                            // css::container::XNameAccess
    virtual css::uno::Any SAL_CALL         getByName(const OUString& aName) override;
    virtual css::uno::Sequence< OUString> SAL_CALL      getElementNames() override;
    virtual sal_Bool SAL_CALL           hasByName(const OUString& aName) override;

                            // css::container::XElementAccess
    virtual css::uno::Type SAL_CALL        getElementType() override;
    virtual sal_Bool SAL_CALL           hasElements() override;

                            // css::lang::XServiceInfo
    virtual OUString SAL_CALL            getImplementationName() override;
    virtual sal_Bool SAL_CALL           supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString> SAL_CALL      getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
