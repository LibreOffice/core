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

#include <sfx2/sfxresid.hxx>
#include "DevToolsStrings.hrc"

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
protected:
    OUString maString;
    css::uno::Reference<css::uno::XInterface> mxObject;

public:
    DocumentModelTreeEntry() = default;

    DocumentModelTreeEntry(OUString const& rString,
                           css::uno::Reference<css::uno::XInterface> const& xObject)
        : maString(rString)
        , mxObject(xObject)
    {
    }

    virtual ~DocumentModelTreeEntry() {}

    /// the node string shown in the tree view
    virtual OUString& getString() { return maString; }

    /// should show the expander for the tree view node
    virtual bool shouldShowExpander() { return false; }

    /// The main UNO object for this entry
    virtual css::uno::Reference<css::uno::XInterface> getMainObject() { return mxObject; }

    /// Create and fill the children to the parent tree view node.
    virtual void fill(std::unique_ptr<weld::TreeView>& /*pDocumentModelTree*/,
                      weld::TreeIter const& /*rParent*/)
    {
    }
};

void lclAppendToParentEntry(std::unique_ptr<weld::TreeView>& rTree, weld::TreeIter const& rParent,
                            DocumentModelTreeEntry* pEntry)
{
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    OUString const& rString = pEntry->getString();
    rTree->insert(&rParent, -1, &rString, &sId, nullptr, nullptr, pEntry->shouldShowExpander(),
                  nullptr);
}

OUString lclAppend(std::unique_ptr<weld::TreeView>& rTree, DocumentModelTreeEntry* pEntry)
{
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    OUString const& rString = pEntry->getString();
    rTree->insert(nullptr, -1, &rString, &sId, nullptr, nullptr, pEntry->shouldShowExpander(),
                  nullptr);
    return sId;
}

class NameAccessTreeEntry : public DocumentModelTreeEntry
{
protected:
    NameAccessTreeEntry(OUString const& rString, uno::Reference<uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
    {
    }

    bool shouldShowExpander() override
    {
        uno::Reference<container::XNameAccess> xNameAccess(getMainObject(), uno::UNO_QUERY);
        return xNameAccess.is() && xNameAccess->getElementNames().getLength() > 0;
    }

    /// A generic fill when the UNO object implements XNameAccess interface
    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<container::XNameAccess> xNameAccess(getMainObject(), uno::UNO_QUERY);
        xNameAccess.set(getMainObject(), uno::UNO_QUERY);
        if (!xNameAccess.is())
            return;

        const uno::Sequence<OUString> aNames = xNameAccess->getElementNames();
        for (auto const& rName : aNames)
        {
            uno::Reference<uno::XInterface> xObject(xNameAccess->getByName(rName), uno::UNO_QUERY);
            auto pEntry = std::make_unique<DocumentModelTreeEntry>(rName, xObject);
            lclAppendToParentEntry(pDocumentModelTree, rParent, pEntry.release());
        }
    }
};

/** Entry that represents the document root object */
class DocumentRootEntry : public DocumentModelTreeEntry
{
public:
    DocumentRootEntry(OUString const& rString, uno::Reference<uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
    {
    }

    bool shouldShowExpander() override { return false; }
};

/** Represents a paragraph object (XParagraph) */
class ParagraphEntry : public DocumentModelTreeEntry
{
public:
    ParagraphEntry(OUString const& rString,
                   css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
    {
    }

    bool shouldShowExpander() override
    {
        uno::Reference<container::XEnumerationAccess> xEnumAccess(getMainObject(), uno::UNO_QUERY);
        if (!xEnumAccess.is())
            return false;
        auto xTextPortions = xEnumAccess->createEnumeration();
        if (!xTextPortions.is())
            return false;
        return xTextPortions->hasMoreElements();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<container::XEnumerationAccess> xEnumAccess(getMainObject(), uno::UNO_QUERY);
        if (!xEnumAccess.is())
            return;

        uno::Reference<container::XEnumeration> xTextPortions = xEnumAccess->createEnumeration();
        if (!xTextPortions.is())
            return;

        for (sal_Int32 i = 0; xTextPortions->hasMoreElements(); i++)
        {
            uno::Reference<text::XTextRange> const xTextPortion(xTextPortions->nextElement(),
                                                                uno::UNO_QUERY);
            OUString aString = lclGetNamed(xTextPortion);
            if (aString.isEmpty())
            {
                OUString aNumber = OUString::number(i + 1);
                aString = SfxResId(STR_TEXT_PORTION).replaceFirst("%1", aNumber);
            }

            auto pEntry = std::make_unique<DocumentModelTreeEntry>(aString, xTextPortion);
            lclAppendToParentEntry(pDocumentModelTree, rParent, pEntry.release());
        }
    }
};

/** Represents a list of paragraphs */
class ParagraphsEntry : public DocumentModelTreeEntry
{
public:
    ParagraphsEntry(OUString const& rString,
                    css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<text::XTextDocument> xDocument(mxObject, uno::UNO_QUERY);
        if (!xDocument.is())
            return mxObject;

        return xDocument->getText()->getText();
    }

