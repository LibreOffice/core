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
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XTextContent.hpp>

#include <unobaseclass.hxx>

class SwDoc;
class SwFormatLineBreak;

/// UNO API wrapper around an SwFormatLineBreak, exposed as the com.sun.star.text.LineBreak service.
class SW_DLLPUBLIC SwXLineBreak final
    : public cppu::WeakImplHelper<css::beans::XPropertySet, css::lang::XServiceInfo,
                                  css::text::XTextContent>
{
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    SwXLineBreak(SwFormatLineBreak& rFormat);
    SwXLineBreak();

    ~SwXLineBreak() override;

public:
    static rtl::Reference<SwXLineBreak> CreateXLineBreak(SwFormatLineBreak* pLineBreakFormat);

    // XPropertySet
    css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;
    void SAL_CALL setPropertyValue(const OUString& rPropertyName,
                                   const css::uno::Any& rValue) override;
    css::uno::Any SAL_CALL getPropertyValue(const OUString& rPropertyName) override;
    void SAL_CALL addPropertyChangeListener(
        const OUString& rPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& xListener) override;
    void SAL_CALL removePropertyChangeListener(
        const OUString& rPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& xListener) override;
    void SAL_CALL addVetoableChangeListener(
        const OUString& rPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& xListener) override;
    void SAL_CALL removeVetoableChangeListener(
        const OUString& rPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& xListener) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XTextContent
    void SAL_CALL attach(const css::uno::Reference<css::text::XTextRange>& xTextRange) override;
    css::uno::Reference<css::text::XTextRange> SAL_CALL getAnchor() override;

    // XComponent, via XTextContent
    void SAL_CALL dispose() override;
    void SAL_CALL
    addEventListener(const css::uno::Reference<css::lang::XEventListener>& xListener) override;
    void SAL_CALL
    removeEventListener(const css::uno::Reference<css::lang::XEventListener>& xListener) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
