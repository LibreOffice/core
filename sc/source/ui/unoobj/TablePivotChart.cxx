/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/chart2/data/XPivotTableDataProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <svx/svdoole2.hxx>
#include <svtools/embedhlp.hxx>
#include <utility>
#include <vcl/svapp.hxx>

#include <miscuno.hxx>
#include <docsh.hxx>

#include <TablePivotChart.hxx>
#include <ChartTools.hxx>

using namespace css;

namespace sc
{

SC_SIMPLE_SERVICE_INFO(TablePivotChart, u"TablePivotChart"_ustr, u"com.sun.star.table.TablePivotChart"_ustr)

TablePivotChart::TablePivotChart(ScDocShell* pDocShell, SCTAB nTab, OUString aName)
    : m_pDocShell(pDocShell)
    , m_nTab(nTab)
    , m_aChartName(std::move(aName))
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
    return m_aChartName;
}

void SAL_CALL TablePivotChart::setName(OUString const & /* aName */)
{
    throw uno::RuntimeException(); // name cannot be changed
}

// XTablePivotChart

OUString SAL_CALL TablePivotChart::getPivotTableName()
{
    SolarMutexGuard aGuard;

    SdrOle2Obj* pObject = sc::tools::findChartsByName(m_pDocShell, m_nTab, m_aChartName, sc::tools::ChartSourceType::PIVOT_TABLE);
    if (!pObject)
        return OUString();

    uno::Reference<embed::XEmbeddedObject> xObject = pObject->GetObjRef();
    if (!xObject.is())
        return OUString();

    uno::Reference<chart2::XChartDocument> xChartDoc(xObject->getComponent(), uno::UNO_QUERY);
    if (!xChartDoc.is())
        return OUString();

    uno::Reference<chart2::data::XPivotTableDataProvider> xPivotTableDataProvider(xChartDoc->getDataProvider(), uno::UNO_QUERY);
    if (!xPivotTableDataProvider.is())
        return OUString();

    return xPivotTableDataProvider->getPivotTableName();
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
