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

#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/view/XFormLayerAccess.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/UnitConversion.hxx>
#include <o3tl/string_view.hxx>

#include <docsh.hxx>
#include <ftninfo.hxx>
#include <unotxdoc.hxx>

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
                "/ContentType:Types/ContentType:Override[@PartName='/word/document.xml']"_ostr,
                "ContentType"_ostr,
                u"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDefaultContentTypes)
{
    loadAndSave("fdo55381.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);
    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Default[@Extension='xml']"_ostr,
                "ContentType"_ostr,
                u"application/xml"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Default[@Extension='rels']"_ostr,
                "ContentType"_ostr,
                u"application/vnd.openxmlformats-package.relationships+xml"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Default[@Extension='png']"_ostr,
                "ContentType"_ostr,
                u"image/png"_ustr);

    assertXPath(pXmlDoc,
                "/ContentType:Types/ContentType:Default[@Extension='jpeg']"_ostr,
                "ContentType"_ostr,
                u"image/jpeg"_ustr);
}

DECLARE_SW_ROUNDTRIP_TEST(testDocmSave, "hello.docm", nullptr, DocmTest)
{
    // This was
    // application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml,
    // we used the wrong content type for .docm files.
    if (isExported())
    {
        xmlDocUniquePtr pXmlDoc = parseExport(u"[Content_Types].xml"_ustr);
        assertXPath(pXmlDoc,
                    "/ContentType:Types/ContentType:Override[@PartName='/word/document.xml']"_ostr,
                    "ContentType"_ostr,
                    u"application/vnd.ms-word.document.macroEnabled.main+xml"_ustr);
    }
}

