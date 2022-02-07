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
#pragma once

#include <ChartTypeTemplate.hxx>
#include <OPropertySet.hxx>
#include <cppuhelper/basemutex.hxx>
#include <StackMode.hxx>
#include <comphelper/uno3.hxx>

namespace chart
{

class LineChartTypeTemplate :
        public cppu::BaseMutex,
        public ChartTypeTemplate,
        public ::property::OPropertySet
{
public:
    explicit LineChartTypeTemplate(
        css::uno::Reference< css::uno::XComponentContext > const & xContext,
        const OUString & rServiceName,
        StackMode eStackMode,
        bool bSymbols,
        bool bHasLines = true,
        sal_Int32 nDim = 2 );
    virtual ~LineChartTypeTemplate() override;

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ OPropertySet ____
    virtual void GetDefaultValue( sal_Int32 nHandle, css::uno::Any& rAny ) const override;
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

    // ____ XChartTypeTemplate ____
    virtual bool matchesTemplate(
        const rtl::Reference< ::chart::Diagram >& xDiagram,
        bool bAdaptProperties ) override;
    virtual rtl::Reference< ::chart::ChartType >
        getChartTypeForNewSeries( const std::vector<
            rtl::Reference< ::chart::ChartType > >& aFormerlyUsedChartTypes ) override;
    virtual void applyStyle(
        const rtl::Reference< ::chart::DataSeries >& xSeries,
        ::sal_Int32 nChartTypeGroupIndex,
        ::sal_Int32 nSeriesIndex,
        ::sal_Int32 nSeriesCount ) override;

    // ____ ChartTypeTemplate ____
    virtual rtl::Reference< ::chart::ChartType >
                getChartTypeForIndex( sal_Int32 nChartTypeIndex ) override;
    virtual sal_Int32 getDimension() const override;
    virtual StackMode getStackMode( sal_Int32 nChartTypeIndex ) const override;

private:
    StackMode          m_eStackMode;
    bool               m_bHasSymbols;
    bool               m_bHasLines;
    sal_Int32          m_nDim;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
