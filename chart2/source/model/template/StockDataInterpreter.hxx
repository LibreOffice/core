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

#include "DataInterpreter.hxx"

#include "StockChartTypeTemplate.hxx"

namespace chart
{

class StockDataInterpreter : public DataInterpreter
{
public:
    explicit StockDataInterpreter(
        StockChartTypeTemplate::StockVariant eVariant );
    virtual ~StockDataInterpreter() override;

protected:
    // ____ XDataInterpreter ____
    virtual css::chart2::InterpretedData SAL_CALL interpretDataSource(
        const css::uno::Reference< css::chart2::data::XDataSource >& xSource,
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments,
        const css::uno::Sequence< css::uno::Reference< css::chart2::XDataSeries > >& aSeriesToReUse ) override;
    virtual sal_Bool SAL_CALL isDataCompatible(
        const css::chart2::InterpretedData& aInterpretedData ) override;
    virtual css::chart2::InterpretedData SAL_CALL reinterpretDataSeries(
        const css::chart2::InterpretedData& aInterpretedData ) override;

private:
    StockChartTypeTemplate::StockVariant m_eStockVariant;

    StockChartTypeTemplate::StockVariant GetStockVariant() const { return m_eStockVariant;}
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
