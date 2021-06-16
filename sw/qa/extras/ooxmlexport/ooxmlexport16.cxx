/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <swmodeltestbase.hxx>

#include <svx/svddef.hxx>
#include <svx/unoapi.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svdobj.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <comphelper/configuration.hxx>
#include <comphelper/propertysequence.hxx>
#include <editeng/escapementitem.hxx>
#include <unotools/fltrcfg.hxx>
#include <textboxhelper.hxx>
#include <unoprnms.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        if (filename == std::string_view("tdf135774_numberingShading.docx"))
        {
            bool bIsExportAsShading = SvtFilterOptions::Get().IsCharBackground2Shading();
            // This function is run at the end of the test - returning the filter options to normal.
            std::unique_ptr<Resetter> pResetter(new Resetter(
                [bIsExportAsShading] () {
                    if (bIsExportAsShading)
                        SvtFilterOptions::Get().SetCharBackground2Shading();
                }));
            // For these tests, ensure exporting CharBackground as w:highlight.
            SvtFilterOptions::Get().SetCharBackground2Highlighting();
            return pResetter;
        }
        return nullptr;
    }

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf136059, "tdf136059.odt")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Contour has not been exported!", true,
        getProperty<bool>(getShape(1), "SurroundContour"));
    // With the fix this shall pass, see tdf136059.
}

DECLARE_OOXMLEXPORT_TEST(testTdf138892_noNumbering, "tdf138892_noNumbering.docx")
{
    CPPUNIT_ASSERT_MESSAGE("Para1: Bullet point", !getProperty<OUString>(getParagraph(1), "NumberingStyleName").isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Para2: <blank line>", getProperty<OUString>(getParagraph(2), "NumberingStyleName").isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Para3: <blank line>", getProperty<OUString>(getParagraph(3), "NumberingStyleName").isEmpty());
}

DECLARE_OOXMLEXPORT_TEST(testTdf141231_arabicHebrewNumbering, "tdf141231_arabicHebrewNumbering.docx")
{
    // The page's numbering type: instead of Hebrew, this was default style::NumberingType::ARABIC (4).
    auto nActual = getProperty<sal_Int16>(getStyles("PageStyles")->getByName("Standard"), "NumberingType");
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_HEBREW, nActual);

    // The footnote numbering type: instead of arabicAbjad, this was the default style::NumberingType::ARABIC.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings = xFootnotesSupplier->getFootnoteSettings();
    nActual = getProperty<sal_Int16>(xFootnotesSupplier->getFootnoteSettings(), "NumberingType");
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::CHARS_ARABIC_ABJAD, nActual);
}


DECLARE_OOXMLEXPORT_TEST(testTdf132752, "tdf132752.docx")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1801), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(getParagraph(1), "ParaRightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
}

