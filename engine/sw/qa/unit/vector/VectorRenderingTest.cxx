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
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/graph.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <itabenum.hxx>
#include <lineinfo.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <sw_primitivetypes2d.hxx>
#include <VectorCollector.hxx>
#include <wrtsh.hxx>

#include <algorithm>
#include <cmath>

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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
