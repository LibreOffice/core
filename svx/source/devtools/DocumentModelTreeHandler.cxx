/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>
#include <svx/devtools/DocumentModelTreeHandler.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTableCharts.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

using namespace css;

namespace
{
void lclAppendToParent(std::unique_ptr<weld::TreeView>& rTree, weld::TreeIter const& rParent,
                       OUString const& rString, bool bChildrenOnDemand = false)
{
    rTree->insert(&rParent, -1, &rString, nullptr, nullptr, nullptr, bChildrenOnDemand, nullptr);
}

void lclAppendToParentWithIter(std::unique_ptr<weld::TreeView>& rTree,
                               weld::TreeIter const& rParent, weld::TreeIter& rCurrent,
                               OUString const& rString, bool bChildrenOnDemand = false)
{
    rTree->insert(&rParent, -1, &rString, nullptr, nullptr, nullptr, bChildrenOnDemand, &rCurrent);
}

void lclAppend(std::unique_ptr<weld::TreeView>& rTree, OUString const& rString)
{
    rTree->insert(nullptr, -1, &rString, nullptr, nullptr, nullptr, true, nullptr);
}

OUString lclGetNamed(uno::Reference<uno::XInterface> const& xObject)
{
    uno::Reference<container::XNamed> xNamed(xObject, uno::UNO_QUERY);
    if (!xNamed.is())
        return OUString();
    return xNamed->getName();
}

} // end anonymous namespace

uno::Reference<uno::XInterface> DocumentModelTreeHandler::getObjectByID(OUString const& rID)
{
    uno::Reference<uno::XInterface> xObject;
    if (maUnoObjectMap.find(rID) == maUnoObjectMap.end())
        return xObject;
    xObject = maUnoObjectMap.at(rID);
    return xObject;
}

void DocumentModelTreeHandler::insertDocModelToParent(
    weld::TreeIter const& rParent, OUString const& rName,
    uno::Reference<uno::XInterface> const& rInterface)
{
    maUnoObjectMap.emplace(rName, rInterface);
    lclAppendToParent(mpDocumentModelTree, rParent, rName);
}

void DocumentModelTreeHandler::clearChildren(weld::TreeIter const& rParent)
{
    bool bChild = false;
    do
    {
        bChild = mpDocumentModelTree->iter_has_child(rParent);
        if (bChild)
        {
            std::unique_ptr<weld::TreeIter> pChild = mpDocumentModelTree->make_iterator(&rParent);
            bChild = mpDocumentModelTree->iter_children(*pChild);
            if (bChild)
            {
                mpDocumentModelTree->remove(*pChild);
            }
        }
    } while (bChild);
}

IMPL_LINK(DocumentModelTreeHandler, ExpandingHandler, weld::TreeIter const&, rParent, bool)
{
    OUString aText = mpDocumentModelTree->get_text(rParent);
    if (aText == "Paragraphs")
    {
        clearChildren(rParent);
        fillParagraphs(rParent);
    }
    else if (aText == "Shapes")
    {
        uno::Reference<lang::XServiceInfo> xDocumentServiceInfo(mxDocument, uno::UNO_QUERY_THROW);
        if (xDocumentServiceInfo->supportsService("com.sun.star.text.TextDocument"))
        {
            clearChildren(rParent);
            fillShapes(rParent);
        }
    }
    else if (aText == "Tables")
    {
        clearChildren(rParent);
        fillTables(rParent);
    }
    else if (aText == "Frames")
    {
        clearChildren(rParent);
        fillFrames(rParent);
    }
    else if (aText == "Graphic Objects")
    {
        clearChildren(rParent);
        fillGraphicObjects(rParent);
    }
    else if (aText == "Embedded Objects")
    {
        clearChildren(rParent);
        fillOLEObjects(rParent);
    }
    else if (aText == "Styles")
    {
        clearChildren(rParent);
        fillStyleFamilies(rParent);
    }
    else if (aText == "Pages")
    {
        clearChildren(rParent);
        fillPages(rParent);
    }
    else if (aText == "Slides")
    {
        clearChildren(rParent);
        fillSlides(rParent);
    }
    else if (aText == "Master Slides")
    {
        clearChildren(rParent);
        fillMasterSlides(rParent);
    }
    else if (aText == "Sheets")
    {
        clearChildren(rParent);
        fillSheets(rParent);
    }

    return true;
}

void DocumentModelTreeHandler::fillGraphicObjects(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxDocument, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xGraphicObjects = xSupplier->getGraphicObjects();
    const uno::Sequence<OUString> aNames = xGraphicObjects->getElementNames();
    for (auto const& rName : aNames)
    {
        uno::Reference<uno::XInterface> xObject(xGraphicObjects->getByName(rName), uno::UNO_QUERY);
        insertDocModelToParent(rParent, rName, xObject);
    }
}

