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

#include <officecfg/Office/Common.hxx>
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
    : UnoApiXmlTest("/xmloff/qa/unit/data/")
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
        xFactory->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed, as the base64 stream was
    // not considered when parsing the fill-image style.
    CPPUNIT_ASSERT(xBitmaps->hasByName("libreoffice_0"));
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

Color asColor(com::sun::star::rendering::RGBColor const& rRGBColor)
{
    basegfx::BColor aBColor(rRGBColor.Red, rRGBColor.Green, rRGBColor.Blue);
    return Color(aBColor);
}
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testFontSorting)
{
    // Given an empty document with default fonts (Liberation Sans, Lucida Sans, etc):
    mxComponent = loadFromDesktop("private:factory/swriter");

    // When saving that document to ODT:
    save("writer8");

    // Then make sure <style:font-face> elements are sorted (by style:name="..."):
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    xmlXPathObjectPtr pXPath = getXPathNode(
        pXmlDoc, "/office:document-content/office:font-face-decls/style:font-face"_ostr);
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
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStandard(xStyleFamily->getByName("Standard"),
                                                  uno::UNO_QUERY);
    bool bRtlGutter{};
    xStandard->getPropertyValue("RtlGutter") >>= bRtlGutter;
    // Without the accompanying fix in place, this test would have failed as
    // <style:page-layout-properties>'s style:writing-mode="..." did not affect RtlGutter.
    CPPUNIT_ASSERT(bRtlGutter);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testWritingModeBTLR)
{
    // Load document. It has a frame style with writing-mode bt-lr.
    // In ODF 1.3 extended it is written as loext:writing-mode="bt-lr".
    // In ODF 1.3 strict, there must not be an attribute at all.
    loadFromFile(u"tdf150407_WritingModeBTLR_style.odt");

    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        return pBatch->commit();
    });

    // Save to ODF 1.3 extended. Adapt 3 (=ODFVER_LATEST) to a to be ODFVER_013_EXTENDED when
    // attribute value "bt-lr" is included in ODF strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        pBatch->commit();
        save("writer8");

        // With applied fix for tdf150407 still loext:writing-mode="bt-lr" has to be written.
        xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
        assertXPath(pXmlDoc,
                    "/office:document-styles/office:styles/style:style[@style:name='FrameBTLR']/"
                    "style:graphic-properties[@loext:writing-mode]"_ostr);
        assertXPath(pXmlDoc,
                    "/office:document-styles/office:styles/style:style[@style:name='FrameBTLR']/"
                    "style:graphic-properties"_ostr,
                    "writing-mode"_ostr, "bt-lr");
    }

    loadFromFile(u"tdf150407_WritingModeBTLR_style.odt");
    // Save to ODF 1.3 strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(10, pBatch);
        pBatch->commit();
        save("writer8");

        // Without the fix an faulty 'writing-mode="bt-lr"' attribute was written in productive build.
        // A debug build fails assertion in SvXMLNamespaceMap::GetQNameByKey().
        xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
        assertXPathNoAttribute(pXmlDoc,
                               "/office:document-styles/office:styles/"
                               "style:style[@style:name='FrameBTLR']/style:graphic-properties"_ostr,
                               "writing-mode"_ostr);
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPosRelBottomMargin)
{
    // Load document. It has a frame position with vertical position relative to bottom margin.
    // In ODF 1.3 extended it is written as loext:vertical-rel="page-content-bottom".
    // In ODF 1.3 strict, there must not be an attribute at all.
    loadFromFile(u"tdf150407_PosRelBottomMargin.docx");

    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        return pBatch->commit();
    });

    // Save to ODF 1.3 extended. Adapt 3 (=ODFVER_LATEST) to a to be ODFVER_013_EXTENDED when
    // attribute value "page-content-bottom" is included in ODF strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        pBatch->commit();
        save("writer8");

        // With applied fix for tdf150407 still loext:vertical-rel="page-content-bottom" has to be
        // written.
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties[@loext:vertical-rel]"_ostr);
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties"_ostr,
            "vertical-rel"_ostr, "page-content-bottom");
    }

    loadFromFile(u"tdf150407_PosRelBottomMargin.docx");
    // Save to ODF 1.3 strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(10, pBatch);
        pBatch->commit();
        save("writer8");

        // Without the fix an faulty 'vertical-rel="page-content-bottom"' attribute was written in
        // productive build. A debug build fails assertion in SvXMLNamespaceMap::GetQNameByKey().
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        assertXPathNoAttribute(pXmlDoc,
                               "/office:document-content/office:automatic-styles/"
                               "style:style[@style:name='gr1']/style:graphic-properties"_ostr,
                               "vertical-rel"_ostr);
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testPosRelTopMargin)
{
    // Load document. It has a frame position with vertical position relative to top margin.
    // In ODF 1.3 extended it is written as loext:vertical-rel="page-content-top".
    // In ODF 1.3 strict, there must not be an attribute at all.
    loadFromFile(u"tdf150407_PosRelTopMargin.docx");

    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        return pBatch->commit();
    });

    // Save to ODF 1.3 extended. Adapt 3 (=ODFVER_LATEST) to a to be ODFVER_013_EXTENDED when
    // attribute value "page-content-top" is included in ODF strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        pBatch->commit();
        save("writer8");

        // With applied fix for tdf150407 still loext:vertical-rel="page-content-top has to be
        // written.
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties[@loext:vertical-rel]"_ostr);
        assertXPath(
            pXmlDoc,
            "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/"
            "style:graphic-properties"_ostr,
            "vertical-rel"_ostr, "page-content-top");
    }

    loadFromFile(u"tdf150407_PosRelTopMargin.docx");
    // Save to ODF 1.3 strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(10, pBatch);
        pBatch->commit();
        save("writer8");

        // Without the fix an faulty 'vertical-rel="page-content-top"' attribute was written in
        // productive build. A debug build fails assertion in SvXMLNamespaceMap::GetQNameByKey().
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        assertXPathNoAttribute(pXmlDoc,
                               "/office:document-content/office:automatic-styles/"
                               "style:style[@style:name='gr1']/style:graphic-properties"_ostr,
                               "vertical-rel"_ostr);
    }
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testMCGR_OldToNew)
{
    // The file contains a shape with linear gradient fill from red #ff0000 to yellow #ffff00,
    // named 'red2yellow'
    loadFromFile(u"MCGR_OldToNew.odg");

    // saveAndReload includes validation and must not fail with the new elements and attributes.
    saveAndReload("draw8");

    // Examine file markup
    // For compatibility the file should still have the old attributes 'start-color' and 'end-color'
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    OString sPath
        = "/office:document-styles/office:styles/draw:gradient[@draw:name='red2yellow']"_ostr;
    assertXPath(pXmlDoc, sPath, "start-color"_ostr, "#ff0000");
    assertXPath(pXmlDoc, sPath, "end-color"_ostr, "#ffff00");

    // And it must have the new 'gradient-stop' elements.
    // The prefix 'loext' needs to be adapted, when the element is available in ODF strict.
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "offset"_ostr, "0");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "color-type"_ostr, "rgb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "color-value"_ostr, "#ff0000");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "offset"_ostr, "1");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "color-type"_ostr, "rgb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "color-value"_ostr, "#ffff00");

    // Examine reloaded file
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);

    // The old properties need to be still available, as they might be used in macros.
    OUString sGradientName;
    xShapeProperties->getPropertyValue("FillGradientName") >>= sGradientName;
    CPPUNIT_ASSERT_EQUAL(u"red2yellow"_ustr, sGradientName);
    awt::Gradient2 aGradient;
    xShapeProperties->getPropertyValue("FillGradient") >>= aGradient;
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
    saveAndReload("draw8");

    // Examine file markup
    // For compatibility the file should still have the old attributes.
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    OString sPath = "/office:document-styles/office:styles/draw:opacity"_ostr;
    assertXPath(pXmlDoc, sPath, "start"_ostr, "10%"); // UI 90% transparency
    assertXPath(pXmlDoc, sPath, "end"_ostr, "100%"); // UI 0% transparency
    assertXPath(pXmlDoc, sPath, "border"_ostr, "20%");
    assertXPath(pXmlDoc, sPath, "cx"_ostr, "50%");
    assertXPath(pXmlDoc, sPath, "cy"_ostr, "50%");
    assertXPath(pXmlDoc, sPath, "style"_ostr, "radial");

    // And it must have the new 'opacity-stop' elements.
    // The prefix 'loext' needs to be adapted, when the element is available in ODF strict.
    OString sFirstStop = sPath + "/loext:opacity-stop[1]";
    assertXPath(pXmlDoc, sFirstStop, "offset"_ostr, "0");
    // Because of converting through color, the grade of opacity is not exact "0.1"
    double fOpacity = getXPathContent(pXmlDoc, sFirstStop + "/@svg:stop-opacity").toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, fOpacity, 0.002);

    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[2]", "offset"_ostr, "1");
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[2]", "stop-opacity"_ostr, "1");

    // Examine reloaded file
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);

    // The old properties need to be still available, as they might be used in macros.
    awt::Gradient2 aGradient;
    xShapeProperties->getPropertyValue("FillTransparenceGradient") >>= aGradient;
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
    saveAndReload("draw8");

    // Examine file markup
    // For compatibility the file should still have the old attributes 'start-color' and 'end-color'
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    OString sPath
        = "/office:document-styles/office:styles/draw:gradient[@draw:name='threeStops']"_ostr;
    assertXPath(pXmlDoc, sPath, "start-color"_ostr, "#ff0000");
    assertXPath(pXmlDoc, sPath, "end-color"_ostr, "#ffff00");
    assertXPath(pXmlDoc, sPath, "style"_ostr, "square");
    assertXPath(pXmlDoc, sPath, "cx"_ostr, "0%");
    assertXPath(pXmlDoc, sPath, "cy"_ostr, "50%");
    assertXPath(pXmlDoc, sPath, "angle"_ostr, "45deg");
    assertXPath(pXmlDoc, sPath, "border"_ostr, "10%");

    // And it must have the new 'gradient-stop' elements.
    // The prefix 'loext' needs to be adapted, when the element is available in ODF strict.
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "offset"_ostr, "0");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "color-type"_ostr, "rgb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "color-value"_ostr, "#ff0000");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "offset"_ostr, "0.3");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "color-type"_ostr, "rgb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "color-value"_ostr, "#0099bb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[3]", "offset"_ostr, "1");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[3]", "color-type"_ostr, "rgb");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[3]", "color-value"_ostr, "#ffff00");

    // Examine reloaded file
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);

    // The old properties need to be still available, as they might be used in macros.
    OUString sGradientName;
    xShapeProperties->getPropertyValue("FillGradientName") >>= sGradientName;
    CPPUNIT_ASSERT_EQUAL(u"threeStops"_ustr, sGradientName);
    awt::Gradient2 aGradient;
    xShapeProperties->getPropertyValue("FillGradient") >>= aGradient;
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
    // Load document. It has a shape with color gradient build from color stop yellow at offset 0.5
    // and color stop red at offset 1.0. For better backward compatibility such gradient has to be
    // exported to ODF with a border of 50%.
    // When gradient-stops are integrated in ODF strict, the test needs to be adapted.

    loadFromFile(u"MCGR_Border_restoration.pptx");

    // Backup original ODF default version
    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion(GetODFDefaultVersion());

    // Save to ODF_LATEST which is currently ODF 1.3 extended. Make sure gradient-stop elements have
    // offsets 0 and 1, and border is written as 50%.
    SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_LATEST);
    save("impress8");
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    OString sPath
        = "/office:document-styles/office:styles/draw:gradient[@draw:name='Gradient_20_1']"_ostr;
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "color-value"_ostr, "#ff0000");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[2]", "offset"_ostr, "1");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "color-value"_ostr, "#ffff00");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop[1]", "offset"_ostr, "0");
    assertXPath(pXmlDoc, sPath, "border"_ostr, "50%");

    // Save to ODF 1.3 strict and make sure border, start-color and end-color are suitable set.
    SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);
    save("impress8");
    pXmlDoc = parseExport("styles.xml");
    assertXPath(pXmlDoc, sPath + "/loext:gradient-stop", 0);
    assertXPath(pXmlDoc, sPath, "start-color"_ostr, "#ffff00");
    assertXPath(pXmlDoc, sPath, "end-color"_ostr, "#ff0000");
    assertXPath(pXmlDoc, sPath, "border"_ostr, "50%");

    // Set back to original ODF default version.
    SetODFDefaultVersion(nCurrentODFVersion);
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testTransparencyBorderRestoration)
{
    // Load document. It has a shape with transparency gradient build from transparency 100% at
    // offset 0, transparency 100% at offset 0.4 and transparency 10% at offset 1.0. For better
    // backward compatibility such gradient is exported with a border of 40% in the transparency
    // gradient. The color itself is the same for all gradient stops.
    // When transparency gradient-stops are integrated in ODF strict, the test needs to be adapted.
    loadFromFile(u"MCGR_TransparencyBorder_restoration.pptx");

    // Backup original ODF default version
    const SvtSaveOptions::ODFDefaultVersion nCurrentODFVersion(GetODFDefaultVersion());

    // Save to ODF_LATEST which is currently ODF 1.3 extended. Make sure transparency gradient-stop
    //elements are written with offset 0 and 1, and border is written as 40%.
    SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_LATEST);
    save("impress8");
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    OString sPath = "/office:document-styles/office:styles/draw:opacity[1]"_ostr;
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[2]", "stop-opacity"_ostr, "0.9");
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[2]", "offset"_ostr, "1");
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[1]", "stop-opacity"_ostr, "0");
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop[1]", "offset"_ostr, "0");
    assertXPath(pXmlDoc, sPath, "border"_ostr, "40%");

    // Save to ODF 1.3 strict and make sure border, start and end opacity are suitable set.
    SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);
    save("impress8");
    pXmlDoc = parseExport("styles.xml");
    assertXPath(pXmlDoc, sPath + "/loext:opacity-stop", 0);
    assertXPath(pXmlDoc, sPath, "start"_ostr, "0%");
    assertXPath(pXmlDoc, sPath, "end"_ostr, "90%");
    assertXPath(pXmlDoc, sPath, "border"_ostr, "40%");

    // Set back to original ODF default version.
    SetODFDefaultVersion(nCurrentODFVersion);
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
    saveAndReload("Office Open XML Text");
    saveAndReload("writer8");

    // Examine reloaded file
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT_MESSAGE("No shape", xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);

    // Without fix these would have failed with Style=0 (=LINEAR), StartColor=0xFFFF00 and Border=0.
    awt::Gradient2 aGradient;
    xShapeProperties->getPropertyValue("FillGradient") >>= aGradient;
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
    saveAndReload("Office Open XML Text");
    saveAndReload("writer8");

    // Examine reloaded file
    uno::Reference<drawing::XShape> xShape(getShape(0));
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);

    // Without fix these would have failed with Style=LINEAR, StartColor=0xCCCCCC and wrong Border.
    awt::Gradient2 aTransGradient;
    xShapeProperties->getPropertyValue("FillTransparenceGradient") >>= aTransGradient;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("gradient style", awt::GradientStyle_AXIAL, aTransGradient.Style);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("EndColor", sal_Int32(0xCCCCCC), aTransGradient.EndColor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("StartColor", sal_Int32(0x191919), aTransGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Border", sal_Int16(60), aTransGradient.Border);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
