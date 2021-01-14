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
#include <com/sun/star/container/XNamed.hpp>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>

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

void lclAppend(std::unique_ptr<weld::TreeView>& rTree, OUString const& rString)
{
    rTree->insert(nullptr, -1, &rString, nullptr, nullptr, nullptr, true, nullptr);
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

        std::unique_ptr<weld::TreeIter> pCurrentSheet = mpLeftSideTreeView->make_iterator();
        OUString aSlideString = "Sheet " + OUString::number(i + 1);
        maUnoObjectMap.emplace(aSlideString, xSheet);
        mpLeftSideTreeView->insert(&rParent, -1, &aSlideString, nullptr, nullptr, nullptr, false,
                                   pCurrentSheet.get());
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

        std::unique_ptr<weld::TreeIter> pCurrentPage = mpLeftSideTreeView->make_iterator();
        OUString aPageString = "Page " + OUString::number(i + 1);
        maUnoObjectMap.emplace(aPageString, xPage);
        mpLeftSideTreeView->insert(&rParent, -1, &aPageString, nullptr, nullptr, nullptr, false,
                                   pCurrentPage.get());

        for (sal_Int32 j = 0; j < xPage->getCount(); ++j)
        {
            uno::Reference<container::XNamed> xShape(xPage->getByIndex(j), uno::UNO_QUERY);

            OUString aShapeName = xShape->getName();
            if (aShapeName.isEmpty())
                aShapeName = "Shape " + OUString::number(j + 1);

            std::unique_ptr<weld::TreeIter> pCurrentShape = mpLeftSideTreeView->make_iterator();
            mpLeftSideTreeView->insert(pCurrentPage.get(), -1, &aShapeName, nullptr, nullptr,
                                       nullptr, false, pCurrentShape.get());
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

        std::unique_ptr<weld::TreeIter> pCurrentPage = mpLeftSideTreeView->make_iterator();
        OUString aSlideString = "Slide " + OUString::number(i + 1);
        maUnoObjectMap.emplace(aSlideString, xPage);
        mpLeftSideTreeView->insert(&rParent, -1, &aSlideString, nullptr, nullptr, nullptr, false,
                                   pCurrentPage.get());

        for (sal_Int32 j = 0; j < xPage->getCount(); ++j)
        {
            uno::Reference<container::XNamed> xShape(xPage->getByIndex(j), uno::UNO_QUERY);

            OUString aShapeName = xShape->getName();
            if (aShapeName.isEmpty())
                aShapeName = "Shape " + OUString::number(j + 1);

            std::unique_ptr<weld::TreeIter> pCurrentShape = mpLeftSideTreeView->make_iterator();
            mpLeftSideTreeView->insert(pCurrentPage.get(), -1, &aShapeName, nullptr, nullptr,
                                       nullptr, false, pCurrentShape.get());
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
        std::unique_ptr<weld::TreeIter> pCurrent = mpLeftSideTreeView->make_iterator();
        while (xParagraphEnum->hasMoreElements())
        {
            OUString aString = "Paragraph " + OUString::number(i + 1);
            mpLeftSideTreeView->insert(&rParent, -1, &aString, nullptr, nullptr, nullptr, false,
                                       pCurrent.get());

            uno::Reference<text::XTextContent> const xElem(xParagraphEnum->nextElement(),
                                                           uno::UNO_QUERY);
            maUnoObjectMap.emplace(aString, xElem);

            i++;
        }
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
    }
    else if (xDocument->supportsService("com.sun.star.presentation.PresentationDocument"))
    {
        msDocumentType = "Presentation Document";

        mpLeftSideTreeView->append_text(msDocumentType);
        maUnoObjectMap.emplace(msDocumentType, mxRoot);

        lclAppend(mpLeftSideTreeView, "Slides");
    }
    else if (xDocument->supportsService("com.sun.star.drawing.DrawingDocument"))
    {
        msDocumentType = "Drawing Document";

        mpLeftSideTreeView->append_text(msDocumentType);
        maUnoObjectMap.emplace(msDocumentType, mxRoot);

        lclAppend(mpLeftSideTreeView, "Pages");
    }
    else if (xDocument->supportsService("com.sun.star.text.TextDocument")
             || xDocument->supportsService("com.sun.star.text.WebDocument"))
    {
        msDocumentType = "Text Document";

        mpLeftSideTreeView->append_text(msDocumentType);
        maUnoObjectMap.emplace(msDocumentType, mxRoot);

        lclAppend(mpLeftSideTreeView, "Paragraphs");
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
