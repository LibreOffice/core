/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ChartType.hxx>

namespace chart
{
class FunnelChartType final : public ChartType
{
public:
    explicit FunnelChartType();
    virtual ~FunnelChartType() override;

    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    virtual rtl::Reference<ChartType> cloneChartType() const override;

private:
    explicit FunnelChartType(const FunnelChartType& rOther);

    // ____ XChartType ____
    virtual OUString SAL_CALL getChartType() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedPropertyRoles() override;

    // ____ XCloneable ____
    virtual css::uno::Reference<css::util::XCloneable> SAL_CALL createClone() override;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