    bool shouldShowExpander() override
    {
        uno::Reference<container::XEnumerationAccess> xEnumAccess(getMainObject(), uno::UNO_QUERY);
        if (!xEnumAccess.is())
            return false;
        auto xParagraphEnum = xEnumAccess->createEnumeration();
        if (!xParagraphEnum.is())
            return false;
        return xParagraphEnum->hasMoreElements();
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        uno::Reference<container::XEnumerationAccess> xEnumAccess(getMainObject(), uno::UNO_QUERY);
        if (!xEnumAccess.is())
            return;

        uno::Reference<container::XEnumeration> xParagraphEnum = xEnumAccess->createEnumeration();

        if (!xParagraphEnum.is())
            return;

        for (sal_Int32 i = 0; xParagraphEnum->hasMoreElements(); i++)
        {
            uno::Reference<text::XTextContent> const xParagraph(xParagraphEnum->nextElement(),
                                                                uno::UNO_QUERY);
            OUString aString = lclGetNamed(xParagraph);
            if (aString.isEmpty())
            {
                aString = SfxResId(STR_PARAGRAPH).replaceFirst("%1", OUString::number(i + 1));
            }

            auto pEntry = std::make_unique<ParagraphEntry>(aString, xParagraph);
            lclAppendToParentEntry(pDocumentModelTree, rParent, pEntry.release());
        }
    }
};

/** Represents a list of shapes */
class ShapesEntry : public DocumentModelTreeEntry
{
public:
    ShapesEntry(OUString const& rString, css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<drawing::XDrawPageSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getDrawPage();
    }

    bool shouldShowExpander() override
    {
        uno::Reference<container::XIndexAccess> xShapes(getMainObject(), uno::UNO_QUERY);
        return xShapes.is() && xShapes->getCount() > 0;
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
            {
                aShapeName
                    = SfxResId(STR_SHAPE).replaceFirst("%1", OUString::number(nIndexShapes + 1));
            }

            auto pEntry = std::make_unique<DocumentModelTreeEntry>(aShapeName, xShape);
            lclAppendToParentEntry(pDocumentModelTree, rParent, pEntry.release());
        }
    }
};

/** Represents a list of tables */
class TablesEntry : public NameAccessTreeEntry
{
public:
    TablesEntry(OUString const& rString, css::uno::Reference<css::uno::XInterface> const& xObject)
        : NameAccessTreeEntry(rString, xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<text::XTextTablesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getTextTables();
    }
};

/** Represents a list of frames */
class FramesEntry : public NameAccessTreeEntry
{
public:
    FramesEntry(OUString const& rString, css::uno::Reference<css::uno::XInterface> const& xObject)
        : NameAccessTreeEntry(rString, xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<text::XTextFramesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getTextFrames();
    }
};

/** Represents a list of writer graphic objects */
class WriterGraphicObjectsEntry : public NameAccessTreeEntry
{
public:
    WriterGraphicObjectsEntry(OUString const& rString,
                              css::uno::Reference<css::uno::XInterface> const& xObject)
        : NameAccessTreeEntry(rString, xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getGraphicObjects();
    }
};

/** Represents a list of writer embedded (OLE) objects */
class EmbeddedObjectsEntry : public NameAccessTreeEntry
{
public:
    EmbeddedObjectsEntry(OUString const& rString,
                         css::uno::Reference<css::uno::XInterface> const& xObject)
        : NameAccessTreeEntry(rString, xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getEmbeddedObjects();
    }
};

/** Represents a style family, which contains a list of styles */
class StylesFamilyEntry : public NameAccessTreeEntry
{
public:
    StylesFamilyEntry(OUString const& rString,
                      css::uno::Reference<css::uno::XInterface> const& xObject)
        : NameAccessTreeEntry(rString, xObject)
    {
    }
};

/** Represents a list of style families */
class StylesFamiliesEntry : public DocumentModelTreeEntry
{
public:
    StylesFamiliesEntry(OUString const& rString,
                        css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<style::XStyleFamiliesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getStyleFamilies();
    }

