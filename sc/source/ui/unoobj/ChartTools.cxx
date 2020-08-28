/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <ChartTools.hxx>
#include <docsh.hxx>
#include <drwlayer.hxx>

#include <com/sun/star/chart2/data/XPivotTableDataProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>

using namespace css;

namespace sc::tools {

namespace {

uno::Reference<chart2::data::XPivotTableDataProvider>
getPivotTableDataProvider(const SdrOle2Obj* pOleObject)
{
    uno::Reference<chart2::data::XPivotTableDataProvider> xPivotTableDataProvider;

    const uno::Reference<embed::XEmbeddedObject>& xObject = pOleObject->GetObjRef();
    if (xObject.is())
    {
        uno::Reference<chart2::XChartDocument> xChartDoc(xObject->getComponent(), uno::UNO_QUERY);
        if (xChartDoc.is())
        {
            xPivotTableDataProvider.set(uno::Reference<chart2::data::XPivotTableDataProvider>(
                                            xChartDoc->getDataProvider(), uno::UNO_QUERY));
        }
    }
    return xPivotTableDataProvider;
}

OUString getAssociatedPivotTableName(const SdrOle2Obj* pOleObject)
{
    OUString aPivotTableName;
    uno::Reference<chart2::data::XPivotTableDataProvider> xPivotTableDataProvider;
    xPivotTableDataProvider.set(getPivotTableDataProvider(pOleObject));
    if (xPivotTableDataProvider.is())
        aPivotTableName = xPivotTableDataProvider->getPivotTableName();
    return aPivotTableName;
}

} // end anonymous namespace

ChartIterator::ChartIterator(ScDocShell* pDocShell, SCTAB nTab, ChartSourceType eChartSourceType)
    : m_eChartSourceType(eChartSourceType)
{
    if (!pDocShell)
        return;
    ScDocument& rDoc = pDocShell->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    if (!pDrawLayer)
        return;
    SdrPage* pPage = pDrawLayer->GetPage(sal_uInt16(nTab));
    if (!pPage)
        return;
    m_pIterator.reset(new SdrObjListIter(pPage, SdrIterMode::DeepNoGroups));
}

SdrOle2Obj* ChartIterator::next()
{
    if (!m_pIterator)
        return nullptr;

    SdrObject* pObject = m_pIterator->Next();
    while (pObject)
    {
        if (pObject->GetObjIdentifier() == OBJ_OLE2 && ScDocument::IsChart(pObject))
        {
            SdrOle2Obj* pOleObject = static_cast<SdrOle2Obj*>(pObject);

            uno::Reference<chart2::data::XPivotTableDataProvider> xPivotTableDataProvider;
            xPivotTableDataProvider.set(getPivotTableDataProvider(pOleObject));

            if (xPivotTableDataProvider.is() && m_eChartSourceType == ChartSourceType::PIVOT_TABLE)
                return pOleObject;
            else if (!xPivotTableDataProvider.is() && m_eChartSourceType == ChartSourceType::CELL_RANGE)
                return pOleObject;
        }
        pObject = m_pIterator->Next();
    }
    return nullptr;
}

SdrOle2Obj* findChartsByName(ScDocShell* pDocShell, SCTAB nTab, OUString const & rName, ChartSourceType eChartSourceType)
{
    if (!pDocShell)
        return nullptr;

    ChartIterator aIterator(pDocShell, nTab, eChartSourceType);

    SdrOle2Obj* pObject = aIterator.next();
    while (pObject)
    {
        uno::Reference<embed::XEmbeddedObject> xObject = pObject->GetObjRef();
        if (xObject.is())
        {
            OUString aObjectName = pDocShell->GetEmbeddedObjectContainer().GetEmbeddedObjectName(xObject);
            if (aObjectName == rName)
                return pObject;
        }
        pObject = aIterator.next();
    }
    return nullptr;
}

SdrOle2Obj* getChartByIndex(ScDocShell* pDocShell, SCTAB nTab, long nIndex, ChartSourceType eChartSourceType)
{
    if (!pDocShell)
        return nullptr;

    ChartIterator aIterator(pDocShell, nTab, eChartSourceType);

    SdrOle2Obj* pObject = aIterator.next();
    long i = 0;
    while (pObject)
    {
        if (i == nIndex)
        {
            return pObject;
        }

        i++;
        pObject = aIterator.next();
    }
    return nullptr;
}

std::vector<SdrOle2Obj*> getAllPivotChartsConnectedTo(OUString const & sPivotTableName, ScDocShell* pDocShell)
{
    std::vector<SdrOle2Obj*> aObjects;

    ScDocument& rDocument = pDocShell->GetDocument();
    ScDrawLayer* pModel = rDocument.GetDrawLayer();
    if (!pModel)
        return aObjects;

    sal_uInt16 nPageCount = pModel->GetPageCount();
    for (sal_uInt16 nPageNo = 0; nPageNo < nPageCount; nPageNo++)
    {
        SdrPage* pPage = pModel->GetPage(nPageNo);
        if (!pPage)
            continue;

        sc::tools::ChartIterator aIterator(pDocShell, nPageNo, ChartSourceType::PIVOT_TABLE);
        SdrOle2Obj* pObject = aIterator.next();
        while (pObject)
        {
            if (sPivotTableName == getAssociatedPivotTableName(pObject))
            {
                aObjects.push_back(pObject);
            }
            pObject = aIterator.next();
        }
    }
    return aObjects;
}

} // end sc::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
