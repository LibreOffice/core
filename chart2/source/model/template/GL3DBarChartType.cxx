/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "GL3DBarChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <PropertyHelper.hxx>
#include <unonames.hxx>
#include <cppuhelper/supportsservice.hxx>

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

GL3DBarChartType::GL3DBarChartType( const uno::Reference<uno::XComponentContext>& xContext ) :
    ChartType(xContext)
{
}

GL3DBarChartType::GL3DBarChartType( const GL3DBarChartType& rOther ) :
    ChartType(rOther)
{
}

GL3DBarChartType::~GL3DBarChartType() {}

OUString SAL_CALL GL3DBarChartType::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString GL3DBarChartType::getImplementationName_Static()
{
    return OUString("com.sun.star.comp.chart.GL3DBarChartType");
}

sal_Bool SAL_CALL GL3DBarChartType::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL GL3DBarChartType::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence<OUString> GL3DBarChartType::getSupportedServiceNames_Static()
{
    uno::Sequence<OUString> aServices(3);
    aServices[0] = CHART2_SERVICE_NAME_CHARTTYPE_GL3DBAR;
    aServices[1] = "com.sun.star.chart2.ChartType";
    aServices[2] = "com.sun.star.beans.PropertySet";
    return aServices;
}

OUString SAL_CALL GL3DBarChartType::getChartType()
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return OUString(CHART2_SERVICE_NAME_CHARTTYPE_GL3DBAR);
}

uno::Sequence<OUString> GL3DBarChartType::getSupportedPropertyRoles()
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aPropRoles { "FillColor" };

    return aPropRoles;
}

com::sun::star::uno::Reference<com::sun::star::util::XCloneable>
GL3DBarChartType::createClone()
    throw (com::sun::star::uno::RuntimeException, std::exception)
{
    return uno::Reference<util::XCloneable>(new GL3DBarChartType(*this));
}

css::uno::Any GL3DBarChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw (css::beans::UnknownPropertyException)
{
    const tPropertyValueMap& rDefaults = *Defaults::get();
    tPropertyValueMap::const_iterator it = rDefaults.find(nHandle);
    return it == rDefaults.end() ? uno::Any() : it->second;
}

cppu::IPropertyArrayHelper& GL3DBarChartType::getInfoHelper()
{
    return *InfoHelper::get();
}

css::uno::Reference<css::beans::XPropertySetInfo> GL3DBarChartType::getPropertySetInfo()
    throw (css::uno::RuntimeException, std::exception)
{
    return *ChartTypeInfo::get();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart_GL3DBarChartType_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::GL3DBarChartType(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
