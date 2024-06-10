/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/processfactory.hxx>
#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svdobj.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XViewCursor.hpp>
#include <comphelper/sequenceashashmap.hxx>
#include <unoprnms.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/scopeguard.hxx>
#include <officecfg/Office/Common.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(Test, testTdf143860NonPrimitiveCustomShape)
{
    loadAndReload("tdf143860_NonPrimitiveCustomShape.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The document has a custom shape of type non-primitive without handles. Make sure that the shape
    // is not exported with preset but with custom geometry.
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDocument);
    assertXPath(pXmlDocument, "//a:prstGeom"_ostr, 0);
    assertXPath(pXmlDocument, "//a:custGeom"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testWrapPolygonCurve)
{
    loadAndSave("tdf136386_WrapPolygonCurve.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Document has a curve with contour wrap and 'outside only'. Error was, that type 'square' was
    // written and no wrap polygon. Make sure we write wrapTight and a wrapPolygon.
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDocument);
    assertXPath(pXmlDocument, "//wp:wrapTight"_ostr, 1);
    assertXPath(pXmlDocument, "//wp:wrapPolygon"_ostr, 1);
    assertXPath(pXmlDocument, "//wp:start"_ostr, 1);
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDocument, "//wp:lineTo"_ostr);
    CPPUNIT_ASSERT_GREATER(sal_Int32(2),
                           static_cast<sal_Int32>(xmlXPathNodeSetGetLength(pXmlObj->nodesetval)));
    xmlXPathFreeObject(pXmlObj);
}

CPPUNIT_TEST_FIXTURE(Test, testWrapPolygonLineShape)
{
    loadAndSave("tdf136386_WrapPolygonLineShape.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Document has a sloping line with contour wrap. Error was, that type 'square' was written and
    // no wrap polygon. Now we write 'through' and use wrap polygon 0|0, 21600|21600, 0|0.
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDocument);
    assertXPath(pXmlDocument, "//wp:wrapThrough"_ostr, 1);
    assertXPath(pXmlDocument, "//wp:lineTo"_ostr, 2);
    sal_Int32 nYCoord = getXPath(pXmlDocument, "(//wp:lineTo)[1]"_ostr, "y"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21600), nYCoord);
    sal_Int32 nXCoord = getXPath(pXmlDocument, "(//wp:lineTo)[2]"_ostr, "x"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nXCoord);
}

CPPUNIT_TEST_FIXTURE(Test, testWrapPolygonCustomShape)
{
    loadAndReload("tdf142433_WrapPolygonCustomShape.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Document has 4-point star with contour wrap. Error was, that the enhanced path was written
    // literally as wrap polygon. But that does not work, because path might have links to equations
    // and handles and not only numbers.
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDocument);
    // Expected coordinates are 0|10800, 8936|8936, 10800|0, 12664|8936, 21600|10800, 12664|12664,
    // 10800|21600, 8936|12664, 0|10800. Assert forth point, which comes from equations. Allow some
    // tolerance.
    sal_Int32 nXCoord = getXPath(pXmlDocument, "(//wp:lineTo)[3]"_ostr, "x"_ostr).toInt32();
    // Without fix it would fail with expected 12664, actual 3
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12664, nXCoord, 10);
    // Without fix it would fail with expected 8936, actual 4
    sal_Int32 nYCoord = getXPath(pXmlDocument, "(//wp:lineTo)[3]"_ostr, "y"_ostr).toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8936, nYCoord, 10);
}

