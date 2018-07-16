/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <string>
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

    /**
     * Validation handling
     */
    bool mustValidate(const char* filename) const override
    {
        const char* aWhitelist[] = { "paragraph-mark-nonempty.odt", "page-graphic-background.odt",
                                     "zoom.docx", "empty.odt" };
        std::vector<const char*> vWhitelist(aWhitelist, aWhitelist + SAL_N_ELEMENTS(aWhitelist));

        return std::find(vWhitelist.begin(), vWhitelist.end(), filename) != vWhitelist.end();
    }

protected:
};

DECLARE_OOXMLEXPORT_TEST(testTdf53856_conflictingStyle, "tdf53856_conflictingStyle.docx")
{
    // The "Text" style conflicted with builtin paragraph style Caption -> Text
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Text"),
                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"),
                         getProperty<OUString>(xStyle, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xStyle, "CharPosture"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104713_undefinedStyles, "tdf104713_undefinedStyles.docx")
{
    // Normal paragraph style was not defined, so don't replace conflicting styles
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Heading 1"),
                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(212),
                         getProperty<sal_Int32>(xStyle, "ParaBottomMargin"));

    // tdf108765: once importing is finished, use default values for any styles not yet defined.
    xStyle.set(getStyles("ParagraphStyles")->getByName("Footnote"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font size", 10.f, getProperty<float>(xStyle, "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testEm, "em.docx")
{
    // Test all possible <w:em> arguments.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::NONE,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharEmphasis"));
    // This was ACCENT_ABOVE.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::DOT_ABOVE,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 2), "CharEmphasis"));
    // This was DOT_ABOVE.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::ACCENT_ABOVE,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 3), "CharEmphasis"));
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::CIRCLE_ABOVE,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 4), "CharEmphasis"));
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::DOT_BELOW,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 5), "CharEmphasis"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo77716, "fdo77716.docx")
{
    // The problem was that there should be 200 twips spacing between the two paragraphs, but there wasn't any.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Standard"),
                                               uno::UNO_QUERY);
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(200)),
                         getProperty<sal_Int32>(xStyle, "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testAfterlines, "afterlines.docx")
{
    // This was 353, i.e. the value of <w:spacing w:after="200"> from <w:pPrDefault>, instead of <w:spacing w:afterLines="100"/> from <w:pPr>.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(423),
                         getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testParagraphMark, "paragraph-mark.docx")
{
    // The problem was that we didn't handle the situation when an empty paragraph's marker had both a char style and some direct formatting.

    // This was 11.
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(getParagraph(1), "CharHeight"));
    // This was empty.
    CPPUNIT_ASSERT_EQUAL(OUString("Emphasis"),
                         getProperty<OUString>(getRun(getParagraph(1), 1), "CharStyleName"));
}

DECLARE_OOXMLEXPORT_TEST(testParagraphMarkNonempty, "paragraph-mark-nonempty.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        // There were two <w:sz> elements, make sure the 40 one is dropped and the 20 one is kept.
        assertXPath(pXmlDoc, "//w:p/w:pPr/w:rPr/w:sz", "val", "20");
}

DECLARE_OOXMLEXPORT_TEST(testTdf88583, "tdf88583.odt")
{
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID,
                         getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0x00cc00),
                         getProperty<sal_Int32>(getParagraph(1), "FillColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf91261, "tdf91261.docx")
{
    bool snapToGrid = true;
    uno::Reference<text::XTextRange> xPara = getParagraph(2);
    uno::Reference<beans::XPropertySet> properties(xPara, uno::UNO_QUERY);
    properties->getPropertyValue("SnapToGrid") >>= snapToGrid;
    CPPUNIT_ASSERT_EQUAL(false, snapToGrid);

    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"),
                                               uno::UNO_QUERY);
    sal_Int16 nGridMode;
    xStyle->getPropertyValue("GridMode") >>= nGridMode;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nGridMode);

    bool bGridSnapToChars;
    xStyle->getPropertyValue("GridSnapToChars") >>= bGridSnapToChars;
    CPPUNIT_ASSERT_EQUAL(true, bGridSnapToChars);
}