DECLARE_SW_ROUNDTRIP_TEST(testBadDocm, "bad.docm", nullptr, DocmTest)
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    // This was 'MS Word 2007 XML', broken docm files were not recognized.
    CPPUNIT_ASSERT_EQUAL(u"MS Word 2007 XML VBA"_ustr, pTextDoc->GetDocShell()->GetMedium()->GetFilter()->GetName());
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
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing"_ostr, "after"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing"_ostr, "after"_ostr, u"0"_ustr);
    // This was 0 (list auto spacing is not zero before tables)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:spacing"_ostr, "after"_ostr, u"280"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[1]/w:pPr/w:spacing"_ostr, "after"_ostr, u"0"_ustr);
    // This was 0 (list auto spacing is not zero at the end of table cells)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[2]/w:pPr/w:spacing"_ostr, "after"_ostr, u"280"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p[1]/w:pPr/w:spacing"_ostr, "after"_ostr, u"280"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p[1]/w:pPr/w:spacing"_ostr, "after"_ostr, u"280"_ustr);
    // This was 0 (list auto spacing is not zero at list end)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing"_ostr, "after"_ostr, u"280"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132807)
{
    loadAndSave("tdf132807.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing"_ostr, "before"_ostr, u"280"_ustr);
    // This was 240 (list auto spacing is zero in lists)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[1]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
    // This was 240 (list auto spacing is zero in lists)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[2]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p[1]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p[2]/w:pPr/w:spacing"_ostr, "before"_ostr, u"280"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p[1]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:spacing"_ostr, "before"_ostr, u"280"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133052)
{
    loadAndSave("tdf133052.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // These were 240 (top auto spacing of list subitems are zero)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
    // in tables, too
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[2]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[3]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[4]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p[5]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134648)
{
    loadAndSave("tdf134648.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // list item with direct top auto spacing
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing"_ostr, "after"_ostr, u"240"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing"_ostr, "beforeAutospacing"_ostr, u"1"_ustr);

    // This was spacing w:after=200, but bottom auto spacing of first list subitem is zero
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing"_ostr, 0);
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
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:pPr/w:pStyle"_ostr, "val"_ostr, u"Normal"_ustr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:pPr/w:spacing"_ostr, "lineRule"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:pPr/w:spacing"_ostr, "line"_ostr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:pPr/w:spacing"_ostr, "before"_ostr, u"480"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:pPr/w:spacing"_ostr, "after"_ostr, u"20"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:r[1]/w:rPr/w:color"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:r[1]/w:rPr/w:sz"_ostr, "val"_ostr, u"16"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:r[2]/w:rPr/w:rStyle"_ostr, "val"_ostr, u"CharSubStyleDefaults"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:r[2]/w:rPr/w:color"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:p/w:r[2]/w:rPr/w:sz"_ostr, "val"_ostr, u"16"_ustr);
    // cell A2
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:pPr/w:pStyle"_ostr, "val"_ostr, u"Normal"_ustr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:pPr/w:spacing"_ostr, "lineRule"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:pPr/w:spacing"_ostr, "line"_ostr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:pPr/w:spacing"_ostr, "before"_ostr, u"480"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:pPr/w:spacing"_ostr, "after"_ostr, u"20"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[1]/w:rPr/w:color"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[1]/w:rPr/w:sz"_ostr, "val"_ostr, u"16"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[2]/w:rPr/w:rStyle"_ostr, "val"_ostr, u"ParaSubStyleDefaultsChar"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[2]/w:rPr/w:color"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[2]/w:rPr/w:sz"_ostr, "val"_ostr, u"16"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[3]/w:rPr/w:rStyle"_ostr, "val"_ostr, u"CharSubStyleNormal"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[3]/w:rPr/w:color"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p/w:r[3]/w:rPr/w:sz"_ostr, "val"_ostr, u"16"_ustr);
    // cell A3
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:pPr/w:pStyle"_ostr, "val"_ostr, u"ParaSubStyleNormal"_ustr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:pPr/w:spacing"_ostr, "lineRule"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:pPr/w:spacing"_ostr, "line"_ostr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:pPr/w:spacing"_ostr, "before"_ostr, u"480"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:pPr/w:spacing"_ostr, "after"_ostr, u"280"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[1]/w:rPr/w:color"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[1]/w:rPr/w:sz"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[2]/w:rPr/w:rStyle"_ostr, "val"_ostr, u"CharSubStyleNormal"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[2]/w:rPr/w:color"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc/w:p/w:r[2]/w:rPr/w:sz"_ostr, 0);
    // cell A4
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:pPr/w:pStyle"_ostr, "val"_ostr, u"ParaSubStyleDefaults"_ustr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:pPr/w:spacing"_ostr, "lineRule"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:pPr/w:spacing"_ostr, "line"_ostr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:pPr/w:spacing"_ostr, "before"_ostr, u"480"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:pPr/w:spacing"_ostr, "after"_ostr, u"200"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[1]/w:rPr/w:color"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[1]/w:rPr/w:sz"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[2]/w:rPr/w:rStyle"_ostr, "val"_ostr, u"CharSubStyleDefaults"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[2]/w:rPr/w:color"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[2]/w:rPr/w:sz"_ostr, 0);
    // cell A5
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:pStyle"_ostr, "val"_ostr, u"Normal"_ustr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:spacing"_ostr, "lineRule"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:spacing"_ostr, "line"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:rPr"_ostr, "color"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:rPr"_ostr, "sz"_ostr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:spacing"_ostr, "before"_ostr, u"480"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:pPr/w:spacing"_ostr, "after"_ostr, u"20"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:r[1]/w:rPr/w:color"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:r[1]/w:rPr/w:color"_ostr, "val"_ostr, u"AAAA00"_ustr); // all text in color
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[5]/w:tc/w:p/w:r[1]/w:rPr/w:sz"_ostr, "val"_ostr, u"16"_ustr);
    // cell A6
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:pStyle"_ostr, "val"_ostr, u"Normal"_ustr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:spacing"_ostr, "lineRule"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:spacing"_ostr, "line"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:rPr"_ostr, "color"_ostr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:rPr/w:sz"_ostr, "val"_ostr, u"16"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:spacing"_ostr, "before"_ostr, u"480"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:pPr/w:spacing"_ostr, "after"_ostr, u"20"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:r[1]/w:rPr/w:color"_ostr, 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:r[1]/w:rPr/w:sz"_ostr, "val"_ostr, u"16"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:r[2]/w:rPr/w:color"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:r[2]/w:rPr/w:color"_ostr, "val"_ostr, u"AAAA00"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[6]/w:tc/w:p/w:r[2]/w:rPr/w:sz"_ostr, "val"_ostr, u"16"_ustr);

    // tdf#131070 keep paragraph style based right indentation with indentation of direct numbering
    // cell A7 - This was <w:ind w:start="1440" w:hanging="0"/>
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[7]/w:tc/w:p/w:pPr/w:ind"_ostr, 0);
    // cell A8
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[8]/w:tc/w:p/w:pPr/w:ind"_ostr, "start"_ostr, u"714"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[8]/w:tc/w:p/w:pPr/w:ind"_ostr, "end"_ostr, u"1701"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[8]/w:tc/w:p/w:pPr/w:ind"_ostr, "hanging"_ostr, u"357"_ustr);
    // cell A9
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[9]/w:tc/w:p/w:pPr/w:ind"_ostr, "end"_ostr, u"1440"_ustr);
    // This was 1440
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[9]/w:tc/w:p/w:pPr/w:ind"_ostr, "start"_ostr, u"720"_ustr);
    // This was 0
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[9]/w:tc/w:p/w:pPr/w:ind"_ostr, "hanging"_ostr, u"360"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107626)
{
    loadAndSave("tdf107626.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 2 (missing trailing cell in merged cell range)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc"_ostr, 3);
}

DECLARE_OOXMLEXPORT_TEST(testTdf106970, "tdf106970.docx")
{
    // The second paragraph (first numbered one) had 0 bottom margin:
    // autospacing was even collapsed between different numbering styles.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(2), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(3), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(4), u"ParaBottomMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf79272_strictDxa, "tdf79272_strictDxa.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4318), getProperty<sal_Int32>(xTables->getByIndex(0), u"Width"_ustr));

    if (!isExported())
         return;
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    // Validation test: order of elements was wrong. Order was: insideH, end, insideV.
    int nEnd = getXPathPosition(pXmlDoc, "/w:styles/w:style[@w:styleId='TableGrid']/w:tblPr/w:tblBorders"_ostr, "end");
    int nInsideH = getXPathPosition(pXmlDoc, "/w:styles/w:style[@w:styleId='TableGrid']/w:tblPr/w:tblBorders"_ostr, "insideH");
    int nInsideV = getXPathPosition(pXmlDoc, "/w:styles/w:style[@w:styleId='TableGrid']/w:tblPr/w:tblBorders"_ostr, "insideV");
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

    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Default Paragraph Style"_ustr), uno::UNO_QUERY);
    //tdf107801: kerning normally isn't enabled by default for .docx
    CPPUNIT_ASSERT_EQUAL_MESSAGE("AutoKern should be false", false, getProperty<bool>(xStyle, u"CharAutoKerning"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf89377, "tdf89377_tableWithBreakBeforeParaStyle.docx")
{
    // the paragraph style should set table's text-flow break-before-page
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );

    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Default Paragraph Style"_ustr), uno::UNO_QUERY);
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

DECLARE_OOXMLEXPORT_TEST(testTdf97648_relativeWidth, "tdf97648_relativeWidth.docx")
{
    CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(7616), getShape(1)->getSize().Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(8001), getShape(2)->getSize().Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(4001), getShape(3)->getSize().Width, 10);
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_LEFT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(6), u"ParaAdjust"_ustr)) );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(1600), getShape(4)->getSize().Width, 10);
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_RIGHT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(8), u"ParaAdjust"_ustr)) );


    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), getProperty<sal_Int32>(getShape(1), u"LeftMargin"_ustr) );
    if (!isExported())
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
    // Check w:rStyle element has w:val attribute - note that w: is not specified for attribute
    assertXPath(pXmlDoc, "/w:endnotes/w:endnote[@w:id='2']/w:p/w:r[1]/w:rPr/w:rStyle"_ostr, "val"_ostr,
                u"EndnoteCharacters1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103389)
{
    loadAndSave("tdf103389.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // No geometry was exported for the second canvas
    // Check both canvases' geometry
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp/wps:spPr/a:prstGeom"_ostr, "prst"_ostr, u"rect"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp/wps:spPr/a:prstGeom"_ostr, "prst"_ostr, u"rect"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf84678, "tdf84678.docx")
{
    // This was 0, left margin inside a shape+text wasn't imported from DOCX.
    // 360000 EMU, but layout uses twips.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(567), parseDump("/root/page/body/txt/anchored/fly/infos/prtBounds"_ostr, "left"_ostr).toInt32());

}

DECLARE_OOXMLEXPORT_TEST(testTdf103544, "tdf103544.docx")
{
    // We have two shapes: a frame and an image
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    // Image was lost because of the frame export
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, u"Graphic"_ustr);
    CPPUNIT_ASSERT(xGraphic.is());
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
    sal_Int32 nDistB = getXPath(pXmlDoc, "//wp:anchor"_ostr, "distB"_ostr).toInt32();
    // This was -260350, which is not a valid value for an unsigned type.
    CPPUNIT_ASSERT(nDistB >= 0);

    // tdf#115670 the shadow should not be enabled (no on="t")
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(!getProperty<bool>(xPropertySet, u"Shadow"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104115)
{
    loadAndSave("tdf104115.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This found 0 nodes: the custom geometry was not written for the Bezier
    // curve -> Word refused to open the document.
    assertXPath(pXmlDoc, "//a:custGeom"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf103651, "tdf103651.docx")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 1), u"TextField"_ustr);
    OUString sContent;
    xTextField->getPropertyValue(u"Content"_ustr) >>= sContent;
    // Comment in the first paragraph should not have smiley ( 0xf04a ).
    CPPUNIT_ASSERT_EQUAL( sal_Int32( -1 ) , sContent.indexOf( u'\xf04a' ));

    // this document has a w:kern setting in the DocDefault character properties.  Ensure it applies.
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), u"CharAutoKerning"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99227)
{
    loadAndSave("tdf99227.docx");
    // A drawing anchored as character to a footnote caused write past end of document.xml at export to docx.
    // After that, importing after export failed with
    // SAXParseException: '[word/document.xml line 2]: Extra content at the end of the document', Stream 'word / document.xml',
    // and before commit ebf767eeb2a169ba533e1b2ffccf16f41d95df35, the drawing was silently lost.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/footnotes.xml"_ustr);

    assertXPath(pXmlDoc, "//w:footnote/w:p/w:r/w:drawing"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf37153, "tdf37153_considerWrapOnObjPos.docx")
{
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH, getProperty<text::WrapTextMode>(getShape(1), u"Surround"_ustr));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::BOTTOM, getProperty<sal_Int16>(xTable->getCellByName(u"A1"_ustr), u"VertOrient"_ustr));

    //For MSO compatibility, the textbox should be at the top of the cell, not at the bottom - despite VertOrientation::BOTTOM
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFlyTop  = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt/anchored/fly/infos/bounds"_ostr, "top"_ostr).toInt32();
    CPPUNIT_ASSERT_MESSAGE("FlyTop should be 2865, not 5649", nFlyTop < sal_Int32(3000));
    sal_Int32 nTextTop  = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt[1]/infos/bounds"_ostr, "top"_ostr).toInt32();
    CPPUNIT_ASSERT_MESSAGE("TextTop should be 3856", nTextTop > 3000);
}

