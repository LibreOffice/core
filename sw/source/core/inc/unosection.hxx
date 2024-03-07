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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/XTextSection.hpp>

#include <cppuhelper/implbase.hxx>

#include <sfx2/Metadatable.hxx>

#include <unobaseclass.hxx>

class SwSectionFormat;

typedef ::cppu::ImplInheritanceHelper
<   ::sfx2::MetadatableMixin
,   css::lang::XServiceInfo
,   css::beans::XPropertySet
,   css::beans::XPropertyState
,   css::beans::XMultiPropertySet
,   css::container::XNamed
,   css::text::XTextSection
> SwXTextSection_Base;

class SwXTextSection final
    : public SwXTextSection_Base
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    SwXTextSection(SwSectionFormat *const pFormat, const bool bIndexHeader);

    virtual ~SwXTextSection() override;

public:

    SwSectionFormat*   GetFormat() const;

    static rtl::Reference< SwXTextSection >
        CreateXTextSection(SwSectionFormat *const pFormat,
                const bool bIndexHeader = false);

    // MetadatableMixin
    virtual ::sfx2::Metadatable* GetCoreObject() override;
    virtual css::uno::Reference< css::frame::XModel >
        GetModel() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener > & xListener) override;
    virtual void SAL_CALL removeEventListener(
            const css::uno::Reference< css::lang::XEventListener > & xListener) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener) override;

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL
        getPropertyState(const OUString& rPropertyName) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
        getPropertyStates(
            const css::uno::Sequence< OUString >& rPropertyNames) override;
    virtual void SAL_CALL setPropertyToDefault(
            const OUString& rPropertyName) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault(
            const OUString& rPropertyName) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues(
            const css::uno::Sequence< OUString >&  rPropertyNames,
            const css::uno::Sequence< css::uno::Any >& rValues) override;
    virtual css::uno::Sequence< css::uno::Any >
        SAL_CALL getPropertyValues(
            const css::uno::Sequence< OUString >& rPropertyNames) override;
    virtual void SAL_CALL addPropertiesChangeListener(
            const css::uno::Sequence< OUString >& rPropertyNames,
            const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener) override;
    virtual void SAL_CALL removePropertiesChangeListener(
            const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener) override;
    virtual void SAL_CALL firePropertiesChangeEvent(
            const css::uno::Sequence< OUString >&  rPropertyNames,
            const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener) override;

    // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& rName) override;

    // XTextContent
    virtual void SAL_CALL attach(
            const css::uno::Reference< css::text::XTextRange > & xTextRange) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor() override;

    // XTextSection
    virtual css::uno::Reference< css::text::XTextSection > SAL_CALL
        getParentSection() override;
    virtual css::uno::Sequence< css::uno::Reference< css::text::XTextSection >  > SAL_CALL
        getChildSections() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
