/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <ndtxt.hxx>
#include <viscrs.hxx>
#include <wrtsh.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ww8import/data/", "MS Word 97")
    {
    }
};

#define DECLARE_WW8IMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, nullptr, Test)

DECLARE_WW8IMPORT_TEST(testFloatingTableSectionMargins, "floating-table-section-margins.doc")
{
    sal_Int32 pageLeft = parseDump("/root/page[2]/infos/bounds", "left").toInt32();
    sal_Int32 pageWidth = parseDump("/root/page[2]/infos/bounds", "width").toInt32();
    sal_Int32 tableLeft = parseDump("//tab/infos/bounds", "left").toInt32();
    sal_Int32 tableWidth = parseDump("//tab/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT( pageWidth > 0 );
    CPPUNIT_ASSERT( tableWidth > 0 );
    // The table's resulting position should be roughly centered.
    CPPUNIT_ASSERT( abs(( pageLeft + pageWidth / 2 ) - ( tableLeft + tableWidth / 2 )) < 20 );

    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    OUString pageStyleName = getProperty<OUString>(getParagraph(2), "PageStyleName");
    uno::Reference<style::XStyle> pageStyle( getStyles("PageStyles")->getByName(pageStyleName), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(pageStyleName), uno::UNO_QUERY);
    uno::Reference<text::XTextColumns> xPageColumns = getProperty< uno::Reference<text::XTextColumns> >(xPageStyle, "TextColumns");

    //either one or the other should get the column's, not both.
    CPPUNIT_ASSERT( xTextColumns->getColumnCount() != xPageColumns->getColumnCount());
}

DECLARE_WW8IMPORT_TEST(testN816593, "n816593.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // Make sure that even if we import the two tables as non-floating, we
    // still consider them different, and not merge them.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

DECLARE_WW8IMPORT_TEST(testBnc875715, "bnc875715.doc")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // Was incorrectly set as -1270.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xSections->getByIndex(0), "SectionLeftMargin"));
}
DECLARE_WW8IMPORT_TEST(testFloatingTableSectionColumns, "floating-table-section-columns.doc")
{
    OUString tableWidth = parseDump("/root/page[1]/body/section/column[2]/body/txt/anchored/fly/tab/infos/bounds", "width");
    // table width was restricted by a column
    CPPUNIT_ASSERT( tableWidth.toInt32() > 10000 );
}

DECLARE_WW8IMPORT_TEST(testTdf107773, "tdf107773.doc")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1, multi-page table was imported as a floating one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

DECLARE_WW8IMPORT_TEST(testTdf106291, "tdf106291.doc")
{
    // Table cell was merged vertically instead of horizontally -> had incorrect dimensions
    OUString cellWidth = parseDump("/root/page[1]/body/tab/row/cell[1]/infos/bounds", "width");
    OUString cellHeight = parseDump("/root/page[1]/body/tab/row/cell[1]/infos/bounds", "height");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8660), cellWidth.toInt32());
    CPPUNIT_ASSERT(cellHeight.toInt32() > 200); // height might depend on font size
}

DECLARE_WW8IMPORT_TEST( testTdf105570, "tdf105570.doc" )
{
    /*****
      * MS-DOC specification ( https://msdn.microsoft.com/en-us/library/cc313153 )
      * ch. 2.6.3, sprmTTableHeader:
      *     A Bool8 value that specifies that the current table row is a header row.
      *     If the value is 0x01 but sprmTTableHeader is not applied with a value of 0x01
      *     for a previous row in the same table, then this property MUST be ignored.
      *
      * The document have three tables with three rows.
      * Table 1 has { 1, 0, 0 } values of the "repeat as header row" property for each row
      * Table 2 has { 1, 1, 0 }
      * Table 3 has { 0, 1, 1 }
      ****/
    SwXTextDocument* pTextDoc     = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc*           pDoc         = pTextDoc->GetDocShell()->GetDoc();
    SwWrtShell*      pWrtShell    = pDoc->GetDocShell()->GetWrtShell();
    SwShellCursor*   pShellCursor = pWrtShell->getShellCursor( false );
    SwNodeIndex      aIdx         = pShellCursor->Start()->nNode;

    // Find first table
    SwTableNode*     pTableNd     = aIdx.GetNode().FindTableNode();

    CPPUNIT_ASSERT_EQUAL( sal_uInt16(1), pTableNd->GetTable().GetRowsToRepeat() );

    // Go to next table
    aIdx.Assign( *pTableNd->EndOfSectionNode(), 1 );
    while ( nullptr == (pTableNd = aIdx.GetNode().GetTableNode()) ) ++aIdx;

    CPPUNIT_ASSERT_EQUAL( sal_uInt16(2), pTableNd->GetTable().GetRowsToRepeat() );

    // Go to next table
    aIdx.Assign( *pTableNd->EndOfSectionNode(), 1 );
    while ( nullptr == (pTableNd = aIdx.GetNode().GetTableNode()) ) ++aIdx;

    // As first row hasn't sprmTTableHeader set, all following must be ignored, so no rows must be repeated
    CPPUNIT_ASSERT_EQUAL( sal_uInt16(0), pTableNd->GetTable().GetRowsToRepeat() );
}

