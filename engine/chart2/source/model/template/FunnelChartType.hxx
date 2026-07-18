/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    virtual rtl::Reference<ChartType> cloneChartType() const override;

private:
    explicit FunnelChartType(const FunnelChartType& rOther);

    // ____ XChartType ____
    virtual OUString getChartType() override;
    virtual cpo::uno::Sequence<OUString> getSupportedPropertyRoles() override;

    // ____ OPropertySet ____
    virtual void GetDefaultValue(sal_Int32 nHandle, cpo::uno::Any& rAny) const override;
    virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference<css::beans::XPropertySetInfo> getPropertySetInfo() override;

    // ____ XCloneable ____
    virtual css::uno::Reference<css::util::XCloneable> createClone() override;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
