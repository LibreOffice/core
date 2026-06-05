/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "sdmodeltestbase.hxx"
#include <sdpage.hxx>
#include <sdtiledrenderingtest.hxx>

#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequence.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <osl/process.h>
#include <unotools/saveopt.hxx>

#include <svx/svdotext.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdomedia.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextColumns.hpp>

#include <svx/svdotable.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace css;
using namespace css::animations;

class SdExportTest4 : public SdModelTestBase
{
public:
    SdExportTest4()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SdExportTest4, testGlow)
{
    createSdDrawDoc("odg/glow.odg");
    saveAndReload(TestFilter::ODG);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Check glow properties
    sal_Int32 nGlowEffectRad = 0;
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"GlowEffectRadius"_ustr) >>= nGlowEffectRad);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(529), nGlowEffectRad); // 15 pt = 529.166... mm/100
    Color nGlowEffectColor;
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"GlowEffectColor"_ustr) >>= nGlowEffectColor);
    CPPUNIT_ASSERT_EQUAL(Color(0x00FF4000), nGlowEffectColor); // "Brick"
    sal_Int16 nGlowEffectTransparency = 0;
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"GlowEffectTransparency"_ustr)
                   >>= nGlowEffectTransparency);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(60), nGlowEffectTransparency); // 60%

    // Test ODF element
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    // check that we actually test graphic style
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[2]",
                "family", u"graphic");
    // check loext graphic attributes
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[2]/style:graphic-properties",
        "glow-radius", u"0.529cm");
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[2]/style:graphic-properties",
        "glow-color", u"#ff4000");
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[2]/style:graphic-properties",
        "glow-transparency", u"60%");
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testGlowTextInShape)
{
    createSdImpressDoc("odp/shape-text-glow-effect.odp");
    saveAndReload(TestFilter::ODP);

    uno::Reference<drawing::XDrawPage> xDP(getPage(0));
    uno::Reference<beans::XPropertySet> xShape(xDP->getByIndex(0), uno::UNO_QUERY);

    // Check glow text properties in shapes
    sal_Int32 nGlowEffectRad = 0;
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"GlowTextEffectRadius"_ustr) >>= nGlowEffectRad);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(706), nGlowEffectRad); // 20 pt = 706 mm/100
    Color nGlowEffectColor;
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"GlowTextEffectColor"_ustr) >>= nGlowEffectColor);
    CPPUNIT_ASSERT_EQUAL(Color(0x4EA72E), nGlowEffectColor);
    sal_Int16 nGlowEffectTransparency = 0;
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"GlowTextEffectTransparency"_ustr)
                   >>= nGlowEffectTransparency);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), nGlowEffectTransparency);

    // Test ODF element
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    // check that we actually test graphic style
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[3]",
                "family", u"graphic");
    // check loext text attributes for text in shapes
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[3]/style:text-properties",
        "glow-text-radius", u"0.706cm");
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[3]/style:text-properties",
        "glow-text-color", u"#4ea72e");
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[3]/style:text-properties",
        "glow-text-transparency", u"5%");
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testSoftEdges)
{
    createSdDrawDoc("odg/softedges.odg");
    saveAndReload(TestFilter::ODG);
    auto xShapeProps(getShapeFromPage(0, 0));

    // Check property
    sal_Int32 nRad = 0;
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"SoftEdgeRadius"_ustr) >>= nRad);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), nRad); // 18 pt

    // Test ODF element
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    // check that we actually test graphic style
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[2]",
                "family", u"graphic");
    // check loext graphic attribute
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[2]/style:graphic-properties",
        "softedge-radius", u"0.635cm");

    // Text frame object
    xShapeProps.set(getShapeFromPage(1, 0));
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"SoftEdgeRadius"_ustr) >>= nRad);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(353), nRad); // 10 pt

    // Test ODF element
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[3]",
                "family", u"graphic");
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[3]/style:graphic-properties",
        "softedge-radius", u"0.353cm");
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testShadowBlur)
{
    createSdImpressDoc("odp/shadow-blur.odp");
    saveAndReload(TestFilter::ODG);
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    sal_Int32 nRad = 0;
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"ShadowBlur"_ustr) >>= nRad);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRad); // 11 pt = 388 Hmm

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[3]",
                "family", u"graphic");
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[3]/style:graphic-properties",
        "shadow-blur", u"0.388cm");
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testRhbz1870501)
{
    //Without the fix in place, it would crash at export time
    createSdDrawDoc("odg/rhbz1870501.odg");
    saveAndReload(TestFilter::ODG);
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testTdf152606)
{
    //Without the fix in place, it would crash at import time
    createSdImpressDoc("pptx/tdf152606.pptx");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDrawPage->getCount());
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testTdf154754)
{
    // odfvalidator warns because we are missing a xlink:href attribute and it therefore concludes that
    // we must have a <office:binary-data> sub-element. Most likely the input file is invalid.
    skipValidation();

    //Without the fix in place, it would crash at export time
    createSdImpressDoc("odp/tdf154754.odp");
    saveAndReload(TestFilter::ODP);
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testTdf91060)
{
    //Without the fix in place, it would crash at import time
    createSdImpressDoc("pptx/tdf91060.pptx");
    saveAndReload(TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testTdf128550)
{
    createSdImpressDoc("pptx/tdf128550.pptx");
    save(TestFilter::ODP);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "//anim:iterate[@anim:sub-item='background']", 1);
    assertXPath(pXmlDoc, "//anim:iterate[@anim:sub-item='text']", 4);
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testTdf140714)
{
    //Without the fix in place, shape will be imported as GraphicObjectShape instead of CustomShape.

    createSdImpressDoc("pptx/tdf140714.pptx");
    saveAndReload(TestFilter::PPTX);

    uno::Reference<drawing::XShape> xShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.CustomShape"_ustr, xShape->getShapeType());
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testTdf156649)
{
    createSdImpressDoc("pptx/tdf156649.pptx");
    saveAndReload(TestFilter::PPTX);

    auto xShapeProps(getShapeFromPage(0, 0));
    // Without the fix in place, this test would have failed with
    //- Expected: 55
    //- Actual  : 0
    // i.e. alphaModFix wasn't imported as fill transparency for the custom shape
    CPPUNIT_ASSERT_EQUAL(sal_Int16(55),
                         xShapeProps->getPropertyValue(u"FillTransparence"_ustr).get<sal_Int16>());
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testMasterPageBackgroundFullSize)
{
    createSdImpressDoc("odp/background.odp");

    // BackgroundFullSize exists on master pages only
    // (note: this document can't be created with the UI because UI keeps
    //  page margins and the flag synchronized across all master pages)
    uno::Reference<drawing::XMasterPagesSupplier> xMPS(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xMPs(xMPS->getMasterPages());
    Color nFillColor;
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(!xMP->getPropertyValue(u"BackgroundFullSize"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderTop"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderLeft"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderBottom"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderRight"_ustr).get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue(u"Background"_ustr).get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_SOLID,
            xBackgroundProps->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>());
        xBackgroundProps->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), nFillColor);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0),
            xBackgroundProps->getPropertyValue(u"FillTransparence"_ustr).get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMP->getPropertyValue(u"BackgroundFullSize"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderTop"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderLeft"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderBottom"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderRight"_ustr).get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue(u"Background"_ustr).get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_SOLID,
            xBackgroundProps->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>());
        xBackgroundProps->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), nFillColor);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0),
            xBackgroundProps->getPropertyValue(u"FillTransparence"_ustr).get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT(!xMP->getPropertyValue(u"BackgroundFullSize"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderTop"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderLeft"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderBottom"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderRight"_ustr).get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue(u"Background"_ustr).get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_BITMAP,
            xBackgroundProps->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::BitmapMode_STRETCH,
            xBackgroundProps->getPropertyValue(u"FillBitmapMode"_ustr).get<drawing::BitmapMode>());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0),
            xBackgroundProps->getPropertyValue(u"FillTransparence"_ustr).get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMP->getPropertyValue(u"BackgroundFullSize"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderTop"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderLeft"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderBottom"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderRight"_ustr).get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue(u"Background"_ustr).get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_BITMAP,
            xBackgroundProps->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::BitmapMode_STRETCH,
            xBackgroundProps->getPropertyValue(u"FillBitmapMode"_ustr).get<drawing::BitmapMode>());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0),
            xBackgroundProps->getPropertyValue(u"FillTransparence"_ustr).get<sal_Int16>());
    }

    saveAndReload(TestFilter::ODP);

    xMPS.set(mxComponent, uno::UNO_QUERY);
    xMPs.set(xMPS->getMasterPages());
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(!xMP->getPropertyValue(u"BackgroundFullSize"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderTop"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderLeft"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderBottom"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderRight"_ustr).get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue(u"Background"_ustr).get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_SOLID,
            xBackgroundProps->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>());
        xBackgroundProps->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), nFillColor);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0),
            xBackgroundProps->getPropertyValue(u"FillTransparence"_ustr).get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMP->getPropertyValue(u"BackgroundFullSize"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderTop"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderLeft"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderBottom"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderRight"_ustr).get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue(u"Background"_ustr).get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_SOLID,
            xBackgroundProps->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>());
        xBackgroundProps->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), nFillColor);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0),
            xBackgroundProps->getPropertyValue(u"FillTransparence"_ustr).get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT(!xMP->getPropertyValue(u"BackgroundFullSize"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderTop"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderLeft"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderBottom"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderRight"_ustr).get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue(u"Background"_ustr).get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_BITMAP,
            xBackgroundProps->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::BitmapMode_STRETCH,
            xBackgroundProps->getPropertyValue(u"FillBitmapMode"_ustr).get<drawing::BitmapMode>());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0),
            xBackgroundProps->getPropertyValue(u"FillTransparence"_ustr).get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMP->getPropertyValue(u"BackgroundFullSize"_ustr).get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderTop"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000),
                             xMP->getPropertyValue(u"BorderLeft"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderBottom"_ustr).get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xMP->getPropertyValue(u"BorderRight"_ustr).get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue(u"Background"_ustr).get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_BITMAP,
            xBackgroundProps->getPropertyValue(u"FillStyle"_ustr).get<drawing::FillStyle>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::BitmapMode_STRETCH,
            xBackgroundProps->getPropertyValue(u"FillBitmapMode"_ustr).get<drawing::BitmapMode>());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0),
            xBackgroundProps->getPropertyValue(u"FillTransparence"_ustr).get<sal_Int16>());
    }

    xmlDocUniquePtr pXmlDoc = parseExport(u"styles.xml"_ustr);
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Default']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Default_20_3']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"full");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Default_20_2']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Default_20_1']/attribute::draw:style-name"
                "]/style:drawing-page-properties",
                "background-size", u"full");
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testColumnsODG)
{
    createSdDrawDoc("odg/two_columns.odg");

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPages> xPages = xDrawPagesSupplier->getDrawPages();
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue(u"TextColumns"_ustr),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(700)),
                             xColProps->getPropertyValue(u"AutomaticDistance"_ustr));

        auto pTextObj = DynCastSdrTextObj(SdrObject::getSdrObjectFromXShape(xShape));
        CPPUNIT_ASSERT(pTextObj);

        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pTextObj->GetTextColumnsNumber());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(700), pTextObj->GetTextColumnsSpacing());
    }

    saveAndReload(TestFilter::ODG);

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPages> xPages = xDrawPagesSupplier->getDrawPages();
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue(u"TextColumns"_ustr),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(700)),
                             xColProps->getPropertyValue(u"AutomaticDistance"_ustr));

        auto pTextObj = DynCastSdrTextObj(SdrObject::getSdrObjectFromXShape(xShape));
        CPPUNIT_ASSERT(pTextObj);

        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pTextObj->GetTextColumnsNumber());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(700), pTextObj->GetTextColumnsSpacing());
    }

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc,
                "/office:document-content/office:automatic-styles/style:style/"
                "style:graphic-properties/style:columns",
                "column-count", u"2");
    assertXPath(pXmlDoc,
                "/office:document-content/office:automatic-styles/style:style/"
                "style:graphic-properties/style:columns",
                "column-gap", u"0.7cm");
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testTdf112126)
{
    createSdDrawDoc("tdf112126.odg");
    saveAndReload(TestFilter::ODG);
    uno::Reference<drawing::XDrawPage> xPage(getPage(0));
    uno::Reference<beans::XPropertySet> xPropertySet(xPage, uno::UNO_QUERY);

    OUString xPageName;
    xPropertySet->getPropertyValue(u"LinkDisplayName"_ustr) >>= xPageName;

    // without the fix in place, it fails with
    // - Expected: Page 1
    // - Actual  : Slide 1
    CPPUNIT_ASSERT_EQUAL(u"Page 1"_ustr, xPageName);
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testCellProperties)
{
    // Silence unrelated failure:
    // Error: element "table:table-template" is missing "first-row-start-column" attribute
    // Looks like an oversight in the schema, as the docs claim this attribute is deprecated.
    skipValidation();

    createSdDrawDoc("odg/tablestyles.fodg");
    saveAndReload(TestFilter::ODG);

    const SdrPage* pPage = GetPage(1);
    auto pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj != nullptr);
    uno::Reference<beans::XPropertySet> xCell(pTableObj->getTable()->getCellByPosition(0, 0),
                                              uno::UNO_QUERY_THROW);

    Color nColor;
    table::BorderLine2 aBorderLine;
    drawing::TextVerticalAdjust aTextAdjust;
    sal_Int32 nPadding;

    xCell->getPropertyValue(u"FillColor"_ustr) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xffcc99), nColor);
    xCell->getPropertyValue(u"RightBorder"_ustr) >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(Color(0x99ccff), Color(ColorTransparency, aBorderLine.Color));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(159), aBorderLine.LineWidth);
    xCell->getPropertyValue(u"TextRightDistance"_ustr) >>= nPadding;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(300), nPadding);
    xCell->getPropertyValue(u"TextVerticalAdjust"_ustr) >>= aTextAdjust;
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust::TextVerticalAdjust_CENTER, aTextAdjust);
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testUserTableStyles)
{
    // Silence unrelated failure:
    // Error: element "table:table-template" is missing "first-row-start-column" attribute
    // Looks like an oversight in the schema, as the docs claim this attribute is deprecated.
    skipValidation();

    createSdDrawDoc("odg/tablestyles.fodg");
    saveAndReload(TestFilter::ODG);

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamiliesSupplier->getStyleFamilies()->getByName(u"table"_ustr), uno::UNO_QUERY);

    uno::Reference<style::XStyle> xTableStyle(xStyleFamily->getByName(u"default"_ustr),
                                              uno::UNO_QUERY);
    CPPUNIT_ASSERT(!xTableStyle->isUserDefined());

    uno::Reference<container::XNameAccess> xNameAccess(xTableStyle, uno::UNO_QUERY);
    uno::Reference<style::XStyle> xCellStyle(xNameAccess->getByName(u"first-row"_ustr),
                                             uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCellStyle);
    CPPUNIT_ASSERT_EQUAL(u"userdefined"_ustr, xCellStyle->getName());

    CPPUNIT_ASSERT(xStyleFamily->hasByName(u"userdefined"_ustr));
    xTableStyle.set(xStyleFamily->getByName(u"userdefined"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTableStyle->isUserDefined());
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testTdf153179)
{
    createSdImpressDoc("pptx/ole-emf_min.pptx");
    saveAndReload(TestFilter::ODP);

    // Check number of shapes after export.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getPage(0)->getCount());
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testSvgImageSupport)
{
    for (TestFilter eFormat : { TestFilter::ODP, TestFilter::PPTX })
    {
        // Load the original file
        createSdImpressDoc("odp/SvgImageTest.odp");
        // Save into the target format
        saveAndReload(eFormat);

        const OString sFailedMessage = "Failed on filter: " + TestFilterNames.at(eFormat).toUtf8();

        // Check whether SVG graphic was exported as expected
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(1),
                                     xDrawPagesSupplier->getDrawPages()->getCount());
        uno::Reference<drawing::XDrawPage> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xDrawPage.is());

        // Get the image
        uno::Reference<drawing::XShape> xImage(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xImage, uno::UNO_QUERY_THROW);

        // Convert to a XGraphic
        uno::Reference<graphic::XGraphic> xGraphic;
        xPropertySet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());

        // Access the Graphic
        Graphic aGraphic(xGraphic);

        // Check if it contains a VectorGraphicData struct
        auto pVectorGraphic = aGraphic.getVectorGraphicData();
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pVectorGraphic);

        // Which should be of type SVG, which means we have a SVG file
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), VectorGraphicDataType::Svg,
                                     pVectorGraphic->getType());
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest4, testTableBordersTransparancy)
{
    createSdImpressDoc("pptx/tdf164936.pptx");
    saveAndReload(TestFilter::PPTX);

    const SdrPage* pPage = GetPage(1);
    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCellPropSet;
    table::BorderLine2 aBorderLine;

    xCellPropSet.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCellPropSet->getPropertyValue(u"LeftBorder"_ustr) >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xff2670c9),
                         Color(ColorTransparency, aBorderLine.Color));
}

