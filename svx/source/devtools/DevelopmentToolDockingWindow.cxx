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

#include <svx/devtools/DevelopmentToolDockingWindow.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <comphelper/processfactory.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svx/svxids.hrc>

#include <sfx2/objsh.hxx>

#include <sfx2/viewfrm.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>

#include <com/sun/star/container/XNamed.hpp>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTableCharts.hpp>

#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

using namespace css;

namespace
{
typedef cppu::WeakComponentImplHelper<css::view::XSelectionChangeListener>
    SelectionChangeHandlerInterfaceBase;

class SelectionChangeHandler final : private ::cppu::BaseMutex,
                                     public SelectionChangeHandlerInterfaceBase
{
private:
    css::uno::Reference<css::frame::XController> mxController;
    VclPtr<DevelopmentToolDockingWindow> mpDockingWindow;

public:
    SelectionChangeHandler(const css::uno::Reference<css::frame::XController>& rxController,
                           DevelopmentToolDockingWindow* pDockingWindow)
        : SelectionChangeHandlerInterfaceBase(m_aMutex)
        , mxController(rxController)
        , mpDockingWindow(pDockingWindow)
    {
    }

    ~SelectionChangeHandler() { mpDockingWindow.disposeAndClear(); }

    virtual void SAL_CALL selectionChanged(const css::lang::EventObject& /*rEvent*/) override
    {
        uno::Reference<view::XSelectionSupplier> xSupplier(mxController, uno::UNO_QUERY);
        if (xSupplier.is())
        {
            uno::Any aAny = xSupplier->getSelection();
            auto aRef = aAny.get<uno::Reference<uno::XInterface>>();
            mpDockingWindow->introspect(aRef);
        }
    }
    virtual void SAL_CALL disposing(const css::lang::EventObject& /*rEvent*/) override {}
    virtual void SAL_CALL disposing() override {}

private:
    SelectionChangeHandler(const SelectionChangeHandler&) = delete;
    SelectionChangeHandler& operator=(const SelectionChangeHandler&) = delete;
};

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

DevelopmentToolDockingWindow::DevelopmentToolDockingWindow(SfxBindings* pInputBindings,
                                                           SfxChildWindow* pChildWindow,
                                                           vcl::Window* pParent)
    : SfxDockingWindow(pInputBindings, pChildWindow, pParent, "DevelopmentTool",
                       "svx/ui/developmenttool.ui")
    , mpClassNameLabel(m_xBuilder->weld_label("class_name_value_id"))
    , mpClassListBox(m_xBuilder->weld_tree_view("class_listbox_id"))
    , mpLeftSideTreeView(m_xBuilder->weld_tree_view("leftside_treeview_id"))
{
    mpLeftSideTreeView->connect_changed(LINK(this, DevelopmentToolDockingWindow, LeftSideSelected));
    mpLeftSideTreeView->connect_expanding(
        LINK(this, DevelopmentToolDockingWindow, ModelTreeViewExpanding));

    auto* pViewFrame = pInputBindings->GetDispatcher()->GetFrame();

    uno::Reference<frame::XController> xController = pViewFrame->GetFrame().GetController();

    mxRoot = pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel();

    introspect(mxRoot);
    inspectDocument();

    uno::Reference<view::XSelectionSupplier> xSupplier(xController, uno::UNO_QUERY);
    if (xSupplier.is())
    {
        uno::Reference<view::XSelectionChangeListener> xChangeListener(
            new SelectionChangeHandler(xController, this));
        xSupplier->addSelectionChangeListener(xChangeListener);
    }
}

void DevelopmentToolDockingWindow::clearChildren(weld::TreeIter const& rParent)
{
    bool bChild = false;
    do
    {
        bChild = mpLeftSideTreeView->iter_has_child(rParent);
        if (bChild)
        {
            std::unique_ptr<weld::TreeIter> pChild = mpLeftSideTreeView->make_iterator(&rParent);
            bChild = mpLeftSideTreeView->iter_children(*pChild);
            if (bChild)
            {
                mpLeftSideTreeView->remove(*pChild);
            }
        }
    } while (bChild);
}

IMPL_LINK(DevelopmentToolDockingWindow, ModelTreeViewExpanding, weld::TreeIter const&, rParent,
          bool)
{
    OUString aText = mpLeftSideTreeView->get_text(rParent);
    if (aText == "Paragraphs")
    {
        clearChildren(rParent);
        fillParagraphs(rParent);
    }
    else if (aText == "Shapes")
    {
        if (msDocumentType == "Text Document")
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

IMPL_LINK_NOARG(DevelopmentToolDockingWindow, LeftSideSelected, weld::TreeView&, void)
{
    OUString sID = mpLeftSideTreeView->get_selected_text();
    if (maUnoObjectMap.find(sID) == maUnoObjectMap.end())
        return;
    auto& rObject = maUnoObjectMap.at(sID);
    if (rObject.is())
        introspect(rObject);
}

void DevelopmentToolDockingWindow::fillGraphicObjects(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxRoot, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xGraphicObjects = xSupplier->getGraphicObjects();
    const uno::Sequence<OUString> aNames = xGraphicObjects->getElementNames();
    for (auto const& rName : aNames)
    {
        maUnoObjectMap.emplace(rName, xGraphicObjects);
        lclAppendToParent(mpLeftSideTreeView, rParent, rName);
    }
}

void DevelopmentToolDockingWindow::fillOLEObjects(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxRoot, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xOleObjects = xSupplier->getEmbeddedObjects();
    const uno::Sequence<OUString> aNames = xOleObjects->getElementNames();
    for (auto const& rName : aNames)
    {
        maUnoObjectMap.emplace(rName, xOleObjects);
        lclAppendToParent(mpLeftSideTreeView, rParent, rName);
    }
}

void DevelopmentToolDockingWindow::fillStyleFamilies(weld::TreeIter const& rParent)
{
    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(mxRoot, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xStyleFamilies = xSupplier->getStyleFamilies();
    const uno::Sequence<OUString> aNames = xStyleFamilies->getElementNames();
    for (auto const& rFamilyName : aNames)
    {
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(rFamilyName),
                                                            uno::UNO_QUERY);
        maUnoObjectMap.emplace(rFamilyName, xStyleFamily);

        std::unique_ptr<weld::TreeIter> pCurrentStyleFamily = mpLeftSideTreeView->make_iterator();
        lclAppendToParentWithIter(mpLeftSideTreeView, rParent, *pCurrentStyleFamily, rFamilyName);

        const uno::Sequence<OUString> aStyleNames = xStyleFamily->getElementNames();
        for (auto const& rStyleName : aStyleNames)
        {
            uno::Reference<uno::XInterface> xStyle(xStyleFamily->getByName(rStyleName),
                                                   uno::UNO_QUERY);
            maUnoObjectMap.emplace(rStyleName, xStyle);
            lclAppendToParent(mpLeftSideTreeView, *pCurrentStyleFamily, rStyleName);
        }
    }
}

void DevelopmentToolDockingWindow::fillFrames(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextFramesSupplier> xSupplier(mxRoot, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xFrames = xSupplier->getTextFrames();
    const uno::Sequence<OUString> aNames = xFrames->getElementNames();
    for (auto const& rName : aNames)
    {
        maUnoObjectMap.emplace(rName, xFrames);
        lclAppendToParent(mpLeftSideTreeView, rParent, rName);
    }
}

void DevelopmentToolDockingWindow::fillTables(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxRoot, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
    const uno::Sequence<OUString> aNames = xTables->getElementNames();
    for (auto const& rName : aNames)
    {
        maUnoObjectMap.emplace(rName, xTables);
        lclAppendToParent(mpLeftSideTreeView, rParent, rName);
    }
}

void DevelopmentToolDockingWindow::fillSheets(weld::TreeIter const& rParent)
{
    uno::Reference<sheet::XSpreadsheetDocument> xSheetDoc(mxRoot, uno::UNO_QUERY);
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

        std::unique_ptr<weld::TreeIter> pCurrentSheet = mpLeftSideTreeView->make_iterator();
        lclAppendToParentWithIter(mpLeftSideTreeView, rParent, *pCurrentSheet, aSlideString);
        maUnoObjectMap.emplace(aSlideString, xSheet);

        {
            uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xSheet, uno::UNO_QUERY);
            uno::Reference<container::XIndexAccess> xDraws = xDrawPageSupplier->getDrawPage();

            std::unique_ptr<weld::TreeIter> pCurrentShapes = mpLeftSideTreeView->make_iterator();

            lclAppendToParentWithIter(mpLeftSideTreeView, *pCurrentSheet, *pCurrentShapes,
                                      "Shapes");
            maUnoObjectMap.emplace("Shapes", xDraws);

            for (sal_Int32 nIndexShapes = 0; nIndexShapes < xDraws->getCount(); ++nIndexShapes)
            {
                uno::Reference<uno::XInterface> xShape(xDraws->getByIndex(nIndexShapes),
                                                       uno::UNO_QUERY);
                OUString aShapeName = lclGetNamed(xShape);
                if (aShapeName.isEmpty())
                    aShapeName = "Shape " + OUString::number(nIndexShapes + 1);

                lclAppendToParent(mpLeftSideTreeView, *pCurrentShapes, aShapeName);
                maUnoObjectMap.emplace(aShapeName, xShape);
            }
        }

        {
            uno::Reference<table::XTableChartsSupplier> xSupplier(xSheet, uno::UNO_QUERY);
            uno::Reference<table::XTableCharts> xCharts = xSupplier->getCharts();
            std::unique_ptr<weld::TreeIter> pCurrentCharts = mpLeftSideTreeView->make_iterator();
            lclAppendToParentWithIter(mpLeftSideTreeView, *pCurrentSheet, *pCurrentCharts,
                                      "Charts");
            maUnoObjectMap.emplace("Charts", xCharts);

            const uno::Sequence<OUString> aNames = xCharts->getElementNames();
            for (auto const& rName : aNames)
            {
                uno::Reference<uno::XInterface> xChart(xCharts->getByName(rName), uno::UNO_QUERY);
                maUnoObjectMap.emplace(rName, xChart);
                lclAppendToParent(mpLeftSideTreeView, *pCurrentCharts, rName);
            }
        }

        {
            uno::Reference<sheet::XDataPilotTablesSupplier> xSupplier(xSheet, uno::UNO_QUERY);
            uno::Reference<sheet::XDataPilotTables> xPivotTables = xSupplier->getDataPilotTables();
            std::unique_ptr<weld::TreeIter> pCurrentPivotTables
                = mpLeftSideTreeView->make_iterator();
            lclAppendToParentWithIter(mpLeftSideTreeView, *pCurrentSheet, *pCurrentPivotTables,
                                      "Pivot Tables");
            maUnoObjectMap.emplace("Pivot Tables", xPivotTables);

            const uno::Sequence<OUString> aNames = xPivotTables->getElementNames();
            for (auto const& rName : aNames)
            {
                uno::Reference<uno::XInterface> xPivotTable(xPivotTables->getByName(rName),
                                                            uno::UNO_QUERY);
                maUnoObjectMap.emplace(rName, xPivotTable);
                lclAppendToParent(mpLeftSideTreeView, *pCurrentPivotTables, rName);
            }
        }
    }
}

void DevelopmentToolDockingWindow::fillPages(weld::TreeIter const& rParent)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxRoot, uno::UNO_QUERY);
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

