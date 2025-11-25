/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/Pair.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

#include <comphelper/configuration.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/propertyvalue.hxx>
#include <officecfg/Office/Common.hxx>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <vcl/gdimtf.hxx>

#include <set>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf165642_glossaryFootnote)
{
    // FIXME: validation error in OOXML export
    skipValidation();

    createSwDoc("tdf165642_glossaryFootnote.docx");
    save(TestFilter::DOCX);
    // round-trip'ing the settings.xml file as is, it contains footnote/endnote references
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/glossary/settings.xml"_ustr);
    assertXPath(pXmlSettings, "//w:endnotePr", 1);
    assertXPath(pXmlSettings, "//w:footnotePr", 1);

    // thus, the footnote and endnote files must also be round-tripped
    parseExport(u"word/glossary/endnotes.xml"_ustr);
    parseExport(u"word/glossary/footnotes.xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf166201_simplePos)
{
    // Given a document with an image at the bottom-right placed there by simplePos

    createSwDoc("tdf166201_simplePos.docx");
    save(TestFilter::DOCX);

    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
    // Without the fix, this was 0 - at the top left, instead of 10.5cm - at the bottom right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10478),
                         getProperty<sal_Int32>(getShape(1), u"HoriOrientPosition"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf166201_simplePosCM)
{
    // Given a document with an image at the bottom-right placed there by simplePos - in cm

    createSwDoc("tdf166201_simplePosCM.docx");
    save(TestFilter::DOCX);

    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), u"VertOrientRelation"_ustr));
    // Without the fix, this was 0 - at the top left, instead of 5cm - at the bottom right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001),
                         getProperty<sal_Int32>(getShape(1), u"VertOrientPosition"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf167770_marginInsideOutside, "tdf167770_marginInsideOutside.docx")
{
    // given shapes and pictures that use inside / outside with the two options margin and page
    uno::Reference<drawing::XShape> xShape(getShapeByName(u"Picture 1")); // page 1
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT,
                         getProperty<sal_Int16>(xShape, u"HoriOrient"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(xShape, u"PageToggle"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, u"LeftMargin"_ustr));

    xShape.set(getShapeByName(u"Right Arrow 2")); // page 1
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT,
                         getProperty<sal_Int16>(xShape, u"HoriOrient"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(xShape, u"PageToggle"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, u"RightMargin"_ustr));

    xShape.set(getShapeByName(u"Picture 4")); // page 2
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT,
                         getProperty<sal_Int16>(xShape, u"HoriOrient"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(xShape, u"PageToggle"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, u"RightMargin"_ustr));

    xShape.set(getShapeByName(u"Right Arrow 3")); // page 2
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT,
                         getProperty<sal_Int16>(xShape, u"HoriOrient"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(xShape, u"PageToggle"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, u"LeftMargin"_ustr));

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Note: emulation has decided to remove the mirrored gap completely.
    // It works well for this document, but obviously wouldn't work for one that depends on margins.
    // This part of the test is just documenting the historical decision
    // (which is now expanded to more consistently cover shapes as well as the OUTSIDE case).
    const SwTwips nPageLeft = getXPath(pXmlDoc, "//page[1]/infos/bounds", "left").toInt32();
    const SwTwips nPicture1Left
        = getXPath(pXmlDoc, "//page[1]//fly/infos/bounds", "left").toInt32();
    CPPUNIT_ASSERT_EQUAL(nPageLeft, nPicture1Left); // page 1: INSIDE == left

    const SwTwips nPageRight = getXPath(pXmlDoc, "//page[1]/infos/bounds", "right").toInt32();
    const SwTwips nArrow2Right
        = getXPath(pXmlDoc, "//page[1]//SwAnchoredDrawObject/bounds", "right").toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nPageRight, nArrow2Right, 50); // page 1: OUTSIDE == right

    const SwTwips nPageMarginRight
        = getXPath(pXmlDoc, "//page[2]//body/infos/bounds", "right").toInt32();
    const SwTwips nPicture4Right
        = getXPath(pXmlDoc, "//page[2]//fly/infos/bounds", "right").toInt32();
    CPPUNIT_ASSERT_EQUAL(nPageMarginRight, nPicture4Right); // page 2: INSIDE == right

    const SwTwips nPageMarginLeft
        = getXPath(pXmlDoc, "//page[2]//body/infos/bounds", "left").toInt32();
    const SwTwips nArrow3Left
        = getXPath(pXmlDoc, "//page[2]//SwAnchoredDrawObject/bounds", "left").toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nPageMarginLeft, nArrow3Left, 50); // page 2: OUTSIDE == left
}

