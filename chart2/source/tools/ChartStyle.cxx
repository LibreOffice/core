/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <ChartStyle.hxx>
#include <com/sun/star/chart2/ChartObjectType.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <vector>

#include <Legend.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{
class XComponentContext;
}
}
}
}

namespace chart2
{
ChartObjectStyle::ChartObjectStyle(css::uno::Reference<css::beans::XPropertySetInfo> xPropSetInfo,
                                   ::cppu::IPropertyArrayHelper& rArrayHelper,
                                   const chart::tPropertyValueMap& rPropertyMap)
    : OPropertySet(m_aMutex)
    , mrArrayHelper(rArrayHelper)
    , mrPropertyMap(rPropertyMap)
    , mxPropSetInfo(xPropSetInfo)
{
}

ChartObjectStyle::~ChartObjectStyle() {}

sal_Bool SAL_CALL ChartObjectStyle::isInUse() { return true; }

sal_Bool SAL_CALL ChartObjectStyle::isUserDefined() { return true; }

OUString SAL_CALL ChartObjectStyle::getParentStyle() { return ""; }

void SAL_CALL ChartObjectStyle::setParentStyle(const OUString&) {}

css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL ChartObjectStyle::getPropertySetInfo()
{
    return mxPropSetInfo;
}

OUString SAL_CALL ChartObjectStyle::getName() { return OUString(); }

void SAL_CALL ChartObjectStyle::setName(const OUString&) {}

// ____ OPropertySet ____
css::uno::Any ChartObjectStyle::GetDefaultValue(sal_Int32 nHandle) const
{
    chart::tPropertyValueMap::const_iterator aFound(mrPropertyMap.find(nHandle));
    if (aFound == mrPropertyMap.end())
        return css::uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper& SAL_CALL ChartObjectStyle::getInfoHelper() { return mrArrayHelper; }

ChartStyle::ChartStyle()
    : m_nNumObjects(css::chart2::ChartObjectType::UNKNOWN)
{
    register_styles();
}

ChartStyle::~ChartStyle() {}

css::uno::Reference<css::beans::XPropertySet>
ChartStyle::getStyleForObject(const sal_Int16 nChartObjectType)
{
    auto itr = m_xChartStyle.find(nChartObjectType);
    if (itr == m_xChartStyle.end())
        throw css::lang::IllegalArgumentException("Unknown Chart Object Style requested", nullptr,
                                                  0);

    return itr->second;
}

void ChartStyle::register_styles()
{
    m_xChartStyle[css::chart2::ChartObjectType::LEGEND] = new ChartObjectStyle(
        *chart::legend::StaticLegendInfo::get(), *chart::legend::StaticLegendInfoHelper::get(),
        *chart::legend::StaticLegendDefaults::get());
}

// _____ XServiceInfo _____
OUString SAL_CALL ChartStyle::getImplementationName()
{
    return OUString("com.sun.star.comp.chart2.ChartStyle");
}

sal_Bool SAL_CALL ChartStyle::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL ChartStyle::getSupportedServiceNames()
{
    return { "com.sun.star.chart2.ChartStyle" };
}
//
// needed by MSC compiler
using impl::ChartObjectStyle_Base;

IMPLEMENT_FORWARD_XINTERFACE2(ChartObjectStyle, ChartObjectStyle_Base, ::property::OPropertySet)

} //  namespace chart2

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart2_ChartStyle_get_implementation(css::uno::XComponentContext*,
                                                       css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new chart2::ChartStyle);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
