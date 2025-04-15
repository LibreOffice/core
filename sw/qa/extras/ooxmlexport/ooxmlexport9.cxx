/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <config_fonts.h>

#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <comphelper/processfactory.hxx>

#include <docsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

class DocmTest : public SwModelTestBase
{
public:
    DocmTest()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"MS Word 2007 XML VBA"_ustr)
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testFdo55381, "fdo55381.docx")
{
    CPPUNIT_ASSERT_EQUAL(4, getPages());
    //TODO: frames not located on the correct pages
}

CPPUNIT_TEST_FIXTURE(Test, testDocm)
{
    loadAndSave("hello.docm");
    // Make sure that we check the name of the export filter.
    // This was application/vnd.ms-word.document.macroEnabled.main+xml when the
    // name of the import filter was checked.
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@PartName='/word/document.xml']",
                "ContentType",
                u"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml");
}

CPPUNIT_TEST_FIXTURE(Test, testDefaultContentTypes)
{
    loadAndSave("fdo55381.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Default[@Extension='xml']",
                "ContentType",
                u"application/xml");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Default[@Extension='rels']",
                "ContentType",
                u"application/vnd.openxmlformats-package.relationships+xml");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Default[@Extension='png']",
                "ContentType",
                u"image/png");

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Default[@Extension='jpeg']",
                "ContentType",
                u"image/jpeg");
}

CPPUNIT_TEST_FIXTURE(DocmTest, testDocmSave)
{
    // This was
    // application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml,
    // we used the wrong content type for .docm files.
    loadAndSave("hello.docm");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Override[@PartName='/word/document.xml']",
                "ContentType",
                u"application/vnd.ms-word.document.macroEnabled.main+xml");
}

