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
#ifndef INCLUDED_CHART2_SOURCE_INC_CHARTSTYLE_HXX
#define INCLUDED_CHART2_SOURCE_INC_CHARTSTYLE_HXX

#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/propshlp.hxx>
#include <com/sun/star/chart2/XChartStyle.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <vector>

#include "PropertyHelper.hxx"
#include "OPropertySet.hxx"
#include "MutexContainer.hxx"

namespace chart2
{

class ChartObjectStyle : public chart::MutexContainer, public property::OPropertySet, public css::style::XStyle
{
public:
    ChartObjectStyle(::cppu::IPropertyArrayHelper& rArrayHelper, const chart::tPropertyValueMap& rPropertyMap);
    virtual ~ChartObjectStyle();

    virtual sal_Bool SAL_CALL isUserDefined() override;
    virtual sal_Bool SAL_CALL isInUse() override;

    virtual OUString SAL_CALL getParentStyle() override;
    virtual void SAL_CALL setParentStyle(const OUString&) override;

    // ____ OPropertySet ____
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

private:

    ::cppu::IPropertyArrayHelper& mrArrayHelper;
    const chart::tPropertyValueMap& mrPropertyMap;
};

class ChartStyle : public cppu::WeakImplHelper<
                      css::chart2::XChartStyle
                    , css::lang::XServiceInfo >
{
public:
    explicit ChartStyle();
    virtual ~ChartStyle();
    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // _____ XChartStyle _____
    virtual css::uno::Reference<css::beans::XPropertySet>
        SAL_CALL getStyleForObject(const sal_Int16 nChartObjectType) override;

private:
    sal_Int16 m_nNumObjects;

    std::vector<css::uno::Reference<css::beans::XPropertySet>> m_xChartStyle;
};

} // namespace chart2

// INCLUDED_CHART2_SOURCE_INC_CHARTSTYLE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