DECLARE_OOXMLEXPORT_TEST(testTdf91594, "tdf91594.docx")
{
    uno::Reference<text::XTextRange> xPara1(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(u'\xf0fb', xPara1->getString()[0]);
    uno::Reference<text::XTextRange> xPara2(getParagraph(2));
    CPPUNIT_ASSERT_EQUAL(u'\xf0fc', xPara2->getString()[0]);
    uno::Reference<text::XTextRange> xPara3(getParagraph(3));
    CPPUNIT_ASSERT_EQUAL(u'\xf0fd', xPara3->getString()[0]);
    uno::Reference<text::XTextRange> xPara4(getParagraph(4));
    CPPUNIT_ASSERT_EQUAL(u'\xf0fe', xPara4->getString()[0]);

    uno::Reference<beans::XPropertySet> xRun(getRun(xPara1, 1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontNameAsian"));
    CPPUNIT_ASSERT_EQUAL(OUString("Wingdings"), getProperty<OUString>(xRun, "CharFontNameComplex"));
}

DECLARE_OOXMLEXPORT_TEST(testTDF99434, "protectedform.docx")
{
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xTextFactory(mxComponent,
                                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        m_xTextFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
    uno::Any aProtect = xSettings->getPropertyValue("ProtectForm");
    bool bProt = false;
    aProtect >>= bProt;
    CPPUNIT_ASSERT(bProt);
}

DECLARE_OOXMLEXPORT_TEST(testPageGraphicBackground, "page-graphic-background.odt")
{
    // No idea how the graphic background should be exported (seems there is no
    // way to do a non-tiling export to OOXML), but at least the background
    // color shouldn't be black.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testZoom, "zoom.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xViewSettingsSupplier->getViewSettings());
    sal_Int16 nValue = 0;
    xPropertySet->getPropertyValue("ZoomValue") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), nValue);

    // Validation test: order of elements were wrong.
    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");
    if (!pXmlDoc)
        return;
    // Order was: rsid, next.
    int nNext = getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "next");
    int nRsid = getXPathPosition(pXmlDoc, "/w:styles/w:style[3]", "rsid");
    CPPUNIT_ASSERT(nNext < nRsid);

    pXmlDoc = parseExport("docProps/app.xml");
    // One paragraph in the document.
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Paragraphs",
                       "1");
    assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:Company",
                       "Example Ltd");
}

DECLARE_OOXMLEXPORT_TEST(defaultTabStopNotInStyles, "empty.odt")
{
    // The default tab stop was mistakenly exported to a style.
    // xray ThisComponent.StyleFamilies(1)(0).ParaTabStop
    uno::Reference<container::XNameAccess> paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference<beans::XPropertySet> properties(paragraphStyles->getByName("Standard"),
                                                   uno::UNO_QUERY);
    uno::Sequence<style::TabStop> stops = getProperty<uno::Sequence<style::TabStop>>(
        paragraphStyles->getByName("Standard"), "ParaTabStops");
    // There actually be one tab stop, but it will be the default.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), stops.getLength());
    CPPUNIT_ASSERT_EQUAL(style::TabAlign_DEFAULT, stops[0].Alignment);
}

DECLARE_OOXMLEXPORT_TEST(testFdo64826, "fdo64826.docx")
{
    // 'Track-Changes' (Track Revisions) wasn't exported.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(mxComponent, "RecordChanges"));
}

