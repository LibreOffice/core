/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/ColumnSeparatorStyle.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/graph.hxx>

#include <boost/property_tree/json_parser.hpp>
#include <tools/json_writer.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <itabenum.hxx>
#include <lineinfo.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <sw_primitivetypes2d.hxx>
#include <unotxdoc.hxx>
#include <VectorCollector.hxx>
#include <VectorPart.hxx>
#include <wrtsh.hxx>

#include <algorithm>
#include <cmath>
#include <optional>
#include <sstream>

using namespace com::sun::star;
using namespace ::cpo;

namespace
{
/// Covers the primitive decomposition of the layout, sw/source/core/layout/VectorCollector.cxx.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/unit/vector/data/"_ustr)
    {
    }

protected:
    std::vector<sw::vector::Entry> decomposeFirstPage()
    {
        SwRootFrame* pLayout = getSwDoc()->getIDocumentLayoutAccess().GetCurrentLayout();
        const SwPageFrame* pPage = pLayout->GetLower()->DynCastPageFrame();
        CPPUNIT_ASSERT(pPage);
        return sw::vector::decomposePage(*pPage);
    }

    const SwPageFrame& firstPage()
    {
        SwRootFrame* pLayout = getSwDoc()->getIDocumentLayoutAccess().GetCurrentLayout();
        return *pLayout->GetLower()->DynCastPageFrame();
    }

    /// The answer to a request for the primitives of the first page, as a client sends it.
    boost::property_tree::ptree requestFirstPage(std::optional<sal_Int64> oSince = std::nullopt)
    {
        OString aCommand = ".uno:VectorPrimitives?partid=" + sw::vector::partIdOf(firstPage());
        if (oSince)
            aCommand += "&since=" + OString::number(*oSince) + "&epoch="
                        + OString::number(getSwTextDoc()->getVectorParts()->getEpoch());
        tools::JsonWriter aWriter;
        getSwTextDoc()->getCommandValues(aWriter, aCommand);
        return parse(aWriter.finishAndGetAsOString());
    }

    /// The delta the document pushes for the first page after a change all over it.
    std::optional<boost::property_tree::ptree> pushFirstPage()
    {
        const SwPageFrame& rPage = firstPage();
        SwRootFrame* pLayout = getSwDoc()->getIDocumentLayoutAccess().GetCurrentLayout();
        getSwTextDoc()->getVectorParts()->invalidate(*pLayout, { rPage.getFrameArea() });
        tools::JsonWriter aWriter;
        getSwTextDoc()->pushVectorPrimitivesDelta(aWriter, sw::vector::partIdOf(rPage),
                                                  sw::vector::constModePrintLayout);
        const OString aJson = aWriter.finishAndGetAsOString();
        if (aJson == tools::JsonWriter::EMPTY_DOCUMENT)
            return std::nullopt;
        return parse(aJson);
    }

    static boost::property_tree::ptree parse(const OString& rJson)
    {
        std::stringstream aStream((std::string(rJson)));
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        return aTree;
    }

    /// Two paragraphs, the second one filled with red.
    void createTwoParagraphsWithFilledSecond()
    {
        createSwDoc();
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->Insert(u"first"_ustr);
        pWrtShell->SplitNode();
        pWrtShell->Insert(u"second"_ustr);
        uno::Reference<beans::XPropertySet> xParagraph(getParagraph(2), uno::UNO_QUERY);
        xParagraph->setPropertyValue(u"FillStyle"_ustr, uno::Any(drawing::FillStyle_SOLID));
        xParagraph->setPropertyValue(u"FillColor"_ustr, uno::Any(sal_Int32(0xff0000)));
        calcLayout();
    }
};

/// The nSkip-th entry of the kind, or nullptr when there are not that many.
const sw::vector::Entry* findEntry(const std::vector<sw::vector::Entry>& rEntries,
                                   sw::vector::EntryKind eKind, size_t nSkip = 0)
{
    for (const sw::vector::Entry& rEntry : rEntries)
    {
        if (rEntry.meKind != eKind)
            continue;
        if (nSkip == 0)
            return &rEntry;
        --nSkip;
    }
    return nullptr;
}

const sw::vector::Entry* findEntryById(const std::vector<sw::vector::Entry>& rEntries,
                                       sal_uInt64 nId)
{
    auto aIterator = std::find_if(rEntries.begin(), rEntries.end(),
                           [nId](const sw::vector::Entry& rEntry) { return rEntry.mnId == nId; });
    return aIterator == rEntries.end() ? nullptr : &*aIterator;
}

basegfx::B2DRange rangeOf(const sw::vector::Entry& rEntry)
{
    return rEntry.maPrimitives.getB2DRange(drawinglayer::geometry::ViewInformation2D());
}

/// Whether a primitive with the id is among rPrimitives or what they decompose to.
bool holds(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitives, sal_uInt32 nId)
{
    const drawinglayer::geometry::ViewInformation2D aViewInformation;
    for (const drawinglayer::primitive2d::Primitive2DReference& rPrimitive : rPrimitives)
    {
        // A text portion with no outlines, such as one of blanks, decomposes to nothing.
        if (!rPrimitive)
            continue;
        if (rPrimitive->getPrimitive2DID() == nId)
            return true;
        drawinglayer::primitive2d::Primitive2DContainer aDecomposed;
        rPrimitive->get2DDecomposition(aDecomposed, aViewInformation);
        if (holds(aDecomposed, nId))
            return true;
    }
    return false;
}

/// The text portions among rPrimitives and inside the groups that hold them, in order.
void collectPortions(
    const drawinglayer::primitive2d::Primitive2DContainer& rPrimitives,
    std::vector<const drawinglayer::primitive2d::TextSimplePortionPrimitive2D*>& rPortions)
{
    for (const drawinglayer::primitive2d::Primitive2DReference& rPrimitive : rPrimitives)
    {
        if (auto pPortion
            = dynamic_cast<const drawinglayer::primitive2d::TextSimplePortionPrimitive2D*>(
                rPrimitive.get()))
            rPortions.push_back(pPortion);
        else if (auto pGroup
                 = dynamic_cast<const drawinglayer::primitive2d::GroupPrimitive2D*>(
                     rPrimitive.get()))
            collectPortions(pGroup->getChildren(), rPortions);
    }
}

std::vector<const drawinglayer::primitive2d::TextSimplePortionPrimitive2D*>
portionsOf(const sw::vector::Entry& rEntry)
{
    std::vector<const drawinglayer::primitive2d::TextSimplePortionPrimitive2D*> aPortions;
    collectPortions(rEntry.maPrimitives, aPortions);
    return aPortions;
}

