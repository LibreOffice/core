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
#include <com/sun/star/text/XTextTablesSupplier.hpp>

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/graph.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <docsh.hxx>
#include <itabenum.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <sw_primitivetypes2d.hxx>
#include <VectorCollector.hxx>
#include <wrtsh.hxx>

#include <algorithm>

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

/// The body draws the separator line between its columns, and each column is an entry.
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
    CPPUNIT_ASSERT(holds(pBody->maPrimitives, PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D));
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
