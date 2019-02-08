/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <unotools/tempfile.hxx>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <ftninfo.hxx>

#include <string>

#if !defined(_WIN32)

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        const char* aBlacklist[] = {
            "math-escape.docx",
            "math-mso2k7.docx"
        };
        std::vector<const char*> vBlacklist(aBlacklist, aBlacklist + SAL_N_ELEMENTS(aBlacklist));

        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx") && std::find(vBlacklist.begin(), vBlacklist.end(), filename) == vBlacklist.end());
    }
};

DECLARE_OOXMLEXPORT_TEST(testFdo68418, "fdo68418.docx")
{
    // The problem was that in 'MSWordExportBase::SectionProperties' function in 'wrt8sty.cxx'
    // it checked if it 'IsPlausableSingleWordSection'.
    // The 'IsPlausableSingleWordSection' compared different aspects of 2 'SwFrameFormat' objects.
    // One of the checks was 'do both formats have the same distance from the top and bottom ?'
    // This check is correct if both have headers or both don't have headers.
    // However - if one has a header, and the other one has an empty header (no header) - it is not correct to compare
    // between them (same goes for 'footer').
    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "FooterText");
    uno::Reference< text::XTextRange > xFooterParagraph = getParagraphOfText( 1, xFooterText );

    // First page footer is empty, second page footer is 'aaaa'
    CPPUNIT_ASSERT_EQUAL(OUString("aaaa"), xFooterParagraph->getString());        // I get an error that it expects ''
}

DECLARE_OOXMLEXPORT_TEST(testA4AndBorders, "a4andborders.docx")
{
    /*
     * The problem was that in case of a document with borders, the pgSz attribute
     * was exported as a child of pgBorders, thus being ignored on reload.
     * We assert dimension against A4 size in mm (to avoid minor rounding errors)
     */
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Page Width (mm)", sal_Int32(210), getProperty<sal_Int32>(xPageStyle, "Width") / 100);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Page Height (mm)", sal_Int32(297), getProperty<sal_Int32>(xPageStyle, "Height") / 100);
}