/// The text that the portions show, one after the other.
OUString textOf(const sw::vector::Entry& rEntry)
{
    OUStringBuffer aText;
    for (const auto* pPortion : portionsOf(rEntry))
    {
        aText.append(pPortion->getText().subView(pPortion->getTextPosition(),
                                                 pPortion->getTextLength()));
    }
    return aText.makeStringAndClear();
}

/// How many line primitives the entry holds.
int countLines(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitives)
{
    int nLines = 0;
    for (const drawinglayer::primitive2d::Primitive2DReference& rPrimitive : rPrimitives)
    {
        if (rPrimitive->getPrimitive2DID() == PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D)
            ++nLines;
        else if (auto pGroup
                 = dynamic_cast<const drawinglayer::primitive2d::GroupPrimitive2D*>(
                     rPrimitive.get()))
            nLines += countLines(pGroup->getChildren());
    }
    return nLines;
}
}

/// The page is the first entry and has no parent. The body sits in the page and the paragraph
/// in the body.
CPPUNIT_TEST_FIXTURE(Test, testVectorPageEntryComesFirst)
{
    createSwDoc();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    CPPUNIT_ASSERT(!aEntries.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(sw::vector::EntryKind::Page),
                         static_cast<int>(aEntries[0].meKind));
    CPPUNIT_ASSERT_EQUAL(sw::vector::constPageEntryId, aEntries[0].mnId);
    CPPUNIT_ASSERT(!aEntries[0].moParent);

    const sw::vector::Entry* pBody = findEntry(aEntries, sw::vector::EntryKind::Body);
    CPPUNIT_ASSERT(pBody);
    CPPUNIT_ASSERT(pBody->moParent);
    CPPUNIT_ASSERT_EQUAL(sw::vector::constPageEntryId, *pBody->moParent);
    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT(pText->moParent);
    CPPUNIT_ASSERT_EQUAL(pBody->mnId, *pText->moParent);
}

/// The fill of a paragraph is drawn relative to the top left of its own frame.
CPPUNIT_TEST_FIXTURE(Test, testVectorBackgroundIsRelativeToItsFrame)
{
    createTwoParagraphsWithFilledSecond();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pSecond
        = findEntry(aEntries, sw::vector::EntryKind::TextFrame, /*nSkip=*/1);
    CPPUNIT_ASSERT(pSecond);
    CPPUNIT_ASSERT(holds(pSecond->maPrimitives, PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D));
    // The frame sits well inside the page, while its fill starts at the frame's own top left.
    CPPUNIT_ASSERT_GREATER(tools::Long(1000), pSecond->maBox.Top());
    const basegfx::B2DRange aRange = rangeOf(*pSecond);
    CPPUNIT_ASSERT_LESS(2.0, std::abs(aRange.getMinX()));
    CPPUNIT_ASSERT_LESS(2.0, std::abs(aRange.getMinY()));
}

/// A paragraph pushed down the page keeps the same primitives, and only its box moves.
CPPUNIT_TEST_FIXTURE(Test, testVectorMovedParagraphKeepsItsPrimitives)
{
    createTwoParagraphsWithFilledSecond();
    std::vector<sw::vector::Entry> aBefore = decomposeFirstPage();
    const sw::vector::Entry* pSecond
        = findEntry(aBefore, sw::vector::EntryKind::TextFrame, /*nSkip=*/1);
    CPPUNIT_ASSERT(pSecond);

    // A new paragraph at the start of the document pushes the second paragraph down.
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->SplitNode();
    calcLayout();
    std::vector<sw::vector::Entry> aAfter = decomposeFirstPage();

    const sw::vector::Entry* pMoved = findEntryById(aAfter, pSecond->mnId);
    CPPUNIT_ASSERT(pMoved);
    CPPUNIT_ASSERT_GREATER(pSecond->maBox.Top(), pMoved->maBox.Top());
    CPPUNIT_ASSERT(pMoved->maPrimitives.operator==(pSecond->maPrimitives));
}

/// The border of a paragraph is an entry of its own after the paragraph, with the same box.
CPPUNIT_TEST_FIXTURE(Test, testVectorParagraphBorder)
{
    createSwDoc();
    table::BorderLine2 aBorder;
    aBorder.Color = 0x0000ff;
    aBorder.OuterLineWidth = 50;
    aBorder.LineWidth = 50;
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue(u"TopBorder"_ustr, uno::Any(aBorder));
    xParagraph->setPropertyValue(u"BottomBorder"_ustr, uno::Any(aBorder));
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT(!holds(pText->maPrimitives, PRIMITIVE2D_ID_SWBORDERRECTANGLERIMITIVE));
    auto aTextIterator = std::find_if(aEntries.begin(), aEntries.end(), [pText](const auto& rEntry)
                                      { return rEntry.mnId == pText->mnId; });
    auto aBordersIterator = std::find_if(aEntries.begin(), aEntries.end(), [](const auto& rEntry) {
        return rEntry.meKind == sw::vector::EntryKind::Borders;
    });
    CPPUNIT_ASSERT(aBordersIterator != aEntries.end());
    CPPUNIT_ASSERT(aTextIterator < aBordersIterator);
    CPPUNIT_ASSERT(aBordersIterator->moParent);
    CPPUNIT_ASSERT_EQUAL(pText->mnId, *aBordersIterator->moParent);
    CPPUNIT_ASSERT_EQUAL(pText->maBox, aBordersIterator->maBox);
    CPPUNIT_ASSERT(holds(aBordersIterator->maPrimitives, PRIMITIVE2D_ID_SWBORDERRECTANGLERIMITIVE));
    CPPUNIT_ASSERT_LESS(100.0, std::abs(rangeOf(*aBordersIterator).getMinY()));
}

/// The separator line between the columns of the body lies over the content of the columns, and
/// each column is an entry.
CPPUNIT_TEST_FIXTURE(Test, testVectorColumnSeparatorLine)
{
    createSwDoc();
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextColumns> xColumns(xStyle->getPropertyValue(u"TextColumns"_ustr),
                                                uno::UNO_QUERY);
    xColumns->setColumnCount(2);
    uno::Reference<beans::XPropertySet> xColumnProperties(xColumns, uno::UNO_QUERY);
    // A separator line has no style and no width until it is given them.
    xColumnProperties->setPropertyValue(u"SeparatorLineIsOn"_ustr, uno::Any(true));
    xColumnProperties->setPropertyValue(u"SeparatorLineStyle"_ustr,
                                        uno::Any(text::ColumnSeparatorStyle::SOLID));
    xColumnProperties->setPropertyValue(u"SeparatorLineWidth"_ustr, uno::Any(sal_Int32(50)));
    xStyle->setPropertyValue(u"TextColumns"_ustr, uno::Any(xColumns));
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pBody = findEntry(aEntries, sw::vector::EntryKind::Body);
    CPPUNIT_ASSERT(pBody);
    // The line is in the border lines of the body, which come after the last column.
    auto aLinesIterator = std::find_if(aEntries.begin(), aEntries.end(), [pBody](const auto& rEntry)
                                       {
                                           return rEntry.meKind == sw::vector::EntryKind::Borders
                                                  && rEntry.moParent
                                                  && *rEntry.moParent == pBody->mnId;
                                       });
    CPPUNIT_ASSERT(aLinesIterator != aEntries.end());
    CPPUNIT_ASSERT(holds(aLinesIterator->maPrimitives, PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D));
    auto aLastColumnIterator
        = std::find_if(aEntries.rbegin(), aEntries.rend(), [](const auto& rEntry)
                       { return rEntry.meKind == sw::vector::EntryKind::Column; });
    CPPUNIT_ASSERT(aLastColumnIterator != aEntries.rend());
    CPPUNIT_ASSERT(aLastColumnIterator.base() <= aLinesIterator);
    // Each column of the body is an entry of its own.
    CPPUNIT_ASSERT(findEntry(aEntries, sw::vector::EntryKind::Column, /*nSkip=*/1));
}

