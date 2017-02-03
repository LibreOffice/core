/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "GL3DBarChartTypeTemplate.hxx"
#include "GL3DBarChartType.hxx"

#include <servicenames_charttypes.hxx>
#include <PropertyHelper.hxx>
#include <DiagramHelper.hxx>
#include <unonames.hxx>
#include <macros.hxx>

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace com::sun::star;

namespace chart {

namespace {

enum
{
    PROP_GL3DCHARTTYPE_ROUNDED_EDGE
};

struct DefaultsInitializer
{
    tPropertyValueMap* operator()()
    {
        static tPropertyValueMap aStaticDefaults;

        if (aStaticDefaults.empty())
            addDefaults(aStaticDefaults);

        return &aStaticDefaults;
    }
private:

    static void addDefaults( tPropertyValueMap & rOutMap )
    {
        PropertyHelper::setPropertyValueDefault(rOutMap, PROP_GL3DCHARTTYPE_ROUNDED_EDGE, false);
    }
};

struct Defaults : public rtl::StaticAggregate<tPropertyValueMap, DefaultsInitializer> {};

struct InfoHelperInitializer
{
    cppu::OPropertyArrayHelper* operator()()
    {
        static cppu::OPropertyArrayHelper aHelper(getProperties());
        return &aHelper;
    }

    static uno::Sequence<beans::Property> getProperties()
    {
        uno::Sequence<beans::Property> aRet(1);

        aRet[0] = beans::Property(
            CHART_UNONAME_ROUNDED_EDGE,
            PROP_GL3DCHARTTYPE_ROUNDED_EDGE,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT);

        return aRet;
    }
};

struct InfoHelper : public rtl::StaticAggregate<cppu::OPropertyArrayHelper, InfoHelperInitializer> {};

struct ChartTypeInfoInitializer
{
    uno::Reference<beans::XPropertySetInfo>* operator()()
    {
        static uno::Reference<beans::XPropertySetInfo> xPropertySetInfo;

        if (!xPropertySetInfo.is())
            xPropertySetInfo = cppu::OPropertySetHelper::createPropertySetInfo(*InfoHelper::get());

        return &xPropertySetInfo;
    }
};

struct ChartTypeInfo : public rtl::StaticAggregate<uno::Reference<beans::XPropertySetInfo>, ChartTypeInfoInitializer> {};

}

GL3DBarChartTypeTemplate::GL3DBarChartTypeTemplate(
    const uno::Reference<uno::XComponentContext>& xContext, const OUString& rServiceName ) :
    property::OPropertySet(m_aMutex),
    ChartTypeTemplate(xContext, rServiceName) {}

GL3DBarChartTypeTemplate::~GL3DBarChartTypeTemplate() {}

uno::Reference<chart2::XChartType> GL3DBarChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    uno::Reference<chart2::XChartType> xResult;

    try
    {
        uno::Reference<lang::XMultiServiceFactory> xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW);

        xResult.set(xFact->createInstance(CHART2_SERVICE_NAME_CHARTTYPE_GL3DBAR), uno::UNO_QUERY_THROW);

        uno::Reference<beans::XPropertySet> xCTProp(xResult, uno::UNO_QUERY);
        if (xCTProp.is())
        {
            xCTProp->setPropertyValue(
                CHART_UNONAME_ROUNDED_EDGE, getFastPropertyValue(PROP_GL3DCHARTTYPE_ROUNDED_EDGE));
        }
    }
    catch (const uno::Exception & ex)
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

sal_Bool SAL_CALL GL3DBarChartTypeTemplate::matchesTemplate(
    const css::uno::Reference<css::chart2::XDiagram>& xDiagram,
    sal_Bool bAdaptProperties )
{
    bool bResult = ChartTypeTemplate::matchesTemplate(xDiagram, bAdaptProperties);

    if (bResult && bAdaptProperties)
    {
        uno::Reference<chart2::XChartType> xChartType = DiagramHelper::getChartTypeByIndex(xDiagram, 0);
        uno::Reference<beans::XPropertySet> xPS(xChartType, uno::UNO_QUERY);
        if (xPS.is())
        {
            setFastPropertyValue_NoBroadcast(
                PROP_GL3DCHARTTYPE_ROUNDED_EDGE, xPS->getPropertyValue(CHART_UNONAME_ROUNDED_EDGE));
        }
    }

    return bResult;
}

uno::Reference<chart2::XChartType>
GL3DBarChartTypeTemplate::getChartTypeForNewSeries( const uno::Sequence<uno::Reference<chart2::XChartType> >& /*xOldChartTypes*/ )
{
    uno::Reference<chart2::XChartType> xResult;

    try
    {
        rtl::Reference<GL3DBarChartType> chart(
            new GL3DBarChartType(GetComponentContext()));
        bool bVal = false;
        getFastPropertyValue(PROP_GL3DCHARTTYPE_ROUNDED_EDGE) >>= bVal;
        chart->setPropertyValue(CHART_UNONAME_ROUNDED_EDGE, uno::Any(bVal));
        xResult = chart.get();
    }
    catch (const uno::Exception & ex)
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

sal_Bool GL3DBarChartTypeTemplate::supportsCategories()
{
    return false;
}

css::uno::Any GL3DBarChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
{
    const tPropertyValueMap& rDefaults = *Defaults::get();
    tPropertyValueMap::const_iterator it = rDefaults.find(nHandle);
    return it == rDefaults.end() ? uno::Any() : it->second;
}

cppu::IPropertyArrayHelper& GL3DBarChartTypeTemplate::getInfoHelper()
{
    return *InfoHelper::get();
}

css::uno::Reference<css::beans::XPropertySetInfo> GL3DBarChartTypeTemplate::getPropertySetInfo()
{
    return *ChartTypeInfo::get();
}

IMPLEMENT_FORWARD_XINTERFACE2(GL3DBarChartTypeTemplate, ChartTypeTemplate, OPropertySet)

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