        std::unique_ptr<weld::TreeIter> pCurrentPage = mpLeftSideTreeView->make_iterator();
        lclAppendToParentWithIter(mpLeftSideTreeView, rParent, *pCurrentPage, aPageString);
        maUnoObjectMap.emplace(aPageString, xPage);

        for (sal_Int32 nPageIndex = 0; nPageIndex < xPage->getCount(); ++nPageIndex)
        {
            uno::Reference<uno::XInterface> xShape(xPage->getByIndex(nPageIndex), uno::UNO_QUERY);

            OUString aShapeName = lclGetNamed(xShape);
            if (aShapeName.isEmpty())
                aShapeName = "Shape " + OUString::number(nPageIndex + 1);

            lclAppendToParent(mpLeftSideTreeView, *pCurrentPage, aShapeName);
            maUnoObjectMap.emplace(aShapeName, xShape);
        }
    }
}

void DevelopmentToolDockingWindow::fillSlides(weld::TreeIter const& rParent)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxRoot, uno::UNO_QUERY);
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

        std::unique_ptr<weld::TreeIter> pCurrentPage = mpLeftSideTreeView->make_iterator();
        lclAppendToParentWithIter(mpLeftSideTreeView, rParent, *pCurrentPage, aSlideName);
        maUnoObjectMap.emplace(aSlideName, xPage);

        for (sal_Int32 nPageIndex = 0; nPageIndex < xPage->getCount(); ++nPageIndex)
        {
            uno::Reference<uno::XInterface> xShape(xPage->getByIndex(nPageIndex), uno::UNO_QUERY);

            OUString aShapeName = lclGetNamed(xShape);
            if (aShapeName.isEmpty())
                aShapeName = "Shape " + OUString::number(nPageIndex + 1);

            lclAppendToParent(mpLeftSideTreeView, *pCurrentPage, aShapeName);
            maUnoObjectMap.emplace(aShapeName, xShape);
        }
    }
}