/// A graphic in the background of a table cell is drawn as the fill of the cell.
CPPUNIT_TEST_FIXTURE(Test, testVectorCellBackgroundGraphicIsDrawn)
{
    // A table cell has its background as a brush, which can hold a graphic.
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/1, /*nCols=*/1);
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    Bitmap aBitmap(Size(8, 8), vcl::PixelFormat::N24_BPP);
    aBitmap.Erase(COL_LIGHTRED);
    xCell->setPropertyValue(u"BackGraphic"_ustr, uno::Any(Graphic(aBitmap).GetXGraphic()));
    xCell->setPropertyValue(u"BackGraphicLocation"_ustr, uno::Any(style::GraphicLocation_TILED));
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pCell = findEntry(aEntries, sw::vector::EntryKind::Cell);
    CPPUNIT_ASSERT(pCell);
    CPPUNIT_ASSERT(holds(pCell->maPrimitives, PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D));
}

/// The collapsing borders of a table are an entry of the table that comes after all its cells.
CPPUNIT_TEST_FIXTURE(Test, testVectorCollapsingTableBordersLieOverTheCells)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/2);
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    xTable->setPropertyValue(u"CollapsingBorders"_ustr, uno::Any(true));
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    auto aBordersIterator = std::find_if(aEntries.begin(), aEntries.end(), [](const auto& rEntry) {
        return rEntry.meKind == sw::vector::EntryKind::TableBorders;
    });
    CPPUNIT_ASSERT(aBordersIterator != aEntries.end());
    CPPUNIT_ASSERT(
        holds(aBordersIterator->maPrimitives, PRIMITIVE2D_ID_SDRFRAMEBORDERTPRIMITIVE2D));
    // The lines are drawn after every cell, so a cell's fill cannot cover them.
    auto aLastCellIterator = std::find_if(aEntries.rbegin(), aEntries.rend(), [](const auto& rEntry)
                                          { return rEntry.meKind == sw::vector::EntryKind::Cell; });
    CPPUNIT_ASSERT(aLastCellIterator != aEntries.rend());
    CPPUNIT_ASSERT(aLastCellIterator.base() <= aBordersIterator);
    const sw::vector::Entry* pTable = findEntry(aEntries, sw::vector::EntryKind::Table);
    CPPUNIT_ASSERT(pTable);
    CPPUNIT_ASSERT(aBordersIterator->moParent);
    CPPUNIT_ASSERT_EQUAL(pTable->mnId, *aBordersIterator->moParent);
}

/// A hidden shape has no entry, as the paint leaves it out.
CPPUNIT_TEST_FIXTURE(Test, testVectorHiddenShapeHasNoEntry)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(3000, 2000));
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY_THROW);
    xShapeProperties->setPropertyValue(u"AnchorType"_ustr,
                                       uno::Any(text::TextContentAnchorType_AT_PARAGRAPH));
    uno::Reference<drawing::XDrawPageSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    xSupplier->getDrawPage()->add(xShape);
    xShapeProperties->setPropertyValue(u"Visible"_ustr, uno::Any(false));
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    CPPUNIT_ASSERT(!findEntry(aEntries, sw::vector::EntryKind::Shape));
}

/// When the document paints the objects behind the text over the header, a frame behind the
/// text is drawn over the header and under the body text.
CPPUNIT_TEST_FIXTURE(Test, testVectorFrameBehindTheTextLiesBetweenHeaderAndBody)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xFactory->createInstance(u"com.sun.star.text.DocumentSettings"_ustr), uno::UNO_QUERY);
    xSettings->setPropertyValue(u"PaintHellOverHeaderFooter"_ustr, uno::Any(true));
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    xStyle->setPropertyValue(u"HeaderIsOn"_ustr, uno::Any(true));
    uno::Reference<text::XTextContent> xFrame(
        xFactory->createInstance(u"com.sun.star.text.TextFrame"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrameProperties(xFrame, uno::UNO_QUERY_THROW);
    xFrameProperties->setPropertyValue(u"Size"_ustr, uno::Any(awt::Size(4000, 2000)));
    xFrameProperties->setPropertyValue(u"Opaque"_ustr, uno::Any(false));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xFrame, false);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    auto aHeaderIterator = std::find_if(aEntries.begin(), aEntries.end(), [](const auto& rEntry)
                                        { return rEntry.meKind == sw::vector::EntryKind::Header; });
    auto aFlyIterator = std::find_if(aEntries.begin(), aEntries.end(), [](const auto& rEntry)
                                     { return rEntry.meKind == sw::vector::EntryKind::Fly; });
    auto aBodyIterator = std::find_if(aEntries.begin(), aEntries.end(), [](const auto& rEntry)
                                      { return rEntry.meKind == sw::vector::EntryKind::Body; });
    CPPUNIT_ASSERT(aHeaderIterator != aEntries.end());
    CPPUNIT_ASSERT(aFlyIterator != aEntries.end());
    CPPUNIT_ASSERT(aBodyIterator != aEntries.end());
    CPPUNIT_ASSERT(aHeaderIterator < aFlyIterator);
    CPPUNIT_ASSERT(aFlyIterator < aBodyIterator);
}

