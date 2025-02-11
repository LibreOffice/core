/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <officecfg/Office/Common.hxx>
#include "sdmodeltestbase.hxx"
#include <sdpage.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>

#include <svx/svdotext.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomedia.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
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

class SdExportTest : public SdModelTestBase
{
public:
    SdExportTest()
        : SdModelTestBase("/sd/qa/unit/data/")
    {
    }

protected:
    uno::Reference<awt::XBitmap> getBitmapFromTable(OUString const& rName);
};

uno::Reference<awt::XBitmap> SdExportTest::getBitmapFromTable(OUString const& rName)
{
    uno::Reference<awt::XBitmap> xBitmap;

    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);

    try
    {
        uno::Reference<container::XNameAccess> xBitmapTable(
            xFactory->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY);
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

CPPUNIT_TEST_FIXTURE(SdExportTest, testBackgroundImage)
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
        uno::Any aAny = xPropertySet->getPropertyValue("Background");
        if (aAny.has<uno::Reference<beans::XPropertySet>>())
        {
            uno::Reference<beans::XPropertySet> xBackgroundPropSet;
            aAny >>= xBackgroundPropSet;
            aAny = xBackgroundPropSet->getPropertyValue("FillBitmapName");
            aAny >>= bgImageName;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Slide Background is not imported from PPTX correctly",
                                     OUString("msFillBitmap 1"), bgImageName);

        uno::Reference<awt::XBitmap> xBitmap = getBitmapFromTable(bgImageName);
        CPPUNIT_ASSERT_MESSAGE("Slide Background Bitmap is missing when imported from PPTX",
                               xBitmap.is());
    }

    // Save as PPTX, reload and check again so we make sure exporting to PPTX is working correctly
    {
        saveAndReload("Impress Office Open XML");
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("not exactly one page", static_cast<sal_Int32>(1),
                                     xDoc->getDrawPages()->getCount());
        uno::Reference<drawing::XDrawPage> xPage(getPage(0));

        uno::Reference<beans::XPropertySet> xPropertySet(xPage, uno::UNO_QUERY);
        uno::Any aAny = xPropertySet->getPropertyValue("Background");
        if (aAny.hasValue())
        {
            uno::Reference<beans::XPropertySet> xBackgroundPropSet;
            aAny >>= xBackgroundPropSet;
            aAny = xBackgroundPropSet->getPropertyValue("FillBitmapName");
            aAny >>= bgImageName;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Slide Background is not exported from PPTX correctly",
                                     OUString("msFillBitmap 1"), bgImageName);

        uno::Reference<awt::XBitmap> xBitmap = getBitmapFromTable(bgImageName);
        CPPUNIT_ASSERT_MESSAGE("Slide Background Bitmap is missing when exported from PPTX",
                               xBitmap.is());
    }

    // Save as ODP, reload and check again so we make sure exporting and importing to ODP is working correctly
    {
        saveAndReload("impress8");
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("not exactly one page", static_cast<sal_Int32>(1),
                                     xDoc->getDrawPages()->getCount());
        uno::Reference<drawing::XDrawPage> xPage(getPage(0));

        uno::Reference<beans::XPropertySet> xPropertySet(xPage, uno::UNO_QUERY);
        uno::Any aAny = xPropertySet->getPropertyValue("Background");
        if (aAny.hasValue())
        {
            uno::Reference<beans::XPropertySet> xBackgroundPropSet;
            aAny >>= xBackgroundPropSet;
            aAny = xBackgroundPropSet->getPropertyValue("FillBitmapName");
            aAny >>= bgImageName;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Slide Background is not exported or imported from ODP correctly",
            OUString("msFillBitmap 1"), bgImageName);

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

CPPUNIT_TEST_FIXTURE(SdExportTest, testTransparentBackground)
{
    createSdImpressDoc("odp/transparent_background.odp");
    saveAndReload("impress8");

    const SdrPage* pPage = GetPage(1);

    const SdrTextObj* pObj1 = DynCastSdrTextObj(pPage->GetObj(0));
    checkFontAttributes<Color, SvxColorItem>(pObj1, COL_TRANSPARENT, EE_CHAR_BKGCOLOR);

    const SdrTextObj* pObj2 = DynCastSdrTextObj(pPage->GetObj(1));
    checkFontAttributes<Color, SvxColorItem>(pObj2, COL_YELLOW, EE_CHAR_BKGCOLOR);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testDecorative)
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
                                       xShape->getPropertyValue("Decorative").get<bool>());
                ++nDecorative;
            }
            else
            {
                CPPUNIT_ASSERT_MESSAGE(OString(test + name).getStr(),
                                       !xShape->getPropertyValue("Decorative").get<bool>());
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
                            xInnerShape->getPropertyValue("Decorative").get<bool>());
                        ++nInnerDecorative;
                    }
                    else
                    {
                        CPPUNIT_ASSERT_MESSAGE(
                            OString(test + innerName).getStr(),
                            !xInnerShape->getPropertyValue("Decorative").get<bool>());
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

    saveAndReload("Impress Office Open XML");
    doTest("reload OOXML: "_ostr);

    saveAndReload("impress8");
    doTest("reload ODF: "_ostr);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf142716)
{
    createSdImpressDoc("pptx/tdf142716.pptx");
    saveAndReload("Impress Office Open XML");

    const SdrPage* pPage = GetPage(1);
    const SdrTextObj* pObj = DynCastSdrTextObj(pPage->GetObj(0));

    OUString sText = pObj->GetOutlinerParaObject()->GetTextObject().GetText(0);

    // Without fix "yyy" part will be lost.
    CPPUNIT_ASSERT_EQUAL(OUString("xxx and yyy"), sText);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testMediaEmbedding)
{
    createSdImpressDoc("media_embedding.odp");

    const SdrPage* pPage = GetPage(1);

    // Second object is a sound
    SdrMediaObj* pMediaObj = dynamic_cast<SdrMediaObj*>(pPage->GetObj(3));
    CPPUNIT_ASSERT_MESSAGE("missing media object", pMediaObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(OUString("vnd.sun.star.Package:Media/button-1.wav"),
                         pMediaObj->getMediaProperties().getURL());
    CPPUNIT_ASSERT_EQUAL(OUString("audio/x-wav"), pMediaObj->getMediaProperties().getMimeType());
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testFillBitmapUnused)
{
    createSdImpressDoc("odp/fillbitmap2.odp");
    save("impress8");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // shapes
    assertXPath(
        pXmlDoc,
        "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fill='bitmap']"_ostr,
        1);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fill='bitmap']"_ostr,
        "fill-image-name"_ostr, "nav_5f_up");
    assertXPath(
        pXmlDoc,
        "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fill='solid']"_ostr,
        1);
    assertXPath(pXmlDoc,
                "//style:style[@style:family='graphic']/"
                "style:graphic-properties[@draw:fill='solid' and @draw:fill-image-name]"_ostr,
                0);
    assertXPath(
        pXmlDoc,
        "//style:style[@style:family='graphic']/style:graphic-properties[@draw:fill='solid']"_ostr,
        "fill-color"_ostr, "#808080");

    xmlDocUniquePtr pStyles = parseExport("styles.xml");
    // master slide presentation style
    assertXPath(pStyles,
                "/office:document-styles/office:styles/style:style[@style:family='presentation' "
                "and @style:name='Default-background']/style:graphic-properties"_ostr,
                "fill"_ostr, "bitmap");
    assertXPath(pStyles,
                "/office:document-styles/office:styles/style:style[@style:family='presentation' "
                "and @style:name='Default-background']/style:graphic-properties"_ostr,
                "fill-image-name"_ostr, "nav_5f_up");
    assertXPath(pStyles,
                "/office:document-styles/office:styles/style:style[@style:family='presentation' "
                "and @style:name='Default_20_1-background']/style:graphic-properties"_ostr,
                "fill"_ostr, "solid");
    assertXPath(pStyles,
                "/office:document-styles/office:styles/style:style[@style:family='presentation' "
                "and @style:name='Default_20_1-background']/style:graphic-properties"_ostr,
                "fill-color"_ostr, "#808080");
    assertXPath(
        pStyles,
        "/office:document-styles/office:styles/style:style[@style:family='presentation' and "
        "@style:name='Default_20_1-background']/style:graphic-properties[@draw:fill-image-name]"_ostr,
        0);
    // master slide drawing-page style
    assertXPath(pStyles,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp1']/"
                "style:drawing-page-properties"_ostr,
                "fill"_ostr, "bitmap");
    assertXPath(pStyles,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp1']/"
                "style:drawing-page-properties"_ostr,
                "fill-image-name"_ostr, "nav_5f_up");
    assertXPath(pStyles,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp2']/"
                "style:drawing-page-properties"_ostr,
                "fill"_ostr, "solid");
    assertXPath(pStyles,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp2']/"
                "style:drawing-page-properties"_ostr,
                "fill-color"_ostr, "#808080");
    assertXPath(pStyles,
                "/office:document-styles/office:automatic-styles/style:style[@style:name='Mdp2']/"
                "style:drawing-page-properties[@draw:fill-image-name]"_ostr,
                0);

    // the named items
    assertXPath(pStyles, "/office:document-styles/office:styles/draw:fill-image"_ostr, 1);
    assertXPath(pStyles, "/office:document-styles/office:styles/draw:fill-image"_ostr, "name"_ostr,
                "nav_5f_up");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testFdo84043)
{
    createSdImpressDoc("fdo84043.odp");
    saveAndReload("impress8");

    // the bug was duplicate attributes, causing crash in a build with asserts
    const SdrPage* pPage = GetPage(1);
    SdrObject const* pShape = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf97630)
{
    createSdImpressDoc("fit-to-size.fodp");
    {
        uno::Reference<drawing::XDrawPage> xDP(getPage(0));

        drawing::TextFitToSizeType tmp;
        // text shapes
        uno::Reference<beans::XPropertySet> xShape0(xDP->getByIndex(0), uno::UNO_QUERY);
        xShape0->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape1(xDP->getByIndex(1), uno::UNO_QUERY);
        xShape1->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
        uno::Reference<beans::XPropertySet> xShape2(xDP->getByIndex(2), uno::UNO_QUERY);
        xShape2->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_ALLLINES, tmp);
        uno::Reference<beans::XPropertySet> xShape3(xDP->getByIndex(3), uno::UNO_QUERY);
        xShape3->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_AUTOFIT, tmp);

        // fontworks
        uno::Reference<beans::XPropertySet> xShape4(xDP->getByIndex(4), uno::UNO_QUERY);
        xShape4->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape5(xDP->getByIndex(5), uno::UNO_QUERY);
        xShape5->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_ALLLINES, tmp);
    }

    saveAndReload("impress8");

    {
        uno::Reference<drawing::XDrawPage> xDP(getPage(0));

        drawing::TextFitToSizeType tmp;
        // text shapes
        uno::Reference<beans::XPropertySet> xShape0(xDP->getByIndex(0), uno::UNO_QUERY);
        xShape0->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape1(xDP->getByIndex(1), uno::UNO_QUERY);
        xShape1->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
        uno::Reference<beans::XPropertySet> xShape2(xDP->getByIndex(2), uno::UNO_QUERY);
        xShape2->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
        uno::Reference<beans::XPropertySet> xShape3(xDP->getByIndex(3), uno::UNO_QUERY);
        xShape3->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_AUTOFIT, tmp);

        // fontworks
        uno::Reference<beans::XPropertySet> xShape4(xDP->getByIndex(4), uno::UNO_QUERY);
        xShape4->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_NONE, tmp);
        uno::Reference<beans::XPropertySet> xShape5(xDP->getByIndex(5), uno::UNO_QUERY);
        xShape5->getPropertyValue("TextFitToSize") >>= tmp;
        CPPUNIT_ASSERT_EQUAL(drawing::TextFitToSizeType_PROPORTIONAL, tmp);
    }

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // text shapes
    assertXPath(pXmlDoc,
                "//style:style[@style:family='presentation']/"
                "style:graphic-properties[@draw:fit-to-size='false' and "
                "@style:shrink-to-fit='false']"_ostr,
                1);
    assertXPath(pXmlDoc,
                "//style:style[@style:family='presentation']/"
                "style:graphic-properties[@draw:fit-to-size='true' and "
                "@style:shrink-to-fit='false']"_ostr,
                2);
    assertXPath(pXmlDoc,
                "//style:style[@style:family='presentation']/"
                "style:graphic-properties[@draw:fit-to-size='false' and "
                "@style:shrink-to-fit='true']"_ostr,
                1);
    // fontworks
    assertXPath(pXmlDoc,
                "//style:style[@style:family='graphic']/"
                "style:graphic-properties[@draw:fit-to-size='false' and "
                "@style:shrink-to-fit='false']"_ostr,
                1);
    assertXPath(pXmlDoc,
                "//style:style[@style:family='graphic']/"
                "style:graphic-properties[@draw:fit-to-size='true' and "
                "@style:shrink-to-fit='false']"_ostr,
                1);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testImpressPasswordExport)
{
    std::vector<OUString> vFormat{ "impress8", "Impress Office Open XML" };

    for (size_t i = 0; i < vFormat.size(); i++)
    {
        createSdImpressDoc();

        saveAndReload(vFormat[i], /*pPassword*/ "test");
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testDrawPasswordExport)
{
    createSdDrawDoc();

    saveAndReload("draw8", /*pPassword*/ "test");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testSwappedOutImageExport)
{
    // Problem was with the swapped out images, which were not swapped in during export.

    std::vector<OUString> vFormat{ "impress8", "Impress Office Open XML", "MS PowerPoint 97" };

    for (size_t i = 0; i < vFormat.size(); i++)
    {
        // Load the original file with one image
        createSdImpressDoc("odp/document_with_two_images.odp");
        const OString sFailedMessage = "Failed on filter: " + vFormat[i].toUtf8();

        // Export the document and import again for a check
        saveAndReload(vFormat[i]);

        // Check whether graphic exported well after it was swapped out
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2),
                                     xDrawPagesSupplier->getDrawPages()->getCount());
        uno::Reference<drawing::XDrawPage> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);

        uno::Reference<drawing::XShape> xImage(xDrawPage->getByIndex(2), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> XPropSet(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381),
                                         xBitmap->getSize().Height);
        }

        // Second Image
        xDrawPage.set(xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW);
        xImage.set(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600),
                                         xBitmap->getSize().Height);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testOOoXMLAnimations)
{
    createSdImpressDoc("sxi/ooo41061-1.sxi");

    // FIXME: Error: unexpected attribute "presentation:preset-property"
    skipValidation();

    save("impress8");

    // the problem was that legacy OOoXML animations were lost if store
    // immediately follows load because they were "converted" async by a timer
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//anim:par[@presentation:node-type='timing-root']"_ostr, 26);
    // currently getting 52 of these without the fix (depends on timing)
    assertXPath(pXmlDoc, "//anim:par"_ostr, 223);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testBnc480256)
{
    createSdImpressDoc("pptx/bnc480256.pptx");
    // In the document, there are two tables with table background properties.
    // Make sure colors are set properly for individual cells.

    // TODO: If you are working on improving table background support, expect
    // this unit test to fail. In that case, feel free to change the numbers.

    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj;
    uno::Reference<table::XCellRange> xTable;
    uno::Reference<beans::XPropertySet> xCell;
    Color nColor;
    table::BorderLine2 aBorderLine;

    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x9bc3ee), nColor);
    xCell->getPropertyValue("LeftBorder") >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(Color(0x5597d3), Color(ColorTransparency, aBorderLine.Color));

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xc6ddff), nColor);
    xCell->getPropertyValue("TopBorder") >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(Color(0x5597d3), Color(ColorTransparency, aBorderLine.Color));

    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(1));
    CPPUNIT_ASSERT(pTableObj);
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x6bace6), nColor);
    xCell->getPropertyValue("LeftBorder") >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(Color(0xbecfe6), Color(ColorTransparency, aBorderLine.Color));

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x4697e0), nColor);

    // This border should be invisible.
    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("BottomBorder") >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), sal_Int32(aBorderLine.LineWidth));
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testUnknownAttributes)
{
    createSdImpressDoc("unknown-attribute.fodp");

    // FIXME: Error: unexpected attribute "foo:non-existent-att"
    skipValidation();

    save("impress8");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/"
                         "style:style[@style:name='gr1']/"
                         "style:graphic-properties[@foo:non-existent-att='bar']"_ostr);
    // TODO: if the namespace is *known*, the attribute is not preserved, but that seems to be a pre-existing problem, or maybe it's even intentional?
    //    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[@style:name='gr1']/style:graphic-properties[@svg:non-existent-att='blah']");
    // this was on style:graphic-properties on the import, but the export moves it to root node which is OK
    assertXPathNSDef(pXmlDoc, "/office:document-content"_ostr, u"foo", u"http://example.com/");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf80020)
{
    createSdImpressDoc("odp/tdf80020.odp");
    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                             uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies
            = xStyleFamiliesSupplier->getStyleFamilies();
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("graphics"),
                                                            uno::UNO_QUERY);
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName("Test Style"), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("text"), xStyle->getParentStyle());
        saveAndReload("impress8");
    }
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("graphics"),
                                                        uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName("Test Style"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("text"), xStyle->getParentStyle());
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf128985)
{
    createSdImpressDoc("odp/tdf128985.odp");
    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                             uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies
            = xStyleFamiliesSupplier->getStyleFamilies();
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("LushGreen"),
                                                            uno::UNO_QUERY);
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName("outline1"), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);

        sal_Int16 nWritingMode = 0;
        xPropSet->getPropertyValue("WritingMode") >>= nWritingMode;
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, nWritingMode);

        xPropSet->setPropertyValue("WritingMode", uno::Any(text::WritingMode2::LR_TB));

        saveAndReload("impress8");
    }
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("LushGreen"),
                                                        uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName("outline1"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xStyle, uno::UNO_QUERY);

    sal_Int16 nWritingMode = 0;
    xPropSet->getPropertyValue("WritingMode") >>= nWritingMode;

    // Without the fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, nWritingMode);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testLinkedGraphicRT)
{
    // FIXME: PPTX fails
    std::vector<OUString> vFormat{ "impress8", "MS PowerPoint 97" };
    for (size_t i = 0; i < vFormat.size(); i++)
    {
        // Load the original file with one image
        createSdImpressDoc("odp/document_with_linked_graphic.odp");

        // Check if the graphic has been imported correctly (before doing the export/import run)
        {
            constexpr OString sFailedImportMessage = "Failed to correctly import the document"_ostr;
            SdXImpressDocument* pXImpressDocument
                = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
            CPPUNIT_ASSERT(pXImpressDocument);
            SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
            CPPUNIT_ASSERT_MESSAGE(sFailedImportMessage.getStr(), pDoc != nullptr);
            const SdrPage* pPage = pDoc->GetPage(1);
            CPPUNIT_ASSERT_MESSAGE(sFailedImportMessage.getStr(), pPage != nullptr);
            SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(2));
            CPPUNIT_ASSERT_MESSAGE(sFailedImportMessage.getStr(), pObject != nullptr);
            CPPUNIT_ASSERT_MESSAGE(sFailedImportMessage.getStr(), pObject->IsLinkedGraphic());

            const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedImportMessage.getStr(), int(GraphicType::Bitmap),
                                         int(rGraphicObj.GetGraphic().GetType()));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedImportMessage.getStr(), sal_uLong(864900),
                                         rGraphicObj.GetGraphic().GetSizeBytes());
        }

        // Save and reload
        saveAndReload(vFormat[i]);

        // Check whether graphic imported well after export
        {
            const OString sFailedMessage = "Failed on filter: " + vFormat[i].toUtf8();

            SdXImpressDocument* pXImpressDocument
                = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
            CPPUNIT_ASSERT(pXImpressDocument);
            SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pDoc != nullptr);
            const SdrPage* pPage = pDoc->GetPage(1);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pPage != nullptr);
            SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(2));
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pObject != nullptr);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pObject->IsLinkedGraphic());

            const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), int(GraphicType::Bitmap),
                                         int(rGraphicObj.GetGraphic().GetType()));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_uLong(864900),
                                         rGraphicObj.GetGraphic().GetSizeBytes());
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf79082)
{
    createSdImpressDoc("ppt/tdf79082.ppt");
    save("impress8");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // P1 should have 6 tab stops defined
    assertXPathChildren(
        pXmlDoc, "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops"_ostr,
        6);
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[1]"_ostr,
                "position"_ostr, "0cm");
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[2]"_ostr,
                "position"_ostr, "5.08cm");
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[3]"_ostr,
                "position"_ostr, "10.16cm");
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[4]"_ostr,
                "position"_ostr, "15.24cm");
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[5]"_ostr,
                "position"_ostr, "20.32cm");
    assertXPath(pXmlDoc,
                "//style:style[@style:name='P1']/style:paragraph-properties/style:tab-stops/"
                "style:tab-stop[6]"_ostr,
                "position"_ostr, "25.4cm");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testImageWithSpecialID)
{
    // Check how LO handles when the imported graphic's ID is different from that one
    // which is generated by LO.

    std::vector<OUString> vFormat{ "impress8", "Impress Office Open XML", "MS PowerPoint 97" };
    for (size_t i = 0; i < vFormat.size(); i++)
    {
        // Load the original file
        createSdImpressDoc("odp/images_with_special_IDs.odp");
        const OString sFailedMessage = "Failed on filter: " + vFormat[i].toUtf8();
        saveAndReload(vFormat[i]);

        // Check whether graphic was exported well
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2),
                                     xDrawPagesSupplier->getDrawPages()->getCount());
        uno::Reference<drawing::XDrawPage> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);

        uno::Reference<drawing::XShape> xImage(xDrawPage->getByIndex(2), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> XPropSet(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381),
                                         xBitmap->getSize().Height);
        }

        // Second Image
        xDrawPage.set(xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW);
        xImage.set(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600),
                                         xBitmap->getSize().Height);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf62176)
{
    createSdImpressDoc("odp/Tdf62176.odp");
    uno::Reference<drawing::XDrawPage> xPage(getPage(0));

    //there should be only *one* shape
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage->getCount());

    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    //checking Paragraph's Left Margin with expected value
    sal_Int32 nParaLeftMargin = 0;
    xShape->getPropertyValue("ParaLeftMargin") >>= nParaLeftMargin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nParaLeftMargin);
    //checking Paragraph's First Line Indent with expected value
    sal_Int32 nParaFirstLineIndent = 0;
    xShape->getPropertyValue("ParaFirstLineIndent") >>= nParaFirstLineIndent;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1300), nParaFirstLineIndent);

    //Checking the *Text* in TextBox
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    CPPUNIT_ASSERT_EQUAL(OUString("Hello World"), xParagraph->getString());

    //Saving and Reloading the file
    saveAndReload("impress8");
    uno::Reference<drawing::XDrawPage> xPage2(getPage(0));
    //there should be only *one* shape
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage2->getCount());

    uno::Reference<beans::XPropertySet> xShape2(getShape(0, xPage2));
    //checking Paragraph's Left Margin with expected value
    sal_Int32 nParaLeftMargin2 = 0;
    xShape2->getPropertyValue("ParaLeftMargin") >>= nParaLeftMargin2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nParaLeftMargin2);
    //checking Paragraph's First Line Indent with expected value
    sal_Int32 nParaFirstLineIndent2 = 0;
    xShape2->getPropertyValue("ParaFirstLineIndent") >>= nParaFirstLineIndent2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1300), nParaFirstLineIndent2);

    //Checking the *Text* in TextBox
    uno::Reference<text::XTextRange> xParagraph2(getParagraphFromShape(0, xShape2));
    CPPUNIT_ASSERT_EQUAL(OUString("Hello World"), xParagraph2->getString());
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testEmbeddedPdf)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    createSdImpressDoc("odp/embedded-pdf.odp");
    saveAndReload("impress8");
    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("ReplacementGraphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testEmbeddedText)
{
    createSdDrawDoc("objectwithtext.fodg");
    saveAndReload("draw8");

    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xText.is());

    uno::Reference<container::XEnumerationAccess> xEA(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xEA->hasElements());
    uno::Reference<container::XEnumeration> xEnum(xEA->createEnumeration());
    uno::Reference<text::XTextContent> xTC;
    xEnum->nextElement() >>= xTC;
    CPPUNIT_ASSERT(xTC.is());

    uno::Reference<container::XEnumerationAccess> xParaEA(xTC, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEA->createEnumeration());
    uno::Reference<beans::XPropertySet> xPortion(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPortion.is());
    uno::Reference<text::XTextRange> xRange(xPortion, uno::UNO_QUERY);
    OUString type;
    xPortion->getPropertyValue("TextPortionType") >>= type;
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), type);
    CPPUNIT_ASSERT_EQUAL(OUString("foobar"), xRange->getString()); //tdf#112547
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTransparenText)
{
    createSdDrawDoc("transparent-text.fodg");
    saveAndReload("draw8");

    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nCharTransparence = 0;
    xShape->getPropertyValue("CharTransparence") >>= nCharTransparence;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 75
    // - Actual  : 0
    // i.e. the 75% transparent text was turned into a "not transparent at all" text.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(75), nCharTransparence);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testDefaultSubscripts)
{
    createSdDrawDoc("tdf80194_defaultSubscripts.fodg");
    saveAndReload("draw8");

    uno::Reference<drawing::XDrawPage> xPage = getPage(0);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(1), uno::UNO_QUERY);
    // Default subscripts were too large, enlarging the gap between the next line.
    // The exact size isn't important. Was 18975, now 16604.
    CPPUNIT_ASSERT(17000 > xShape->getSize().Height);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf98477)
{
    createSdImpressDoc("pptx/tdf98477grow.pptx");
    save("impress8");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//anim:animateTransform"_ostr, "by"_ostr, "0.5,0.5");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testAuthorField)
{
    createSdImpressDoc("odp/author_fixed.odp");

    saveAndReload("impress8");

    uno::Reference<text::XTextField> xField = getTextFieldFromPage(0, 0, 0, 0);
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is());

    uno::Reference<beans::XPropertySet> xPropSet(xField, uno::UNO_QUERY_THROW);
    bool bFixed = false;
    xPropSet->getPropertyValue("IsFixed") >>= bFixed;
    CPPUNIT_ASSERT_MESSAGE("Author field is not fixed", bFixed);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf50499)
{
    createSdImpressDoc("pptx/tdf50499.pptx");

    save("impress8");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//anim:animate[1]"_ostr, "from"_ostr, "(-width/2)");
    assertXPath(pXmlDoc, "//anim:animate[1]"_ostr, "to"_ostr, "(x)");
    assertXPath(pXmlDoc, "//anim:animate[3]"_ostr, "by"_ostr, "(height/3+width*0.1)");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf100926)
{
    createSdImpressDoc("pptx/tdf100926_ODP.pptx");

    saveAndReload("impress8");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT(pPage != nullptr);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj != nullptr);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    sal_Int32 nRotation = 0;
    uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0),
                                              uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("RotateAngle") >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27000), nRotation);

    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("RotateAngle") >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9000), nRotation);

    xCell.set(xTable->getCellByPosition(2, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("RotateAngle") >>= nRotation;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRotation);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testPageWithTransparentBackground)
{
    createSdImpressDoc("odp/page_transparent_background.odp");

    saveAndReload("impress8");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There should be exactly one page", static_cast<sal_Int32>(1),
                                 xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(getPage(0));

    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);
    uno::Any aAny = xPropSet->getPropertyValue("Background");
    CPPUNIT_ASSERT_MESSAGE("Slide background is missing", aAny.hasValue());

    uno::Reference<beans::XPropertySet> aXBackgroundPropSet;
    aAny >>= aXBackgroundPropSet;
    sal_Int32 nTransparence;
    aAny = aXBackgroundPropSet->getPropertyValue("FillTransparence");
    aAny >>= nTransparence;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Slide background transparency is wrong", sal_Int32(42),
                                 nTransparence);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTextRotation)
{
    // Save behavior depends on whether ODF strict or extended is used.
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        return pBatch->commit();
    });

    // The contained shape has a text rotation vert="vert" which corresponds to
    // loext:writing-mode="tb-rl90" in the graphic-properties of the style of the shape in ODF 1.3
    // extended.
    // Save to ODF 1.3 extended. Adapt 3 (=ODFVER_LATEST) to a to be ODFVER_013_EXTENDED when
    // attribute value "tb-rl90" is included in ODF strict.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(3, pBatch);
        pBatch->commit();

        createSdImpressDoc("pptx/shape-text-rotate.pptx");
        saveAndReload("impress8");

        uno::Reference<drawing::XDrawPage> xPage(getPage(0));
        uno::Reference<beans::XPropertySet> xPropSet(getShape(0, xPage));
        CPPUNIT_ASSERT(xPropSet.is());

        auto aWritingMode = xPropSet->getPropertyValue("WritingMode").get<sal_Int16>();
        CPPUNIT_ASSERT_EQUAL(sal_Int16(text::WritingMode2::TB_RL90), aWritingMode);
    }
    // In ODF 1.3 strict the workaround to use the TextRotateAngle is used instead.
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Save::ODF::DefaultVersion::set(10, pBatch);
        pBatch->commit();

        createSdImpressDoc("pptx/shape-text-rotate.pptx");
        saveAndReload("impress8");

        uno::Reference<drawing::XDrawPage> xPage(getPage(0));
        uno::Reference<beans::XPropertySet> xPropSet(getShape(0, xPage));

        CPPUNIT_ASSERT(xPropSet.is());
        auto aGeomPropSeq = xPropSet->getPropertyValue("CustomShapeGeometry")
                                .get<uno::Sequence<beans::PropertyValue>>();
        comphelper::SequenceAsHashMap aCustomShapeGeometry(aGeomPropSeq);

        auto it = aCustomShapeGeometry.find("TextRotateAngle");
        CPPUNIT_ASSERT(it != aCustomShapeGeometry.end());

        CPPUNIT_ASSERT_EQUAL(double(-90), aCustomShapeGeometry["TextRotateAngle"].get<double>());
    }
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf115394PPT)
{
    createSdImpressDoc("ppt/tdf115394.ppt");

    // Export the document and import again for a check
    saveAndReload("MS PowerPoint 97");

    double fTransitionDuration;

    // Fast
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    SdPage* pPage1 = pDoc->GetSdPage(0, PageKind::Standard);
    fTransitionDuration = pPage1->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.5, fTransitionDuration);

    // Medium
    SdPage* pPage2 = pDoc->GetSdPage(1, PageKind::Standard);
    fTransitionDuration = pPage2->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(0.75, fTransitionDuration);

    // Slow
    SdPage* pPage3 = pDoc->GetSdPage(2, PageKind::Standard);
    fTransitionDuration = pPage3->getTransitionDuration();
    CPPUNIT_ASSERT_EQUAL(1.0, fTransitionDuration);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testBulletsAsImageImpress8)
{
    createSdImpressDoc("odp/BulletsAsImage.odp");
    saveAndReload("impress8");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xLevels(xPropSet->getPropertyValue("NumberingRules"),
                                                    uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aProperties;
    xLevels->getByIndex(0) >>= aProperties; // 1st level

    uno::Reference<awt::XBitmap> xBitmap;
    awt::Size aSize;
    sal_Int16 nNumberingType = -1;

    for (beans::PropertyValue const& rProperty : std::as_const(aProperties))
    {
        if (rProperty.Name == "NumberingType")
        {
            nNumberingType = rProperty.Value.get<sal_Int16>();
        }
        else if (rProperty.Name == "GraphicBitmap")
        {
            xBitmap = rProperty.Value.get<uno::Reference<awt::XBitmap>>();
        }
        else if (rProperty.Name == "GraphicSize")
        {
            aSize = rProperty.Value.get<awt::Size>();
        }
    }

    CPPUNIT_ASSERT_EQUAL(style::NumberingType::BITMAP, nNumberingType);

    // Graphic Bitmap
    CPPUNIT_ASSERT_MESSAGE("No bitmap for the bullets", xBitmap.is());
    Graphic aGraphic(uno::Reference<graphic::XGraphic>(xBitmap, uno::UNO_QUERY));
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > o3tl::make_unsigned(0));

    CPPUNIT_ASSERT_EQUAL(tools::Long(16), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(16), aGraphic.GetSizePixel().Height());

    // Graphic Size
    CPPUNIT_ASSERT_EQUAL(sal_Int32(500), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(500), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testBulletsAsImageImpressOfficeOpenXml)
{
    createSdImpressDoc("odp/BulletsAsImage.odp");
    saveAndReload("Impress Office Open XML");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xLevels(xPropSet->getPropertyValue("NumberingRules"),
                                                    uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aProperties;
    xLevels->getByIndex(0) >>= aProperties; // 1st level

    uno::Reference<awt::XBitmap> xBitmap;
    awt::Size aSize;
    sal_Int16 nNumberingType = -1;

    for (beans::PropertyValue const& rProperty : std::as_const(aProperties))
    {
        if (rProperty.Name == "NumberingType")
        {
            nNumberingType = rProperty.Value.get<sal_Int16>();
        }
        else if (rProperty.Name == "GraphicBitmap")
        {
            xBitmap = rProperty.Value.get<uno::Reference<awt::XBitmap>>();
        }
        else if (rProperty.Name == "GraphicSize")
        {
            aSize = rProperty.Value.get<awt::Size>();
        }
    }

    CPPUNIT_ASSERT_EQUAL(style::NumberingType::BITMAP, nNumberingType);

    // Graphic Bitmap
    CPPUNIT_ASSERT_MESSAGE("No bitmap for the bullets", xBitmap.is());
    Graphic aGraphic(uno::Reference<graphic::XGraphic>(xBitmap, uno::UNO_QUERY));
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > o3tl::make_unsigned(0));

    // FIXME: what happened here
    CPPUNIT_ASSERT_EQUAL(tools::Long(64), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(64), aGraphic.GetSizePixel().Height());

    // Graphic Size
    // FIXME: totally wrong
    CPPUNIT_ASSERT_EQUAL(sal_Int32(790), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(790), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testBulletsAsImageMsPowerpoint97)
{
    createSdImpressDoc("odp/BulletsAsImage.odp");
    saveAndReload("MS PowerPoint 97");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xLevels(xPropSet->getPropertyValue("NumberingRules"),
                                                    uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aProperties;
    xLevels->getByIndex(0) >>= aProperties; // 1st level

    uno::Reference<awt::XBitmap> xBitmap;
    awt::Size aSize;
    sal_Int16 nNumberingType = -1;

    for (beans::PropertyValue const& rProperty : std::as_const(aProperties))
    {
        if (rProperty.Name == "NumberingType")
        {
            nNumberingType = rProperty.Value.get<sal_Int16>();
        }
        else if (rProperty.Name == "GraphicBitmap")
        {
            xBitmap = rProperty.Value.get<uno::Reference<awt::XBitmap>>();
        }
        else if (rProperty.Name == "GraphicSize")
        {
            aSize = rProperty.Value.get<awt::Size>();
        }
    }

    CPPUNIT_ASSERT_EQUAL(style::NumberingType::BITMAP, nNumberingType);

    // Graphic Bitmap
    CPPUNIT_ASSERT_MESSAGE("No bitmap for the bullets", xBitmap.is());
    Graphic aGraphic(uno::Reference<graphic::XGraphic>(xBitmap, uno::UNO_QUERY));
    CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT(aGraphic.GetSizeBytes() > o3tl::make_unsigned(0));

    CPPUNIT_ASSERT_EQUAL(tools::Long(16), aGraphic.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(16), aGraphic.GetSizePixel().Height());

    // Graphic Size
    // seems like a conversion error
    CPPUNIT_ASSERT_EQUAL(sal_Int32(504), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(504), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf113822)
{
    createSdImpressDoc("pptx/tdf113822underline.pptx");

    // Was unable to export iterate container (tdf#99213).
    saveAndReload("Impress Office Open XML");
    // Was unable to import iterate container (tdf#113822).
    save("impress8");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // IterateContainer was created as ParallelTimeContainer before, so
    // the iterate type is not set too.
    assertXPath(pXmlDoc, "//anim:iterate"_ostr, "iterate-type"_ostr, "by-letter");
    // The target of the child animation nodes need to be in the iterate container.
    assertXPath(pXmlDoc, "//anim:iterate"_ostr, "targetElement"_ostr, "id1");
    assertXPath(pXmlDoc, "//anim:iterate/anim:set"_ostr, "attributeName"_ostr, "text-underline");
    assertXPath(pXmlDoc, "//anim:iterate/anim:set"_ostr, "to"_ostr, "solid");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf113818)
{
    createSdImpressDoc("pptx/tdf113818-swivel.pptx");
    saveAndReload("MS PowerPoint 97");
    saveAndReload("Impress Office Open XML");
    save("impress8");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//anim:animate[1]"_ostr, "formula"_ostr, "width*sin(2.5*pi*$)");
    assertXPath(pXmlDoc, "//anim:animate[1]"_ostr, "values"_ostr, "0;1");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf119629)
{
    createSdImpressDoc("ppt/tdf119629.ppt");
    saveAndReload("MS PowerPoint 97");
    save("impress8");

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // MSO's effect node type Click parallel node, with group node, after group node
    // were missing.
    assertXPath(pXmlDoc,
                "//draw:page"
                "/anim:par[@presentation:node-type='timing-root']"
                "/anim:seq[@presentation:node-type='main-sequence']"
                "/anim:par[@presentation:node-type='on-click']"
                "/anim:par[@presentation:node-type='with-previous']"
                "/anim:par[@presentation:node-type='on-click']"
                "/anim:animate[@anim:formula='width*sin(2.5*pi*$)']"_ostr,
                1);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf141269)
{
    createSdImpressDoc("odp/tdf141269.odp");
    saveAndReload("MS PowerPoint 97");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<graphic::XGraphic> xGraphic;
    xShape->getPropertyValue("Graphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    Graphic aGraphic(xGraphic);
    BitmapEx aBitmap(aGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1920), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1080), aBitmap.GetSizePixel().Height());

    Color aExpectedColor(0xC2DEEA);
    aExpectedColor.SetAlpha(0xF);

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:194 G:222 B:234 A:240
    // - Actual  : Color: R:194 G:222 B:234 A:15
    CPPUNIT_ASSERT_EQUAL(aExpectedColor, aBitmap.GetPixelColor(960, 540));
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf123557)
{
    createSdImpressDoc("pptx/trigger.pptx");
    saveAndReload("Impress Office Open XML");
    save("impress8");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // Contains 2 interactive sequences and 3 triggered effects.
    assertXPath(pXmlDoc, "//draw:page"_ostr, 1);
    assertXPath(pXmlDoc, "//draw:page/anim:par"_ostr, 1);
    assertXPath(pXmlDoc,
                "//draw:page"
                "/anim:par[@presentation:node-type='timing-root']"
                "/anim:seq[@presentation:node-type='interactive-sequence']"_ostr,
                2);
    assertXPath(pXmlDoc,
                "//draw:page"
                "/anim:par[@presentation:node-type='timing-root']"
                "/anim:seq[@presentation:node-type='interactive-sequence']"
                "/anim:par[@smil:begin]"_ostr,
                3);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf126761)
{
    createSdImpressDoc("ppt/tdf126761.ppt");
    saveAndReload("impress8");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    // Check character underline, to make sure it has been set correctly
    sal_uInt32 nCharUnderline;
    xPropSet->getPropertyValue("CharUnderline") >>= nCharUnderline;
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), nCharUnderline);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testGlow)
{
    createSdDrawDoc("odg/glow.odg");
    saveAndReload("draw8");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Check glow properties
    sal_Int32 nGlowEffectRad = 0;
    CPPUNIT_ASSERT(xShape->getPropertyValue("GlowEffectRadius") >>= nGlowEffectRad);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(529), nGlowEffectRad); // 15 pt = 529.166... mm/100
    Color nGlowEffectColor;
    CPPUNIT_ASSERT(xShape->getPropertyValue("GlowEffectColor") >>= nGlowEffectColor);
    CPPUNIT_ASSERT_EQUAL(Color(0x00FF4000), nGlowEffectColor); // "Brick"
    sal_Int16 nGlowEffectTransparency = 0;
    CPPUNIT_ASSERT(xShape->getPropertyValue("GlowEffectTransparency") >>= nGlowEffectTransparency);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(60), nGlowEffectTransparency); // 60%

    // Test ODF element
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // check that we actually test graphic style
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[2]"_ostr,
                "family"_ostr, "graphic");
    // check loext graphic attributes
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[2]/style:graphic-properties"_ostr,
        "glow-radius"_ostr, "0.529cm");
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[2]/style:graphic-properties"_ostr,
        "glow-color"_ostr, "#ff4000");
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[2]/style:graphic-properties"_ostr,
        "glow-transparency"_ostr, "60%");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testSoftEdges)
{
    createSdDrawDoc("odg/softedges.odg");
    saveAndReload("draw8");
    auto xShapeProps(getShapeFromPage(0, 0));

    // Check property
    sal_Int32 nRad = 0;
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue("SoftEdgeRadius") >>= nRad);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), nRad); // 18 pt

    // Test ODF element
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // check that we actually test graphic style
    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[2]"_ostr,
                "family"_ostr, "graphic");
    // check loext graphic attribute
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[2]/style:graphic-properties"_ostr,
        "softedge-radius"_ostr, "0.635cm");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testShadowBlur)
{
    createSdImpressDoc("odp/shadow-blur.odp");
    saveAndReload("draw8");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    sal_Int32 nRad = 0;
    CPPUNIT_ASSERT(xShape->getPropertyValue("ShadowBlur") >>= nRad);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(388), nRad); // 11 pt = 388 Hmm

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    assertXPath(pXmlDoc, "/office:document-content/office:automatic-styles/style:style[3]"_ostr,
                "family"_ostr, "graphic");
    assertXPath(
        pXmlDoc,
        "/office:document-content/office:automatic-styles/style:style[3]/style:graphic-properties"_ostr,
        "shadow-blur"_ostr, "0.388cm");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testRhbz1870501)
{
    //Without the fix in place, it would crash at export time
    createSdDrawDoc("odg/rhbz1870501.odg");
    saveAndReload("draw8");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf152606)
{
    //Without the fix in place, it would crash at import time
    createSdImpressDoc("pptx/tdf152606.pptx");
    saveAndReload("Impress Office Open XML");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xDrawPage->getCount());
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf154754)
{
    //Without the fix in place, it would crash at export time
    skipValidation();
    createSdImpressDoc("odp/tdf154754.odp");
    saveAndReload("impress8");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf91060)
{
    //Without the fix in place, it would crash at import time
    createSdImpressDoc("pptx/tdf91060.pptx");
    saveAndReload("Impress Office Open XML");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf128550)
{
    createSdImpressDoc("pptx/tdf128550.pptx");
    save("impress8");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc, "//anim:iterate[@anim:sub-item='background']"_ostr, 1);
    assertXPath(pXmlDoc, "//anim:iterate[@anim:sub-item='text']"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf140714)
{
    //Without the fix in place, shape will be imported as GraphicObjectShape instead of CustomShape.

    createSdImpressDoc("pptx/tdf140714.pptx");
    saveAndReload("Impress Office Open XML");

    uno::Reference<drawing::XShape> xShape(getShapeFromPage(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString{ "com.sun.star.drawing.CustomShape" }, xShape->getShapeType());
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf156649)
{
    createSdImpressDoc("pptx/tdf156649.pptx");
    saveAndReload("Impress Office Open XML");

    auto xShapeProps(getShapeFromPage(0, 0));
    // Without the fix in place, this test would have failed with
    //- Expected: 55
    //- Actual  : 0
    // i.e. alphaModFix wasn't imported as fill transparency for the custom shape
    CPPUNIT_ASSERT_EQUAL(sal_Int16(55),
                         xShapeProps->getPropertyValue("FillTransparence").get<sal_Int16>());
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testMasterPageBackgroundFullSize)
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
        CPPUNIT_ASSERT(!xMP->getPropertyValue("BackgroundFullSize").get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderTop").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderLeft").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderBottom").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderRight").get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue("Background").get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_SOLID,
            xBackgroundProps->getPropertyValue("FillStyle").get<drawing::FillStyle>());
        xBackgroundProps->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), nFillColor);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0), xBackgroundProps->getPropertyValue("FillTransparence").get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMP->getPropertyValue("BackgroundFullSize").get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderTop").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderLeft").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderBottom").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderRight").get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue("Background").get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_SOLID,
            xBackgroundProps->getPropertyValue("FillStyle").get<drawing::FillStyle>());
        xBackgroundProps->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), nFillColor);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0), xBackgroundProps->getPropertyValue("FillTransparence").get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT(!xMP->getPropertyValue("BackgroundFullSize").get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderTop").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderLeft").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderBottom").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderRight").get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue("Background").get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_BITMAP,
            xBackgroundProps->getPropertyValue("FillStyle").get<drawing::FillStyle>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::BitmapMode_STRETCH,
            xBackgroundProps->getPropertyValue("FillBitmapMode").get<drawing::BitmapMode>());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0), xBackgroundProps->getPropertyValue("FillTransparence").get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMP->getPropertyValue("BackgroundFullSize").get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderTop").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderLeft").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderBottom").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderRight").get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue("Background").get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_BITMAP,
            xBackgroundProps->getPropertyValue("FillStyle").get<drawing::FillStyle>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::BitmapMode_STRETCH,
            xBackgroundProps->getPropertyValue("FillBitmapMode").get<drawing::BitmapMode>());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0), xBackgroundProps->getPropertyValue("FillTransparence").get<sal_Int16>());
    }

    saveAndReload("impress8");

    xMPS.set(mxComponent, uno::UNO_QUERY);
    xMPs.set(xMPS->getMasterPages());
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(!xMP->getPropertyValue("BackgroundFullSize").get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderTop").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderLeft").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderBottom").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderRight").get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue("Background").get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_SOLID,
            xBackgroundProps->getPropertyValue("FillStyle").get<drawing::FillStyle>());
        xBackgroundProps->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), nFillColor);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0), xBackgroundProps->getPropertyValue("FillTransparence").get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMP->getPropertyValue("BackgroundFullSize").get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderTop").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderLeft").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderBottom").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderRight").get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue("Background").get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_SOLID,
            xBackgroundProps->getPropertyValue("FillStyle").get<drawing::FillStyle>());
        xBackgroundProps->getPropertyValue("FillColor") >>= nFillColor;
        CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), nFillColor);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0), xBackgroundProps->getPropertyValue("FillTransparence").get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT(!xMP->getPropertyValue("BackgroundFullSize").get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderTop").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderLeft").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderBottom").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderRight").get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue("Background").get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_BITMAP,
            xBackgroundProps->getPropertyValue("FillStyle").get<drawing::FillStyle>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::BitmapMode_STRETCH,
            xBackgroundProps->getPropertyValue("FillBitmapMode").get<drawing::BitmapMode>());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0), xBackgroundProps->getPropertyValue("FillTransparence").get<sal_Int16>());
    }
    {
        uno::Reference<beans::XPropertySet> xMP(xMPs->getByIndex(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMP->getPropertyValue("BackgroundFullSize").get<bool>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderTop").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), xMP->getPropertyValue("BorderLeft").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderBottom").get<sal_Int32>());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xMP->getPropertyValue("BorderRight").get<sal_Int32>());
        uno::Reference<beans::XPropertySet> xBackgroundProps(
            xMP->getPropertyValue("Background").get<uno::Reference<beans::XPropertySet>>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::FillStyle_BITMAP,
            xBackgroundProps->getPropertyValue("FillStyle").get<drawing::FillStyle>());
        CPPUNIT_ASSERT_EQUAL(
            drawing::BitmapMode_STRETCH,
            xBackgroundProps->getPropertyValue("FillBitmapMode").get<drawing::BitmapMode>());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(0), xBackgroundProps->getPropertyValue("FillTransparence").get<sal_Int16>());
    }

    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Default']/attribute::draw:style-name"
                "]/style:drawing-page-properties"_ostr,
                "background-size"_ostr, "border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Default_20_3']/attribute::draw:style-name"
                "]/style:drawing-page-properties"_ostr,
                "background-size"_ostr, "full");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Default_20_2']/attribute::draw:style-name"
                "]/style:drawing-page-properties"_ostr,
                "background-size"_ostr, "border");
    assertXPath(pXmlDoc,
                "/office:document-styles/office:automatic-styles/"
                "style:style[@style:family='drawing-page' and @style:name = "
                "/office:document-styles/office:master-styles/"
                "style:master-page[@style:name='Default_20_1']/attribute::draw:style-name"
                "]/style:drawing-page-properties"_ostr,
                "background-size"_ostr, "full");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testColumnsODG)
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
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue("TextColumns"),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(700)),
                             xColProps->getPropertyValue("AutomaticDistance"));

        auto pTextObj = DynCastSdrTextObj(SdrObject::getSdrObjectFromXShape(xShape));
        CPPUNIT_ASSERT(pTextObj);

        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pTextObj->GetTextColumnsNumber());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(700), pTextObj->GetTextColumnsSpacing());
    }

    saveAndReload("draw8");

    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPages> xPages = xDrawPagesSupplier->getDrawPages();
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndexAccess(xPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xShape, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextColumns> xCols(xProps->getPropertyValue("TextColumns"),
                                                 uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCols->getColumnCount());
        uno::Reference<beans::XPropertySet> xColProps(xCols, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(sal_Int32(700)),
                             xColProps->getPropertyValue("AutomaticDistance"));

        auto pTextObj = DynCastSdrTextObj(SdrObject::getSdrObjectFromXShape(xShape));
        CPPUNIT_ASSERT(pTextObj);

        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pTextObj->GetTextColumnsNumber());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(700), pTextObj->GetTextColumnsSpacing());
    }

    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    assertXPath(pXmlDoc,
                "/office:document-content/office:automatic-styles/style:style/"
                "style:graphic-properties/style:columns"_ostr,
                "column-count"_ostr, "2");
    assertXPath(pXmlDoc,
                "/office:document-content/office:automatic-styles/style:style/"
                "style:graphic-properties/style:columns"_ostr,
                "column-gap"_ostr, "0.7cm");
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf112126)
{
    createSdDrawDoc("tdf112126.odg");
    saveAndReload("draw8");
    uno::Reference<drawing::XDrawPage> xPage(getPage(0));
    uno::Reference<beans::XPropertySet> xPropertySet(xPage, uno::UNO_QUERY);

    OUString xPageName;
    xPropertySet->getPropertyValue("LinkDisplayName") >>= xPageName;

    // without the fix in place, it fails with
    // - Expected: Page 1
    // - Actual  : Slide 1
    CPPUNIT_ASSERT_EQUAL(OUString("Page 1"), xPageName);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testCellProperties)
{
    // Silence unrelated failure:
    // Error: element "table:table-template" is missing "first-row-start-column" attribute
    // Looks like an oversight in the schema, as the docs claim this attribute is deprecated.
    skipValidation();

    createSdDrawDoc("odg/tablestyles.fodg");
    saveAndReload("draw8");

    const SdrPage* pPage = GetPage(1);
    auto pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj != nullptr);
    uno::Reference<beans::XPropertySet> xCell(pTableObj->getTable()->getCellByPosition(0, 0),
                                              uno::UNO_QUERY_THROW);

    Color nColor;
    table::BorderLine2 aBorderLine;
    drawing::TextVerticalAdjust aTextAdjust;
    sal_Int32 nPadding;

    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xffcc99), nColor);
    xCell->getPropertyValue("RightBorder") >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(Color(0x99ccff), Color(ColorTransparency, aBorderLine.Color));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(159), aBorderLine.LineWidth);
    xCell->getPropertyValue("TextRightDistance") >>= nPadding;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(300), nPadding);
    xCell->getPropertyValue("TextVerticalAdjust") >>= aTextAdjust;
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust::TextVerticalAdjust_CENTER, aTextAdjust);
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testUserTableStyles)
{
    // Silence unrelated failure:
    // Error: element "table:table-template" is missing "first-row-start-column" attribute
    // Looks like an oversight in the schema, as the docs claim this attribute is deprecated.
    skipValidation();

    createSdDrawDoc("odg/tablestyles.fodg");
    saveAndReload("draw8");

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamiliesSupplier->getStyleFamilies()->getByName("table"), uno::UNO_QUERY);

    uno::Reference<style::XStyle> xTableStyle(xStyleFamily->getByName("default"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(!xTableStyle->isUserDefined());

    uno::Reference<container::XNameAccess> xNameAccess(xTableStyle, uno::UNO_QUERY);
    uno::Reference<style::XStyle> xCellStyle(xNameAccess->getByName("first-row"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCellStyle);
    CPPUNIT_ASSERT_EQUAL(OUString("userdefined"), xCellStyle->getName());

    CPPUNIT_ASSERT(xStyleFamily->hasByName("userdefined"));
    xTableStyle.set(xStyleFamily->getByName("userdefined"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTableStyle->isUserDefined());
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testTdf153179)
{
    createSdImpressDoc("pptx/ole-emf_min.pptx");
    saveAndReload("impress8");

    // Check number of shapes after export.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getPage(0)->getCount());
}

CPPUNIT_TEST_FIXTURE(SdExportTest, testSvgImageSupport)
{
    for (OUString const& rFormat : { u"impress8"_ustr, u"Impress Office Open XML"_ustr })
    {
        // Load the original file
        createSdImpressDoc("odp/SvgImageTest.odp");
        // Save into the target format
        saveAndReload(rFormat);

        const OString sFailedMessage = "Failed on filter: " + rFormat.toUtf8();

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
        xPropertySet->getPropertyValue("Graphic") >>= xGraphic;
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());

        // Access the Graphic
        Graphic aGraphic(xGraphic);

        // Check if it contian a VectorGraphicData struct
        auto pVectorGraphic = aGraphic.getVectorGraphicData();
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pVectorGraphic);

        // Which should be of type SVG, which means we have a SVG file
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), VectorGraphicDataType::Svg,
                                     pVectorGraphic->getType());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
