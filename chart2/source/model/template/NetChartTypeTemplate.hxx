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
#include <StackMode.hxx>

namespace chart
{

class NetChartTypeTemplate : public ChartTypeTemplate
{
public:
    explicit NetChartTypeTemplate(
        css::uno::Reference< css::uno::XComponentContext > const & xContext,
        const OUString & rServiceName,
        StackMode eStackMode,
        bool bSymbols,
        bool bHasLines = true,
        bool bHasFilledArea = false
        );
    virtual ~NetChartTypeTemplate() override;

protected:
    // ____ XChartTypeTemplate ____
    virtual sal_Bool SAL_CALL matchesTemplate(
        const css::uno::Reference< css::chart2::XDiagram >& xDiagram,
        sal_Bool bAdaptProperties ) override;
    virtual css::uno::Reference< css::chart2::XChartType > SAL_CALL
        getChartTypeForNewSeries( const css::uno::Sequence< css::uno::Reference< css::chart2::XChartType > >& aFormerlyUsedChartTypes ) override;
    virtual void SAL_CALL applyStyle(
        const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
        ::sal_Int32 nChartTypeGroupIndex,
        ::sal_Int32 nSeriesIndex,
        ::sal_Int32 nSeriesCount ) override;

    // ____ ChartTypeTemplate ____
    virtual css::uno::Reference< css::chart2::XChartType >
                getChartTypeForIndex( sal_Int32 nChartTypeIndex ) override;
    virtual StackMode getStackMode( sal_Int32 nChartTypeIndex ) const override;

private:
    StackMode  m_eStackMode;
    bool                                 m_bHasSymbols;
    bool                                 m_bHasLines;
    bool                                 m_bHasFilledArea;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
