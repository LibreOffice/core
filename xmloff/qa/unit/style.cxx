/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/color.hxx>
#include <sal/config.h>

#include <test/unoapixml_test.hxx>

#include <com/sun/star/awt/ColorStop.hpp>
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

#include <rtl/character.hxx>
#include <unotools/saveopt.hxx>

using namespace ::com::sun::star;

/// Covers xmloff/source/style/ fixes.
class XmloffStyleTest : public UnoApiXmlTest
{
public:
    XmloffStyleTest();
    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex);
};

XmloffStyleTest::XmloffStyleTest()
    : UnoApiXmlTest(u"/xmloff/qa/unit/data/"_ustr)
{
}

uno::Reference<drawing::XShape> XmloffStyleTest::getShape(sal_uInt8 nShapeIndex)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(nShapeIndex),
                                           uno::UNO_QUERY_THROW);
    return xShape;
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFillImageBase64)
{
    // Load a flat ODG that has base64-encoded bitmap as a fill style.
    loadFromFile(u"fill-image-base64.fodg");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xBitmaps(
        xFactory->createInstance(u"com.sun.star.drawing.BitmapTable"_ustr), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed, as the base64 stream was
    // not considered when parsing the fill-image style.
    CPPUNIT_ASSERT(xBitmaps->hasByName(u"libreoffice_0"_ustr));
}

namespace
{
struct XmlFont
{
    OString aName;
    OString aFontFamilyGeneric;
    bool operator<(const XmlFont& rOther) const
    {
        sal_Int32 nRet = aName.compareTo(rOther.aName);
        if (nRet != 0)
        {
            return nRet < 0;
        }

        return aFontFamilyGeneric.compareTo(rOther.aFontFamilyGeneric) < 0;
    }
};

Color asColor(css::rendering::RGBColor const& rRGBColor)
{
    basegfx::BColor aBColor(rRGBColor.Red, rRGBColor.Green, rRGBColor.Blue);
    return Color(aBColor);
}
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFontSorting)
{
    // Given an empty document with default fonts (Liberation Sans, Lucida Sans, etc):
    loadFromURL(u"private:factory/swriter"_ustr);

    // When saving that document to ODT:
    save(u"writer8"_ustr);

    // Then make sure <style:font-face> elements are sorted (by style:name="..."):
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    xmlXPathObjectPtr pXPath
        = getXPathNode(pXmlDoc, "/office:document-content/office:font-face-decls/style:font-face");
    xmlNodeSetPtr pXmlNodes = pXPath->nodesetval;
    int nNodeCount = xmlXPathNodeSetGetLength(pXmlNodes);
    std::vector<XmlFont> aXMLFonts;
    std::vector<XmlFont> aSortedFonts;
    for (int i = 0; i < nNodeCount; ++i)
    {
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[i];
        xmlChar* pName = xmlGetProp(pXmlNode, BAD_CAST("name"));
        OString aName(reinterpret_cast<char const*>(pName));

        // Ignore numbers at the end, those are just appended to make all names unique.
        while (rtl::isAsciiDigit(static_cast<sal_uInt32>(aName[aName.getLength() - 1])))
        {
            aName = aName.copy(0, aName.getLength() - 1);
        }

        xmlChar* pFontFamilyGeneric = xmlGetProp(pXmlNode, BAD_CAST("font-family-generic"));
        OString aFontFamilyGeneric;
        if (pFontFamilyGeneric)
        {
            aFontFamilyGeneric = OString(reinterpret_cast<char const*>(pFontFamilyGeneric));
        }

        aXMLFonts.push_back(XmlFont{ aName, aFontFamilyGeneric });
        aSortedFonts.push_back(XmlFont{ aName, aFontFamilyGeneric });
        xmlFree(pName);
    }
    std::sort(aSortedFonts.begin(), aSortedFonts.end());
    size_t nIndex = 0;
    for (const auto& rFont : aSortedFonts)
    {
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: Liberation Sans
        // - Actual  : Lucida Sans1
        // i.e. the output was not lexicographically sorted, "u" was before "i".
        CPPUNIT_ASSERT_EQUAL(rFont.aName, aXMLFonts[nIndex].aName);
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: swiss
        // - Actual  : system
        // i.e. the output was not lexicographically sorted when style:name was the same, but
        // style:font-family-generic was not the same.
        CPPUNIT_ASSERT_EQUAL(rFont.aFontFamilyGeneric, aXMLFonts[nIndex].aFontFamilyGeneric);
        ++nIndex;
    }
    xmlXPathFreeObject(pXPath);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testRtlGutter)
{
    // Given a document with a gutter margin and an RTL writing mode:
    // When loading that document from ODF:
    loadFromFile(u"rtl-gutter.fodt");

    // Then make sure the page style's RtlGutter property is true.
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStandard(xStyleFamily->getByName(u"Standard"_ustr),
                                                  uno::UNO_QUERY);
    bool bRtlGutter{};
    xStandard->getPropertyValue(u"RtlGutter"_ustr) >>= bRtlGutter;
    // Without the accompanying fix in place, this test would have failed as
    // <style:page-layout-properties>'s style:writing-mode="..." did not affect RtlGutter.
    CPPUNIT_ASSERT(bRtlGutter);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testWritingModeBTLR)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });

    // Load document. It has a frame style with writing-mode bt-lr.
    // In ODF 1.3 extended it is written as loext:writing-mode="bt-lr".
    // In ODF 1.3 strict, there must not be an attribute at all.
    loadFromFile(u"tdf150407_WritingModeBTLR_style.odt");

    // Save to latest extended. Adapt test, when attribute value "bt-lr" is included in ODF strict.
    {
        save(u"writer8"_ustr);

        // With applied fix for tdf150407 still loext:writing-mode="bt-lr" has to be written.
        xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
        assertXPath(pXmlDoc,
                    "/office:document-styles/office:styles/style:style[@style:name='FrameBTLR']/"
                    "style:graphic-properties[@loext:writing-mode]");
        assertXPath(pXmlDoc,
                    "/office:document-styles/office:styles/style:style[@style:name='FrameBTLR']/"
                    "style:graphic-properties",
                    "writing-mode", u"bt-lr");
    }

    loadFromFile(u"tdf150407_WritingModeBTLR_style.odt");
    // Save to ODF 1.3 strict.
    {
        SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);
        // As of Nov 2024, validating against a version other than LATEST is not implemented.
        skipValidation();
        save(u"writer8"_ustr);

        // Without the fix an faulty 'writing-mode="bt-lr"' attribute was written in productive build.
        // A debug build fails assertion in SvXMLNamespaceMap::GetQNameByKey().
        xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
        assertXPathNoAttribute(pXmlDoc,
                               "/office:document-styles/office:styles/"
                               "style:style[@style:name='FrameBTLR']/style:graphic-properties",
                               "writing-mode");
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPosRelBottomMargin)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });

    // Load document. It has a frame position with vertical position relative to bottom margin.
    // In ODF 1.3 extended it is written as loext:vertical-rel="page-content-bottom".
    // In ODF 1.3 strict, there must not be an attribute at all.
    loadFromFile(u"tdf150407_PosRelBottomMargin.docx");

    // Save to ODF 1.3 extended. Adapt 3 (=ODFVER_LATEST) to a to be ODFVER_013_EXTENDED when
    // attribute value "page-content-bottom" is included in ODF strict.
    {
        save(u"writer8"_ustr);

        // With applied fix for tdf150407 still loext:vertical-rel="page-content-bottom" has to be
        // written.
        xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties[@loext:vertical-rel]");
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties",
            "vertical-rel", u"page-content-bottom");
    }

    loadFromFile(u"tdf150407_PosRelBottomMargin.docx");
    // Save to ODF 1.3 strict.
    {
        SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);
        save(u"writer8"_ustr);

        // Without the fix an faulty 'vertical-rel="page-content-bottom"' attribute was written in
        // productive build. A debug build fails assertion in SvXMLNamespaceMap::GetQNameByKey().
        xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
        assertXPathNoAttribute(pXmlDoc,
                               "/office:document-content/office:automatic-styles/"
                               "style:style[@style:name='gr1']/style:graphic-properties",
                               "vertical-rel");
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPosRelTopMargin)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });

    // Load document. It has a frame position with vertical position relative to top margin.
    // In ODF 1.3 extended it is written as loext:vertical-rel="page-content-top".
    // In ODF 1.3 strict, there must not be an attribute at all.
    loadFromFile(u"tdf150407_PosRelTopMargin.docx");

    // Save to ODF 1.3 extended. Adapt 3 (=ODFVER_LATEST) to a to be ODFVER_013_EXTENDED when
    // attribute value "page-content-top" is included in ODF strict.
    {
        save(u"writer8"_ustr);

        // With applied fix for tdf150407 still loext:vertical-rel="page-content-top has to be
        // written.
        xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties[@loext:vertical-rel]");
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties",
            "vertical-rel", u"page-content-top");
    }

    loadFromFile(u"tdf150407_PosRelTopMargin.docx");
    // Save to ODF 1.3 strict.
    {
        SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);
        save(u"writer8"_ustr);

        // Without the fix an faulty 'vertical-rel="page-content-top"' attribute was written in
        // productive build. A debug build fails assertion in SvXMLNamespaceMap::GetQNameByKey().
        xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
        assertXPathNoAttribute(pXmlDoc,
                               "/office:document-content/office:automatic-styles/"
                               "style:style[@style:name='gr1']/style:graphic-properties",
                               "vertical-rel");
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testTdf156707)
{
    loadFromFile(u"tdf156707_text_form_control_borders.odt");
    saveAndReload(u"writer8"_ustr);

    uno::Reference<drawing::XShape> xShape = getShape(0);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY_THROW);

    sal_uInt16 nBorderStyle = 0; // 0 = none, 1 = 3d [default], 2 = flat
    xShapeProperties->getPropertyValue(u"ControlBorder"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), nBorderStyle);

    xShape = getShape(1);
    xShapeProperties.set(xShape, uno::UNO_QUERY_THROW);
    xShapeProperties->getPropertyValue(u"ControlBorder"_ustr) >>= nBorderStyle;
    // since tdf#152974, this shape SHOULD ACTUALLY have a 3d border (1), NOT a flat one(2).
    CPPUNIT_ASSERT_EQUAL_MESSAGE("DID YOU FIX ME?", sal_uInt16(2), nBorderStyle);

    xShape = getShape(2);
    xShapeProperties.set(xShape, uno::UNO_QUERY_THROW);
    xShapeProperties->getPropertyValue(u"ControlBorder"_ustr) >>= nBorderStyle;
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), nBorderStyle);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testTdf167358)
{
    // The file contains label form fields. Labels default to having no border.
    loadFromFile(u"tdf167358_label_form_control_borders.odt");
    saveAndReload(u"writer8"_ustr);

    uno::Reference<drawing::XShape> xShape = getShape(0);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY_THROW);

    sal_uInt16 nBorderStyle = SAL_MAX_UINT16; // 0 = none, 1 = 3d [default], 2 = flat
    xShapeProperties->getPropertyValue(u"ControlBorder"_ustr) >>= nBorderStyle;
    // In this case, no fo:border style element exists, so the default must be none.
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), nBorderStyle);

    xShape = getShape(1);
    xShapeProperties.set(xShape, uno::UNO_QUERY_THROW);
    xShapeProperties->getPropertyValue(u"ControlBorder"_ustr) >>= nBorderStyle;
    // In this case, the fo:border style element doesn't specify 3d/flat/none,
    // so the default must be still be none.
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), nBorderStyle);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testMCGR_OldToNew)
{
    // The file contains a shape with linear gradient fill from red #ff0000 to yellow #ffff00,
    // named 'red2yellow'
    loadFromFile(u"MCGR_OldToNew.odg");

    // saveAndReload includes validation and must not fail with the new elements and attributes.
    saveAndReload(u"draw8"_ustr);

    // Examine file markup
    // For compatibility the file should still have the old attributes 'start-color' and 'end-color'
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    OString sPath
        = "/office:document-styles/office:styles/draw:gradient[@draw:name='red2yellow']"_ostr;
    assertXPath(pXmlDoc, sPath, "start-color", u"#ff0000");
    assertXPath(pXmlDoc, sPath, "end-color", u"#ffff00");

    // And it must have the new 'gradient-stop' elements.
    // The prefix 'loext' needs to be adapted, when the element is available in ODF strict.
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "offset", u"0");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "color-type", u"rgb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "color-value", u"#ff0000");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "offset", u"1");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "color-type", u"rgb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "color-value", u"#ffff00");

    // Examine reloaded file
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);

    // The old properties need to be still available, as they might be used in macros.
    OUString sGradientName;
    xShapeProperties->getPropertyValue(u"FillGradientName"_ustr) >>= sGradientName;
    CPPUNIT_ASSERT_EQUAL(u"red2yellow"_ustr, sGradientName);
    awt::Gradient2 aGradient;
    xShapeProperties->getPropertyValue(u"FillGradient"_ustr) >>= aGradient;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, Color(ColorTransparency, aGradient.EndColor));

    // Test new properties
    auto aColorStopSeq = aGradient.ColorStops;
    {
        awt::ColorStop aColorStop = aColorStopSeq[0];
        CPPUNIT_ASSERT_EQUAL(0.0, aColorStop.StopOffset);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, asColor(aColorStop.StopColor));
    }
    {
        awt::ColorStop aColorStop = aColorStopSeq[1];
        CPPUNIT_ASSERT_EQUAL(1.0, aColorStop.StopOffset);
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, asColor(aColorStop.StopColor));
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testMCGR_OldToNew_opacity)
{
    // The file contains a shape with solid fill and a radial transparency gradient with start 90%,
    // end 0%, border 20% and center at 50%|50%. There is only one draw:opacity element in file.
    loadFromFile(u"MCGR_OldToNew_opacity.odg");

    // saveAndReload includes validation and must not fail with the new elements and attributes.
    saveAndReload(u"draw8"_ustr);

    // Examine file markup
    // For compatibility the file should still have the old attributes.
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    OString sPath = "/office:document-styles/office:styles/draw:opacity"_ostr;
    assertXPath(pXmlDoc, sPath, "start", u"10%"); // UI 90% transparency
    assertXPath(pXmlDoc, sPath, "end", u"100%"); // UI 0% transparency
    assertXPath(pXmlDoc, sPath, "border", u"20%");
    assertXPath(pXmlDoc, sPath, "cx", u"50%");
    assertXPath(pXmlDoc, sPath, "cy", u"50%");
    assertXPath(pXmlDoc, sPath, "style", u"radial");

    // And it must have the new 'opacity-stop' elements.
    // The prefix 'loext' needs to be adapted, when the element is available in ODF strict.
    OString sFirstStop = sPath + "/loext:opacity-stop[1]";
    assertXPath(pXmlDoc, sFirstStop, "offset", u"0");
    // Because of converting through color, the grade of opacity is not exact "0.1"
    double fOpacity = getXPathContent(pXmlDoc, sFirstStop + "/@svg:stop-opacity").toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, fOpacity, 0.002);

    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[2]", "offset", u"1");
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[2]", "stop-opacity", u"1");

    // Examine reloaded file
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);

    // The old properties need to be still available, as they might be used in macros.
    awt::Gradient2 aGradient;
    xShapeProperties->getPropertyValue(u"FillTransparenceGradient"_ustr) >>= aGradient;
    CPPUNIT_ASSERT_EQUAL(Color(0xE5E5E5), Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(ColorTransparency, aGradient.EndColor));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(20), aGradient.Border);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), aGradient.XOffset);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), aGradient.YOffset);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RADIAL, aGradient.Style);

    // Test new properties
    auto aColorStopSeq = aGradient.ColorStops;
    {
        awt::ColorStop aColorStop = aColorStopSeq[0];
        CPPUNIT_ASSERT_EQUAL(0.0, aColorStop.StopOffset);
        CPPUNIT_ASSERT_EQUAL(Color(0xe5e5e5), asColor(aColorStop.StopColor));
    }
    {
        awt::ColorStop aColorStop = aColorStopSeq[1];
        CPPUNIT_ASSERT_EQUAL(1.0, aColorStop.StopOffset);
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, asColor(aColorStop.StopColor));
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testMCGR_threeStops)
{
    // The file contains a shape with square gradient fill from red #ff0000 over teal #0099bb to
    // yellow #ffff00, named 'threeStops'. It has 45deg rotation, center 0%|50%, border 10%.
    loadFromFile(u"MCGR_threeStops.fodt");

    // saveAndReload includes validation and must not fail with the new elements and attributes.
    saveAndReload(u"draw8"_ustr);

    // Examine file markup
    // For compatibility the file should still have the old attributes 'start-color' and 'end-color'
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    OString sPath
        = "/office:document-styles/office:styles/draw:gradient[@draw:name='threeStops']"_ostr;
    assertXPath(pXmlDoc, sPath, "start-color", u"#ff0000");
    assertXPath(pXmlDoc, sPath, "end-color", u"#ffff00");
    assertXPath(pXmlDoc, sPath, "style", u"square");
    assertXPath(pXmlDoc, sPath, "cx", u"0%");
    assertXPath(pXmlDoc, sPath, "cy", u"50%");
    assertXPath(pXmlDoc, sPath, "angle", u"45deg");
    assertXPath(pXmlDoc, sPath, "border", u"10%");

    // And it must have the new 'gradient-stop' elements.
    // The prefix 'loext' needs to be adapted, when the element is available in ODF strict.
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "offset", u"0");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "color-type", u"rgb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "color-value", u"#ff0000");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "offset", u"0.3");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "color-type", u"rgb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "color-value", u"#0099bb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[3]", "offset", u"1");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[3]", "color-type", u"rgb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[3]", "color-value", u"#ffff00");

    // Examine reloaded file
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);

    // The old properties need to be still available, as they might be used in macros.
    OUString sGradientName;
    xShapeProperties->getPropertyValue(u"FillGradientName"_ustr) >>= sGradientName;
    CPPUNIT_ASSERT_EQUAL(u"threeStops"_ustr, sGradientName);
    awt::Gradient2 aGradient;
    xShapeProperties->getPropertyValue(u"FillGradient"_ustr) >>= aGradient;
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, Color(ColorTransparency, aGradient.EndColor));
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_SQUARE, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aGradient.XOffset);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), aGradient.YOffset);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(450), aGradient.Angle);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), aGradient.Border);

    // Test new properties
    auto aColorStopSeq = aGradient.ColorStops;
    {
        awt::ColorStop aColorStop = aColorStopSeq[0];
        CPPUNIT_ASSERT_EQUAL(0.0, aColorStop.StopOffset);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, asColor(aColorStop.StopColor));
    }
    {
        awt::ColorStop aColorStop = aColorStopSeq[1];
        CPPUNIT_ASSERT_EQUAL(0.3, aColorStop.StopOffset);
        CPPUNIT_ASSERT_EQUAL(Color(0x0099bb), asColor(aColorStop.StopColor));
    }
    {
        awt::ColorStop aColorStop = aColorStopSeq[2];
        CPPUNIT_ASSERT_EQUAL(1.0, aColorStop.StopOffset);
        CPPUNIT_ASSERT_EQUAL(COL_YELLOW, asColor(aColorStop.StopColor));
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testBorderRestoration)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });

    // Load document. It has a shape with color gradient build from color stop yellow at offset 0.5
    // and color stop red at offset 1.0. For better backward compatibility such gradient has to be
    // exported to ODF with a border of 50%.
    // When gradient-stops are integrated in ODF strict, the test needs to be adapted.

    loadFromFile(u"MCGR_Border_restoration.pptx");

    // Backup original ODF default version

    // Save to LATEST; that is extended. Make sure gradient-stop elements have
    // offsets 0 and 1, and border is written as 50%.
    save(u"impress8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    OString sPath
        = "/office:document-styles/office:styles/draw:gradient[@draw:name='Gradient_20_1']"_ostr;
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "color-value", u"#ff0000");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "offset", u"1");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "color-value", u"#ffff00");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "offset", u"0");
    assertXPath(pXmlDoc, sPath, "border", u"50%");

    // Save to ODF 1.3 strict and make sure border, start-color and end-color are suitable set.
    SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);
    // As of Nov 2024, validating against a version other than LATEST is not implemented.
    skipValidation();
    save(u"impress8"_ustr);
    pXmlDoc = parseExport(u"styles.xml"_ustr);
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop", 0);
    assertXPath(pXmlDoc, sPath, "start-color", u"#ffff00");
    assertXPath(pXmlDoc, sPath, "end-color", u"#ff0000");
    assertXPath(pXmlDoc, sPath, "border", u"50%");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testTransparencyBorderRestoration)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });

    // Load document. It has a shape with transparency gradient build from transparency 100% at
    // offset 0, transparency 100% at offset 0.4 and transparency 10% at offset 1.0. For better
    // backward compatibility such gradient is exported with a border of 40% in the transparency
    // gradient. The color itself is the same for all gradient stops.
    // When transparency gradient-stops are integrated in ODF strict, the test needs to be adapted.
    loadFromFile(u"MCGR_TransparencyBorder_restoration.pptx");

    // Save to LATEST, that is extended. Make sure transparency gradient-stop
    //elements are written with offset 0 and 1, and border is written as 40%.
    save(u"impress8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    OString sPath = "/office:document-styles/office:styles/draw:opacity[1]"_ostr;
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[2]", "stop-opacity", u"0.9");
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[2]", "offset", u"1");
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[1]", "stop-opacity", u"0");
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[1]", "offset", u"0");
    assertXPath(pXmlDoc, sPath, "border", u"40%");
    // As of Nov 2024, validating against a version other than LATEST is not implemented.
    skipValidation();
    // Save to ODF 1.3 strict and make sure border, start and end opacity are suitable set.
    SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);

    save(u"impress8"_ustr);
    pXmlDoc = parseExport(u"styles.xml"_ustr);
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop", 0);
    assertXPath(pXmlDoc, sPath, "start", u"0%");
    assertXPath(pXmlDoc, sPath, "end", u"90%");
    assertXPath(pXmlDoc, sPath, "border", u"40%");
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testAxialGradientCompatible)
{
    // tdf#155549. An axial gradient with Border, StartColor A and EndColor B is exported to OOXML as
    // symmetrical linear gradient with three stops, colors B A B. After the changes for multi-color
    // gradients (MCGR) this is imported as linear gradient with colors B A B. So a consumer not able
    // of MCGR would get a linear gradient with start and end color B. For better compatibility
    // ODF export writes an axial gradient. with colors A and B.
    // This test needs to be adapted when color stops are available in ODF strict and widely
    // supported in even older LibreOffice versions.
    loadFromFile(u"tdf155549_MCGR_AxialGradientCompatible.odt");

    //Round-trip through OOXML.
    // FixMe tdf#153183. Here "Attribute 'ID' is not allowed to appear in element 'v:rect'".
    skipValidation();
    saveAndReload(u"Office Open XML Text"_ustr);
    saveAndReload(u"writer8"_ustr);

    // Examine reloaded file
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT_MESSAGE("No shape", xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);

    // Without fix these would have failed with Style=0 (=LINEAR), StartColor=0xFFFF00 and Border=0.
    awt::Gradient2 aGradient;
    xShapeProperties->getPropertyValue(u"FillGradient"_ustr) >>= aGradient;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("gradient style", awt::GradientStyle_AXIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("EndColor", sal_Int32(0xFFFF00), aGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StartColor", sal_Int32(0x1E90FF), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Border", sal_Int16(20), aGradient.Border);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testAxialTransparencyCompatible)
{
    // tdf#155549. The shape in the document has a solid color and an axial transparency gradient
    // with 'Transition start 60%', 'Start value 10%' and 'End value 80%'. The gradient is exported
    // to OOXML as linear symmetrical gradient with three gradient stops. After the changes for
    // multi-color gradients (MCGR) this is imported as linear transparency gradient. For better
    // compatibility with consumers not able to use MCGR, the ODF export writes the transparency as
    // axial transparency gradient that is same as in the original document.
    // This test needs to be adapted when color stops are available in ODF strict and widely
    // supported in even older LibreOffice versions.
    loadFromFile(u"tdf155549_MCGR_AxialTransparencyCompatible.odt");

    //Round-trip through OOXML.
    // FixMe tdf#153183, and error in charSpace and in CharacterSet
    //skipValidation();
    saveAndReload(u"Office Open XML Text"_ustr);
    saveAndReload(u"writer8"_ustr);

    // Examine reloaded file
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);

    // Without fix these would have failed with Style=LINEAR, StartColor=0xCCCCCC and wrong Border.
    awt::Gradient2 aTransGradient;
    xShapeProperties->getPropertyValue(u"FillTransparenceGradient"_ustr) >>= aTransGradient;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("gradient style", awt::GradientStyle_AXIAL, aTransGradient.Style);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("EndColor", sal_Int32(0xCCCCCC), aTransGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StartColor", sal_Int32(0x191919), aTransGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Border", sal_Int16(60), aTransGradient.Border);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
