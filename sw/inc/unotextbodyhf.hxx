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
#include <cppuhelper/implbase2.hxx>

#include <unotext.hxx>

class SwDoc;
class SwFrameFormat;
class SwXTextCursor;

typedef ::cppu::WeakAggImplHelper2
<   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::container::XEnumerationAccess
> SwXBodyText_Base;

class SwXBodyText
    : public SwXBodyText_Base
    , public SwXText
{

protected:

    virtual ~SwXBodyText();

public:

    SwXBodyText(SwDoc *const pDoc);

    SwXTextCursor * CreateTextCursor(const bool bIgnoreTables = false);

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type& rType)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw() override { OWeakObject::release(); }

    // XAggregation
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(
            const ::com::sun::star::uno::Type& rType)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
        SAL_CALL getTypes()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XSimpleText
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >  SAL_CALL
        createTextCursor()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextPosition)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

};

typedef ::cppu::WeakImplHelper
<   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::container::XEnumerationAccess
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
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >
        CreateCursor()
        throw (::com::sun::star::uno::RuntimeException) override;

    virtual ~SwXHeadFootText();

    SwXHeadFootText(SwFrameFormat & rHeadFootFormat, const bool bIsHeader);

public:

    static ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >
        CreateXHeadFootText(SwFrameFormat & rHeadFootFormat, const bool bIsHeader);
    static bool IsXHeadFootText(SwClient *const pClient);

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type& rType)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw() override { OWeakObject::release(); }

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
        SAL_CALL getTypes()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XSimpleText
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >  SAL_CALL
        createTextCursor()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextPosition)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

};

#endif // INCLUDED_SW_INC_UNOTEXTBODYHF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
