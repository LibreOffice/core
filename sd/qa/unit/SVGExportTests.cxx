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
#include <test/unoapixml_test.hxx>

#include <sal/macros.h>
#include <unotools/syslocaleoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <regex>

#define SVG_SVG  *[name()='svg']
#define SVG_G *[name()='g']
#define SVG_TEXT *[name()='text']
#define SVG_TSPAN *[name()='tspan']
#define SVG_DEFS *[name()='defs']
#define SVG_IMAGE *[name()='image']
#define SVG_USE *[name()='use']
#define SVG_PATTERN *[name()='pattern']
#define SVG_RECT *[name()='rect']
#define SVG_FOREIGNOBJECT *[name()='foreignObject']
#define SVG_BODY *[name()='body']
#define SVG_VIDEO *[name()='video']

using namespace css;

namespace
{
bool isValidBitmapId(const OUString& sId)
{
    std::regex aRegEx("bitmap\\(\\d+\\)");
    return std::regex_match(sId.toUtf8().getStr(), aRegEx);
}

BitmapChecksum getBitmapChecksumFromId(std::u16string_view sId)
{
    size_t nStart = sId.find(u"(") + 1;
    size_t nCount = sId.find(u")") - nStart;
    bool bIsValidRange = nStart > 0 && nStart != std::u16string_view::npos && nCount > 0;
    CPPUNIT_ASSERT(bIsValidRange);
    OUString sChecksum( sId.substr( nStart, nCount ) );
    return sChecksum.toUInt64();
}

bool isValidBackgroundPatternId(const OUString& sId)
{
    std::regex aRegEx( R"(bg\-pattern\.id\d+\.\d+)" );
    return std::regex_match(sId.toUtf8().getStr(), aRegEx);
}

bool isValidTiledBackgroundId(const OUString& sId)
{
    std::regex aRegEx( R"(bg\-id\d+\.\d+)" );
    return std::regex_match(sId.toUtf8().getStr(), aRegEx);
}

}

class SdSVGFilterTest : public UnoApiXmlTest
{
public:
    SdSVGFilterTest()
        : UnoApiXmlTest(u"/sd/qa/unit/data/odp/"_ustr)
    {
    }

