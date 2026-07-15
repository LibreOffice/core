/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "FunnelChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <CartesianCoordinateSystem.hxx>
#include <PropertyHelper.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace ::com::sun::star;

using namespace ::com::sun::star;
using ::cpo::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{
enum
{
    PROP_FUNNELCHARTTYPE_OVERLAP_SEQUENCE,
    PROP_FUNNELCHARTTYPE_GAPWIDTH_SEQUENCE
};

void lcl_AddPropertiesToVector(std::vector<Property>& rOutProperties)
{
    rOutProperties.emplace_back("OverlapSequence", PROP_FUNNELCHARTTYPE_OVERLAP_SEQUENCE,
                                cppu::UnoType<Sequence<sal_Int32>>::get(),
                                beans::PropertyAttribute::BOUND
                                    | beans::PropertyAttribute::MAYBEDEFAULT);

    rOutProperties.emplace_back("GapwidthSequence", PROP_FUNNELCHARTTYPE_GAPWIDTH_SEQUENCE,
                                cppu::UnoType<Sequence<sal_Int32>>::get(),
                                beans::PropertyAttribute::BOUND
                                    | beans::PropertyAttribute::MAYBEDEFAULT);
}

::cppu::OPropertyArrayHelper& StaticFunnelChartTypeInfoHelper()
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
FunnelChartType::FunnelChartType() {}

FunnelChartType::FunnelChartType(const FunnelChartType& rOther)
    : ChartType(rOther)
{
}

FunnelChartType::~FunnelChartType() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> FunnelChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new FunnelChartType(*this));
}

rtl::Reference<ChartType> FunnelChartType::cloneChartType() const
{
    return new FunnelChartType(*this);
}

// ____ XChartType ____
OUString FunnelChartType::getChartType() { return CHART2_SERVICE_NAME_CHARTTYPE_FUNNEL; }

cpo::uno::Sequence<OUString> FunnelChartType::getSupportedPropertyRoles()
{
    return { u"FillColor"_ustr, u"BorderColor"_ustr };
}

// ____ OPropertySet ____
void FunnelChartType::GetDefaultValue(sal_Int32 nHandle, cpo::uno::Any& rAny) const
{
    static const ::chart::tPropertyValueMap aStaticDefaults = []() {
        ::chart::tPropertyValueMap aTmp;
        Sequence<sal_Int32> aSeq{ 0, 0 };
        ::chart::PropertyHelper::setPropertyValueDefault(
            aTmp, PROP_FUNNELCHARTTYPE_OVERLAP_SEQUENCE, aSeq);
        aSeq = { 100, 100 };
        ::chart::PropertyHelper::setPropertyValueDefault(
            aTmp, PROP_FUNNELCHARTTYPE_GAPWIDTH_SEQUENCE, aSeq);
        return aTmp;
    }();
    tPropertyValueMap::const_iterator aFound(aStaticDefaults.find(nHandle));
    if (aFound == aStaticDefaults.end())
        rAny.clear();
    else
        rAny = (*aFound).second;
}

rtl::Reference<::chart::BaseCoordinateSystem>
FunnelChartType::createCoordinateSystem2(sal_Int32 DimensionCount)
{
    rtl::Reference<CartesianCoordinateSystem> xResult
        = new CartesianCoordinateSystem(DimensionCount);

    for (sal_Int32 i = 0; i < DimensionCount; i++)
    {
        rtl::Reference<Axis> xAxis = xResult->getAxisByDimension2(i, MAIN_AXIS_INDEX);
        if (!xAxis.is())
            continue;

        chart2::ScaleData aScaleData = xAxis->getScaleData();

        if (i == 0) // X-axis
        {
            // Set up X-axis specifically for histogram bins
            aScaleData.AxisType = chart2::AxisType::CATEGORY;
            aScaleData.AutoDateAxis = false;
            aScaleData.ShiftedCategoryPosition = true;
            aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;

            // Clear any existing scaling/categories
            AxisHelper::removeExplicitScaling(aScaleData);
            aScaleData.Categories.clear();
        }
        else if (i == 1) // Y-axis
        {
            aScaleData.AxisType = chart2::AxisType::REALNUMBER;
            aScaleData.ShiftedCategoryPosition = false;
            aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
            aScaleData.Scaling = AxisHelper::createLinearScaling();
        }

        xAxis->setScaleData(aScaleData);
    }

    return xResult;
}

::cppu::IPropertyArrayHelper& FunnelChartType::getInfoHelper()
{
    return StaticFunnelChartTypeInfoHelper();
}

// ____ XPropertySet ____
uno::Reference<beans::XPropertySetInfo> FunnelChartType::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticFunnelChartTypeInfoHelper()));
    return xPropertySetInfo;
}

OUString FunnelChartType::getImplementationName()
{
    return u"com.sun.star.comp.chart.FunnelChartType"_ustr;
}

bool FunnelChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

cpo::uno::Sequence<OUString> FunnelChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_FUNNEL, u"com.sun.star.chart2.ChartType"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_FunnelChartType_get_implementation(css::uno::XComponentContext* /*context*/,
                                                           cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new ::chart::FunnelChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