DECLARE_OOXMLEXPORT_TEST(testPageBackground, "page-background.docx")
{
    // 'Document Background' wasn't exported.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x92D050), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65265, "fdo65265.docx")
{
    // Redline (tracked changes) of text formatting were not exported
    uno::Reference<text::XTextRange> xParagraph1 = getParagraph(1);
    uno::Reference<text::XTextRange> xParagraph2 = getParagraph(2);

    CPPUNIT_ASSERT_EQUAL(OUString("Format"),
                         getProperty<OUString>(getRun(xParagraph1, 3), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Format"),
                         getProperty<OUString>(getRun(xParagraph2, 2), "RedlineType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo43093, "fdo43093.docx")
{
    // The problem was that the alignment are not exchange when the paragraph are RTL.
    uno::Reference<uno::XInterface> xParaRtlLeft(getParagraph(1, "RTL Left"));
    sal_Int32 nRtlLeft = getProperty<sal_Int32>(xParaRtlLeft, "ParaAdjust");
    // test the text Direction value for the pragraph
    sal_Int16 nRLDir = getProperty<sal_Int32>(xParaRtlLeft, "WritingMode");

    uno::Reference<uno::XInterface> xParaRtlRight(getParagraph(3, "RTL Right"));
    sal_Int32 nRtlRight = getProperty<sal_Int32>(xParaRtlRight, "ParaAdjust");
    sal_Int16 nRRDir = getProperty<sal_Int32>(xParaRtlRight, "WritingMode");

    uno::Reference<uno::XInterface> xParaLtrLeft(getParagraph(5, "LTR Left"));
    sal_Int32 nLtrLeft = getProperty<sal_Int32>(xParaLtrLeft, "ParaAdjust");
    sal_Int16 nLLDir = getProperty<sal_Int32>(xParaLtrLeft, "WritingMode");

    uno::Reference<uno::XInterface> xParaLtrRight(getParagraph(7, "LTR Right"));
    sal_Int32 nLtrRight = getProperty<sal_Int32>(xParaLtrRight, "ParaAdjust");
    sal_Int16 nLRDir = getProperty<sal_Int32>(xParaLtrRight, "WritingMode");

    // this will test the both the text direction and alignment for each paragraph
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_LEFT), nRtlLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nRLDir);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_RIGHT), nRtlRight);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nRRDir);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_LEFT), nLtrLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLLDir);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_RIGHT), nLtrRight);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLRDir);
}

DECLARE_OOXMLEXPORT_TEST(testFdo64238_a, "fdo64238_a.docx")
{
    // The problem was that when 'Show Only Odd Footer' was marked in Word and the Even footer *was filled*
    // then LO would still import the Even footer and concatenate it to the odd footer.
    // This case specifically is for :
    // 'Blank Odd Footer' with 'Non-Blank Even Footer' when 'Show Only Odd Footer' is marked in Word
    // In this case the imported footer in LO was supposed to be blank, but instead was the 'even' footer
    uno::Reference<text::XText> xFooterText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "FooterText");
    uno::Reference<text::XTextRange> xFooterParagraph = getParagraphOfText(1, xFooterText);
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xFooterParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    sal_Int32 numOfRuns = 0;
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
        numOfRuns++;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), numOfRuns);
}

DECLARE_OOXMLEXPORT_TEST(testFdo64238_b, "fdo64238_b.docx")
{
    // The problem was that when 'Show Only Odd Footer' was marked in Word and the Even footer *was filled*
    // then LO would still import the Even footer and concatenate it to the odd footer.
    // This case specifically is for :
    // 'Non-Blank Odd Footer' with 'Non-Blank Even Footer' when 'Show Only Odd Footer' is marked in Word
    // In this case the imported footer in LO was supposed to be just the odd footer, but instead was the 'odd' and 'even' footers concatenated
    uno::Reference<text::XText> xFooterText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "FooterText");
    uno::Reference<text::XTextRange> xFooterParagraph = getParagraphOfText(1, xFooterText);
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xFooterParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    sal_Int32 numOfRuns = 0;
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
        numOfRuns++;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), numOfRuns);
}

