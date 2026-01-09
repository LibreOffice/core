/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ParetoLineChartTypeTemplate.hxx"
#include "ParetoLineChartType.hxx"
#include <Diagram.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{
enum
{
    PROP_PARETOLINE_TEMPLATE_DIMENSION,
    PROP_PARETOLINE_TEMPLATE_GEOMETRY3D
};

void lcl_AddPropertiesToVector(std::vector<Property>& rOutProperties)
{
    rOutProperties.emplace_back(
        "Dimension", PROP_PARETOLINE_TEMPLATE_DIMENSION, cppu::UnoType<sal_Int32>::get(),
        beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT);
    rOutProperties.emplace_back(
        "Geometry3D", PROP_PARETOLINE_TEMPLATE_GEOMETRY3D, cppu::UnoType<sal_Int32>::get(),
        beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT);
}

::cppu::OPropertyArrayHelper& StaticParetoLineChartTypeTemplateInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []() {
        std::vector<css::beans::Property> aProperties;
        lcl_AddPropertiesToVector(aProperties);

        std::sort(aProperties.begin(), aProperties.end(), ::chart::PropertyNameLess());

        return comphelper::containerToSequence(aProperties);
    }();
    return aPropHelper;
};

} // anonymous namespace

namespace chart
{
ParetoLineChartTypeTemplate::ParetoLineChartTypeTemplate(
    Reference<uno::XComponentContext> const& xContext, const OUString& rServiceName,
    sal_Int32 nDim /* = 2 */)
    : ChartTypeTemplate(xContext, rServiceName)
    , m_nDim(nDim)
{
}

ParetoLineChartTypeTemplate::~ParetoLineChartTypeTemplate() {}

sal_Int32 ParetoLineChartTypeTemplate::getDimension() const { return m_nDim; }

// ____ ChartTypeTemplate ____
bool ParetoLineChartTypeTemplate::matchesTemplate2(const rtl::Reference<::chart::Diagram>& xDiagram,
                                                   bool bAdaptProperties)
{
    bool bResult = ChartTypeTemplate::matchesTemplate2(xDiagram, bAdaptProperties);

    // adapt solid-type of template according to values in series
    if (bAdaptProperties && bResult && getDimension() == 3)
    {
        bool bGeomFound = false, bGeomAmbiguous = false;
        sal_Int32 aCommonGeom = xDiagram->getGeometry3D(bGeomFound, bGeomAmbiguous);

        if (!bGeomAmbiguous)
        {
            setFastPropertyValue_NoBroadcast(PROP_PARETOLINE_TEMPLATE_GEOMETRY3D,
                                             uno::Any(aCommonGeom));
        }
    }

    return bResult;
}

rtl::Reference<ChartType>
    ParetoLineChartTypeTemplate::getChartTypeForIndex(sal_Int32 /*nChartTypeIndex*/)
{
    return new ParetoLineChartType();
}

rtl::Reference<ChartType> ParetoLineChartTypeTemplate::getChartTypeForNewSeries2(
    const std::vector<rtl::Reference<ChartType>>& aFormerlyUsedChartTypes)
{
    rtl::Reference<ChartType> xResult(getChartTypeForIndex(0));
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem(aFormerlyUsedChartTypes, xResult);
    return xResult;
}

// ____ OPropertySet ____
void ParetoLineChartTypeTemplate::GetDefaultValue(sal_Int32 nHandle, uno::Any& rAny) const
{
    static ::chart::tPropertyValueMap aStaticDefaults = []() {
        ::chart::tPropertyValueMap aTmp;
        ::chart::PropertyHelper::setPropertyValueDefault<sal_Int32>(
            aTmp, PROP_PARETOLINE_TEMPLATE_DIMENSION, 2);
        ::chart::PropertyHelper::setPropertyValueDefault(aTmp, PROP_PARETOLINE_TEMPLATE_GEOMETRY3D,
                                                         ::chart2::DataPointGeometry3D::CUBOID);
        return aTmp;
    }();
    tPropertyValueMap::const_iterator aFound(aStaticDefaults.find(nHandle));
    if (aFound == aStaticDefaults.end())
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper& SAL_CALL ParetoLineChartTypeTemplate::getInfoHelper()
{
    return StaticParetoLineChartTypeTemplateInfoHelper();
}

// ____ XPropertySet ____
Reference<beans::XPropertySetInfo> SAL_CALL ParetoLineChartTypeTemplate::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(
            StaticParetoLineChartTypeTemplateInfoHelper()));
    return xPropertySetInfo;
}

void ParetoLineChartTypeTemplate::applyStyle2(const rtl::Reference<DataSeries>& xSeries,
                                              ::sal_Int32 nChartTypeIndex, ::sal_Int32 nSeriesIndex,
                                              ::sal_Int32 nSeriesCount)
{
    ChartTypeTemplate::applyStyle2(xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount);
    xSeries->setPropertyAlsoToAllAttributedDataPoints("BorderStyle",
                                                      uno::Any(drawing::LineStyle_NONE));
    if (getDimension() != 3)
        return;

    try
    {
        //apply Geometry3D
        uno::Any aAGeometry3D;
        getFastPropertyValue(aAGeometry3D, PROP_PARETOLINE_TEMPLATE_GEOMETRY3D);
        xSeries->setPropertyAlsoToAllAttributedDataPoints("Geometry3D", aAGeometry3D);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ParetoLineChartTypeTemplate::resetStyles2(const rtl::Reference<::chart::Diagram>& xDiagram)
{
    ChartTypeTemplate::resetStyles2(xDiagram);
    std::vector<rtl::Reference<DataSeries>> aSeriesVec(xDiagram->getDataSeries());
    uno::Any aLineStyleAny(drawing::LineStyle_NONE);
    for (auto const& series : aSeriesVec)
    {
        if (getDimension() == 3)
            series->setPropertyToDefault("Geometry3D");
        if (series->getPropertyValue("BorderStyle") == aLineStyleAny)
        {
            series->setPropertyToDefault("BorderStyle");
        }
    }

    xDiagram->setVertical(false);
}

IMPLEMENT_FORWARD_XINTERFACE2(ParetoLineChartTypeTemplate, ChartTypeTemplate, OPropertySet)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(ParetoLineChartTypeTemplate, ChartTypeTemplate, OPropertySet)

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
