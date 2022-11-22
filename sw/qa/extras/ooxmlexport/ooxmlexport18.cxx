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
#include <com/sun/star/text/WritingMode2.hpp>
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

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}
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

DECLARE_OOXMLEXPORT_TEST(testTdf147646, "tdf147646_mergedCellNumbering.docx")
{
    parseLayoutDump();
    //Without the fix in place, it would have failed with
    //- Expected: 2.
    //- Actual  : 4.
    CPPUNIT_ASSERT_EQUAL(OUString("2."),parseDump("/root/page/body/tab/row[4]/cell/txt/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']","expand"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149551_mongolianVert)
{
    // Given a docx document with a shape with vert="mongolianVert".
    createSwDoc("tdf149551_mongolianVert.docx");

    // The shape is imported as custom shape with attached frame.
    // Without fix the shape itself had WritingMode = 0 = LR_TB,
    // the frame in it had WritingMode = 2 = TB_RL.
    // It should be WritingMode = 3 = TB_LR in both cases.
    const sal_Int16 eExpected(text::WritingMode2::TB_LR);
    CPPUNIT_ASSERT_EQUAL(eExpected, getProperty<sal_Int16>(getShape(1), "WritingMode"));
    uno::Reference<beans::XPropertySet> xShapeProps(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrameProps(xShapeProps->getPropertyValue("TextBoxContent"),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(eExpected, getProperty<sal_Int16>(xFrameProps, "WritingMode"));

    // Such shape must have vert="mongolianVert" again after saving.
    // Without fix the orientation was vert="vert".
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "//wps:bodyPr", "vert", "mongolianVert");
}

DECLARE_OOXMLEXPORT_TEST(testTdf151912, "tdf151912.docx")
{
    // For now just ensure roundtrip is successful
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

CPPUNIT_TEST_FIXTURE(Test, testNumberPortionFormatFromODT)
{
    // Given a document with a single paragraph, direct formatting asks 24pt font size for the
    // numbering and the text portion:
    createSwDoc("number-portion-format.odt");

    // When saving to DOCX:
    save("Office Open XML Text");

    // Then make sure that the paragraph marker's char format has that custom font size:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:pPr/w:rPr/w:sz' number of nodes is incorrect
    // i.e. <w:sz> was missing under <w:pPr>'s <w:rPr>.
    assertXPath(pXmlDoc, "//w:pPr/w:rPr/w:sz", "val", "48");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150966_regularInset)
{
    // Given a docx document with a rectangular shape with height cy="900000" (EMU), tIns="180000"
    // and bIns="360000", resulting in 360000EMU text area height.
    createSwDoc("tdf150966_regularInset.docx");

    // The shape is imported as custom shape with attached frame.
    // The insets are currently imported as margin top="4.99mm" and bottom="10mm".
    // That should result in tIns="179640" and bIns="360000" on export.

    // Without fix the insets were tIns="359280" and bIns="539640". The text area had 1080Emu height
    // and Word displays no text at all.
    save("Office Open XML Text");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPathAttrs(pXmlDoc, "//wps:bodyPr", { { "tIns", "179640" }, { "bIns", "360000" } });
}

CPPUNIT_TEST_FIXTURE(Test, testSdtDuplicatedId)
{
    // Given a document with 2 inline <w:sdt>, with each a <w:id>:
    createSwDoc("sdt-duplicated-id.docx");

    // When exporting that back to DOCX:
    save("Office Open XML Text");

    // Then make sure we write 2 <w:sdt> and no duplicates:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 4
    // i.e. grab-bags introduced 2 unwanted duplicates.
    assertXPath(pXmlDoc, "//w:sdt", 2);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
