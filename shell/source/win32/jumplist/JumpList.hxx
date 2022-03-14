/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cppuhelper/compbase.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/system/windows/XJumpList.hpp>
#include <com/sun/star/system/windows/JumpListItem.hpp>

class JumpListImpl
    : public cppu::WeakComponentImplHelper<css::system::windows::XJumpList, css::lang::XServiceInfo>
{
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    osl::Mutex m_aMutex;

public:
    explicit JumpListImpl(const css::uno::Reference<css::uno::XComponentContext>& xContext);
    ~JumpListImpl();

    // XJumpList
    virtual void SAL_CALL
    appendCategory(const OUString& sCategory,
                   const css::uno::Sequence<css::system::windows::JumpListItem>& aJumpListItems,
                   const OUString& sDocumentService) override;
    virtual css::uno::Sequence<css::system::windows::JumpListItem>
        SAL_CALL getRemovedItems() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