void DevelopmentToolDockingWindow::fillMasterSlides(weld::TreeIter const& rParent)
{
    uno::Reference<drawing::XMasterPagesSupplier> xSupplier(mxRoot, uno::UNO_QUERY);
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

        std::unique_ptr<weld::TreeIter> pCurrentPage = mpLeftSideTreeView->make_iterator();
        lclAppendToParentWithIter(mpLeftSideTreeView, rParent, *pCurrentPage, aSlideName);
        maUnoObjectMap.emplace(aSlideName, xPage);

        for (sal_Int32 nPageIndex = 0; nPageIndex < xPage->getCount(); ++nPageIndex)
        {
            uno::Reference<container::XNamed> xShape(xPage->getByIndex(nPageIndex), uno::UNO_QUERY);

            OUString aShapeName = xShape->getName();
            if (aShapeName.isEmpty())
                aShapeName = "Shape " + OUString::number(nPageIndex + 1);

            lclAppendToParent(mpLeftSideTreeView, *pCurrentPage, aShapeName);
            maUnoObjectMap.emplace(aShapeName, xShape);
        }
    }
}

void DevelopmentToolDockingWindow::fillParagraphs(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextDocument> xDocument(mxRoot, uno::UNO_QUERY);
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
        sal_Int32 i = 0;
        while (xParagraphEnum->hasMoreElements())
        {
            uno::Reference<text::XTextContent> const xParagraph(xParagraphEnum->nextElement(),
                                                                uno::UNO_QUERY);
            OUString aString = lclGetNamed(xParagraph);
            if (aString.isEmpty())
                aString = "Paragraph " + OUString::number(i + 1);
            lclAppendToParent(mpLeftSideTreeView, rParent, aString);

            maUnoObjectMap.emplace(aString, xParagraph);

            i++;
        }
    }
}

