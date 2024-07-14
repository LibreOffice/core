/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
class HistogramChartType : public ChartType
{
public:
    explicit HistogramChartType();
    virtual ~HistogramChartType() override;

    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    virtual rtl::Reference<ChartType> cloneChartType() const override;

protected:
    explicit HistogramChartType(const HistogramChartType& rOther);

    // ____ XChartType ____
    rtl::Reference<::chart::BaseCoordinateSystem>
    createCoordinateSystem2(sal_Int32 DimensionCount) override;
    OUString SAL_CALL getChartType() override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedPropertyRoles() override;

    // ____ OPropertySet ____
    void GetDefaultValue(sal_Int32 nHandle, css::uno::Any& rAny) const override;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;

    // ____ XCloneable ____
    css::uno::Reference<css::util::XCloneable> SAL_CALL createClone() override;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
