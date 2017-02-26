/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_INC_PIVOTCHARTDATASOURCE_HXX
#define INCLUDED_SC_INC_PIVOTCHARTDATASOURCE_HXX

#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <svl/lstner.hxx>
#include <cppuhelper/implbase.hxx>

#include "document.hxx"

#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>

namespace sc
{

typedef cppu::WeakImplHelper<css::chart2::data::XDataSource,
                             css::lang::XServiceInfo>
            PivotChartDataSource_Base;

class PivotChartDataSource : public PivotChartDataSource_Base, public SfxListener
{
public:
    explicit PivotChartDataSource(OUString const & aRangeRepresentation, std::vector<css::uno::Reference<css::chart2::data::XLabeledDataSequence>>& xLabeledSequence);
    virtual ~PivotChartDataSource() override;
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XDataSource
    virtual css::uno::Sequence<css::uno::Reference<css::chart2::data::XLabeledDataSequence>> SAL_CALL
        getDataSequences() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService( const OUString&
            rServiceName) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

private:
    std::vector<css::uno::Reference<css::chart2::data::XLabeledDataSequence>> m_xLabeledSequence;
    OUString m_aRangeRepresentation;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
