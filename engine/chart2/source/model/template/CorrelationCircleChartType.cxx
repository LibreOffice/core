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

#include "CorrelationCircleChartType.hxx"
#include <PropertyHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <CartesianCoordinateSystem.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <com/sun/star/chart2/AxisType.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <algorithm>

using namespace ::com::sun::star;

namespace
{
::cppu::OPropertyArrayHelper& StaticCorrelationCircleChartTypeInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []() {
        std::vector<css::beans::Property> aProperties;
        std::sort(aProperties.begin(), aProperties.end(), ::chart::PropertyNameLess());
        return comphelper::containerToSequence(aProperties);
    }();
    return aPropHelper;
};

} // anonymous namespace

namespace chart
{
CorrelationCircleChartType::CorrelationCircleChartType() {}

CorrelationCircleChartType::CorrelationCircleChartType(const CorrelationCircleChartType& rOther)
    : ChartType(rOther)
{
}

CorrelationCircleChartType::~CorrelationCircleChartType() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> CorrelationCircleChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new CorrelationCircleChartType(*this));
}

rtl::Reference<ChartType> CorrelationCircleChartType::cloneChartType() const
{
    return new CorrelationCircleChartType(*this);
}

// ____ XChartType ____
rtl::Reference<::chart::BaseCoordinateSystem>
CorrelationCircleChartType::createCoordinateSystem2(sal_Int32 DimensionCount)
{
    rtl::Reference<CartesianCoordinateSystem> xResult = new CartesianCoordinateSystem(DimensionCount);

    for (sal_Int32 i = 0; i < DimensionCount; ++i)
    {
        rtl::Reference<Axis> xAxis = xResult->getAxisByDimension2(i, MAIN_AXIS_INDEX);
        if (!xAxis.is())
        {
            OSL_FAIL("a created coordinate system should have an axis for each dimension");
            continue;
        }

        chart2::ScaleData aScaleData = xAxis->getScaleData();
        aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
        aScaleData.Scaling = AxisHelper::createLinearScaling();
        aScaleData.AxisType = chart2::AxisType::REALNUMBER;

        xAxis->setScaleData(aScaleData);
    }

    return xResult;
}

OUString CorrelationCircleChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_CORRELATIONCIRCLE;
}

cpo::uno::Sequence<OUString> CorrelationCircleChartType::getSupportedMandatoryRoles()
{
    return { u"label"_ustr, u"values-feature"_ustr, u"values-x"_ustr, u"values-y"_ustr };
}

cpo::uno::Sequence<OUString> CorrelationCircleChartType::getSupportedPropertyRoles()
{
    return { u"FillColor"_ustr, u"BorderColor"_ustr };
}

OUString CorrelationCircleChartType::getRoleOfSequenceForSeriesLabel()
{
    return u"values-feature"_ustr;
}

// ____ OPropertySet ____
void CorrelationCircleChartType::GetDefaultValue(sal_Int32 nHandle, cpo::uno::Any& rAny) const
{
    static ::chart::tPropertyValueMap aStaticDefaults;
    tPropertyValueMap::const_iterator aFound(aStaticDefaults.find(nHandle));
    if (aFound == aStaticDefaults.end())
        rAny.clear();
    else
        rAny = (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper& CorrelationCircleChartType::getInfoHelper()
{
    return StaticCorrelationCircleChartTypeInfoHelper();
}

// ____ XPropertySet ____
uno::Reference<beans::XPropertySetInfo> CorrelationCircleChartType::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(
            StaticCorrelationCircleChartTypeInfoHelper()));
    return xPropertySetInfo;
}

OUString CorrelationCircleChartType::getImplementationName()
{
    return u"com.sun.star.comp.chart.CorrelationCircleChartType"_ustr;
}

bool CorrelationCircleChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

cpo::uno::Sequence<OUString> CorrelationCircleChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_CORRELATIONCIRCLE, u"com.sun.star.chart2.ChartType"_ustr,
             u"com.sun.star.beans.PropertySet"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_CorrelationCircleChartType_get_implementation(
    cpo::uno::XComponentContext* /*context*/, cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new ::chart::CorrelationCircleChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
