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
uno::Reference<util::XCloneable> SAL_CALL FunnelChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new FunnelChartType(*this));
}

rtl::Reference<ChartType> FunnelChartType::cloneChartType() const
{
    return new FunnelChartType(*this);
}

// ____ XChartType ____
OUString SAL_CALL FunnelChartType::getChartType() { return CHART2_SERVICE_NAME_CHARTTYPE_FUNNEL; }

uno::Sequence<OUString> FunnelChartType::getSupportedPropertyRoles()
{
    return { "FillColor", "BorderColor" };
}

// ____ OPropertySet ____
void FunnelChartType::GetDefaultValue(sal_Int32 nHandle, uno::Any& rAny) const
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

::cppu::IPropertyArrayHelper& SAL_CALL FunnelChartType::getInfoHelper()
{
    return StaticFunnelChartTypeInfoHelper();
}

// ____ XPropertySet ____
uno::Reference<beans::XPropertySetInfo> SAL_CALL FunnelChartType::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticFunnelChartTypeInfoHelper()));
    return xPropertySetInfo;
}

OUString SAL_CALL FunnelChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.FunnelChartType";
}

sal_Bool SAL_CALL FunnelChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL FunnelChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_FUNNEL, "com.sun.star.chart2.ChartType" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_FunnelChartType_get_implementation(css::uno::XComponentContext* /*context*/,
                                                           css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::FunnelChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
