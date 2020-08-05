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

#include "ChartTypeTemplate.hxx"
#include <OPropertySet.hxx>
#include <MutexContainer.hxx>
#include <comphelper/uno3.hxx>

namespace chart
{

class BubbleChartTypeTemplate :
        public MutexContainer,
        public ChartTypeTemplate,
        public ::property::OPropertySet
{
public:
    explicit BubbleChartTypeTemplate(
        css::uno::Reference< css::uno::XComponentContext > const & xContext,
        const OUString & rServiceName );
    virtual ~BubbleChartTypeTemplate() override;

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ OPropertySet ____
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const override;
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

    // ____ XChartTypeTemplate ____
    virtual sal_Bool SAL_CALL supportsCategories() override;
    virtual css::uno::Reference< css::chart2::XChartType > SAL_CALL
        getChartTypeForNewSeries( const css::uno::Sequence< css::uno::Reference< css::chart2::XChartType > >& aFormerlyUsedChartTypes ) override;
    virtual css::uno::Reference< css::chart2::XDataInterpreter > SAL_CALL getDataInterpreter() override;
    virtual void SAL_CALL applyStyle(
        const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
        ::sal_Int32 nChartTypeGroupIndex,
        ::sal_Int32 nSeriesIndex,
        ::sal_Int32 nSeriesCount ) override;

    // ____ ChartTypeTemplate ____
    virtual css::uno::Reference< css::chart2::XChartType >
                getChartTypeForIndex( sal_Int32 nChartTypeIndex ) override;
    virtual sal_Int32 getDimension() const override;
    virtual StackMode getStackMode( sal_Int32 nChartTypeIndex ) const override;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