    bool shouldShowExpander() override
    {
        uno::Reference<container::XNameAccess> xStyleFamilies(getMainObject(), uno::UNO_QUERY);
        return xStyleFamilies.is() && xStyleFamilies->getElementNames().getLength() > 0;
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

            auto pStylesFamilyEntry
                = std::make_unique<StylesFamilyEntry>(rFamilyName, xStyleFamily);
            lclAppendToParentEntry(pDocumentModelTree, rParent, pStylesFamilyEntry.release());
        }
    }
};

/** Represents a list of pages */
class PagesEntry : public DocumentModelTreeEntry
{
public:
    PagesEntry(OUString const& rString, css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getDrawPages();
    }

    bool shouldShowExpander() override
    {
        uno::Reference<drawing::XDrawPages> xDrawPages(getMainObject(), uno::UNO_QUERY);
        return xDrawPages.is() && xDrawPages->getCount() > 0;
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
                aPageString = SfxResId(STR_PAGE).replaceFirst("%1", OUString::number(i + 1));

            auto pShapesEntry = std::make_unique<ShapesEntry>(aPageString, xPage);
            lclAppendToParentEntry(pDocumentModelTree, rParent, pShapesEntry.release());
        }
    }
};

/** Represents a list of (Impress) slides */
class SlidesEntry : public DocumentModelTreeEntry
{
public:
    SlidesEntry(OUString const& rString, css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getDrawPages();
    }

    bool shouldShowExpander() override
    {
        uno::Reference<drawing::XDrawPages> xDrawPages(getMainObject(), uno::UNO_QUERY);
        return xDrawPages.is() && xDrawPages->getCount() > 0;
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
                aPageString = SfxResId(STR_SLIDE).replaceFirst("%1", OUString::number(i + 1));

            auto pShapesEntry = std::make_unique<ShapesEntry>(aPageString, xPage);
            lclAppendToParentEntry(pDocumentModelTree, rParent, pShapesEntry.release());
        }
    }
};

/** Represents a list of (Impress) master slides */
class MasterSlidesEntry : public DocumentModelTreeEntry
{
public:
    MasterSlidesEntry(OUString const& rString,
                      css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<drawing::XMasterPagesSupplier> xSupplier(mxObject, uno::UNO_QUERY);
        if (!xSupplier.is())
            return mxObject;
        return xSupplier->getMasterPages();
    }

    bool shouldShowExpander() override
    {
        uno::Reference<drawing::XDrawPages> xDrawPages(getMainObject(), uno::UNO_QUERY);
        return xDrawPages.is() && xDrawPages->getCount() > 0;
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
            {
                aPageString
                    = SfxResId(STR_MASTER_SLIDE).replaceFirst("%1", OUString::number(i + 1));
            }

            auto pShapesEntry = std::make_unique<ShapesEntry>(aPageString, xPage);
            lclAppendToParentEntry(pDocumentModelTree, rParent, pShapesEntry.release());
        }
    }
};

/** Represents a list of charts */
class ChartsEntry : public NameAccessTreeEntry
{
public:
    ChartsEntry(OUString const& rString, css::uno::Reference<css::uno::XInterface> const& xObject)
        : NameAccessTreeEntry(rString, xObject)
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
        NameAccessTreeEntry::fill(pDocumentModelTree, rParent);
    }
};

/** Represents a list of pivot tables */
class PivotTablesEntry : public NameAccessTreeEntry
{
public:
    PivotTablesEntry(OUString const& rString,
                     css::uno::Reference<css::uno::XInterface> const& xObject)
        : NameAccessTreeEntry(rString, xObject)
    {
    }

    bool shouldShowExpander() override { return true; }

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
        NameAccessTreeEntry::fill(pDocumentModelTree, rParent);
    }
};

/** Represents a (Calc) sheet */
class SheetEntry : public DocumentModelTreeEntry
{
public:
    SheetEntry(OUString const& rString, css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
    {
    }

    bool shouldShowExpander() override { return true; }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        auto pShapesEntry
            = std::make_unique<ShapesEntry>(SfxResId(STR_SHAPES_ENTRY), getMainObject());
        lclAppendToParentEntry(pDocumentModelTree, rParent, pShapesEntry.release());

        auto pChartsEntry
            = std::make_unique<ChartsEntry>(SfxResId(STR_CHARTS_ENTRY), getMainObject());
        lclAppendToParentEntry(pDocumentModelTree, rParent, pChartsEntry.release());

        auto pPivotTablesEntry
            = std::make_unique<PivotTablesEntry>(SfxResId(STR_PIVOT_TABLES_ENTRY), getMainObject());
        lclAppendToParentEntry(pDocumentModelTree, rParent, pPivotTablesEntry.release());
    }
};

