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

#include <sfx2/devtools/DocumentModelTreeHandler.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
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
class DocumentModelTreeEntry;

void lclAppendToParentEntry(std::unique_ptr<weld::TreeView>& rTree, weld::TreeIter const& rParent,
                            OUString const& rString, DocumentModelTreeEntry* pEntry,
                            bool bChildrenOnDemand = false)
{
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    rTree->insert(&rParent, -1, &rString, &sId, nullptr, nullptr, bChildrenOnDemand, nullptr);
}

OUString lclAppend(std::unique_ptr<weld::TreeView>& rTree, OUString const& rString,
                   DocumentModelTreeEntry* pEntry, bool bChildrenOnDemand = false)
{
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    rTree->insert(nullptr, -1, &rString, &sId, nullptr, nullptr, bChildrenOnDemand, nullptr);
    return sId;
}

// returns a name of the object, if available
OUString lclGetNamed(uno::Reference<uno::XInterface> const& xObject)
{
    uno::Reference<container::XNamed> xNamed(xObject, uno::UNO_QUERY);
    if (!xNamed.is())
        return OUString();
    return xNamed->getName();
}

/** DocumentModelTreeEntry is an object "attached" to a tree node.
 *
 * It represents an object that is "attached" to the tree view an is
 * responsible to provide the UNO object associated with the current
 * node and on demand create and fill the children of the said node.
 */
class DocumentModelTreeEntry
{
public:
    css::uno::Reference<css::uno::XInterface> mxObject;

    DocumentModelTreeEntry() = default;

    DocumentModelTreeEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : mxObject(xObject)
    {
    }

    virtual ~DocumentModelTreeEntry() {}

    /// The main UNO object for this entry
    virtual css::uno::Reference<css::uno::XInterface> getMainObject() { return mxObject; }

    /// Create and fill the children to the parent tree view node.
    virtual void fill(std::unique_ptr<weld::TreeView>& /*pDocumentModelTree*/,
                      weld::TreeIter const& /*rParent*/)
    {
    }

protected:
    /// A generic fill when the UNO object implements XNameAccess interface
    void fillNameAccess(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
                        weld::TreeIter const& rParent)
    {
        uno::Reference<container::XNameAccess> xNameAccess(getMainObject(), uno::UNO_QUERY);
        if (!xNameAccess.is())
            return;

        const uno::Sequence<OUString> aNames = xNameAccess->getElementNames();
        for (auto const& rName : aNames)
        {
            uno::Reference<uno::XInterface> xObject(xNameAccess->getByName(rName), uno::UNO_QUERY);
            auto pEntry = std::make_unique<DocumentModelTreeEntry>(xObject);
            lclAppendToParentEntry(pDocumentModelTree, rParent, rName, pEntry.release());
        }
    }
};

/** Entry that represents the document root object */
class DocumentRootEntry : public DocumentModelTreeEntry
{
public:
    DocumentRootEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }
};

/** Represents a paragraph object (XParagraph) */
class ParagraphEntry : public DocumentModelTreeEntry
{
public:
    ParagraphEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<container::XEnumerationAccess> xEnumAccess(getMainObject(), uno::UNO_QUERY);
        if (!xEnumAccess.is())
            return;

        uno::Reference<container::XEnumeration> xTextPortions = xEnumAccess->createEnumeration();

        if (xTextPortions.is())
        {
            for (sal_Int32 i = 0; xTextPortions->hasMoreElements(); i++)
            {
                uno::Reference<text::XTextRange> const xTextPortion(xTextPortions->nextElement(),
                                                                    uno::UNO_QUERY);
                OUString aString = lclGetNamed(xTextPortion);
                if (aString.isEmpty())
                    aString = "Text Portion " + OUString::number(i + 1);

                auto pEntry = std::make_unique<DocumentModelTreeEntry>(xTextPortion);
                lclAppendToParentEntry(pDocumentModelTree, rParent, aString, pEntry.release());
            }
        }
    }
};

/** Represents a list of paragraphs */
class ParagraphsEntry : public DocumentModelTreeEntry
{
public:
    ParagraphsEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<text::XTextDocument> xDocument(mxObject, uno::UNO_QUERY);
        if (!xDocument.is())
            return mxObject;

        return xDocument->getText()->getText();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<container::XEnumerationAccess> xEnumAccess(getMainObject(), uno::UNO_QUERY);
        if (!xEnumAccess.is())
            return;

        uno::Reference<container::XEnumeration> xParagraphEnum = xEnumAccess->createEnumeration();