CPPUNIT_TEST_FIXTURE(DocmTest, testBadDocm)
{
    createSwDoc("bad.docm");
    // This was 'MS Word 2007 XML', broken docm files were not recognized.
    CPPUNIT_ASSERT_EQUAL(u"MS Word 2007 XML VBA"_ustr, getSwDocShell()->GetMedium()->GetFilter()->GetName());

    saveAndReload(mpFilter);
    // This was 'MS Word 2007 XML', broken docm files were not recognized.
    CPPUNIT_ASSERT_EQUAL(u"MS Word 2007 XML VBA"_ustr, getSwDocShell()->GetMedium()->GetFilter()->GetName());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf109063)
{
    auto verify = [this]() {
        // A near-page-width table should be allowed to split:
        uno::Reference<text::XTextFramesSupplier> xDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFrame(xDocument->getTextFrames()->getByName(u"Frame1"_ustr),
                                                   uno::UNO_QUERY);
        bool bIsSplitAllowed{};
        xFrame->getPropertyValue(u"IsSplitAllowed"_ustr) >>= bIsSplitAllowed;
        CPPUNIT_ASSERT(bIsSplitAllowed);
    };
    createSwDoc("tdf109063.docx");
    verify();
    saveAndReload(u"Office Open XML Text"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(DocmTest, testTdf108269)
{
    loadAndReload("tdf108269.docm");
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    // This failed: VBA streams were not roundtripped via the doc-level
    // grab-bag.
    CPPUNIT_ASSERT(xNameAccess->hasByName(u"word/vbaProject.bin"_ustr));
    CPPUNIT_ASSERT(xNameAccess->hasByName(u"word/vbaData.xml"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125338)
{
    loadAndSave("tdf125338.docm");
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    // docm files should not retain macros when saved as docx
    CPPUNIT_ASSERT(!xNameAccess->hasByName(u"word/vbaProject.bin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf92045, "tdf92045.docx")
{
    // This was true, <w:effect w:val="none"/> resulted in setting the blinking font effect.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getRun(getParagraph(1), 1), u"CharFlash"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf95031, "tdf95031.docx")
{
    // This was 494, in-numbering paragraph's automating spacing was handled as visible spacing, while it should not.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(2), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(3), u"ParaTopMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106690, "tdf106690.docx")
{
    // This was 0, numbering rules with automatic spacing meant 0
    // before/autospacing for all text nodes, even for ones at the start/end of
    // a numbered text node block.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(2), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(2), u"ParaTopMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106690Cell, "tdf106690-cell.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    // This was 0, bottom margin of the second paragraph in the A1 table cell
    // had a reduced auto-space, just because of a next paragraph in the A2
    // cell.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaBottomMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf122342, "tdf122342.docx")
{
    // These were 494, style based numbering rules with automatic spacing meant 0
    // before/autospacing for all text nodes, even for ones at the start/end of
    // a numbered text node block.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(1), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(2), u"ParaBottomMargin"_ustr));
    // last list item
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(3), u"ParaBottomMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132802)
{
    loadAndSave("tdf132802.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "after", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "after", u"0");
    // This was 0 (list auto spacing is not zero before tables)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:spacing", "after", u"280");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[1]/w:pPr/w:spacing", "after", u"0");
    // This was 0 (list auto spacing is not zero at the end of table cells)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[2]/w:pPr/w:spacing", "after", u"280");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p[1]/w:pPr/w:spacing", "after", u"280");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p[1]/w:pPr/w:spacing", "after", u"280");
    // This was 0 (list auto spacing is not zero at list end)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing", "after", u"280");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132807)
{
    loadAndSave("tdf132807.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "before", u"280");
    // This was 240 (list auto spacing is zero in lists)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:spacing", "before", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing", "before", u"0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[1]/w:pPr/w:spacing", "before", u"0");
    // This was 240 (list auto spacing is zero in lists)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[2]/w:pPr/w:spacing", "before", u"0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p[1]/w:pPr/w:spacing", "before", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p[2]/w:pPr/w:spacing", "before", u"280");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p[1]/w:pPr/w:spacing", "before", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:spacing", "before", u"280");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133052)
{
    loadAndSave("tdf133052.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // These were 240 (top auto spacing of list subitems are zero)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing", "before", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:spacing", "before", u"0");
    // in tables, too
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[2]/w:pPr/w:spacing", "before", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[3]/w:pPr/w:spacing", "before", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[4]/w:pPr/w:spacing", "before", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[5]/w:pPr/w:spacing", "before", u"0");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134648)
{
    loadAndSave("tdf134648.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // list item with direct top auto spacing
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "after", u"240");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "beforeAutospacing", u"1");

    // This was spacing w:after=200, but bottom auto spacing of first list subitem is zero
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf129575_directBefore, "tdf129575-directBefore.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    // direct paragraph formatting
    // This was 212 twips from the table style, but always direct paragraph formatting wins, in the case of the default 0 margin, too
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaTopMargin"_ustr));
    // default margin
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf129575_directAfter, "tdf129575-directAfter.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    // from table style
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(212), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaTopMargin"_ustr));
    // direct paragraph formatting
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf129575_styleAfter, "tdf129575-styleAfter.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    // direct paragraph formatting
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaTopMargin"_ustr));
    // from table style
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(212), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf129575_docDefault, "tdf129575-docDefault.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    // docDefault defines both bottom margin and line spacing, but
    // applied bottom margin values are based on non-docDefault paragraph styles, line spacing is based on table style

    // docDefault: <w:spacing w:after="160" w:line="320" w:lineRule="auto"/>
    // table style: <w:spacing w:after="0" w:line="240" w:lineRule="auto"/> (single line space, overwriting bigger docDefault)

    // Paragraph style Normal: <w:spacing w:after="160"/> (same as docDefault),
    // table style based single line spacing
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(282), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));
    style::LineSpacing aLineSpacing = getProperty<style::LineSpacing>(getParagraphOfText(1, xCell->getText()), u"ParaLineSpacing"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aLineSpacing.Height);
    // Heading 2: <w:spacing w:after="360"/> (different from docDefault),
    // table style based single line spacing
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(635), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaBottomMargin"_ustr));
    aLineSpacing = getProperty<style::LineSpacing>(getParagraphOfText(1, xCell->getText()), u"ParaLineSpacing"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aLineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aLineSpacing.Height);

}

CPPUNIT_TEST_FIXTURE(Test, testTdf118812)
{
    loadAndSave("tdf118812_tableStyles-comprehensive.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // cell A1
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:pPr/w:pStyle", "val", u"Normal");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:pPr/w:spacing", "lineRule");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:pPr/w:spacing", "line");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:pPr/w:spacing", "before", u"480");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:pPr/w:spacing", "after", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:r[1]/w:rPr/w:color", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:r[1]/w:rPr/w:sz", "val", u"16");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:r[2]/w:rPr/w:rStyle", "val", u"CharSubStyleDefaults");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:r[2]/w:rPr/w:color", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:r[2]/w:rPr/w:sz", "val", u"16");
    // cell A2
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:pPr/w:pStyle", "val", u"Normal");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:pPr/w:spacing", "lineRule");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:pPr/w:spacing", "line");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:pPr/w:spacing", "before", u"480");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:pPr/w:spacing", "after", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[1]/w:rPr/w:color", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[1]/w:rPr/w:sz", "val", u"16");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[2]/w:rPr/w:rStyle", "val", u"ParaSubStyleDefaultsChar");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[2]/w:rPr/w:color", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[2]/w:rPr/w:sz", "val", u"16");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[3]/w:rPr/w:rStyle", "val", u"CharSubStyleNormal");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[3]/w:rPr/w:color", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[3]/w:rPr/w:sz", "val", u"16");
    // cell A3
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:pPr/w:pStyle", "val", u"ParaSubStyleNormal");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:pPr/w:spacing", "lineRule");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:pPr/w:spacing", "line");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:pPr/w:spacing", "before", u"480");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:pPr/w:spacing", "after", u"280");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[1]/w:rPr/w:color", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[1]/w:rPr/w:sz", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[2]/w:rPr/w:rStyle", "val", u"CharSubStyleNormal");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[2]/w:rPr/w:color", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[2]/w:rPr/w:sz", 0);
    // cell A4
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:pPr/w:pStyle", "val", u"ParaSubStyleDefaults");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:pPr/w:spacing", "lineRule");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:pPr/w:spacing", "line");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:pPr/w:spacing", "before", u"480");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:pPr/w:spacing", "after", u"200");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[1]/w:rPr/w:color", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[1]/w:rPr/w:sz", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[2]/w:rPr/w:rStyle", "val", u"CharSubStyleDefaults");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[2]/w:rPr/w:color", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[2]/w:rPr/w:sz", 0);
    // cell A5
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:pStyle", "val", u"Normal");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:spacing", "lineRule");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:spacing", "line");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:rPr", "color");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:rPr", "sz");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:spacing", "before", u"480");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:spacing", "after", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:r[1]/w:rPr/w:color", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:r[1]/w:rPr/w:color", "val", u"AAAA00"); // all text in color
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:r[1]/w:rPr/w:sz", "val", u"16");
    // cell A6
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:pStyle", "val", u"Normal");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:spacing", "lineRule");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:spacing", "line");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:rPr", "color");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:rPr/w:sz", "val", u"16");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:spacing", "before", u"480");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:spacing", "after", u"20");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:r[1]/w:rPr/w:color", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:r[1]/w:rPr/w:sz", "val", u"16");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:r[2]/w:rPr/w:color", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:r[2]/w:rPr/w:color", "val", u"AAAA00");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:r[2]/w:rPr/w:sz", "val", u"16");

    // tdf#131070 keep paragraph style based right indentation with indentation of direct numbering
    // cell A7 - This was <w:ind w:start="1440" w:hanging="0"/>
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[7]/w:tc/w:p/w:pPr/w:ind", 0);
    // cell A8
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[8]/w:tc/w:p/w:pPr/w:ind", "start", u"714");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[8]/w:tc/w:p/w:pPr/w:ind", "end", u"1701");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[8]/w:tc/w:p/w:pPr/w:ind", "hanging", u"357");
    // cell A9
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[9]/w:tc/w:p/w:pPr/w:ind", "end", u"1440");
    // This was 1440
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[9]/w:tc/w:p/w:pPr/w:ind", "start", u"720");
    // This was 0
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[9]/w:tc/w:p/w:pPr/w:ind", "hanging", u"360");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107626)
{
    loadAndSave("tdf107626.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 2 (missing trailing cell in merged cell range)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc", 3);
}

