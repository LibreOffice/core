/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <svx/svddef.hxx>
#include <svx/unoapi.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svdobj.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <comphelper/configuration.hxx>
#include <editeng/escapementitem.hxx>
#include <unotools/fltrcfg.hxx>
#include <textboxhelper.hxx>
#include <unoprnms.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf138892_noNumbering, "tdf138892_noNumbering.docx")
{
    CPPUNIT_ASSERT_MESSAGE("Para1: Bullet point", !getProperty<OUString>(getParagraph(1), "NumberingStyleName").isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Para2: <blank line>", getProperty<OUString>(getParagraph(2), "NumberingStyleName").isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Para3: <blank line>", getProperty<OUString>(getParagraph(3), "NumberingStyleName").isEmpty());
}

<<<<<<< HEAD   (7b6bf0 tdf#142128 sw: set author-color strikethrough for AS_CHAR im)
=======
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

CPPUNIT_TEST_FIXTURE(Test, testGutterTop)
{
    load(mpTestDocumentPath, "gutter-top.docx");
    save("Office Open XML Text", maTempFile);
    mbExported = true;
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

>>>>>>> CHANGE (88e6a1 tdf#69635 DOCX: export hidden (shared) headers/footers)
DECLARE_OOXMLEXPORT_TEST(testTdf140668, "tdf140668.docx")
{
    // Don't crash when document is opened
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf140572_docDefault_superscript, "tdf140572_docDefault_superscript.docx")
{
    // A round-trip was crashing.

    // Without the fix, everything was DFLT_ESC_AUTO_SUPER (default superscript)
    CPPUNIT_ASSERT_EQUAL( sal_Int16(0), getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharEscapement") );
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

DECLARE_OOXMLEXPORT_TEST(testTdf140137, "tdf140137.docx")
{
    // Don't throw exception during load
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