void DocumentModelTreeHandler::fillOLEObjects(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxDocument, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xOleObjects = xSupplier->getEmbeddedObjects();
    const uno::Sequence<OUString> aNames = xOleObjects->getElementNames();
    for (auto const& rName : aNames)
    {
        uno::Reference<uno::XInterface> xObject(xOleObjects->getByName(rName), uno::UNO_QUERY);
        insertDocModelToParent(rParent, rName, xObject);
    }
}

void DocumentModelTreeHandler::fillStyleFamilies(weld::TreeIter const& rParent)
{
    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(mxDocument, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xStyleFamilies = xSupplier->getStyleFamilies();
    const uno::Sequence<OUString> aNames = xStyleFamilies->getElementNames();
    for (auto const& rFamilyName : aNames)
    {
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(rFamilyName),
                                                            uno::UNO_QUERY);

        std::unique_ptr<weld::TreeIter> pCurrentStyleFamily = mpDocumentModelTree->make_iterator();
        lclAppendToParentWithIter(mpDocumentModelTree, rParent, *pCurrentStyleFamily, rFamilyName);
        maUnoObjectMap.emplace(rFamilyName, xStyleFamily);

        const uno::Sequence<OUString> aStyleNames = xStyleFamily->getElementNames();
        for (auto const& rStyleName : aStyleNames)
        {
            uno::Reference<uno::XInterface> xStyle(xStyleFamily->getByName(rStyleName),
                                                   uno::UNO_QUERY);
            insertDocModelToParent(*pCurrentStyleFamily, rStyleName, xStyle);
        }
    }
}

void DocumentModelTreeHandler::fillFrames(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextFramesSupplier> xSupplier(mxDocument, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xFrames = xSupplier->getTextFrames();
    const uno::Sequence<OUString> aNames = xFrames->getElementNames();
    for (auto const& rName : aNames)
    {
        uno::Reference<uno::XInterface> xObject(xFrames->getByName(rName), uno::UNO_QUERY);
        insertDocModelToParent(rParent, rName, xObject);
    }
}

void DocumentModelTreeHandler::fillTables(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxDocument, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
    const uno::Sequence<OUString> aNames = xTables->getElementNames();
    for (auto const& rName : aNames)
    {
        uno::Reference<uno::XInterface> xObject(xTables->getByName(rName), uno::UNO_QUERY);
        insertDocModelToParent(rParent, rName, xObject);
    }
}

void DocumentModelTreeHandler::fillSheets(weld::TreeIter const& rParent)
{
    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxDocument, uno::UNO_QUERY);
    if (!xSheetDoc.is())
        return;
    uno::Reference<sheet::XSpreadsheets> xSheets = xSheetDoc->getSheets();
    uno::Reference<container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY);
    for (sal_Int32 i = 0; i < xIndex->getCount(); ++i)
    {
        uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(i), uno::UNO_QUERY);

        OUString aSlideString = lclGetNamed(xSheet);
        if (aSlideString.isEmpty())
            aSlideString = "Sheet " + OUString::number(i + 1);

        std::unique_ptr<weld::TreeIter> pCurrentSheet = mpDocumentModelTree->make_iterator();
        lclAppendToParentWithIter(mpDocumentModelTree, rParent, *pCurrentSheet, aSlideString);
        maUnoObjectMap.emplace(aSlideString, xSheet);

        {
            uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xSheet, uno::UNO_QUERY);
            uno::Reference<container::XIndexAccess> xDraws = xDrawPageSupplier->getDrawPage();

            std::unique_ptr<weld::TreeIter> pCurrentShapes = mpDocumentModelTree->make_iterator();

            lclAppendToParentWithIter(mpDocumentModelTree, *pCurrentSheet, *pCurrentShapes,
                                      "Shapes");
            maUnoObjectMap.emplace("Shapes", xDraws);

            for (sal_Int32 nIndexShapes = 0; nIndexShapes < xDraws->getCount(); ++nIndexShapes)
            {
                uno::Reference<uno::XInterface> xShape(xDraws->getByIndex(nIndexShapes),
                                                       uno::UNO_QUERY);
                OUString aShapeName = lclGetNamed(xShape);
                if (aShapeName.isEmpty())
                    aShapeName = "Shape " + OUString::number(nIndexShapes + 1);

                insertDocModelToParent(*pCurrentShapes, aShapeName, xShape);
            }
        }

        {
            uno::Reference<table::XTableChartsSupplier> xSupplier(xSheet, uno::UNO_QUERY);
            uno::Reference<table::XTableCharts> xCharts = xSupplier->getCharts();
            std::unique_ptr<weld::TreeIter> pCurrentCharts = mpDocumentModelTree->make_iterator();
            lclAppendToParentWithIter(mpDocumentModelTree, *pCurrentSheet, *pCurrentCharts,
                                      "Charts");
            maUnoObjectMap.emplace("Charts", xCharts);

            const uno::Sequence<OUString> aNames = xCharts->getElementNames();
            for (auto const& rName : aNames)
            {
                uno::Reference<uno::XInterface> xChart(xCharts->getByName(rName), uno::UNO_QUERY);
                insertDocModelToParent(*pCurrentCharts, rName, xChart);
            }
        }

        {
            uno::Reference<sheet::XDataPilotTablesSupplier> xSupplier(xSheet, uno::UNO_QUERY);
            uno::Reference<sheet::XDataPilotTables> xPivotTables = xSupplier->getDataPilotTables();
            std::unique_ptr<weld::TreeIter> pCurrentPivotTables
                = mpDocumentModelTree->make_iterator();
            lclAppendToParentWithIter(mpDocumentModelTree, *pCurrentSheet, *pCurrentPivotTables,
                                      "Pivot Tables");
            maUnoObjectMap.emplace("Pivot Tables", xPivotTables);

            const uno::Sequence<OUString> aNames = xPivotTables->getElementNames();
            for (auto const& rName : aNames)
            {
                uno::Reference<uno::XInterface> xPivotTable(xPivotTables->getByName(rName),
                                                            uno::UNO_QUERY);
                insertDocModelToParent(*pCurrentPivotTables, rName, xPivotTable);
            }
        }
    }
}

