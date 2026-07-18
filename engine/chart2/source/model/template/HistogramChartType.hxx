/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <ChartModel.hxx>
#include <ChartType.hxx>
#include <Diagram.hxx>

namespace chart
{
class HistogramChartType : public ChartType
{
public:
    explicit HistogramChartType();
    virtual ~HistogramChartType() override;

    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    virtual rtl::Reference<ChartType> cloneChartType() const override;

protected:
    explicit HistogramChartType(const HistogramChartType& rOther);

    // ____ XChartType ____
    rtl::Reference<::chart::BaseCoordinateSystem>
    createCoordinateSystem2(sal_Int32 DimensionCount) override;

    virtual void createCalculatedDataSeries() override;

    OUString getChartType() override;
    cpo::uno::Sequence<OUString> getSupportedPropertyRoles() override;
    cpo::uno::Sequence<OUString> getSupportedMandatoryRoles() override;
    cpo::uno::Sequence<OUString> getSupportedOptionalRoles() override;

    // ____ OPropertySet ____
    void GetDefaultValue(sal_Int32 nHandle, cpo::uno::Any& rAny) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;
    virtual void firePropertyChangeEvent() override;

    // ____ XPropertySet ____
    css::uno::Reference<css::beans::XPropertySetInfo> getPropertySetInfo() override;

    // ____ XCloneable ____
    css::uno::Reference<css::util::XCloneable> createClone() override;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */