/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/ShadingPattern.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

#include <comphelper/propertyvalue.hxx>
#include <docmodel/color/ComplexColor.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <cppuhelper/implbase.hxx>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <wrtsh.hxx>

using namespace css;
using namespace ::cpo;
using namespace ::cpo::uno;

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

class ProgressRecorder : public cppu::WeakImplHelper<css::task::XStatusIndicator>
{
public:
    sal_Int32 mnStartCount = 0;
    sal_Int32 mnEndCount = 0;
    sal_Int32 mnLastValue = -1;
    OUString maText;

    void SAL_CALL start(const OUString& rText, sal_Int32 /*nRange*/) override
    {
        ++mnStartCount;
        maText = rText;
    }
    void SAL_CALL end() override { ++mnEndCount; }
    void SAL_CALL setText(const OUString& rText) override { maText = rText; }
    void SAL_CALL setValue(sal_Int32 nValue) override { mnLastValue = nValue; }
    void SAL_CALL reset() override {}
};

DECLARE_OOXMLEXPORT_TEST(testTdf38575_fullWidthLine, "tdf38575_fullWidthLine.docx")
{
    // given a document where the header contains a graphic that spans the entire left side,
    // reducing the available body text area for every page in the document,
    // and an AS_CHAR horizontal line that wants the entire width (and thus could never fit)

    // for compatibilityMode 15, in MS Word 2024, this is 4 pages long (not infinitely long)
    CPPUNIT_ASSERT_EQUAL(4, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124398_groupshapeChart)
{
    // given a document with grouped chart and textbox
    // TODO: seeing the chart is broken since 24.8.4

    createSwDoc("tdf124398_groupshapeChart.docx");

    // MS Word considered this document to be corrupt for several reasons.
    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // DOCX/wps: doesn't use nvGraphicFramePr - only PPTX/p: and XLSX/xdr: do.
    assertXPath(pXmlDoc, "//wpg:graphicFrame/wpg:cNvPr", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf171527_flyInFramePr)
{
    // given a with a framePr'd image anchoring a drawing shape
    createSwDoc("tdf171527_flyInFramePr.doc");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the fix, MS Word complained the file was corrupt.
    // Round-trip the framePr'd image as a framePr instead of as a drawingML
    assertXPath(pXmlDoc, "//w:body/w:p/w:pPr/w:framePr", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf171433_equation)
{
    // given a document with formula
    createSwDoc("tdf171433_equation.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the fix, the formula was completely broken - missing SubSup entirely
    assertXPath(pXmlDoc, "//m:sSubSup/m:sup", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtPictureDataBinding)
{
    // Given a DOCX with a picture content control (w:picture + w:dataBinding)
    // where the data binding resolves to base64 image data from custom XML.
    // The sdtContent has a w:drawing with the actual rendered image.
    // Without the fix, the drawing was replaced by the raw base64 text.
    createSwDoc("sdt-picture-databinding.docx");

    // The first paragraph should contain a graphic, not text
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(Test, testCool15788_symbolContentControl)
{
    createSwDoc("Cool15788_symbolContentControl.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // this must not exist, otherwise Word complains about the file
    // <w:sym w:font="Wingdings" w:char="f04b"/>
    CPPUNIT_ASSERT_EQUAL(0, countXPathNodes(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:sym"));
    // simply a <w:t> element with the symbol must exist instead
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:t", u"\xf04b");
}

CPPUNIT_TEST_FIXTURE(Test, testStarBatsBulletKeepsAVisibleGlyph)
{
    createSwDoc("starbats-bullet.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    // The bullet comes out in a font that is widely available, at the character that font holds
    // the bullet at. A reader without the old StarBats font still gets a bullet rather than an
    // empty rectangle.
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:rPr/w:rFonts", "ascii",
                u"Symbol");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:lvlText", "val", u"\xF0B7");
}

CPPUNIT_TEST_FIXTURE(Test, testDocxSaveReportsProgress)
{
    createSwDoc();

    rtl::Reference<ProgressRecorder> xRecorder(new ProgressRecorder);
    cpo::uno::Sequence<beans::PropertyValue> aStoreArguments{
        comphelper::makePropertyValue(u"FilterName"_ustr, u"MS Word 2007 XML"_ustr),
        comphelper::makePropertyValue(
            u"StatusIndicator"_ustr, uno::Reference<task::XStatusIndicator>(xRecorder))
    };

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    xStorable->storeToURL(maTempFile.GetURL(), aStoreArguments);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xRecorder->mnStartCount);
    CPPUNIT_ASSERT(xRecorder->mnLastValue >= 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xRecorder->mnEndCount);
}

CPPUNIT_TEST_FIXTURE(Test, testCharStyleShadingPattern)
{
    // A character style whose shading is a 20 percent pattern.
    createSwDoc("special_styles.docx");

    // Without the fix a named character style had nowhere to keep the pattern, so the style
    // held only the color that the pattern and the fill blend into.
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"CharacterStyles"_ustr)->getByName(u"Message Header Char"_ustr),
        uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(drawing::ShadingPattern::PCT20),
                         getProperty<sal_Int32>(xStyle, u"CharShadingValue"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testShadingPatternKeepsFillColor)
{
    createSwDoc("char-style-shading-pattern.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    // Without the fix a 15 percent shading was always written over white, so the yellow of this
    // style turned grey.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Shaded']/w:rPr/w:shd", "fill",
                u"FFFF00");

    // This one blends red over white, which no 15 percent shading over black reaches, so it
    // keeps the color it shows rather than a shading that would paint something else.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='ShadedRed']/w:rPr/w:shd", "val",
                u"clear");
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='ShadedRed']/w:rPr/w:shd", "fill",
                u"FFD8D8");
}

CPPUNIT_TEST_FIXTURE(Test, testShadingPatternsOtherThanPct15)
{
    createSwDoc("char-shading-patterns.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    // Without the fix only a 15 percent shading was written back as a shading, and every other
    // one came out as a plain fill of the color it blends into.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Pct35Yellow']/w:rPr/w:shd", "val",
                u"pct35");
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Pct35Yellow']/w:rPr/w:shd", "fill",
                u"FFFF00");

    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='DiagStripe']/w:rPr/w:shd", "val",
                u"diagStripe");
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='DiagStripe']/w:rPr/w:shd", "fill",
                u"00FF00");

    // A solid shading paints its own color over the whole fill, so that color is the one the
    // style shows.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='SolidRed']/w:rPr/w:shd", "val", u"solid");
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='SolidRed']/w:rPr/w:shd", "color",
                u"FF0000");
}

