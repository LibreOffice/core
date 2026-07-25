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

#include <ChartType.hxx>

namespace chart
{
/** A correlation circle plot. One series per feature, each holding the feature
    column under the role values-feature and the two dimension columns under
    values-x and values-y.
 */
class CorrelationCircleChartType final : public ChartType
{
public:
    explicit CorrelationCircleChartType();
    virtual ~CorrelationCircleChartType() override;

    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& rServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    virtual rtl::Reference<ChartType> cloneChartType() const override;

private:
    explicit CorrelationCircleChartType(const CorrelationCircleChartType& rOther);

    // ____ XChartType ____
    virtual OUString getChartType() override;
    virtual cpo::uno::Sequence<OUString> getSupportedMandatoryRoles() override;
    virtual cpo::uno::Sequence<OUString> getSupportedPropertyRoles() override;
    virtual OUString getRoleOfSequenceForSeriesLabel() override;

    // ____ OPropertySet ____
    virtual void GetDefaultValue(sal_Int32 nHandle, cpo::uno::Any& rAny) const override;
    virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference<css::beans::XPropertySetInfo> getPropertySetInfo() override;

    // ____ XCloneable ____
    virtual css::uno::Reference<css::util::XCloneable> createClone() override;

    virtual rtl::Reference<::chart::BaseCoordinateSystem>
        createCoordinateSystem2(sal_Int32 DimensionCount) override;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