DECLARE_OOXMLEXPORT_TEST(testFdo68787, "fdo68787.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    // This was 25, the 'lack of w:separator' <-> '0 line width' mapping was missing.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf92470_footnoteRestart, "tdf92470_footnoteRestart.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT( pTextDoc );
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT( pDoc );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Footnote doesn't restart every Page", FTNNUM_PAGE, pDoc->GetFootnoteInfo().eNum );

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(25), getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testCharacterBorder, "charborder.odt")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // OOXML has just one border attribute (<w:bdr>) for text border so all side has
    // the same border with the same padding
    // Border
    {
        const table::BorderLine2 aTopBorder = getProperty<table::BorderLine2>(xRun,"CharTopBorder");
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF6600,0,318,0,0,318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    // Padding (w:space)
    {
        const sal_Int32 nTopPadding = getProperty<sal_Int32>(xRun,"CharTopBorderDistance");
        // In the original ODT the padding is 150, but the unit conversion round it down.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(141), nTopPadding);
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    // Shadow (w:shadow)
    /* OOXML use just one bool value for shadow so the next conversions
       are made during an export-import round
       color: any -> black
       location: any -> bottom-right
       width: any -> border width */
    {
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xRun, "CharShadowFormat");
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }

    // Also check shadow when it is in middle of the paragraph
    // (problem can be during export with SwWW8AttrIter::HasTextItem())
    {
        uno::Reference<beans::XPropertySet> xMiddleRun(getRun(getParagraph(2),2), uno::UNO_QUERY);
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xMiddleRun, "CharShadowFormat");
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
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference< beans::XPropertySet > properties(paragraphStyles->getByName("Heading 1"), uno::UNO_QUERY);
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
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts", "ascii", "Times New Roman");
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:lang", "bidi", "ar-SA");
    // pPrDefault is empty
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:pPrDefault/w:pPr/*", 0);

    // Check latent styles
    uno::Sequence<beans::PropertyValue> aGrabBag = getProperty< uno::Sequence<beans::PropertyValue> >(mxComponent, "InteropGrabBag");
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
    CPPUNIT_ASSERT_EQUAL(OUString("Normal"), aName); // This checks the "name" attribute of the first exception.

    // This numbering style wasn't roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoList']/w:name", "val", "No List");

    // Table style wasn't roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:tblPr/w:tblCellMar/w:left", "w", "108");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:semiHidden", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:unhideWhenUsed", 1);

    // Additional para style properties should be also roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListParagraph']/w:uiPriority", "val", "34");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:qFormat", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:rsid", "val", "00780346");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']", "default", "1");

    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:link", "val", "Heading1Char");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:locked", 1);

    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading11']", "customStyle", "1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading11']/w:autoRedefine", 1);

    // Additional char style properties should be also roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='DefaultParagraphFont']", "default", "1");

    // Finally check the same for numbering styles.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoList']", "default", "1");

    // This was 1, the default style had <w:suppressAutoHyphens w:val="true"/> even for a default style having no RES_PARATR_HYPHENZONE set.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:pPr/w:suppressAutoHyphens", 0);
}

DECLARE_OOXMLEXPORT_TEST(testCalendar1, "calendar1.docx")
{
    // Document has a non-trivial table style, test the roundtrip of it.
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:basedOn", "val", "TableNormal");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:rsid", "val", "00903003");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblPr/w:tblStyleColBandSize", "val", "1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tcPr/w:shd", "val", "clear");

    // Table style lost its paragraph / run properties.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:pPr/w:spacing", "lineRule", "auto");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:rPr/w:lang", "eastAsia", "ja-JP");

    // Table style lost its conditional table formatting properties.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:pPr/w:wordWrap", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:rFonts", "hAnsiTheme", "minorHAnsi");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:tblPr", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:tcPr/w:vAlign", "val", "bottom");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='lastRow']/w:tcPr/w:tcBorders/w:tr2bl", "val", "nil");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='band2Horz']/w:tcPr/w:tcBorders/w:top", "themeColor", "text1");

    // w:tblLook element and its attributes were missing.
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "firstRow", "1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "lastRow", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "lastColumn", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "firstColumn", "1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "noHBand", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "noVBand", "1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook", "val", "04a0");
}

DECLARE_OOXMLEXPORT_TEST(testCalendar2, "calendar2.docx")
{
    // Problem was that CharCaseMap was style::CaseMap::NONE.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE, getProperty<sal_Int16>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharCaseMap"));
    // Font size in the second row was 11.
    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharHeight"));
    // Font size in the third row was 11 as well.
    xCell.set(xTable->getCellByName("B3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(14.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharHeight"));

    // This paragraph property was missing in table style.
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:pPr/w:jc", "val", "center");

    // These run properties were missing
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:rPr/w:lang", "val", "en-US");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:rPr/w:lang", "bidi", "ar-SA");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:caps", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:smallCaps", "val", "0");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:color", "themeColor", "accent1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:spacing", "val", "20");

    // Table borders were also missing
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblPr/w:tblBorders/w:insideV", "themeTint", "99");
}

DECLARE_OOXMLEXPORT_TEST(testCalendar3, "calendar3.docx")
{
    // TableStyle:firstRow (for header rows 1 and 2) color and size overrides document rPrDefault
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x5B9BD5), getProperty<sal_Int32>(getRun(xCell,1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(getRun(xCell,1), "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testTcBorders, "testTcBorders.docx")
{
    //fdo#76635 : Table borders are not getting preserved.

    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    assertXPath(pXmlDocument, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/w:bottom[1][@w:val = 'single']",1);
    assertXPath(pXmlDocument, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/w:bottom[1][@w:sz = 4]", 1);
    assertXPath(pXmlDocument, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/w:bottom[1][@w:space = 0]", 1);
    assertXPath(pXmlDocument, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/w:bottom[1][@w:color = 808080]", 1);
}

DECLARE_OOXMLEXPORT_TEST(testQuicktables, "quicktables.docx")
{
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;

    // These were missing in the Calendar3 table style.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:rPr/w:rFonts", "cstheme", "majorBidi");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:rPr/w:color", "themeTint", "80");
    CPPUNIT_ASSERT(getXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:color", "themeShade").equalsIgnoreAsciiCase("BF"));

    // Calendar4.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:pPr/w:snapToGrid", "val", "0");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:rPr/w:bCs", 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tcPr/w:shd", "themeFill", "accent1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tcPr/w:shd", "themeFillShade", "80");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='firstCol']/w:pPr/w:ind", "rightChars", "0");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='firstCol']/w:pPr/w:ind", "right", "144");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='band2Horz']/w:tcPr/w:tcMar/w:bottom", "w", "86");

    // LightList.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='LightList']/w:tblStylePr[@w:type='firstRow']/w:pPr/w:spacing", "before", "0");

    // MediumList2-Accent1.
    CPPUNIT_ASSERT(getXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MediumList2-Accent1']/w:tblStylePr[@w:type='band1Vert']/w:tcPr/w:shd", "themeFillTint").equalsIgnoreAsciiCase("3F"));

    // MediumShading2-Accent5.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MediumShading2-Accent5']/w:tblStylePr[@w:type='firstRow']/w:tcPr/w:tcBorders/w:top", "color", "auto");
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
    for(int i = 0; i < aGrabBag.getLength(); ++i)
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
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount()); // One groupshape in the doc

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xGroup->getCount()); // 1 rendered bitmap from the original shapes

    uno::Reference<beans::XPropertySet> xGroupPropertySet(getShape(1), uno::UNO_QUERY);
    xGroupPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty

    bool bData = false, bLayout = false, bQStyle = false, bColor = false, bDrawing = false;
    for(int i = 0; i < aGrabBag.getLength(); ++i)
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
        uno::Sequence< uno::Any > diagramDrawing;
        uno::Reference<xml::dom::XDocument> aDrawingDom;
        CPPUNIT_ASSERT(aGrabBag[i].Value >>= diagramDrawing);
        CPPUNIT_ASSERT(diagramDrawing[0] >>= aDrawingDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aDrawingDom.get()); // Reference not empty
      }
    }
    CPPUNIT_ASSERT(bData && bLayout && bQStyle && bColor && bDrawing); // Grab Bag has all the expected elements

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
    for(sal_Int32 index = 0; index < length; index++){
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[index];
        OUString attrVal = OUString::createFromAscii(reinterpret_cast<char*>(xmlGetProp(pXmlNode, BAD_CAST("name"))));
        if (attrVal.isEmpty()){
            CPPUNIT_FAIL("Font name is empty.");
        }
    }
    xmlXPathFreeObject(pXmlObj);
}

DECLARE_OOXMLEXPORT_TEST(testMultiColumnLineSeparator, "multi-column-line-separator-SAVED.docx")
{
    // Check for the Column Separator value.It should be FALSE as the document does not contain separator line.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:sectPr/w:cols","sep","false");
}

DECLARE_OOXMLEXPORT_TEST(testCustomXmlGrabBag, "customxml.docx")
{
    // The problem was that item[n].xml and itemProps[n].xml and .rels files for item[n].xml
    // files were missing from docx file after saving file.
    // This test case tests whether customxml files grabbagged properly in correct object.

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGrabBag(0);
    xTextDocumentPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty
    bool CustomXml = false;
    for(int i = 0; i < aGrabBag.getLength(); ++i)
    {
        if (aGrabBag[i].Name == "OOXCustomXml" || aGrabBag[i].Name == "OOXCustomXmlProps")
        {
            CustomXml = true;
            uno::Reference<xml::dom::XDocument> aCustomXmlDom;
            uno::Sequence<uno::Reference<xml::dom::XDocument> > aCustomXmlDomList;
            CPPUNIT_ASSERT(aGrabBag[i].Value >>= aCustomXmlDomList); // PropertyValue of proper type
            sal_Int32 length = aCustomXmlDomList.getLength();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), length);
            aCustomXmlDom = aCustomXmlDomList[0];
            CPPUNIT_ASSERT(aCustomXmlDom.get()); // Reference not empty
        }
    }
    CPPUNIT_ASSERT(CustomXml); // Grab Bag has all the expected elements
}

DECLARE_OOXMLEXPORT_TEST(testCustomXmlRelationships, "customxml.docx")
{
    xmlDocPtr pXmlDoc = parseExport("customXml/_rels/item1.xml.rels");
    if(!pXmlDoc)
        return;

    // Check there is a relation to itemProps1.xml.
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship", 1);
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']", "Target", "itemProps1.xml");
}

DECLARE_OOXMLEXPORT_TEST(testFdo69644, "fdo69644.docx")
{
    // The problem was that the exporter exported the table definition
    // with only 3 columns, instead of 5 columns.
    // Check that the table grid is exported with 5 columns
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol", 5);
}

DECLARE_OOXMLEXPORT_TEST(testFdo70812, "fdo70812.docx")
{
    // Import just crashed.
    getParagraph(1, "Sample pages document.");
}

DECLARE_OOXMLEXPORT_TEST(testPgMargin, "testPgMargin.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgMar", "left", "1440");
}

DECLARE_OOXMLEXPORT_TEST(testImageCrop, "ImageCrop.docx")
{
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropStruct;

    CPPUNIT_ASSERT_EQUAL( sal_Int32( 2955 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 5477 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 2856 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 2291 ), aGraphicCropStruct.Bottom );
}

DECLARE_OOXMLEXPORT_TEST(testTdf106974_int32Crop, "tdf106974_int32Crop.docx")
{
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropStruct;

    CPPUNIT_ASSERT_MESSAGE(
        OString::number(aGraphicCropStruct.Right).getStr(),
        sal_Int32( 40470 ) < aGraphicCropStruct.Right );
}

DECLARE_OOXMLEXPORT_TEST(testLineSpacingexport, "test_line_spacing.docx")
{
     // The Problem was that the w:line attribute value in w:spacing tag was incorrect
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
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

DECLARE_OOXMLEXPORT_TEST(testTextBoxGradientAngle, "fdo65295.docx")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xIndexAccess->getCount());

    // Angle of frame#1 is 135 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame1, "FillStyle"));
    awt::Gradient aGradient1 = getProperty<awt::Gradient>(xFrame1, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(135 * 10), aGradient1.Angle);

    // Angle of frame#2 is 180 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame2, "FillStyle"));
    awt::Gradient aGradient2 = getProperty<awt::Gradient>(xFrame2, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(180 * 10), aGradient2.Angle);

    // Angle of frame#3 is  90 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame3(xIndexAccess->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame3, "FillStyle"));
    awt::Gradient aGradient3 = getProperty<awt::Gradient>(xFrame3, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16( 90 * 10), aGradient3.Angle);

    // Angle of frame#4 is 225 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame4(xIndexAccess->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame4, "FillStyle"));
    awt::Gradient aGradient4 = getProperty<awt::Gradient>(xFrame4, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(225 * 10), aGradient4.Angle);

    // Angle of frame#5 is 270 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame5(xIndexAccess->getByIndex(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame5, "FillStyle"));
    awt::Gradient aGradient5 = getProperty<awt::Gradient>(xFrame5, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(270 * 10), aGradient5.Angle);

    // Angle of frame#6 is 315 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame6(xIndexAccess->getByIndex(5), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame6, "FillStyle"));
    awt::Gradient aGradient6 = getProperty<awt::Gradient>(xFrame6, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(315 * 10), aGradient6.Angle);

    // Angle of frame#7 is   0 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame7(xIndexAccess->getByIndex(6), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame7, "FillStyle"));
    awt::Gradient aGradient7 = getProperty<awt::Gradient>(xFrame7, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(  0 * 10), aGradient7.Angle);

    // Angle of frame#8 is  45 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame8(xIndexAccess->getByIndex(7), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame8, "FillStyle"));
    awt::Gradient aGradient8 = getProperty<awt::Gradient>(xFrame8, "FillGradient");
    CPPUNIT_ASSERT_EQUAL(sal_Int16( 45 * 10), aGradient8.Angle);
}

DECLARE_OOXMLEXPORT_TEST(testCellGridSpan, "cell-grid-span.docx")
{
    // The problem was during export gridSpan value for 1st & 2nd cells for test document
    // used to get set wrongly to 5 and 65532 respectively which was the reason for crash during save operation
    // Verifying gridSpan element is not present in RoundTripped Document (As it's Default value is 1).
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:tcPr/w:gridSpan",0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:gridSpan",0);
}
DECLARE_OOXMLEXPORT_TEST(testFdo71646, "fdo71646.docx")
{
    // The problem was after save file created by MS the direction changed to RTL.
    uno::Reference<uno::XInterface> xParaLTRLeft(getParagraph( 1, "LTR LEFT"));
    sal_Int32 nLTRLeft = getProperty< sal_Int32 >( xParaLTRLeft, "ParaAdjust" );
    // test the text Direction value for the pragraph
    sal_Int16 nLRDir  = getProperty< sal_Int32 >( xParaLTRLeft, "WritingMode" );

    // this will test the both the text direction and alignment for paragraph
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_LEFT), nLTRLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLRDir);
}

DECLARE_OOXMLEXPORT_TEST(testParaAutoSpacing, "para-auto-spacing.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "beforeAutospacing","1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing", "afterAutospacing","1");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "beforeAutospacing","");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "afterAutospacing","");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "before","400");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing", "after","400");
}

DECLARE_OOXMLEXPORT_TEST(testGIFImageCrop, "test_GIF_ImageCrop.docx")
{
    // FIXME why does this fail on Mac?
#if !defined(MACOSX)
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropStruct;

    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1085 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 3651 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 953 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1244 ), aGraphicCropStruct.Bottom );
#endif
}

DECLARE_OOXMLEXPORT_TEST(testPNGImageCrop, "test_PNG_ImageCrop.docx")
{
    // FIXME why does this fail on Mac?
#if !defined(MACOSX)
    /* The problem was image cropping information was not getting saved
     * after roundtrip.
     * Check for presence of cropping parameters in exported file.
     */
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( "GraphicCrop" ) >>= aGraphicCropStruct;

    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1058 ), aGraphicCropStruct.Left );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1111 ), aGraphicCropStruct.Right );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 1164 ), aGraphicCropStruct.Top );
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 635 ), aGraphicCropStruct.Bottom );
#endif
}

