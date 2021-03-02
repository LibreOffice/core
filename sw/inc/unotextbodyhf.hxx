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

#ifndef INCLUDED_SW_INC_UNOTEXTBODYHF_HXX
#define INCLUDED_SW_INC_UNOTEXTBODYHF_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implbase2.hxx>

#include "unotext.hxx"

class SwDoc;
class SwFrameFormat;
class SwXTextCursor;

typedef ::cppu::WeakAggImplHelper2
<   css::lang::XServiceInfo
,   css::container::XEnumerationAccess
> SwXBodyText_Base;

class SwXBodyText final
    : public SwXBodyText_Base
    , public SwXText
{

    virtual ~SwXBodyText() override;

public:

    SwXBodyText(SwDoc *const pDoc);

    rtl::Reference<SwXTextCursor> CreateTextCursor(const bool bIgnoreTables = false);

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(
            const css::uno::Type& rType) override;
    virtual void SAL_CALL acquire() throw() override { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw() override { OWeakObject::release(); }

    // XAggregation
    virtual css::uno::Any SAL_CALL queryAggregation(
            const css::uno::Type& rType) override;

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

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL
        createEnumeration() override;

    // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursor() override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const css::uno::Reference< css::text::XTextRange > & xTextPosition) override;

};

typedef ::cppu::WeakImplHelper
<   css::lang::XServiceInfo
,   css::container::XEnumerationAccess
> SwXHeadFootText_Base;

class SwXHeadFootText final
    : public SwXHeadFootText_Base
    , public SwXText
{
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual const SwStartNode *GetStartNode() const override;
    virtual css::uno::Reference< css::text::XTextCursor >
        CreateCursor() override;

    virtual ~SwXHeadFootText() override;

    SwXHeadFootText(SwFrameFormat & rHeadFootFormat, const bool bIsHeader);

public:

    static css::uno::Reference< css::text::XText >
        CreateXHeadFootText(SwFrameFormat & rHeadFootFormat, const bool bIsHeader);

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(
            const css::uno::Type& rType) override;
    virtual void SAL_CALL acquire() throw() override { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw() override { OWeakObject::release(); }

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

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL
        createEnumeration() override;

    // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursor() override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const css::uno::Reference< css::text::XTextRange > & xTextPosition) override;

};

#endif // INCLUDED_SW_INC_UNOTEXTBODYHF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