DECLARE_OOXMLEXPORT_TEST(testFdo56679, "fdo56679.docx")
{
    // The problem was that the DOCX importer and exporter did not handle the 'color' of an underline
    // (not the color of the text, the color of the underline itself)
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 2, "This is a simple sentence.");

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xText, "CharUnderlineHasColor"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFF0000), getProperty<sal_Int32>(xText, "CharUnderlineColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65400, "fdo65400.docx")
{
    // The problem was that if in Word you choose 'Character Shading' - then the text portion
    // is marked with 'w:shd val=pct15'. LO did not store this value and so when importing and exporting
    // this value was lost (and so Word did not show 'Character Shading' was on)
    uno::Reference<text::XTextRange> paragraph1 = getParagraph(1);
    uno::Reference<text::XTextRange> shaded = getRun(paragraph1, 2, "normal");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0026), getProperty<sal_Int32>(shaded, "CharShadingValue"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xd8d8d8), getProperty<sal_Int32>(shaded, "CharBackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66543, "fdo66543.docx")
{
    // The problem was that when importing DOCX with 'line numbers' - the 'start value' was imported
    // but nothing was done with it.

    uno::Reference<text::XTextRange> paragraph1 = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(paragraph1, "ParaLineNumberStartValue"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo66773, "fdo66773.docx")
{
    // The problem was the line spacing was interpreted by Word as 'Multiple 1.08' if no default settings were written.
    // Now after the 'docDefaults' section is written in <styles.xml> - there is no more problem.
    // (Word does not try to calculate some arbitrary value for line spacing).
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());

    style::LineSpacing alineSpacing
        = getProperty<style::LineSpacing>(xParaEnum->nextElement(), "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::PROP, alineSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(100), static_cast<sal_Int32>(alineSpacing.Height));
}

DECLARE_OOXMLEXPORT_TEST(testGrabBag, "grabbag.docx")
{
    // w:mirrorIndents was lost on roundtrip, now should be handled as a grab bag property
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:mirrorIndents");
}

DECLARE_OOXMLEXPORT_TEST(testFdo60990, "fdo60990.odt")
{
    // The shape had no background, no paragraph adjust and no font color.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00CFE7F5), getProperty<sal_Int32>(xShape, "FillColor"));
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText);
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_CENTER,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xParagraph, "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00FF00),
                         getProperty<sal_Int32>(getRun(xParagraph, 1), "CharColor"));
}

DECLARE_OOXMLEXPORT_TEST(testBnc837302, "bnc837302.docx")
{
    // The problem was that text with empty author was not inserted as a redline
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);

    // previously 'AAA' was not an own run
    getRun(xParagraph, 3, "AAA");
    // interestingly the 'Insert' is set on the _previous_ run
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),
                         getProperty<OUString>(getRun(xParagraph, 2), "RedlineType"));

    // make sure we don't introduce a redlined delete in the 2nd paragraph
    xParagraph = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(xParagraph, 1), "RedlineType"));
}

