/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <OPropertySet.hxx>
#include <comphelper/uno3.hxx>

#include <ChartTypeTemplate.hxx>

namespace chart
{
class FunnelChartTypeTemplate : public ChartTypeTemplate, public ::property::OPropertySet
{
public:
    FunnelChartTypeTemplate(css::uno::Reference<css::uno::XComponentContext> const& xContext,
                            const OUString& rServiceName, sal_Int32 nDim = 2);
    virtual ~FunnelChartTypeTemplate() override;

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

protected:
    // ____ OPropertySet ____
    virtual void GetDefaultValue(sal_Int32 nHandle, css::uno::Any& rAny) const override;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo() override;

    // ____ ChartTypeTemplate ____
    virtual bool matchesTemplate2(const rtl::Reference<::chart::Diagram>& xDiagram,
                                  bool bAdaptProperties) override;
    virtual rtl::Reference<::chart::ChartType> getChartTypeForNewSeries2(
        const std::vector<rtl::Reference<::chart::ChartType>>& aFormerlyUsedChartTypes) override;
    virtual void applyStyle2(const rtl::Reference<::chart::DataSeries>& xSeries,
                             ::sal_Int32 nChartTypeGroupIndex, ::sal_Int32 nSeriesIndex,
                             ::sal_Int32 nSeriesCount) override;
    virtual void resetStyles2(const rtl::Reference<::chart::Diagram>& xDiagram) override;

    // ____ ChartTypeTemplate ____
    virtual sal_Int32 getDimension() const override;

    virtual rtl::Reference<::chart::ChartType>
    getChartTypeForIndex(sal_Int32 nChartTypeIndex) override;

private:
    sal_Int32 m_nDim;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