void DevelopmentToolDockingWindow::fillShapes(weld::TreeIter const& rParent)
{
    uno::Reference<text::XTextDocument> xDocument(mxRoot, uno::UNO_QUERY);
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

        lclAppendToParent(mpLeftSideTreeView, rParent, aShapeName);
        maUnoObjectMap.emplace(aShapeName, xShape);
    }
}

void DevelopmentToolDockingWindow::inspectDocument()
{
    uno::Reference<lang::XServiceInfo> xDocument(mxRoot, uno::UNO_QUERY_THROW);

    if (xDocument->supportsService("com.sun.star.sheet.SpreadsheetDocument"))
    {
        msDocumentType = "Spreadsheet Document";

        mpLeftSideTreeView->append_text(msDocumentType);
        maUnoObjectMap.emplace(msDocumentType, mxRoot);

        lclAppend(mpLeftSideTreeView, "Sheets");
        lclAppend(mpLeftSideTreeView, "Styles");
    }
    else if (xDocument->supportsService("com.sun.star.presentation.PresentationDocument"))
    {
        msDocumentType = "Presentation Document";

        mpLeftSideTreeView->append_text(msDocumentType);
        maUnoObjectMap.emplace(msDocumentType, mxRoot);

        lclAppend(mpLeftSideTreeView, "Slides");
        lclAppend(mpLeftSideTreeView, "Styles");
        lclAppend(mpLeftSideTreeView, "Master Slides");
    }
    else if (xDocument->supportsService("com.sun.star.drawing.DrawingDocument"))
    {
        msDocumentType = "Drawing Document";

        mpLeftSideTreeView->append_text(msDocumentType);
        maUnoObjectMap.emplace(msDocumentType, mxRoot);

        lclAppend(mpLeftSideTreeView, "Pages");
        lclAppend(mpLeftSideTreeView, "Styles");
    }
    else if (xDocument->supportsService("com.sun.star.text.TextDocument")
             || xDocument->supportsService("com.sun.star.text.WebDocument"))
    {
        msDocumentType = "Text Document";

        mpLeftSideTreeView->append_text(msDocumentType);
        maUnoObjectMap.emplace(msDocumentType, mxRoot);

        lclAppend(mpLeftSideTreeView, "Paragraphs");
        lclAppend(mpLeftSideTreeView, "Shapes");
        lclAppend(mpLeftSideTreeView, "Tables");
        lclAppend(mpLeftSideTreeView, "Frames");
        lclAppend(mpLeftSideTreeView, "Graphic Objects");
        lclAppend(mpLeftSideTreeView, "Embedded Objects");
        lclAppend(mpLeftSideTreeView, "Styles");
    }
}