DECLARE_OOXMLEXPORT_TEST(testTdf106970, "tdf106970.docx")
{
    // The second paragraph (first numbered one) had 0 bottom margin:
    // autospacing was even collapsed between different numbering styles.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(2), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(3), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(4), u"ParaBottomMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf79272_strictDxa)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4318), getProperty<sal_Int32>(xTables->getByIndex(0), u"Width"_ustr));
    };

    createSwDoc("tdf79272_strictDxa.docx");
    verify();
    saveAndReload(mpFilter);
    verify();

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    // Validation test: order of elements was wrong. Order was: insideH, end, insideV.
    int nEnd = getXPathPosition(pXmlDoc, "/w:styles/w:style[@w:styleId='TableGrid']/w:tblPr/w:tblBorders", "end");
    int nInsideH = getXPathPosition(pXmlDoc, "/w:styles/w:style[@w:styleId='TableGrid']/w:tblPr/w:tblBorders", "insideH");
    int nInsideV = getXPathPosition(pXmlDoc, "/w:styles/w:style[@w:styleId='TableGrid']/w:tblPr/w:tblBorders", "insideV");
    CPPUNIT_ASSERT(nEnd < nInsideH);
    CPPUNIT_ASSERT(nInsideH < nInsideV);
}

DECLARE_OOXMLEXPORT_TEST(testTdf109306, "tdf109306.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // Both types of relative width specification (pct): simple integers (in fiftieths of percent)
    // and floats with "%" unit specification must be treated correctly
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTables->getByIndex(0), u"IsWidthRelative"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(9), getProperty<sal_Int16>(xTables->getByIndex(0), u"RelativeWidth"_ustr));

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTables->getByIndex(1), u"IsWidthRelative"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(80), getProperty<sal_Int16>(xTables->getByIndex(1), u"RelativeWidth"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testKern, "kern.docx")
{
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), u"CharAutoKerning"_ustr));
    // This failed: kerning was also enabled for the second paragraph.
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(2), 1), u"CharAutoKerning"_ustr));

    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    //tdf107801: kerning normally isn't enabled by default for .docx
    CPPUNIT_ASSERT_EQUAL_MESSAGE("AutoKern should be false", false, getProperty<bool>(xStyle, u"CharAutoKerning"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf89377, "tdf89377_tableWithBreakBeforeParaStyle.docx")
{
    // the paragraph style should set table's text-flow break-before-page
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );

    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    //tdf107801: kerning info wasn't exported previously.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("AutoKern should be true", true, getProperty<bool>(xStyle, u"CharAutoKerning"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104420, "tdf104420_lostParagraph.docx")
{
    // the add/remove dummy paragraph was losing an entire header and paragraph
    CPPUNIT_ASSERT_EQUAL( 2, getPages() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf41542_borderlessPadding)
{
    loadAndReload("tdf41542_borderlessPadding.odt");
    // the page style's borderless padding should force this to 3 pages, not 1
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}

#if HAVE_MORE_FONTS
DECLARE_OOXMLEXPORT_TEST(tdf105490_negativeMargins, "tdf105490_negativeMargins.docx")
{
    // negative margins should change to minimal margins, not default margins.
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testTdf97648_relativeWidth)
{
    auto verify = [this](bool bIsExport = false) {
        CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(7616), getShape(1)->getSize().Width, 10);
        CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(8001), getShape(2)->getSize().Width, 10);
        CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(4001), getShape(3)->getSize().Width, 10);
        CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_LEFT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(6), u"ParaAdjust"_ustr)) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(1600), getShape(4)->getSize().Width, 10);
        CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_RIGHT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(8), u"ParaAdjust"_ustr)) );


        CPPUNIT_ASSERT_EQUAL( sal_Int32(0), getProperty<sal_Int32>(getShape(1), u"LeftMargin"_ustr) );
        if (!bIsExport)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Text should wrap above/below the line", text::WrapTextMode_NONE, getProperty<text::WrapTextMode>(getShape(1), u"Surround"_ustr));
            CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(getShape(2), u"HoriOrient"_ustr));
            CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT, getProperty<sal_Int16>(getShape(3), u"HoriOrient"_ustr));
            CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT, getProperty<sal_Int16>(getShape(4), u"HoriOrient"_ustr));
        }

        uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xSections->getCount());

        uno::Reference<beans::XPropertySet> xTextSection(xSections->getByIndex(2), uno::UNO_QUERY);
        uno::Reference<text::XTextColumns> xTextColumns
            = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());
    };

    createSwDoc("tdf97648_relativeWidth.docx");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