void DocumentModelTreeHandler::fillPages(weld::TreeIter const& rParent)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxDocument, uno::UNO_QUERY);
    if (!xDrawPagesSupplier.is())
        return;
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    for (sal_Int32 i = 0; i < xDrawPages->getCount(); ++i)
    {
        uno::Reference<drawing::XDrawPage> xPage(xDrawPages->getByIndex(i), uno::UNO_QUERY);
        if (!xPage.is())
            continue;

        OUString aPageString = lclGetNamed(xPage);
        if (aPageString.isEmpty())
            aPageString = "Page " + OUString::number(i + 1);

        std::unique_ptr<weld::TreeIter> pCurrentPage = mpDocumentModelTree->make_iterator();
        lclAppendToParentWithIter(mpDocumentModelTree, rParent, *pCurrentPage, aPageString);
        maUnoObjectMap.emplace(aPageString, xPage);

        for (sal_Int32 nPageIndex = 0; nPageIndex < xPage->getCount(); ++nPageIndex)
        {
            uno::Reference<uno::XInterface> xShape(xPage->getByIndex(nPageIndex), uno::UNO_QUERY);

            OUString aShapeName = lclGetNamed(xShape);
            if (aShapeName.isEmpty())
                aShapeName = "Shape " + OUString::number(nPageIndex + 1);

            insertDocModelToParent(*pCurrentPage, aShapeName, xShape);
        }
    }
}

void DocumentModelTreeHandler::fillSlides(weld::TreeIter const& rParent)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxDocument, uno::UNO_QUERY);
    if (!xDrawPagesSupplier.is())
        return;
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    for (sal_Int32 i = 0; i < xDrawPages->getCount(); ++i)
    {
        uno::Reference<drawing::XDrawPage> xPage(xDrawPages->getByIndex(i), uno::UNO_QUERY);
        if (!xPage.is())
            continue;

        OUString aSlideName = lclGetNamed(xPage);
        if (aSlideName.isEmpty())
            aSlideName = "Slide " + OUString::number(i + 1);

        std::unique_ptr<weld::TreeIter> pCurrentPage = mpDocumentModelTree->make_iterator();
        lclAppendToParentWithIter(mpDocumentModelTree, rParent, *pCurrentPage, aSlideName);
        maUnoObjectMap.emplace(aSlideName, xPage);

        for (sal_Int32 nPageIndex = 0; nPageIndex < xPage->getCount(); ++nPageIndex)
        {
            uno::Reference<uno::XInterface> xShape(xPage->getByIndex(nPageIndex), uno::UNO_QUERY);

            OUString aShapeName = lclGetNamed(xShape);
            if (aShapeName.isEmpty())
                aShapeName = "Shape " + OUString::number(nPageIndex + 1);

            insertDocModelToParent(*pCurrentPage, aShapeName, xShape);
        }
    }
}