DevelopmentToolDockingWindow::~DevelopmentToolDockingWindow() { disposeOnce(); }

void DevelopmentToolDockingWindow::dispose()
{
    mpClassNameLabel.reset();
    mpClassListBox.reset();
    mpLeftSideTreeView.reset();

    SfxDockingWindow::dispose();
}

void DevelopmentToolDockingWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel(Size(300, 300));

    Invalidate();
}

void DevelopmentToolDockingWindow::introspect(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;

    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
    if (!xContext.is())
        return;

    auto xServiceInfo = uno::Reference<lang::XServiceInfo>(xInterface, uno::UNO_QUERY);
    OUString aImplementationName = xServiceInfo->getImplementationName();

    mpClassNameLabel->set_label(aImplementationName);

    mpClassListBox->freeze();
    mpClassListBox->clear();

    std::unique_ptr<weld::TreeIter> pParent = mpClassListBox->make_iterator();
    OUString aServicesString("Services");
    mpClassListBox->insert(nullptr, -1, &aServicesString, nullptr, nullptr, nullptr, false,
                           pParent.get());
    mpClassListBox->set_text_emphasis(*pParent, true, 0);

    std::unique_ptr<weld::TreeIter> pResult = mpClassListBox->make_iterator();
    const uno::Sequence<OUString> aServiceNames(xServiceInfo->getSupportedServiceNames());
    for (auto const& aServiceName : aServiceNames)
    {
        mpClassListBox->insert(pParent.get(), -1, &aServiceName, nullptr, nullptr, nullptr, false,
                               pResult.get());
    }

    uno::Reference<beans::XIntrospection> xIntrospection;
    xIntrospection = beans::theIntrospection::get(xContext);

    uno::Reference<beans::XIntrospectionAccess> xIntrospectionAccess;
    xIntrospectionAccess = xIntrospection->inspect(uno::makeAny(xInterface));

    OUString aPropertiesString("Properties");
    mpClassListBox->insert(nullptr, -1, &aPropertiesString, nullptr, nullptr, nullptr, false,
                           pParent.get());
    mpClassListBox->set_text_emphasis(*pParent, true, 0);

    const auto xProperties = xIntrospectionAccess->getProperties(
        beans::PropertyConcept::ALL - beans::PropertyConcept::DANGEROUS);
    for (auto const& xProperty : xProperties)
    {
        mpClassListBox->insert(pParent.get(), -1, &xProperty.Name, nullptr, nullptr, nullptr, false,
                               pResult.get());
    }

    OUString aMethodsString("Methods");
    mpClassListBox->insert(nullptr, -1, &aMethodsString, nullptr, nullptr, nullptr, false,
                           pParent.get());
    mpClassListBox->set_text_emphasis(*pParent, true, 0);

    const auto xMethods = xIntrospectionAccess->getMethods(beans::MethodConcept::ALL);
    for (auto const& xMethod : xMethods)
    {
        OUString aMethodName = xMethod->getName();
        mpClassListBox->insert(pParent.get(), -1, &aMethodName, nullptr, nullptr, nullptr, false,
                               pResult.get());
    }

    mpClassListBox->thaw();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
