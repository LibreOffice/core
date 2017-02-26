/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "PivotChartDataSource.hxx"

#include <sal/config.h>

#include "miscuno.hxx"
#include "docsh.hxx"

#include <comphelper/sequence.hxx>

using namespace css;

namespace sc
{

SC_SIMPLE_SERVICE_INFO(PivotChartDataSource, "PivotChartDataSource", "com.sun.star.chart2.data.DataSource")

PivotChartDataSource::PivotChartDataSource(OUString const & aRangeRepresentation,
                                           std::vector<css::uno::Reference<css::chart2::data::XLabeledDataSequence>>& xLabeledSequence)
    : m_xLabeledSequence(xLabeledSequence)
    , m_aRangeRepresentation(aRangeRepresentation)
{
}

PivotChartDataSource::~PivotChartDataSource()
{
}

void PivotChartDataSource::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
}

uno::Sequence<uno::Reference<chart2::data::XLabeledDataSequence>> SAL_CALL
    PivotChartDataSource::getDataSequences()
{
    SolarMutexGuard aGuard;

    return comphelper::containerToSequence(m_xLabeledSequence);
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
