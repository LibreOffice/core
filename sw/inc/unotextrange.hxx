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

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XRedline.hpp>

#include <cppuhelper/implbase.hxx>

#include <pam.hxx>
#include <unobaseclass.hxx>

class SwDoc;
class SwUnoCrsr;
class SwFrameFormat;

class SW_DLLPUBLIC SwUnoInternalPaM
    : public SwPaM
{

private:
    SwUnoInternalPaM(const SwUnoInternalPaM&) = delete;

public:
    SwUnoInternalPaM(SwDoc& rDoc);
    virtual ~SwUnoInternalPaM();

    SwUnoInternalPaM& operator=(const SwPaM& rPaM);
};

namespace sw {

    void DeepCopyPaM(SwPaM const & rSource, SwPaM & rTarget);

    SW_DLLPUBLIC bool XTextRangeToSwPaM(SwUnoInternalPaM& rToFill,
            const css::uno::Reference< css::text::XTextRange > & xTextRange);

    css::uno::Reference< css::text::XText >
        CreateParentXText(SwDoc & rDoc, const SwPosition& rPos);

    bool GetDefaultTextContentValue(css::uno::Any& rAny,
        const OUString& rPropertyName, sal_uInt16 nWID = 0);

} // namespace sw

typedef ::cppu::WeakImplHelper
<   css::lang::XUnoTunnel
,   css::lang::XServiceInfo
,   css::beans::XPropertySet
,   css::beans::XPropertyState
,   css::container::XEnumerationAccess
,   css::container::XContentEnumerationAccess
,   css::text::XTextRange
,   css::text::XRedline
> SwXTextRange_Base;

class SW_DLLPUBLIC SwXTextRange
    : public SwXTextRange_Base
{

private:

    friend class SwXText;

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    enum RangePosition
    {
        RANGE_IN_TEXT,  // "ordinary" css::text::TextRange
        RANGE_IN_CELL,  // position created with a cell that has no uno object
        RANGE_IS_TABLE, // anchor of a table
    };

    void    SetPositions(SwPaM const& rPam);
    //TODO: new exception type for protected content
    void    DeleteAndInsert(
                const OUString& rText, const bool bForceExpandHints)
        throw (css::uno::RuntimeException);
    void    Invalidate();

    virtual ~SwXTextRange();

public:

    SwXTextRange(SwPaM& rPam,
            const css::uno::Reference< css::text::XText > & xParent,
            const enum RangePosition eRange = RANGE_IN_TEXT);
    // only for RANGE_IS_TABLE
    SwXTextRange(SwFrameFormat& rTableFormat);

    const SwDoc& GetDoc() const;
          SwDoc& GetDoc();
    bool GetPositions(SwPaM & rToFill) const;

    static css::uno::Reference< css::text::XTextRange > CreateXTextRange(
            SwDoc & rDoc,
            const SwPosition& rPos, const SwPosition *const pMark);

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const css::uno::Sequence< sal_Int8 >& rIdentifier)
        throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue)
        throw (css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL
        getPropertyState(const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
        getPropertyStates(
            const css::uno::Sequence< OUString >& rPropertyNames)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (css::uno::RuntimeException, std::exception) override;

    // XContentEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
        createContentEnumeration(const OUString& rServiceName)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // XTextRange
    virtual css::uno::Reference< css::text::XText >
        SAL_CALL getText()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
                css::text::XTextRange > SAL_CALL getStart()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
                css::text::XTextRange > SAL_CALL getEnd()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setString(const OUString& rString)
        throw (css::uno::RuntimeException, std::exception) override;

    // XRedline
    virtual void SAL_CALL makeRedline(
            const OUString& rRedlineType,
            const css::uno::Sequence< css::beans::PropertyValue >& RedlineProperties)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;

};

typedef ::cppu::WeakImplHelper
<   css::lang::XUnoTunnel
,   css::lang::XServiceInfo
,   css::container::XIndexAccess
> SwXTextRanges_Base;

struct SwXTextRanges : public SwXTextRanges_Base
{
    virtual SwUnoCrsr* GetCursor() =0;
    static SwXTextRanges* Create(SwPaM* const pCrsr);
    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();
};

#endif // INCLUDED_SW_INC_UNOTEXTRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
