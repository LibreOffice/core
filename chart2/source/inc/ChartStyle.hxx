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
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <map>

#include "PropertyHelper.hxx"
#include "OPropertySet.hxx"
#include "MutexContainer.hxx"

namespace chart
{
namespace impl
{
typedef ::cppu::WeakImplHelper<css::style::XStyle> ChartObjectStyle_Base;
}

class ChartObjectStyle : public chart::MutexContainer,
                         public impl::ChartObjectStyle_Base,
                         public property::OPropertySet
{
public:
    ChartObjectStyle(css::uno::Reference<css::beans::XPropertySetInfo> xPropertySetInfo,
                     ::cppu::IPropertyArrayHelper& rArrayHelper,
                     const chart::tPropertyValueMap& rPropertyMap);
    explicit ChartObjectStyle( const ChartObjectStyle & rOther );
    virtual ~ChartObjectStyle();

    /// merge XInterface implementations
    DECLARE_XINTERFACE()

    virtual sal_Bool SAL_CALL isUserDefined() override;
    virtual sal_Bool SAL_CALL isInUse() override;

    virtual OUString SAL_CALL getParentStyle() override;
    virtual void SAL_CALL setParentStyle(const OUString&) override;

    // ____ OPropertySet ____
    virtual css::uno::Any GetDefaultValue(sal_Int32 nHandle) const override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo() override;

    virtual OUString SAL_CALL getName();

    virtual void SAL_CALL setName(const OUString&);

private:
    ::cppu::IPropertyArrayHelper& mrArrayHelper;
    const chart::tPropertyValueMap& mrPropertyMap;
    css::uno::Reference<css::beans::XPropertySetInfo> mxPropSetInfo;
};

class ChartStyle : public cppu::WeakImplHelper<css::chart2::XChartStyle, css::lang::XServiceInfo,
                                               css::style::XStyle>
{
public:
    explicit ChartStyle();
    explicit ChartStyle( const ChartStyle & rOther );
    virtual ~ChartStyle();
    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // _____ XChartStyle _____
    virtual css::uno::Reference<css::beans::XPropertySet>
        SAL_CALL getStyleForObject(const sal_Int16 nChartObjectType) override;

    virtual void SAL_CALL
    applyStyleToDiagram(const css::uno::Reference<css::chart2::XDiagram>& xDiagram) override;

    virtual void SAL_CALL
    applyStyleToTitle(const css::uno::Reference<css::chart2::XTitle>& xTitle) override;

    virtual void SAL_CALL applyStyleToBackground(
        const css::uno::Reference<css::beans::XPropertySet>& xBackground) override;

    virtual void updateStyleElement(const sal_Int16 nChartObjectType, const css::uno::Sequence<css::beans::PropertyValue>& rProperties) override;

    virtual void updateChartStyle(const css::uno::Reference<css::chart2::XChartDocument>& rxModel) override;

    // XStyle
    virtual sal_Bool SAL_CALL isUserDefined() override;
    virtual sal_Bool SAL_CALL isInUse() override;
    virtual OUString SAL_CALL getParentStyle() override;
    virtual void setParentStyle(const OUString& rParentStyle) override;

    // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& rName) override;

private:
    sal_Int16 m_nNumObjects;
    OUString maName;

    std::map<sal_Int16, css::uno::Reference<css::beans::XPropertySet>> m_xChartStyle;

    void register_styles();

    void applyStyleToAxis(const css::uno::Reference<css::chart2::XAxis>& xAxis);

    void applyStyleToCoordinates(
        const css::uno::Reference<css::chart2::XCoordinateSystemContainer>& xCooSysCont);
};

OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference<css::container::XNameContainer> getChartStyles();

} // namespace chart

// INCLUDED_CHART2_SOURCE_INC_CHARTSTYLE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