        if (xParagraphEnum.is())
        {
            for (sal_Int32 i = 0; xParagraphEnum->hasMoreElements(); i++)
            {
                uno::Reference<text::XTextContent> const xParagraph(xParagraphEnum->nextElement(),
                                                                    uno::UNO_QUERY);
                OUString aString = lclGetNamed(xParagraph);
                if (aString.isEmpty())
                    aString = "Paragraph " + OUString::number(i + 1);

                auto pEntry = std::make_unique<ParagraphEntry>(xParagraph);
                lclAppendToParentEntry(pDocumentModelTree, rParent, aString, pEntry.release(),
                                       true);
            }
        }
    }
};

/** Represents a list of shapes */
class ShapesEntry : public DocumentModelTreeEntry
{
public:
    ShapesEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<drawing::XDrawPageSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getDrawPage();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<container::XIndexAccess> xShapes(getMainObject(), uno::UNO_QUERY);
        if (!xShapes.is())
            return;
        for (sal_Int32 nIndexShapes = 0; nIndexShapes < xShapes->getCount(); ++nIndexShapes)
        {
            uno::Reference<uno::XInterface> xShape(xShapes->getByIndex(nIndexShapes),
                                                   uno::UNO_QUERY);
            OUString aShapeName = lclGetNamed(xShape);
            if (aShapeName.isEmpty())
                aShapeName = "Shape " + OUString::number(nIndexShapes + 1);

            auto pEntry = std::make_unique<DocumentModelTreeEntry>(xShape);
            lclAppendToParentEntry(pDocumentModelTree, rParent, aShapeName, pEntry.release());
        }
    }
};

/** Represents a list of tables */
class TablesEntry : public DocumentModelTreeEntry
{
public:
    TablesEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<text::XTextTablesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getTextTables();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        fillNameAccess(pDocumentModelTree, rParent);
    }
};

/** Represents a list of frames */
class FramesEntry : public DocumentModelTreeEntry
{
public:
    FramesEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<text::XTextFramesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getTextFrames();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        fillNameAccess(pDocumentModelTree, rParent);
    }
};

/** Represents a list of writer graphic objects */
class WriterGraphicObjectsEntry : public DocumentModelTreeEntry
{
public:
    WriterGraphicObjectsEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getGraphicObjects();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        fillNameAccess(pDocumentModelTree, rParent);
    }
};

/** Represents a list of writer embedded (OLE) objects */
class EmbeddedObjectsEntry : public DocumentModelTreeEntry
{
public:
    EmbeddedObjectsEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getEmbeddedObjects();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        fillNameAccess(pDocumentModelTree, rParent);
    }
};

/** Represents a style family, which contains a list of styles */
class StylesFamilyEntry : public DocumentModelTreeEntry
{
public:
    StylesFamilyEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        fillNameAccess(pDocumentModelTree, rParent);
    }
};

/** Represents a list of style families */
class StylesFamiliesEntry : public DocumentModelTreeEntry
{
public:
    StylesFamiliesEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<style::XStyleFamiliesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getStyleFamilies();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<container::XNameAccess> xStyleFamilies(getMainObject(), uno::UNO_QUERY);
        if (!xStyleFamilies.is())
            return;

        const uno::Sequence<OUString> aNames = xStyleFamilies->getElementNames();
        for (auto const& rFamilyName : aNames)
        {
            uno::Reference<uno::XInterface> xStyleFamily(xStyleFamilies->getByName(rFamilyName),
                                                         uno::UNO_QUERY);

            auto pStylesFamilyEntry = std::make_unique<StylesFamilyEntry>(xStyleFamily);
            lclAppendToParentEntry(pDocumentModelTree, rParent, rFamilyName,
                                   pStylesFamilyEntry.release(), true);
        }
    }
};

/** Represents a list of pages */
class PagesEntry : public DocumentModelTreeEntry
{
public:
    PagesEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getDrawPages();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<drawing::XDrawPages> xDrawPages(getMainObject(), uno::UNO_QUERY);
        for (sal_Int32 i = 0; i < xDrawPages->getCount(); ++i)
        {
            uno::Reference<drawing::XDrawPage> xPage(xDrawPages->getByIndex(i), uno::UNO_QUERY);
            if (!xPage.is())
                continue;

            OUString aPageString = lclGetNamed(xPage);
            if (aPageString.isEmpty())
                aPageString = "Page " + OUString::number(i + 1);

            auto pShapesEntry = std::make_unique<ShapesEntry>(xPage);
            lclAppendToParentEntry(pDocumentModelTree, rParent, aPageString, pShapesEntry.release(),
                                   true);
        }
    }
};

