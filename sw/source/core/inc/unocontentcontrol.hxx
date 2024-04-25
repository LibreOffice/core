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
#include <memory>
#include <deque>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppuhelper/implbase.hxx>

#include <unobaseclass.hxx>
#include <unocoll.hxx>

class SwPaM;
class SwTextNode;
class SwFormatContentControl;
class SwContentControl;
class SwXText;
class SwXTextPortion;

typedef std::deque<rtl::Reference<SwXTextPortion>> TextRangeList_t;

/**
 * UNO API wrapper around an SwContentControl, exposed as the com.sun.star.text.ContentControl
 * service.
 */
class SW_DLLPUBLIC SwXContentControl final
    : public cppu::WeakImplHelper<css::lang::XServiceInfo, css::container::XEnumerationAccess,
                                  css::text::XTextContent, css::text::XText,
                                  css::beans::XPropertySet>
{
    class Impl;
    sw::UnoImplPtr<Impl> m_pImpl;

protected:
    void AttachImpl(const css::uno::Reference<css::text::XTextRange>& xTextRange,
                    sal_uInt16 nWhich);

    ~SwXContentControl() override;

    SwXContentControl(const SwXContentControl&) = delete;
    SwXContentControl& operator=(const SwXContentControl&) = delete;

    SwXContentControl(SwDoc* pDoc, SwContentControl* pContentControl,
                      const css::uno::Reference<SwXText>& xParentText,
                      std::unique_ptr<const TextRangeList_t> pPortions);

    SwXContentControl(SwDoc* pDoc);

public:
    static rtl::Reference<SwXContentControl>
    CreateXContentControl(SwContentControl& rContentControl,
                          const css::uno::Reference<SwXText>& xParentText = nullptr,
                          std::unique_ptr<const TextRangeList_t>&& pPortions
                          = std::unique_ptr<const TextRangeList_t>());

    static rtl::Reference<SwXContentControl> CreateXContentControl(SwDoc& rDoc);

    /// Initializes params with position of the attribute content (without CH_TXTATR).
    bool SetContentRange(SwTextNode*& rpNode, sal_Int32& rStart, sal_Int32& rEnd) const;
    const css::uno::Reference<SwXText>& GetParentText() const;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XComponent
    void SAL_CALL dispose() override;
    void SAL_CALL
    addEventListener(const css::uno::Reference<css::lang::XEventListener>& xListener) override;
    void SAL_CALL
    removeEventListener(const css::uno::Reference<css::lang::XEventListener>& xListener) override;

    // XElementAccess
    css::uno::Type SAL_CALL getElementType() override;
    sal_Bool SAL_CALL hasElements() override;

    // XEnumerationAccess
    css::uno::Reference<css::container::XEnumeration> SAL_CALL createEnumeration() override;

    // XTextContent
    void SAL_CALL attach(const css::uno::Reference<css::text::XTextRange>& xTextRange) override;
    css::uno::Reference<css::text::XTextRange> SAL_CALL getAnchor() override;

    // XTextRange
    css::uno::Reference<css::text::XText> SAL_CALL getText() override;
    css::uno::Reference<css::text::XTextRange> SAL_CALL getStart() override;
    css::uno::Reference<css::text::XTextRange> SAL_CALL getEnd() override;
    OUString SAL_CALL getString() override;
    void SAL_CALL setString(const OUString& rString) override;

    // XSimpleText
    css::uno::Reference<css::text::XTextCursor> SAL_CALL createTextCursor() override;
    css::uno::Reference<css::text::XTextCursor> SAL_CALL createTextCursorByRange(
        const css::uno::Reference<css::text::XTextRange>& xTextPosition) override;
    void SAL_CALL insertString(const css::uno::Reference<css::text::XTextRange>& xRange,
                               const OUString& aString, sal_Bool bAbsorb) override;
    void SAL_CALL insertControlCharacter(const css::uno::Reference<css::text::XTextRange>& xRange,
                                         sal_Int16 nControlCharacter, sal_Bool bAbsorb) override;

    // XText
    void SAL_CALL insertTextContent(const css::uno::Reference<css::text::XTextRange>& xRange,
                                    const css::uno::Reference<css::text::XTextContent>& xContent,
                                    sal_Bool bAbsorb) override;
    void SAL_CALL
    removeTextContent(const css::uno::Reference<css::text::XTextContent>& xContent) override;

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
};

/// UNO wrapper around SwContentControlManager.
class SwXContentControls final : public cppu::WeakImplHelper<css::container::XIndexAccess>,
                                 public SwUnoCollection
{
    ~SwXContentControls() override;

public:
    SwXContentControls(SwDoc* pDoc);

    // XIndexAccess
    sal_Int32 SAL_CALL getCount() override;
    css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    // XElementAccess
    css::uno::Type SAL_CALL getElementType() override;
    sal_Bool SAL_CALL hasElements() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