DECLARE_OOXMLEXPORT_TEST(testTdf41542_imagePadding, "tdf41542_imagePadding.odt")
{
    // borderlessImage - image WITHOUT BORDERS : simulate padding with -crop
    text::GraphicCrop crop = getProperty<text::GraphicCrop>(getShape(2), "GraphicCrop");
    CPPUNIT_ASSERT( crop.Left != 0 && crop.Right != 0 );
    CPPUNIT_ASSERT( crop.Left == crop.Top && crop.Right == crop.Bottom && crop.Left == crop.Right);

    // borderedImage - image WITH BORDERS : simulate padding with -crop
    crop = getProperty<text::GraphicCrop>(getShape(3), "GraphicCrop");
    CPPUNIT_ASSERT( crop.Left != 0 && crop.Right != 0 );
    CPPUNIT_ASSERT( crop.Left == crop.Top && crop.Right == crop.Bottom && crop.Left == crop.Right);
}

DECLARE_OOXMLEXPORT_TEST(testFootnoteParagraphTag, "testFootnote.docx")
{
    /* In footnotes.xml, the paragraph tag inside <w:footnote w:id="2"> was getting written into document.xml.
     * Check for, paragraph tag is correctly written into footnotes.xml.
     */
    xmlDocPtr pXmlFootnotes = parseExport("word/footnotes.xml");
    if (!pXmlFootnotes)
        return;
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]","id","2");
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]/w:p/w:r/w:footnoteRef", 1);
}