void DocumentModelTreeHandler::fillMasterSlides(weld::TreeIter const& rParent)
{
    uno::Reference<drawing::XMasterPagesSupplier> xSupplier(mxDocument, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<drawing::XDrawPages> xDrawPages = xSupplier->getMasterPages();
    for (sal_Int32 i = 0; i < xDrawPages->getCount(); ++i)
    {
        uno::Reference<drawing::XDrawPage> xPage(xDrawPages->getByIndex(i), uno::UNO_QUERY);
        if (!xPage.is())
            continue;

        OUString aSlideName = lclGetNamed(xPage);
        if (aSlideName.isEmpty())
            aSlideName = "Master " + OUString::number(i + 1);

        std::unique_ptr<weld::TreeIter> pCurrentPage = mpDocumentModelTree->make_iterator();
        lclAppendToParentWithIter(mpDocumentModelTree, rParent, *pCurrentPage, aSlideName);
        maUnoObjectMap.emplace(aSlideName, xPage);

        for (sal_Int32 nPageIndex = 0; nPageIndex < xPage->getCount(); ++nPageIndex)
        {
            uno::Reference<container::XNamed> xShape(xPage->getByIndex(nPageIndex), uno::UNO_QUERY);

            OUString aShapeName = xShape->getName();
            if (aShapeName.isEmpty())
                aShapeName = "Shape " + OUString::number(nPageIndex + 1);

            insertDocModelToParent(*pCurrentPage, aShapeName, xShape);
        }
    }
}

void DocumentModelTreeHandler::fillParagraphs(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextDocument> xDocument(mxDocument, uno::UNO_QUERY);
    if (!xDocument.is())
        return;
    uno::Reference<container::XEnumerationAccess> xParagraphEnumAccess(
        xDocument->getText()->getText(), uno::UNO_QUERY);

    if (!xParagraphEnumAccess.is())
        return;

    uno::Reference<container::XEnumeration> xParagraphEnum
        = xParagraphEnumAccess->createEnumeration();

    if (xParagraphEnum.is())
    {
        for (sal_Int32 i = 0; xParagraphEnum->hasMoreElements(); i++)
        {
            uno::Reference<text::XTextContent> const xParagraph(xParagraphEnum->nextElement(),
                                                                uno::UNO_QUERY);
            OUString aString = lclGetNamed(xParagraph);
            if (aString.isEmpty())
                aString = "Paragraph " + OUString::number(i + 1);

            insertDocModelToParent(rParent, aString, xParagraph);
        }
    }
}

void DocumentModelTreeHandler::fillShapes(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextDocument> xDocument(mxDocument, uno::UNO_QUERY);
    if (!xDocument.is())
        return;
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xDocument, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws = xDrawPageSupplier->getDrawPage();
    for (sal_Int32 nIndexShapes = 0; nIndexShapes < xDraws->getCount(); ++nIndexShapes)
    {
        uno::Reference<uno::XInterface> xShape(xDraws->getByIndex(nIndexShapes), uno::UNO_QUERY);
        OUString aShapeName = lclGetNamed(xShape);
        if (aShapeName.isEmpty())
            aShapeName = "Shape " + OUString::number(nIndexShapes + 1);

        insertDocModelToParent(rParent, aShapeName, xShape);
    }
}

void DocumentModelTreeHandler::inspectDocument()
{
    uno::Reference<lang::XServiceInfo> xDocumentServiceInfo(mxDocument, uno::UNO_QUERY_THROW);

    mpDocumentModelTree->append_text("Document");
    maUnoObjectMap.emplace("Document", mxDocument);

    if (xDocumentServiceInfo->supportsService("com.sun.star.sheet.SpreadsheetDocument"))
    {
        lclAppend(mpDocumentModelTree, "Sheets");
        lclAppend(mpDocumentModelTree, "Styles");
    }
    else if (xDocumentServiceInfo->supportsService(
                 "com.sun.star.presentation.PresentationDocument"))
    {
        lclAppend(mpDocumentModelTree, "Slides");
        lclAppend(mpDocumentModelTree, "Styles");
        lclAppend(mpDocumentModelTree, "Master Slides");
    }
    else if (xDocumentServiceInfo->supportsService("com.sun.star.drawing.DrawingDocument"))
    {
        lclAppend(mpDocumentModelTree, "Pages");
        lclAppend(mpDocumentModelTree, "Styles");
    }
    else if (xDocumentServiceInfo->supportsService("com.sun.star.text.TextDocument")
             || xDocumentServiceInfo->supportsService("com.sun.star.text.WebDocument"))
    {
        lclAppend(mpDocumentModelTree, "Paragraphs");
        lclAppend(mpDocumentModelTree, "Shapes");
        lclAppend(mpDocumentModelTree, "Tables");
        lclAppend(mpDocumentModelTree, "Frames");
        lclAppend(mpDocumentModelTree, "Graphic Objects");
        lclAppend(mpDocumentModelTree, "Embedded Objects");
        lclAppend(mpDocumentModelTree, "Styles");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
