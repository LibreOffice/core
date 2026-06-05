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

class SdExportTest1 : public SdModelTestBase
{
public:
    SdExportTest1()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }

protected:
    uno::Reference<awt::XBitmap> getBitmapFromTable(OUString const& rName);
};

uno::Reference<awt::XBitmap> SdExportTest1::getBitmapFromTable(OUString const& rName)
{
    uno::Reference<awt::XBitmap> xBitmap;

    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);

    try
    {
        uno::Reference<container::XNameAccess> xBitmapTable(
            xFactory->createInstance(u"com.sun.star.drawing.BitmapTable"_ustr), uno::UNO_QUERY);
        uno::Any rValue = xBitmapTable->getByName(rName);
        if (rValue.has<uno::Reference<awt::XBitmap>>())
        {
            return rValue.get<uno::Reference<awt::XBitmap>>();
        }
    }
    catch (const uno::Exception& /*rEx*/)
    {
    }

    return xBitmap;
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testBackgroundImage)
{
    // Initial bug: N821567

    // Check if Slide background image is imported from PPTX and exported to PPTX, PPT and ODP correctly

    OUString bgImageName;
    createSdImpressDoc("pptx/n821567.pptx");

    // Check that imported background image from PPTX exists
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("not exactly one page", static_cast<sal_Int32>(1),
                                     xDoc->getDrawPages()->getCount());
        uno::Reference<drawing::XDrawPage> xPage(getPage(0));

        uno::Reference<beans::XPropertySet> xPropertySet(xPage, uno::UNO_QUERY);
        uno::Any aAny = xPropertySet->getPropertyValue(u"Background"_ustr);
        if (aAny.has<uno::Reference<beans::XPropertySet>>())
        {
            uno::Reference<beans::XPropertySet> xBackgroundPropSet;
            aAny >>= xBackgroundPropSet;
            aAny = xBackgroundPropSet->getPropertyValue(u"FillBitmapName"_ustr);
            aAny >>= bgImageName;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Slide Background is not imported from PPTX correctly",
                                     u"msFillBitmap 1"_ustr, bgImageName);

        uno::Reference<awt::XBitmap> xBitmap = getBitmapFromTable(bgImageName);
        CPPUNIT_ASSERT_MESSAGE("Slide Background Bitmap is missing when imported from PPTX",
                               xBitmap.is());
    }

    // Save as PPTX, reload and check again so we make sure exporting to PPTX is working correctly
    {
        saveAndReload(TestFilter::PPTX);
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("not exactly one page", static_cast<sal_Int32>(1),
                                     xDoc->getDrawPages()->getCount());
        uno::Reference<drawing::XDrawPage> xPage(getPage(0));

        uno::Reference<beans::XPropertySet> xPropertySet(xPage, uno::UNO_QUERY);
        uno::Any aAny = xPropertySet->getPropertyValue(u"Background"_ustr);
        if (aAny.hasValue())
        {
            uno::Reference<beans::XPropertySet> xBackgroundPropSet;
            aAny >>= xBackgroundPropSet;
            aAny = xBackgroundPropSet->getPropertyValue(u"FillBitmapName"_ustr);
            aAny >>= bgImageName;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Slide Background is not exported from PPTX correctly",
                                     u"msFillBitmap 1"_ustr, bgImageName);

        uno::Reference<awt::XBitmap> xBitmap = getBitmapFromTable(bgImageName);
        CPPUNIT_ASSERT_MESSAGE("Slide Background Bitmap is missing when exported from PPTX",
                               xBitmap.is());
    }

    // Save as ODP, reload and check again so we make sure exporting and importing to ODP is working correctly
    {
        saveAndReload(TestFilter::ODP);
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("not exactly one page", static_cast<sal_Int32>(1),
                                     xDoc->getDrawPages()->getCount());
        uno::Reference<drawing::XDrawPage> xPage(getPage(0));

        uno::Reference<beans::XPropertySet> xPropertySet(xPage, uno::UNO_QUERY);
        uno::Any aAny = xPropertySet->getPropertyValue(u"Background"_ustr);
        if (aAny.hasValue())
        {
            uno::Reference<beans::XPropertySet> xBackgroundPropSet;
            aAny >>= xBackgroundPropSet;
            aAny = xBackgroundPropSet->getPropertyValue(u"FillBitmapName"_ustr);
            aAny >>= bgImageName;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Slide Background is not exported or imported from ODP correctly",
            u"msFillBitmap 1"_ustr, bgImageName);

        uno::Reference<awt::XBitmap> xBitmap = getBitmapFromTable(bgImageName);
        CPPUNIT_ASSERT_MESSAGE(
            "Slide Background Bitmap is missing when exported or imported from ODP", xBitmap.is());
    }
}