DECLARE_OOXMLEXPORT_TEST(testSpacingLineRule,"table_lineRule.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p/w:pPr/w:spacing", "lineRule", "auto");
}

DECLARE_OOXMLEXPORT_TEST(testTableLineSpacing, "table_atleast.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:pPr/w:spacing", "line", "320");
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlTriangle, "ooxml-triangle.docx")
{
    // The problem was that ooxml-triangle shape type wasn't handled by VML
    // export (only isosceles-triangle), leading to a missing shape.
    getShape(1);
}

DECLARE_OOXMLEXPORT_TEST(testMce, "mce.docx")
{
    // The shape is red in Word2007, green in Word2010. Check that our import follows the later.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x9bbb59), getProperty<sal_Int32>(getShape(1), "FillColor"));
}

DECLARE_OOXMLEXPORT_TEST(testThemePreservation, "theme-preservation.docx")
{
    // check default font theme values have been preserved
    xmlDocPtr pXmlStyles = parseExport("word/styles.xml");
    if (!pXmlStyles)
        return;
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts", "asciiTheme", "minorHAnsi");
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts", "cstheme", "minorBidi");

    // check the font theme values in style definitions
    assertXPath(pXmlStyles, "/w:styles/w:style[1]/w:rPr/w:rFonts", "eastAsiaTheme", "minorEastAsia");

    // check the color theme values in style definitions
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Custom1']/w:rPr/w:color", "themeColor", "accent1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Custom1']/w:rPr/w:color", "themeTint", "99");

    // check direct format font theme values have been preserved
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:r[1]/w:rPr/w:rFonts", "hAnsiTheme", "majorHAnsi");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:r[1]/w:rPr/w:rFonts", "asciiTheme", "majorHAnsi");

    // check theme font color value has been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w:color", "themeColor", "accent3");
    OUString sThemeShade = getXPath(pXmlDocument, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w:color", "themeShade");
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
    CPPUNIT_ASSERT_EQUAL(OUString(fontName, 2), getProperty<OUString>(getParagraph(1), "CharFontNameAsian"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"),
                         getProperty<OUString>(getParagraph(2), "CharFontNameComplex"));
    CPPUNIT_ASSERT_EQUAL(OUString("Trebuchet MS"),
                         getProperty<OUString>(getParagraph(3, "Default style theme font"), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial Black"),
                         getProperty<OUString>(getRun(getParagraph(4, "Direct format font"), 1), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Trebuchet MS"),
                         getProperty<OUString>(getParagraph(5, "Major theme font"), "CharFontName"));

    // check the paragraph background pattern has been preserved including theme colors
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "val", "thinHorzStripe");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "themeFill", "text2");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "themeFillTint", "33");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "themeColor", "accent1");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd", "themeShade", "80");
}

