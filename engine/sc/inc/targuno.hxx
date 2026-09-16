/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

class ScDocShell;

#define SC_LINKTARGETTYPE_SHEET     0
#define SC_LINKTARGETTYPE_RANGENAME 1
#define SC_LINKTARGETTYPE_DBAREA    2
#define SC_LINKTARGETTYPE_OLEOBJECT 3

#define SC_LINKTARGETTYPE_COUNT     4

inline constexpr OUString SCLINKTARGET_SERVICE = u"com.sun.star.document.LinkTarget"_ustr;

//! Graphic (needs its own collection too, see ScOleObjectsObj for the pattern)

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
    virtual cpo::uno::Any     getByName(const OUString& aName) override;
    virtual cpo::uno::Sequence< OUString>      getElementNames() override;
    virtual bool           hasByName(const OUString& aName) override;

                            // css::container::XElementAccess
    virtual cpo::uno::Type        getElementType() override;
    virtual bool           hasElements() override;

                            // css::lang::XServiceInfo
    virtual OUString            getImplementationName() override;
    virtual bool           supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString>      getSupportedServiceNames() override;
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

    static void             SetLinkTargetBitmap( cpo::uno::Any& rRet, sal_uInt16 nType );

                            // css::beans::XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >  getPropertySetInfo() override;
    virtual void   setPropertyValue(const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue) override;
    virtual cpo::uno::Any     getPropertyValue(const OUString& PropertyName) override;
    virtual void           addPropertyChangeListener(const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener > & xListener) override;
    virtual void           removePropertyChangeListener(const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener > & aListener) override;
    virtual void           addVetoableChangeListener(const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
    virtual void           removeVetoableChangeListener(const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;

                            // css::document::XLinkTargetSupplier
    virtual cpo::uno::Reference< css::container::XNameAccess >  getLinks() override;

                            // css::lang::XServiceInfo
    virtual OUString            getImplementationName() override;
    virtual bool           supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString> getSupportedServiceNames() override;
};

class ScLinkTargetsObj final : public ::cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::lang::XServiceInfo >
{
private:
    cpo::uno::Reference< css::container::XNameAccess >            xCollection;

public:
                            ScLinkTargetsObj( cpo::uno::Reference< css::container::XNameAccess > xColl );
    virtual                 ~ScLinkTargetsObj() override;

                            // css::container::XNameAccess
    virtual cpo::uno::Any         getByName(const OUString& aName) override;
    virtual cpo::uno::Sequence< OUString>      getElementNames() override;
    virtual bool           hasByName(const OUString& aName) override;

                            // css::container::XElementAccess
    virtual cpo::uno::Type        getElementType() override;
    virtual bool           hasElements() override;

                            // css::lang::XServiceInfo
    virtual OUString            getImplementationName() override;
    virtual bool           supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString>      getSupportedServiceNames() override;
};

// Names of the OLE objects (for example charts) drawn on any sheet, exposed as link
// targets so the Navigator and the Insert Hyperlink dialog can jump to them, and so
// they show up in the .uno:ExtractLinkTargets output next to sheets and named ranges.
class ScOleObjectsObj final : public ::cppu::WeakImplHelper<
                            css::container::XNameAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;

public:
                            ScOleObjectsObj(ScDocShell* pDocSh);
    virtual                 ~ScOleObjectsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // css::container::XNameAccess
    virtual cpo::uno::Any     getByName(const OUString& aName) override;
    virtual cpo::uno::Sequence< OUString>      getElementNames() override;
    virtual bool           hasByName(const OUString& aName) override;

                            // css::container::XElementAccess
    virtual cpo::uno::Type        getElementType() override;
    virtual bool           hasElements() override;

                            // css::lang::XServiceInfo
    virtual OUString            getImplementationName() override;
    virtual bool           supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString>      getSupportedServiceNames() override;
};

// A single named OLE object, as a leaf link target. It carries only the display
// name and icon that document::LinkTarget requires - it has no properties of its
// own the way a sheet or a database range object does.
class ScOleObjectLinkTargetObj final : public ::cppu::WeakImplHelper<
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo >
{
private:
    OUString                aName;

public:
                            ScOleObjectLinkTargetObj( OUString aObjectName );
    virtual                 ~ScOleObjectLinkTargetObj() override;

                            // css::beans::XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >  getPropertySetInfo() override;
    virtual void   setPropertyValue(const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue) override;
    virtual cpo::uno::Any     getPropertyValue(const OUString& PropertyName) override;
    virtual void           addPropertyChangeListener(const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener > & xListener) override;
    virtual void           removePropertyChangeListener(const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener > & aListener) override;
    virtual void           addVetoableChangeListener(const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
    virtual void           removeVetoableChangeListener(const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;

                            // css::lang::XServiceInfo
    virtual OUString            getImplementationName() override;
    virtual bool           supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString> getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