/// The text of a paragraph is drawn as text portions with the advances of the layout,
/// positioned relative to the frame.
CPPUNIT_TEST_FIXTURE(Test, testVectorParagraphTextIsDrawnAsPortions)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"Hello world"_ustr);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT_EQUAL(u"Hello world"_ustr, textOf(*pText));
    CPPUNIT_ASSERT_EQUAL(1, countLines(pText->maPrimitives));
    for (const auto* pPortion : portionsOf(*pText))
    {
        // Every character has an advance from the layout.
        CPPUNIT_ASSERT_EQUAL(size_t(pPortion->getTextLength()), pPortion->getDXArray().size());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pPortion->getLayout().mnParagraph);
        CPPUNIT_ASSERT(pPortion->getLayout().mfLineWidth > 0.0);
        CPPUNIT_ASSERT(pPortion->getLayout().mfLineHeight > 0.0);
        // The baseline lies inside the frame, near its top, so the position is local.
        const double fBaselineY = pPortion->getTextTransform().get(1, 2);
        CPPUNIT_ASSERT(fBaselineY > 0.0);
        CPPUNIT_ASSERT(fBaselineY < pText->maBox.Height());
    }
}

/// A paragraph that wraps has one line primitive per row, going down the frame.
CPPUNIT_TEST_FIXTURE(Test, testVectorWrappedParagraphHasALinePerRow)
{
    createSwDoc();
    OUStringBuffer aLong;
    for (int i = 0; i < 40; ++i)
        aLong.append("lorem ipsum ");
    getSwDocShell()->GetWrtShell()->Insert(aLong.makeStringAndClear());
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT_GREATER(1, countLines(pText->maPrimitives));
    // The lines go down the frame one after the other.
    double fPreviousY = -1.0;
    for (const auto* pPortion : portionsOf(*pText))
    {
        const double fY = pPortion->getTextTransform().get(1, 2);
        CPPUNIT_ASSERT(fY >= fPreviousY);
        fPreviousY = fY;
    }
}

/// A line number is text of its own left of the frame, outside the lines of the paragraph.
CPPUNIT_TEST_FIXTURE(Test, testVectorLineNumberIsDrawnBesideTheParagraph)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"numbered"_ustr);
    SwDoc* pDoc = getSwDoc();
    SwLineNumberInfo aInfo(pDoc->GetLineNumberInfo());
    aInfo.SetPaintLineNumbers(true);
    aInfo.SetCountBy(1);
    pDoc->SetLineNumberInfo(aInfo);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    bool bFoundNumber = false;
    for (const drawinglayer::primitive2d::Primitive2DReference& rPrimitive : pText->maPrimitives)
    {
        auto pPortion
            = dynamic_cast<const drawinglayer::primitive2d::TextSimplePortionPrimitive2D*>(
                rPrimitive.get());
        if (!pPortion)
            continue;
        const OUString aText(
            pPortion->getText().copy(pPortion->getTextPosition(), pPortion->getTextLength()));
        if (aText != "1")
            continue;
        bFoundNumber = true;
        CPPUNIT_ASSERT_LESS(0.0, pPortion->getTextTransform().get(0, 2));
    }
    CPPUNIT_ASSERT(bFoundNumber);
    CPPUNIT_ASSERT_EQUAL(1, countLines(pText->maPrimitives));
}

/// The text portions of a centered paragraph carry the alignment and sit in the middle of the
/// line.
CPPUNIT_TEST_FIXTURE(Test, testVectorPortionsCarryTheParagraphAlignment)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"centered"_ustr);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue(u"ParaAdjust"_ustr,
                                 uno::Any(sal_Int16(style::ParagraphAdjust_CENTER)));
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    const auto aPortions = portionsOf(*pText);
    CPPUNIT_ASSERT(!aPortions.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(drawinglayer::primitive2d::TextPortionAlign::Center),
                         static_cast<int>(aPortions[0]->getLayout().meAlign));
    // The text sits in the middle of the line, away from where the line starts.
    const double fTextStart = aPortions[0]->getTextTransform().get(0, 2);
    CPPUNIT_ASSERT(fTextStart > aPortions[0]->getLayout().mfLineIndent + 100);
}

/// Text in the automatic color stays automatic in the primitives.
CPPUNIT_TEST_FIXTURE(Test, testVectorAutomaticTextColorStaysAutomatic)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"automatic"_ustr);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    const auto aPortions = portionsOf(*pText);
    CPPUNIT_ASSERT(!aPortions.empty());
    CPPUNIT_ASSERT(aPortions[0]->getFontColor().isAutomatic());
}

/// Text in the automatic color on a dark paragraph fill is white, as every client draws the
/// same fill.
CPPUNIT_TEST_FIXTURE(Test, testVectorAutomaticTextColorOnADarkFillIsWhite)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"on dark"_ustr);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue(u"FillStyle"_ustr, uno::Any(drawing::FillStyle_SOLID));
    xParagraph->setPropertyValue(u"FillColor"_ustr, uno::Any(sal_Int32(0x000080)));
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    const auto aPortions = portionsOf(*pText);
    CPPUNIT_ASSERT(!aPortions.empty());
    CPPUNIT_ASSERT(!aPortions[0]->getFontColor().isAutomatic());
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, Color(aPortions[0]->getFontColor()));
}

/// A form checkbox is drawn with the paragraph and marked as a field.
CPPUNIT_TEST_FIXTURE(Test, testVectorFormCheckboxIsDrawnWithTheText)
{
    createSwDoc();
    dispatchCommand(mxComponent, u".uno:CheckBoxFormField"_ustr, {});
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    // The box of the checkbox is hairlines among the primitives of the paragraph.
    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT(holds(pText->maPrimitives, PRIMITIVE2D_ID_POLYPOLYGONHAIRLINEPRIMITIVE2D));
    // The checkbox is marked as a field, so it is told apart from the text of the paragraph.
    CPPUNIT_ASSERT(holds(pText->maPrimitives, PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D));
}

/// Bold and underlined words get portions of their own that carry the font weight and the
/// underline.
CPPUNIT_TEST_FIXTURE(Test, testVectorBoldAndUnderlinedWordsHaveTheirOwnPortions)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"plain bold under"_ustr);
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextCursor> xCursor
        = xParagraph->getText()->createTextCursorByRange(xParagraph->getStart());
    xCursor->goRight(6, false);
    xCursor->goRight(4, true);
    uno::Reference<beans::XPropertySet>(xCursor, uno::UNO_QUERY_THROW)
        ->setPropertyValue(u"CharWeight"_ustr, uno::Any(awt::FontWeight::BOLD));
    xCursor->goRight(1, false);
    xCursor->goRight(5, true);
    uno::Reference<beans::XPropertySet>(xCursor, uno::UNO_QUERY_THROW)
        ->setPropertyValue(u"CharUnderline"_ustr, uno::Any(awt::FontUnderline::SINGLE));
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT_EQUAL(u"plain bold under"_ustr, textOf(*pText));
    bool bFoundBold = false;
    bool bFoundUnderline = false;
    for (const auto* pPortion : portionsOf(*pText))
    {
        const OUString aText(pPortion->getText().copy(pPortion->getTextPosition(),
                                                      pPortion->getTextLength()));
        if (aText == "bold")
            bFoundBold = pPortion->getFontAttribute().getWeight() >= 7; // WEIGHT_BOLD
        if (aText == "under")
        {
            auto pDecorated = dynamic_cast<
                const drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D*>(pPortion);
            bFoundUnderline = pDecorated
                              && pDecorated->getFontUnderline()
                                     != drawinglayer::primitive2d::TEXT_LINE_NONE;
        }
    }
    CPPUNIT_ASSERT(bFoundBold);
    CPPUNIT_ASSERT(bFoundUnderline);
}

