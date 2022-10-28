/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <queue>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/util/XRefreshable.hpp>


#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/propertyvalue.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}
};

CPPUNIT_TEST_FIXTURE(Test, testTdf150197_predefinedNumbering)
{
    createSwDoc();

    // The exact numbering style doesn't matter - just any non-bullet pre-defined numbering style.
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(OUString("Numbering 123")) },
        { "FamilyName", uno::Any(OUString("NumberingStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(1), "ListLabelString"));

    reload("Office Open XML Text", "");
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(1), "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(Test, testInlineSdtHeader)
{
    // Without the accompanying fix in place, this test would have failed with an assertion failure,
    // we produced not-well-formed XML on save.
    loadAndSave("inline-sdt-header.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testCellSdtRedline)
{
    // Without the accompanying fix in place, this test would have failed with an assertion failure,
    // we produced not-well-formed XML on save.
    loadAndSave("cell-sdt-redline.docx");
}

DECLARE_OOXMLEXPORT_TEST(testTdf147724, "tdf147724.docx")
{
    const auto& pLayout = parseLayoutDump();

    // Ensure we load field value from external XML correctly (it was "HERUNTERLADEN")
    assertXPathContent(pLayout, "/root/page[1]/body/txt[1]", "Placeholder -> *ABC*");

    // This SDT has no storage id, it is not an error, but content can be taken from any suitable XML
    // There 2 variants possible, both are acceptable
    OUString sFieldResult = getXPathContent(pLayout, "/root/page[1]/body/txt[2]");
    CPPUNIT_ASSERT(sFieldResult == "Placeholder -> *HERUNTERLADEN*" || sFieldResult == "Placeholder -> *ABC*");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150966_regularInset)
{
    // Given a docx document with a rectangular shape with height cy="900000" (EMU), tIns="180000"
    // and bIns="360000", resulting in 360000EMU text area height.
    load(DATA_DIRECTORY, "tdf150966_regularInset.docx");

    // The shape is imported as custom shape with attached frame.
    // The insets are currently imported as margin top="4.99mm" and bottom="10mm".
    // That should result in tIns="179640" and bIns="360000" on export.

    // Without fix the insets were tIns="359280" and bIns="539640". The text area had 1080Emu height
    // and Word displayes no text at all.
    save("Office Open XML Text", maTempFile);
    mbExported = true;
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPathAttrs(pXmlDoc, "//wps:bodyPr", { { "tIns", "179640" }, { "bIns", "360000" } });
}

CPPUNIT_TEST_FIXTURE(Test, testImageCropping)
{
    loadAndReload("crop-roundtrip.docx");

    // the image has no cropping after roundtrip, because it has been physically cropped
    // NB: this test should be fixed when the core feature to show image cropped when it
    // has the "GraphicCrop" is set is implemented
    auto aGraphicCropStruct = getProperty<text::GraphicCrop>(getShape(1), "GraphicCrop");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Left);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Right);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Top);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aGraphicCropStruct.Bottom);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152200)
{
    // Given a document with a fly anchored after a FORMTEXT in the end of the paragraph:
    load(DATA_DIRECTORY, "tdf152200-field+textbox.docx");

    // When exporting that back to DOCX:
    save("Office Open XML Text", maTempFile);
    mbExported = true;

    // Then make sure that fldChar with type 'end' goes prior to the at-char anchored fly.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    const int nRunsBeforeFldCharEnd = countXPathNodes(pXmlDoc, "//w:fldChar[@w:fldCharType='end']/preceding::w:r");
    CPPUNIT_ASSERT(nRunsBeforeFldCharEnd);
    const int nRunsBeforeAlternateContent = countXPathNodes(pXmlDoc, "//mc:AlternateContent/preceding::w:r");
    CPPUNIT_ASSERT(nRunsBeforeAlternateContent);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 6
    // - Actual  : 5
    CPPUNIT_ASSERT_GREATER(nRunsBeforeFldCharEnd, nRunsBeforeAlternateContent);
    // Make sure we only have one paragraph in body, and only three field characters overal,
    // located directly in runs of this paragraph
    assertXPath(pXmlDoc, "/w:document/w:body/w:p");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:fldChar", 3);
    assertXPath(pXmlDoc, "//w:fldChar", 3); // no field characters elsewhere
}

CPPUNIT_TEST_FIXTURE(Test, testNumberPortionFormatFromODT)
{
    // Given a document with a single paragraph, direct formatting asks 24pt font size for the
    // numbering and the text portion:
    load(DATA_DIRECTORY, "number-portion-format.odt");

    // When saving to DOCX:
    save("Office Open XML Text", maTempFile);
    mbExported = true;

    // Then make sure that the paragraph marker's char format has that custom font size:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:pPr/w:rPr/w:sz' number of nodes is incorrect
    // i.e. <w:sz> was missing under <w:pPr>'s <w:rPr>.
    assertXPath(pXmlDoc, "//w:pPr/w:rPr/w:sz", "val", "48");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