DECLARE_OOXMLEXPORT_TEST(testTableThemePreservation, "table-theme-preservation.docx")
{
    xmlDocPtr pXmlDocument = parseExport("word/document.xml");
    if (!pXmlDocument)
        return;

    // check cell theme colors have been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:shd", "themeFill", "accent6");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:shd", "themeFillShade", "");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:shd", "themeFillTint", "33");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:shd", "themeFill", "accent6");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:shd", "themeFillShade", "");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:shd", "themeFillTint", "");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "themeFill", "accent6");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "themeFillShade", "80");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "themeFillTint", "");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "val", "horzStripe");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "themeColor", "accent3");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "themeTint", "33");
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd", "color", "E7EEEE");

    // check table style has been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tblPr/w:tblStyle", "val", "Sombreadoclaro-nfasis1");
    // check table style is not overwritten by other properties
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[2]/w:tcPr/w:tcBorders/*", 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[3]/w:tcPr/w:tcBorders/*", 0);
    // check that one cell attribute present in the original document has been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/*", 1);

    // Check that w:cnfStyle row, cell and paragraph property is preserved.
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle", "val", "100000000000");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle", "firstRow", "1");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle", "lastRow", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle", "firstColumn", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle", "lastColumn", "0");

    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle", "val", "001000000000");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle", "oddVBand", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle", "evenVBand", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle", "oddHBand", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle", "evenHBand", "0");

    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle", "val", "100000000000");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle", "firstRowFirstColumn", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle", "firstRowLastColumn", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle", "lastRowFirstColumn", "0");
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle", "lastRowLastColumn", "0");

}

DECLARE_OOXMLEXPORT_TEST(testcantSplit, "2_table_doc.docx")
{
    // if Split table value is true for a table then during export do not write <w:cantSplit w:val="false"/>
    // in table row property,As default row prop is allow row to break across page.
    // writing <w:cantSplit w:val="false"/> during export was causing problem that all the cell data used to come on same page
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr/w:trPr/w:cantSplit",0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr/w:trPr/w:cantSplit","val","true");
}

DECLARE_OOXMLEXPORT_TEST(testDontSplitTable, "tdf101589_dontSplitTable.odt")
{
    //single row tables need to prevent split by setting row to no split
    CPPUNIT_ASSERT_EQUAL( OUString("Row 1"), parseDump("/root/page[2]/body/tab[1]/row[1]/cell[1]/txt[1]") );

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable (xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTable, "Split"));

    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTableRows->getByIndex(0), "IsSplitAllowed"));
}

DECLARE_OOXMLEXPORT_TEST(testExtraSectionBreak, "1_page.docx")
{
    // There was a problem for some documents during export.Invalid sectPr getting added
    // because of faulty calculation of PageDesc value
    // This was the reason for increasing number of pages after RT
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testcolumnbreak, "columnbreak.docx")
{
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(5, "This is first line after col brk."), "BreakType"));
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[1]/w:br", "type", "column");
}

DECLARE_OOXMLEXPORT_TEST(testGlossary, "testGlossary.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/glossary/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:glossaryDocument", "Ignorable", "w14 wp14");
}

DECLARE_OOXMLEXPORT_TEST(testFdo71785, "fdo71785.docx")
{
    // crashtest
}

DECLARE_OOXMLEXPORT_TEST(testCrashWhileSave, "testCrashWhileSave.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/footer1.xml");
    if (!pXmlDoc)
        return;
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:ftr/w:tbl/w:tr/w:tc[1]/w:p[1]/w:pPr/w:pStyle", "val").match("Normal"));
}

DECLARE_OOXMLEXPORT_TEST(testFileOpenInputOutputError,"floatingtbl_with_formula.docx")
{
    // Docx containing Floating table with formula was giving "General input/output error" while opening in LibreOffice
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pStyle", "val", "Normal");

    // let's also assert that the formula was exported properly
    assertXPathContent(pXmlDoc, "//w:tbl/w:tr/w:tc[2]/w:p/m:oMath/m:sSubSup/m:e/m:r/m:t", u"\u03C3");
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