namespace
{
template <typename ItemValue, typename ItemType>
void checkFontAttributes(const SdrTextObj* pObj, ItemValue nVal, sal_uInt32 nId)
{
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
    std::vector<EECharAttrib> rLst;
    aEdit.GetCharAttribs(0, rLst);
    for (std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it != rLst.rend(); ++it)
    {
        const ItemType* pAttrib = dynamic_cast<const ItemType*>((*it).pAttr);
        if (pAttrib && pAttrib->Which() == nId)
        {
            CPPUNIT_ASSERT_EQUAL(nVal, static_cast<ItemValue>(pAttrib->GetValue()));
        }
    }
}
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testTransparentBackground)
{
    createSdImpressDoc("odp/transparent_background.odp");
    saveAndReload(TestFilter::ODP);

    const SdrPage* pPage = GetPage(1);

    const SdrTextObj* pObj1 = DynCastSdrTextObj(pPage->GetObj(0));
    checkFontAttributes<Color, SvxColorItem>(pObj1, COL_TRANSPARENT, EE_CHAR_BKGCOLOR);

    const SdrTextObj* pObj2 = DynCastSdrTextObj(pPage->GetObj(1));
    checkFontAttributes<Color, SvxColorItem>(pObj2, COL_YELLOW, EE_CHAR_BKGCOLOR);
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testTdf162283)
{
    createSdImpressDoc("pptx/tdf162283.pptx");
    saveAndReload(TestFilter::PPT);

    uno::Reference<drawing::XDrawPage> xPage(getPage(0));
    uno::Reference<beans::XPropertySet> xPropSet(getShape(0, xPage));

    CPPUNIT_ASSERT(xPropSet.is());
    auto aGeomPropSeq = xPropSet->getPropertyValue(u"CustomShapeGeometry"_ustr)
                            .get<uno::Sequence<beans::PropertyValue>>();
    auto aGeomPropVec
        = comphelper::sequenceToContainer<std::vector<beans::PropertyValue>>(aGeomPropSeq);

    sal_Int32 nWidth = 0, nHeight = 0, nAdjValue = 0;

    auto aIt1 = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [](const beans::PropertyValue& rValue) { return rValue.Name == u"ViewBox"_ustr; });
    if (aIt1 != aGeomPropVec.end())
    {
        awt::Rectangle aViewBox;
        aIt1->Value >>= aViewBox;
        nWidth = aViewBox.Width;
        nHeight = aViewBox.Height;
    }

    auto aIt2 = std::find_if(
        aGeomPropVec.begin(), aGeomPropVec.end(),
        [](const beans::PropertyValue& rValue) { return rValue.Name == u"AdjustmentValues"_ustr; });
    if (aIt2 != aGeomPropVec.end())
    {
        uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustment;
        aIt2->Value >>= aAdjustment;
        aAdjustment[0].Value >>= nAdjValue;
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(21600), nWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21600), nHeight);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6688), nAdjValue);
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testDecorative)
{
    createSdImpressDoc("pptx/tdf141058-1.pptx");

    auto doTest = [this](OString const test) {
        ::std::set<OString> const decorative = {
            "Picture 6"_ostr,
            "Rectangle 7"_ostr,
            "Group 24"_ostr,
            "Connector: Elbow 9"_ostr,
            "Connector: Elbow 11"_ostr,
            "Connector: Elbow 14"_ostr,
            "Connector: Elbow 17"_ostr,
            "Straight Arrow Connector 21"_ostr,
            "Straight Arrow Connector 22"_ostr,
            "Straight Arrow Connector 23"_ostr,
        };

        uno::Reference<drawing::XDrawPage> const xPage(getPage(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(test.getStr(), sal_Int32(5), xPage->getCount());
        auto nDecorative(0);
        auto nShapes(0);
        auto nInnerDecorative(0);
        auto nInnerShapes(0);
        for (auto i = xPage->getCount(); i != 0; --i)
        {
            uno::Reference<beans::XPropertySet> xShape(getShape(i - 1, xPage));
            uno::Reference<container::XNamed> xNamed(xShape, uno::UNO_QUERY);
            OString const name(OUStringToOString(xNamed->getName(), RTL_TEXTENCODING_UTF8));
            if (decorative.find(name) != decorative.end())
            {
                CPPUNIT_ASSERT_MESSAGE(OString(test + name).getStr(),
                                       xShape->getPropertyValue(u"Decorative"_ustr).get<bool>());
                ++nDecorative;
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE(OString(test + name).getStr(),
                                       !xShape->getPropertyValue(u"Decorative"_ustr).get<bool>());
                ++nShapes;
            }
            uno::Reference<drawing::XShapes> const xShapes(xShape, uno::UNO_QUERY);
            if (xShapes.is())
            {
                for (auto j = xShapes->getCount(); j != 0; --j)
                {
                    uno::Reference<beans::XPropertySet> xInnerShape(xShapes->getByIndex(i - 1),
                                                                    uno::UNO_QUERY);
                    uno::Reference<container::XNamed> xInnerNamed(xInnerShape, uno::UNO_QUERY);
                    OString const innerName(
                        OUStringToOString(xInnerNamed->getName(), RTL_TEXTENCODING_UTF8));
                    if (decorative.find(innerName) != decorative.end())
                    {
                        CPPUNIT_ASSERT_MESSAGE(
                            OString(test + innerName).getStr(),
                            xInnerShape->getPropertyValue(u"Decorative"_ustr).get<bool>());
                        ++nInnerDecorative;
                    }
                    else
                    {
                        CPPUNIT_ASSERT_MESSAGE(
                            OString(test + innerName).getStr(),
                            !xInnerShape->getPropertyValue(u"Decorative"_ustr).get<bool>());
                        ++nInnerShapes;
                    }
                }
            }
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE(test.getStr(), static_cast<decltype(nDecorative)>(3),
                                     nDecorative);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(test.getStr(), static_cast<decltype(nShapes)>(2), nShapes);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(test.getStr(), static_cast<decltype(nInnerDecorative)>(7),
                                     nInnerDecorative);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(test.getStr(), static_cast<decltype(nInnerShapes)>(16),
                                     nInnerShapes);
    };

    doTest("initial pptx load: "_ostr);

    saveAndReload(TestFilter::PPTX);
    doTest("reload OOXML: "_ostr);

    saveAndReload(TestFilter::ODP);
    doTest("reload ODF: "_ostr);
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testTdf142716)
{
    createSdImpressDoc("pptx/tdf142716.pptx");
    saveAndReload(TestFilter::PPTX);

    const SdrPage* pPage = GetPage(1);
    const SdrTextObj* pObj = DynCastSdrTextObj(pPage->GetObj(0));

    OUString sText = pObj->GetOutlinerParaObject()->GetTextObject().GetText(0);

    // Without fix "yyy" part will be lost.
    CPPUNIT_ASSERT_EQUAL(u"xxx and yyy"_ustr, sText);
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testMediaEmbedding)
{
    createSdImpressDoc("media_embedding.odp");

    const SdrPage* pPage = GetPage(1);

    // Second object is a sound
    SdrMediaObj* pMediaObj = dynamic_cast<SdrMediaObj*>(pPage->GetObj(3));
    CPPUNIT_ASSERT_MESSAGE("missing media object", pMediaObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(u"vnd.sun.star.Package:Media/button-1.wav"_ustr,
                         pMediaObj->getMediaProperties().getURL());
    CPPUNIT_ASSERT_EQUAL(u"audio/x-wav"_ustr, pMediaObj->getMediaProperties().getMimeType());
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testFillBitmapUnused)
{
    createSdImpressDoc("odp/fillbitmap2.odp");
    save(TestFilter::ODP);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // shapes
    assertXPath(
        pXmlDoc,
        "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fill='bitmap']", 1);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fill='bitmap']",
        "fill-image-name", u"nav_5f_up");
    assertXPath(
        pXmlDoc,
        "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fill='solid']", 1);
    assertXPath(pXmlDoc,
                "//style:style[@style:family='graphic']/"
                "style:graphic-properties[@draw:fill='solid' and @draw:fill-image-name]",
                0);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fill='solid']",
        "fill-color", u"#808080");

    xmlDocUniquePtr pStyles = parseExport(u"styles.xml"_ustr);
    // master slide presentation style
    assertXPath(pStyles,
                "/office:document-styles/office:styles/style:style[@style:family='presentation' "
                "and @style:name='Default-background']/style:graphic-properties",
                "fill", u"bitmap");
    assertXPath(pStyles,
                "/office:document-styles/office:styles/style:style[@style:family='presentation' "
                "and @style:name='Default-background']/style:graphic-properties",
                "fill-image-name", u"nav_5f_up");
    assertXPath(pStyles,
                "/office:document-styles/office:styles/style:style[@style:family='presentation' "
                "and @style:name='Default_20_1-background']/style:graphic-properties",
                "fill", u"solid");
    assertXPath(pStyles,
                "/office:document-styles/office:styles/style:style[@style:family='presentation' "
                "and @style:name='Default_20_1-background']/style:graphic-properties",
                "fill-color", u"#808080");
    assertXPath(
        pStyles,
        "/office:document-styles/office:styles/style:style[@style:family='presentation' and "
        "@style:name='Default_20_1-background']/style:graphic-properties[@draw:fill-image-name]",
        0);
    // master slide drawing-page style
    assertXPath(pStyles,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp1']/"
                "style:drawing-page-properties",
                "fill", u"bitmap");
    assertXPath(pStyles,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp1']/"
                "style:drawing-page-properties",
                "fill-image-name", u"nav_5f_up");
    assertXPath(pStyles,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp2']/"
                "style:drawing-page-properties",
                "fill", u"solid");
    assertXPath(pStyles,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp2']/"
                "style:drawing-page-properties",
                "fill-color", u"#808080");
    assertXPath(pStyles,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp2']/"
                "style:drawing-page-properties[@draw:fill-image-name]",
                0);

    // the named items
    assertXPath(pStyles, "/office:document-styles/office:styles/draw:fill-image", 1);
    assertXPath(pStyles, "/office:document-styles/office:styles/draw:fill-image", "name",
                u"nav_5f_up");
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testFdo84043)
{
    createSdImpressDoc("fdo84043.odp");
    saveAndReload(TestFilter::ODP);

    // the bug was duplicate attributes, causing crash in a build with asserts
    const SdrPage* pPage = GetPage(1);
    SdrObject const* pShape = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testTdf97630)
{
    createSdImpressDoc("fit-to-size.fodp");
    {
        uno::Reference<drawing::XDrawPage> xDP(getPage(0));

        drawing::TextFitToSizeType tmp;
        // text shapes
        uno::Reference<beans::XPropertySet> xShape0(xDP->getByIndex(0), uno::UNO_QUERY);
        xShape0->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape1(xDP->getByIndex(1), uno::UNO_QUERY);
        xShape1->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
        uno::Reference<beans::XPropertySet> xShape2(xDP->getByIndex(2), uno::UNO_QUERY);
        xShape2->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_ALLLINES, tmp);
        uno::Reference<beans::XPropertySet> xShape3(xDP->getByIndex(3), uno::UNO_QUERY);
        xShape3->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_AUTOFIT, tmp);

        // fontworks
        uno::Reference<beans::XPropertySet> xShape4(xDP->getByIndex(4), uno::UNO_QUERY);
        xShape4->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape5(xDP->getByIndex(5), uno::UNO_QUERY);
        xShape5->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_ALLLINES, tmp);
    }

    saveAndReload(TestFilter::ODP);

    {
        uno::Reference<drawing::XDrawPage> xDP(getPage(0));

        drawing::TextFitToSizeType tmp;
        // text shapes
        uno::Reference<beans::XPropertySet> xShape0(xDP->getByIndex(0), uno::UNO_QUERY);
        xShape0->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape1(xDP->getByIndex(1), uno::UNO_QUERY);
        xShape1->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
        uno::Reference<beans::XPropertySet> xShape2(xDP->getByIndex(2), uno::UNO_QUERY);
        xShape2->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
        uno::Reference<beans::XPropertySet> xShape3(xDP->getByIndex(3), uno::UNO_QUERY);
        xShape3->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_AUTOFIT, tmp);

        // fontworks
        uno::Reference<beans::XPropertySet> xShape4(xDP->getByIndex(4), uno::UNO_QUERY);
        xShape4->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape5(xDP->getByIndex(5), uno::UNO_QUERY);
        xShape5->getPropertyValue(u"TextFitToSize"_ustr) >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
    }

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // text shapes
    assertXPath(pXmlDoc,
                "//style:style[@style:family='presentation']/"
                "style:graphic-properties[@draw:fit-to-size='false' and "
                "@style:shrink-to-fit='false']",
                1);
    assertXPath(pXmlDoc,
                "//style:style[@style:family='presentation']/"
                "style:graphic-properties[@draw:fit-to-size='true' and "
                "@style:shrink-to-fit='false']",
                2);
    assertXPath(pXmlDoc,
                "//style:style[@style:family='presentation']/"
                "style:graphic-properties[@draw:fit-to-size='false' and "
                "@style:shrink-to-fit='true']",
                1);
    // fontworks
    assertXPath(pXmlDoc,
                "//style:style[@style:family='graphic']/"
                "style:graphic-properties[@draw:fit-to-size='false' and "
                "@style:shrink-to-fit='false']",
                1);
    assertXPath(pXmlDoc,
                "//style:style[@style:family='graphic']/"
                "style:graphic-properties[@draw:fit-to-size='true' and "
                "@style:shrink-to-fit='false']",
                1);
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testImpressPasswordExport)
{
    std::vector<TestFilter> vFormat{ TestFilter::ODP, TestFilter::PPTX };

    for (size_t i = 0; i < vFormat.size(); i++)
    {
        createSdImpressDoc();

        saveAndReload(vFormat[i], /*pPassword*/ "test");
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest1, testDrawPasswordExport)
{
    createSdDrawDoc();

    saveAndReload(TestFilter::ODG, /*pPassword*/ "test");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