/** Represents a list of (Impress) slides */
class SlidesEntry : public DocumentModelTreeEntry
{
public:
    SlidesEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getDrawPages();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<drawing::XDrawPages> xDrawPages(getMainObject(), uno::UNO_QUERY);
        for (sal_Int32 i = 0; i < xDrawPages->getCount(); ++i)
        {
            uno::Reference<drawing::XDrawPage> xPage(xDrawPages->getByIndex(i), uno::UNO_QUERY);
            if (!xPage.is())
                continue;

            OUString aPageString = lclGetNamed(xPage);
            if (aPageString.isEmpty())
                aPageString = "Slide " + OUString::number(i + 1);

            auto pShapesEntry = std::make_unique<ShapesEntry>(xPage);
            lclAppendToParentEntry(pDocumentModelTree, rParent, aPageString, pShapesEntry.release(),
                                   true);
        }
    }
};

/** Represents a list of (Impress) master slides */
class MasterSlidesEntry : public DocumentModelTreeEntry
{
public:
    MasterSlidesEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<drawing::XMasterPagesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getMasterPages();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<drawing::XDrawPages> xDrawPages(getMainObject(), uno::UNO_QUERY);
        for (sal_Int32 i = 0; i < xDrawPages->getCount(); ++i)
        {
            uno::Reference<drawing::XDrawPage> xPage(xDrawPages->getByIndex(i), uno::UNO_QUERY);
            if (!xPage.is())
                continue;

            OUString aPageString = lclGetNamed(xPage);
            if (aPageString.isEmpty())
                aPageString = "Master Slide " + OUString::number(i + 1);

            auto pShapesEntry = std::make_unique<ShapesEntry>(xPage);
            lclAppendToParentEntry(pDocumentModelTree, rParent, aPageString, pShapesEntry.release(),
                                   true);
        }
    }
};

/** Represents a list of charts */
class ChartsEntry : public DocumentModelTreeEntry
{
public:
    ChartsEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<table::XTableChartsSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getCharts();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<table::XTableCharts> xCharts(getMainObject(), uno::UNO_QUERY);
        if (!xCharts.is())
            return;
        fillNameAccess(pDocumentModelTree, rParent);
    }
};

/** Represents a list of pivot tables */
class PivotTablesEntry : public DocumentModelTreeEntry
{
public:
    PivotTablesEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<sheet::XDataPilotTablesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getDataPilotTables();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<sheet::XDataPilotTables> xPivotTables(getMainObject(), uno::UNO_QUERY);
        if (!xPivotTables.is())
            return;
        fillNameAccess(pDocumentModelTree, rParent);
    }
};

/** Represents a (Calc) sheet */
class SheetEntry : public DocumentModelTreeEntry
{
public:
    SheetEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        auto pShapesEntry = std::make_unique<ShapesEntry>(getMainObject());
        lclAppendToParentEntry(pDocumentModelTree, rParent, "Shapes", pShapesEntry.release(), true);

        auto pChartsEntry = std::make_unique<ChartsEntry>(getMainObject());
        lclAppendToParentEntry(pDocumentModelTree, rParent, "Charts", pChartsEntry.release(), true);

        auto pPivotTablesEntry = std::make_unique<PivotTablesEntry>(getMainObject());
        lclAppendToParentEntry(pDocumentModelTree, rParent, "Pivot Tables",
                               pPivotTablesEntry.release(), true);
    }
};

/** Represents a list of (Calc) sheet */
class SheetsEntry : public DocumentModelTreeEntry
{
public:
    SheetsEntry(css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<sheet::XSpreadsheetDocument> xSheetDocument(mxObject, uno::UNO_QUERY);
        if (!xSheetDocument.is())
            return mxObject;
        return xSheetDocument->getSheets();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<container::XIndexAccess> xIndex(getMainObject(), uno::UNO_QUERY);
        if (!xIndex.is())
            return;
        for (sal_Int32 i = 0; i < xIndex->getCount(); ++i)
        {
            uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(i), uno::UNO_QUERY);
            OUString aString = lclGetNamed(xSheet);
            if (aString.isEmpty())
                aString = "Sheet " + OUString::number(i + 1);
            auto pEntry = std::make_unique<SheetEntry>(xSheet);
            lclAppendToParentEntry(pDocumentModelTree, rParent, aString, pEntry.release(), true);
        }
    }
};

} // end anonymous namespace

DocumentModelTreeHandler::DocumentModelTreeHandler(
    std::unique_ptr<weld::TreeView>& pDocumentModelTree,
    css::uno::Reference<css::uno::XInterface> const& xDocument)
    : mpDocumentModelTree(pDocumentModelTree)
    , mxDocument(xDocument)
{
    mpDocumentModelTree->connect_expanding(LINK(this, DocumentModelTreeHandler, ExpandingHandler));
}

