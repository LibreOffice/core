/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <PivotTableDataSource.hxx>

#include <sal/config.h>

#include <miscuno.hxx>

#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>

using namespace css;

namespace sc
{

SC_SIMPLE_SERVICE_INFO(PivotTableDataSource, u"PivotTableDataSource"_ustr, u"com.sun.star.chart2.data.DataSource"_ustr)

PivotTableDataSource::PivotTableDataSource(std::vector<css::uno::Reference<css::chart2::data::XLabeledDataSequence>>&& xLabeledSequence)
    : m_xLabeledSequence(std::move(xLabeledSequence))
{
}

PivotTableDataSource::~PivotTableDataSource()
{
}

void PivotTableDataSource::Notify(SfxBroadcaster& /*rBroadcaster*/, const SfxHint& /*rHint*/)
{
}

uno::Sequence<uno::Reference<chart2::data::XLabeledDataSequence>> SAL_CALL
    PivotTableDataSource::getDataSequences()
{
    SolarMutexGuard aGuard;

    return comphelper::containerToSequence(m_xLabeledSequence);
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
