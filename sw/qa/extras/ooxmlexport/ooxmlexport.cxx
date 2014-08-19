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
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
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
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/Hatch.hpp>

#include <string>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const SAL_OVERRIDE {
        const char* aBlacklist[] = {
            "math-escape.docx",
            "math-mso2k7.docx",
            "ImageCrop.docx",
            "test_GIF_ImageCrop.docx",
            "test_PNG_ImageCrop.docx"
        };
        std::vector<const char*> vBlacklist(aBlacklist, aBlacklist + SAL_N_ELEMENTS(aBlacklist));

        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx") && std::find(vBlacklist.begin(), vBlacklist.end(), filename) == vBlacklist.end());
    }
};

DECLARE_OOXMLEXPORT_TEST(testfdo81381, "fdo81381.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:object[1]/o:OLEObject[1]", "DrawAspect", "Icon");
}

DECLARE_OOXMLEXPORT_TEST(testSdtAlias, "sdt-alias.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    // <w:alias> was completely missing.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:alias", "val", "Subtitle");
}

DECLARE_OOXMLEXPORT_TEST(testSdtDateCharformat, "sdt-date-charformat.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
    {
        // character formatting (bold) was missing, this was 0
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:rPr/w:b", 1);
        // alias was also missing.
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:alias", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testFooterBodyDistance, "footer-body-distance.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        // Page break was exported as section break, this was 0
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br", 1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo81031, "fdo81031.docx")
{
    // vml image was not rendered
    // As there are also numPicBullets in the file,
    // the fragmentPath was not changed hence relationships were not resolved.

    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> xImage(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, "Graphic");
    uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(381), xBitmap->getSize().Width );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(148), xBitmap->getSize().Height );
}

DECLARE_OOXMLEXPORT_TEST(testPlausableBorder, "plausable-border.docx")
{
    // sw::util::IsPlausableSingleWordSection() did not merge two page styles due to borders.
    if (xmlDocPtr pXmlDoc = parseExport())
        // Page break was exported as section break, this was 0
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br", 1);
}

DECLARE_OOXMLEXPORT_TEST(testUnwantedSectionBreak, "unwanted-section-break.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        // This was 2: an additional sectPr was added to the document.
        assertXPath(pXmlDoc, "//w:sectPr", 1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo80897 , "fdo80897.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/wps:bodyPr/a:prstTxWarp", "prst", "textTriangle");
}



DECLARE_OOXMLEXPORT_TEST(testFdo80997, "fdo80997.docx")
{
    // The problem was that the DOCX exporter not able to export text behind textbox, if textbox has a wrap property.
    uno::Reference< text::XTextRange > xParagraph = getParagraph( 1 );
    uno::Reference< text::XTextRange > xText = getRun( xParagraph, 1, " text");
}

DECLARE_OOXMLEXPORT_TEST(testFdo80902, "fdo80902.docx")
{
    // The problem was that the docGrid type was set as default so fix it for other grid type
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:docGrid", "type", "lines");
}

DECLARE_OOXMLEXPORT_TEST(testParaShading, "para-shading.docx")
{
    // Make sure the themeColor attribute is not written when it would be empty.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        xmlXPathObjectPtr pXPath = getXPathNode(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:shd");
        xmlNodeSetPtr pXmlNodes = pXPath->nodesetval;
        CPPUNIT_ASSERT_EQUAL(1, xmlXPathNodeSetGetLength(pXmlNodes));
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
        // The attribute existed, so xmlGetProp() returned non-NULL.
        CPPUNIT_ASSERT_EQUAL(static_cast<xmlChar*>(0), xmlGetProp(pXmlNode, BAD_CAST("themeColor")));
    }
}

DECLARE_OOXMLEXPORT_TEST(testFirstHeaderFooter, "first-header-footer.docx")
{
    // Test import and export of a section's headerf/footerf properties.
    // (copied from a ww8export test, with doc converted to docx using Word)

    // The document has 6 pages. Note that we don't test if 4 or just 2 page
    // styles are created, the point is that layout should be correct.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer"),  parseDump("/root/page[1]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header"),   parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer"),   parseDump("/root/page[2]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header"),  parseDump("/root/page[3]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer"),  parseDump("/root/page[3]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page header2"), parseDump("/root/page[4]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer 2"), parseDump("/root/page[4]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header 2"), parseDump("/root/page[5]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer 2"), parseDump("/root/page[5]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header 2"),  parseDump("/root/page[6]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer 2"),  parseDump("/root/page[6]/footer/txt/text()"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
