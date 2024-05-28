/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <tools/gen.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <unotools/moduleoptions.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/classids.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/globname.hxx>
#include <svtools/embedhlp.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>

#include <TablePivotChart.hxx>
#include <TablePivotCharts.hxx>
#include <PivotTableDataProvider.hxx>
#include <ChartTools.hxx>

#include <miscuno.hxx>
#include <docsh.hxx>
#include <drwlayer.hxx>

using namespace css;

namespace sc
{

SC_SIMPLE_SERVICE_INFO(TablePivotCharts, u"TablePivotCharts"_ustr, u"com.sun.star.table.TablePivotCharts"_ustr)

TablePivotCharts::TablePivotCharts(ScDocShell* pDocShell, SCTAB nTab)
    : m_pDocShell(pDocShell)
    , m_nTab(nTab)
{
    m_pDocShell->GetDocument().AddUnoObject(*this);
}

TablePivotCharts::~TablePivotCharts()
{
    SolarMutexGuard aGuard;

    if (m_pDocShell)
        m_pDocShell->GetDocument().RemoveUnoObject(*this);
}

void TablePivotCharts::Notify(SfxBroadcaster& /*rBroadcaster*/, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
        m_pDocShell = nullptr;
}

// XTablePivotCharts
void SAL_CALL TablePivotCharts::addNewByName(OUString const & rName,
                                             const awt::Rectangle& aRect,
                                             OUString const & rDataPilotName)
{
    SolarMutexGuard aGuard;

    if (!m_pDocShell)
        return;

    ScDocument& rDoc = m_pDocShell->GetDocument();
    ScDrawLayer* pModel = m_pDocShell->MakeDrawLayer();
    SdrPage* pPage = pModel->GetPage(sal_uInt16(m_nTab));
    if (!pPage)
        return;

    //  chart can't be inserted if any ole object with that name exists on any table
    //  (empty string: generate valid name)

    OUString aName = rName;
    SCTAB nDummy;
    if (!aName.isEmpty() && pModel->GetNamedObject(aName, SdrObjKind::OLE2, nDummy))
    {
        //  object exists - only RuntimeException is specified
        throw uno::RuntimeException();
    }

    uno::Reference<embed::XEmbeddedObject> xObject;

    if (SvtModuleOptions().IsChart())
        xObject = m_pDocShell->GetEmbeddedObjectContainer().CreateEmbeddedObject(SvGlobalName(SO3_SCH_CLASSID).GetByteSequence(), aName);

    if (!xObject.is())
            return;

    Point aRectPos(aRect.X, aRect.Y);
    bool bLayoutRTL = rDoc.IsLayoutRTL(m_nTab);
    if ((aRectPos.X() < 0 && !bLayoutRTL) || (aRectPos.X() > 0 && bLayoutRTL))
        aRectPos.setX( 0 );

    if (aRectPos.Y() < 0)
         aRectPos.setY( 0 );

    Size aRectSize(aRect.Width, aRect.Height);
    if (aRectSize.Width() <= 0)
        aRectSize.setWidth( 5000 ); // default size

    if (aRectSize.Height() <= 0)
        aRectSize.setHeight( 5000 );

    ::tools::Rectangle aInsRect(aRectPos, aRectSize);

    sal_Int64 nAspect(embed::Aspects::MSOLE_CONTENT);
    MapUnit aMapUnit(VCLUnoHelper::UnoEmbed2VCLMapUnit(xObject->getMapUnit(nAspect)));
    Size aSize(aInsRect.GetSize());
    aSize = OutputDevice::LogicToLogic(aSize, MapMode(MapUnit::Map100thMM), MapMode(aMapUnit));
    awt::Size aAwtSize;
    aAwtSize.Width = aSize.Width();
    aAwtSize.Height = aSize.Height();

    rtl::Reference<sc::PivotTableDataProvider> pPivotTableDataProvider(new sc::PivotTableDataProvider(rDoc));
    pPivotTableDataProvider->setPivotTableName(rDataPilotName);

    uno::Reference<chart2::data::XDataProvider> xDataProvider(pPivotTableDataProvider);

    uno::Reference<chart2::data::XDataReceiver> xReceiver;

    if (xObject.is())
        xReceiver.set(xObject->getComponent(), uno::UNO_QUERY);

    if (xReceiver.is())
    {
        xReceiver->attachDataProvider(xDataProvider);

        uno::Reference<util::XNumberFormatsSupplier> xNumberFormatsSupplier(cppu::getXWeak(m_pDocShell->GetModel()), uno::UNO_QUERY);
        xReceiver->attachNumberFormatsSupplier(xNumberFormatsSupplier);

        uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
                    { "CellRangeRepresentation", uno::Any(rDataPilotName) },
                    { "HasCategories", uno::Any(true) },
                    { "DataRowSource", uno::Any(chart::ChartDataRowSource_COLUMNS) }
                }));
        xReceiver->setArguments(aArgs);
    }

    rtl::Reference<SdrOle2Obj> pObject = new SdrOle2Obj(
            *pModel,
            svt::EmbeddedObjectRef(xObject, embed::Aspects::MSOLE_CONTENT),
            aName,
            aInsRect);

    if (xObject.is())
        xObject->setVisualAreaSize(nAspect, aAwtSize);

    pPage->InsertObject(pObject.get());
    pModel->AddUndo(std::make_unique<SdrUndoInsertObj>(*pObject));
}