DECLARE_OOXMLEXPORT_TEST(testTdf112446_frameStyle, "tdf112446_frameStyle.docx")
{
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::NONE, getProperty<sal_Int16>(getShape(1), u"HoriOrient"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf82173_footnoteStyle, "tdf82173_footnoteStyle.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();

    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;
    // This was footnote text, which didn't match with newly created footnotes
    CPPUNIT_ASSERT_EQUAL(u"Footnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xFootnoteText), u"ParaStyleName"_ustr));

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"CharacterStyles"_ustr)->getByName(u"Footnote Characters"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(58),       getProperty< sal_Int32 >(xPageStyle, u"CharEscapementHeight"_ustr) );
    CPPUNIT_ASSERT_EQUAL( COL_LIGHTGREEN, getProperty<Color>(xPageStyle, u"CharColor"_ustr));

    xPageStyle.set(getStyles(u"CharacterStyles"_ustr)->getByName(u"Footnote anchor"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(58),       getProperty< sal_Int32 >(xPageStyle, u"CharEscapementHeight"_ustr) );
    CPPUNIT_ASSERT_EQUAL( COL_LIGHTGREEN, getProperty<Color>(xPageStyle, u"CharColor"_ustr));

    //tdf#118361 - in RTL locales, the footnote separator should still be left aligned.
    uno::Any aPageStyle = getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote separator LTR", sal_Int16(0), getProperty<sal_Int16>(aPageStyle, u"FootnoteLineAdjust"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf82173_endnoteStyle, "tdf82173_endnoteStyle.docx")
{
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();
    uno::Reference<text::XFootnote> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;
    // character properties were previously not assigned to the footnote/endnote in-text anchor.
    CPPUNIT_ASSERT_EQUAL( 24.0f, getProperty< float >(xEndnote->getAnchor(), u"CharHeight"_ustr) );
    CPPUNIT_ASSERT_EQUAL( COL_LIGHTRED, getProperty<Color>(xEndnote->getAnchor(), u"CharColor"_ustr));

    uno::Reference<text::XText> xEndnoteText;
    xEndnotes->getByIndex(0) >>= xEndnoteText;
    // This was Endnote Symbol
    CPPUNIT_ASSERT_EQUAL(u"Endnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xEndnoteText), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x993300), getProperty<Color>(getParagraphOfText(1, xEndnoteText), u"CharColor"_ustr));

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"CharacterStyles"_ustr)->getByName(u"Endnote Characters"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(58),       getProperty< sal_Int32 >(xPageStyle, u"CharEscapementHeight"_ustr) );
    CPPUNIT_ASSERT_EQUAL( COL_LIGHTMAGENTA, getProperty<Color>(xPageStyle, u"CharColor"_ustr));

    xPageStyle.set(getStyles(u"CharacterStyles"_ustr)->getByName(u"Endnote anchor"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(58),       getProperty< sal_Int32 >(xPageStyle, u"CharEscapementHeight"_ustr) );
    CPPUNIT_ASSERT_EQUAL( COL_LIGHTMAGENTA, getProperty<Color>(xPageStyle, u"CharColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf55427_footnote2endnote)
{
    loadAndReload("tdf55427_footnote2endnote.odt");
    CPPUNIT_ASSERT_EQUAL(4, getPages());
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Footnote"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Footnote style is rose color", Color(0xFF007F), getProperty<Color>(xPageStyle, u"CharColor"_ustr));
    xPageStyle.set(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Endnote"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Endnote style is cyan3 color", Color(0x2BD0D2), getProperty<Color>(xPageStyle, u"CharColor"_ustr));

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    // The footnote numbering type of ARABIC will not transfer over when those footnotes are converted to endnotes.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Footnote numbering type", SVX_NUM_ARABIC, pDoc->GetFootnoteInfo().m_aFormat.GetNumberingType() );
    // The original document has a real endnote using ROMAN_LOWER numbering, so that setting MUST remain unchanged.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Endnote numbering type", SVX_NUM_ROMAN_LOWER, pDoc->GetEndNoteInfo().m_aFormat.GetNumberingType() );

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();

    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();
    uno::Reference<text::XFootnote> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;
    uno::Reference<text::XText> xEndnoteText;
    xEndnotes->getByIndex(0) >>= xEndnoteText;

    // ODT footnote-at-document-end's closest DOCX match is an endnote, so the two imports will not exactly match by design.
    if (!isExported())
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote count", sal_Int32(5), xFootnotes->getCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote count", sal_Int32(1), xEndnotes->getCount() );

        uno::Reference<text::XFootnote> xFootnote;
        xFootnotes->getByIndex(0) >>= xFootnote;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote's number", u"1"_ustr, xFootnote->getAnchor()->getString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote's number", u"i"_ustr, xEndnote->getAnchor()->getString() );

        uno::Reference<text::XText> xFootnoteText;
        xFootnotes->getByIndex(0) >>= xFootnoteText;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote style", u"Footnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xFootnoteText), u"ParaStyleName"_ustr) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote style", u"Endnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xEndnoteText), u"ParaStyleName"_ustr) );
    }
    else
    {
        // These asserted items are major differences in the conversion from footnote to endnote, NOT necessary conditions for a proper functioning document.
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "At-Document-End footnotes were converted into endnotes", sal_Int32(0), xFootnotes->getCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "At-Document-End footnotes became endnotes", sal_Int32(6), xEndnotes->getCount() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "converted footnote's number", u"i"_ustr, xEndnote->getAnchor()->getString() );
        xEndnotes->getByIndex(4) >>= xEndnote;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote's new number", u"v"_ustr, xEndnote->getAnchor()->getString() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "retained footnote style", u"Footnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xEndnoteText), u"ParaStyleName"_ustr) );
        xEndnotes->getByIndex(4) >>= xEndnoteText;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote style", u"Endnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xEndnoteText), u"ParaStyleName"_ustr) );
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf104162, "tdf104162.docx")
{
    // This crashed: the comment field contained a table with a <w:hideMark/>.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XElementAccess> xTextFields(xTextFieldsSupplier->getTextFields());
    CPPUNIT_ASSERT(xTextFields->hasElements());
}