DECLARE_OOXMLEXPORT_TEST(testGutterLeft, "gutter-left.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle;
    getStyles("PageStyles")->getByName("Standard") >>= xPageStyle;
    sal_Int32 nGutterMargin{};
    xPageStyle->getPropertyValue("GutterMargin") >>= nGutterMargin;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1270
    // - Actual  : 0
    // i.e. gutter margin was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), nGutterMargin);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testGutterTop, "gutter-top.docx")
{
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlSettings);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. <w:gutterAtTop> was lost.
    assertXPath(pXmlSettings, "/w:settings/w:gutterAtTop", 1);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf69635, "tdf69635.docx")
{
    xmlDocUniquePtr pXmlHeader1 = parseExport("word/header1.xml");
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlHeader1);
    CPPUNIT_ASSERT(pXmlSettings);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: "left"
    // - Actual  : "right"
    assertXPathContent(pXmlHeader1, "/w:hdr/w:p/w:r/w:t", "left");

    // Make sure "left" appears as a hidden header
    assertXPath(pXmlSettings, "/w:settings/w:evenAndOddHeaders", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf140668, "tdf140668.docx")
{
    // Don't crash when document is opened
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf138771, "tdf138771.docx")
{
    // Don't crash when document is imported
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf125936_numberingSuperscript, "tdf125936_numberingSuperscript.docx")
{
    // Without the fix, the first character run was superscripted.
    CPPUNIT_ASSERT_EQUAL( sal_Int16(0), getProperty<sal_Int16>(getRun(getParagraph(1), 1, "A-570-108"), "CharEscapement") );
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf134619_numberingProps, "tdf134619_numberingProps.doc")
{
    // Get the third paragraph's numbering style's 1st level's bullet size
    uno::Reference<text::XTextRange> xParagraph = getParagraph(3);
    auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(0) >>= aLevel; // 1st level
    OUString aCharStyleName = std::find_if(aLevel.begin(), aLevel.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "CharStyleName"; })->Value.get<OUString>();

    // Make sure that the blue bullet's font size is 72 points, not 12 points.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName(aCharStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(72.f, getProperty<float>(xStyle, "CharHeight"));
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf134951_duplicates, "tdf134951_duplicates.odt")
{
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xEndnotesSupplier->getEndnotes()->getCount());

    getParagraph(5, "Duplicate fields: 1");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf135773_numberingShading, "tdf135774_numberingShading.docx")
{
    // This test uses preTest to export CharBackground as Highlight instead of the 7.0 default of Shading.

    // Before the fix, the imported shading was converted into a red highlight.
    xmlDocUniquePtr pXmlStyles = parseExport("word/numbering.xml");
    assertXPath(pXmlStyles, "/w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:rPr/w:shd", "fill", "ED4C05");
}