void SAL_CALL TablePivotCharts::removeByName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    SdrOle2Obj* pObject = sc::tools::findChartsByName(m_pDocShell, m_nTab, rName, sc::tools::ChartSourceType::PIVOT_TABLE);
    if (pObject)
    {
        ScDocument& rDoc = m_pDocShell->GetDocument();
        ScDrawLayer* pModel = rDoc.GetDrawLayer();
        SdrPage* pPage = pModel->GetPage(sal_uInt16(m_nTab));
        pModel->AddUndo(std::make_unique<SdrUndoDelObj>(*pObject));
        pPage->RemoveObject(pObject->GetOrdNum());
    }
}

// XIndexAccess
sal_Int32 SAL_CALL TablePivotCharts::getCount()
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = 0;

    if (!m_pDocShell)
        return nCount;

    sc::tools::ChartIterator aIterator(m_pDocShell, m_nTab, sc::tools::ChartSourceType::PIVOT_TABLE);

    SdrOle2Obj* pOleObject = aIterator.next();
    while (pOleObject)
    {
        if (pOleObject->GetObjRef().is())
            nCount++;
        pOleObject = aIterator.next();
    }
    return nCount;
}

uno::Any SAL_CALL TablePivotCharts::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    SdrOle2Obj* pObject = sc::tools::getChartByIndex(m_pDocShell, m_nTab, nIndex,
                                                     sc::tools::ChartSourceType::PIVOT_TABLE);
    if (!pObject)
        throw lang::IndexOutOfBoundsException();

    OUString aName;
    uno::Reference<embed::XEmbeddedObject> xObject = pObject->GetObjRef();
    if (xObject.is())
        aName = m_pDocShell->GetEmbeddedObjectContainer().GetEmbeddedObjectName(xObject);

    if (aName.isEmpty())
        throw lang::IndexOutOfBoundsException();

    uno::Reference<table::XTablePivotChart> xChart(new TablePivotChart(m_pDocShell, m_nTab, aName));
    if (!xChart.is())
        throw lang::IndexOutOfBoundsException();

    return uno::Any(xChart);
}

uno::Type SAL_CALL TablePivotCharts::getElementType()
{
    return cppu::UnoType<table::XTablePivotChart>::get();
}

sal_Bool SAL_CALL TablePivotCharts::hasElements()
{
    SolarMutexGuard aGuard;
    return getCount() != 0;
}

uno::Any SAL_CALL TablePivotCharts::getByName(OUString const & rName)
{
    SolarMutexGuard aGuard;

    if (!sc::tools::findChartsByName(m_pDocShell, m_nTab, rName, sc::tools::ChartSourceType::PIVOT_TABLE))
        throw container::NoSuchElementException();

    uno::Reference<table::XTablePivotChart> xChart(new TablePivotChart(m_pDocShell, m_nTab, rName));
    if (!xChart.is())
        throw container::NoSuchElementException();

    return uno::Any(xChart);
}

uno::Sequence<OUString> SAL_CALL TablePivotCharts::getElementNames()
{
    SolarMutexGuard aGuard;

    std::vector<OUString> aElements;
    sc::tools::ChartIterator aIterator(m_pDocShell, m_nTab, sc::tools::ChartSourceType::PIVOT_TABLE);

    SdrOle2Obj* pOleObject = aIterator.next();
    while (pOleObject)
    {
        uno::Reference<embed::XEmbeddedObject> xObject = pOleObject->GetObjRef();
        if (xObject.is())
        {
            OUString aName = m_pDocShell->GetEmbeddedObjectContainer().GetEmbeddedObjectName(xObject);
            aElements.push_back(aName);
        }
        pOleObject = aIterator.next();
    }
    return comphelper::containerToSequence(aElements);
}

sal_Bool SAL_CALL TablePivotCharts::hasByName(OUString const & rName)
{
    SolarMutexGuard aGuard;

    return sc::tools::findChartsByName(m_pDocShell, m_nTab, rName, sc::tools::ChartSourceType::PIVOT_TABLE) != nullptr;
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