CPPUNIT_TEST_FIXTURE(Test, testFrameWrapTextMode)
{
    loadAndSave("tdf143432_Frame_WrapTextMode.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDocument);
    // Without the fix the value "largest" was written to file in both cases.
    assertXPath(pXmlDocument, "(//wp:wrapSquare)[1]"_ostr, "wrapText"_ostr, u"right"_ustr);
    assertXPath(pXmlDocument, "(//wp:wrapSquare)[2]"_ostr, "wrapText"_ostr, u"left"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134219ContourWrap_glow_rotate)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1461),
                                     getProperty<sal_Int32>(getShape(1), u"LeftMargin"_ustr), 2);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1302),
                                     getProperty<sal_Int32>(getShape(1), u"RightMargin"_ustr), 1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1522),
                                     getProperty<sal_Int32>(getShape(1), u"TopMargin"_ustr), 1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1296),
                                     getProperty<sal_Int32>(getShape(1), u"BottomMargin"_ustr), 1);
    };
    // Given a document with a shape with contour wrap, that has glow effect and rotation.
    createSwDoc("tdf143219ContourWrap_glow_rotate.docx");

    // Error was, that the margins, which were added on import to approximate Word's rendering of
    // contour wrap, contained the effect extent for rotation. But LibreOffice extents the wrap
    // distance automatically. The distance was too large on first load and because the extent was
    // not removed on export, much larger on reload.
    // Test fails on reload without fix with left: expected 1461 actual 2455; right: expected 1302
    // actual 4177; top: expected 1522 actual 2457; bottom: expected 1296, actual 4179
    verify();
    saveAndReload(u"Office Open XML Text"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134219ContourWrap_stroke_shadow)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(318),
                                     getProperty<sal_Int32>(getShape(1), u"LeftMargin"_ustr), 1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1164),
                                     getProperty<sal_Int32>(getShape(1), u"RightMargin"_ustr), 1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(318),
                                     getProperty<sal_Int32>(getShape(1), u"TopMargin"_ustr), 1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1164),
                                     getProperty<sal_Int32>(getShape(1), u"BottomMargin"_ustr), 1);
    };
    // Given a document with a shape with contour wrap, that has a fat stroke and large shadow.
    createSwDoc("tdf143219ContourWrap_stroke_shadow.docx");

    // Error was, that the margins, which were added on import to approximate Word's rendering of
    // contour wrap, were not removed on export and so used twice on reload.
    // Test after reload would fail without fix with
    // left, top: expected 318 actual 635; right, bottom: expected 1164 actual 2434
    verify();
    saveAndReload(u"Office Open XML Text"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123569_rotWriterImage)
{
    loadAndReload("tdf123569_rotWriterImage_46deg.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    // Error was, that position of logical rectangle was treated as position of snap rectangle.
    // Thus a wrong position was calculated.
    // Without fix this would have failed with expected 4798, actual 4860
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4798),
                                 getProperty<sal_Int32>(xFrame, u"HoriOrientPosition"_ustr), 1);
    // Without fix this would have failed with expected 1438, actual 4062
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1438),
                                 getProperty<sal_Int32>(xFrame, u"VertOrientPosition"_ustr), 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf142486_LeftMarginShadowLeft, "tdf142486_LeftMarginShadowLeft.docx")
{
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    // Error was, that the shadow distance appeared as additional margin.
    // Without fix this would have failed with expected 953, actual 2822
    // Margin is 36px (= 952.5Hmm) in Word.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(953), getProperty<sal_Int32>(xFrame, u"LeftMargin"_ustr), 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf151384Hyperlink, "151384Hyperlink.odt")
{
    loadAndSave("151384Hyperlink.odt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:hyperlink/w:r/w:rPr/w:rStyle"_ostr, "val"_ostr, u"Hyperlink"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Hyperlink']/w:name"_ostr, "val"_ostr, u"Hyperlink"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf66039, "tdf66039.docx")
{
    // This bugdoc has a groupshape (WPG) with a table inside its each member shape.
    // Before there was no table after import at all. From now, there must be 2 tables.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // This was 0 before:
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Where are the tables?!", static_cast<sal_Int32>(2),
                                 xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142486_FrameShadow)
{
    loadAndReload("tdf142486_FrameShadow.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextViewCursor> xViewCursor(xTextViewCursorSupplier->getViewCursor());
    xViewCursor->gotoStart(/*bExpand=*/false);
    uno::Reference<view::XViewCursor> xCursor(xViewCursor, uno::UNO_QUERY);
    xCursor->goDown(/*nCount=*/3, /*bExpand=*/false);
    xViewCursor->goRight(/*nCount=*/1, /*bExpand=*/true);
    OUString sText = xViewCursor->getString();
    // Without fix in place, the frame size including shadow width was exported as object size. On
    // import the shadow width was added as wrap "distance from text". That results in totally
    // different wrapping of the surrounding text.
    // Here line started with "x" instead of expected "e".
    CPPUNIT_ASSERT(sText.startsWith("e"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136059)
{
    loadAndReload("tdf136059.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Contour has not been exported!", true,
        getProperty<bool>(getShape(1), u"SurroundContour"_ustr));
    // With the fix this shall pass, see tdf136059.
}

DECLARE_OOXMLEXPORT_TEST(testTdf138892_noNumbering, "tdf138892_noNumbering.docx")
{
    CPPUNIT_ASSERT_MESSAGE("Para1: Bullet point", !getProperty<OUString>(getParagraph(1), u"NumberingStyleName"_ustr).isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Para2: <blank line>", getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr).isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Para3: <blank line>", getProperty<OUString>(getParagraph(3), u"NumberingStyleName"_ustr).isEmpty());
}

DECLARE_OOXMLEXPORT_TEST(testTdf44278, "tdf44278.docx")
{
    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf137742, "tdf137742.docx")
{
    lang::Locale locale(
        getProperty<lang::Locale>(getParagraph(1), u"CharLocale"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"en"_ustr, locale.Language);

    // Without the fix in place, this test would have failed with
    // - Expected: US
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(u"US"_ustr, locale.Country);
}

DECLARE_OOXMLEXPORT_TEST(testTdf141231_arabicHebrewNumbering, "tdf141231_arabicHebrewNumbering.docx")
{
    // The page's numbering type: instead of Hebrew, this was default style::NumberingType::ARABIC (4).
    auto nActual = getProperty<sal_Int16>(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"NumberingType"_ustr);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_HEBREW, nActual);

    // The footnote numbering type: instead of arabicAbjad, this was the default style::NumberingType::ARABIC.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings = xFootnotesSupplier->getFootnoteSettings();
    nActual = getProperty<sal_Int16>(xFootnotesSupplier->getFootnoteSettings(), u"NumberingType"_ustr);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::CHARS_ARABIC_ABJAD, nActual);
}

DECLARE_OOXMLEXPORT_TEST(testTdf141966_chapterNumbering, "tdf141966_chapterNumbering.docx")
{
    uno::Reference<text::XChapterNumberingSupplier> xNumberingSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xNumberingRules = xNumberingSupplier->getChapterNumberingRules();
    comphelper::SequenceAsHashMap hashMap(xNumberingRules->getByIndex(0));

    CPPUNIT_ASSERT(hashMap[u"HeadingStyleName"_ustr].get<OUString>().match("CN1"));

    uno::Reference<beans::XPropertySet> xPara(getParagraph(7, u"Direct formatting with \"Outline\" numbering."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"2nd"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf141966_chapterNumberTortureTest, "tdf141966_chapterNumberTortureTest.docx")
{
    // There is no point in identifying what the wrong values where in this test,
    //because EVERYTHING was wrong, and MANY different fixes are required to solve the problems.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1, u"No numId in style or paragraph"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));

    xPara.set(getParagraph(2, u"Paragraph cancels numbering(0)"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));

    xPara.set(getParagraph(3, u"First numbered line"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1st.i.a.1.I"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));

    xPara.set(getParagraph(5, u"Outline with listLvl 5"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));

    xPara.set(getParagraph(7, u"inheritOnly: inherit outlineLvl and listLvl."_ustr), uno::UNO_QUERY);
    // 2nd.iii in MS Word 2003.  2nd.ii in MS Word 2010/2016 where para5 is not numbered.
    CPPUNIT_ASSERT_EQUAL(u"2nd.ii"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr)); // Level 2

    xPara.set(getParagraph(9, u"outline with Body listLvl(9)."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));

    xPara.set(getParagraph(10, u"outline with Body listLvl(9) #2."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));

    xPara.set(getParagraph(11, u"direct formatting - Body listLvl(9)."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));

    xPara.set(getParagraph(12, u"direct numId, inherit listLvl."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"2nd.ii.a.1.I"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(4), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr)); // Level 5

    xPara.set(getParagraph(13, u"Style numId0 cancels inherited numbering."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf143692_outlineLevelTortureTest, "tdf143692_outlineLevelTortureTest.docx")
{
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1, u"Head non Toc style"_ustr), uno::UNO_QUERY);
    // fixed missing inherit from Heading 1
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xPara, u"OutlineLevel"_ustr));

    xPara.set(getParagraph(2, u"noInheritHeading1"_ustr), uno::UNO_QUERY);
    // fixed body level not cancelling inherited level
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xPara, u"OutlineLevel"_ustr));

    xPara.set(getParagraph(4, u"illegal -3"_ustr), uno::UNO_QUERY);
    // illegal value is ignored, so inherit from Heading 1
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xPara, u"OutlineLevel"_ustr));

    xPara.set(getParagraph(5, u"Heading 1 with invalid direct -2"_ustr), uno::UNO_QUERY);
    // fixed illegal does not mean body level, it means inherit from style.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xPara, u"OutlineLevel"_ustr));

    xPara.set(getParagraph(7, u"InheritCN3"_ustr), uno::UNO_QUERY);
    // fixed Chapter Numbering cancelling inheritance
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), getProperty<sal_Int16>(xPara, u"OutlineLevel"_ustr));

    xPara.set(getParagraph(8, u"noInheritCN3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xPara, u"OutlineLevel"_ustr));

    xPara.set(getParagraph(9, u"override6CN3"_ustr), uno::UNO_QUERY);
    // fixed body level not cancelling inherited level
    CPPUNIT_ASSERT_EQUAL(sal_Int16(6), getProperty<sal_Int16>(xPara, u"OutlineLevel"_ustr));

    xPara.set(getParagraph(10, u"illegal 25"_ustr), uno::UNO_QUERY);
    // fixed illegal value is ignored, so inherit from List Level (Chapter Numbering)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), getProperty<sal_Int16>(xPara, u"OutlineLevel"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf132752, "tdf132752.docx")
{
    uno::Reference<beans::XPropertySet> xPara1(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1801), getProperty<sal_Int32>(xPara1, u"ParaLeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(xPara1, u"ParaRightMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara1, u"ParaFirstLineIndent"_ustr));

    uno::Reference<beans::XPropertySet> xPara2(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1801), getProperty<sal_Int32>(xPara2, u"ParaLeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(xPara2, u"ParaRightMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-630), getProperty<sal_Int32>(xPara2, u"ParaFirstLineIndent"_ustr));

    uno::Reference<beans::XPropertySet> xPara3(getParagraph(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara3, u"ParaLeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5891), getProperty<sal_Int32>(xPara3, u"ParaRightMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara3, u"ParaFirstLineIndent"_ustr));

    uno::Reference<beans::XPropertySet> xPara4(getParagraph(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1801), getProperty<sal_Int32>(xPara4, u"ParaLeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(xPara4, u"ParaRightMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4157), getProperty<sal_Int32>(xPara4, u"ParaFirstLineIndent"_ustr));

    uno::Reference<beans::XPropertySet> xPara5(getParagraph(5), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1801), getProperty<sal_Int32>(xPara5, u"ParaLeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(xPara5, u"ParaRightMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-630), getProperty<sal_Int32>(xPara5, u"ParaFirstLineIndent"_ustr));

    uno::Reference<beans::XPropertySet> xPara6(getParagraph(6), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3565), getProperty<sal_Int32>(xPara6, u"ParaLeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2764), getProperty<sal_Int32>(xPara6, u"ParaRightMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2394), getProperty<sal_Int32>(xPara6, u"ParaFirstLineIndent"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testGutterLeft, "gutter-left.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle;
    getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr) >>= xPageStyle;
    sal_Int32 nGutterMargin{};
    xPageStyle->getPropertyValue(u"GutterMargin"_ustr) >>= nGutterMargin;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1270
    // - Actual  : 0
    // i.e. gutter margin was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), nGutterMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testGutterTop)
{
    loadAndSave("gutter-top.docx");
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    CPPUNIT_ASSERT(pXmlSettings);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. <w:gutterAtTop> was lost.
    assertXPath(pXmlSettings, "/w:settings/w:gutterAtTop"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testCustomShapePresetExport)
{
    loadAndReload("testCustomShapePresetExport.odt");
    // Check if the load failed.
    CPPUNIT_ASSERT(getPages());

    // Check all shapes of the file
    int nCount = 0;
    for (int i = 1; i <= getShapes(); i++)
    {
        uno::Reference<beans::XPropertySet> xProperties(getShape(i), uno::UNO_QUERY);
        if (!xProperties->getPropertySetInfo()->hasPropertyByName(u"CustomShapeGeometry"_ustr))
            continue;
        // Get the custom shape property
        auto aCustomShapeGeometry = xProperties->getPropertyValue(u"CustomShapeGeometry"_ustr)
                                        .get<uno::Sequence<beans::PropertyValue>>();
        // Find for shape type
        for (const auto& aCustomGeometryIterator : aCustomShapeGeometry)
        {
            if (aCustomGeometryIterator.Name == "Type")
                CPPUNIT_ASSERT_MESSAGE(
                    "This is an ooxml preset shape with custom geometry! Shape type lost!",
                    aCustomGeometryIterator.Value.get<OUString>() != "ooxml-non-primitive");
            // Without the fix, all shapes have ooxml-non-primitive type, and lost their
            // real type (like triangle) with the textbox padding.
        }
        nCount++;
    }
    // Without the fix the count does not match.
    CPPUNIT_ASSERT_EQUAL(17, nCount);
}

DECLARE_OOXMLEXPORT_TEST(testTdf148671, "tdf148671.docx")
{
    // Don't assert with 'pFieldMark' failed when document is opened
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    if (!isExported())
        return;
    // Preserve tag on SDT blocks. (Before the fix, these were all lost)
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:tag"_ostr, 3);
}

DECLARE_OOXMLEXPORT_TEST(testTdf140668, "tdf140668.docx")
{
    // Don't crash when document is opened
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf149649, "tdf149649.docx")
{
    // Don't crash when document is opened
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf138771, "tdf138771.docx")
{
    // Don't crash when document is imported
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf125936_numberingSuperscript, "tdf125936_numberingSuperscript.docx")
{
    // Without the fix, the first character run was superscripted.
    CPPUNIT_ASSERT_EQUAL( sal_Int16(0), getProperty<sal_Int16>(getRun(getParagraph(1), 1, u"A-570-108"_ustr), u"CharEscapement"_ustr) );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134619_numberingProps)
{
    loadAndReload("tdf134619_numberingProps.doc");
    // Get the third paragraph's numbering style's 1st level's bullet size
    uno::Reference<text::XTextRange> xParagraph = getParagraph(3);
    auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, u"NumberingRules"_ustr);
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(0) >>= aLevel; // 1st level
    OUString aCharStyleName = std::find_if(std::cbegin(aLevel), std::cend(aLevel), [](const beans::PropertyValue& rValue) { return rValue.Name == "CharStyleName"; })->Value.get<OUString>();

    // Make sure that the blue bullet's font size is 72 points, not 12 points.
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"CharacterStyles"_ustr)->getByName(aCharStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(72.f, getProperty<float>(xStyle, u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134951_duplicates)
{
    loadAndReload("tdf134951_duplicates.odt");
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xEndnotesSupplier->getEndnotes()->getCount());

    getParagraph(5, u"Duplicate fields: 1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135773_numberingShading)
{
    bool bIsExportAsShading = !officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::get();
    auto batch = comphelper::ConfigurationChanges::create();

    // This function is run at the end of the test - returning the filter options to normal.
    comphelper::ScopeGuard g(
        [bIsExportAsShading, batch]
        {
            if (bIsExportAsShading)
            {
                officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::set(false, batch);
                batch->commit();
            }
        });
    // For these test, ensure exporting CharBackground as w:highlight.
    officecfg::Office::Common::Filter::Microsoft::Export::CharBackgroundToHighlighting::set(true, batch);
    batch->commit();

    loadAndSave("tdf135774_numberingShading.docx");
    // This test uses a custom setting to export CharBackground as Highlight instead of the 7.0 default of Shading.

    // Before the fix, the imported shading was converted into a red highlight.
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/numbering.xml"_ustr);
    assertXPath(pXmlStyles, "/w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:shd"_ostr, "fill"_ostr, u"ED4C05"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf140336_paraNoneShading)
{
    loadAndReload("tdf140336_paraNoneShading.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Before the fix, the background from a style was exported to dis-inheriting paragraphs/styles.
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(COL_AUTO), getProperty<sal_uInt32>(getParagraph(1), u"ParaBackColor"_ustr));
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"CanclledBackground"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xStyle, u"FillStyle"_ustr));

    // sanity check: backgroundColor paragraph style has a golden color(FF7F50), which para2 inherits
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(16744272), getProperty<sal_uInt32>(getParagraph(2), u"ParaBackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141173_missingFrames)
{
    loadAndReload("tdf141173_missingFrames.rtf");
    // Without the fix in place, almost all of the text and textboxes were missing.
    // Without the fix, there were only 2 shapes (mostly unseen).
    CPPUNIT_ASSERT_EQUAL(13, getShapes());
}

DECLARE_OOXMLEXPORT_TEST(testTdf142404_tabSpacing, "tdf142404_tabSpacing.docx")
{
    // The tabstops should be laid out as triple-spaced when the paragraph takes multiple lines.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("too big for one page", 2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf142404_tabOverMarginC15, "tdf142404_tabOverMarginC15.docx")
{
    // TabOverMargin no longer applies to compatibilityMode 15 DOCX files. In Word 2016 this is 3pg.
    // One page long if tabOverMargin is true. Two pages long if tabOverflow is true.
    // Really should be 3 pages long, when tabOverflow is also false, but inadequate implementation.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("too big for one page", 2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142404_tabOverSpacingC15)
{
    loadAndReload("tdf142404_tabOverSpacingC15.odt");
    // Although TabOverMargin no longer applies to compatibilityMode 15 DOCX files,
    // it still applies to a tab over the paragraph end (inside text boundaries).
    // The original 3-page ODT saved as DOCX would fit on one page in MS Word 2010, but 3 in Word 2013.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("too big for two pages", 3, getPages());
    // The tab goes over the paragraph margin
    CPPUNIT_ASSERT_EQUAL(u"A left tab positioned at"_ustr, parseDump("//page[1]/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwLinePortion[1]"_ostr, "portion"_ostr));
    sal_Int32 nTextLen = parseDump("//page[1]/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwLinePortion[1]"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(u"*"_ustr, parseDump("//page[1]/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwFixPortion[1]"_ostr, "portion"_ostr));
    sal_Int32 nTabLen = parseDump("//page[1]/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwFixPortion[1]"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_MESSAGE("Large left tab", nTextLen < nTabLen);
    // Not 1 line high (Word 2010 DOCX), or 3 lines high (LO DOCX) or 5 lines high (ODT), but 4 lines high
    sal_Int32 nHeight = parseDump("//page[1]/body/txt[2]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_MESSAGE("4 lines high", 1100 < nHeight);
    CPPUNIT_ASSERT_MESSAGE("4 lines high", nHeight < 1300);

    CPPUNIT_ASSERT_EQUAL(u"TabOverflow does what?"_ustr, parseDump("//page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]/SwLinePortion[1]"_ostr, "portion"_ostr));
    // Not 1 line high (Word 2010 DOCX), or 4 lines high (prev LO DOCX) or 8 lines high (ODT).
    // but two lines high. (3 in Word 2016 because it pulls down "what?" to the second line - weird)
    nHeight = parseDump("//page[1]/body/txt[7]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("2 lines high (but 3 in Word)", 242*2.5, nHeight, 242);

    CPPUNIT_ASSERT_EQUAL(u"A centered tab positioned at"_ustr, parseDump("//page[1]/body/txt[3]/SwParaPortion/SwLineLayout[1]/SwLinePortion[1]"_ostr, "portion"_ostr));
    sal_Int32 nLineWidth = parseDump("//page[1]/body/txt[3]/SwParaPortion/SwLineLayout[1]/SwFixPortion[1]"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Big tab: full paragraph area used", 737, nLineWidth, 100);

    // Pages 2/3 are TabOverMargin - in this particular case tabs should not go over margin.
    CPPUNIT_ASSERT_EQUAL(u"A right tab positioned at"_ustr, parseDump("//page[2]/body/txt[6]/SwParaPortion/SwLineLayout[1]/SwLinePortion[1]"_ostr, "portion"_ostr));
    sal_Int32 nParaWidth = parseDump("//page[2]/body/txt[6]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    // the clearest non-first-line visual example is this second tab in the right-tab paragraph.
    nLineWidth = parseDump("//page[2]/body/txt[6]/SwParaPortion/SwLineLayout[4]"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Full paragraph area used", nLineWidth, nParaWidth);

    CPPUNIT_ASSERT_EQUAL(u"TabOverflow does what?"_ustr, parseDump("//page[3]/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwLinePortion[1]"_ostr, "portion"_ostr));
    // Not 1 line high (Word 2010 DOCX and ODT), or 4 lines high (prev LO DOCX),
    // but 8 lines high.
    nHeight = parseDump("//page[3]/body/txt[2]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("8 lines high", 242*8, nHeight, 121);
}

DECLARE_OOXMLEXPORT_TEST(testShapeHyperlink, "hyperlinkshape.docx")
{
    // Test import/export of hyperlink property on shapes
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_EQUAL(u"https://libreoffice.org/"_ustr, getProperty<OUString>(xShape, u"Hyperlink"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTextframeHyperlink)
{
    // Make sure hyperlink is imported correctly
    createSwDoc("docxopenhyperlinkbox.docx");
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"https://libreoffice.org/"_ustr, getProperty<OUString>(xFrame, u"HyperLinkURL"_ustr));

    // FIXME: After save&reload, the text frame should still be a text frame, and the above test should still work.
    // (Currently the Writer text frame becomes a text box (shape based)). See tdf#140961
    saveAndReload(u"Office Open XML Text"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // DML
    assertXPath(pXmlDoc, "//w:drawing/wp:anchor/wp:docPr/a:hlinkClick"_ostr, 1);
    // VML
    assertXPath(pXmlDoc, "//w:pict/v:rect"_ostr, "href"_ostr, u"https://libreoffice.org/"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146171_invalid_change_date)
{
    createSwDoc("tdf146171.docx");
    // false alarm? during ODF roundtrip:
    // 'Error: "1970-01-01" does not satisfy the "dateTime" type'
    // disable and check only the conversion of the invalid (zeroed) change date
    // 0000-00-00T00:00:00Z, resulting loss of change tracking during ODF roundtrip
    // reload("writer8", "tdf146171.odt");
    saveAndReload(u"Office Open XML Text"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 0
    assertXPath(pXmlDoc, "//w:ins"_ostr, 5);
    // This was 0
    assertXPath(pXmlDoc, "//w:del"_ostr, 2);
    // no date (anonymized change)
    // This failed, date was exported as w:date="1970-01-01T00:00:00Z" before fixing tdf#147760
    assertXPathNoAttribute(pXmlDoc, "//w:del[1]"_ostr, "date"_ostr);
    assertXPathNoAttribute(pXmlDoc, "//w:del[2]"_ostr, "date"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf139580)
{
    loadAndReload("tdf139580.odt");
    // Without the fix in place, this test would have crashed at export time
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf149198, "tdf149198.docx")
{
    // Without the fix in place, this test would have crashed at export time
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(u"Documentation"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testFooterMarginLost)
{
    loadAndSave("footer-margin-lost.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 709
    // - Actual  : 0
    // i.e. import + export lost the footer margin value.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgMar"_ostr, "footer"_ostr, u"709"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testEffectExtentLineWidth)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(506),
                             getProperty<sal_Int32>(getShape(1), u"TopMargin"_ustr));
    };

    // Given a document with a shape that has a non-zero line width and effect extent:
    // When loading the document:
    createSwDoc("effect-extent-line-width.docx");
    // Then make sure that the line width is not taken twice (once as part of the margin, and then
    // also as the line width):
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 508
    // - Actual  : 561
    // i.e. the upper spacing was too large, the last line of the text moved below the shape.
    verify();
    saveAndReload(u"Office Open XML Text"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testRtlGutter)
{
    // Given a document with RTL gutter:
    createSwDoc("rtl-gutter.docx");
    uno::Reference<beans::XPropertySet> xStandard(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr),
            uno::UNO_QUERY);
    CPPUNIT_ASSERT(getProperty<bool>(xStandard, u"RtlGutter"_ustr));

    // When saving back to DOCX:
    saveAndReload(u"Office Open XML Text"_ustr);

    // Then make sure the section's gutter is still RTL:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed as the XML element was
    // missing.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:rtlGutter"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf140572_docDefault_superscript)
{
    loadAndReload("tdf140572_docDefault_superscript.docx");
    // A round-trip was crashing.

    // Without the fix, everything was DFLT_ESC_AUTO_SUPER (default superscript)
    CPPUNIT_ASSERT_EQUAL( sal_Int16(0), getProperty<sal_Int16>(getRun(getParagraph(1), 1), u"CharEscapement"_ustr) );
}

DECLARE_OOXMLEXPORT_TEST(testTdf136841, "tdf136841.docx")
{
    if (!IsDefaultDPI())
        return;
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> graphic;
    imageProperties->getPropertyValue( u"Graphic"_ustr ) >>= graphic;
    Graphic vclGraphic(graphic);
    BitmapEx bitmap(vclGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL( tools::Long(76), bitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL( tools::Long(76), bitmap.GetSizePixel().Height());

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:228 G:71 B:69 A:0
    // - Actual  : Color: R:0 G:0 B:0 A:0
    CPPUNIT_ASSERT_EQUAL( Color(228,71,69), bitmap.GetPixelColor(38,38));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138953)
{
    loadAndReload("croppedAndRotated.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_EQUAL(27000.0, getProperty<double>(xShape, u"RotateAngle"_ustr));
    auto frameRect = getProperty<css::awt::Rectangle>(xShape, u"FrameRect"_ustr);
    // Before the fix, original object size (i.e., before cropping) was written to spPr in OOXML,
    // and the resulting object size was much larger than should be.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12961), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8664), frameRect.Width);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf118535)
{
    loadAndReload("tdf118535.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName(u"word/media/image1.jpeg"_ustr)));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: false
    // - Actual  : true
    // i.e. the embedded picture would have been saved twice.
    CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName(u"word/media/image2.jpeg"_ustr)));
}

DECLARE_OOXMLEXPORT_TEST(testTdf133473_shadowSize, "tdf133473.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);

    SdrObject* pObj(SdrObject::getSdrObjectFromXShape(xShape));
    const SfxItemSet& rSet = pObj->GetMergedItemSet();
    sal_Int32 nSize1 = rSet.Get(SDRATTR_SHADOWSIZEX).GetValue();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 200000
    // - Actual  : 113386
    // I.e. Shadow size will be smaller than actual.

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(200000), nSize1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf153874, "image_through_shape.docx")
{
    uno::Reference<beans::XPropertySet> const xShape1(getShapeByName(u"Test1"),  uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> const xShape2(getShapeByName(u"Rectangle 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, xShape1->getPropertyValue(u"AnchorType"_ustr).get<text::TextContentAnchorType>());
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, xShape2->getPropertyValue(u"AnchorType"_ustr).get<text::TextContentAnchorType>());
    CPPUNIT_ASSERT_LESS(xShape2->getPropertyValue(u"ZOrder"_ustr).get<sal_uInt64>(), xShape1->getPropertyValue(u"ZOrder"_ustr).get<sal_uInt64>());
    CPPUNIT_ASSERT(xShape1->getPropertyValue(u"Decorative"_ustr).get<bool>());
    // not implemented on shape yet
    //CPPUNIT_ASSERT(xShape2->getPropertyValue("Decorative").get<bool>());
}

DECLARE_OOXMLEXPORT_TEST(testTextBoxZOrder, "testTextBoxZOrder.docx")
{
    // Is load successful?
    CPPUNIT_ASSERT(mxComponent);
    // Collect the z-order values of the textboxes
    std::vector<sal_uInt64> ShapeZorders;
    std::vector<sal_uInt64> FrameZorders;
    for (int i = 1; i < 4; i++)
    {
        uno::Reference<drawing::XShape> xShape(getShape(i));
        CPPUNIT_ASSERT(xShape);
        uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapeProperties);
        uno::Reference<text::XTextFrame> xFrame = SwTextBoxHelper::getUnoTextFrame(xShape);
        CPPUNIT_ASSERT(xFrame.is());
        uno::Reference<beans::XPropertySet> const xFrameProperties(xFrame, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xFrameProperties);
        ShapeZorders.push_back(xShapeProperties->getPropertyValue(u"ZOrder"_ustr).get<sal_uInt64>());
        FrameZorders.push_back(xFrameProperties->getPropertyValue(u"ZOrder"_ustr).get<sal_uInt64>());
    }
    // Check the z-order values.
    for (int i = 1; i < 3; i++)
    {
        CPPUNIT_ASSERT_GREATER(ShapeZorders[i - 1], ShapeZorders[i]);
        CPPUNIT_ASSERT_GREATER(FrameZorders[i - 1], FrameZorders[i]);
        CPPUNIT_ASSERT_GREATER(ShapeZorders[i - 1], FrameZorders[i - 1]);
    }
    // Without the fix it failed, because the z-order was wrong.
}

DECLARE_OOXMLEXPORT_TEST(testTdf141550, "tdf141550.docx")
{
    uno::Reference<drawing::XShape> xShape(getShape(1));
    uno::Reference<text::XTextFrame> xFrame = SwTextBoxHelper::getUnoTextFrame(xShape);

    CPPUNIT_ASSERT(xShape);
    CPPUNIT_ASSERT(xFrame);

    const sal_uInt16 nShapeRelOri = getProperty<sal_uInt16>(xShape, UNO_NAME_HORI_ORIENT_RELATION);
    const sal_uInt16 nFrameRelOri = getProperty<sal_uInt16>(xFrame, UNO_NAME_HORI_ORIENT_RELATION);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Textbox fallen apart!", nShapeRelOri, nFrameRelOri);
    // Without the fix in place it fails with difference.
}

DECLARE_OOXMLEXPORT_TEST(testTdf140137, "tdf140137.docx")
{
    // Don't throw exception during load
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf105688, "tdf105688.docx")
{
    // Don't throw exception during load
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testCommentReply)
{
    loadAndSave("CommentReply.docx");
    xmlDocUniquePtr pXmlComm = parseExport(u"word/comments.xml"_ustr);
    xmlDocUniquePtr pXmlCommExt = parseExport(u"word/commentsExtended.xml"_ustr);
    CPPUNIT_ASSERT(pXmlComm);
    CPPUNIT_ASSERT(pXmlCommExt);
    OUString sParentId = getXPath(pXmlComm, "/w:comments/w:comment[1]/w:p[1]"_ostr, "paraId"_ostr);
    OUString sChildId = getXPath(pXmlComm, "/w:comments/w:comment[2]/w:p[1]"_ostr, "paraId"_ostr);
    OUString sChildIdEx = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx"_ostr, "paraId"_ostr);
    OUString sChildParentId = getXPath(pXmlCommExt,
        "/w15:commentsEx/w15:commentEx"_ostr, "paraIdParent"_ostr);
    // Make sure exported extended paraId matches the one in comments.xml
    CPPUNIT_ASSERT_EQUAL(sChildId, sChildIdEx);
    // Make sure the paraIdParent matches the id of its parent
    CPPUNIT_ASSERT_EQUAL(sParentId, sChildParentId);
}

CPPUNIT_TEST_FIXTURE(Test, testCommentDone)
{
    loadAndSave("CommentDone.docx");
    xmlDocUniquePtr pXmlComm = parseExport(u"word/comments.xml"_ustr);
    assertXPath(pXmlComm, "/w:comments/w:comment[1]/w:p"_ostr, 2);
    OUString idLastPara = getXPath(pXmlComm, "/w:comments/w:comment[1]/w:p[2]"_ostr, "paraId"_ostr);
    xmlDocUniquePtr pXmlCommExt = parseExport(u"word/commentsExtended.xml"_ustr);
    assertXPath(pXmlCommExt, "/w15:commentsEx"_ostr, "Ignorable"_ostr, u"w15"_ustr);
    assertXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx"_ostr, 1);
    OUString idLastParaEx = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx"_ostr, "paraId"_ostr);
    CPPUNIT_ASSERT_EQUAL(idLastPara, idLastParaEx);
    assertXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx"_ostr, "done"_ostr, u"1"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTableWidth, "frame_size_export.docx")
{
    // after exporting: table width was overwritten in the doc model
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<sal_Int16>(xTables->getByIndex(0), u"RelativeWidth"_ustr));
}


DECLARE_OOXMLEXPORT_TEST(testCommentDoneModel, "CommentDone.docx")
{
    css::uno::Reference<css::text::XTextFieldsSupplier> xTextFieldsSupplier(
        mxComponent, css::uno::UNO_QUERY_THROW);
    auto xFields(xTextFieldsSupplier->getTextFields()->createEnumeration());

    // First comment: initially resolved, toggled to unresolved on import, unresolved on roundtrip
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    css::uno::Any aComment = xFields->nextElement();
    css::uno::Reference<css::beans::XPropertySet> xComment(aComment, css::uno::UNO_QUERY_THROW);

    if (!isExported())
    {
        // Check that it's resolved when initially read
        CPPUNIT_ASSERT_EQUAL(true, xComment->getPropertyValue(u"Resolved"_ustr).get<bool>());
        // Set to unresolved
        xComment->setPropertyValue(u"Resolved"_ustr, css::uno::Any(false));
    }
    else
    {
        // After the roundtrip, it keeps the "unresolved" state set above
        CPPUNIT_ASSERT_EQUAL(false, xComment->getPropertyValue(u"Resolved"_ustr).get<bool>());
    }

    // Second comment: initially unresolved, toggled to resolved on import, resolved on roundtrip
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    aComment = xFields->nextElement();
    xComment.set(aComment, css::uno::UNO_QUERY_THROW);

    if (!isExported())
    {
        // Check that it's unresolved when initially read
        CPPUNIT_ASSERT_EQUAL(false, xComment->getPropertyValue(u"Resolved"_ustr).get<bool>());
        // Set to resolved
        xComment->setPropertyValue(u"Resolved"_ustr, css::uno::Any(true));
    }
    else
    {
        // After the roundtrip, it keeps the "resolved" state set above
        CPPUNIT_ASSERT_EQUAL(true, xComment->getPropertyValue(u"Resolved"_ustr).get<bool>());
    }
}

CPPUNIT_TEST_FIXTURE(Test, Test_ShadowDirection)
{
    loadAndSave("tdf142361ShadowDirection.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The attribute 'rotWithShape' has the default value 'true' in OOXML, so Words interprets a
    // missing attribute as 'true'. That means that Word rotates the shadow if the shape is
    // rotated. Because in LibreOffice a shadow is never rotated, we must not omit this
    // attribute.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw"_ostr,
                "rotWithShape"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150542)
{
    loadAndSave("tdf150542.docx");

    xmlDocUniquePtr pSettingsDoc = parseExport(u"word/settings.xml"_ustr);
    // Ensure that all docvars from input are written back and with correct values.
    // Order of document variables is not checked. So this can fail at some time if order is changed.
    assertXPath(pSettingsDoc,
                "/w:settings/w:docVars/w:docVar[1]"_ostr, "name"_ostr, u"LocalChars\u00C1\u0072\u0076\u00ED\u007A\u0074\u0075\u0072\u006F\u0054\u00FC\u006B\u00F6\u0072\u0066\u00FA\u0072\u00F3\u0067\u00E9\u0070"_ustr);
    assertXPath(pSettingsDoc,
                "/w:settings/w:docVars/w:docVar[1]"_ostr, "val"_ostr, u"Correct value (\u00E1\u0072\u0076\u00ED\u007A\u0074\u0075\u0072\u006F\u0020\u0074\u00FC\u006B\u00F6\u0072\u0066\u00FA\u0072\u00F3\u0067\u00E9\u0070)"_ustr);
    assertXPath(pSettingsDoc,
                "/w:settings/w:docVars/w:docVar[2]"_ostr, "name"_ostr, u"DocVar1"_ustr);
    assertXPath(pSettingsDoc,
                "/w:settings/w:docVars/w:docVar[2]"_ostr, "val"_ostr, u"DocVar1 Value"_ustr);
    assertXPath(pSettingsDoc,
                "/w:settings/w:docVars/w:docVar[3]"_ostr, "name"_ostr, u"DocVar3"_ustr);
    assertXPath(pSettingsDoc,
                "/w:settings/w:docVars/w:docVar[3]"_ostr, "val"_ostr, u"DocVar3 Value"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf139549)
{
    loadAndSave("tdf139549.docx");
    // Document contains a VML textbox, the position of the textbox was incorrect.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    OUString aStyle = getXPath(pXmlDoc, "//w:pict/v:shape"_ostr, "style"_ostr);
    /* original is: "position:absolute;margin-left:138.5pt;margin-top:40.1pt;width:183pt;
                     height:68pt;z-index:251675648;mso-position-horizontal:absolute;
                     mso-position-horizontal-relative:page;mso-position-vertical:absolute;
                     mso-position-vertical-relative:page" */
    CPPUNIT_ASSERT(!aStyle.isEmpty());

    sal_Int32 nextTokenPos = 0;
    OUString aStyleCommand = aStyle.getToken(0, ';', nextTokenPos);
    CPPUNIT_ASSERT(!aStyleCommand.isEmpty());

    OUString aStyleCommandName = aStyleCommand.getToken(0, ':');
    OUString aStyleCommandValue = aStyleCommand.getToken(1, ':');
    CPPUNIT_ASSERT_EQUAL(u"position"_ustr, aStyleCommandName);
    CPPUNIT_ASSERT_EQUAL(u"absolute"_ustr, aStyleCommandValue);

    aStyleCommand = aStyle.getToken(0, ';', nextTokenPos);
    CPPUNIT_ASSERT(!aStyleCommand.isEmpty());

    aStyleCommandName = aStyleCommand.getToken(0, ':');
    aStyleCommandValue = aStyleCommand.getToken(1, ':');
    CPPUNIT_ASSERT_EQUAL(u"margin-left"_ustr, aStyleCommandName);
    // Without the fix it failed, because the margin-left was 171.85pt.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(138.5, aStyleCommandValue.toFloat(), 0.1);

    aStyleCommand = aStyle.getToken(0, ';', nextTokenPos);
    CPPUNIT_ASSERT(!aStyleCommand.isEmpty());

    aStyleCommandName = aStyleCommand.getToken(0, ':');
    aStyleCommandValue = aStyleCommand.getToken(1, ':');
    CPPUNIT_ASSERT_EQUAL(u"margin-top"_ustr, aStyleCommandName);
    // Without the fix it failed, because the margin-top was 55.45pt.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(40.1, aStyleCommandValue.toFloat(), 0.1);
}


CPPUNIT_TEST_FIXTURE(Test, testTdf143726)
{
    loadAndReload("Simple-TOC.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    CPPUNIT_ASSERT(pXmlStyles);
    // Without the fix this was "TOA Heading" which belongs to the "Table of Authorities" index in Word
    // TOC's heading style should be exported as "TOC Heading" as that's the default Word style name
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOCHeading']/w:name"_ostr, "val"_ostr, u"TOC Heading"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152153)
{
    loadAndReload("embedded_images.odt");

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    const uno::Sequence<OUString> aNames(xNameAccess->getElementNames());
    int nImageFiles = 0;
    for (const auto& rElementName : aNames)
        if (rElementName.startsWith("word/media/image"))
            nImageFiles++;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 3
    // i.e. the once embedded picture wouldn't have been saved.
    CPPUNIT_ASSERT_EQUAL(4, nImageFiles);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf152152)
{
    loadAndReload("artistic_effects.docx");

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    const uno::Sequence<OUString> aNames(xNameAccess->getElementNames());
    int nImageFiles = 0;
    for (const auto& rElementName : aNames)
        if (rElementName.startsWith("word/media/hdphoto"))
            nImageFiles++;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the once WDP picture wouldn't have been saved.
    CPPUNIT_ASSERT_EQUAL(2, nImageFiles);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
