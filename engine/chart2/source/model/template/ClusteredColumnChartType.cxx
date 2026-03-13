/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClusteredColumnChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace ::com::sun::star;

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{
enum
{
    PROP_CLUSTEREDCOLUMNCHARTTYPE_OVERLAP_SEQUENCE,
    PROP_CLUSTEREDCOLUMNCHARTTYPE_GAPWIDTH_SEQUENCE
};

void lcl_AddPropertiesToVector(std::vector<Property>& rOutProperties)
{
    rOutProperties.emplace_back("OverlapSequence", PROP_CLUSTEREDCOLUMNCHARTTYPE_OVERLAP_SEQUENCE,
                                cppu::UnoType<Sequence<sal_Int32>>::get(),
                                beans::PropertyAttribute::BOUND
                                    | beans::PropertyAttribute::MAYBEDEFAULT);

    rOutProperties.emplace_back("GapwidthSequence", PROP_CLUSTEREDCOLUMNCHARTTYPE_GAPWIDTH_SEQUENCE,
                                cppu::UnoType<Sequence<sal_Int32>>::get(),
                                beans::PropertyAttribute::BOUND
                                    | beans::PropertyAttribute::MAYBEDEFAULT);
}

::cppu::OPropertyArrayHelper& StaticClusteredColumnChartTypeInfoHelper()
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
ClusteredColumnChartType::ClusteredColumnChartType() {}

ClusteredColumnChartType::ClusteredColumnChartType(const ClusteredColumnChartType& rOther)
    : ChartType(rOther)
{
}

ClusteredColumnChartType::~ClusteredColumnChartType() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> SAL_CALL ClusteredColumnChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new ClusteredColumnChartType(*this));
}

rtl::Reference<ChartType> ClusteredColumnChartType::cloneChartType() const
{
    return new ClusteredColumnChartType(*this);
}

// ____ XChartType ____
OUString SAL_CALL ClusteredColumnChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_CLUSTEREDCOLUMN;
}

uno::Sequence<OUString> ClusteredColumnChartType::getSupportedPropertyRoles()
{
    return { "FillColor", "BorderColor" };
}

// ____ OPropertySet ____
void ClusteredColumnChartType::GetDefaultValue(sal_Int32 nHandle, uno::Any& rAny) const
{
    static const ::chart::tPropertyValueMap aStaticDefaults = []() {
        ::chart::tPropertyValueMap aTmp;
        Sequence<sal_Int32> aSeq{ 0, 0 };
        ::chart::PropertyHelper::setPropertyValueDefault(
            aTmp, PROP_CLUSTEREDCOLUMNCHARTTYPE_OVERLAP_SEQUENCE, aSeq);
        aSeq = { 100, 100 };
        ::chart::PropertyHelper::setPropertyValueDefault(
            aTmp, PROP_CLUSTEREDCOLUMNCHARTTYPE_GAPWIDTH_SEQUENCE, aSeq);
        return aTmp;
    }();
    tPropertyValueMap::const_iterator aFound(aStaticDefaults.find(nHandle));
    if (aFound == aStaticDefaults.end())
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper& SAL_CALL ClusteredColumnChartType::getInfoHelper()
{
    return StaticClusteredColumnChartTypeInfoHelper();
}

// ____ XPropertySet ____
uno::Reference<beans::XPropertySetInfo> SAL_CALL ClusteredColumnChartType::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(
            StaticClusteredColumnChartTypeInfoHelper()));
    return xPropertySetInfo;
}

OUString SAL_CALL ClusteredColumnChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.ClusteredColumnChartType";
}

sal_Bool SAL_CALL ClusteredColumnChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL ClusteredColumnChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_CLUSTEREDCOLUMN, "com.sun.star.chart2.ChartType" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_ClusteredColumnChartType_get_implementation(
    css::uno::XComponentContext* /*context*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::ClusteredColumnChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