DECLARE_OOXMLEXPORT_TEST(testTdf144362, "tdf144362.odt")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);

    // This is difference OK: tdf#107837 extra section added on export to preserve balanced columns.
    CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(2), xSections->getCount());

    uno::Reference<beans::XPropertySet> xTextSection(xSections->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf104061_tableSectionColumns,"tdf104061_tableSectionColumns.docx")
{
    CPPUNIT_ASSERT_MESSAGE("There should be two or three pages", getPages() <= 3 );

    //tdf#95114 - follow style is Text Body - DOCX test
    uno::Reference< beans::XPropertySet > properties(getStyles(u"ParagraphStyles"_ustr)->getByName(u"annotation subject"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Marginalia"_ustr, getProperty<OUString>(properties, u"FollowStyle"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf46940_dontEquallyDistributeColumns, "tdf46940_dontEquallyDistributeColumns.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTextSections->getByIndex(0), u"DontBalanceTextColumns"_ustr));
    // This was false, columns before a section-page-break were balanced.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(2), u"DontBalanceTextColumns"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(3), u"DontBalanceTextColumns"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf98700_keepWithNext)
{
    loadAndReload("tdf98700_keepWithNext.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Heading style keeps with next", true, getProperty<bool>(getParagraph(1), u"ParaKeepTogether"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default style doesn't keep with next", false, getProperty<bool>(getParagraph(2), u"ParaKeepTogether"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Heading 1 style inherits keeps with next", true, getProperty<bool>(getParagraph(3), u"ParaKeepTogether"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Heading 2 style disabled keep with next", false, getProperty<bool>(getParagraph(4), u"ParaKeepTogether"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Text Body style toggled off keep with next", false, getProperty<bool>(getParagraph(5), u"ParaKeepTogether"_ustr));

    //tdf#95114 - follow style is Text Body - ODT test
    uno::Reference< beans::XPropertySet > properties(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Heading 1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text body"_ustr, getProperty<OUString>(properties, u"FollowStyle"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testRhbz988516, "rhbz988516.docx")
{
    // The problem was that the list properties of the footer leaked into body
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(1), u"NumberingStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Enclosure 3"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(3), u"NumberingStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(4), u"NumberingStyleName"_ustr));

    // tdf#103975 The problem was that an empty paragraph with page break info was removed.
    CPPUNIT_ASSERT_EQUAL( 2, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakB, "tdf103975_notPageBreakB.docx")
{
    // turn on View Formatting Marks to see these documents.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(3), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(4), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), u"BreakType"_ustr));
    CPPUNIT_ASSERT_EQUAL( 4, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakC, "tdf103975_notPageBreakC.docx")
{
    // turn on View Formatting Marks to see these documents.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(3), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(4), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), u"BreakType"_ustr));
    CPPUNIT_ASSERT_EQUAL( 4, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakD, "tdf103975_notPageBreakD.docx")
{
    // The problem was that the column break was moving outside of the columns, making a page break.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), u"BreakType"_ustr));
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakE, "tdf103975_notPageBreakE.docx")
{
    // The problem was that the column break was getting lost.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), u"BreakType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf112352_nextPageColumns, "tdf112352_nextPageColumns.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), u"TextSection"_ustr);
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(3), u"TextSection"_ustr);
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf109310_endnoteStyleForMSO)
{
    loadAndSave("tdf109310_endnoteStyleForMSO.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/endnotes.xml"_ustr);
    // It is important that EndnoteCharacters exists, and is not duplicated on each round-trip
    assertXPath(pXmlDoc, "/w:endnotes/w:endnote[@w:id='2']/w:p/w:r[1]/w:rPr/w:rStyle", "val",
                u"EndnoteCharacters");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103389)
{
    loadAndSave("tdf103389.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // No geometry was exported for the second canvas
    // Check both canvases' geometry
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp/wps:spPr/a:prstGeom", "prst", u"rect");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp/wps:spPr/a:prstGeom", "prst", u"rect");
}

DECLARE_OOXMLEXPORT_TEST(testTdf84678, "tdf84678.docx")
{
    // This was 0, left margin inside a shape+text wasn't imported from DOCX.
    // 360000 EMU, but layout uses twips.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(567), getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "left").toInt32());

}

DECLARE_OOXMLEXPORT_TEST(testTdf103544, "tdf103544.docx")
{
    // We have two shapes: a frame and an image
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    // Image was lost because of the frame export
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, u"Graphic"_ustr);
    CPPUNIT_ASSERT(xGraphic.is());

    // tdf#143899: framePr textbox not affected by compat15's aversion to vertical page margin
    // The textbox requests to be 0.72inch above its anchor paragraph (the first paragraph)
    xmlDocUniquePtr pDump = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"), getXPathContent(pDump, "//fly[2]/txt"));
    sal_Int32 nShapeBottom = getXPath(pDump, "//fly[2]/infos/bounds", "bottom").toInt32();
    sal_Int32 nBodyTop = getXPath(pDump, "//page/body/infos/bounds", "top").toInt32();
    // The framePr textbox is NOT vertically limited like other shapes to the page margins
    CPPUNIT_ASSERT(nBodyTop > nShapeBottom); // textbox is fully above the text body area
}

DECLARE_OOXMLEXPORT_TEST(testTdf103573, "tdf103573.docx")
{
    // Relative positions to the left or right margin (MS Word naming) was not handled.
    uno::Reference<beans::XPropertySet> xShapeProperties( getShape(1), uno::UNO_QUERY );
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue(u"HoriOrient"_ustr) >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally", text::HoriOrientation::CENTER, nValue);
    xShapeProperties->getPropertyValue(u"HoriOrientRelation"_ustr) >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally relatively to left page border", text::RelOrientation::PAGE_LEFT, nValue);

    xShapeProperties.set( getShape(2), uno::UNO_QUERY );
    xShapeProperties->getPropertyValue(u"HoriOrient"_ustr) >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally", text::HoriOrientation::CENTER, nValue);
    xShapeProperties->getPropertyValue(u"HoriOrientRelation"_ustr) >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally relatively to right page border", text::RelOrientation::PAGE_RIGHT, nValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf106132, "tdf106132.docx")
{
    uno::Reference<beans::XPropertySet> xShape(getShapeByName(u"Frame1"), uno::UNO_QUERY);
    // This was 250, <wps:bodyPr ... rIns="0" ...> was ignored for an outer shape.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(xShape, u"TextRightDistance"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf79329)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
        // This was 1: only the inner, not the outer table was created.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xTables->getCount());
    };
    createSwDoc("tdf79329.docx");
    verify();
    saveAndReload(u"Office Open XML Text"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103982)
{
    loadAndReload("tdf103982.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    sal_Int32 nDistB = getXPath(pXmlDoc, "//wp:anchor", "distB").toInt32();
    // This was -260350, which is not a valid value for an unsigned type.
    CPPUNIT_ASSERT(nDistB >= 0);

    // tdf#115670 the shadow should not be enabled (no on="t")
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(!getProperty<bool>(xPropertySet, u"Shadow"_ustr));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