    void testSVGExportTextDecorations()
    {
        loadFromFile(u"svg-export-text-decorations.odp");
        save(u"impress_svg_Export"_ustr);

        xmlDocUniquePtr svgDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(svgDoc);

        svgDoc->name = reinterpret_cast<char *>(xmlStrdup(reinterpret_cast<xmlChar const *>(OUStringToOString(maTempFile.GetURL(), RTL_TEXTENCODING_UTF8).getStr())));

        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG ), 1);
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2] ), "class", u"SlideGroup");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G ), "class", u"Slide");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[1] ), "class", u"TitleText");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[1]/SVG_G/SVG_TEXT ), "class", u"SVGTextShape");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[1]/SVG_G/SVG_TEXT/SVG_TSPAN/SVG_TSPAN ), "class", u"TextPosition");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[1]/SVG_G/SVG_TEXT/SVG_TSPAN/SVG_TSPAN/SVG_TSPAN[1] ), "text-decoration", u"underline");

        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[2]/SVG_G/SVG_TEXT ), "class", u"SVGTextShape");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[2]/SVG_G/SVG_TEXT/SVG_TSPAN/SVG_TSPAN ), "class", u"TextPosition");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_G[2]/SVG_G/SVG_TEXT/SVG_TSPAN/SVG_TSPAN/SVG_TSPAN ), "text-decoration", u"line-through");
    }

    void testSVGExportJavascriptURL()
    {
        loadFromFile(u"textbox-link-javascript.odp");
        save(u"impress_svg_Export"_ustr);

        xmlDocUniquePtr svgDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(svgDoc);

        // There should be only one child (no link to javascript url)
        assertXPathChildren(svgDoc,
                            SAL_STRINGIFY(/ SVG_SVG / SVG_G[2] / SVG_G / SVG_G / SVG_G / SVG_G
                                             / SVG_G[3] / SVG_G),
                            1);
    }

    void testSVGExportSlideCustomBackground()
    {
        loadFromFile(u"slide-custom-background.odp");
        save(u"impress_svg_Export"_ustr);

        xmlDocUniquePtr svgDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(svgDoc);

        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G/SVG_G/SVG_G/SVG_G/SVG_DEFS ), "class", u"SlideBackground");
    }

    void testSVGExportTextFieldsInMasterPage()
    {
        loadFromFile(u"text-fields.odp");
        save(u"impress_svg_Export"_ustr);

        xmlDocUniquePtr svgDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(svgDoc);

        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2] ), "class", u"Master_Slide");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2] ), "class", u"BackgroundObjects");
        // Current Date Field
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[4] ), "class", u"TextShape");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[4]/SVG_G/SVG_TEXT/SVG_TSPAN/SVG_TSPAN/SVG_TSPAN ), "class", u"PlaceholderText Date");
        // Current Time Field
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[5] ), "class", u"TextShape");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[5]/SVG_G/SVG_TEXT/SVG_TSPAN/SVG_TSPAN/SVG_TSPAN ), "class", u"PlaceholderText Time");
        // Slide Name Field
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[6] ), "class", u"TextShape");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[6]/SVG_G/SVG_TEXT/SVG_TSPAN/SVG_TSPAN/SVG_TSPAN ), "class", u"PlaceholderText PageName");
        // Slide Number Field
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[7] ), "class", u"TextShape");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[7]/SVG_G/SVG_TEXT/SVG_TSPAN/SVG_TSPAN/SVG_TSPAN ), "class", u"PlaceholderText PageNumber");
    }

    void testSVGExportEmbeddedVideo()
    {
        loadFromFile(u"slide-video-thumbnail.odp");
        save(u"impress_svg_Export"_ustr);

        xmlDocUniquePtr svgDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(svgDoc);

        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG ), 1);
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2] ), "class", u"SlideGroup");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1] ), "visibility", u"hidden");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1] ), "id", u"container-id1");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1] ), "class", u"Slide");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1] ), "class", u"Page");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1] ), "class", u"TitleText");

        // First one has no valid video, so we just generate the stock thumbnail as an image.
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[2] ), "class", u"com.sun.star.presentation.MediaShape");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[2]/SVG_G[1]/SVG_IMAGE ), 1);

        // The second one is a valid video, with the thumbnail embedded.
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[5] ), "class", u"com.sun.star.drawing.MediaShape");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[5]/SVG_G[1] ), 1);
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[5]/SVG_G[1]/SVG_FOREIGNOBJECT ), 1);
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[5]/SVG_G[1]/SVG_FOREIGNOBJECT/SVG_BODY ), 1);
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[5]/SVG_G[1]/SVG_FOREIGNOBJECT/SVG_BODY/SVG_VIDEO ), "preload", u"auto");

        const OUString poster = getXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[1]/SVG_G[5]/SVG_G[1]/SVG_FOREIGNOBJECT/SVG_BODY/SVG_VIDEO), "poster");
        CPPUNIT_ASSERT_MESSAGE("The video poster is invalid", poster.startsWith("data:image/png;base64,"));
    }

    void testSVGExportSlideBitmapBackground()
    {
        loadFromFile(u"slide-bitmap-background.odp");
        save(u"impress_svg_Export"_ustr);

        xmlDocUniquePtr svgDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(svgDoc);

        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9] ), "class", u"BackgroundBitmaps");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_IMAGE ), 1);

        OUString sImageId = getXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_IMAGE ), "id");
        CPPUNIT_ASSERT_MESSAGE(OString("The exported bitmap has not a valid id: " + sImageId.toUtf8()).getStr(), isValidBitmapId(sImageId));

        BitmapChecksum nChecksum = getBitmapChecksumFromId(sImageId);
        CPPUNIT_ASSERT_MESSAGE(OString("The exported bitmap has not a valid checksum: " + sImageId.toUtf8()).getStr(), nChecksum != 0);

        // single image case
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G/SVG_G/SVG_G/SVG_DEFS ), "class", u"SlideBackground");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G/SVG_G/SVG_G/SVG_DEFS/SVG_G/SVG_G/SVG_USE ), 1);
        OUString sRef = getXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G/SVG_G/SVG_G/SVG_DEFS/SVG_G/SVG_G/SVG_USE ), "href");
        CPPUNIT_ASSERT_MESSAGE("The <use> element has not a valid href attribute: starting '#' not present.", sRef.startsWith("#"));
        sRef = sRef.copy(1);
        CPPUNIT_ASSERT_MESSAGE(OString("The <use> element does not point to a valid bitmap id: " + sRef.toUtf8()).getStr(), isValidBitmapId(sRef));

        BitmapChecksum nUseChecksum = getBitmapChecksumFromId(sRef);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The bitmap checksum used in <use> does not match the expected one: ", nChecksum, nUseChecksum);
    }

    void testSVGExportSlideTileBitmapBackground()
    {
        loadFromFile(u"slide-tile-background.odp");
        save(u"impress_svg_Export"_ustr);

        xmlDocUniquePtr svgDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(svgDoc);

        // check the bitmap
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9] ), "class", u"BackgroundBitmaps");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_IMAGE ), 1);

        // check the pattern and background rectangle
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[10] ), "class", u"BackgroundPatterns");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[10]/SVG_PATTERN ), 1);
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[10]/SVG_PATTERN/SVG_USE ), 1);
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[10]/SVG_G/SVG_RECT ), 1);


        // check that <pattern><use> is pointing to the correct <image>
        OUString sImageId = getXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_IMAGE ), "id");
        CPPUNIT_ASSERT_MESSAGE(OString("The exported bitmap has not a valid id: " + sImageId.toUtf8()).getStr(), isValidBitmapId(sImageId));

        BitmapChecksum nChecksum = getBitmapChecksumFromId(sImageId);
        CPPUNIT_ASSERT_MESSAGE(OString("The exported bitmap has not a valid checksum: " + sImageId.toUtf8()).getStr(), nChecksum != 0);

        OUString sRef = getXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[10]/SVG_PATTERN/SVG_USE ), "href");
        CPPUNIT_ASSERT_MESSAGE("The <pattern><use> element has not a valid href attribute: starting '#' not present.", sRef.startsWith("#"));
        sRef = sRef.copy(1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The href attribute for <pattern><use> does not match the <image> id attribute: ", sImageId, sRef);

        OUString sPatternId = getXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[10]/SVG_PATTERN ), "id");
        CPPUNIT_ASSERT_MESSAGE(OString("The exported pattern has not a valid id: " + sPatternId.toUtf8()).getStr(), isValidBackgroundPatternId(sPatternId));

        OUString sFillUrl = getXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[10]/SVG_G/SVG_RECT ), "fill");
        bool bIsUrlFormat = sFillUrl.startsWith("url(#") && sFillUrl.endsWith(")");
        CPPUNIT_ASSERT_MESSAGE("The fill attribute for the <rectangle> element has not a url format .", bIsUrlFormat);
        // remove "url(#" and ")"
        sFillUrl = sFillUrl.copy(5, sFillUrl.getLength() - 6);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The fill url for <rectangle> does not match the <pattern> id attribute: ", sPatternId, sFillUrl);

        OUString sBackgroundId = getXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[10]/SVG_G ), "id");
        CPPUNIT_ASSERT_MESSAGE(OString("The exported tiled background has not a valid id: " + sBackgroundId.toUtf8()).getStr(), isValidTiledBackgroundId(sBackgroundId));

        // check <use> element that point to the tiled background
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G/SVG_G/SVG_G/SVG_DEFS ), "class", u"SlideBackground");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G/SVG_G/SVG_G/SVG_DEFS/SVG_G/SVG_USE ), 1);

        sRef = getXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_G[2]/SVG_G[1]/SVG_G/SVG_G/SVG_G/SVG_DEFS/SVG_G/SVG_USE ), "href");
        CPPUNIT_ASSERT_MESSAGE("The <use> element has not a valid href attribute: starting '#' not present.", sRef.startsWith("#"));
        sRef = sRef.copy(1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The href attribute for <use> does not match the tiled background id attribute: ", sBackgroundId, sRef);
    }

    void testSVGPlaceholderLocale()
    {
        static constexpr OUString aLangISO(u"it-IT"_ustr);
        SvtSysLocaleOptions aSysLocaleOptions;
        aSysLocaleOptions.SetLocaleConfigString(aLangISO);
        aSysLocaleOptions.SetUILocaleConfigString(aLangISO);

        auto aSavedSettings = Application::GetSettings();
        Resetter aResetter([&]() { Application::SetSettings(aSavedSettings); });
        AllSettings aSettings(aSavedSettings);
        aSettings.SetLanguageTag(aLangISO, true);
        Application::SetSettings(aSettings);

        loadFromFile(u"text-fields.odp");
        save(u"impress_svg_Export"_ustr);

        xmlDocUniquePtr svgDoc = parseXml(maTempFile);
        CPPUNIT_ASSERT(svgDoc);

        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2] ), "class", u"Master_Slide");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2] ), "class", u"BackgroundObjects");

        // Slide Name Field
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[6] ), "class", u"TextShape");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[6]/SVG_G/SVG_TEXT/SVG_TSPAN/SVG_TSPAN/SVG_TSPAN ), "class", u"PlaceholderText PageName");
        // Slide Number Field
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[7] ), "class", u"TextShape");
        assertXPath(svgDoc, SAL_STRINGIFY( /SVG_SVG/SVG_DEFS[9]/SVG_G[2]/SVG_G[2]/SVG_G[7]/SVG_G/SVG_TEXT/SVG_TSPAN/SVG_TSPAN/SVG_TSPAN ), "class", u"PlaceholderText PageNumber");
    }

    CPPUNIT_TEST_SUITE(SdSVGFilterTest);
    CPPUNIT_TEST(testSVGExportTextDecorations);
    CPPUNIT_TEST(testSVGExportJavascriptURL);
    CPPUNIT_TEST(testSVGExportSlideCustomBackground);
    CPPUNIT_TEST(testSVGExportTextFieldsInMasterPage);
    CPPUNIT_TEST(testSVGExportEmbeddedVideo);
    CPPUNIT_TEST(testSVGExportSlideBitmapBackground);
    CPPUNIT_TEST(testSVGExportSlideTileBitmapBackground);
    CPPUNIT_TEST(testSVGPlaceholderLocale);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SdSVGFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