CPPUNIT_TEST_FIXTURE(Test, testTdf166975)
{
    createSwDoc("WordOK.docx");

    CPPUNIT_ASSERT_EQUAL(u"a)"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));
    // this was a%6%)
    CPPUNIT_ASSERT_EQUAL(u"aa)"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));
    // this was a%7%%7%)
    CPPUNIT_ASSERT_EQUAL(u"aaa)"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"bbb)"_ustr,
                         getProperty<OUString>(getParagraph(5), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"ccc)"_ustr,
                         getProperty<OUString>(getParagraph(6), u"ListLabelString"_ustr));

    saveAndReload(TestFilter::DOCX);

    CPPUNIT_ASSERT_EQUAL(u"a)"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));
    // this was aa%)
    CPPUNIT_ASSERT_EQUAL(u"aa)"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));
    // this was aa%a%)
    CPPUNIT_ASSERT_EQUAL(u"aaa)"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"bbb)"_ustr,
                         getProperty<OUString>(getParagraph(5), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"ccc)"_ustr,
                         getProperty<OUString>(getParagraph(6), u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165492_exactWithBottomSpacing)
{
    // Given a document with "exact row height" of 2cm
    // and table bottom border padding of 1.5cm...

    createSwDoc("tdf165492_exactWithBottomSpacing.docx");
    save(TestFilter::DOCX);

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // the actual row heights should be exactly 3.5cm each (rounded up = 1985 twips)
    // giving a total table height of 5955 instead of 3402.
    SwTwips nTableHeight
        = getXPath(pXmlDoc, "//column[1]/body/tab/infos/prtBounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(5955), nTableHeight);

    // the table in the right column has no bottom border padding, so its height is 3402.
    nTableHeight = getXPath(pXmlDoc, "//column[2]/body/tab/infos/prtBounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(3402), nTableHeight);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165492_atLeastWithBottomSpacing)
{
    // Given a document with "minimum row height" of 2cm
    // and table bottom border padding of 1.5cm...

    createSwDoc("tdf165492_atLeastWithBottomSpacing.docx");
    save(TestFilter::DOCX);

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // the actual row heights should be at least 3.5cm each (rounded up = 1985 twips)
    // giving a total table height of 5955 instead of 3402.
    SwTwips nTableHeight
        = getXPath(pXmlDoc, "//column[1]/body/tab/infos/prtBounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(5955), nTableHeight);

    // the table in the right column has no bottom border padding, so its height is 3402.
    nTableHeight = getXPath(pXmlDoc, "//column[2]/body/tab/infos/prtBounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(3402), nTableHeight);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165047_consolidatedTopMargin)
{
    // Given a two page document with a section page break
    // which is preceded by a paragraph with a lot of lower spacing
    // and followed by a paragraph with even more upper spacing...
    createSwDoc("tdf165047_consolidatedTopMargin.docx");
    save(TestFilter::DOCX);

    // the upper spacing is mostly "absorbed" by the preceding lower spacing, and is barely noticed
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // the effective top margin should be 60pt - 50pt = 10pt (0.36cm) after the page break
    SwTwips nParaTopMargin
        = getXPath(pXmlDoc, "/root/page[2]/body/section/infos/prtBounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(200), nParaTopMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165047_contextualSpacingTopMargin)
{
    // Given a two page document with a section page break
    // which is preceded by a paragraph with a lot of lower spacing
    // and followed by a paragraph with a lot of upper spacing,
    // but that paragraph says "don't add space between identical paragraph styles...
    createSwDoc("tdf165047_contextualSpacingTopMargin.docx");
    save(TestFilter::DOCX);

    // the upper spacing is ignored since the paragraph styles are the same
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // The effective top margin (after the page break) must be 0
    SwTwips nParaTopMargin
        = getXPath(pXmlDoc, "/root/page[2]/body/section/infos/prtBounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(0), nParaTopMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf166553_paraStyleAfterBreak)
{
    // Given a distinctively styled paragraph containing a (column) break in the middle of the para
    createSwDoc("tdf166553_paraStyleAfterBreak.docx");
    saveAndReload(TestFilter::DOCX);

    // Note: we emulate this by creating two real paragraphs.
    // The paragraph style/formatting after the break must be (almost) the same as before the break
    uno::Reference<text::XTextRange> xPara
        = getParagraph(2, "A paragraph with a large 75pt top margin, split");
    CPPUNIT_ASSERT_EQUAL(u"Subtitle"_ustr, getProperty<OUString>(xPara, u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2646), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));

    xPara.set(getParagraph(3, " by a column break")); // after the break
    CPPUNIT_ASSERT_EQUAL(u"Subtitle"_ustr, getProperty<OUString>(xPara, u"ParaStyleName"_ustr));
    // The top margin ONLY applies at the beginning of the paragraph (i.e. before the break)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    xPara.set(getRun(getParagraph(3), 1));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, u"CharWeight"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf167721_chUnits, "tdf167721_chUnits.docx")
{
    // given a document that specifies some margins using Ch-based Left/Right indentation
    // where w:rightChars is inherited from the parent styles - so it overrides w:right
    // where w:firstLineChars is specified as a direct property
    // and w:leftChars is disabled (0), so it inherits the style's w:left

    // direct formatting (of the paragraph) in document.xml
    //     <w:ind w:right="1134"(2 cm)  w:hangingChars="100" (1 ic) w:leftChars="0"/>
    // inherited formatting from the style chain in styles.xml
    //     <w:ind w:rightChars="200" (2 ic) w:hangingChars=400 (4 ic)
    //            w:leftChars="300" (3 ic) w:left="2834"/> (5 cm)

    // Test the style #############################################################################
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Inherited List Paragraph"_ustr),
        uno::UNO_QUERY);

    auto aFirstCh
        = getProperty<css::beans::Pair<double, sal_Int16>>(xStyle, u"ParaFirstLineIndentUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(-4), aFirstCh.First);

    auto aLeftCh
        = getProperty<css::beans::Pair<double, sal_Int16>>(xStyle, u"ParaLeftMarginUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(7), aLeftCh.First);

    auto aRightCh
        = getProperty<css::beans::Pair<double, sal_Int16>>(xStyle, u"ParaRightMarginUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(2), aRightCh.First);

    // Test the paragraph #########################################################################
    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    aFirstCh
        = getProperty<css::beans::Pair<double, sal_Int16>>(xPara, u"ParaFirstLineIndentUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(-1), aFirstCh.First);

    // oddly, w:left is always ignored by MS Word in special case with hangingChars indent
    aLeftCh = getProperty<css::beans::Pair<double, sal_Int16>>(xPara, u"ParaLeftMarginUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(1), aLeftCh.First);

    aRightCh = getProperty<css::beans::Pair<double, sal_Int16>>(xPara, u"ParaRightMarginUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(2), aRightCh.First);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf167721_chUnits2)
{
    // given a nasty edge-case document
    // Style "List Paragraph": left = (2 inch minus 1 Ch), right = 2 Ch, hanging indent = +1 Ch
    //     <w:ind w:left="2880" w:rightChars="200" w:hangingChars="100" w:firstLineChars="200"/>
    // Style "Inherited List Paragraph": left = -1 inch, right = 2 Ch, hanging indent = -2 Ch
    //     <w:ind w:leftChars="0" w:right="1134" w:firstLineChars="200" w:hanging="1440" w:firstLine="2880"/>
    // Paragraph: left = -1 inch, right = 2 Ch, hanging indent = +1 inch
    //     <w:ind w:firstLineChars="0"/>

    createSwDoc("tdf167721_chUnits2.docx");
    saveAndReload(TestFilter::DOCX);

    // Test the parent style ######################################################################
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"List Paragraph"_ustr), uno::UNO_QUERY);

    // auto aFirstCh
    //     = getProperty<css::beans::Pair<double, sal_Int16>>(xStyle, u"ParaFirstLineIndentUnit"_ustr);
    // CPPUNIT_ASSERT_EQUAL(double(-1), aFirstCh.First);

    // 5 cm - 1 Ch  (1Ch == 100 TWIP == 0.176 cm), so 4825???  It was 5080
    // CPPUNIT_ASSERT_EQUAL(sal_Int32(5001-176), getProperty<sal_Int32>(xStyle, u"ParaLeftMargin"_ustr));

    auto aRightCh
        = getProperty<css::beans::Pair<double, sal_Int16>>(xStyle, u"ParaRightMarginUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(2), aRightCh.First);

    // Test the style #############################################################################
    xStyle.set(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Inherited List Paragraph"_ustr),
               uno::UNO_QUERY);

    auto aFirstCh
        = getProperty<css::beans::Pair<double, sal_Int16>>(xStyle, u"ParaFirstLineIndentUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(2), aFirstCh.First);

    // IMPROVEMENT: while this probably ought to be -2540 (for some w:hanging adjustment reason)
    // from a purely ParaLeftMargin standpoint, it ought to be zero
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xStyle, u"ParaLeftMargin"_ustr));

    aRightCh
        = getProperty<css::beans::Pair<double, sal_Int16>>(xStyle, u"ParaRightMarginUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(2), aRightCh.First);

    // Test the paragraph #########################################################################
    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    // CPPUNIT_ASSERT_EQUAL(sal_Int32(2540), getProperty<sal_Int32>(xPara, u"ParaFirstLineIndent"_ustr));

    // IMPROVEMENT: while this probably ought to be -2540, zero is OK
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaLeftMargin"_ustr));

    aRightCh = getProperty<css::beans::Pair<double, sal_Int16>>(xPara, u"ParaRightMarginUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(2), aRightCh.First);
}

DECLARE_OOXMLEXPORT_TEST(testTdf167721_chUnits3, "tdf167721_chUnits3.docx")
{
    // given a nasty edge-case document
    // Style "List Paragraph": left = 2 inch, right = 2 cm, first line = none
    //     <w:ind w:left="2880" w:right="1134" w:hangingChars="0" w:firstLine="2880"/>
    // Style "Inherited List Paragraph": left = -1 inch, right = 2 Ch, hanging indent = +1 inch
    //     <w:ind w:leftChars="0" left="2880" w:rightChars="200" w:hangingChars="0" w:hanging="1440"/>
    // Paragraph: left = 0, right = 0.14 inch, first line = +2 inch
    //     <w:ind w:rightChars="0" w:hangingChars="0" w:firstLine="2880" />

    // Test the parent style ######################################################################
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"List Paragraph"_ustr), uno::UNO_QUERY);

    // auto aFirstCh
    //     = getProperty<css::beans::Pair<double, sal_Int16>>(xStyle, u"ParaFirstLineIndentUnit"_ustr);
    // CPPUNIT_ASSERT_EQUAL(double(0), aFirstCh.First);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5080), getProperty<sal_Int32>(xStyle, u"ParaLeftMargin"_ustr));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(xStyle, u"ParaRightMargin"_ustr));

    // Test the style #############################################################################
    xStyle.set(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Inherited List Paragraph"_ustr),
               uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2540),
                         getProperty<sal_Int32>(xStyle, u"ParaFirstLineIndent"_ustr));

    // IMPROVEMENT: probably should be -2540 (adjusted by hanging indent), but zero is OK.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xStyle, u"ParaLeftMargin"_ustr));

    auto aRightCh
        = getProperty<css::beans::Pair<double, sal_Int16>>(xStyle, u"ParaRightMarginUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(2), aRightCh.First);

    // Test the paragraph #########################################################################
    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5080),
                         getProperty<sal_Int32>(xPara, u"ParaFirstLineIndent"_ustr));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaLeftMargin"_ustr));

    // 200 twip = 0.353 cm - surprisingly, the inherited 200 Ch is turned into 200 twip
    CPPUNIT_ASSERT_EQUAL(sal_Int32(353), getProperty<sal_Int32>(xPara, u"ParaRightMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf167721_chUnits4, "tdf167721_chUnits4.docx")
{
    // given an edge-case document
    // Style "List Paragraph": left = 5cm, right = 2 ic
    //     <w:ind w:left="2835" w:rightChars="200"/>
    // Style "Inherited List Paragraph": left = 5cmm right = 2 Ch
    //    <w:ind w:right="2268" />
    // Paragraph: left = 5cm, right = 4cm
    //     <w:ind w:rightChars="0" />

    // Test the style #############################################################################
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Inherited List Paragraph"_ustr),
        uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001), getProperty<sal_Int32>(xStyle, u"ParaLeftMargin"_ustr));

    auto aRightCh
        = getProperty<css::beans::Pair<double, sal_Int16>>(xStyle, u"ParaRightMarginUnit"_ustr);
    CPPUNIT_ASSERT_EQUAL(double(2), aRightCh.First);

    // Test the paragraph #########################################################################
    uno::Reference<text::XTextRange> xPara(getParagraph(1));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaFirstLineIndent"_ustr));

    // the left margin is inherited from the style, though it is never written as a direct property
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001), getProperty<sal_Int32>(xPara, u"ParaLeftMargin"_ustr));

    // the unused w:right in the style is what gets inherited (and written as a direct property)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4001), getProperty<sal_Int32>(xPara, u"ParaRightMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf167721_zeroCh)
{
    createSwDoc("tdf167721_zeroCh.odt");
    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);

    // only one style should disable the ic-unit right margin with a zero.
    assertXPath(pXmlStyles, "//w:ind[@w:endChars='0']", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf83844)
{
    createSwDoc("tdf83844.fodt");

    auto fnVerify = [this] {
        auto pXmlDoc = parseLayoutDump();

        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                    u"A A A A ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                    u"B B B B B B B B ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]", "portion",
                    u"C C C C C C C C ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[4]", "portion",
                    u"D D D D");
    };

    fnVerify();
    saveAndReload(TestFilter::DOCX);
    fnVerify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf83844Hanging)
{
    createSwDoc("tdf83844-hanging.fodt");

    auto fnVerify = [this] {
        auto pXmlDoc = parseLayoutDump();

        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                    u"A A A A A A A A A ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                    u"B B B B B B ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]", "portion",
                    u"C C C C C C C ");
        assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[4]", "portion",
                    u"D D");
    };

    fnVerify();
    saveAndReload(TestFilter::DOCX);
    fnVerify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf88908)
{
    createSwDoc();

    {
        SwDoc* pDoc = getSwDoc();
        IDocumentSettingAccess& rIDSA = pDoc->getIDocumentSettingAccess();
        CPPUNIT_ASSERT(!rIDSA.get(DocumentSettingId::BALANCE_SPACES_AND_IDEOGRAPHIC_SPACES));
    }

    saveAndReload(TestFilter::DOCX);

    {
        SwDoc* pDoc = getSwDoc();
        IDocumentSettingAccess& rIDSA = pDoc->getIDocumentSettingAccess();
        CPPUNIT_ASSERT(!rIDSA.get(DocumentSettingId::BALANCE_SPACES_AND_IDEOGRAPHIC_SPACES));

        rIDSA.set(DocumentSettingId::BALANCE_SPACES_AND_IDEOGRAPHIC_SPACES, true);
    }

    saveAndReload(TestFilter::DOCX);

    {
        SwDoc* pDoc = getSwDoc();
        IDocumentSettingAccess& rIDSA = pDoc->getIDocumentSettingAccess();
        CPPUNIT_ASSERT(rIDSA.get(DocumentSettingId::BALANCE_SPACES_AND_IDEOGRAPHIC_SPACES));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165933_noDelTextOnMove)
{
    createSwDoc("tdf165933.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the fix it fails with
    // - Expected: 0
    // - Actual  : 1
    // a w:delText is created inside a w:moveFrom, which is invalid
    assertXPath(pXmlDoc, "//w:moveFrom/w:r/w:delText", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testDelThenFormatDocxExport)
{
    // Given a document with <del>A<format>B</format>C</del> style redlines:
    // When exporting that document:
    createSwDoc("del-then-format.docx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save(TestFilter::DOCX);

    // Then make sure delete "under" format uses the <w:delText> markup:
    // Without the accompanying fix in place, this test would have failed with:
    // - In <>, XPath '/w:document/w:body/w:p/w:del[2]/w:r/w:delText' not found
    // i.e. <w:t> was used, not <w:delText>.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:del[2]/w:r/w:delText", u"BBB");
}

DECLARE_OOXMLEXPORT_TEST(testTdf139418, "tdf139418.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);

    sal_Int32 nBaseWidth;
    xPropertySet->getPropertyValue(u"GridBaseWidth"_ustr) >>= nBaseWidth;
    // Without the fix, this will fail with
    // - Expected: ~795
    // - Actual  : 848
    CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(794), nBaseWidth);
    CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(796), nBaseWidth);

    auto pXmlDoc = parseLayoutDump();

    // Vertical DOCX should insert kern portions to align text to the grid
    sal_Int32 nPorLen1 = getXPath(pXmlDoc, "(//SwLinePortion)[1]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nPorLen1);

    sal_Int32 nPorLen2 = getXPath(pXmlDoc, "(//SwLinePortion)[2]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nPorLen2);

    sal_Int32 nPorLen3 = getXPath(pXmlDoc, "(//SwLinePortion)[3]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(41), nPorLen3);

    sal_Int32 nPorLen4 = getXPath(pXmlDoc, "(//SwLinePortion)[4]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nPorLen4);
}

CPPUNIT_TEST_FIXTURE(Test, testAnnotationRef)
{
    createSwDoc("comment-annotationref.docx");
    save(TestFilter::DOCX);

    // Check that the exported comment also has annotationRef
    xmlDocUniquePtr pXmlComments = parseExport(u"word/comments.xml"_ustr);
    CPPUNIT_ASSERT(pXmlComments);
    // Without the fix it fails with
    // - Expected: 1
    // - Actual  : 0
    assertXPath(pXmlComments, "//w:comments/w:comment[1]/w:p[1]/w:r[1]/w:annotationRef");
}

CPPUNIT_TEST_FIXTURE(Test, testBadFormulaResult)
{
    // Given a loaded by not laid out document:
    uno::Sequence<beans::PropertyValue> aFilterOptions = {
        comphelper::makePropertyValue("Hidden", true),
    };
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(u"/sw/qa/extras/ooxmlexport/data/")
                                      + "formula-result.fodt",
                                  "com.sun.star.text.TextDocument", aFilterOptions);

    // When saving to DOCX:
    save(TestFilter::DOCX);

    // Then make sure that the field result in the last row's last run is not empty:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6
    // - Actual  : 0
    // i.e. the SUM() field evaluated to an empty result.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[4]/w:t", u"6");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf166511)
{
    createSwDoc("tdf166511.docx");
    save(TestFilter::DOCX);

    // The reproducer for tdf#166511 depends on result having footer2 and footer3.xml
    // Original bugdoc only has footer1.xml, and the result only started having multiples
    //  after tdf#136472's fix
    // This test uses a "good" roundtripped version of the bugdoc after the fix already
    //  having the extra footer XMLs, so it doesn't depend on the change coming from tdf#136472

    // Count total docPr ids and collect unique docPr ids to be counted later from various parts
    //  of the doc, in the end both counts must be the same, as ids have to be unique
    std::set<sal_Int32> aDiffIds;
    size_t nIds = 0;
    const OUString aFooters[] = { u"word/footer2.xml"_ustr, u"word/footer3.xml"_ustr };
    for (const OUString& footer : aFooters)
    {
        xmlDocUniquePtr pXmlDoc = parseExport(footer);
        CPPUNIT_ASSERT(pXmlDoc);

        const int aDrawingNodes1 = countXPathNodes(pXmlDoc, "/w:ftr/w:p/w:r/mc:AlternateContent");
        for (int i = 1; i <= aDrawingNodes1; i++)
        {
            OUString aId = getXPath(pXmlDoc,
                                    "/w:ftr/w:p/w:r/mc:AlternateContent[" + OString::number(i)
                                        + "]/mc:Choice/w:drawing/wp:anchor/wp:docPr",
                                    "id");
            aDiffIds.insert(aId.toInt32());
            nIds++;
            const OString aFallbackXPath
                = "/w:ftr/w:p/w:r/mc:AlternateContent[" + OString::number(i)
                  + "]/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:r/"
                    "w:drawing/wp:inline/wp:docPr";
            if (countXPathNodes(pXmlDoc, aFallbackXPath) > 0)
            {
                aId = getXPath(pXmlDoc, aFallbackXPath, "id");
                aDiffIds.insert(aId.toInt32());
                nIds++;
            }
        }
        const int aDrawingNodes2 = countXPathNodes(pXmlDoc, "/w:ftr/w:p/w:r/w:drawing");
        for (int i = 1; i <= aDrawingNodes2; i++)
        {
            OUString aId = getXPath(
                pXmlDoc, "/w:ftr/w:p/w:r/w:drawing[" + OString::number(i) + "]/wp:anchor/wp:docPr",
                "id");
            aDiffIds.insert(aId.toInt32());
            nIds++;
        }
    }
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected : 7
    // - Actual : 8
    CPPUNIT_ASSERT_EQUAL(aDiffIds.size(), nIds);
}

CPPUNIT_TEST_FIXTURE(Test, testLineBreakInRef)
{
    createSwDoc("line-break-in-ref.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Without the fix if fails with
    // assertion failed
    // - Expression: xmlXPathNodeSetGetLength(pXmlNodes) > 0
    // - In <>, XPath '/w:document/w:body/w:p[1]/w:r[4]/w:t' not found
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[4]/w:t", u"Text1");
}

DECLARE_OOXMLEXPORT_TEST(testFieldMarkFormat, "fontsize-field-separator.docx")
{
    uno::Reference<text::XTextRange> xRun(getRun(getParagraph(1), 1));

    // Without the fix it fails with
    // - Expected: 12
    // - Actual  : 42
    // i.e. the field content has the properties of the field marks
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(xRun, u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, tdf167527_title_letters_cut_from_below)
{
    createSwDoc("tdf167527_title_letters_cut_from_below.docx");

    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);

    // bOutputForScreen of true to ensure field backgrounds are rendered
    std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile(false, true);

    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);

    auto nFieldShadingsBottom = getXPath(pXmlDoc, "(//rect)[2]", "bottom").toInt32();
    auto nFieldShadingsTop = getXPath(pXmlDoc, "(//rect)[2]", "top").toInt32();
    sal_Int32 nFieldShadingsHeight = nFieldShadingsBottom - nFieldShadingsTop;

    // Without the accompanying fix in place, this test would have failed with:
    // less equal assertion failed
    // Expected less or equal than: 700
    // Actual  : 810
    // i.e. the field background overlaps the previous row of text
    CPPUNIT_ASSERT_LESSEQUAL(static_cast<sal_Int32>(700), nFieldShadingsHeight);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