DECLARE_OOXMLIMPORT_TEST(testImageLazyRead, "image-lazy-read.doc")
{
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    Graphic aGraphic(xGraphic);
    // This failed, import loaded the graphic, it wasn't lazy-read.
    CPPUNIT_ASSERT(!aGraphic.isAvailable());
}

DECLARE_WW8IMPORT_TEST(testTdf106799, "tdf106799.doc")
{
    sal_Int32 const nCellWidths[3][4] = { { 9530, 0, 0, 0 },{ 2382, 2382, 2382, 2384 },{ 2382, 2382, 2382, 2384 } };
    sal_Int32 const nCellTxtLns[3][4] = { { 1, 0, 0, 0 },{ 1, 0, 0, 0},{ 1, 1, 1, 1 } };
    // Table was distorted because of missing sprmPFInnerTableCell at paragraph marks (0x0D) with sprmPFInnerTtp
    for (sal_Int32 nRow : { 0, 1, 2 })
        for (sal_Int32 nCell : { 0, 1, 2, 3 })
        {
            OString cellXPath("/root/page/body/tab/row/cell/tab/row[" + OString::number(nRow+1) + "]/cell[" + OString::number(nCell+1) + "]/");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(cellXPath.getStr(), nCellWidths[nRow][nCell], parseDump(cellXPath + "infos/bounds", "width").toInt32());
            if (nCellTxtLns[nRow][nCell] != 0)
                CPPUNIT_ASSERT_EQUAL_MESSAGE(cellXPath.getStr(), nCellTxtLns[nRow][nCell], parseDump(cellXPath + "txt/Text", "nLength").toInt32());
        }
}

DECLARE_WW8IMPORT_TEST(testTdf112346, "tdf112346.doc")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1, multi-page table was imported as a floating one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

DECLARE_WW8IMPORT_TEST(testTdf121734, "tdf121734.doc")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwPosFlyFrames aPosFlyFrames = pDoc->GetAllFlyFormats(nullptr, false);
    // There is only one fly frame in the document: the one with the imported floating table
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPosFlyFrames.size());
    for (const auto& rPosFlyFrame : aPosFlyFrames)
    {
        const SwFrameFormat& rFormat = rPosFlyFrame->GetFormat();
        const SfxPoolItem* pItem = nullptr;

        // The LR and UL spacings and borders must all be set explicitly;
        // spacings and border distances must be 0; borders must be absent.

        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, rFormat.GetItemState(RES_LR_SPACE, false, &pItem));
        auto pLR = static_cast<const SvxLRSpaceItem*>(pItem);
        CPPUNIT_ASSERT(pLR);
        CPPUNIT_ASSERT_EQUAL(long(0), pLR->GetLeft());
        CPPUNIT_ASSERT_EQUAL(long(0), pLR->GetRight());

        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, rFormat.GetItemState(RES_UL_SPACE, false, &pItem));
        auto pUL = static_cast<const SvxULSpaceItem*>(pItem);
        CPPUNIT_ASSERT(pUL);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pUL->GetUpper());
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pUL->GetLower());

        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, rFormat.GetItemState(RES_BOX, false, &pItem));
        auto pBox = static_cast<const SvxBoxItem*>(pItem);
        CPPUNIT_ASSERT(pBox);
        for (auto eLine : { SvxBoxItemLine::TOP, SvxBoxItemLine::BOTTOM,
                            SvxBoxItemLine::LEFT, SvxBoxItemLine::RIGHT })
        {
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pBox->GetDistance(eLine));
            CPPUNIT_ASSERT(!pBox->GetLine(eLine));
        }
    }
}

