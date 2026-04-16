/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <DataInterpreter.hxx>

namespace chart
{
/** DataInterpreter for histogram charts.
 *  Preserves the input column as values-y and attaches derived HistogramDataSequence
 *  sidecars for bin labels and frequencies. */
class HistogramDataInterpreter : public DataInterpreter
{
public:
    explicit HistogramDataInterpreter() = default;

protected:
    virtual InterpretedData interpretDataSource(
        const css::uno::Reference<css::chart2::data::XDataSource>& xSource,
        const css::uno::Sequence<css::beans::PropertyValue>& aArguments,
        const std::vector<rtl::Reference<::chart::DataSeries>>& aSeriesToReUse) override;

    virtual InterpretedData reinterpretDataSeries(const InterpretedData& aInterpretedData) override;

    virtual bool isDataCompatible(const InterpretedData& aInterpretedData) override;
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
