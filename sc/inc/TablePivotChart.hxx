/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>

#include "types.hxx"

class ScDocShell;

namespace sc
{

typedef cppu::WeakComponentImplHelper<css::table::XTablePivotChart,
                                      css::document::XEmbeddedObjectSupplier,
                                      css::container::XNamed,
                                      css::lang::XServiceInfo>
        TablePivotChart_Base;

class TablePivotChart final : public cppu::BaseMutex,
                        public TablePivotChart_Base,
                        public SfxListener
{
private:
    ScDocShell* m_pDocShell;
    SCTAB m_nTab; // Charts are per sheet
    OUString m_aChartName;

public:
    TablePivotChart(ScDocShell* pDocShell, SCTAB nTab, OUString const & rName);
    virtual ~TablePivotChart() override;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XComponent
    using TablePivotChart_Base::disposing;

    // XEmbeddedObjectSupplier
    virtual css::uno::Reference<css::lang::XComponent> SAL_CALL
        getEmbeddedObject() override;

    // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(OUString const & aName) override;

    // XTablePivotChart
    virtual OUString SAL_CALL getPivotTableName() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
