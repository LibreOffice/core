/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <tools/UnitConversion.hxx>
#include <comphelper/propertyvalue.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <fmtpdsc.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentSettingAccess.hxx>
#include <itabenum.hxx>
#include <frmmgr.hxx>
#include <formatflysplit.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>

using namespace css;

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format")
    {
    }
};

DECLARE_RTFEXPORT_TEST(testTdf158586_0, "tdf158586_pageBreak0.rtf")
{
    // The specified page break must be lost because it is in a text frame
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // There should be no empty carriage return at the start of the second page
    // const auto& pLayout = parseLayoutDump();
    // assertXPathContent(pLayout, "//page[1]/body/txt"_ostr, "First page");}
}

DECLARE_RTFEXPORT_TEST(testTdf158586_0B, "tdf158586_pageBreak0B.rtf")
{
    // The specified page break must be lost because it is in a text frame
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFEXPORT_TEST(testTdf158586_1, "tdf158586_pageBreak1.rtf")
{
    // None of the specified text frame settings initiates a real text frame - page break not lost
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

    // There should be no empty carriage return at the start of the second page
    const auto& pLayout = parseLayoutDump();
    // on import there is a section on page 2; on reimport there is no section
    // (probably not an important difference?)
    assertXPathContent(pLayout, "/root/page[2]/body//txt"_ostr, "Second page");
}

DECLARE_RTFEXPORT_TEST(testTdf158586_lostFrame, "tdf158586_lostFrame.rtf")
{
    // The anchor and align properties are sufficient to define a frame
    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "//anchored"_ostr, 1);
    assertXPathContent(pLayout, "//page[1]/body//txt"_ostr, "1st page");
    assertXPathContent(pLayout, "//page[2]/body//txt"_ostr, "2nd page");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testTdf158826_extraCR, "tdf158826_extraCR.rtf")
{
    // Note: this is a hand-minimized sample, and very likely doesn't follow RTF { } rules...

    // The page break defined before the document content should not cause a page break
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // There is a two-column floating table [that SHOULD be getParagraphOrTable(1)]
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY_THROW);
}

DECLARE_RTFEXPORT_TEST(testTdf158830, "tdf158830.rtf")
{
    //check centered text in table
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_CENTER,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf158978, "tdf158978.rtf")
{
    //check right alignment in table1 or table3
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_RIGHT,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf158982, "tdf158982.rtf")
{
    //check table count == 6
    // check left margin in a cell of the last table
    //
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(5), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A2"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(508),
                         getProperty<sal_Int32>(xPara, "ParaLeftMargin"));
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
