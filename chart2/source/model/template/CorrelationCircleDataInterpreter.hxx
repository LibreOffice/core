/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <DataInterpreter.hxx>

namespace chart
{
/** Reads a flat list of columns as a feature table followed by two dimension
    columns: the last two columns become the dimensions, shared by every series,
    and each column before them becomes the feature column of one series.
 */
class CorrelationCircleDataInterpreter : public DataInterpreter
{
public:
    explicit CorrelationCircleDataInterpreter();
    virtual ~CorrelationCircleDataInterpreter() override;

protected:
    // ____ DataInterpreter ____
    virtual InterpretedData interpretDataSource(
        const css::uno::Reference<css::chart2::data::XDataSource>& xSource,
        const css::uno::Sequence<css::beans::PropertyValue>& aArguments,
        const std::vector<rtl::Reference<::chart::DataSeries>>& aSeriesToReUse) override;
    virtual InterpretedData reinterpretDataSeries(const InterpretedData& aInterpretedData) override;
    virtual bool isDataCompatible(const InterpretedData& aInterpretedData) override;
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