/// The numbering label of a numbered paragraph is a bullet in front of the paragraph text.
CPPUNIT_TEST_FIXTURE(Test, testVectorNumberedParagraphHasItsLabelAsBullet)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"item"_ustr);
    dispatchCommand(mxComponent, u".uno:DefaultNumbering"_ustr, {});
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT(holds(pText->maPrimitives, PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D));
    CPPUNIT_ASSERT(textOf(*pText).endsWith("item"));
}

/// A right-to-left run in a left-to-right paragraph starts at the left end of the line.
CPPUNIT_TEST_FIXTURE(Test, testVectorRightToLeftRunStartsAtTheLineStart)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"\u05E9\u05DC\u05D5\u05DD"_ustr);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    const auto aPortions = portionsOf(*pText);
    CPPUNIT_ASSERT(!aPortions.empty());
    CPPUNIT_ASSERT(aPortions[0]->getFontAttribute().getRTL());
    // The frame has no indent, so the run starts at the left edge of the frame.
    const double fStart = aPortions[0]->getTextTransform().get(0, 2);
    CPPUNIT_ASSERT_LESS(20.0, std::abs(fStart));
}

/// The text of a right-to-left paragraph lies inside the frame, against its right edge.
CPPUNIT_TEST_FIXTURE(Test, testVectorRightToLeftParagraphEndsAtTheRightEdge)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"\u05E9\u05DC\u05D5\u05DD"_ustr);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue(u"WritingMode"_ustr, uno::Any(text::WritingMode2::RL_TB));
    // Left and right alignment keep their side in a right-to-left paragraph.
    xParagraph->setPropertyValue(u"ParaAdjust"_ustr,
                                 uno::Any(sal_Int16(style::ParagraphAdjust_RIGHT)));
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pText = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pText);
    const auto aPortions = portionsOf(*pText);
    CPPUNIT_ASSERT(!aPortions.empty());
    const auto* pPortion = aPortions[0];
    CPPUNIT_ASSERT(pPortion->getFontAttribute().getRTL());
    CPPUNIT_ASSERT(!pPortion->getDXArray().empty());
    const double fStart = pPortion->getTextTransform().get(0, 2);
    const double fEnd = fStart + pPortion->getDXArray().back();
    const double fFrameWidth = pText->maBox.Width();
    CPPUNIT_ASSERT_GREATEREQUAL(0.0, fStart);
    // The run ends at the right edge of the frame.
    CPPUNIT_ASSERT_LESS(20.0, std::abs(fFrameWidth - fEnd));
}

/// A paragraph with text pushed down the page keeps the same primitives.
CPPUNIT_TEST_FIXTURE(Test, testVectorMovedParagraphKeepsItsText)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"first"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"second paragraph"_ustr);
    calcLayout();
    std::vector<sw::vector::Entry> aBefore = decomposeFirstPage();
    const sw::vector::Entry* pSecond
        = findEntry(aBefore, sw::vector::EntryKind::TextFrame, /*nSkip=*/1);
    CPPUNIT_ASSERT(pSecond);

    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->SplitNode();
    calcLayout();
    std::vector<sw::vector::Entry> aAfter = decomposeFirstPage();

    const sw::vector::Entry* pMoved = findEntryById(aAfter, pSecond->mnId);
    CPPUNIT_ASSERT(pMoved);
    CPPUNIT_ASSERT_GREATER(pSecond->maBox.Top(), pMoved->maBox.Top());
    CPPUNIT_ASSERT(pMoved->maPrimitives.operator==(pSecond->maPrimitives));
}

/// The picture among the primitives of the entry, or nullptr when the entry draws none.
static const drawinglayer::primitive2d::GraphicPrimitive2D*
pictureOf(const sw::vector::Entry& rEntry)
{
    for (const drawinglayer::primitive2d::Primitive2DReference& rPrimitive : rEntry.maPrimitives)
    {
        if (auto pPicture
            = dynamic_cast<const drawinglayer::primitive2d::GraphicPrimitive2D*>(rPrimitive.get()))
            return pPicture;
    }
    return nullptr;
}

/// A small red bitmap, so that the picture has content to draw.
static uno::Any redPicture()
{
    Bitmap aBitmap(Size(8, 8), vcl::PixelFormat::N24_BPP);
    aBitmap.Erase(COL_LIGHTRED);
    return uno::Any(Graphic(aBitmap).GetXGraphic());
}

/// A picture anchored at a paragraph is a fly entry that names the paragraph as its anchor,
/// with the picture drawn inside the fly over the whole entry, cropped and mirrored as set.
CPPUNIT_TEST_FIXTURE(Test, testVectorPictureIsAFlyAnchoredInItsParagraph)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphic(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xGraphic->setPropertyValue(u"AnchorType"_ustr,
                               uno::Any(text::TextContentAnchorType_AT_PARAGRAPH));
    xGraphic->setPropertyValue(u"Size"_ustr, uno::Any(awt::Size(2000, 1000)));
    xGraphic->setPropertyValue(u"Graphic"_ustr, redPicture());
    text::GraphicCrop aCrop;
    aCrop.Left = 50;
    xGraphic->setPropertyValue(u"GraphicCrop"_ustr, uno::Any(aCrop));
    xGraphic->setPropertyValue(u"VertMirrored"_ustr, uno::Any(true));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertTextContent(xText->createTextCursor(),
                             uno::Reference<text::XTextContent>(xGraphic, uno::UNO_QUERY), false);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pParagraph = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    const sw::vector::Entry* pFly = findEntry(aEntries, sw::vector::EntryKind::Fly);
    const sw::vector::Entry* pPicture = findEntry(aEntries, sw::vector::EntryKind::NoTextFrame);
    CPPUNIT_ASSERT(pParagraph);
    CPPUNIT_ASSERT(pFly);
    CPPUNIT_ASSERT(pPicture);
    CPPUNIT_ASSERT(pFly->moAnchor);
    CPPUNIT_ASSERT_EQUAL(pParagraph->mnId, *pFly->moAnchor);
    CPPUNIT_ASSERT(pPicture->moParent);
    CPPUNIT_ASSERT_EQUAL(pFly->mnId, *pPicture->moParent);

    // The picture covers the entry from its top left corner and keeps the crop and the mirroring.
    const drawinglayer::primitive2d::GraphicPrimitive2D* pGraphic = pictureOf(*pPicture);
    CPPUNIT_ASSERT(pGraphic);
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, pGraphic->getGraphicObject().GetType());
    CPPUNIT_ASSERT_EQUAL(basegfx::utils::createScaleB2DHomMatrix(pPicture->maBox.Width(),
                                                                 pPicture->maBox.Height()),
                         pGraphic->getTransform());
    CPPUNIT_ASSERT(pGraphic->getGraphicAttr().IsCropped());
    CPPUNIT_ASSERT(pGraphic->getGraphicAttr().IsMirrored());
}

