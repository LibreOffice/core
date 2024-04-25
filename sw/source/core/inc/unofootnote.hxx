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

#include <swdllapi.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/text/XFootnote.hpp>

#include <cppuhelper/implbase.hxx>

#include <unotext.hxx>

class SwDoc;
class SwFormatFootnote;

typedef ::cppu::WeakImplHelper
<   css::lang::XServiceInfo
,   css::beans::XPropertySet
,   css::container::XEnumerationAccess
,   css::text::XFootnote
> SwXFootnote_Base;

class SW_DLLPUBLIC SwXFootnote final
    : public SwXFootnote_Base
    , public SwXText
{
    friend class SwXFootnotes;

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual const SwStartNode *GetStartNode() const override;

    virtual ~SwXFootnote() override;

    SwXFootnote(SwDoc & rDoc, SwFormatFootnote & rFormat);
    SwXFootnote(const bool bEndnote);

public:

    static rtl::Reference<SwXFootnote>
        CreateXFootnote(SwDoc & rDoc, SwFormatFootnote * pFootnoteFormat,
                bool isEndnote = false);

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(
            const css::uno::Type& rType) override;
    virtual void SAL_CALL acquire() noexcept override { OWeakObject::acquire(); }
    virtual void SAL_CALL release() noexcept override { OWeakObject::release(); }

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type >
        SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId() override;

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

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL
        createEnumeration() override;

    // XTextContent
    virtual void SAL_CALL attach(
            const css::uno::Reference< css::text::XTextRange > & xTextRange) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor() override;

    // XFootnote
    virtual OUString SAL_CALL getLabel() override;
    virtual void SAL_CALL setLabel(const OUString& rLabel) override;

    // XSimpleText
    virtual rtl::Reference< SwXTextCursor > createXTextCursor() override;
    virtual rtl::Reference< SwXTextCursor > createXTextCursorByRange(
            const ::css::uno::Reference< ::css::text::XTextRange >& aTextPosition ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
