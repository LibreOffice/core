/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/charthelper.hxx>
#include <svtools/embedhlp.hxx>

#include "miscuno.hxx"
#include "docsh.hxx"

#include "TablePivotChart.hxx"
#include "ChartTools.hxx"

using namespace css;

namespace sc
{

SC_SIMPLE_SERVICE_INFO(TablePivotChart, "TablePivotChart", "com.sun.star.table.TablePivotChart")

TablePivotChart::TablePivotChart(ScDocShell* pDocShell, SCTAB nTab, const OUString& rName)
    : TablePivotChart_Base(m_aMutex)
    , m_pDocShell(pDocShell)
    , m_nTab(nTab)
    , m_aChartName(rName)
{
    if (m_pDocShell)
        m_pDocShell->GetDocument().AddUnoObject(*this);
}

TablePivotChart::~TablePivotChart()
{
    SolarMutexGuard aGuard;

    if (m_pDocShell)
        m_pDocShell->GetDocument().RemoveUnoObject(*this);
}

void TablePivotChart::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
        m_pDocShell = nullptr;
}

// XEmbeddedObjectSupplier

uno::Reference<lang::XComponent> SAL_CALL TablePivotChart::getEmbeddedObject()
{
    SolarMutexGuard aGuard;
    SdrOle2Obj* pObject = sc::tools::findChartsByName(m_pDocShell, m_nTab, m_aChartName, sc::tools::ChartSourceType::PIVOT_TABLE);
    if (pObject && svt::EmbeddedObjectRef::TryRunningState(pObject->GetObjRef()))
        return uno::Reference<lang::XComponent>(pObject->GetObjRef()->getComponent(), uno::UNO_QUERY);
    return nullptr;
}

// XNamed

OUString SAL_CALL TablePivotChart::getName()
{
    SolarMutexGuard aGuard;
    return m_aChartName;
}

void SAL_CALL TablePivotChart::setName(OUString const & /* aName */)
{
    SolarMutexGuard aGuard;
    throw uno::RuntimeException(); // name cannot be changed
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