/// A picture that cannot be read on a rotated frame shows the box that says why upright over the
/// whole entry, as the paint draws it.
CPPUNIT_TEST_FIXTURE(Test, testVectorBrokenRotatedPictureShowsAnUprightBox)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphic(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xGraphic->setPropertyValue(u"AnchorType"_ustr,
                               uno::Any(text::TextContentAnchorType_AT_PARAGRAPH));
    xGraphic->setPropertyValue(u"Size"_ustr, uno::Any(awt::Size(2000, 1000)));
    // A rotation of 30 degrees, in tenths of a degree.
    xGraphic->setPropertyValue(u"GraphicRotation"_ustr, uno::Any(sal_Int16(300)));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertTextContent(xText->createTextCursor(),
                             uno::Reference<text::XTextContent>(xGraphic, uno::UNO_QUERY), false);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pPicture = findEntry(aEntries, sw::vector::EntryKind::NoTextFrame);
    CPPUNIT_ASSERT(pPicture);
    // The frame is rotated, so its area is wider than the 1134 twips of the picture.
    CPPUNIT_ASSERT_GREATER(tools::Long(1200), pPicture->maBox.Width());
    const drawinglayer::primitive2d::GraphicPrimitive2D* pGraphic = pictureOf(*pPicture);
    CPPUNIT_ASSERT(pGraphic);
    CPPUNIT_ASSERT_EQUAL(GraphicType::GdiMetafile, pGraphic->getGraphicObject().GetType());
    CPPUNIT_ASSERT_EQUAL(basegfx::utils::createScaleB2DHomMatrix(pPicture->maBox.Width(),
                                                                 pPicture->maBox.Height()),
                         pGraphic->getTransform());
}

/// A picture anchored as a character is drawn once, by its own fly, and the paragraph draws
/// only its text.
CPPUNIT_TEST_FIXTURE(Test, testVectorPictureAsCharacterIsNotDrawnByTheText)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"before "_ustr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphic(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xGraphic->setPropertyValue(u"AnchorType"_ustr,
                               uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    xGraphic->setPropertyValue(u"Size"_ustr, uno::Any(awt::Size(2000, 1000)));
    xGraphic->setPropertyValue(u"Graphic"_ustr, redPicture());
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xCursor->gotoEnd(false);
    xText->insertTextContent(xCursor, uno::Reference<text::XTextContent>(xGraphic, uno::UNO_QUERY),
                             false);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pParagraph = findEntry(aEntries, sw::vector::EntryKind::TextFrame);
    CPPUNIT_ASSERT(pParagraph);
    CPPUNIT_ASSERT(!holds(pParagraph->maPrimitives, PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D));
    CPPUNIT_ASSERT(findEntry(aEntries, sw::vector::EntryKind::Fly));
    const sw::vector::Entry* pPicture = findEntry(aEntries, sw::vector::EntryKind::NoTextFrame);
    CPPUNIT_ASSERT(pPicture);
    const drawinglayer::primitive2d::GraphicPrimitive2D* pGraphic = pictureOf(*pPicture);
    CPPUNIT_ASSERT(pGraphic);
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, pGraphic->getGraphicObject().GetType());
}

/// The header and the footer are entries of the page, and the header text is a paragraph of
/// the header.
CPPUNIT_TEST_FIXTURE(Test, testVectorHeaderAndFooterAreEntriesOfThePage)
{
    createSwDoc();
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    xStyle->setPropertyValue(u"HeaderIsOn"_ustr, uno::Any(true));
    xStyle->setPropertyValue(u"FooterIsOn"_ustr, uno::Any(true));
    uno::Reference<text::XText> xHeader(xStyle->getPropertyValue(u"HeaderText"_ustr),
                                        uno::UNO_QUERY);
    xHeader->setString(u"header text"_ustr);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pHeader = findEntry(aEntries, sw::vector::EntryKind::Header);
    const sw::vector::Entry* pFooter = findEntry(aEntries, sw::vector::EntryKind::Footer);
    CPPUNIT_ASSERT(pHeader);
    CPPUNIT_ASSERT(pFooter);
    CPPUNIT_ASSERT(pHeader->moParent);
    CPPUNIT_ASSERT_EQUAL(sw::vector::constPageEntryId, *pHeader->moParent);
    CPPUNIT_ASSERT(pFooter->moParent);
    CPPUNIT_ASSERT_EQUAL(sw::vector::constPageEntryId, *pFooter->moParent);
    auto aTextIterator = std::find_if(aEntries.begin(), aEntries.end(),
                                      [pHeader](const auto& rEntry)
                                      {
                                          return rEntry.meKind == sw::vector::EntryKind::TextFrame
                                                 && rEntry.moParent
                                                 && *rEntry.moParent == pHeader->mnId;
                                      });
    CPPUNIT_ASSERT(aTextIterator != aEntries.end());
    CPPUNIT_ASSERT_EQUAL(u"header text"_ustr, textOf(*aTextIterator));
}

/// A footnote is an entry of the footnote container, which draws the separator line.
CPPUNIT_TEST_FIXTURE(Test, testVectorFootnoteIsAnEntryWithSeparatorLine)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xFootnote(
        xFactory->createInstance(u"com.sun.star.text.Footnote"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xFootnote, false);
    uno::Reference<text::XText>(xFootnote, uno::UNO_QUERY_THROW)->setString(u"note"_ustr);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pContainer
        = findEntry(aEntries, sw::vector::EntryKind::FootnoteContainer);
    CPPUNIT_ASSERT(pContainer);
    CPPUNIT_ASSERT(holds(pContainer->maPrimitives, PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D));
    const sw::vector::Entry* pFootnote = findEntry(aEntries, sw::vector::EntryKind::Footnote);
    CPPUNIT_ASSERT(pFootnote);
    CPPUNIT_ASSERT(pFootnote->moParent);
    CPPUNIT_ASSERT_EQUAL(pContainer->mnId, *pFootnote->moParent);
}

