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

DECLARE_OOXMLEXPORT_TEST(testTdf153613_sdtAfterPgBreak, "tdf153613_sdtAfterPgBreak.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf153964_topMarginAfterBreak14, "tdf153964_topMarginAfterBreak14.docx")
{
    //The top margin should only apply once in a split paragraph.
    //In this compat14 (Windows 2010) version, it applies after the break if no prior visible run.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, "a w:br at the start of the document. Does it use 60 point top margin?"), uno::UNO_QUERY);
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));

    xPara.set(getParagraph(3, u"60 pt spacing before"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was applied to paragraph 3, so it shouldn't apply here
    xPara.set(getParagraph(4, u"column break1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    //CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, "ParaBackColor"));

    xPara.set(getParagraph(5, u"60 pt followed by page break"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was applied to paragraph 5, so it shouldn't apply here
    xPara.set(getParagraph(6, u"page break1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was not applied yet, so with compat14 it should apply here.
    xPara.set(getParagraph(7, u"column break2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xe2efd9), getProperty<Color>(xPara, "ParaBackColor"));

    // In an odd twist, the w:br was actually at the end of the previous w:p, so in that case
    // we ignore the top margin definition this time.
    xPara.set(getParagraph(9, u"page break2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));

    // The top margin was not applied before the column break, so with compat14 it should apply here
    xPara.set(getParagraph(10, u""), uno::UNO_QUERY); // after column break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    //CPPUNIT_ASSERT_EQUAL(Color(0xfff2cc), getProperty<Color>(xPara, "ParaBackColor"));

    // In an odd twist, the w:br was actually at the end of the previous w:p, so in that case
    // we ignore the top margin definition this time.
    xPara.set(getParagraph(12, u""), uno::UNO_QUERY); // after page break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153964_topMarginAfterBreak15, "tdf153964_topMarginAfterBreak15.docx")
{
    //The top margin should only apply once (at most) in a split paragraph.
    //In this compat15 (Windows 2013) version, it never applies after the break.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, "a w:br at the start of the document. Does it use 60 point top margin?"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));

    xPara.set(getParagraph(3, u"60 pt spacing before"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was applied to paragraph 3, so it shouldn't apply here
    xPara.set(getParagraph(4, u"column break1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    //CPPUNIT_ASSERT_EQUAL(Color(0xfbe4d5), getProperty<Color>(xPara, "ParaBackColor"));

    xPara.set(getParagraph(5, u"60 pt followed by page break"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2117), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was applied to paragraph 5, so it shouldn't apply here
    xPara.set(getParagraph(6, u"page break1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xdeeaf6), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was not applied to paragraph 6, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(7, u"column break2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(Color(0xe2efd9), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin not was applied to paragraph 8, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(9, u"page break2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));

    // The top margin was not applied to paragraph 9, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(10, u""), uno::UNO_QUERY); // after column break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    //CPPUNIT_ASSERT_EQUAL(Color(0xfff2cc), getProperty<Color>(xPara, "ParaBackColor"));

    // The top margin was not applied to paragraph 11, and with compat15 it shouldn't apply here.
    xPara.set(getParagraph(12, u""), uno::UNO_QUERY); // after page break
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152636_lostPageBreak2)
{
    loadAndReload("tdf152636_lostPageBreak2.docx");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testSdtDuplicatedId)
{
    // Given a document with 2 inline <w:sdt>, with each a <w:id>:
    // When exporting that back to DOCX:
    loadAndSave("sdt-duplicated-id.docx");

    // Then make sure we write 2 <w:sdt> and no duplicates:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 4
    // i.e. grab-bags introduced 2 unwanted duplicates.
    assertXPath(pXmlDoc, "//w:sdt", 2);
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
    // When exporting that back to DOCX:
    loadAndSave("tdf152200-field+textbox.docx");

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

CPPUNIT_TEST_FIXTURE(Test, testTdf152425)
{
    loadAndReload("tdf152425.docx");

    // Check that "List Number" and "List 5" styles don't get merged
    const OUString Para3Style = getProperty<OUString>(getParagraph(3), "ParaStyleName");
    CPPUNIT_ASSERT_EQUAL(OUString("Numbering 1"), Para3Style);
    const OUString Para4Style = getProperty<OUString>(getParagraph(4), "ParaStyleName");
    CPPUNIT_ASSERT_EQUAL(OUString("List 5 (WW)"), Para4Style);
    // Also check that "List 5" and "List Bullet 5" styles don't get merged
    const OUString Para5Style = getProperty<OUString>(getParagraph(5), "ParaStyleName");
    CPPUNIT_ASSERT_EQUAL(OUString("List 5"), Para5Style);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