DECLARE_WW8IMPORT_TEST(testTdf122425_1, "tdf122425_1.doc")
{
    // This is for header text in case we use a hack for fixed-height headers
    // (see SwWW8ImplReader::Read_HdFtTextAsHackedFrame)
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwPosFlyFrames aPosFlyFrames = pDoc->GetAllFlyFormats(nullptr, false);
    // There are two fly frames in the document: for first page's header, and for other pages'
    CPPUNIT_ASSERT_EQUAL(size_t(2), aPosFlyFrames.size());
    for (const auto& rPosFlyFrame : aPosFlyFrames)
    {
        const SwFrameFormat& rFormat = rPosFlyFrame->GetFormat();
        const SfxPoolItem* pItem = nullptr;

        // The LR and UL spacings and borders must all be set explicitly;
        // spacings and border distances must be 0; borders must be absent

        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, rFormat.GetItemState(RES_LR_SPACE, false, &pItem));
        auto pLR = static_cast<const SvxLRSpaceItem*>(pItem);
        CPPUNIT_ASSERT(pLR);
        CPPUNIT_ASSERT_EQUAL(long(0), pLR->GetLeft());
        CPPUNIT_ASSERT_EQUAL(long(0), pLR->GetRight());

        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, rFormat.GetItemState(RES_UL_SPACE, false, &pItem));
        auto pUL = static_cast<const SvxULSpaceItem*>(pItem);
        CPPUNIT_ASSERT(pUL);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pUL->GetUpper());
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pUL->GetLower());

        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, rFormat.GetItemState(RES_BOX, false, &pItem));
        auto pBox = static_cast<const SvxBoxItem*>(pItem);
        CPPUNIT_ASSERT(pBox);
        for (auto eLine : { SvxBoxItemLine::TOP, SvxBoxItemLine::BOTTOM,
                            SvxBoxItemLine::LEFT, SvxBoxItemLine::RIGHT })
        {
            CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pBox->GetDistance(eLine));
            CPPUNIT_ASSERT(!pBox->GetLine(eLine));
        }
    }
}

DECLARE_WW8IMPORT_TEST(testTdf122425_2, "tdf122425_2.doc")
{
    // This is for graphic objects in headers/footers
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwPosFlyFrames aPosFlyFrames = pDoc->GetAllFlyFormats(nullptr, false);
    // There is one fly frame in the document: the text box
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPosFlyFrames.size());
    for (const auto& rPosFlyFrame : aPosFlyFrames)
    {
        const SwFrameFormat& rFormat = rPosFlyFrame->GetFormat();
        const SfxPoolItem* pItem = nullptr;

        // Check for correct explicitly-set values of UL spacings. Previously this was "DEFAULT",
        // and resulted in inherited values (114 = 2 mm) used.
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, rFormat.GetItemState(RES_UL_SPACE, false, &pItem));
        auto pUL = static_cast<const SvxULSpaceItem*>(pItem);
        CPPUNIT_ASSERT(pUL);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pUL->GetUpper());
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pUL->GetLower());
    }
}

DECLARE_WW8IMPORT_TEST(testTdf110987, "tdf110987")
{
    // The input document is an empty .doc, but without file name
    // extension. Check that it was loaded as a normal .doc document,
    // and not a template.
    SwXTextDocument* pTextDoc     = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    OUString sFilterName = pTextDoc->GetDocShell()->GetMedium()->GetFilter()->GetFilterName();
    CPPUNIT_ASSERT(sFilterName != "MS Word 97 Vorlage");
}

// tests should only be added to ww8IMPORT *if* they fail round-tripping in ww8EXPORT

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