/// A text frame holds its paragraphs, and its border lies over them in an entry after them.
CPPUNIT_TEST_FIXTURE(Test, testVectorTextFrameHoldsItsParagraphs)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFrame(
        xFactory->createInstance(u"com.sun.star.text.TextFrame"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet>(xFrame, uno::UNO_QUERY_THROW)
        ->setPropertyValue(u"Size"_ustr, uno::Any(awt::Size(4000, 2000)));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xFrame, false);
    uno::Reference<text::XText>(xFrame, uno::UNO_QUERY_THROW)->setString(u"inside"_ustr);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pFly = findEntry(aEntries, sw::vector::EntryKind::Fly);
    CPPUNIT_ASSERT(pFly);
    auto aTextIterator = std::find_if(aEntries.begin(), aEntries.end(), [pFly](const auto& rEntry) {
        return rEntry.meKind == sw::vector::EntryKind::TextFrame && rEntry.moParent
               && *rEntry.moParent == pFly->mnId;
    });
    CPPUNIT_ASSERT(aTextIterator != aEntries.end());
    CPPUNIT_ASSERT_EQUAL(u"inside"_ustr, textOf(*aTextIterator));
    // The default frame has a border, which lies over the content of the fly.
    auto aBordersIterator = std::find_if(aEntries.begin(), aEntries.end(),
                                         [pFly](const auto& rEntry)
                                         {
                                             return rEntry.meKind == sw::vector::EntryKind::Borders
                                                    && rEntry.moParent
                                                    && *rEntry.moParent == pFly->mnId;
                                         });
    CPPUNIT_ASSERT(aBordersIterator != aEntries.end());
    CPPUNIT_ASSERT(aTextIterator < aBordersIterator);
    CPPUNIT_ASSERT(holds(aBordersIterator->maPrimitives, PRIMITIVE2D_ID_SWBORDERRECTANGLERIMITIVE));
}

/// A text frame without a fill of its own is still filled, so it hides what is behind it.
CPPUNIT_TEST_FIXTURE(Test, testVectorTextFrameWithoutFillHidesWhatIsBehind)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFrame(
        xFactory->createInstance(u"com.sun.star.text.TextFrame"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrameProperties(xFrame, uno::UNO_QUERY_THROW);
    xFrameProperties->setPropertyValue(u"Size"_ustr, uno::Any(awt::Size(4000, 2000)));
    xFrameProperties->setPropertyValue(u"FillStyle"_ustr, uno::Any(drawing::FillStyle_NONE));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xFrame, false);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    // The fly is opaque although neither it nor the frames it is anchored in have a fill.
    const sw::vector::Entry* pFly = findEntry(aEntries, sw::vector::EntryKind::Fly);
    CPPUNIT_ASSERT(pFly);
    CPPUNIT_ASSERT(holds(pFly->maPrimitives, PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D));
}

/// A shape is an entry whose geometry starts at the top left of its own box.
CPPUNIT_TEST_FIXTURE(Test, testVectorShapeIsAnEntryInItsOwnCoordinates)
{
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(3000, 2000));
    uno::Reference<beans::XPropertySet>(xShape, uno::UNO_QUERY_THROW)
        ->setPropertyValue(u"AnchorType"_ustr, uno::Any(text::TextContentAnchorType_AT_PARAGRAPH));
    uno::Reference<drawing::XDrawPageSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    xSupplier->getDrawPage()->add(xShape);
    calcLayout();

    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();

    const sw::vector::Entry* pShape = findEntry(aEntries, sw::vector::EntryKind::Shape);
    CPPUNIT_ASSERT(pShape);
    CPPUNIT_ASSERT(!pShape->maPrimitives.empty());
    const basegfx::B2DRange aRange = rangeOf(*pShape);
    CPPUNIT_ASSERT_LESS(100.0, std::abs(aRange.getMinX()));
    CPPUNIT_ASSERT_LESS(100.0, std::abs(aRange.getMinY()));
}

namespace
{
/// The object with the id in a payload, or nullptr.
const boost::property_tree::ptree* objectOf(const boost::property_tree::ptree& rPayload,
                                            sal_uInt64 nId)
{
    for (const auto& rObject : rPayload.get_child("objects"))
    {
        if (rObject.second.get<sal_uInt64>("id") == nId)
            return &rObject.second;
    }
    return nullptr;
}
}

/// The first request for a page answers the whole page, with the page entry first and the
/// coordinates of each entry relative to the entry.
CPPUNIT_TEST_FIXTURE(Test, testVectorPageIsServedWhole)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"text"_ustr);
    calcLayout();

    boost::property_tree::ptree aPayload = requestFirstPage();

    CPPUNIT_ASSERT_EQUAL(std::string("vectorprimitives"), aPayload.get<std::string>("type"));
    CPPUNIT_ASSERT_EQUAL(std::string(sw::vector::partIdOf(firstPage())),
                         aPayload.get<std::string>("partId"));
    CPPUNIT_ASSERT_EQUAL(std::string("entry"), aPayload.get<std::string>("coordinates"));
    CPPUNIT_ASSERT(aPayload.get<sal_Int64>("version") > 0);
    const auto& rObjects = aPayload.get_child("objects");
    CPPUNIT_ASSERT(!rObjects.empty());
    const auto& rPage = rObjects.begin()->second;
    CPPUNIT_ASSERT_EQUAL(std::string("page"), rPage.get<std::string>("kind"));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(-1), rPage.get<sal_Int64>("parent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), rPage.get<sal_Int64>("x"));
    // Every entry of a whole page comes with what it draws.
    for (const auto& rObject : rObjects)
        CPPUNIT_ASSERT(rObject.second.get_child_optional("primitives"));
}

/// Typing in one paragraph gives a delta that carries that paragraph and no other.
CPPUNIT_TEST_FIXTURE(Test, testVectorDeltaCarriesTheChangedParagraphOnly)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"first"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"second"_ustr);
    calcLayout();
    const sal_Int64 nServed = requestFirstPage().get<sal_Int64>("version");
    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();
    const sal_uInt64 nFirstId
        = findEntry(aEntries, sw::vector::EntryKind::TextFrame)->mnId;
    const sal_uInt64 nSecondId
        = findEntry(aEntries, sw::vector::EntryKind::TextFrame, /*nSkip=*/1)->mnId;

    // Typing at the end of the second paragraph changes it and nothing else.
    pWrtShell->Insert(u" more"_ustr);
    calcLayout();
    std::optional<boost::property_tree::ptree> oDelta = pushFirstPage();

    CPPUNIT_ASSERT(oDelta);
    CPPUNIT_ASSERT_EQUAL(std::string("vectorprimitivesdelta"), oDelta->get<std::string>("type"));
    CPPUNIT_ASSERT_EQUAL(nServed, oDelta->get<sal_Int64>("from"));
    CPPUNIT_ASSERT_GREATER(nServed, oDelta->get<sal_Int64>("version"));
    const boost::property_tree::ptree* pSecond = objectOf(*oDelta, nSecondId);
    CPPUNIT_ASSERT(pSecond);
    CPPUNIT_ASSERT(pSecond->get_child_optional("primitives"));
    CPPUNIT_ASSERT(!objectOf(*oDelta, nFirstId));
}

