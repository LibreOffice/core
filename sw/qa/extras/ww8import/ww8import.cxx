/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <ndtxt.hxx>
#include <viscrs.hxx>
#include <wrtsh.hxx>

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
    sal_Int32 tableLeft = parseDump("/root/page[2]/body/column/body/section/column/body/txt[2]/anchored/fly/tab/infos/bounds", "left").toInt32();
    sal_Int32 tableWidth = parseDump("/root/page[2]/body/column/body/section/column/body/txt[2]/anchored/fly/tab/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT( pageWidth > 0 );
    CPPUNIT_ASSERT( tableWidth > 0 );
    // The table's resulting position should be roughly centered.
    CPPUNIT_ASSERT( abs(( pageLeft + pageWidth / 2 ) - ( tableLeft + tableWidth / 2 )) < 20 );
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
    SwDoc*           pDoc         = pTextDoc->GetDocShell()->GetDoc();
    SwWrtShell*      pWrtShell    = pDoc->GetDocShell()->GetWrtShell();
    SwShellCursor*   pShellCursor = pWrtShell->getShellCursor( false );
    SwNodeIndex      aIdx         = SwNodeIndex( pShellCursor->Start()->nNode );

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

// tests should only be added to ww8IMPORT *if* they fail round-tripping in ww8EXPORT

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