DECLARE_OOXMLEXPORT_TEST(testTdf104150, "tdf104150.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was 0xff0000, i.e. red: background shape wasn't ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), getProperty<sal_Int32>(xPageStyle, u"BackColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf103976, "tdf103976.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    // This was 0, table style inheritance went wrong and w:afterLines had priority over w:after.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(60)), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));

    // tdf#116549: heading 2 style should not have a bottom border.
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Heading 2"_ustr), uno::UNO_QUERY);
    table::BorderLine2 aBottomBorder = getProperty<table::BorderLine2>(xStyle, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), aBottomBorder.LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf106001, "tdf106001.docx")
{
    // This was 0 (1 on UI), while Word treats outliers as 100 (outlier = not in [1..600])
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int16>( 100 ), getProperty<sal_Int16>(getRun(getParagraph(1), 1), u"CharScaleWidth"_ustr ));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106001_2)
{
    loadAndSave("tdf106001-2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // In test ODT CharScaleWidth = 900, this was not changed upon OOXML export to stay in [1..600], now it's clamped to 600
    // Note: we disregard what's set in pPr / rPr and only care about r / rPr
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:w"_ostr,"val"_ostr,u"600"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf99074, "tdf99074.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> const xController(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> const xViewSettings(
        xController->getViewSettings());

    // The behavior changed - Word 2013 and 2016 ignore this setting on
    // import, and instead honor the user's setting.
    // Let's ignore the <w:view w:val="web"/> too.
    CPPUNIT_ASSERT(!getProperty<bool>(xViewSettings, u"ShowOnlineLayout"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testDefaultSectBreakCols, "default-sect-break-cols.docx")
{
    // First problem: the first two paragraphs did not have their own text section, so the whole document had two columns.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1, u"First."_ustr), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    // Second problem: the page style had two columns, while it shouldn't have any.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xPageStyle, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be FALSE as the document does not contain separator line.
    bool bValue = getProperty< bool >(xTextColumns, u"SeparatorLineIsOn"_ustr);
    CPPUNIT_ASSERT(!bValue) ;
}

DECLARE_OOXMLEXPORT_TEST(testMultiColumnSeparator, "multi-column-separator-with-line.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1, u"First data."_ustr), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be TRUE as the document contains separator line.
    bool  bValue = getProperty< bool >(xTextColumns, u"SeparatorLineIsOn"_ustr);
    CPPUNIT_ASSERT(bValue);
}

DECLARE_OOXMLEXPORT_TEST(testUnbalancedColumns, "unbalanced-columns.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was false, last section was balanced, but it's unbalanced in Word.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(2), u"DontBalanceTextColumns"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf121670_columnsInSectionsOnly, "tdf121670_columnsInSectionsOnly.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DontBalanceTextColumns?", true, getProperty<bool>(xTextSections->getByIndex(0), u"DontBalanceTextColumns"_ustr));

    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of columns", sal_Int16(3), xTextColumns->getColumnCount());

    xTextSection.set( getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(3), u"TextSection"_ustr) );
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns.set( getProperty< uno::Reference<text::XTextColumns> >(xTextSection, u"TextColumns"_ustr) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of columns", sal_Int16(0), xTextColumns->getColumnCount());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106492)
{
    loadAndSave("tdf106492.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 4: an additional sectPr was added to the document.
    assertXPath(pXmlDoc, "//w:sectPr"_ostr, 3);
}

DECLARE_OOXMLEXPORT_TEST(testTdf107104, "tdf107104.docx")
{
    CPPUNIT_ASSERT(getShape(1)->getSize().Width > 0);
    // This failed: the second arrow was invisible because it had zero width.
    CPPUNIT_ASSERT(getShape(2)->getSize().Width > 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf107033, "tdf107033.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was 0: footnote separator was disabled even in case the document
    // had no footnotes.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(25), getProperty<sal_Int32>(xPageStyle, u"FootnoteLineRelativeWidth"_ustr));
}

#if HAVE_MORE_FONTS
CPPUNIT_TEST_FIXTURE(Test, testTdf107889)
{
    auto verify = [this]() {
        // This was 1, multi-page table was imported as a non-split fly.
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//tab"_ostr, 2);
    };
    createSwDoc("tdf107889.docx");
    verify();
    saveAndReload(u"Office Open XML Text"_ustr);
    verify();
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testTdf107837)
{
    loadAndReload("tdf107837.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was true, a balanced section from ODF turned into a non-balanced one after OOXML roundtrip.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTextSections->getByIndex(0), u"DontBalanceTextColumns"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107684)
{
    loadAndReload("tdf107684.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    // This was 1, <w:outlineLvl> was duplicated for Heading1.
    assertXPath(pXmlDoc, "//w:style[@w:styleId='Heading1']/w:pPr/w:outlineLvl"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107618)
{
    loadAndReload("tdf107618.doc");
    // This was false, header was lost on export.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf108682, "tdf108682.docx")
{
    auto aLineSpacing = getProperty<style::LineSpacing>(getParagraph(1), u"ParaLineSpacing"_ustr);
    // This was style::LineSpacingMode::PROP.
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::FIX, aLineSpacing.Mode);
    // 260 twips in mm100, this was a negative value.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(459), aLineSpacing.Height);
}

DECLARE_OOXMLEXPORT_TEST(testTdf100075, "tdf100075.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);

    // There are two frames in document
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xIndexAccess->getCount());

    uno::Reference<beans::XPropertySet> xFrame1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

    // Ensure that frame#1 height is more that frame#2: if no hRul attribute
    // defined, MS Word will use hRul=auto if height is not defined,
    // and hRul=atLeast if height is provided. So frame#1 should be higher
    CPPUNIT_ASSERT(getProperty<sal_Int32>(xFrame1, u"Height"_ustr) > getProperty<sal_Int32>(xFrame2, u"Height"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf105095, "tdf105095.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This failed, tab between the footnote number and the footnote content
    // was lost on import.
    CPPUNIT_ASSERT_EQUAL( u"\tfootnote"_ustr, xTextRange->getString() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106062_nonHangingFootnote)
{
    loadAndReload("tdf106062_nonHangingFootnote.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This failed, tab between the footnote number and the footnote content was lost on import.
    CPPUNIT_ASSERT_MESSAGE( "Footnote starts with a tab", xTextRange->getString().startsWith("\t") );
}

DECLARE_OOXMLEXPORT_TEST( testActiveXTextfield, "activex_textbox.docx" )
{
    uno::Reference<drawing::XControlShape> xControlShape( getShape(1), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xControlShape.is() );

    // Check control type
    uno::Reference<beans::XPropertySet> xPropertySet( xControlShape->getControl(), uno::UNO_QUERY );
    uno::Reference<lang::XServiceInfo> xServiceInfo( xPropertySet, uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL( true, bool( xServiceInfo->supportsService ( u"com.sun.star.form.component.TextField"_ustr ) ) );

    // Check textfield is multi-line
    CPPUNIT_ASSERT_EQUAL( true, getProperty<bool>(xPropertySet, u"MultiLine"_ustr) );

    uno::Reference<drawing::XControlShape> xControlShape2( getShape(2), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xControlShape2.is() );

    // Check control type
    uno::Reference<beans::XPropertySet> xPropertySet2( xControlShape2->getControl(), uno::UNO_QUERY );
    uno::Reference<lang::XServiceInfo> xServiceInfo2( xPropertySet2, uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL( true, bool( xServiceInfo2->supportsService ( u"com.sun.star.form.component.TextField"_ustr ) ) );

    // Check textfield is single-line
    CPPUNIT_ASSERT_EQUAL( false, getProperty<bool>(xPropertySet2, u"MultiLine"_ustr) );

    // Don't open in design mode when form controls exist
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XFormLayerAccess> xFormLayerAccess(xModel->getCurrentController(), uno::UNO_QUERY);
    CPPUNIT_ASSERT( !xFormLayerAccess->isFormDesignMode() );
}

DECLARE_OOXMLEXPORT_TEST( testActiveXCheckbox, "activex_checkbox.docx" )
{
    uno::Reference<drawing::XControlShape> xControlShape( getShape(1), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xControlShape.is() );

    // Check control type
    uno::Reference<beans::XPropertySet> xPropertySet( xControlShape->getControl(), uno::UNO_QUERY );
    uno::Reference<lang::XServiceInfo> xServiceInfo( xPropertySet, uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL( true, bool( xServiceInfo->supportsService( u"com.sun.star.form.component.CheckBox"_ustr ) ) );

    // Check custom label
    CPPUNIT_ASSERT_EQUAL( u"Custom Caption"_ustr, getProperty<OUString>(xPropertySet, u"Label"_ustr) );

    // Check background color (highlight system color)
    CPPUNIT_ASSERT_EQUAL( Color( 0x316AC5 ), getProperty<Color>(xPropertySet, u"BackgroundColor"_ustr) );

    // Check Text color (active border system color)
    CPPUNIT_ASSERT_EQUAL(Color(0xD4D0C8), getProperty<Color>(xPropertySet, u"TextColor"_ustr));

    // Check state of the checkbox
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xPropertySet, u"State"_ustr));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,u"AnchorType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testActiveXControlAlign)
{
    loadAndReload("activex_control_align.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // First check box aligned as a floating object
    uno::Reference<drawing::XControlShape> xControlShape(getShape(2), uno::UNO_QUERY_THROW);

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService( u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"Floating Check Box"_ustr, getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,u"AnchorType"_ustr));

    // Also check position and size
    uno::Reference<drawing::XShape> xShape(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4470), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1427), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5126), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2341), xShape->getPosition().Y);

    // Second check box aligned inline / as character
    xControlShape.set(getShape(1), uno::UNO_QUERY_THROW);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"Inline Check Box"_ustr, getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,u"AnchorType"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(text::VertOrientation::TOP),getProperty<sal_Int32>(xPropertySet2,u"VertOrient"_ustr));

    // Also check position and size
    xShape.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4410), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1083), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1085), xShape->getPosition().Y);

    // Also check the specific OOXML elements
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // For inline controls use w:object as parent element and pictureFrame shapetype
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object/v:shapetype"_ostr, "spt"_ostr, u"75"_ustr);
    // For floating controls use w:pict as parent element and hostControl shapetype
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict/v:shapetype"_ostr, "spt"_ostr, u"201"_ustr);

     // Have different shape ids
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object/v:shape"_ostr, "id"_ostr) !=
        getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict/v:shape"_ostr, "id"_ostr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf109184, "tdf109184.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Before table background color was white, should be transparent (auto).
    uno::Reference<text::XTextRange> xCell1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xCell1, u"BackColor"_ustr));

    // Cell with auto color but with 15% fill, shouldn't be transparent.
    uno::Reference<text::XTextRange> xCell2(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xd8d8d8), getProperty<Color>(xCell2, u"BackColor"_ustr));

    // Cell with color defined (red).
    uno::Reference<text::XTextRange> xCell3(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(xCell3, u"BackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf111964)
{
    loadAndSave("tdf111964.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Unicode spaces that are not XML whitespace must not be trimmed
    static constexpr OUStringLiteral sWSReference = u"\u2002\u2002\u2002\u2002\u2002";
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:t"_ostr, sWSReference);
}

DECLARE_OOXMLEXPORT_TEST(testWatermark, "watermark-shapetype.docx")
{
    uno::Reference<drawing::XShape> xShape1(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape1.is());
    uno::Reference<beans::XPropertySet> xPropertySet1(xShape1, uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape2.is());
    uno::Reference<beans::XPropertySet> xPropertySet2(xShape2, uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(xPropertySet1->getPropertyValue(u"TextAutoGrowHeight"_ustr), xPropertySet2->getPropertyValue(u"TextAutoGrowHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testActiveXControlAtRunEnd)
{
    loadAndReload("activex_control_at_run_end.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Two issues were here:
    //  1) second shape was not export (it is anchored to the end of the run)
    //  2) inline property was inherited to the second shape by mistake

    // First checkbox is the inlined one
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService( u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"Inline Checkbox"_ustr, getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,u"AnchorType"_ustr));

    // Second check box anchored to character
    xControlShape.set(getShape(2), uno::UNO_QUERY);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"Floating Checkbox"_ustr, getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,u"AnchorType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testActiveXOptionButtonGroup, "activex_option_button_group.docx")
{
    // Optionbutton groups were not handled
    // The two optionbutton should have the same group name
    constexpr OUString sGroupName = u"GroupX"_ustr;

    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sGroupName, getProperty<OUString>(xPropertySet, u"GroupName"_ustr));

    xControlShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sGroupName, getProperty<OUString>(xPropertySet, u"GroupName"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, tdf112169)
{
    loadAndSave("tdf112169.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(6, getPages());
    // LO crashed while export because of character background color handling

    //tdf76683 - Cannot be negative number - use firstLine instead of hanging
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:pPr/w:ind"_ostr, "hanging"_ostr);
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:pPr/w:ind"_ostr, "firstLine"_ostr,u"360"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103090)
{
    loadAndSave("tdf103090.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Get bookmark name
    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:bookmarkStart"_ostr, "name"_ostr);

    // Ensure that name has no spaces
    CPPUNIT_ASSERT(bookmarkName.indexOf(" ") < 0);

    // Get PAGEREF field
    OUString fieldName = getXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:instrText"_ostr);

    // Ensure that PAGEREF field refers exactly our bookmark
    OUString expectedFieldName = " PAGEREF " + bookmarkName + " \\h ";
    CPPUNIT_ASSERT_EQUAL(expectedFieldName, fieldName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107111)
{
    loadAndSave("tdf107111.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Ensure that hyperlink and its properties are in place.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:hyperlink/w:r/w:rPr"_ostr, 1);

    // Ensure that hyperlink properties do not contain <w:webHidden/>.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:hyperlink/w:r/w:rPr/w:webHidden"_ostr, 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf90789, "tdf90789.docx")
{
    uno::Reference<text::XTextContent> xShape(getShape(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xShape->getAnchor() != nullptr);

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(), uno::UNO_QUERY_THROW);
    xCtrl->select(uno::Any(xShape->getAnchor()));

    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xCtrl, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextViewCursor> xTextCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_SET_THROW);
    uno::Reference<text::XPageCursor> xPageCursor(xTextCursor, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), xPageCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90789_2, "tdf90789-2.docx")
{
    // Section break before frame and shape was ignored
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf104354_2, "tdf104354-2.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);

    // top margin of the first paragraph and bottom margin of the last paragraph
    // is zero, when auto spacing is used.

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), u"ParaBottomMargin"_ustr));

    // top margin is not auto spacing
    uno::Reference<text::XTextRange> xCell2(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(847), getProperty<sal_Int32>(getParagraphOfText(1, xCell2->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell2->getText()), u"ParaBottomMargin"_ustr));

    // bottom margin is not auto spacing
    uno::Reference<text::XTextRange> xCell3(xTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell3->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(847), getProperty<sal_Int32>(getParagraphOfText(1, xCell3->getText()), u"ParaBottomMargin"_ustr));

    // auto spacing, if the paragraph contains footnotes
    uno::Reference<text::XTextRange> xCell4(xTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell4->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell4->getText()), u"ParaBottomMargin"_ustr));

    // auto spacing is explicitly disabled, and no margins are defined.
    xCell.set(xTable->getCellByName(u"A5"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), u"ParaBottomMargin"_ustr));

    // auto spacing on a paragraph
    uno::Reference<text::XTextTable> xTable2(xTables->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell5(xTable2->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell5->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell5->getText()), u"ParaBottomMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf137593)
{
    loadAndSave("tdf137593.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // zero auto spacing, if the first paragraph contains text boxes
    // This was 280.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr/w:tc/w:p[1]/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115557)
{
    loadAndSave("tdf115557.docx");
    // A chart anchored to a footnote multiplied during import
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/footnotes.xml"_ustr);

    assertXPath(pXmlDoc, "//w:footnote/w:p/w:r/w:drawing"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testAlignmentRelativeFromTopMarginDML)
{
    loadAndSave("tdf137641_RelativeFromTopMargin.docx");
    // Import as DML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionV"_ostr,
                "relativeFrom"_ostr, u"topMargin"_ustr);
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align"_ostr,
                       u"top"_ustr);
    assertXPath(pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/"
        "wp:anchor/wp:positionV"_ostr,
        "relativeFrom"_ostr, u"topMargin"_ustr);
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align"_ostr,
                       u"bottom"_ustr);
    assertXPath(pXmlDoc,
        "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/"
        "wp:anchor/wp:positionV"_ostr,
        "relativeFrom"_ostr, u"topMargin"_ustr);
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align"_ostr,
                       u"center"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testAlignmentRelativeFromTopMarginVML)
{
    loadAndSave("tdf137642_Vertical_Alignment_toppage.docx");
    // Import as VML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionV"_ostr,
                "relativeFrom"_ostr, u"topMargin"_ustr);
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align"_ostr,
                       u"top"_ustr);
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionV"_ostr,
                "relativeFrom"_ostr, u"topMargin"_ustr);
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align"_ostr,
                       u"bottom"_ustr);
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionV"_ostr,
                "relativeFrom"_ostr, u"topMargin"_ustr);
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align"_ostr,
                       u"center"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testVmlShapeWithTextbox)
{
    loadAndSave("tdf41466_testVmlShapeWithTextbox.docx");
    // Import as VML.
    // tdf#41466: check whether VML DOCX shape with text is imported as shape with a text frame
    // (text box). These kind of shapes were imported only as text frames previously, losing the
    // preset shape geometry, in this case "wedgeRectCallout".
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // the wrong value was "rect" instead of "wedgeRectCallout"
    assertXPath(pXmlDoc,
        "/w:document/w:body/w:p/w:r/"
        "mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:prstGeom"_ostr,
        "prst"_ostr, u"wedgeRectCallout"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testLayoutFlowAltAlone)
{
    loadAndSave("layout-flow-alt-alone.docx");
    // moved from oox/qa/unit/vml.cxx
    // FIXME: now the DML part is checked, but we should check VML part in Fallback (too)
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
        "a:graphic/a:graphicData/wps:wsp/wps:bodyPr"_ostr, "vert"_ostr, u"vert270"_ustr);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