/// A paragraph pushed down by a new one comes in the delta without its primitives, and the
/// new order comes along.
CPPUNIT_TEST_FIXTURE(Test, testVectorDeltaMovesAParagraphWithoutItsPrimitives)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"first"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"second"_ustr);
    calcLayout();
    requestFirstPage();
    const sal_uInt64 nSecondId
        = findEntry(decomposeFirstPage(), sw::vector::EntryKind::TextFrame, /*nSkip=*/1)->mnId;

    // A paragraph added at the start pushes the second paragraph down.
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->SplitNode();
    calcLayout();
    std::optional<boost::property_tree::ptree> oDelta = pushFirstPage();

    CPPUNIT_ASSERT(oDelta);
    const boost::property_tree::ptree* pSecond = objectOf(*oDelta, nSecondId);
    CPPUNIT_ASSERT(pSecond);
    CPPUNIT_ASSERT(!pSecond->get_child_optional("primitives"));
    // A new paragraph joined the page, so the delta holds the order.
    CPPUNIT_ASSERT(oDelta->get_child_optional("order"));
}

/// A paragraph that only moves, outside the area of the change, comes without its primitives.
CPPUNIT_TEST_FIXTURE(Test, testVectorDeltaMovesAParagraphOutsideTheChangedArea)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"first"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"second"_ustr);
    calcLayout();
    requestFirstPage();
    std::vector<sw::vector::Entry> aEntries = decomposeFirstPage();
    const SwRect aFirstBox = findEntry(aEntries, sw::vector::EntryKind::TextFrame)->maBox;
    const sal_uInt64 nSecondId
        = findEntry(aEntries, sw::vector::EntryKind::TextFrame, /*nSkip=*/1)->mnId;

    // The change is in the area of the first paragraph, and the second one only moves down.
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->SplitNode();
    calcLayout();
    SwRootFrame* pLayout = getSwDoc()->getIDocumentLayoutAccess().GetCurrentLayout();
    getSwTextDoc()->getVectorParts()->invalidate(*pLayout, { aFirstBox });
    tools::JsonWriter aWriter;
    getSwTextDoc()->pushVectorPrimitivesDelta(aWriter, sw::vector::partIdOf(firstPage()),
                                              sw::vector::constModePrintLayout);
    boost::property_tree::ptree aDelta = parse(aWriter.finishAndGetAsOString());

    const boost::property_tree::ptree* pSecond = objectOf(aDelta, nSecondId);
    CPPUNIT_ASSERT(pSecond);
    CPPUNIT_ASSERT(!pSecond->get_child_optional("primitives"));
}

/// A push for a page that did not change writes nothing.
CPPUNIT_TEST_FIXTURE(Test, testVectorPushWithoutChangeWritesNothing)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"text"_ustr);
    calcLayout();
    requestFirstPage();

    CPPUNIT_ASSERT(!pushFirstPage());
}

/// A request since the version the client already holds answers a delta with no objects.
CPPUNIT_TEST_FIXTURE(Test, testVectorPullSinceTheCurrentVersionIsEmpty)
{
    createSwDoc();
    getSwDocShell()->GetWrtShell()->Insert(u"text"_ustr);
    calcLayout();
    const sal_Int64 nVersion = requestFirstPage().get<sal_Int64>("version");

    boost::property_tree::ptree aDelta = requestFirstPage(nVersion);

    CPPUNIT_ASSERT_EQUAL(std::string("vectorprimitivesdelta"), aDelta.get<std::string>("type"));
    CPPUNIT_ASSERT_EQUAL(nVersion, aDelta.get<sal_Int64>("version"));
    CPPUNIT_ASSERT(aDelta.get_child("objects").empty());
}

/// The part ids list one id per page, in page order.
CPPUNIT_TEST_FIXTURE(Test, testVectorPageIdsFollowThePages)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();
    calcLayout();

    const std::vector<OString> aPartIds = getSwTextDoc()->getVectorPartIds();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aPartIds.size());
    CPPUNIT_ASSERT_EQUAL(sw::vector::partIdOf(firstPage()), aPartIds[0]);
    CPPUNIT_ASSERT(aPartIds[0] != aPartIds[1]);
}

/// Asking for the part ids does not make the store of served pages.
CPPUNIT_TEST_FIXTURE(Test, testVectorPageIdsLeaveTheStoreUnmade)
{
    // A client that draws tiles gets the page ids with every status and needs no store.
    createSwDoc();
    getSwTextDoc()->getVectorPartIds();

    CPPUNIT_ASSERT(!getSwTextDoc()->getVectorParts());
}

/// A push for a page that is gone writes the header alone, once, and the page is no longer
/// served.
CPPUNIT_TEST_FIXTURE(Test, testVectorPushForAGonePageWritesTheHeaderAlone)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();
    calcLayout();
    const SwFrame* pSecondPage = firstPage().GetNext();
    CPPUNIT_ASSERT(pSecondPage);
    CPPUNIT_ASSERT(pSecondPage->IsPageFrame());
    const OString aPartId
        = sw::vector::partIdOf(static_cast<const SwPageFrame&>(*pSecondPage));
    {
        tools::JsonWriter aWriter;
        getSwTextDoc()->getCommandValues(aWriter,
                                         Concat2View(".uno:VectorPrimitives?partid=" + aPartId));
        aWriter.finishAndGetAsOString();
    }

    pWrtShell->Undo();
    calcLayout();
    CPPUNIT_ASSERT(!firstPage().GetNext());

    // The header alone tells the readers of the page that it is gone.
    tools::JsonWriter aWriter;
    getSwTextDoc()->pushVectorPrimitivesDelta(aWriter, aPartId, sw::vector::constModePrintLayout);
    boost::property_tree::ptree aGone = parse(aWriter.finishAndGetAsOString());
    CPPUNIT_ASSERT_EQUAL(std::string("vectorprimitives"), aGone.get<std::string>("type"));
    CPPUNIT_ASSERT(!aGone.get_child_optional("objects"));
    CPPUNIT_ASSERT(!getSwTextDoc()->getVectorParts()->isServed(aPartId));

    // It is told once.
    tools::JsonWriter aAgain;
    getSwTextDoc()->pushVectorPrimitivesDelta(aAgain, aPartId, sw::vector::constModePrintLayout);
    CPPUNIT_ASSERT_EQUAL(tools::JsonWriter::EMPTY_DOCUMENT, aAgain.finishAndGetAsOString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
