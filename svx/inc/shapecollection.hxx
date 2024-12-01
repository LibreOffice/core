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

#include <cppuhelper/implbase3.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <mutex>
#include <vector>

class SvxShapeCollection final
    : public cppu::WeakAggImplHelper3<css::drawing::XShapes, css::lang::XServiceInfo,
                                      css::lang::XComponent>
{
private:
    mutable std::mutex m_aMutex;
    std::vector<css::uno::Reference<css::drawing::XShape>> maShapeContainer;
    comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> maEventListeners;
    bool bDisposed = false;
    bool bInDispose = false;

public:
    SvxShapeCollection() noexcept;

    // XInterface
    virtual void SAL_CALL release() noexcept override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL
    addEventListener(const css::uno::Reference<css::lang::XEventListener>& aListener) override;
    virtual void SAL_CALL
    removeEventListener(const css::uno::Reference<css::lang::XEventListener>& aListener) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 Index) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XShapes
    virtual void SAL_CALL add(const css::uno::Reference<css::drawing::XShape>& xShape) override;
    virtual void SAL_CALL remove(const css::uno::Reference<css::drawing::XShape>& xShape) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    std::vector<css::uno::Reference<css::drawing::XShape>> getAllShapes() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
