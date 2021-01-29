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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOFIELD_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOFIELD_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>

#include <cppuhelper/implbase.hxx>

#include <unobaseclass.hxx>
#include <unocoll.hxx>
#include <fldbas.hxx>

class SwDoc;
class SwFormatField;
class SwSetExpField;

typedef ::cppu::WeakImplHelper
<   css::beans::XPropertySet
,   css::lang::XServiceInfo
,   css::lang::XUnoTunnel
,   css::lang::XComponent
> SwXFieldMaster_Base;

class SwXFieldMaster final
    : public SwXFieldMaster_Base
{

private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual ~SwXFieldMaster() override;

    SwXFieldMaster(SwFieldType& rType, SwDoc * pDoc);

    /// descriptor
    SwXFieldMaster(SwDoc* pDoc, SwFieldIds nResId);

public:

    static css::uno::Reference<css::beans::XPropertySet>
        CreateXFieldMaster(SwDoc * pDoc, SwFieldType * pType,
                SwFieldIds nResId = SwFieldIds::Unknown);

    static OUString GetProgrammaticName(const SwFieldType& rType, SwDoc& rDoc);
    static OUString LocalizeFormula(const SwSetExpField& rField, const OUString& rFormula, bool bQuery);

    SwFieldType* GetFieldType(bool bDontCreate = false) const;

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const css::uno::Sequence< sal_Int8 >& rIdentifier) override;

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

};

typedef ::cppu::WeakImplHelper
<   css::text::XDependentTextField
,   css::lang::XServiceInfo
,   css::beans::XPropertySet
,   css::lang::XUnoTunnel
,   css::util::XUpdatable
> SwXTextField_Base;

class SwXTextField final
    : public SwXTextField_Base
{

private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual ~SwXTextField() override;

    SwXTextField(SwFormatField& rFormat, SwDoc & rDoc);

    /// descriptor
    SwXTextField(SwServiceType nServiceId, SwDoc* pDoc);

public:
    SwServiceType GetServiceId() const;

    static void TransmuteLeadToInputField(SwSetExpField & rField);

    /// @return an SwXTextField, either an already existing one or a new one
    static css::uno::Reference< css::text::XTextField>
        CreateXTextField(SwDoc * pDoc, SwFormatField const* pFormat,
                SwServiceType nServiceId = SwServiceType::Invalid);

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const css::uno::Sequence< sal_Int8 >& rIdentifier) override;

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

    // XUpdatable
    virtual void SAL_CALL update() override;

    // XTextContent
    virtual void SAL_CALL attach(
            const css::uno::Reference< css::text::XTextRange > & xTextRange) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor() override;

    // XTextField
    virtual OUString SAL_CALL getPresentation(sal_Bool bShowCommand) override;

    // XDependentTextField
    virtual void SAL_CALL attachTextFieldMaster(
            const css::uno::Reference< css::beans::XPropertySet > & xFieldMaster) override;
    virtual css::uno::Reference< css::beans::XPropertySet> SAL_CALL getTextFieldMaster() override;

};

typedef ::cppu::WeakImplHelper
<   css::container::XEnumeration
,   css::lang::XServiceInfo
> SwXFieldEnumeration_Base;

class SwXFieldEnumeration final
    : public SwXFieldEnumeration_Base
{

private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual ~SwXFieldEnumeration() override;

public:
    explicit SwXFieldEnumeration(SwDoc & rDoc);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual css::uno::Any SAL_CALL nextElement() override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