// CharOpticalSizing should be enabled by default for new documents
CPPUNIT_TEST_FIXTURE(SdExportTest4, testOpticalSizing1)
{
    createSdImpressDoc();

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->insertNewByIndex(0),
                                                 uno::UNO_SET_THROW);
    uno::Reference<drawing::XShapes> xShapes(xDrawPage, uno::UNO_QUERY_THROW);

    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.TextShape"_ustr), uno::UNO_QUERY_THROW);
    xShapes->add(xShape);

    uno::Reference<text::XText> xText(xShape, uno::UNO_QUERY_THROW);
    xText->setString(u"test"_ustr);

    {
        // It should be true by default for new documents
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xProps));
        uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
        uno::Reference<beans::XPropertySet> xRunProps(xRun, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(true,
                             xRunProps->getPropertyValue(u"CharOpticalSizing"_ustr).get<bool>());

        // and it should survive save-and-reload
        saveAndReload(TestFilter::ODP);
        uno::Reference<beans::XPropertySet> xProps2(getShapeFromPage(0, 1));
        uno::Reference<text::XTextRange> const xParagraph2(getParagraphFromShape(0, xProps2));
        uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(0, xParagraph2));
        uno::Reference<beans::XPropertySet> xRunProps2(xRun2, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(true,
                             xRunProps2->getPropertyValue(u"CharOpticalSizing"_ustr).get<bool>());
    }

    {
        // Setting it manually should set it in contents
        uno::Reference<beans::XPropertySet> xProps(getShapeFromPage(0, 1));
        uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xProps));
        uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
        uno::Reference<beans::XPropertySet> xRunProps(xRun, uno::UNO_QUERY_THROW);
        xRunProps->setPropertyValue(u"CharOpticalSizing"_ustr, uno::Any(true));
        save(TestFilter::ODP);
        xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
        assertXPath(pXmlDoc,
                    "//style:style/style:text-properties[@loext:font-optical-sizing='auto']", 1);

        // and it should survive save-and-reload
        saveAndReload(TestFilter::ODP);
        uno::Reference<beans::XPropertySet> xProps2(getShapeFromPage(0, 1));
        uno::Reference<text::XTextRange> const xParagraph2(getParagraphFromShape(0, xProps2));
        uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(0, xParagraph2));
        uno::Reference<beans::XPropertySet> xRunProps2(xRun2, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(true,
                             xRunProps2->getPropertyValue(u"CharOpticalSizing"_ustr).get<bool>());
        pXmlDoc = parseExport(u"content.xml"_ustr);
        assertXPath(pXmlDoc,
                    "//style:style/style:text-properties[@loext:font-optical-sizing='auto']", 1);
    }

    {
        // It can also be disabled
        uno::Reference<beans::XPropertySet> xProps(getShapeFromPage(0, 1));
        uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xProps));
        uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
        uno::Reference<beans::XPropertySet> xRunProps(xRun, uno::UNO_QUERY_THROW);
        xRunProps->setPropertyValue(u"CharOpticalSizing"_ustr, uno::Any(false));
        save(TestFilter::ODP);
        xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
        assertXPath(pXmlDoc,
                    "//style:style/style:text-properties[@loext:font-optical-sizing='none']", 1);

        // and it should survive save-and-reload
        saveAndReload(TestFilter::ODP);
        uno::Reference<beans::XPropertySet> xProps2(getShapeFromPage(0, 1));
        uno::Reference<text::XTextRange> const xParagraph2(getParagraphFromShape(0, xProps2));
        uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(0, xParagraph2));
        uno::Reference<beans::XPropertySet> xRunProps2(xRun2, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(false,
                             xRunProps2->getPropertyValue(u"CharOpticalSizing"_ustr).get<bool>());
        pXmlDoc = parseExport(u"content.xml"_ustr);
        assertXPath(pXmlDoc,
                    "//style:style/style:text-properties[@loext:font-optical-sizing='none']", 1);
    }
}

// CharOpticalSizing should be disabled by default for old documents
CPPUNIT_TEST_FIXTURE(SdExportTest4, testOpticalSizing2)
{
    createSdImpressDoc("odp/transparent_background.odp");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xRunProps(xRun, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(false, xRunProps->getPropertyValue(u"CharOpticalSizing"_ustr).get<bool>());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