uno::Reference<uno::XInterface> DocumentModelTreeHandler::getObjectByID(OUString const& rID)
{
    uno::Reference<uno::XInterface> xObject;
    if (rID.isEmpty())
        return xObject;
    auto* pEntry = reinterpret_cast<DocumentModelTreeEntry*>(rID.toInt64());
    return pEntry->getMainObject();
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
                clearChildren(*pChild);
                OUString sID = mpDocumentModelTree->get_id(*pChild);
                auto* pEntry = reinterpret_cast<DocumentModelTreeEntry*>(sID.toInt64());
                delete pEntry;
                mpDocumentModelTree->remove(*pChild);
            }
        }
    } while (bChild);
}

void DocumentModelTreeHandler::dispose()
{
    mpDocumentModelTree->all_foreach([this](weld::TreeIter& rEntry) {
        OUString sID = mpDocumentModelTree->get_id(rEntry);
        auto* pEntry = reinterpret_cast<DocumentModelTreeEntry*>(sID.toInt64());
        delete pEntry;
        return false;
    });
}

IMPL_LINK(DocumentModelTreeHandler, ExpandingHandler, weld::TreeIter const&, rParent, bool)
{
    OUString sID = mpDocumentModelTree->get_id(rParent);
    if (sID.isEmpty())
        return true;

    clearChildren(rParent);
    auto* pEntry = reinterpret_cast<DocumentModelTreeEntry*>(sID.toInt64());
    pEntry->fill(mpDocumentModelTree, rParent);

    return true;
}

void DocumentModelTreeHandler::selectObject(
    css::uno::Reference<css::uno::XInterface> const& xInterface)
{
    mpDocumentModelTree->unselect_all();

    mpDocumentModelTree->all_foreach([this, xInterface](weld::TreeIter& rEntry) {
        OUString sID = mpDocumentModelTree->get_id(rEntry);
        auto* pEntry = reinterpret_cast<DocumentModelTreeEntry*>(sID.toInt64());
        if (xInterface == pEntry->getMainObject())
        {
            mpDocumentModelTree->select(rEntry);
            return true;
        }
        return false;
    });
}

void DocumentModelTreeHandler::inspectDocument()
{
    uno::Reference<lang::XServiceInfo> xDocumentServiceInfo(mxDocument, uno::UNO_QUERY_THROW);

    lclAppend(mpDocumentModelTree, u"Document", new DocumentRootEntry(mxDocument), false);

    if (xDocumentServiceInfo->supportsService("com.sun.star.sheet.SpreadsheetDocument"))
    {
        lclAppend(mpDocumentModelTree, u"Sheets", new SheetsEntry(mxDocument), true);
        lclAppend(mpDocumentModelTree, u"Styles", new StylesFamiliesEntry(mxDocument), true);
    }
    else if (xDocumentServiceInfo->supportsService(
                 "com.sun.star.presentation.PresentationDocument"))
    {
        lclAppend(mpDocumentModelTree, u"Slides", new SlidesEntry(mxDocument), true);
        lclAppend(mpDocumentModelTree, u"Master Slides", new MasterSlidesEntry(mxDocument), true);
        lclAppend(mpDocumentModelTree, u"Styles", new StylesFamiliesEntry(mxDocument), true);
    }
    else if (xDocumentServiceInfo->supportsService("com.sun.star.drawing.DrawingDocument"))
    {
        lclAppend(mpDocumentModelTree, u"Pages", new PagesEntry(mxDocument), true);
        lclAppend(mpDocumentModelTree, u"Styles", new StylesFamiliesEntry(mxDocument), true);
    }
    else if (xDocumentServiceInfo->supportsService("com.sun.star.text.TextDocument")
             || xDocumentServiceInfo->supportsService("com.sun.star.text.WebDocument"))
    {
        lclAppend(mpDocumentModelTree, u"Paragraphs", new ParagraphsEntry(mxDocument), true);
        lclAppend(mpDocumentModelTree, u"Shapes", new ShapesEntry(mxDocument), true);
        lclAppend(mpDocumentModelTree, u"Tables", new TablesEntry(mxDocument), true);
        lclAppend(mpDocumentModelTree, u"Frames", new FramesEntry(mxDocument), true);
        lclAppend(mpDocumentModelTree, u"Graphic Objects",
                  new WriterGraphicObjectsEntry(mxDocument), true);
        lclAppend(mpDocumentModelTree, u"Embedded Objects", new EmbeddedObjectsEntry(mxDocument),
                  true);
        lclAppend(mpDocumentModelTree, u"Styles", new StylesFamiliesEntry(mxDocument), true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
