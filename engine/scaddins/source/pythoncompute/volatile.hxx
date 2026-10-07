/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/sheet/XVolatileResult.hpp>
#include <com/sun/star/sheet/XResultListener.hpp>
#include <com/sun/star/sheet/ResultEvent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <vector>

namespace co::pythoncompute
{
/** Async formula result: interim "#BUSY!", then modified() with the final Any. */
class PythonComputeVolatileResult final
    : public cppu::WeakImplHelper<css::sheet::XVolatileResult, css::lang::XServiceInfo>
{
public:
    PythonComputeVolatileResult();
    virtual ~PythonComputeVolatileResult() override;

    void finish(const cpo::uno::Any& aValue);

    // XVolatileResult
    virtual void
    addResultListener(const cpo::uno::Reference<css::sheet::XResultListener>& xListener) override;
    virtual void removeResultListener(
        const cpo::uno::Reference<css::sheet::XResultListener>& xListener) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

private:
    void notifyListeners(const cpo::uno::Any& aValue);

    std::vector<cpo::uno::Reference<css::sheet::XResultListener>> m_aListeners;
    cpo::uno::Any m_aCurrent;
    bool m_bFinished = false;
};

} // namespace co::pythoncompute

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