DECLARE_OOXMLEXPORT_TEST(testCharacterBorder, "charborder.odt")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY);
    // OOXML has just one border attribute (<w:bdr>) for text border so all side has
    // the same border with the same padding
    // Border
    {
        const table::BorderLine2 aTopBorder
            = getProperty<table::BorderLine2>(xRun, "CharTopBorder");
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF6600, 0, 318, 0, 0, 318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder,
                                    getProperty<table::BorderLine2>(xRun, "CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder,
                                    getProperty<table::BorderLine2>(xRun, "CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder,
                                    getProperty<table::BorderLine2>(xRun, "CharRightBorder"));
    }

    // Padding (w:space)
    {
        const sal_Int32 nTopPadding = getProperty<sal_Int32>(xRun, "CharTopBorderDistance");
        // In the original ODT the padding is 150, but the unit conversion round it down.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(141), nTopPadding);
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun, "CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun, "CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun, "CharRightBorderDistance"));
    }

    // Shadow (w:shadow)
    /* OOXML use just one bool value for shadow so the next conversions
    are made during an export-import round
    color: any -> black
    location: any -> bottom-right
    width: any -> border width */
    {
        const table::ShadowFormat aShadow
            = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }

    // Also check shadow when it is in middle of the paragraph
    // (problem can be during export with SwWW8AttrIter::HasTextItem())
    {
        uno::Reference<beans::XPropertySet> xMiddleRun(getRun(getParagraph(2), 2), uno::UNO_QUERY);
        const table::ShadowFormat aShadow
            = getProperty<table::ShadowFormat>(xMiddleRun, "CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }

    if (xmlDocPtr pXmlStyles = parseExport("word/styles.xml"))
    {
        // Make sure we write qFormat for custom style names.
        assertXPath(pXmlStyles, "//w:style[@w:styleId='Heading']/w:qFormat", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testStyleInheritance, "style-inheritance.docx")
{
    // Check that now styleId's are more like what MSO produces
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    // the 1st style always must be Normal
    assertXPath(pXmlStyles, "/w:styles/w:style[1]", "styleId", "Normal");
    // some random style later
    assertXPath(pXmlStyles, "/w:styles/w:style[4]", "styleId", "Heading3");

    // Check that we do _not_ export w:next for styles that point to themselves.
    assertXPath(pXmlStyles, "/w:styles/w:style[1]/w:next", 0);

    // Check that we roundtrip <w:next> correctly - on XML level
    assertXPath(pXmlStyles, "/w:styles/w:style[2]/w:next", "val", "Normal");
    // And to be REALLY sure, check it on the API level too ;-)
    uno::Reference<container::XNameAccess> paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference<beans::XPropertySet> properties(paragraphStyles->getByName("Heading 1"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"), getProperty<OUString>(properties, "FollowStyle"));

    // This was 0, as export of w:outlineLvl was missing.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(properties, "OutlineLevel"));

    properties.set(paragraphStyles->getByName("Heading 11"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"), getProperty<OUString>(properties, "FollowStyle"));

    // Make sure style #2 is Heading 1.
    assertXPath(pXmlStyles, "/w:styles/w:style[2]", "styleId", "Heading1");
    // w:ind was copied from the parent (Normal) style without a good reason.
    assertXPath(pXmlStyles, "/w:styles/w:style[2]/w:pPr/w:ind", 0);

    // We output exactly 2 properties in rPrDefault, nothing else was
    // introduced as an additional default
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/*", 2);
    // Check that we output real content of rPrDefault
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts", "ascii",
                "Times New Roman");
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:lang", "bidi", "ar-SA");
    // pPrDefault is empty
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:pPrDefault/w:pPr/*", 0);

    // Check latent styles
    uno::Sequence<beans::PropertyValue> aGrabBag
        = getProperty<uno::Sequence<beans::PropertyValue>>(mxComponent, "InteropGrabBag");
    uno::Sequence<beans::PropertyValue> aLatentStyles;
    for (sal_Int32 i = 0; i < aGrabBag.getLength(); ++i)
        if (aGrabBag[i].Name == "latentStyles")
            aGrabBag[i].Value >>= aLatentStyles;
    CPPUNIT_ASSERT(aLatentStyles.getLength()); // document should have latent styles

    // Check latent style default attributes
    OUString aCount;
    uno::Sequence<beans::PropertyValue> aLatentStyleExceptions;
    for (sal_Int32 i = 0; i < aLatentStyles.getLength(); ++i)
    {
        if (aLatentStyles[i].Name == "count")
            aCount = aLatentStyles[i].Value.get<OUString>();
        else if (aLatentStyles[i].Name == "lsdExceptions")
            aLatentStyles[i].Value >>= aLatentStyleExceptions;
    }
    CPPUNIT_ASSERT_EQUAL(OUString("371"), aCount); // This check the "count" attribute.

    // Check exceptions to the latent style defaults.
    uno::Sequence<beans::PropertyValue> aLatentStyleException;
    aLatentStyleExceptions[0].Value >>= aLatentStyleException;
    OUString aName;
    for (sal_Int32 i = 0; i < aLatentStyleException.getLength(); ++i)
        if (aLatentStyleException[i].Name == "name")
            aName = aLatentStyleException[i].Value.get<OUString>();
    CPPUNIT_ASSERT_EQUAL(OUString("Normal"),
                         aName); // This checks the "name" attribute of the first exception.

    // This numbering style wasn't roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoList']/w:name", "val", "No List");

    // Table style wasn't roundtripped.
    assertXPath(pXmlStyles,
                "/w:styles/w:style[@w:styleId='TableNormal']/w:tblPr/w:tblCellMar/w:left", "w",
                "108");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:semiHidden", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:unhideWhenUsed", 1);

    // Additional para style properties should be also roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListParagraph']/w:uiPriority", "val",
                "34");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:qFormat", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:rsid", "val", "00780346");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']", "default", "1");

    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:link", "val",
                "Heading1Char");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:locked", 1);

    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading11']", "customStyle", "1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading11']/w:autoRedefine", 1);

    // Additional char style properties should be also roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='DefaultParagraphFont']", "default", "1");

    // Finally check the same for numbering styles.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoList']", "default", "1");

    // This was 1, the default style had <w:suppressAutoHyphens w:val="true"/> even for a default style having no RES_PARATR_HYPHENZONE set.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:pPr/w:suppressAutoHyphens",
                0);
}

DECLARE_OOXMLEXPORT_TEST(testFdo71302, "fdo71302.docx")
{
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;

    // This got renamed to "Strong Emphasis" without a good reason.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Strong']", 1);
}

DECLARE_OOXMLEXPORT_TEST(testSmartart, "smartart.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGrabBag(0);
    xTextDocumentPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty

    bool bTheme = false;
    for (int i = 0; i < aGrabBag.getLength(); ++i)
    {
        if (aGrabBag[i].Name == "OOXTheme")
        {
            bTheme = true;
            uno::Reference<xml::dom::XDocument> aThemeDom;
            CPPUNIT_ASSERT(aGrabBag[i].Value >>= aThemeDom); // PropertyValue of proper type
            CPPUNIT_ASSERT(aThemeDom.get()); // Reference not empty
        }
    }
    CPPUNIT_ASSERT(bTheme); // Grab Bag has all the expected elements

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount()); // One groupshape in the doc

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         xGroup->getCount()); // 1 rendered bitmap from the original shapes

    uno::Reference<beans::XPropertySet> xGroupPropertySet(getShape(1), uno::UNO_QUERY);
    xGroupPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty

    bool bData = false, bLayout = false, bQStyle = false, bColor = false, bDrawing = false;
    for (int i = 0; i < aGrabBag.getLength(); ++i)
    {
        if (aGrabBag[i].Name == "OOXData")
        {
            bData = true;
            uno::Reference<xml::dom::XDocument> aDataDom;
            CPPUNIT_ASSERT(aGrabBag[i].Value >>= aDataDom); // PropertyValue of proper type
            CPPUNIT_ASSERT(aDataDom.get()); // Reference not empty
        }
        else if (aGrabBag[i].Name == "OOXLayout")
        {
            bLayout = true;
            uno::Reference<xml::dom::XDocument> aLayoutDom;
            CPPUNIT_ASSERT(aGrabBag[i].Value >>= aLayoutDom); // PropertyValue of proper type
            CPPUNIT_ASSERT(aLayoutDom.get()); // Reference not empty
        }
        else if (aGrabBag[i].Name == "OOXStyle")
        {
            bQStyle = true;
            uno::Reference<xml::dom::XDocument> aStyleDom;
            CPPUNIT_ASSERT(aGrabBag[i].Value >>= aStyleDom); // PropertyValue of proper type
            CPPUNIT_ASSERT(aStyleDom.get()); // Reference not empty
        }
        else if (aGrabBag[i].Name == "OOXColor")
        {
            bColor = true;
            uno::Reference<xml::dom::XDocument> aColorDom;
            CPPUNIT_ASSERT(aGrabBag[i].Value >>= aColorDom); // PropertyValue of proper type
            CPPUNIT_ASSERT(aColorDom.get()); // Reference not empty
        }
        else if (aGrabBag[i].Name == "OOXDrawing")
        {
            bDrawing = true;
            uno::Sequence<uno::Any> diagramDrawing;
            uno::Reference<xml::dom::XDocument> aDrawingDom;
            CPPUNIT_ASSERT(aGrabBag[i].Value >>= diagramDrawing);
            CPPUNIT_ASSERT(diagramDrawing[0] >>= aDrawingDom); // PropertyValue of proper type
            CPPUNIT_ASSERT(aDrawingDom.get()); // Reference not empty
        }
    }
    CPPUNIT_ASSERT(bData && bLayout && bQStyle && bColor
                   && bDrawing); // Grab Bag has all the expected elements

    uno::Reference<beans::XPropertySet> xPropertySet(xGroup->getByIndex(0), uno::UNO_QUERY);
    OUString nValue;
    xPropertySet->getPropertyValue("Name") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(OUString("RenderedShapes"), nValue); // Rendered bitmap has the proper name
}

DECLARE_OOXMLEXPORT_TEST(testFontNameIsEmpty, "font-name-is-empty.docx")
{
    // Check no empty font name is exported
    // This test does not fail, if the document contains a font with empty name.

    xmlDocPtr pXmlFontTable = parseExport("word/fontTable.xml");
    if (!pXmlFontTable)
        return;
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlFontTable, "/w:fonts/w:font");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    sal_Int32 length = xmlXPathNodeSetGetLength(pXmlNodes);
    for (sal_Int32 index = 0; index < length; index++)
    {
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[index];
        OUString attrVal = OUString::createFromAscii(
            reinterpret_cast<char*>(xmlGetProp(pXmlNode, BAD_CAST("name"))));
        if (attrVal.isEmpty())
        {
            CPPUNIT_FAIL("Font name is empty.");
        }
    }
    xmlXPathFreeObject(pXmlObj);
}

DECLARE_OOXMLEXPORT_TEST(testFdo70812, "fdo70812.docx")
{
    // Import just crashed.
    getParagraph(1, "Sample pages document.");
}

DECLARE_OOXMLEXPORT_TEST(testLineSpacingexport, "test_line_spacing.docx")
{
    // The Problem was that the w:line attribute value in w:spacing tag was incorrect
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());

    // FIXME The test passes on most machines (including Linux x86_64 with gcc-4.7), but fails on various configs:
    // Linux arm, Linux x86_64 with gcc-4.8 and Mac. Need to figure out what goes wrong and fix that.
#if 0
    style::LineSpacing alineSpacing = getProperty<style::LineSpacing>(xParaEnum->nextElement(), "ParaLineSpacing");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(13200), static_cast<sal_Int16>(alineSpacing.Height));
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "line", "31680");
#endif
}

DECLARE_OOXMLEXPORT_TEST(testFdo71646, "fdo71646.docx")
{
    // The problem was after save file created by MS the direction changed to RTL.
    uno::Reference<uno::XInterface> xParaLTRLeft(getParagraph(1, "LTR LEFT"));
    sal_Int32 nLTRLeft = getProperty<sal_Int32>(xParaLTRLeft, "ParaAdjust");
    // test the text Direction value for the pragraph
    sal_Int16 nLRDir = getProperty<sal_Int32>(xParaLTRLeft, "WritingMode");

    // this will test the both the text direction and alignment for paragraph
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_LEFT), nLTRLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLRDir);
}

DECLARE_OOXMLEXPORT_TEST(testParaAutoSpacing, "para-auto-spacing.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "beforeAutospacing", "1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "afterAutospacing", "1");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "beforeAutospacing", "");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "afterAutospacing", "");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "before", "400");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "after", "400");
}
DECLARE_OOXMLEXPORT_TEST(testThemePreservation, "theme-preservation.docx")
{
    // check default font theme values have been preserved
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts", "asciiTheme",
                "minorHAnsi");
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts", "cstheme",
                "minorBidi");

    // check the font theme values in style definitions
    assertXPath(pXmlStyles, "/w:styles/w:style[1]/w:rPr/w:rFonts", "eastAsiaTheme",
                "minorEastAsia");

    // check the color theme values in style definitions
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Custom1']/w:rPr/w:color", "themeColor",
                "accent1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Custom1']/w:rPr/w:color", "themeTint",
                "99");

    // check direct format font theme values have been preserved
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:r[1]/w:rPr/w:rFonts", "hAnsiTheme",
                "majorHAnsi");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:r[1]/w:rPr/w:rFonts", "asciiTheme",
                "majorHAnsi");

    // check theme font color value has been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w:color", "themeColor",
                "accent3");
    OUString sThemeShade
        = getXPath(pXmlDocument, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w:color", "themeShade");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xbf), sThemeShade.toInt32(16));

    // check the themeFontLang values in settings file
    xmlDocPtr pXmlSettings = parseExport("word/settings.xml");
    if (!pXmlSettings)
        return;
    assertXPath(pXmlSettings, "/w:settings/w:themeFontLang", "val", "en-US");
    assertXPath(pXmlSettings, "/w:settings/w:themeFontLang", "eastAsia", "zh-CN");
    assertXPath(pXmlSettings, "/w:settings/w:themeFontLang", "bidi", "he-IL");

    // check fonts have been applied properly
    sal_Unicode fontName[2]; //represents the string "宋体"
    fontName[0] = 0x5b8b;
    fontName[1] = 0x4f53;
    CPPUNIT_ASSERT_EQUAL(OUString(fontName, 2),
                         getProperty<OUString>(getParagraph(1), "CharFontNameAsian"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"),
                         getProperty<OUString>(getParagraph(2), "CharFontNameComplex"));
    CPPUNIT_ASSERT_EQUAL(
        OUString("Trebuchet MS"),
        getProperty<OUString>(getParagraph(3, "Default style theme font"), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(
        OUString("Arial Black"),
        getProperty<OUString>(getRun(getParagraph(4, "Direct format font"), 1), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(
        OUString("Trebuchet MS"),
        getProperty<OUString>(getParagraph(5, "Major theme font"), "CharFontName"));

    // check the paragraph background pattern has been preserved including theme colors
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "val", "thinHorzStripe");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "themeFill", "text2");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "themeFillTint", "33");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "themeColor", "accent1");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "themeShade", "80");
}

DECLARE_OOXMLEXPORT_TEST(testFDO78384, "fdo78384.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w:rFonts", "ascii", "Wingdings");
}

DECLARE_OOXMLEXPORT_TEST(testfdo76934, "fdo76934.docx")
{
    /* Issue was, AutoSpacing property if present inside styles.xml, LO was not able to
    * preserve it.
    */

    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");

    if (!pXmlDoc)
        return;

    // Ensure that after fix LO is preserving AutoSpacing property in styles.xml
    assertXPath(pXmlDoc, "/w:styles[1]/w:style[@w:styleId='Title']/w:pPr[1]/w:spacing[1]",
                "beforeAutospacing", "1");
}

DECLARE_OOXMLEXPORT_TEST(testNonBMPChar, "nonbmpchar.docx")
{
    sal_uInt32 nCh = 0x24b62;
    OUString aExpected(&nCh, 1);
    // Assert that UTF8 encoded non-BMP Unicode character is correct
    uno::Reference<text::XTextRange> xTextRange1 = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange1->getString());
}

DECLARE_OOXMLEXPORT_TEST(testFdo76101, "fdo76101.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/styles.xml");

    if (!pXmlDoc)
        return;
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/w:styles/w:style");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT(4091 >= xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
}

DECLARE_OOXMLEXPORT_TEST(testTdf89377, "tdf89377_tableWithBreakBeforeParaStyle.docx")
{
    // the paragraph style should set table's text-flow break-before-page
    CPPUNIT_ASSERT_EQUAL(3, getPages());

    uno::Reference<beans::XPropertySet> xStyle(
        getStyles("ParagraphStyles")->getByName("Default Style"), uno::UNO_QUERY);
    //tdf107801: kerning info wasn't exported previously.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("AutoKern should be true", true,
                                 getProperty<bool>(xStyle, "CharAutoKerning"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf109310_endnoteStyleForMSO, "tdf109310_endnoteStyleForMSO.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/endnotes.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:endnotes/w:endnote[@w:id='2']/w:p/w:r[1]/w:rPr/w:rStyle", "w:val");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
