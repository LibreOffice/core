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

#include <memory>
#include <deque>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include <cppuhelper/implbase.hxx>

#include <sfx2/Metadatable.hxx>

#include <unobaseclass.hxx>

class SwXTextPortion;
class SwPaM;
class SwTextNode;
class SwXText;
namespace sw {
    class Meta;
}

typedef std::deque<
    rtl::Reference<SwXTextPortion> >
    TextRangeList_t;



typedef ::cppu::ImplInheritanceHelper
<   ::sfx2::MetadatableMixin
,   css::lang::XServiceInfo
,   css::container::XChild
,   css::container::XEnumerationAccess
,   css::text::XTextContent
,   css::text::XText
> SwXMeta_Base;

class SwXMeta
    : public SwXMeta_Base
{

public:

    class Impl;

protected:

    ::sw::UnoImplPtr<Impl> m_pImpl;

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void AttachImpl(
            const css::uno::Reference< css::text::XTextRange > & xTextRange,
            const sal_uInt16 nWhich);

    virtual ~SwXMeta() override;

    SwXMeta(SwXMeta const&) = delete;
    SwXMeta& operator=(SwXMeta const&) = delete;

    /// @param pDoc and pMeta != 0, but not & because of ImplInheritanceHelper
    SwXMeta(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        css::uno::Reference<SwXText> const&  xParentText,
        std::unique_ptr<TextRangeList_t const> pPortions);

    SwXMeta(SwDoc *const pDoc);

public:

    static rtl::Reference<SwXMeta>
        CreateXMeta(
            ::sw::Meta & rMeta,
            css::uno::Reference<SwXText> xParentText,
            std::unique_ptr<TextRangeList_t const> && pPortions);

    static rtl::Reference<SwXMeta>
        CreateXMeta(SwDoc & rDoc, bool isField);

    /// init params with position of the attribute content (w/out CH_TXTATR)
    bool SetContentRange( SwTextNode *& rpNode, sal_Int32 & rStart, sal_Int32 & rEnd) const;
    css::uno::Reference< SwXText > const & GetParentText() const;

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    bool CheckForOwnMemberMeta(const SwPaM & rPam, const bool bAbsorb);

    // MetadatableMixin
    virtual ::sfx2::Metadatable * GetCoreObject() override;
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

    // XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
        getParent() override;
    virtual void SAL_CALL setParent(
            css::uno::Reference< css::uno::XInterface> const& xParent) override;

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

    // XTextRange
    virtual css::uno::Reference< css::text::XText >
        SAL_CALL getText() override;
    virtual css::uno::Reference<
                css::text::XTextRange > SAL_CALL getStart() override;
    virtual css::uno::Reference<
                css::text::XTextRange > SAL_CALL getEnd() override;
    virtual OUString SAL_CALL getString() override;
    virtual void SAL_CALL setString(const OUString& rString) override;

    // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursor() override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const css::uno::Reference< css::text::XTextRange > & xTextPosition) override;
    virtual void SAL_CALL insertString(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            const OUString& aString, sal_Bool bAbsorb) override;
    virtual void SAL_CALL insertControlCharacter(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            sal_Int16 nControlCharacter, sal_Bool bAbsorb) override;

    // XText
    virtual void SAL_CALL insertTextContent(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            const css::uno::Reference< css::text::XTextContent > & xContent,
            sal_Bool bAbsorb) override;
    virtual void SAL_CALL removeTextContent(
            const css::uno::Reference< css::text::XTextContent > & xContent) override;

};

typedef ::cppu::ImplInheritanceHelper
<   SwXMeta
,   css::beans::XPropertySet
,   css::text::XTextField
> SwXMetaField_Base;

class SwXMetaField final
    : public SwXMetaField_Base
{

private:

    virtual ~SwXMetaField() override;

    friend rtl::Reference<SwXMeta>
        SwXMeta::CreateXMeta(::sw::Meta &,
            css::uno::Reference<SwXText>,
            std::unique_ptr<TextRangeList_t const> && pPortions);

    SwXMetaField(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        css::uno::Reference<SwXText> const& xParentText,
        std::unique_ptr<TextRangeList_t const> pPortions);

    friend rtl::Reference<SwXMeta>
        SwXMeta::CreateXMeta(SwDoc &, bool);

    SwXMetaField(SwDoc *const pDoc);

public:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames( ) override;

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
    virtual css::uno::Any SAL_CALL
        getPropertyValue(const OUString& rPropertyName) override;
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

    // XTextContent
    virtual void SAL_CALL attach(
            const css::uno::Reference< css::text::XTextRange > & xTextRange) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor() override;

    // XTextField
    virtual OUString SAL_CALL getPresentation(sal_Bool bShowCommand) override;

};

/// get prefix/suffix from the RDF repository. @throws RuntimeException
void getPrefixAndSuffix(
        const css::uno::Reference< css::frame::XModel>& xModel,
        const css::uno::Reference< css::rdf::XMetadatable>& xMetaField,
        OUString *const o_pPrefix, OUString *const o_pSuffix, OUString *const o_pShadowColor);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
