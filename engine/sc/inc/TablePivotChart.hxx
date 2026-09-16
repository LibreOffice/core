/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/table/XTablePivotChart.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <svl/lstner.hxx>
#include <comphelper/compbase.hxx>

#include "types.hxx"

class ScDocShell;

namespace sc
{

typedef comphelper::WeakComponentImplHelper<css::table::XTablePivotChart,
                                      css::document::XEmbeddedObjectSupplier,
                                      css::container::XNamed,
                                      css::lang::XServiceInfo>
        TablePivotChart_Base;

class TablePivotChart final : public TablePivotChart_Base,
                        public SfxListener
{
private:
    ScDocShell* m_pDocShell;
    SCTAB m_nTab; // Charts are per sheet
    OUString m_aChartName;

public:
    TablePivotChart(ScDocShell* pDocShell, SCTAB nTab, OUString aName);
    virtual ~TablePivotChart() override;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XComponent
    using TablePivotChart_Base::disposing;

    // XEmbeddedObjectSupplier
    virtual cpo::uno::Reference<css::lang::XComponent>
        getEmbeddedObject() override;

    // XNamed
    virtual OUString getName() override;
    virtual void setName(OUString const & aName) override;

    // XTablePivotChart
    virtual OUString getPivotTableName() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(OUString const & ServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
