/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/style/BreakType.hpp>

#include <ftninfo.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
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
    uno::Reference<text::XText> xFooterText = getProperty< uno::Reference<text::XText> >(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"FooterText"_ustr);
    uno::Reference< text::XTextRange > xFooterParagraph = getParagraphOfText( 1, xFooterText );

    // First page footer is empty, second page footer is 'aaaa'
    CPPUNIT_ASSERT_EQUAL(u"aaaa"_ustr, xFooterParagraph->getString());        // I get an error that it expects ''
}

DECLARE_OOXMLEXPORT_TEST(testA4AndBorders, "a4andborders.docx")
{
    /*
     * The problem was that in case of a document with borders, the pgSz attribute
     * was exported as a child of pgBorders, thus being ignored on reload.
     * We assert dimension against A4 size in mm (to avoid minor rounding errors)
     */
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Page Width (mm)", sal_Int32(210), getProperty<sal_Int32>(xPageStyle, u"Width"_ustr) / 100);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Page Height (mm)", sal_Int32(297), getProperty<sal_Int32>(xPageStyle, u"Height"_ustr) / 100);
}

DECLARE_OOXMLEXPORT_TEST(testFdo68787, "fdo68787.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was 25, the 'lack of w:separator' <-> '0 line width' mapping was missing.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPageStyle, u"FootnoteLineRelativeWidth"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf92470_footnoteRestart, "tdf92470_footnoteRestart.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT( pTextDoc );
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT( pDoc );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Footnote doesn't restart every Page", FTNNUM_PAGE, pDoc->GetFootnoteInfo().m_eNum );

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(25), getProperty<sal_Int32>(xPageStyle, u"FootnoteLineRelativeWidth"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108944_footnoteSeparator2)
{
    loadAndReload("tdf108944_footnoteSeparator2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    //This was zero. The comment was causing the bHasFtnSep flag to be reset to false, so the separator was missing.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(25), getProperty<sal_Int32>(xPageStyle, u"FootnoteLineRelativeWidth"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testCharacterBorder)
{
    loadAndReload("charborder.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // OOXML has just one border attribute (<w:bdr>) for text border so all side has
    // the same border with the same padding
    // Border
    {
        const table::BorderLine2 aTopBorder = getProperty<table::BorderLine2>(xRun,u"CharTopBorder"_ustr);
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF6600,0,318,0,0,318), aTopBorder);
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,u"CharLeftBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,u"CharBottomBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(aTopBorder, getProperty<table::BorderLine2>(xRun,u"CharRightBorder"_ustr));
    }

    // Padding (w:space)
    {
        const sal_Int32 nTopPadding = getProperty<sal_Int32>(xRun,u"CharTopBorderDistance"_ustr);
        // In the original ODT the padding is 150, but the unit conversion round it down.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(141), nTopPadding);
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,u"CharLeftBorderDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,u"CharBottomBorderDistance"_ustr));
        CPPUNIT_ASSERT_EQUAL(nTopPadding, getProperty<sal_Int32>(xRun,u"CharRightBorderDistance"_ustr));
    }

    // Shadow (w:shadow)
    /* OOXML use just one bool value for shadow so the next conversions
       are made during an export-import round
       color: any -> black
       location: any -> bottom-right
       width: any -> border width */
    {
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xRun, u"CharShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }

    // Also check shadow when it is in middle of the paragraph
    // (problem can be during export with SwWW8AttrIter::HasTextItem())
    {
        uno::Reference<beans::XPropertySet> xMiddleRun(getRun(getParagraph(2),2), uno::UNO_QUERY);
        const table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xMiddleRun, u"CharShadowFormat"_ustr);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aShadow.Color));
        CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(318), aShadow.ShadowWidth);
    }

    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    // Make sure we write qFormat for custom style names.
    assertXPath(pXmlStyles, "//w:style[@w:styleId='Heading']/w:qFormat"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testStyleInheritance)
{
    loadAndSave("style-inheritance.docx");

    // Check that now styleId's are more like what MSO produces
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    // the 1st style always must be Normal
    assertXPath(pXmlStyles, "/w:styles/w:style[1]"_ostr, "styleId"_ostr, u"Normal"_ustr);
    // some random style later
    assertXPath(pXmlStyles, "/w:styles/w:style[4]"_ostr, "styleId"_ostr, u"Heading3"_ustr);

    // Check that we do _not_ export w:next for styles that point to themselves.
    assertXPath(pXmlStyles, "/w:styles/w:style[1]/w:next"_ostr, 0);

    // Check that we roundtrip <w:next> correctly - on XML level
    assertXPath(pXmlStyles, "/w:styles/w:style[2]/w:next"_ostr, "val"_ostr, u"Normal"_ustr);
    // And to be REALLY sure, check it on the API level too ;-)
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles(u"ParagraphStyles"_ustr);
    uno::Reference< beans::XPropertySet > properties(paragraphStyles->getByName(u"Heading 1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr, getProperty<OUString>(properties, u"FollowStyle"_ustr));

    // This was 0, as export of w:outlineLvl was missing.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(properties, u"OutlineLevel"_ustr));

    properties.set(paragraphStyles->getByName(u"Heading 11"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr, getProperty<OUString>(properties, u"FollowStyle"_ustr));

    // Make sure style #2 is Heading 1.
    assertXPath(pXmlStyles, "/w:styles/w:style[2]"_ostr, "styleId"_ostr, u"Heading1"_ustr);
    // w:ind was copied from the parent (Normal) style without a good reason.
    assertXPath(pXmlStyles, "/w:styles/w:style[2]/w:pPr/w:ind"_ostr, 0);

    // We output exactly 2 properties in rPrDefault, nothing else was
    // introduced as an additional default
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/*"_ostr, 2);
    // Check that we output real content of rPrDefault
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts"_ostr, "ascii"_ostr, u"Times New Roman"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:lang"_ostr, "bidi"_ostr, u"ar-SA"_ustr);
    // pPrDefault contains only one hyphenation property
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:pPrDefault/w:pPr/*"_ostr, 1);

    // Check latent styles
    uno::Sequence<beans::PropertyValue> aGrabBag = getProperty< uno::Sequence<beans::PropertyValue> >(mxComponent, u"InteropGrabBag"_ustr);
    uno::Sequence<beans::PropertyValue> aLatentStyles;
    for (beans::PropertyValue const& prop : aGrabBag)
        if (prop.Name == "latentStyles")
            prop.Value >>= aLatentStyles;
    CPPUNIT_ASSERT(aLatentStyles.getLength()); // document should have latent styles

    // Check latent style default attributes
    OUString aCount;
    uno::Sequence<beans::PropertyValue> aLatentStyleExceptions;
    for (beans::PropertyValue const& prop : aLatentStyles)
    {
        if (prop.Name == "count")
            aCount = prop.Value.get<OUString>();
        else if (prop.Name == "lsdExceptions")
            prop.Value >>= aLatentStyleExceptions;
    }
    CPPUNIT_ASSERT_EQUAL(u"371"_ustr, aCount); // This check the "count" attribute.

    // Check exceptions to the latent style defaults.
    uno::Sequence<beans::PropertyValue> aLatentStyleException;
    aLatentStyleExceptions[0].Value >>= aLatentStyleException;
    OUString aName;
    for (beans::PropertyValue const& prop : aLatentStyleException)
        if (prop.Name == "name")
            aName = prop.Value.get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"Normal"_ustr, aName); // This checks the "name" attribute of the first exception.

    // This numbering style wasn't roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoList']/w:name"_ostr, "val"_ostr, u"No List"_ustr);

    // Table style wasn't roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:tblPr/w:tblCellMar/w:left"_ostr, "w"_ostr, u"108"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:semiHidden"_ostr, 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableNormal']/w:unhideWhenUsed"_ostr, 1);

    // Additional para style properties should be also roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListParagraph']/w:uiPriority"_ostr, "val"_ostr, u"34"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:qFormat"_ostr, 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:rsid"_ostr, "val"_ostr, u"00780346"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']"_ostr, "default"_ostr, u"1"_ustr);

    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:link"_ostr, "val"_ostr, u"Heading1Char"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:locked"_ostr, 1);

    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading11']"_ostr, "customStyle"_ostr, u"1"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading11']/w:autoRedefine"_ostr, 1);

    // Additional char style properties should be also roundtripped.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='DefaultParagraphFont']"_ostr, "default"_ostr, u"1"_ustr);

    // Finally check the same for numbering styles.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoList']"_ostr, "default"_ostr, u"1"_ustr);

    // This was 1, the default style had <w:suppressAutoHyphens w:val="true"/> even for a default style having no RES_PARATR_HYPHENZONE set.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:pPr/w:suppressAutoHyphens"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testCalendar1)
{
    loadAndSave("calendar1.docx");
    // Document has a non-trivial table style, test the roundtrip of it.
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:basedOn"_ostr, "val"_ostr, u"TableNormal"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:rsid"_ostr, "val"_ostr, u"00903003"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblPr/w:tblStyleColBandSize"_ostr, "val"_ostr, u"1"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tcPr/w:shd"_ostr, "val"_ostr, u"clear"_ustr);

    // Table style lost its paragraph / run properties.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:pPr/w:spacing"_ostr, "lineRule"_ostr, u"auto"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:rPr/w:lang"_ostr, "eastAsia"_ostr, u"ja-JP"_ustr);

    // Table style lost its conditional table formatting properties.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:pPr/w:wordWrap"_ostr, 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:rFonts"_ostr, "hAnsiTheme"_ostr, u"minorHAnsi"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:tblPr"_ostr, 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='firstRow']/w:tcPr/w:vAlign"_ostr, "val"_ostr, u"bottom"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='lastRow']/w:tcPr/w:tcBorders/w:tr2bl"_ostr, "val"_ostr, u"nil"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar1']/w:tblStylePr[@w:type='band2Horz']/w:tcPr/w:tcBorders/w:top"_ostr, "themeColor"_ostr, u"text1"_ustr);

    // w:tblLook element and its attributes were missing.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook"_ostr, "firstRow"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook"_ostr, "lastRow"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook"_ostr, "lastColumn"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook"_ostr, "firstColumn"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook"_ostr, "noHBand"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook"_ostr, "noVBand"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblLook"_ostr, "val"_ostr, u"04a0"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testCalendar2, "calendar2.docx")
{
    // Problem was that CharCaseMap was style::CaseMap::NONE.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE, getProperty<sal_Int16>(getRun(getParagraphOfText(1, xCell->getText()), 1), u"CharCaseMap"_ustr));
    // Font size in the second row was 11.
    xCell.set(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1), u"CharHeight"_ustr));
    // Font size in the third row was 11 as well.
    xCell.set(xTable->getCellByName(u"B3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(14.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1), u"CharHeight"_ustr));

    // This paragraph property was missing in table style.
    if (!isExported())
        return;
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:pPr/w:jc"_ostr, "val"_ostr, u"center"_ustr);

    // These run properties were missing
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:rPr/w:lang"_ostr, "val"_ostr, u"en-US"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:rPr/w:lang"_ostr, "bidi"_ostr, u"ar-SA"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:caps"_ostr, 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:smallCaps"_ostr, "val"_ostr, u"0"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:color"_ostr, "themeColor"_ostr, u"accent1"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:spacing"_ostr, "val"_ostr, u"20"_ustr);

    // Table borders were also missing
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar2']/w:tblPr/w:tblBorders/w:insideV"_ostr, "themeTint"_ostr, u"99"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testCalendar3, "calendar3.docx")
{
    // TableStyle:firstRow (for header rows 1 and 2) color and size overrides document rPrDefault
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x5B9BD5), getProperty<Color>(getRun(getParagraphOfText(1, xCell->getText()), 1), u"CharColor"_ustr));
    CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()),1), u"CharHeight"_ustr));
    // direct formatting in A1
    uno::Reference<text::XTextRange> xCell2(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x2E74B5), getProperty<Color>(getRun(getParagraphOfText(1, xCell2->getText()), 1), u"CharColor"_ustr));
    CPPUNIT_ASSERT_EQUAL(20.f, getProperty<float>(getRun(getParagraphOfText(1, xCell2->getText()),1), u"CharHeight"_ustr));

    // tdf#132149 Despite specifying portrait, the page size's specified width is greater than its height.
    // Both Word and LO display it as landscape, so ensure that it round-trips with landscape dimensions.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page Width (mm) ", sal_Int32(148), getProperty<sal_Int32>(xPageStyle, u"Width"_ustr) / 100);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page Height (mm)", sal_Int32(104), getProperty<sal_Int32>(xPageStyle, u"Height"_ustr) / 100);
}

DECLARE_OOXMLEXPORT_TEST(testCalendar4, "calendar4.docx")
{
    // TableStyle:fontsize - overrides DocDefaults, but not Style.
    // In this case the style does not define anything (but does copy-inherit the DocDefaults size 36)
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(14.f, getProperty<float>(getRun(xCell,1), u"CharHeight"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testCalendar5, "calendar5.docx")
{
    // check text portions with and without direct formatting
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    // text portions with direct formatting
    CPPUNIT_ASSERT_EQUAL(Color(0x2E74B5), getProperty<Color>(getRun(getParagraphOfText(1, xCell->getText()), 1), u"CharColor"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(getRun(getParagraphOfText(1, xCell->getText()), 2), u"CharColor"_ustr));
    // default paragraph text color
    CPPUNIT_ASSERT_EQUAL(Color(0x5B9BD5), getProperty<Color>(getRun(getParagraphOfText(1, xCell->getText()), 3), u"CharColor"_ustr));
    // text portions with direct formatting
    CPPUNIT_ASSERT_EQUAL(20.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()),1), u"CharHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(10.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()),2), u"CharHeight"_ustr));
    // default paragraph text size
    CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()),3), u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTcBorders)
{
    loadAndSave("testTcBorders.docx");
    //fdo#76635 : Table borders are not getting preserved.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/w:bottom[1][@w:val = 'single']"_ostr,1);
    assertXPath(pXmlDocument, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/w:bottom[1][@w:sz = 4]"_ostr, 1);
    assertXPath(pXmlDocument, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/w:bottom[1][@w:space = 0]"_ostr, 1);
    assertXPath(pXmlDocument, "/w:document[1]/w:body[1]/w:tbl[1]/w:tr[1]/w:tc[1]/w:tcPr[1]/w:tcBorders[1]/w:bottom[1][@w:color = 808080]"_ostr, 1);

    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"CharacterStyles"_ustr)->getByName(u"Code Featured Element"_ustr),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bold", float(150), getProperty<float>(xStyle, u"CharWeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testQuicktables)
{
    loadAndSave("quicktables.docx");
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);

    // These were missing in the Calendar3 table style.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:rPr/w:rFonts"_ostr, "cstheme"_ostr, u"majorBidi"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:rPr/w:color"_ostr, "themeTint"_ostr, u"80"_ustr);
    CPPUNIT_ASSERT(getXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar3']/w:tblStylePr[@w:type='firstRow']/w:rPr/w:color"_ostr, "themeShade"_ostr).equalsIgnoreAsciiCase("BF"));

    // Calendar4.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:pPr/w:snapToGrid"_ostr, "val"_ostr, u"0"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:rPr/w:bCs"_ostr, 1);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tcPr/w:shd"_ostr, "themeFill"_ostr, u"accent1"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tcPr/w:shd"_ostr, "themeFillShade"_ostr, u"80"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='firstCol']/w:pPr/w:ind"_ostr, "rightChars"_ostr, u"0"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='firstCol']/w:pPr/w:ind"_ostr, "right"_ostr, u"144"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Calendar4']/w:tblStylePr[@w:type='band2Horz']/w:tcPr/w:tcMar/w:bottom"_ostr, "w"_ostr, u"86"_ustr);

    // LightList.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='LightList']/w:tblStylePr[@w:type='firstRow']/w:pPr/w:spacing"_ostr, "before"_ostr, u"0"_ustr);

    // MediumList2-Accent1.
    CPPUNIT_ASSERT(getXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MediumList2-Accent1']/w:tblStylePr[@w:type='band1Vert']/w:tcPr/w:shd"_ostr, "themeFillTint"_ostr).equalsIgnoreAsciiCase("3F"));

    // MediumShading2-Accent5.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MediumShading2-Accent5']/w:tblStylePr[@w:type='firstRow']/w:tcPr/w:tcBorders/w:top"_ostr, "color"_ostr, u"auto"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo71302)
{
    loadAndSave("fdo71302.docx");
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);

    // This got renamed to "Strong Emphasis" without a good reason.
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Strong']"_ostr, 1);
}


DECLARE_OOXMLEXPORT_TEST(testSmartart, "smartart.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGrabBag(0);
    xTextDocumentPropertySet->getPropertyValue(u"InteropGrabBag"_ustr) >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty

    bool bTheme = false;
    for (beans::PropertyValue const& prop : aGrabBag)
    {
      if (prop.Name == "OOXTheme")
      {
        bTheme = true;
        uno::Reference<xml::dom::XDocument> aThemeDom;
        CPPUNIT_ASSERT(prop.Value >>= aThemeDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aThemeDom); // Reference not empty
      }
    }
    CPPUNIT_ASSERT(bTheme); // Grab Bag has all the expected elements

    CPPUNIT_ASSERT_EQUAL(1, getShapes()); // One groupshape in the doc

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xGroup->getCount()); // 1 rendered bitmap from the original shapes

    uno::Reference<beans::XPropertySet> xGroupPropertySet(getShape(1), uno::UNO_QUERY);
    xGroupPropertySet->getPropertyValue(u"InteropGrabBag"_ustr) >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty

    bool bData = false, bLayout = false, bQStyle = false, bColor = false, bDrawing = false;
    for (beans::PropertyValue const& prop : aGrabBag)
    {
      if (prop.Name == "OOXData")
      {
        bData = true;
        uno::Reference<xml::dom::XDocument> aDataDom;
        CPPUNIT_ASSERT(prop.Value >>= aDataDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aDataDom); // Reference not empty
      }
      else if (prop.Name == "OOXLayout")
      {
        bLayout = true;
        uno::Reference<xml::dom::XDocument> aLayoutDom;
        CPPUNIT_ASSERT(prop.Value >>= aLayoutDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aLayoutDom); // Reference not empty
      }
      else if (prop.Name == "OOXStyle")
      {
        bQStyle = true;
        uno::Reference<xml::dom::XDocument> aStyleDom;
        CPPUNIT_ASSERT(prop.Value >>= aStyleDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aStyleDom); // Reference not empty
      }
      else if (prop.Name == "OOXColor")
      {
        bColor = true;
        uno::Reference<xml::dom::XDocument> aColorDom;
        CPPUNIT_ASSERT(prop.Value >>= aColorDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aColorDom); // Reference not empty
      }
      else if (prop.Name == "OOXDrawing")
      {
        bDrawing = true;
        uno::Sequence< uno::Any > diagramDrawing;
        uno::Reference<xml::dom::XDocument> aDrawingDom;
        CPPUNIT_ASSERT(prop.Value >>= diagramDrawing);
        CPPUNIT_ASSERT(diagramDrawing[0] >>= aDrawingDom); // PropertyValue of proper type
        CPPUNIT_ASSERT(aDrawingDom); // Reference not empty
      }
    }
    // Grab Bag has all the expected elements:
    CPPUNIT_ASSERT(bData);
    CPPUNIT_ASSERT(bLayout);
    CPPUNIT_ASSERT(bQStyle);
    CPPUNIT_ASSERT(bColor);
    CPPUNIT_ASSERT(bDrawing);

    uno::Reference<beans::XPropertySet> xPropertySet(xGroup->getByIndex(0), uno::UNO_QUERY);
    OUString nValue;
    xPropertySet->getPropertyValue(u"Name"_ustr) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(u"RenderedShapes"_ustr, nValue); // Rendered bitmap has the proper name
}

CPPUNIT_TEST_FIXTURE(Test, testFontNameIsEmpty)
{
    loadAndSave("font-name-is-empty.docx");
    // Check no empty font name is exported
    // This test does not fail, if the document contains a font with empty name.

    xmlDocUniquePtr pXmlFontTable = parseExport(u"word/fontTable.xml"_ustr);
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlFontTable, "/w:fonts/w:font"_ostr);
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

CPPUNIT_TEST_FIXTURE(Test, testMultiColumnLineSeparator)
{
    loadAndSave("multi-column-line-separator-SAVED.docx");
    // Check for the Column Separator value.It should be FALSE as the document does not contain separator line.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:sectPr/w:cols"_ostr,"sep"_ostr,u"false"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testCustomXmlGrabBag, "customxml.docx")
{
    // The problem was that item[n].xml and itemProps[n].xml and .rels files for item[n].xml
    // files were missing from docx file after saving file.
    // This test case tests whether customxml files grabbagged properly in correct object.

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGrabBag(0);
    xTextDocumentPropertySet->getPropertyValue(u"InteropGrabBag"_ustr) >>= aGrabBag;
    CPPUNIT_ASSERT(aGrabBag.hasElements()); // Grab Bag not empty
    bool CustomXml = false;
    for (beans::PropertyValue const& prop : aGrabBag)
    {
        if (prop.Name == "OOXCustomXml" || prop.Name == "OOXCustomXmlProps")
        {
            CustomXml = true;
            uno::Reference<xml::dom::XDocument> aCustomXmlDom;
            uno::Sequence<uno::Reference<xml::dom::XDocument> > aCustomXmlDomList;
            CPPUNIT_ASSERT(prop.Value >>= aCustomXmlDomList); // PropertyValue of proper type
            sal_Int32 length = aCustomXmlDomList.getLength();
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), length);
            aCustomXmlDom = aCustomXmlDomList[0];
            CPPUNIT_ASSERT(aCustomXmlDom); // Reference not empty
        }
    }
    CPPUNIT_ASSERT(CustomXml); // Grab Bag has all the expected elements
}

CPPUNIT_TEST_FIXTURE(Test, testCustomXmlRelationships)
{
    loadAndSave("customxml.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"customXml/_rels/item1.xml.rels"_ustr);

    // Check there is a relation to itemProps1.xml.
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship"_ostr, 1);
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr, u"itemProps1.xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo69644)
{
    loadAndSave("fdo69644.docx");
    // The problem was that the exporter exported the table definition
    // with only 3 columns, instead of 5 columns.
    // Check that the table grid is exported with 5 columns
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblGrid/w:gridCol"_ostr, 5);
}

DECLARE_OOXMLEXPORT_TEST(testFdo70812, "fdo70812.docx")
{
    // Import just crashed.
    getParagraph(1, u"Sample pages document."_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testPgMargin)
{
    loadAndSave("testPgMargin.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgMar"_ostr, "left"_ostr, u"1440"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testImageCrop, "ImageCrop.docx")
{
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( u"GraphicCrop"_ustr ) >>= aGraphicCropStruct;

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

    imageProperties->getPropertyValue( u"GraphicCrop"_ustr ) >>= aGraphicCropStruct;

    // The crop is constructed in GraphicProperties::pushToPropMap, where
    // GraphicHelper::getOriginalSize tries to get graphic size in mm, then falls back to pixels,
    // which are then converted to mm taking screen DPI scaling into account. Thus, the resulting
    // values are DPI-dependent.
    const double fXScaleFactor = 96.0 / Application::GetDefaultDevice()->GetDPIX();

    CPPUNIT_ASSERT_MESSAGE(
        OString::number(aGraphicCropStruct.Right).getStr(),
        40470 * fXScaleFactor < aGraphicCropStruct.Right);
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
    CPPUNIT_ASSERT_EQUAL(8, getShapes());

    // Angle of frame#1 is 135 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame1(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame1, u"FillStyle"_ustr));
    awt::Gradient aGradient1 = getProperty<awt::Gradient>(xFrame1, u"FillGradient"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(135 * 10), aGradient1.Angle);

    // Angle of frame#2 is 180 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame2, u"FillStyle"_ustr));
    awt::Gradient aGradient2 = getProperty<awt::Gradient>(xFrame2, u"FillGradient"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(180 * 10), aGradient2.Angle);

    // Angle of frame#3 is  90 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame3(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame3, u"FillStyle"_ustr));
    awt::Gradient aGradient3 = getProperty<awt::Gradient>(xFrame3, u"FillGradient"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16( 90 * 10), aGradient3.Angle);

    // Angle of frame#4 is 225 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame4(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame4, u"FillStyle"_ustr));
    awt::Gradient aGradient4 = getProperty<awt::Gradient>(xFrame4, u"FillGradient"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(225 * 10), aGradient4.Angle);

    // Angle of frame#5 is 270 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame5(getShape(5), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame5, u"FillStyle"_ustr));
    awt::Gradient aGradient5 = getProperty<awt::Gradient>(xFrame5, u"FillGradient"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(270 * 10), aGradient5.Angle);

    // Angle of frame#6 is 315 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame6(getShape(6), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame6, u"FillStyle"_ustr));
    awt::Gradient aGradient6 = getProperty<awt::Gradient>(xFrame6, u"FillGradient"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(315 * 10), aGradient6.Angle);

    // Angle of frame#7 is   0 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame7(getShape(7), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame7, u"FillStyle"_ustr));
    awt::Gradient aGradient7 = getProperty<awt::Gradient>(xFrame7, u"FillGradient"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(  0 * 10), aGradient7.Angle);

    // Angle of frame#8 is  45 degrees, but 'aGradient.Angle' holds value in 1/10 of a degree
    uno::Reference<beans::XPropertySet> xFrame8(getShape(8), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, getProperty<drawing::FillStyle>(xFrame8, u"FillStyle"_ustr));
    awt::Gradient aGradient8 = getProperty<awt::Gradient>(xFrame8, u"FillGradient"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16( 45 * 10), aGradient8.Angle);
}

CPPUNIT_TEST_FIXTURE(Test, testCellGridSpan)
{
    loadAndSave("cell-grid-span.docx");
    // The problem was during export gridSpan value for 1st & 2nd cells for test document
    // used to get set wrongly to 5 and 65532 respectively which was the reason for crash during save operation
    // Verifying gridSpan element is not present in RoundTripped Document (As it's Default value is 1).
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:tcPr/w:gridSpan"_ostr,0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:gridSpan"_ostr,0);
}
DECLARE_OOXMLEXPORT_TEST(testFdo71646, "fdo71646.docx")
{
    // The problem was after save file created by MS the direction changed to RTL.
    uno::Reference<uno::XInterface> xParaLTRLeft(getParagraph( 1, u"LTR LEFT"_ustr));
    sal_Int32 nLTRLeft = getProperty< sal_Int32 >( xParaLTRLeft, u"ParaAdjust"_ustr );
    // test the text Direction value for the paragraph
    sal_Int16 nLRDir  = getProperty< sal_Int32 >( xParaLTRLeft, u"WritingMode"_ustr );

    // this will test the both the text direction and alignment for paragraph
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_LEFT), nLTRLeft);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nLRDir);
}

CPPUNIT_TEST_FIXTURE(Test, testParaAutoSpacing)
{
    loadAndSave("para-auto-spacing.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing"_ostr, "beforeAutospacing"_ostr,u"1"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:spacing"_ostr, "afterAutospacing"_ostr,u"1"_ustr);

    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing"_ostr,
                           "beforeAutospacing"_ostr);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing"_ostr,
                           "afterAutospacing"_ostr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing"_ostr, "before"_ostr,u"400"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:spacing"_ostr, "after"_ostr,u"400"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testGIFImageCrop, "test_GIF_ImageCrop.docx")
{
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( u"GraphicCrop"_ustr ) >>= aGraphicCropStruct;

    // The crop is constructed in GraphicProperties::pushToPropMap, where
    // GraphicHelper::getOriginalSize tries to get graphic size in mm, then falls back to pixels,
    // which are then converted to mm taking screen DPI scaling into account. Thus, the resulting
    // values are DPI-dependent.
    const double fXScaleFactor = 96.0 / Application::GetDefaultDevice()->GetDPIX();
    const double fYScaleFactor = 96.0 / Application::GetDefaultDevice()->GetDPIY();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1085 * fXScaleFactor, aGraphicCropStruct.Left, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3651 * fXScaleFactor, aGraphicCropStruct.Right, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(953 * fYScaleFactor, aGraphicCropStruct.Top, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1244 * fYScaleFactor, aGraphicCropStruct.Bottom, 1);
}

DECLARE_OOXMLEXPORT_TEST(testPNGImageCrop, "test_PNG_ImageCrop.docx")
{
    /* The problem was image cropping information was not getting saved
     * after roundtrip.
     * Check for presence of cropping parameters in exported file.
     */
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    css::text::GraphicCrop aGraphicCropStruct;

    imageProperties->getPropertyValue( u"GraphicCrop"_ustr ) >>= aGraphicCropStruct;

    // The crop is constructed in GraphicProperties::pushToPropMap, where
    // GraphicHelper::getOriginalSize tries to get graphic size in mm, then falls back to pixels,
    // which are then converted to mm taking screen DPI scaling into account. Thus, the resulting
    // values are DPI-dependent.
    const double fXScaleFactor = 96.0 / Application::GetDefaultDevice()->GetDPIX();
    const double fYScaleFactor = 96.0 / Application::GetDefaultDevice()->GetDPIY();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1058 * fXScaleFactor, aGraphicCropStruct.Left, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1111 * fXScaleFactor, aGraphicCropStruct.Right, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1164 * fYScaleFactor, aGraphicCropStruct.Top, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(635 * fYScaleFactor, aGraphicCropStruct.Bottom, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf41542_imagePadding)
{
    loadAndReload("tdf41542_imagePadding.odt");
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // borderlessImage - image WITHOUT BORDERS : simulate padding with -crop
    text::GraphicCrop crop = getProperty<text::GraphicCrop>(getShape(2), u"GraphicCrop"_ustr);
    CPPUNIT_ASSERT( crop.Left != 0 );
    CPPUNIT_ASSERT( crop.Right != 0 );
    CPPUNIT_ASSERT_EQUAL( crop.Left, crop.Top);
    CPPUNIT_ASSERT_EQUAL( crop.Right, crop.Bottom);
    CPPUNIT_ASSERT_EQUAL( crop.Left, crop.Right);

    // borderedImage - image WITH BORDERS : simulate padding with -crop
    crop = getProperty<text::GraphicCrop>(getShape(3), u"GraphicCrop"_ustr);
    CPPUNIT_ASSERT( crop.Left != 0 );
    CPPUNIT_ASSERT( crop.Right != 0 );
    CPPUNIT_ASSERT_EQUAL( crop.Left, crop.Top);
    CPPUNIT_ASSERT_EQUAL( crop.Right, crop.Bottom);
    CPPUNIT_ASSERT_EQUAL( crop.Left, crop.Right);
}

CPPUNIT_TEST_FIXTURE(Test, testFootnoteParagraphTag)
{
    loadAndSave("testFootnote.docx");
    /* In footnotes.xml, the paragraph tag inside <w:footnote w:id="2"> was getting written into document.xml.
     * Check for, paragraph tag is correctly written into footnotes.xml.
     */
    xmlDocUniquePtr pXmlFootnotes = parseExport(u"word/footnotes.xml"_ustr);
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]"_ostr,"id"_ostr,u"2"_ustr);
    assertXPath(pXmlFootnotes, "/w:footnotes/w:footnote[3]/w:p/w:r/w:footnoteRef"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testSpacingLineRule)
{
    loadAndReload("table_lineRule.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p/w:pPr/w:spacing"_ostr, "lineRule"_ostr, u"auto"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTableLineSpacing)
{
    loadAndSave("table_atleast.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:pPr/w:spacing"_ostr, "line"_ostr, u"320"_ustr);
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
    CPPUNIT_ASSERT_EQUAL(Color(0x9bbb59), getProperty<Color>(getShape(1), u"FillColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testThemePreservation)
{
    loadAndSave("theme-preservation.docx");
    // check default font theme values have been preserved
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts"_ostr, "asciiTheme"_ostr, u"minorHAnsi"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:docDefaults/w:rPrDefault/w:rPr/w:rFonts"_ostr, "cstheme"_ostr, u"minorBidi"_ustr);

    // check the font theme values in style definitions
    assertXPath(pXmlStyles, "/w:styles/w:style[1]/w:rPr/w:rFonts"_ostr, "eastAsiaTheme"_ostr, u"minorEastAsia"_ustr);

    // check the color theme values in style definitions
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Custom1']/w:rPr/w:color"_ostr, "themeColor"_ostr, u"accent1"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Custom1']/w:rPr/w:color"_ostr, "themeTint"_ostr, u"99"_ustr);

    // check direct format font theme values have been preserved
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:r[1]/w:rPr/w:rFonts"_ostr, "hAnsiTheme"_ostr, u"majorHAnsi"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[5]/w:r[1]/w:rPr/w:rFonts"_ostr, "asciiTheme"_ostr, u"majorHAnsi"_ustr);

    // check theme font color value has been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w:color"_ostr, "themeColor"_ostr, u"accent3"_ustr);
    OUString sThemeShade = getXPath(pXmlDocument, "/w:document/w:body/w:p[4]/w:r[1]/w:rPr/w:color"_ostr, "themeShade"_ostr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xbf), sThemeShade.toInt32(16));

    // check the themeFontLang values in settings file
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:themeFontLang"_ostr, "val"_ostr, u"en-US"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:themeFontLang"_ostr, "eastAsia"_ostr, u"zh-CN"_ustr);
    assertXPath(pXmlSettings, "/w:settings/w:themeFontLang"_ostr, "bidi"_ostr, u"he-IL"_ustr);

    // check fonts have been applied properly
    sal_Unicode fontName[2]; //represents the string "宋体"
    fontName[0] = 0x5b8b;
    fontName[1] = 0x4f53;
    CPPUNIT_ASSERT_EQUAL(OUString(fontName, 2), getProperty<OUString>(getParagraph(1), u"CharFontNameAsian"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr,
                         getProperty<OUString>(getParagraph(2), u"CharFontNameComplex"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Trebuchet MS"_ustr,
                         getProperty<OUString>(getParagraph(3, u"Default style theme font"_ustr), u"CharFontName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Arial Black"_ustr,
                         getProperty<OUString>(getRun(getParagraph(4, u"Direct format font"_ustr), 1), u"CharFontName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Trebuchet MS"_ustr,
                         getProperty<OUString>(getParagraph(5, u"Major theme font"_ustr), u"CharFontName"_ustr));

    // check the paragraph background pattern has been preserved including theme colors
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd"_ostr, "val"_ostr, u"thinHorzStripe"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd"_ostr, "themeFill"_ostr, u"text2"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd"_ostr, "themeFillTint"_ostr, u"33"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd"_ostr, "themeColor"_ostr, u"accent1"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[6]/w:pPr/w:shd"_ostr, "themeShade"_ostr, u"80"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTableThemePreservation)
{
    loadAndSave("table-theme-preservation.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // check cell theme colors have been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:shd"_ostr, "themeFill"_ostr, u"accent6"_ustr);
    assertXPathNoAttribute(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:shd"_ostr,
                           "themeFillShade"_ostr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:tcPr/w:shd"_ostr, "themeFillTint"_ostr, u"33"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:shd"_ostr, "themeFill"_ostr, u"accent6"_ustr);
    assertXPathNoAttribute(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:shd"_ostr,
                           "themeFillShade"_ostr);
    assertXPathNoAttribute(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:tcPr/w:shd"_ostr,
                           "themeFillTint"_ostr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd"_ostr, "themeFill"_ostr, u"accent6"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd"_ostr, "themeFillShade"_ostr, u"80"_ustr);
    assertXPathNoAttribute(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd"_ostr,
                           "themeFillTint"_ostr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd"_ostr, "val"_ostr, u"horzStripe"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd"_ostr, "themeColor"_ostr, u"accent3"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd"_ostr, "themeTint"_ostr, u"33"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[3]/w:tcPr/w:shd"_ostr, "color"_ostr, u"E7EEEE"_ustr);

    // check table style has been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tblPr/w:tblStyle"_ostr, "val"_ostr, u"Sombreadoclaro-nfasis1"_ustr);
    // check table style is not overwritten by other properties
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[2]/w:tcPr/w:tcBorders/*"_ostr, 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[3]/w:tcPr/w:tcBorders/*"_ostr, 0);
    // check that one cell attribute present in the original document has been preserved
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcBorders/*"_ostr, 1);

    // Check that w:cnfStyle row, cell and paragraph property is preserved.
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle"_ostr, "val"_ostr, u"100000000000"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle"_ostr, "firstRow"_ostr, u"1"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle"_ostr, "lastRow"_ostr, u"0"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle"_ostr, "firstColumn"_ostr, u"0"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:trPr/w:cnfStyle"_ostr, "lastColumn"_ostr, u"0"_ustr);

    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle"_ostr, "val"_ostr, u"001000000000"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle"_ostr, "oddVBand"_ostr, u"0"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle"_ostr, "evenVBand"_ostr, u"0"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle"_ostr, "oddHBand"_ostr, u"0"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc/w:tcPr/w:cnfStyle"_ostr, "evenHBand"_ostr, u"0"_ustr);

    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle"_ostr, "val"_ostr, u"100000000000"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle"_ostr, "firstRowFirstColumn"_ostr, u"0"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle"_ostr, "firstRowLastColumn"_ostr, u"0"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle"_ostr, "lastRowFirstColumn"_ostr, u"0"_ustr);
    assertXPath(pXmlDocument, "//w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:cnfStyle"_ostr, "lastRowLastColumn"_ostr, u"0"_ustr);

}

CPPUNIT_TEST_FIXTURE(Test, testcantSplit)
{
    loadAndSave("2_table_doc.docx");
    // if Split table value is true for a table then during export do not write <w:cantSplit w:val="false"/>
    // in table row property,As default row prop is allow row to break across page.
    // writing <w:cantSplit w:val="false"/> during export was causing problem that all the cell data used to come on same page
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr/w:trPr/w:cantSplit"_ostr,0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr/w:trPr/w:cantSplit"_ostr,"val"_ostr,u"true"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDontSplitTable)
{
    loadAndReload("tdf101589_dontSplitTable.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    //single row tables need to prevent split by setting row to no split
    CPPUNIT_ASSERT_EQUAL( u"Row 1"_ustr, parseDump("/root/page[2]/body/tab[1]/row[1]/cell[1]/txt[1]"_ostr) );

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable (xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTable, u"Split"_ustr));

    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTableRows->getByIndex(0), u"IsSplitAllowed"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testExtraSectionBreak, "1_page.docx")
{
    // There was a problem for some documents during export.Invalid sectPr getting added
    // because of faulty calculation of PageDesc value
    // This was the reason for increasing number of pages after RT
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // tdf126544 Styles were being added before their base/parent/inherited-from style existed, and so were using default settings.
    uno::Reference<container::XNameAccess> xParaStyles(getStyles(u"ParagraphStyles"_ustr));
    uno::Reference<style::XStyle> xStyle(xParaStyles->getByName(u"Heading 1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( u"Heading Base"_ustr, xStyle->getParentStyle() );
}

CPPUNIT_TEST_FIXTURE(Test, testcolumnbreak)
{
    loadAndSave("columnbreak.docx");
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(5, u"This is first line after col brk."_ustr), u"BreakType"_ustr));
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[1]/w:br"_ostr, "type"_ostr, u"column"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testGlossary)
{
    loadAndSave("testGlossary.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/glossary/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:glossaryDocument"_ostr, "Ignorable"_ostr, u"w14 wp14"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testGlossaryWithEmail)
{
    // tdf#152289
    loadAndSave("glossaryWithEmail.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/glossary/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDoc, "/rels:Relationships/rels:Relationship[@Id='rId4' "
        "and @Type='http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink' "
        "and @Target='mailto:emailgoeshere@example.com' "
        "and @TargetMode='External']"_ostr);

    // preserve the ShowingPlaceholder setting on both block SDTs.
    pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:showingPlcHdr"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:hyperlink/w:sdt/w:sdtPr/w:showingPlcHdr"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testFdo71785, "fdo71785.docx")
{
    // crashtest
}

CPPUNIT_TEST_FIXTURE(Test, testCrashWhileSave)
{
    loadAndSave("testCrashWhileSave.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/footer1.xml"_ustr);
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:ftr/w:tbl/w:tr/w:tc[1]/w:p[1]/w:pPr/w:pStyle"_ostr, "val"_ostr).match("Normal"));
}

CPPUNIT_TEST_FIXTURE(Test, testFileOpenInputOutputError)
{
    loadAndReload("floatingtbl_with_formula.docx");
    // Docx containing Floating table with formula was giving "General input/output error" while opening in LibreOffice
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pStyle"_ostr, "val"_ostr, u"Normal"_ustr);

    // let's also assert that the formula was exported properly
    assertXPathContent(pXmlDoc, "//w:tbl/w:tr/w:tc[2]/w:p/m:oMathPara/m:oMath/m:sSubSup/m:e/m:r/m:t"_ostr, u"\u03C3"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testSingleCellTableBorders)
{
    loadAndSave("tdf124399_SingleCellTableBorders.docx");
    // tdf#124399: Extra borders on single cell tables fixed.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tcPr/w:tcBorders/w:top    [@w:val = 'nil']"_ostr, 1);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tcPr/w:tcBorders/w:bottom [@w:val = 'nil']"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testInsideBorders)
{
    loadAndSave("tdf129242_InsideBorders.docx");
    // tdf#129242: Don't remove inside borders if the table has more than one cells.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // If this is not 0, then inside borders are removed.
    assertXPathChildren(pXmlDocument, "/w:document/w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:tcBorders"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testRightBorder)
{
    loadAndSave("tdf129442_RightBorder.docx");
    // tdf#129442: Right border of a one column table was missing.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // If the right border is missing like in the bug, then there is a <w:right w:val="nil" /> tag in tcBorders.
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc/w:tcPr/w:tcBorders/w:end [@w:val = 'nil']"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testBottomBorder)
{
    loadAndSave("tdf129450_BottomBorder.docx");
    // tdf#129450: Missing bottom border in one row table.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // If there is no bottom border, it is shown in tcBorders.
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:tcPr/w:tcBorders/w:bottom [@w:val = 'nil']"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testBottomBorders)
{
    loadAndSave("tdf129452_BottomBorders.docx");
    // tdf#129452: Do not omit bottom borders when a column in a table is vertically merged and
    // the inside borders are turned off.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:tcPr/w:tcBorders/w:bottom [@w:val = 'nil']"_ostr, 0);
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[2]/w:tcPr/w:tcBorders/w:bottom [@w:val = 'nil']"_ostr, 0);

    // But also don't treat separately merged cells as one - the topmost merged cell shouldn't gain a border.
    assertXPath(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[4]/w:tcPr/w:tcBorders/w:bottom"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testFontTypes)
{
    loadAndSave("tdf120344_FontTypes.docx");
    // tdf#120344: Font type of numbering symbols can be different than the font type of the text.

    // Check the font type of the text, should be Consolas.
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[1]/w:r/w:rPr/w:rFonts [@w:ascii='Consolas']"_ostr, 1);

    // Now the font type of the numbering symbols, should be Arial Black.
    xmlDocUniquePtr qXmlDocument = parseExport(u"word/numbering.xml"_ustr);
    assertXPath(qXmlDocument, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:rPr/w:rFonts [@w:ascii='Arial Black']"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testNumberingLevels)
{
    loadAndSave("tdf95495.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // tdf#95495: set list level of the custom style based on the setting of the parent style
    // [this assertXPath is not a very good test, since the numbering definition is not set on the paragraph itself,
    //  but in a style. This just tests the current copy-to-paragraph implementation. But leaving it for now,
    //  since this example is very much a corner case, so anyone trespassing here should double-check everything...]
    assertXPath(pXmlDocument, "/w:document/w:body/w:p[2]/w:pPr/w:numPr/w:ilvl [@w:val = '1']"_ostr, 1);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Note: _Toc and _Ref hidden bookmarks are imported from OOXML as normal bookmarks.
    // Without hiding them from visible bookmarks (SwBookmarkPortion), this line would be
    // shown as "A.2.1 [[[[[.DESCRIPTION]]]] with XML layout dump "A.2.1 #_Ref... Bookmark Start..."
    assertXPath(pXmlDoc, "//body/txt[5]/SwParaPortion/SwLineLayout/child::*[1]"_ostr, "expand"_ostr, u"A.2.1 "_ustr);
    assertXPath(pXmlDoc, "//body/txt[5]/SwParaPortion/SwLineLayout/child::*[2]"_ostr, "portion"_ostr, u".DESCRIPTION"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testVerticalBorders)
{
    loadAndSave("calendar3.docx");
    // tdf#130799: Inside vertical borders of a table should not be missing.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    // Left and right borders.
    assertXPathChildren(pXmlDocument, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[3]/w:tcPr/w:tcBorders"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testArrowFlipXY)
{
    loadAndSave("tdf100751_arrowBothFlip.docx");
    // tdf#100751: Both x and y flip should happen.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    OUString arrowStyle = getXPath(pXmlDocument,
                                   "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Fallback/"
                                   "w:pict/v:group/v:shape[@type='_x0000_t32']"_ostr,
                                   "style"_ostr);
    CPPUNIT_ASSERT(arrowStyle.indexOf(u"flip:xy") != sal_Int32(-1));
}

CPPUNIT_TEST_FIXTURE(Test, testArrowPosition)
{
    loadAndSave("tdf104565_ArrowPosition.docx");
    // tdf#104565: Test correct position.
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // This is the correct Y coordinate, the incorrect was 817880.
    assertXPathContent(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/wp:positionV/wp:posOffset"_ostr, u"516255"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testArrowMarker)
{
    loadAndSave("tdf123346_ArrowMarker.docx");
    // tdf#123346: Check arrow marker.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:tailEnd"_ostr, "type"_ostr, u"arrow"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testShapeLineWidth)
{
    loadAndSave("tdf92526_ShapeLineWidth.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#92526: Make sure that line with stays 0.
    xmlDocUniquePtr pXml = parseExport(u"word/document.xml"_ustr);

    // "w" attribute was not exported.
    assertXPath(pXml, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing"
        "/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln"_ostr, "w"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeAnchorWidthFromLeftMargin)
{
    loadAndSave("tdf132976_testRelativeAnchorWidthFromLeftMargin.docx");
    // tdf#132976 The size of the width of this shape should come from the size of the left margin.
    // It was set to the size of the width of the entire page before.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "width"_ostr, u"1133"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeAnchorWidthFromInsideOutsideMargin)
{
    loadAndSave("tdf133861_RelativeAnchorWidthFromInsideOutsideMargin.docx");
    // tdf#133863 tdf#133864 The sizes of the width of these shapes depend on the sizes of the inside and outside margins.
    // The open book: outside --text-- inside | inside --text-- outside
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Inside
    assertXPath(pXmlDoc, "(//anchored/SwAnchoredDrawObject)[1]/bounds"_ostr, "width"_ostr, u"1440"_ustr);
    // Outside
    assertXPath(pXmlDoc, "(//anchored/SwAnchoredDrawObject)[2]/bounds"_ostr, "width"_ostr, u"2552"_ustr);
    // Outside
    assertXPath(pXmlDoc, "(//anchored/SwAnchoredDrawObject)[3]/bounds"_ostr, "width"_ostr, u"2552"_ustr);
    // Inside
    assertXPath(pXmlDoc, "(//anchored/SwAnchoredDrawObject)[4]/bounds"_ostr, "width"_ostr, u"1440"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testBodyPrUpright)
{
    loadAndSave("tdf123610_handle_upright.docx");
    // tdf#123610: Check grab-bag attribute upright to keep text upright regardless of shape rotation.

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wps:wsp/wps:bodyPr"_ostr, "upright"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testLostArrow)
{
    loadAndReload("tdf99810-lost-arrow.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#99810: check whether we use normal shape instead of connector shape if the XML namespace
    // is wps, because wps:
    xmlDocUniquePtr pDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor"
        "/a:graphic/a:graphicData/wps:wsp"_ostr);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
