/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>

class ScExcelBiffDetect
    : public cppu::WeakImplHelper<css::document::XExtendedFilterDetection, css::lang::XServiceInfo>
{
public:
    explicit ScExcelBiffDetect();
    virtual ~ScExcelBiffDetect() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& aName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XExtendedFilterDetection
    virtual OUString SAL_CALL
    detect(css::uno::Sequence<css::beans::PropertyValue>& lDescriptor) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
