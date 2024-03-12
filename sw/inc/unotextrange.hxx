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

#ifndef INCLUDED_SW_INC_UNOTEXTRANGE_HXX
#define INCLUDED_SW_INC_UNOTEXTRANGE_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XRedline.hpp>

#include <cppuhelper/implbase.hxx>

#include "pam.hxx"
#include "unobaseclass.hxx"

class SwDoc;
class SwUnoCursor;
class SwFrameFormat;
class SwXText;

class SW_DLLPUBLIC SwUnoInternalPaM final
    : public SwPaM
{

private:
    SwUnoInternalPaM(const SwUnoInternalPaM&) = delete;

public:
    SwUnoInternalPaM(SwDoc& rDoc);
    virtual ~SwUnoInternalPaM() override;

    SwUnoInternalPaM& operator=(const SwPaM& rPaM);
};

namespace sw {

    enum class TextRangeMode {
        RequireTextNode,
        AllowNonTextNode,
        AllowTableNode
    };

    void DeepCopyPaM(SwPaM const & rSource, SwPaM & rTarget);

    SW_DLLPUBLIC bool XTextRangeToSwPaM(SwUnoInternalPaM& rToFill,
            const css::uno::Reference<css::text::XTextRange> & xTextRange,
            TextRangeMode eMode = TextRangeMode::RequireTextNode);

    css::uno::Reference< SwXText >
        CreateParentXText(SwDoc & rDoc, const SwPosition& rPos);

    bool GetDefaultTextContentValue(css::uno::Any& rAny,
        std::u16string_view rPropertyName, sal_uInt16 nWID = 0);

} // namespace sw

typedef ::cppu::WeakImplHelper
<   css::lang::XServiceInfo
,   css::beans::XPropertySet
,   css::beans::XPropertyState
,   css::container::XEnumerationAccess
,   css::container::XContentEnumerationAccess
,   css::text::XTextRange
,   css::text::XRedline
> SwXTextRange_Base;

class SAL_DLLPUBLIC_RTTI SwXTextRange final
    : public SwXTextRange_Base
{

private:

    friend class SwXText;

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    void    SetPositions(SwPaM const& rPam);
    //TODO: new exception type for protected content
    /// @throws css::uno::RuntimeException
    void    DeleteAndInsert(
                std::u16string_view aText, ::sw::DeleteAndInsertMode eMode);
    void    Invalidate();

    virtual ~SwXTextRange() override;

public:

    enum RangePosition
    {
        RANGE_IN_TEXT,  // "ordinary" css::text::TextRange
        RANGE_IN_CELL,  // position created with a cell that has no uno object
        RANGE_IS_TABLE, // anchor of a table
        RANGE_IS_SECTION, // anchor of a section
    };

    SW_DLLPUBLIC SwXTextRange(SwPaM const & rPam,
            const css::uno::Reference< css::text::XText > & xParent,
            const enum RangePosition eRange = RANGE_IN_TEXT);
    // only for RANGE_IS_TABLE
    SwXTextRange(SwTableFormat& rTableFormat);
    // only for RANGE_IS_SECTION
    SwXTextRange(SwSectionFormat& rSectionFormat);

    const SwDoc& GetDoc() const;
          SwDoc& GetDoc();
    SW_DLLPUBLIC bool GetPositions(SwPaM & rToFill,
        ::sw::TextRangeMode eMode = ::sw::TextRangeMode::RequireTextNode) const;

    SW_DLLPUBLIC static rtl::Reference< SwXTextRange > CreateXTextRange(
            SwDoc & rDoc,
            const SwPosition& rPos, const SwPosition *const pMark);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

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

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL
        createEnumeration() override;

    // XContentEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
        createContentEnumeration(const OUString& rServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames() override;

    // XTextRange
    SW_DLLPUBLIC virtual css::uno::Reference< css::text::XText >
        SAL_CALL getText() override;
    SW_DLLPUBLIC virtual css::uno::Reference<
                css::text::XTextRange > SAL_CALL getStart() override;
    SW_DLLPUBLIC virtual css::uno::Reference<
                css::text::XTextRange > SAL_CALL getEnd() override;
    virtual OUString SAL_CALL getString() override;
    virtual void SAL_CALL setString(const OUString& rString) override;

    // XRedline
    virtual void SAL_CALL makeRedline(
            const OUString& rRedlineType,
            const css::uno::Sequence< css::beans::PropertyValue >& RedlineProperties) override;

};

typedef ::cppu::WeakImplHelper
<   css::lang::XServiceInfo
,   css::container::XIndexAccess
> SwXTextRanges_Base;

struct SwXTextRanges : public SwXTextRanges_Base
{
    virtual SwUnoCursor* GetCursor() =0;
    static rtl::Reference<SwXTextRanges> Create(SwPaM* const pCursor);
};

#endif // INCLUDED_SW_INC_UNOTEXTRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