DECLARE_OOXMLEXPORT_TEST(testTdf140336_paraNoneShading, "tdf140336_paraNoneShading.odt")
{
    // Before the fix, the background from a style was exported to dis-inheriting paragraphs/styles.
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(COL_AUTO), getProperty<sal_uInt32>(getParagraph(1), "ParaBackColor"));
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("CanclledBackground"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xStyle, "FillStyle"));

    // sanity check: backgroundColor paragraph style has a golden color(FF7F50), which para2 inherits
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(16744272), getProperty<sal_uInt32>(getParagraph(2), "ParaBackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf141173_missingFrames, "tdf141173_missingFrames.rtf")
{
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf142404_tabOverSpacingC15, "tdf142404_tabOverSpacingC15.odt")
{
    // Although TabOverMargin no longer applies to compatibilityMode 15 DOCX files,
    // it still applies to a tab over the paragraph end (inside text boundaries).
    // The original 3-page ODT saved as DOCX would fit on one page in MS Word 2010, but 3 in Word 2013.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("too big for two pages", 3, getPages());
    // The tab goes over the paragraph margin
    CPPUNIT_ASSERT_EQUAL(OUString("A left tab positioned at"), parseDump("//page[1]/body/txt[2]/Text[1]", "Portion"));
    sal_Int32 nTextLen = parseDump("//page[1]/body/txt[2]/Text[1]", "nWidth").toInt32();
    CPPUNIT_ASSERT_EQUAL(OUString("*"), parseDump("//page[1]/body/txt[2]/Text[2]", "Portion"));
    sal_Int32 nTabLen = parseDump("//page[1]/body/txt[2]/Text[2]", "nWidth").toInt32();
    CPPUNIT_ASSERT_MESSAGE("Large left tab", nTextLen < nTabLen);
    // Not 1 line high (Word 2010 DOCX), or 3 lines high (LO DOCX) or 5 lines high (ODT), but 4 lines high
    sal_Int32 nHeight = parseDump("//page[1]/body/txt[2]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_MESSAGE("4 lines high", 1100 < nHeight);
    CPPUNIT_ASSERT_MESSAGE("4 lines high", nHeight < 1300);

    CPPUNIT_ASSERT_EQUAL(OUString("TabOverflow does what?"), parseDump("//page[1]/body/txt[7]/Text[1]", "Portion"));
    // Not 1 line high (Word 2010 DOCX), or 4 lines high (prev LO DOCX) or 8 lines high (ODT).
    // but two lines high. (3 in Word 2016 because it pulls down "what?" to the second line - weird)
    nHeight = parseDump("//page[1]/body/txt[7]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("2 lines high (but 3 in Word)", 242*2.5, nHeight, 242);

    // Pages 2/3 are TabOverMargin - in this particular case tabs should not go over margin.
    CPPUNIT_ASSERT_EQUAL(OUString("TabOverflow does what?"), parseDump("//page[3]/body/txt[2]/Text[1]", "Portion"));
    // Not 1 line high (Word 2010 DOCX and ODT), or 4 lines high (prev LO DOCX),
    // but 8 lines high.
    nHeight = parseDump("//page[3]/body/txt[2]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("8 lines high", 242*8, nHeight, 121);
}

DECLARE_OOXMLEXPORT_TEST(testTdf139580, "tdf139580.odt")
{
    // Without the fix in place, this test would have crashed at export time
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testFooterMarginLost, "footer-margin-lost.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 709
    // - Actual  : 0
    // i.e. import + export lost the footer margin value.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgMar", "footer", "709");
}

CPPUNIT_TEST_FIXTURE(Test, testEffectExtentLineWidth)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(508),
                             getProperty<sal_Int32>(getShape(1), "TopMargin"));
    };

    // Given a document with a shape that has a non-zero line width and effect extent:
    // When loading the document:
    load(mpTestDocumentPath, "effect-extent-line-width.docx");
    // Then make sure that the line width is not taken twice (once as part of the margin, and then
    // also as the line width):
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 508
    // - Actual  : 561
    // i.e. the upper spacing was too large, the last line of the text moved below the shape.
    verify();
    reload(mpFilter, "effect-extent-line-width.docx");
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testRtlGutter)
{
    // Given a document with RTL gutter:
    load(mpTestDocumentPath, "rtl-gutter.docx");
    uno::Reference<beans::XPropertySet> xStandard(getStyles("PageStyles")->getByName("Standard"),
            uno::UNO_QUERY);
    CPPUNIT_ASSERT(getProperty<bool>(xStandard, "RtlGutter"));

    // When saving back to DOCX:
    reload(mpFilter, "rtl-gutter.docx");

    // Then make sure the section's gutter is still RTL:
    xmlDocUniquePtr pXmlDoc = parseExport();
    // Without the accompanying fix in place, this test would have failed as the XML element was
    // missing.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:rtlGutter", 1);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf140572_docDefault_superscript, "tdf140572_docDefault_superscript.docx")
{
    // A round-trip was crashing.

    // Without the fix, everything was DFLT_ESC_AUTO_SUPER (default superscript)
    CPPUNIT_ASSERT_EQUAL( sal_Int16(0), getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharEscapement") );
}

DECLARE_OOXMLEXPORT_TEST(testTdf136841, "tdf136841.docx")
{
    if (!IsDefaultDPI())
        return;
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> graphic;
    imageProperties->getPropertyValue( "Graphic" ) >>= graphic;
    Graphic vclGraphic(graphic);
    BitmapEx bitmap(vclGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL( tools::Long(76), bitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL( tools::Long(76), bitmap.GetSizePixel().Height());

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:228 G:71 B:69 A:0
    // - Actual  : Color: R:0 G:0 B:0 A:0
    CPPUNIT_ASSERT_EQUAL( Color(228,71,69), bitmap.GetPixelColor(38,38));
}

DECLARE_OOXMLEXPORT_TEST(testTdf138953, "croppedAndRotated.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_EQUAL(27000.0, getProperty<double>(xShape, "RotateAngle"));
    auto frameRect = getProperty<css::awt::Rectangle>(xShape, "FrameRect");
    // Before the fix, original object size (i.e., before cropping) was written to spPr in OOXML,
    // and the resulting object size was much larger than should be.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12961), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8664), frameRect.Width);
}

DECLARE_OOXMLEXPORT_TEST(testTdf118535, "tdf118535.odt")
{
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(true, bool(xNameAccess->hasByName("word/media/image1.jpeg")));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: false
    // - Actual  : true
    // i.e. the embedded picture would have been saved twice.
    CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName("word/media/image2.jpeg")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf133473_shadowSize, "tdf133473.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);

    SdrObject* pObj(GetSdrObjectFromXShape(xShape));
    const SfxItemSet& rSet = pObj->GetMergedItemSet();
    sal_Int32 nSize1 = rSet.Get(SDRATTR_SHADOWSIZEX).GetValue();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 200000
    // - Actual  : 113386
    // I.e. Shadow size will be smaller than actual.

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(200000), nSize1);
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
        ShapeZorders.push_back(xShapeProperties->getPropertyValue("ZOrder").get<sal_uInt64>());
        FrameZorders.push_back(xFrameProperties->getPropertyValue("ZOrder").get<sal_uInt64>());
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testCommentDone, "CommentDone.docx")
{
    xmlDocUniquePtr pXmlComm = parseExport("word/comments.xml");
    assertXPath(pXmlComm, "/w:comments/w:comment[1]/w:p", 2);
    OUString idLastPara = getXPath(pXmlComm, "/w:comments/w:comment[1]/w:p[2]", "paraId");
    xmlDocUniquePtr pXmlCommExt = parseExport("word/commentsExtended.xml");
    assertXPath(pXmlCommExt, "/w15:commentsEx", "Ignorable", "w15");
    assertXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx", 1);
    OUString idLastParaEx = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx", "paraId");
    CPPUNIT_ASSERT_EQUAL(idLastPara, idLastParaEx);
    assertXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx", "done", "1");
}

DECLARE_OOXMLEXPORT_TEST(testTableWidth, "frame_size_export.docx")
{
    // after exporting: table width was overwritten in the doc model
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<sal_Int16>(xTables->getByIndex(0), "RelativeWidth"));
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

    if (!mbExported)
    {
        // Check that it's resolved when initially read
        CPPUNIT_ASSERT_EQUAL(true, xComment->getPropertyValue("Resolved").get<bool>());
        // Set to unresolved
        xComment->setPropertyValue("Resolved", css::uno::Any(false));
    }
    else
    {
        // After the roundtrip, it keeps the "unresolved" state set above
        CPPUNIT_ASSERT_EQUAL(false, xComment->getPropertyValue("Resolved").get<bool>());
    }

    // Second comment: initially unresolved, toggled to resolved on import, resolved on roundtrip
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    aComment = xFields->nextElement();
    xComment.set(aComment, css::uno::UNO_QUERY_THROW);

    if (!mbExported)
    {
        // Check that it's unresolved when initially read
        CPPUNIT_ASSERT_EQUAL(false, xComment->getPropertyValue("Resolved").get<bool>());
        // Set to resolved
        xComment->setPropertyValue("Resolved", css::uno::Any(true));
    }
    else
    {
        // After the roundtrip, it keeps the "resolved" state set above
        CPPUNIT_ASSERT_EQUAL(true, xComment->getPropertyValue("Resolved").get<bool>());
    }
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(Test_ShadowDirection, "tdf142361ShadowDirection.odt")
{
    // The attribute 'rotWithShape' has the default value 'true' in OOXML, so Words interprets a
    // missing attribute as 'true'. That means that Word rotates the shadow if the shape is
    // rotated. Because in LibreOffice a shadow is never rotated, we must not omit this
    // attribute.
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[1]/w:r[1]/mc:AlternateContent/mc:Choice/w:drawing/"
                "wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:effectLst/a:outerShdw",
                "rotWithShape", "0");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