/** Represents a list of (Calc) sheet */
class SheetsEntry : public DocumentModelTreeEntry
{
private:
    uno::Reference<container::XIndexAccess> mxIndexAccess;

public:
    SheetsEntry(OUString const& rString, css::uno::Reference<css::uno::XInterface> const& xObject)
        : DocumentModelTreeEntry(rString, xObject)
        , mxIndexAccess(xObject, uno::UNO_QUERY)
    {
    }

    css::uno::Reference<css::uno::XInterface> getMainObject() override
    {
        uno::Reference<sheet::XSpreadsheetDocument> xSheetDocument(mxObject, uno::UNO_QUERY);
        if (!xSheetDocument.is())
            return mxObject;
        return xSheetDocument->getSheets();
    }

    bool shouldShowExpander() override
    {
        return mxIndexAccess.is() && mxIndexAccess->getCount() > 0;
    }

    void fill(std::unique_ptr<weld::TreeView>& pDocumentModelTree,
              weld::TreeIter const& rParent) override
    {
        if (!mxIndexAccess.is())
            return;

        for (sal_Int32 i = 0; i < mxIndexAccess->getCount(); ++i)
        {
            uno::Reference<sheet::XSpreadsheet> xSheet(mxIndexAccess->getByIndex(i),
                                                       uno::UNO_QUERY);
            OUString aString = lclGetNamed(xSheet);
            if (aString.isEmpty())
                aString = SfxResId(STR_SHEETS).replaceFirst("%1", OUString::number(i + 1));
            auto pEntry = std::make_unique<SheetEntry>(aString, xSheet);
            lclAppendToParentEntry(pDocumentModelTree, rParent, pEntry.release());
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

    lclAppend(mpDocumentModelTree, new DocumentRootEntry(SfxResId(STR_DOCUMENT_ENTRY), mxDocument));

    if (xDocumentServiceInfo->supportsService("com.sun.star.sheet.SpreadsheetDocument"))
    {
        lclAppend(mpDocumentModelTree, new SheetsEntry(SfxResId(STR_SHEETS_ENTRY), mxDocument));
        lclAppend(mpDocumentModelTree,
                  new StylesFamiliesEntry(SfxResId(STR_STYLES_ENTRY), mxDocument));
    }
    else if (xDocumentServiceInfo->supportsService(
                 "com.sun.star.presentation.PresentationDocument"))
    {
        lclAppend(mpDocumentModelTree, new SlidesEntry(SfxResId(STR_SLIDES_ENTRY), mxDocument));
        lclAppend(mpDocumentModelTree,
                  new MasterSlidesEntry(SfxResId(STR_MASTER_SLIDES_ENTRY), mxDocument));
        lclAppend(mpDocumentModelTree,
                  new StylesFamiliesEntry(SfxResId(STR_STYLES_ENTRY), mxDocument));
    }
    else if (xDocumentServiceInfo->supportsService("com.sun.star.drawing.DrawingDocument"))
    {
        lclAppend(mpDocumentModelTree, new PagesEntry(SfxResId(STR_PAGES_ENTRY), mxDocument));
        lclAppend(mpDocumentModelTree,
                  new StylesFamiliesEntry(SfxResId(STR_STYLES_ENTRY), mxDocument));
    }
    else if (xDocumentServiceInfo->supportsService("com.sun.star.text.TextDocument")
             || xDocumentServiceInfo->supportsService("com.sun.star.text.WebDocument"))
    {
        lclAppend(mpDocumentModelTree,
                  new ParagraphsEntry(SfxResId(STR_PARAGRAPHS_ENTRY), mxDocument));
        lclAppend(mpDocumentModelTree, new ShapesEntry(SfxResId(STR_SHAPES_ENTRY), mxDocument));
        lclAppend(mpDocumentModelTree, new TablesEntry(SfxResId(STR_TABLES_ENTRY), mxDocument));
        lclAppend(mpDocumentModelTree, new FramesEntry(SfxResId(STR_FRAMES_ENTRY), mxDocument));
        lclAppend(mpDocumentModelTree,
                  new WriterGraphicObjectsEntry(SfxResId(STR_GRAPHIC_OBJECTS_ENTRY), mxDocument));
        lclAppend(mpDocumentModelTree,
                  new EmbeddedObjectsEntry(SfxResId(STR_EMBEDDED_OBJECTS_ENTRY), mxDocument));
        lclAppend(mpDocumentModelTree,
                  new StylesFamiliesEntry(SfxResId(STR_STYLES_ENTRY), mxDocument));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