CPPUNIT_TEST_FIXTURE(Test, testCharShadingRoundTripOnSavedFile)
{
    // A run shaded with a 15 percent pattern, in a file that was saved out in this format
    // rather than built by hand for the test.
    createSwDoc("fdo65400.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // A 15 percent pattern over white blends to the grey the brush item holds, and the fill
    // worked back out of that grey is white again, so the run goes out as it came in.
    assertXPath(pXmlDoc, "//w:r/w:rPr/w:shd[@w:val='pct15']", 1);
    assertXPath(pXmlDoc, "//w:r/w:rPr/w:shd[@w:val='pct15']", "color", u"auto");
    assertXPath(pXmlDoc, "//w:r/w:rPr/w:shd[@w:val='pct15']", "fill", u"FFFFFF");
}

CPPUNIT_TEST_FIXTURE(Test, testCharShadingThemeFill)
{
    createSwDoc("char-shading-theme-fill.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the fix the theme color of a character shading was dropped and only the color it
    // resolves to came back, so the shading no longer followed the document theme.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/w:rPr/w:shd", "themeFill", u"accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:rPr/w:shd", "themeFill", u"accent1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:rPr/w:shd", "themeFillTint", u"33");
}

CPPUNIT_TEST_FIXTURE(Test, testCharShadingThemeFillOnlyWithItsOwnFill)
{
    createSwDoc();

    // A run shaded with a 15 percent pattern over a color that no fill blends into, and a
    // theme color on top of that. The document is built here because no file in the corpus
    // carries a character shading of that shape.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY_THROW);
    xTextDocument->getText()->setString(u"shaded"_ustr);
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY_THROW);
    xRun->setPropertyValue(u"CharBackColor"_ustr, cpo::uno::Any(sal_Int32(0xffd8d8)));
    xRun->setPropertyValue(u"CharShadingValue"_ustr,
                           cpo::uno::Any(sal_Int32(drawing::ShadingPattern::PCT15)));
    model::ComplexColor aComplexColor;
    aComplexColor.setThemeColor(model::ThemeColorType::Accent1);
    xRun->setPropertyValue(u"CharBackgroundComplexColor"_ustr,
                           cpo::uno::Any(model::color::createXComplexColor(aComplexColor)));

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // The color written is the one the pattern and the fill blend into, which the theme color
    // does not name, so the run goes out with that color alone.
    assertXPath(pXmlDoc, "//w:r/w:rPr/w:shd", "val", u"clear");
    assertXPath(pXmlDoc, "//w:r/w:rPr/w:shd", "fill", u"FFD8D8");
    assertXPathNoAttribute(pXmlDoc, "//w:r/w:rPr/w:shd", "themeFill");
}

CPPUNIT_TEST_FIXTURE(Test, testCellBorderThemeColor)
{
    // A cell whose borders name a theme color, one of them with a tint, one with a shade, and
    // one border with a plain color.
    createSwDoc("cell-border-theme-color.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the fix the theme color was read but never reached the cell, so only the color it
    // resolves to came back and the borders stopped following the document theme.
    assertXPath(pXmlDoc, "//w:tcBorders/w:top", "themeColor", u"accent1");
    assertXPath(pXmlDoc, "//w:tcBorders/w:top", "themeTint", u"33");
    assertXPath(pXmlDoc, "//w:tcBorders/w:start", "themeColor", u"accent2");
    assertXPath(pXmlDoc, "//w:tcBorders/w:start", "themeShade", u"80");
    assertXPath(pXmlDoc, "//w:tcBorders/w:end", "themeColor", u"accent3");

    // The border that names no theme color keeps only its own color.
    assertXPathNoAttribute(pXmlDoc, "//w:tcBorders/w:bottom", "themeColor");
}

CPPUNIT_TEST_FIXTURE(Test, testHyperlinkKeepsNoEmptyStyledSpan)
{
    // Hyperlink fields whose text already carries the hyperlink character style.
    createSwDoc("fdo76597.docx");

    save(TestFilter::ODT);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Resetting the character style of such a run was refused, because it was asked for by
    // the name an English user interface shows rather than the one the API knows, so every
    // hyperlink left an empty styled span behind.
    // Without the fix there were 42 of them, one per hyperlink.
    assertXPath(pXmlDoc, "//text:span[@text:style-name='Internet_20_link']", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTableAlignmentFromTableStyle)
{
    // The first table takes its alignment from the w:jc of its table style, the second one
    // overrides the w:jc of its style with a direct one.
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();

        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 2
        // - Actual  : 7
        // i.e. the table was left aligned, the w:jc of the table style having been dropped.
        CPPUNIT_ASSERT_EQUAL(
            text::HoriOrientation::CENTER,
            getProperty<sal_Int16>(xTables->getByName(u"Table1"_ustr), u"HoriOrient"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            text::HoriOrientation::LEFT_AND_WIDTH,
            getProperty<sal_Int16>(xTables->getByName(u"Table2"_ustr), u"HoriOrient"_ustr));
    };

    createSwDoc("table-style-jc.docx");
    verify();
    saveAndReload(TestFilter::DOCX);
    verify();
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
