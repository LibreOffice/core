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

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <cppuhelper/implbase.hxx>

#include <unotext.hxx>

class SwDoc;
class SwFrameFormat;
class SwXTextCursor;

typedef ::cppu::WeakAggImplHelper
<   css::lang::XServiceInfo
,   css::container::XEnumerationAccess
> SwXBodyText_Base;

class SwXBodyText
    : public SwXBodyText_Base
    , public SwXText
{

protected:

    virtual ~SwXBodyText() override;

public:

    SwXBodyText(SwDoc *const pDoc);

    SwXTextCursor * CreateTextCursor(const bool bIgnoreTables = false);

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(
            const css::uno::Type& rType)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw() override { OWeakObject::release(); }

    // XAggregation
    virtual css::uno::Any SAL_CALL queryAggregation(
            const css::uno::Type& rType)
        throw (css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type >
        SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId()
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

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (css::uno::RuntimeException, std::exception) override;

    // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursor()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const css::uno::Reference< css::text::XTextRange > & xTextPosition)
        throw (css::uno::RuntimeException, std::exception) override;

};

typedef ::cppu::WeakImplHelper
<   css::lang::XServiceInfo
,   css::container::XEnumerationAccess
> SwXHeadFootText_Base;

class SwXHeadFootText
    : public SwXHeadFootText_Base
    , public SwXText
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

protected:

    virtual const SwStartNode *GetStartNode() const override;
    virtual css::uno::Reference< css::text::XTextCursor >
        CreateCursor()
        throw (css::uno::RuntimeException) override;

    virtual ~SwXHeadFootText() override;

    SwXHeadFootText(SwFrameFormat & rHeadFootFormat, const bool bIsHeader);

public:

    static css::uno::Reference< css::text::XText >
        CreateXHeadFootText(SwFrameFormat & rHeadFootFormat, const bool bIsHeader);
    static bool IsXHeadFootText(SwClient *const pClient);

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(
            const css::uno::Type& rType)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw() override { OWeakObject::release(); }

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type >
        SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId()
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

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (css::uno::RuntimeException, std::exception) override;

    // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursor()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const css::uno::Reference< css::text::XTextRange > & xTextPosition)
        throw (css::uno::RuntimeException, std::exception) override;

};

#endif // INCLUDED_SW_INC_UNOTEXTBODYHF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
