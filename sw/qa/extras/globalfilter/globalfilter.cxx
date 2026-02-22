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
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <officecfg/Office/Common.hxx>
#include <tools/zcodec.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <vcl/pdf/PDFAnnotationSubType.hxx>
#include <vcl/pdf/PDFObjectType.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <sfx2/linkmgr.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <editsh.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentContentOperations.hxx>
#include <doc.hxx>
#include <ndgrf.hxx>
#include <ndtxt.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <wrtsh.hxx>
#include <xmloff/odffields.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IMark.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <test/commontesttools.hxx>
#include <unotools/saveopt.hxx>

namespace
{
using MSFilterCfg = officecfg::Office::Common::Filter::Microsoft;

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/globalfilter/data/"_ustr) {}

    void testEmbeddedGraphicRoundtrip(TestFilter eFilterName);
    void testLinkedGraphicRT(TestFilter eFilterName);
    void testImageWithSpecialID(TestFilter eFilterName);
    void testGraphicShape(TestFilter eFilterName);
    void testMultipleIdenticalGraphics(TestFilter eFilterName);
    void testCharHighlightBody(TestFilter eFilterName);
    void testCharStyleHighlight(TestFilter eFilterName);
    void testMSCharBackgroundEditing(TestFilter eFilterName);
    void testCharBackgroundToHighlighting(TestFilter eFilterName);
#if !defined(_WIN32)
    void testSkipImages(OUString const& rName, OUString const& rFilterOptions);
#endif
    void testNestedFieldmark(TestFilter eFilterName, OUString const& rName);
    void verifyText13(char const*const pTestName);
    void testRedlineFlags(TestFilter eFilterName);
    void testBulletAsImage(TestFilter eFilterName);
    void testTextFormField(TestFilter eFilterName);
    void testCheckBoxFormField(TestFilter eFilterName);
    void testDropDownFormField(TestFilter eFilterName);
    void testDateFormField(TestFilter eFilterName);
    void testDateFormFieldCharacterFormatting(TestFilter eFilterName);
    void testSvgImageSupport(TestFilter eFilterName);
};

void Test::testEmbeddedGraphicRoundtrip(TestFilter eFilterName)
{
    // Check whether the export code swaps in the image which was swapped out before by auto mechanism

    createSwDoc("document_with_two_images.odt");

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check whether graphic exported well after it was swapped out
    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), 2, getShapes());

    // First image
    uno::Reference<drawing::XShape> xImage(getShape(1), uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );

    // Check graphic, size
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), graphic::GraphicType::PIXEL, xGraphic->getType());
        uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height);
    }

    // Second Image
    xImage.set(getShape(2), uno::UNO_QUERY);
    XPropSet.set( xImage, uno::UNO_QUERY_THROW );

    // Check graphic, size
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), graphic::GraphicType::PIXEL, xGraphic->getType());
        uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedGraphicRoundtrip_ODT)
{
    testEmbeddedGraphicRoundtrip(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedGraphicRoundtrip_RTF)
{
    testEmbeddedGraphicRoundtrip(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedGraphicRoundtrip_DOC)
{
    testEmbeddedGraphicRoundtrip(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testEmbeddedGraphicRoundtrip_DOCX)
{
    testEmbeddedGraphicRoundtrip(TestFilter::DOCX);
}

void Test::testLinkedGraphicRT(TestFilter eFilterName)
{
    createSwDoc("document_with_linked_graphic.odt");

    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pDoc);
    SwNodes& aNodes = pDoc->GetNodes();

    // Find the image
    bool bImageFound = false;
    Graphic aGraphic;
    for (SwNodeOffset nIndex(0); nIndex < aNodes.Count(); ++nIndex)
    {
        if (aNodes[nIndex]->IsGrfNode())
        {
            SwGrfNode* pGrfNode = aNodes[nIndex]->GetGrfNode();
            CPPUNIT_ASSERT(pGrfNode);

            const GraphicObject& rGraphicObj = pGrfNode->GetGrfObj(true);
            aGraphic = rGraphicObj.GetGraphic();
            bImageFound = true;
        }
    }

    CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), bImageFound);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), GraphicType::Bitmap, aGraphic.GetType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_uLong(864900), aGraphic.GetSizeBytes());

    // Check if linked graphic is registered in LinkManager
    SwEditShell* const pEditShell(getSwDoc()->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    sfx2::LinkManager& rLinkManager = pEditShell->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), size_t(1), rLinkManager.GetLinks().size());
    const tools::SvRef<sfx2::SvBaseLink> & rLink = rLinkManager.GetLinks()[0];
    CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), rLink->GetLinkSourceName().indexOf("linked_graphic.jpg") >= 0);
}

CPPUNIT_TEST_FIXTURE(Test, testLinkedGraphicRT_ODT)
{
    testLinkedGraphicRT(TestFilter::ODT);
}

/* Note: picture is there, but SwGrfNode is not found?
CPPUNIT_TEST_FIXTURE(Test, testLinkedGraphicRT_RTF)
{
    testLinkedGraphicRT(TestFilter::RTF);
}
*/

CPPUNIT_TEST_FIXTURE(Test, testLinkedGraphicRT_DOC)
{
    testLinkedGraphicRT(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testLinkedGraphicRT_DOCX)
{
    testLinkedGraphicRT(TestFilter::DOCX);
}

void Test::testImageWithSpecialID(TestFilter eFilterName)
{
    // Check how LO handles when the imported graphic's ID is different from that one
    // which is generated by LO.

    createSwDoc("images_with_special_IDs.odt");

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check whether graphic exported well
    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), 2, getShapes());

    uno::Reference<drawing::XShape> xImage = getShape(1);
    uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );

    // Check graphic, size
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), graphic::GraphicType::PIXEL, xGraphic->getType());
        uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height);
    }

    // Second Image
    xImage.set(getShape(2), uno::UNO_QUERY);
    XPropSet.set( xImage, uno::UNO_QUERY_THROW );

    // Check graphic, size
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), graphic::GraphicType::PIXEL, xGraphic->getType());
        uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testImageWithSpecialID_ODT)
{
    testImageWithSpecialID(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testImageWithSpecialID_RTF)
{
    testImageWithSpecialID(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testImageWithSpecialID_DOC)
{
    testImageWithSpecialID(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testImageWithSpecialID_DOCX)
{
    testImageWithSpecialID(TestFilter::DOCX);
}

/// Gives the first embedded or linked image in a document.
uno::Reference<drawing::XShape> lcl_getShape(const uno::Reference<lang::XComponent>& xComponent, bool bEmbedded)
{
    uno::Reference<drawing::XShape> xShape;

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    for (sal_Int32 i = 0; i < xDrawPage->getCount(); ++i)
    {
        uno::Reference<beans::XPropertySet> xShapeProperties(xDrawPage->getByIndex(i), uno::UNO_QUERY);
        uno::Reference<graphic::XGraphic> xGraphic;
        xShapeProperties->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        if (xGraphic.is())
        {
            Graphic aGraphic(xGraphic);

            if (bEmbedded == aGraphic.getOriginURL().isEmpty())
            {
                xShape.set(xShapeProperties, uno::UNO_QUERY);
                return xShape;
            }
        }
    }

    return xShape;
}

void Test::testGraphicShape(TestFilter eFilterName)
{
    // There are two kind of images in Writer: 1) Writer specific handled by SwGrfNode and
    // 2) graphic shape handled by SdrGrafObj (e.g. after copy&paste from Impress).
    createSwDoc("graphic_shape.odt");

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check whether graphic exported well
    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), 2, getShapes());

    uno::Reference<drawing::XShape> xImage = lcl_getShape(mxComponent, true);
    CPPUNIT_ASSERT_MESSAGE("Couldn't load the shape/image", xImage.is());
    uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY );
    // First image is embedded
    // Check size
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
        uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width );
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height );
    }

    // MS filters make this kind of linked images broken !?
    if (eFilterName != TestFilter::ODT)
        return;

    // Second image is a linked one
    xImage = lcl_getShape(mxComponent, false);
    XPropSet.set(xImage, uno::UNO_QUERY);
    const OString sFailedImageLoad = OString::Concat("Couldn't load the shape/image for ") + TestFilterNames.at(eFilterName).toUtf8();
    CPPUNIT_ASSERT_MESSAGE(sFailedImageLoad.getStr(), xImage.is());

    // Check size
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());

        Graphic aGraphic(xGraphic);
        OUString sURL = aGraphic.getOriginURL();
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL.endsWith("linked_graphic.jpg"));

        uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(620), xBitmap->getSize().Width);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(465), xBitmap->getSize().Height);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testGraphicShape_ODT)
{
    testGraphicShape(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testGraphicShape_RTF)
{
    testGraphicShape(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testGraphicShape_DOC)
{
    testGraphicShape(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testGraphicShape_DOCX)
{
    testGraphicShape(TestFilter::DOCX);
}

std::vector<uno::Reference<graphic::XGraphic>>
    lcl_getGraphics(const uno::Reference<lang::XComponent>& xComponent)
{
    std::vector<uno::Reference<graphic::XGraphic>> aGraphics;

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    for (sal_Int32 i = 0; i < xDrawPage->getCount(); ++i)
    {
        uno::Reference<beans::XPropertySet> xShapeProperties(xDrawPage->getByIndex(i), uno::UNO_QUERY);
        uno::Reference<graphic::XGraphic> xGraphic;
        xShapeProperties->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        if (xGraphic.is())
        {
            aGraphics.push_back(xGraphic);
        }
    }

    return aGraphics;
}

void Test::testMultipleIdenticalGraphics(TestFilter eFilterName)
{
    // We have multiple identical graphics. When we save them we want
    // them to be saved de-duplicated and the same should still be true
    // after loading them again. This test check that the de-duplication
    // works as expected.

    createSwDoc("multiple_identical_graphics.odt");

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check whether graphic exported well
    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();
    auto aGraphics = lcl_getGraphics(mxComponent);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), size_t(5), aGraphics.size());

    // Get all GfxLink addresses, we expect all of them to be the same
    // indicating we use the same graphic instance for all shapes
    std::vector<sal_Int64> aGfxLinkAddresses;
    for (auto const & rxGraphic : aGraphics)
    {
        GfxLink* pLink = Graphic(rxGraphic).GetSharedGfxLink().get();
        aGfxLinkAddresses.emplace_back(reinterpret_cast<sal_Int64>(pLink));
    }

    // Check all addresses are the same
    bool bResult = std::equal(aGfxLinkAddresses.begin() + 1, aGfxLinkAddresses.end(), aGfxLinkAddresses.begin());
    const OString sGraphicNotTheSameFailedMessage = OString::Concat("Graphics not the same for filter: '") +
        TestFilterNames.at(eFilterName).toUtf8() + OString::Concat("'");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sGraphicNotTheSameFailedMessage.getStr(), true, bResult);
}

CPPUNIT_TEST_FIXTURE(Test, testMultipleIdenticalGraphics_ODT)
{
    testMultipleIdenticalGraphics(TestFilter::ODT);
}

/* doesn't work correctly for now
CPPUNIT_TEST_FIXTURE(Test, testMultipleIdenticalGraphics_RTF)
{
    testMultipleIdenticalGraphics(TestFilter::RTF);
}
*/

CPPUNIT_TEST_FIXTURE(Test, testMultipleIdenticalGraphics_DOC)
{
    testMultipleIdenticalGraphics(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testMultipleIdenticalGraphics_DOCX)
{
    testMultipleIdenticalGraphics(TestFilter::DOCX);
}

void Test::testCharHighlightBody(TestFilter eFilterName)
{
    // MS Word has two kind of character backgrounds called character shading and highlighting
    // MS filters handle these attributes separately, but ODF export merges them into one background attribute

    createSwDoc("char_highlight.docx");

    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    const uno::Reference< text::XTextRange > xPara = getParagraph(1);
    // Both highlight and background
    const Color nBackColor(0x4F81BD);
    for( int nRun = 1; nRun <= 16; ++nRun )
    {
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,nRun), uno::UNO_QUERY);
        Color nHighlightColor;
        switch( nRun )
        {
            case 1: nHighlightColor = COL_BLACK; break; //black 0x000000
            case 2: nHighlightColor = COL_LIGHTBLUE; break; //light blue 0x0000ff
            case 3: nHighlightColor = COL_LIGHTCYAN; break; //light cyan 0x00ffff
            case 4: nHighlightColor = COL_LIGHTGREEN; break; //light green 0x00ff00
            case 5: nHighlightColor = COL_LIGHTMAGENTA; break; //light magenta 0xff00ff
            case 6: nHighlightColor = COL_LIGHTRED; break; //light red 0xff0000
            case 7: nHighlightColor = COL_YELLOW; break; //yellow 0xffff00
            case 8: nHighlightColor = COL_WHITE; break; //white 0xffffff
            case 9: nHighlightColor = COL_BLUE;  break;//blue 0x000080
            case 10: nHighlightColor = COL_CYAN; break; //cyan 0x008080
            case 11: nHighlightColor = COL_GREEN; break; //green 0x008000
            case 12: nHighlightColor = COL_MAGENTA; break; //magenta 0x800080
            case 13: nHighlightColor = COL_RED; break; //red 0x800000
            case 14: nHighlightColor = COL_BROWN; break; //brown 0x808000
            case 15: nHighlightColor = COL_GRAY; break; //dark gray 0x808080
            case 16: nHighlightColor = COL_LIGHTGRAY; break; //light gray 0xC0C0C0
        }

        if (eFilterName == TestFilter::ODT)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xRun,u"CharHighlight"_ustr));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nHighlightColor, getProperty<Color>(xRun,u"CharBackColor"_ustr));
        }
        else // MS filters
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nHighlightColor, getProperty<Color>(xRun,u"CharHighlight"_ustr));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nBackColor, getProperty<Color>(xRun,u"CharBackColor"_ustr));
        }
    }

    // Only highlight
    {
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,18), uno::UNO_QUERY);
        if (eFilterName == TestFilter::ODT)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xRun,u"CharHighlight"_ustr));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_LIGHTRED, getProperty<Color>(xRun,u"CharBackColor"_ustr));
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_LIGHTRED, getProperty<Color>(xRun,u"CharHighlight"_ustr));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xRun,u"CharBackColor"_ustr));
        }
    }

    // Only background
    {
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,19), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xRun,u"CharHighlight"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_LIGHTBLUE, getProperty<Color>(xRun,u"CharBackColor"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testCharHighlightBody_ODT)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(false);
    testCharHighlightBody(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testCharHighlightBody_RTF)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(false);
    testCharHighlightBody(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testCharHighlightBody_DOC)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(false);
    testCharHighlightBody(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testCharHighlightBody_DOCX)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(false);
    testCharHighlightBody(TestFilter::DOCX);
}

CPPUNIT_TEST_FIXTURE(Test, testCharHighlightBody_ODT2)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(true);
    testCharHighlightBody(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testCharHighlightBody_RTF2)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(true);
    testCharHighlightBody(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testCharHighlightBody_DOC2)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(true);
    testCharHighlightBody(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testCharHighlightBody_DOCX2)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(true);
    testCharHighlightBody(TestFilter::DOCX);
}

void Test::testCharStyleHighlight(TestFilter eFilterName)
{
    // MS Word has two kind of character backgrounds called character shading and highlighting.
    // However, their character style can only accept shading. It ignores the highlighting value.

    createSwDoc("tdf138345_charstyle_highlight.odt");

    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    uno::Reference<beans::XPropertySet> xCharStyle;
    getStyles(u"CharacterStyles"_ustr)->getByName(u"charBackground"_ustr) >>= xCharStyle;
    const Color nBackColor(0xFFDBB6); //orange-y

    // Always export character style's background colour as shading, never as highlighting.
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xCharStyle,u"CharHighlight"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nBackColor, getProperty<Color>(xCharStyle,u"CharBackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testCharStyleHighlight_ODT)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(false);
    testCharStyleHighlight(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testCharStyleHighlight_RTF)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(false);
    testCharStyleHighlight(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testCharStyleHighlight_DOC)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(false);
    testCharStyleHighlight(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testCharStyleHighlight_DOCX)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(false);
    testCharStyleHighlight(TestFilter::DOCX);
}

CPPUNIT_TEST_FIXTURE(Test, testCharStyleHighlight_ODT2)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(true);
    testCharStyleHighlight(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testCharStyleHighlight_RTF2)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(true);
    testCharStyleHighlight(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testCharStyleHighlight_DOC2)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(true);
    testCharStyleHighlight(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testCharStyleHighlight_DOCX2)
{
    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(true);
    testCharStyleHighlight(TestFilter::DOCX);
}

CPPUNIT_TEST_FIXTURE(Test, testCharHighlightODF)
{
    createSwDoc("char_background_editing.docx");

    // don't check import, testMSCharBackgroundEditing already does that

    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    for (int i = 1; i <= 4; ++i)
    {
        uno::Reference<beans::XPropertySet> xRun(getRun(xPara,i), uno::UNO_QUERY);
        switch (i)
        {
            case 1: // non-transparent highlight
            xRun->setPropertyValue(u"CharBackColor"_ustr, uno::Any(static_cast<sal_Int32>(128)));
            xRun->setPropertyValue(u"CharBackTransparent"_ustr, uno::Any(true));
            xRun->setPropertyValue(u"CharHighlight"_ustr, uno::Any(static_cast<sal_Int32>(64)));
            break;

            case 2: // transparent backcolor
            xRun->setPropertyValue(u"CharBackColor"_ustr, uno::Any(static_cast<sal_Int32>(128)));
            xRun->setPropertyValue(u"CharBackTransparent"_ustr, uno::Any(true));
            xRun->setPropertyValue(u"CharHighlight"_ustr, uno::Any(static_cast<sal_Int32>(COL_TRANSPARENT)));
            break;

            case 3: // non-transparent backcolor
            xRun->setPropertyValue(u"CharBackColor"_ustr, uno::Any(static_cast<sal_Int32>(128)));
            xRun->setPropertyValue(u"CharBackTransparent"_ustr, uno::Any(false));
            xRun->setPropertyValue(u"CharHighlight"_ustr, uno::Any(static_cast<sal_Int32>(COL_TRANSPARENT)));
            break;

            case 4: // non-transparent highlight again
            xRun->setPropertyValue(u"CharBackColor"_ustr, uno::Any(static_cast<sal_Int32>(128)));
            xRun->setPropertyValue(u"CharBackTransparent"_ustr, uno::Any(false));
            xRun->setPropertyValue(u"CharHighlight"_ustr, uno::Any(static_cast<sal_Int32>(64)));
            break;
        }
    }

    saveAndReload(TestFilter::ODT);

    xPara.set(getParagraph(1));
    for (int i = 1; i <= 4; ++i)
    {
        uno::Reference<beans::XPropertySet> xRun(getRun(xPara,i), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_TRANSPARENT), getProperty<sal_Int32>(xRun, u"CharHighlight"_ustr));
        switch (i)
        {
            case 1: // non-transparent highlight
            CPPUNIT_ASSERT_EQUAL(Color(0x000040), getProperty<Color>(xRun, u"CharBackColor"_ustr));
            CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xRun, u"CharBackTransparent"_ustr));
            break;
            case 2: // transparent backcolor
            CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xRun, u"CharBackColor"_ustr));
            CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xRun, u"CharBackTransparent"_ustr));
            break;
            case 3: // non-transparent backcolor
            CPPUNIT_ASSERT_EQUAL(COL_BLUE, getProperty<Color>(xRun, u"CharBackColor"_ustr));
            CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xRun, u"CharBackTransparent"_ustr));
            break;
            case 4: // non-transparent highlight again
            CPPUNIT_ASSERT_EQUAL(Color(0x000040), getProperty<Color>(xRun, u"CharBackColor"_ustr));
            CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xRun, u"CharBackTransparent"_ustr));
            break;
        }
    }
}

void Test::testMSCharBackgroundEditing(TestFilter eFilterName)
{
    // Simulate the editing process of imported MSO character background attributes
    // and check how export behaves.

    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(true);

    createSwDoc("char_background_editing.docx");

    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    // Check whether import was done on the right way
    uno::Reference< text::XTextRange > xPara = getParagraph(1);
    {
        uno::Reference<beans::XPropertySet> xRun(getRun(xPara,1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xRun,u"CharHighlight"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_LIGHTRED, getProperty<Color>(xRun,u"CharBackColor"_ustr));

        xRun.set(getRun(xPara,2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_LIGHTBLUE, getProperty<Color>(xRun,u"CharHighlight"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xRun,u"CharBackColor"_ustr));

        xRun.set(getRun(xPara,3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_LIGHTBLUE, getProperty<Color>(xRun,u"CharHighlight"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_LIGHTRED, getProperty<Color>(xRun,u"CharBackColor"_ustr));

        xRun.set(getRun(xPara,4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xRun,u"CharHighlight"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xRun,u"CharBackColor"_ustr));
    }

    // Simulate editing
    for( int i = 1; i <= 4; ++i )
    {
        uno::Reference<beans::XPropertySet> xRun(getRun(xPara,i), uno::UNO_QUERY);
        // Change background
        Color nBackColor;
        switch( i )
        {
            case 1: nBackColor = COL_BLACK; break; //black 0x000000
            case 2: nBackColor = COL_LIGHTCYAN; break; //cyan 0x00ffff
            case 3: nBackColor = COL_LIGHTGREEN; break; //green 0x00ff00
            case 4: nBackColor = COL_LIGHTMAGENTA; break; //magenta 0xff00ff
        }
        xRun->setPropertyValue(u"CharBackColor"_ustr, uno::Any(nBackColor));
        // Remove highlighting
        xRun->setPropertyValue(u"CharHighlight"_ustr, uno::Any(COL_TRANSPARENT));
        // Remove shading marker
        uno::Sequence<beans::PropertyValue> aGrabBag = getProperty<uno::Sequence<beans::PropertyValue> >(xRun,u"CharInteropGrabBag"_ustr);
        for (beans::PropertyValue& rProp : asNonConstRange(aGrabBag))
        {
            if (rProp.Name == "CharShadingMarker")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), true, rProp.Value.get<bool>());
                rProp.Value <<= false;
            }
        }
        xRun->setPropertyValue(u"CharInteropGrabBag"_ustr, uno::Any(aGrabBag));
    }

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check whether background was exported as highlighting
    xPara.set(getParagraph(1));
    for( int i = 1; i <= 4; ++i )
    {
        Color nBackColor;
        switch( i )
        {
            case 1: nBackColor = COL_BLACK; break; //black 0x000000
            case 2: nBackColor = COL_LIGHTCYAN; break; //light cyan 0x00ffff
            case 3: nBackColor = COL_LIGHTGREEN; break; //light green 0x00ff00
            case 4: nBackColor = COL_LIGHTMAGENTA; break; //light magenta 0xff00ff
        }
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,i), uno::UNO_QUERY);
        if (eFilterName == TestFilter::ODT)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xRun,u"CharHighlight"_ustr));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nBackColor, getProperty<Color>(xRun,u"CharBackColor"_ustr));
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), nBackColor, getProperty<Color>(xRun,u"CharHighlight"_ustr));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_TRANSPARENT, getProperty<Color>(xRun,u"CharBackColor"_ustr));
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testMSCharBackgroundEditing_ODT)
{
    testMSCharBackgroundEditing(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testMSCharBackgroundEditing_RTF)
{
    testMSCharBackgroundEditing(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testMSCharBackgroundEditing_DOC)
{
    testMSCharBackgroundEditing(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testMSCharBackgroundEditing_DOCX)
{
    testMSCharBackgroundEditing(TestFilter::DOCX);
}

void Test::testCharBackgroundToHighlighting(TestFilter eFilterName)
{
    // MSO highlighting has less kind of values so let's see how LO character background is converted
    // to these values
    createSwDoc("char_background.odt");

    OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    ScopedConfigValue<MSFilterCfg::Export::CharBackgroundToHighlighting> aCfg(true);

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check highlight color
    const uno::Reference< text::XTextRange > xPara = getParagraph(1);
    for( int nRun = 1; nRun <= 19; ++nRun )
    {
        const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,nRun), uno::UNO_QUERY);
        Color nHighlightColor;
        switch( nRun )
        {
            case 1: nHighlightColor = COL_BLACK; break; //black 0x000000
            case 2: nHighlightColor = COL_YELLOW; break; //yellow 0xffff00
            case 3: nHighlightColor = COL_LIGHTMAGENTA; break; //light magenta 0xff00ff
            case 4: nHighlightColor = COL_LIGHTCYAN; break; //light cyan 0x00ffff
            case 5: nHighlightColor = COL_YELLOW; break; //yellow 0xffff00
            case 6: nHighlightColor = COL_LIGHTRED; break; //light red 0xff0000
            case 7: nHighlightColor = COL_LIGHTBLUE; break; //light blue 0x0000ff
            case 8: nHighlightColor = COL_LIGHTGREEN; break; //light green 0x00ff00
            case 9: nHighlightColor = COL_GREEN; break; //dark green 0x008000
            case 10: nHighlightColor = COL_MAGENTA; break; //dark magenta 0x800080
            case 11: nHighlightColor = COL_BLUE; break; //dark blue 0x000080
            case 12: nHighlightColor = COL_BROWN; break; //brown 0x808000
            case 13: nHighlightColor = COL_GRAY; break; //dark gray 0x808080
            case 14: nHighlightColor = COL_BLACK; break; //black 0x000000
            case 15: nHighlightColor = COL_LIGHTRED; break; //light red 0xff0000
            case 16: nHighlightColor = COL_LIGHTGRAY; break; //light gray 0xC0C0C0
            case 17: nHighlightColor = COL_RED; break; //dark red 0x800000
            case 18: nHighlightColor = COL_GRAY; break; //dark gray 0x808080
            case 19: nHighlightColor = COL_YELLOW; break; //yellow 0xffff00
        }
        const OString sMessage = sFailedMessage +". Index of run with unmatched color: " + OString::number(nRun);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sMessage.getStr(), nHighlightColor, getProperty<Color>(xRun,u"CharHighlight"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testCharBackgroundToHighlighting_RTF)
{
    testCharBackgroundToHighlighting(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testCharBackgroundToHighlighting_DOC)
{
    testCharBackgroundToHighlighting(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testCharBackgroundToHighlighting_DOCX)
{
    testCharBackgroundToHighlighting(TestFilter::DOCX);
}

#if !defined(_WIN32)
void Test::testSkipImages(OUString const& rName, OUString const& rFilterOptions)
{
    // Check how LO skips image loading (but not texts of textboxes and custom shapes)
    // during DOC and DOCX import, using the "SkipImages" FilterOptions.

    bool bSkipImages = !rFilterOptions.isEmpty();
    OString sFailedMessage = OString::Concat("Failed on filter: ") + rName.toUtf8();

    createSwDoc(rName.toUtf8().getStr(),
        {
            comphelper::makePropertyValue(u"FilterOptions"_ustr, rFilterOptions),
        });
    sFailedMessage += " - " + rFilterOptions.toUtf8();

    // Check shapes (images, textboxes, custom shapes)
    uno::Reference<drawing::XShape> xShape;
    uno::Reference<graphic::XGraphic> xGraphic;
    uno::Reference< beans::XPropertySet > XPropSet;
    uno::Reference<awt::XBitmap> xBitmap;

    bool bHasTextboxText = false;
    bool bHasCustomShapeText = false;
    sal_Int32 nImageCount = 0;

    for (int i = 1; i<= getShapes(); i++)
    {
        xShape = getShape(i);
        XPropSet.set( xShape, uno::UNO_QUERY_THROW );
        try
        {
            XPropSet->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
            xBitmap.set(xGraphic, uno::UNO_QUERY);
            if (xBitmap.is())
                nImageCount++;
        }
        catch (beans::UnknownPropertyException &)
        { /* ignore */ }

        uno::Reference<text::XTextRange> xText(xShape, uno::UNO_QUERY);
        if (xText.is())
        {
            OUString shapeText = xText->getString();
            if (shapeText.startsWith("Lorem ipsum"))
                bHasTextboxText = true;
            else if (shapeText.startsWith("Nam pretium"))
                bHasCustomShapeText = true;
        }
    }

    CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), bHasTextboxText);
    CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), bHasCustomShapeText);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(bSkipImages ? 0 : 3), nImageCount );
}

CPPUNIT_TEST_FIXTURE(Test, testSkipImages_DOC)
{
    testSkipImages("skipimages.doc", "");
}

CPPUNIT_TEST_FIXTURE(Test, testSkipImages_DOC2)
{
    testSkipImages("skipimages.doc", "SkipImages");
}

CPPUNIT_TEST_FIXTURE(Test, testSkipImages_DOCX)
{
    testSkipImages("skipimages.docx", "");
}

CPPUNIT_TEST_FIXTURE(Test, testSkipImages_DOCX2)
{
    testSkipImages("skipimages.docx", "SkipImages");
}
#endif

void Test::testNestedFieldmark(TestFilter eFilterName, OUString const& rName)
{
    // experimental config setting
    ScopedConfigValue<MSFilterCfg::Import::ForceImportWWFieldsAsGenericFields> aCfg(true);

    auto verify = [this](OUString const& rTestName) {
        SwDoc* pDoc = getSwDoc();
        IDocumentMarkAccess const& rIDMA(*pDoc->getIDocumentMarkAccess());

        // no spurious bookmarks have been created
        CPPUNIT_ASSERT_EQUAL_MESSAGE(rTestName.toUtf8().getStr(),
                sal_Int32(0), rIDMA.getBookmarksCount());

        // check inner fieldmark
        SwNodeIndex const node1(*pDoc->GetNodes().GetEndOfContent().StartOfSectionNode(), +2);
        SwPosition const innerPos(*node1.GetNode().GetTextNode(),
            node1.GetNode().GetTextNode()->GetText().indexOf(CH_TXT_ATR_FIELDSTART));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(rTestName.toUtf8().getStr(),
                sal_Int32(1), innerPos.GetContentIndex());
        ::sw::mark::Fieldmark *const pInner(rIDMA.getFieldmarkAt(innerPos));
        CPPUNIT_ASSERT_MESSAGE(rTestName.toUtf8().getStr(), pInner);
        OUString const innerString(SwPaM(pInner->GetMarkPos(), pInner->GetOtherMarkPos()).GetText());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(rTestName.toUtf8().getStr(), OUString(
            OUStringChar(CH_TXT_ATR_FIELDSTART) + u" QUOTE  \"foo " + OUStringChar(CH_TXTATR_NEWLINE)
            + u" bar " + OUStringChar(CH_TXTATR_NEWLINE)
            + u"baz\" " + OUStringChar(CH_TXT_ATR_FIELDSEP) + u"foo " + OUStringChar(CH_TXTATR_NEWLINE)
            + u" bar " + OUStringChar(CH_TXTATR_NEWLINE)
            + u"baz" + OUStringChar(CH_TXT_ATR_FIELDEND)), innerString);

        // check outer fieldmark
        SwNodeIndex const node2(node1, -1);
        SwPosition const outerPos(*node2.GetNode().GetTextNode(),
            node2.GetNode().GetTextNode()->GetText().indexOf(CH_TXT_ATR_FIELDSTART));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(rTestName.toUtf8().getStr(),
                sal_Int32(0), outerPos.GetContentIndex());
        ::sw::mark::Fieldmark const*const pOuter(rIDMA.getFieldmarkAt(outerPos));
        CPPUNIT_ASSERT_MESSAGE(rTestName.toUtf8().getStr(), pOuter);
        OUString const outerString(SwPaM(pOuter->GetMarkPos(), pOuter->GetOtherMarkPos()).GetText());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(rTestName.toUtf8().getStr(), OUString(
            OUStringChar(CH_TXT_ATR_FIELDSTART) + u" QUOTE  \"foo " + OUStringChar(CH_TXTATR_NEWLINE)
            + u" " + OUStringChar(CH_TXT_ATR_FIELDSTART) + u" QUOTE  \"foo " + OUStringChar(CH_TXTATR_NEWLINE)
            + u" bar " + OUStringChar(CH_TXTATR_NEWLINE)
            + u"baz\" " + OUStringChar(CH_TXT_ATR_FIELDSEP) + u"foo " + OUStringChar(CH_TXTATR_NEWLINE)
            + u" bar " + OUStringChar(CH_TXTATR_NEWLINE)
            + u"baz" + OUStringChar(CH_TXT_ATR_FIELDEND) + OUStringChar(CH_TXTATR_NEWLINE)
            + u"bar " + OUStringChar(CH_TXTATR_NEWLINE)
            + u"baz\" " + OUStringChar(CH_TXT_ATR_FIELDSEP) + u"foo " + OUStringChar(CH_TXTATR_NEWLINE)
            + u" foo " + OUStringChar(CH_TXTATR_NEWLINE)
            + u" bar " + OUStringChar(CH_TXTATR_NEWLINE)
            + u"baz" + OUStringChar(CH_TXTATR_NEWLINE)
            + u"bar " + OUStringChar(CH_TXTATR_NEWLINE)
            + u"baz" + OUStringChar(CH_TXT_ATR_FIELDEND)), outerString);

        // must return innermost mark
        CPPUNIT_ASSERT_EQUAL(pInner, rIDMA.getInnerFieldmarkFor(innerPos));
    };

    createSwDoc(rName.toUtf8().getStr());

    verify(TestFilterNames.at(eFilterName) + ", load");

    // Export the document and import again
    saveAndReload(eFilterName);

    verify(TestFilterNames.at(eFilterName) + " exported-reload");
}

CPPUNIT_TEST_FIXTURE(Test, testNestedFieldmark1)
{
    testNestedFieldmark(TestFilter::ODT, "fieldmark_QUOTE_nest.fodt");
}

CPPUNIT_TEST_FIXTURE(Test, testNestedFieldmark2)
{
    testNestedFieldmark(TestFilter::DOCX, "fieldmark_QUOTE_nest.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testNestedFieldmark3)
{
    testNestedFieldmark(TestFilter::RTF, "fieldmark_QUOTE_nest.rtf");
}

void Test::verifyText13(char const*const pTestName)
{
    // OFFICE-3789 style:header-first/style:footer-first
    uno::Reference<beans::XPropertySet> xPageStyle;
    getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr) >>= xPageStyle;
    uno::Reference<text::XText> xHF(getProperty<uno::Reference<text::XText>>(xPageStyle, u"HeaderTextFirst"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, u"Header first"_ustr, xHF->getString());
    uno::Reference<text::XText> xFF(getProperty<uno::Reference<text::XText>>(xPageStyle, u"FooterTextFirst"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, u"Footer first"_ustr, xFF->getString());
    // OFFICE-3767 text:contextual-spacing
    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    CPPUNIT_ASSERT_MESSAGE(pTestName, getProperty<bool>(xPara, u"ParaContextMargin"_ustr));
    // OFFICE-3776 meta:creator-initials
    uno::Reference<text::XTextRange> xRun(getRun(xPara, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, u"Annotation"_ustr, getProperty<OUString>(xRun, u"TextPortionType"_ustr));
    uno::Reference<beans::XPropertySet> xComment(getProperty<uno::Reference<beans::XPropertySet>>(xRun, u"TextField"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, u"dj"_ustr, getProperty<OUString>(xComment, u"Initials"_ustr));
    // OFFICE-3941 text:index-entry-link-start/text:index-entry-link-end
    uno::Reference<text::XDocumentIndexesSupplier> xDIS(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes(xDIS->getDocumentIndexes());
    uno::Reference<text::XDocumentIndex> xIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XIndexReplace> xLevels(getProperty<uno::Reference<container::XIndexReplace>>(xIndex, u"LevelFormat"_ustr));
    uno::Sequence<beans::PropertyValues> format;
    xLevels->getByIndex(1) >>= format; // 1-based?
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, u"TokenType"_ustr, format[0][0].Name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, u"TokenHyperlinkStart"_ustr, format[0][0].Value.get<OUString>());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, u"TokenType"_ustr, format[4][0].Name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(pTestName, u"TokenHyperlinkEnd"_ustr, format[4][0].Value.get<OUString>());
}

// test ODF 1.3 new text document features
CPPUNIT_TEST_FIXTURE(Test, testODF13)
{
    comphelper::ScopeGuard g([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });

    // import
    createSwDoc("text13e.odt");

    // check model
    verifyText13("import");

    {
        // export ODF 1.3
        SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);

        saveAndReload(TestFilter::ODT);

        // check XML
        xmlDocUniquePtr pContentXml = parseExport(u"content.xml"_ustr);
        assertXPath(pContentXml, "/office:document-content/office:automatic-styles/style:style/style:paragraph-properties[@style:contextual-spacing='true']");
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:p/office:annotation/meta:creator-initials");
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:p/office:annotation/loext:sender-initials", 0);
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/text:index-entry-link-start");
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/loext:index-entry-link-start", 0);
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/text:index-entry-link-end");
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/loext:index-entry-link-end", 0);
        xmlDocUniquePtr pStylesXml = parseExport(u"styles.xml"_ustr);
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/style:header-first");
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/loext:header-first", 0);
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/style:footer-first");
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/loext:footer-first", 0);

        // check model
        verifyText13("1.3 reload");
    }
    {
        // export ODF 1.2 extended
        SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_012_EXTENDED);

        // FIXME: Error: unexpected attribute "loext:contextual-spacing"
        skipValidation();
        saveAndReload(TestFilter::ODT);

        // check XML
        xmlDocUniquePtr pContentXml = parseExport(u"content.xml"_ustr);
        assertXPath(pContentXml, "/office:document-content/office:automatic-styles/style:style/style:paragraph-properties[@loext:contextual-spacing='true']");
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:p/office:annotation/loext:sender-initials");
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:p/office:annotation/meta:creator-initials", 0);
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/loext:index-entry-link-start");
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/text:index-entry-link-start", 0);
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/loext:index-entry-link-end");
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/text:index-entry-link-end", 0);
        xmlDocUniquePtr pStylesXml = parseExport(u"styles.xml"_ustr);
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/loext:header-first");
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/style:header-first", 0);
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/loext:footer-first");
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/style:footer-first", 0);

        // check model
        verifyText13("1.2 Extended reload");
    }
    {
        // export ODF 1.2
        SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_012);

        // don't reload - no point
        save(TestFilter::ODT);

        // check XML
        xmlDocUniquePtr pContentXml = parseExport(u"content.xml"_ustr);
        assertXPathNoAttribute(pContentXml, "/office:document-content/office:automatic-styles/style:style/style:paragraph-properties", "contextual-spacing");
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:p/office:annotation/meta:creator-initials", 0);
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:p/office:annotation/loext:sender-initials", 0);
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/text:index-entry-link-start", 0);
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/loext:index-entry-link-start", 0);
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/text:index-entry-link-end", 0);
        assertXPath(pContentXml, "/office:document-content/office:body/office:text/text:illustration-index/text:illustration-index-source/text:illustration-index-entry-template/loext:index-entry-link-end", 0);
        xmlDocUniquePtr pStylesXml = parseExport(u"styles.xml"_ustr);
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/style:header-first", 0);
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/loext:header-first", 0);
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/style:footer-first", 0);
        assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page/loext:footer-first", 0);
    }
}

void Test::testRedlineFlags(TestFilter eFilterName)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    SwPaM pam(SwPosition(pDoc->GetNodes().GetEndOfContent(), SwNodeOffset(-1)));
    pDoc->getIDocumentContentOperations().InsertString(pam, u"foo bar baz"_ustr);

    IDocumentRedlineAccess & rIDRA(pDoc->getIDocumentRedlineAccess());
    // enable change tracking
    rIDRA.SetRedlineFlags(rIDRA.GetRedlineFlags()
        | RedlineFlags::On | RedlineFlags::ShowDelete);

    // need a delete redline to trigger mode switching
    pam.Move(fnMoveForward, GoInDoc);
    pam.SetMark();
    pam.Move(fnMoveBackward, GoInDoc);
    pDoc->getIDocumentContentOperations().DeleteAndJoin(pam);

    // hide delete redlines
    RedlineFlags const nRedlineFlags =
        rIDRA.GetRedlineFlags() & ~RedlineFlags::ShowDelete;
    rIDRA.SetRedlineFlags(nRedlineFlags);

    // export the document
    save(eFilterName);

    // tdf#97103 check that redline mode is properly restored
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        OString(OString::Concat("redline mode not restored in ") + TestFilterNames.at(eFilterName).toUtf8()).getStr(),
        static_cast<int>(nRedlineFlags), static_cast<int>(rIDRA.GetRedlineFlags()));
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineFlags_ODT)
{
    testRedlineFlags(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineFlags_RTF)
{
    testRedlineFlags(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineFlags_DOC)
{
    testRedlineFlags(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineFlags_DOCX)
{
    testRedlineFlags(TestFilter::DOCX);
}

void Test::testBulletAsImage(TestFilter eFilterName)
{
    OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    createSwDoc("BulletAsImage.odt");

    // Check if import was successful
    {
        uno::Reference<text::XTextRange> xPara(getParagraph(1));
        uno::Reference<beans::XPropertySet> xPropertySet(xPara, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xLevels;
        xLevels.set(xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aProperties;
        xLevels->getByIndex(0) >>= aProperties;
        uno::Reference<awt::XBitmap> xBitmap;
        awt::Size aSize;
        sal_Int16 nNumberingType = -1;

        for (beans::PropertyValue const& rProperty : aProperties)
        {
            if (rProperty.Name == "NumberingType")
            {
                nNumberingType = rProperty.Value.get<sal_Int16>();
            }
            else if (rProperty.Name == "GraphicBitmap")
            {
                if (rProperty.Value.has<uno::Reference<awt::XBitmap>>())
                {
                    xBitmap = rProperty.Value.get<uno::Reference<awt::XBitmap>>();
                }
            }
            else if (rProperty.Name == "GraphicSize")
            {
                aSize = rProperty.Value.get<awt::Size>();
            }
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), style::NumberingType::BITMAP, nNumberingType);

        // Graphic Bitmap
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
        Graphic aGraphic(uno::Reference<graphic::XGraphic>(xBitmap, uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), GraphicType::Bitmap, aGraphic.GetType());
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), aGraphic.GetSizeBytes() > o3tl::make_unsigned(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), tools::Long(16), aGraphic.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), tools::Long(16), aGraphic.GetSizePixel().Height());

        // Graphic Size
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(400), aSize.Width);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(400), aSize.Height);
    }

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    {
        uno::Reference<text::XTextRange> xPara(getParagraph(1));
        uno::Reference<beans::XPropertySet> xPropertySet(xPara, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xLevels;
        xLevels.set(xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aProperties;
        xLevels->getByIndex(0) >>= aProperties;
        uno::Reference<awt::XBitmap> xBitmap;
        awt::Size aSize;
        sal_Int16 nNumberingType = -1;

        for (beans::PropertyValue const& rProperty : aProperties)
        {
            if (rProperty.Name == "NumberingType")
            {
                nNumberingType = rProperty.Value.get<sal_Int16>();
            }
            else if (rProperty.Name == "GraphicBitmap")
            {
                if (rProperty.Value.has<uno::Reference<awt::XBitmap>>())
                {
                    xBitmap = rProperty.Value.get<uno::Reference<awt::XBitmap>>();
                }
            }
            else if (rProperty.Name == "GraphicSize")
            {
                aSize = rProperty.Value.get<awt::Size>();
            }
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), style::NumberingType::BITMAP, nNumberingType);

        // Graphic Bitmap
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
        Graphic aGraphic(uno::Reference<graphic::XGraphic>(xBitmap, uno::UNO_QUERY));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), GraphicType::Bitmap, aGraphic.GetType());
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), aGraphic.GetSizeBytes() > o3tl::make_unsigned(0));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), tools::Long(16), aGraphic.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), tools::Long(16), aGraphic.GetSizePixel().Height());

        // Graphic Size
        if (eFilterName == TestFilter::ODT) // ODT is correct
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(400), aSize.Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(400), aSize.Height);
        }
        // FIXME: MS Filters don't work correctly for graphic bullet size
        else if (eFilterName == TestFilter::DOCX || eFilterName == TestFilter::RTF)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(279), aSize.Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(279), aSize.Height);
        }
        else if (eFilterName == TestFilter::DOC)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(296), aSize.Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(296), aSize.Height);
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testBulletAsImage_ODT)
{
    testBulletAsImage(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testBulletAsImage_RTF)
{
    testBulletAsImage(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testBulletAsImage_DOC)
{
    testBulletAsImage(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testBulletAsImage_DOCX)
{
    testBulletAsImage(TestFilter::DOCX);
}

CPPUNIT_TEST_FIXTURE(Test, testListLabelPDFExport)
{
    createSwDoc();

    uno::Reference<text::XTextDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xText(xDoc->getText());
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexReplace> xNumRule(
        xFactory->createInstance(u"com.sun.star.text.NumberingRules"_ustr),
        uno::UNO_QUERY_THROW);
    OUStringBuffer listFormat;
    for (sal_Int32 i = 0; i < xNumRule->getCount(); ++i)
    {
        uno::Sequence<beans::PropertyValue> format;
        format.getArray();
        xNumRule->getByIndex(i) >>= format;
        {
            auto it(::std::find_if(format.begin(), format.end(),
                    [](auto const& r) { return r.Name == "NumberingType"; }));
            // need something RTL
            const_cast<uno::Any&>(it->Value) <<= style::NumberingType::CHARS_ARABIC;
        }
        {
#if 0
            // this doesn't work any more
            auto it(::std::find_if(format.begin(), format.end(),
                    [](auto const& r) { return r.Name == "ParentNumbering"; }));
            const_cast<uno::Any&>(it->Value) <<= sal_Int16(i + 1);
#endif
            listFormat.append("%" + OUString::number(i+1) + "%.");
            auto it(::std::find_if(format.begin(), format.end(),
                    [](auto const& r) { return r.Name == "ListFormat"; }));
            const_cast<uno::Any&>(it->Value) <<= listFormat.toString();
        }
        xNumRule->replaceByIndex(i, uno::Any(format));
    }
    uno::Reference<beans::XPropertySet>(getParagraph(1), uno::UNO_QUERY_THROW)->setPropertyValue(u"NumberingRules"_ustr, uno::Any(xNumRule));
    xText->insertControlCharacter(xText->getEnd(), text::ControlCharacter::PARAGRAPH_BREAK, false);
    uno::Reference<beans::XPropertySet>(getParagraph(2), uno::UNO_QUERY_THROW)->setPropertyValue(u"NumberingLevel"_ustr, uno::Any(sal_Int16(1)));
    xText->insertControlCharacter(xText->getEnd(), text::ControlCharacter::PARAGRAPH_BREAK, false);
    uno::Reference<beans::XPropertySet>(getParagraph(3), uno::UNO_QUERY_THROW)->setPropertyValue(u"NumberingLevel"_ustr, uno::Any(sal_Int16(2)));

    // check PDF export of the list items (label in particular)
    comphelper::SequenceAsHashMap aMediaDescriptor;
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // Non-NULL pPdfDocument means pdfium is available.
    if (pPdfDocument != nullptr)
    {
        // The document has one page.
        CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
        CPPUNIT_ASSERT(pPdfPage);

        std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage = pPdfPage->getTextPage();
        CPPUNIT_ASSERT(pPdfTextPage);

        int nChars = pPdfTextPage->countChars();
        CPPUNIT_ASSERT_EQUAL(22, nChars);

        // Check that the label strings were exported correctly
        std::vector<sal_uInt32> aChars(nChars);
        for (int i = 0; i < nChars; i++)
            aChars[i] = pPdfTextPage->getUnicode(i);
        OUString aText(aChars.data(), aChars.size());
        CPPUNIT_ASSERT_EQUAL(u"\u0623\r\n.\r\n\u0623.\u0623\r\n.\r\n\u0623.\u0623.\u0623\r\n."_ustr, aText);
    }

    // Parse the document again to get its raw content
    // TODO: get the content from PDFiumPage somehow
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    enum
    {
        Default,
        Lbl,
        LblFoundText
    } state
        = Default;

    auto nLine(0);
    auto nLbl(0);
    auto nLblTj(0);
    auto nLblTJ(0);
    std::vector<int> mcids;
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n";
            if (o3tl::starts_with(line, "/Lbl<</MCID") && o3tl::ends_with(line, ">>BDC"))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                mcids.push_back(o3tl::toInt32(line.substr(12)));
                state = Lbl;
                ++nLbl;
            }
            else if (state == Lbl)
            {
                auto const endj(line.find(">Tj"));
                if (endj != ::std::string_view::npos)
                {
                    state = LblFoundText;
                    ++nLblTj;
                }
                else
                {
                    auto const endJ(line.find("]TJ"));
                    if (endJ != ::std::string_view::npos)
                    {
                        state = LblFoundText;
                        ++nLblTJ;
                    }
                }
            }
            else if (state != Default && line == "EMC")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("missing text", LblFoundText, state);
                state = Default;
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unclosed MCS", Default, state);
    // ideally there should be 3 but apparently every text portion gets its own
    // tag - this should not be a problem if these are grouped in the structure
    // tree into 3 Lbl.
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nLbl)>(6), nLbl);
    // these are quite arbitrary?
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nLbl)>(6), nLblTJ + nLblTj);

    auto nL(0);
    for (const auto& rDocElement : aDocument.GetElements())
    {
        auto pObject0 = dynamic_cast<vcl::filter::PDFObjectElement*>(rDocElement.get());
        if (!pObject0)
            continue;
        auto pType0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject0->Lookup("Type"_ostr));
        if (!pType0 || pType0->GetValue() != "StructElem")
        {
            continue;
        }
        auto pS0 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject0->Lookup("S"_ostr));
        if (!pS0 || pS0->GetValue() != "Document")
        {
            continue;
        }
        auto pKids0 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject0->Lookup("K"_ostr));
        CPPUNIT_ASSERT(pKids0);

        for (const auto& pKid0 : pKids0->GetElements())
        {
            auto pRefKid0 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKid0);
            CPPUNIT_ASSERT(pRefKid0);
            auto pObject1 = pRefKid0->LookupObject();
            CPPUNIT_ASSERT(pObject1);
            auto pType1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("Type"_ostr));
            CPPUNIT_ASSERT(pType1);

            if (pType1 && pType1->GetValue() == "StructElem")
            {
                auto pS1 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1->Lookup("S"_ostr));
                if (pS1 && pS1->GetValue() == "L")
                {
                    ++nL;
                    auto pKids1 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids1);
                    // this is purely structural so there should be 1 child
                    CPPUNIT_ASSERT_EQUAL(size_t(1), pKids1->GetElements().size());

                    auto pRefKid11 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid11);
                    auto pObject11 = pRefKid11->LookupObject();
                    CPPUNIT_ASSERT(pObject11);
                    auto pType11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType11);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType11->GetValue());
                    auto pS11 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS11);
                    CPPUNIT_ASSERT_EQUAL("LI"_ostr, pS11->GetValue());
                    // LI has 2 children: Lbl and LBody
                    auto pKids11 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject11->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids11);
                    CPPUNIT_ASSERT_EQUAL(size_t(2), pKids11->GetElements().size());

                    auto pRefKid111 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids11->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid111);
                    auto pObject111 = pRefKid111->LookupObject();
                    CPPUNIT_ASSERT(pObject111);
                    auto pType111 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject111->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType111);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType111->GetValue());
                    auto pS111 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject111->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS111);
                    CPPUNIT_ASSERT_EQUAL("Lbl"_ostr, pS111->GetValue());
                    // Lbl has 2 children: the first 2 mcids (in order)
                    auto pKids111 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject111->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids111);
                    CPPUNIT_ASSERT_EQUAL(size_t(2), pKids111->GetElements().size());

                    auto pRefKid1111 = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids111->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid1111);
                    CPPUNIT_ASSERT_EQUAL(mcids[0], int(pRefKid1111->GetValue()));
                    auto pRefKid1112 = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids111->GetElements()[1]);
                    CPPUNIT_ASSERT(pRefKid1112);
                    CPPUNIT_ASSERT_EQUAL(mcids[1], int(pRefKid1112->GetValue()));

                    auto pRefKid112 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids11->GetElements()[1]);
                    CPPUNIT_ASSERT(pRefKid112);
                    auto pObject112 = pRefKid112->LookupObject();
                    CPPUNIT_ASSERT(pObject112);
                    auto pType112 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject112->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType112);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType112->GetValue());
                    auto pS112 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject112->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS112);
                    CPPUNIT_ASSERT_EQUAL("LBody"_ostr, pS112->GetValue());
                    // LBody has 2 children: paragraph and nested L (in order)
                    auto pKids112 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject112->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids112);
                    CPPUNIT_ASSERT_EQUAL(size_t(2), pKids112->GetElements().size());

                    auto pRefKid1121 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids112->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid1121);
                    auto pObject1121 = pRefKid1121->LookupObject();
                    CPPUNIT_ASSERT(pObject1121);
                    auto pType1121 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1121->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType1121);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1121->GetValue());
                    auto pS1121 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1121->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS1121);
                    CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS1121->GetValue());

                    auto pRefKid1122 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids112->GetElements()[1]);
                    CPPUNIT_ASSERT(pRefKid1122);
                    auto pObject1122 = pRefKid1122->LookupObject();
                    CPPUNIT_ASSERT(pObject1122);
                    auto pType1122 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1122->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType1122);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1122->GetValue());
                    auto pS1122 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1122->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS1122);
                    CPPUNIT_ASSERT_EQUAL("L"_ostr, pS1122->GetValue());
                    auto pKids1122 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1122->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids1122);
                    // this is purely structural so there should be 1 child
                    CPPUNIT_ASSERT_EQUAL(size_t(1), pKids1122->GetElements().size());

                    auto pRefKid11221 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1122->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid11221);
                    auto pObject11221 = pRefKid11221->LookupObject();
                    CPPUNIT_ASSERT(pObject11221);
                    auto pType11221 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11221->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType11221);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType11221->GetValue());
                    auto pS11221 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11221->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS11221);
                    CPPUNIT_ASSERT_EQUAL("LI"_ostr, pS11221->GetValue());
                    // LI has 2 children: Lbl and LBody
                    auto pKids11221 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject11221->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids11221);
                    CPPUNIT_ASSERT_EQUAL(size_t(2), pKids11221->GetElements().size());

                    auto pRefKid112211 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids11221->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid112211);
                    auto pObject112211 = pRefKid112211->LookupObject();
                    CPPUNIT_ASSERT(pObject112211);
                    auto pType112211 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject112211->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType112211);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType112211->GetValue());
                    auto pS112211 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject112211->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS112211);
                    CPPUNIT_ASSERT_EQUAL("Lbl"_ostr, pS112211->GetValue());
                    // Lbl has 2 children: the first 2 mcids (in order)
                    auto pKids112211 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject112211->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids112211);
                    CPPUNIT_ASSERT_EQUAL(size_t(2), pKids112211->GetElements().size());

                    auto pRefKid1122111 = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids112211->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid1122111);
                    CPPUNIT_ASSERT_EQUAL(mcids[2], int(pRefKid1122111->GetValue()));
                    auto pRefKid1122112 = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids112211->GetElements()[1]);
                    CPPUNIT_ASSERT(pRefKid1122112);
                    CPPUNIT_ASSERT_EQUAL(mcids[3], int(pRefKid1122112->GetValue()));

                    auto pRefKid112212 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids11221->GetElements()[1]);
                    CPPUNIT_ASSERT(pRefKid112212);
                    auto pObject112212 = pRefKid112212->LookupObject();
                    CPPUNIT_ASSERT(pObject112212);
                    auto pType112212 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject112212->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType112212);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType112212->GetValue());
                    auto pS112212 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject112212->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS112212);
                    CPPUNIT_ASSERT_EQUAL("LBody"_ostr, pS112212->GetValue());
                    // LBody has 2 children: paragraph and nested L (in order)
                    auto pKids112212 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject112212->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids112212);
                    CPPUNIT_ASSERT_EQUAL(size_t(2), pKids112212->GetElements().size());

                    auto pRefKid1122121 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids112212->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid1122121);
                    auto pObject1122121 = pRefKid1122121->LookupObject();
                    CPPUNIT_ASSERT(pObject1122121);
                    auto pType1122121 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1122121->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType1122121);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1122121->GetValue());
                    auto pS1122121 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1122121->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS1122121);
                    CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS1122121->GetValue());

                    auto pRefKid1122122 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids112212->GetElements()[1]);
                    CPPUNIT_ASSERT(pRefKid1122122);
                    auto pObject1122122 = pRefKid1122122->LookupObject();
                    CPPUNIT_ASSERT(pObject1122122);
                    auto pType1122122 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1122122->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType1122122);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1122122->GetValue());
                    auto pS1122122 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1122122->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS1122122);
                    CPPUNIT_ASSERT_EQUAL("L"_ostr, pS1122122->GetValue());
                    auto pKids1122122 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject1122122->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids1122122);
                    // this is purely structural so there should be 1 child
                    CPPUNIT_ASSERT_EQUAL(size_t(1), pKids1122122->GetElements().size());

                    auto pRefKid11221221 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids1122122->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid11221221);
                    auto pObject11221221 = pRefKid11221221->LookupObject();
                    CPPUNIT_ASSERT(pObject11221221);
                    auto pType11221221 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11221221->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType11221221);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType11221221->GetValue());
                    auto pS11221221 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject11221221->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS11221221);
                    CPPUNIT_ASSERT_EQUAL("LI"_ostr, pS11221221->GetValue());
                    // LI has 2 children: Lbl and LBody
                    auto pKids11221221 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject11221221->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids11221221);
                    CPPUNIT_ASSERT_EQUAL(size_t(2), pKids11221221->GetElements().size());

                    auto pRefKid112212211 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids11221221->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid112212211);
                    auto pObject112212211 = pRefKid112212211->LookupObject();
                    CPPUNIT_ASSERT(pObject112212211);
                    auto pType112212211 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject112212211->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType112212211);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType112212211->GetValue());
                    auto pS112212211 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject112212211->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS112212211);
                    CPPUNIT_ASSERT_EQUAL("Lbl"_ostr, pS112212211->GetValue());
                    // Lbl has 2 children: the first 2 mcids (in order)
                    auto pKids112212211 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject112212211->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids112212211);
                    CPPUNIT_ASSERT_EQUAL(size_t(2), pKids112212211->GetElements().size());

                    auto pRefKid1122122111 = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids112212211->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid1122122111);
                    CPPUNIT_ASSERT_EQUAL(mcids[4], int(pRefKid1122122111->GetValue()));
                    auto pRefKid1122122112 = dynamic_cast<vcl::filter::PDFNumberElement*>(pKids112212211->GetElements()[1]);
                    CPPUNIT_ASSERT(pRefKid1122122112);
                    CPPUNIT_ASSERT_EQUAL(mcids[5], int(pRefKid1122122112->GetValue()));

                    auto pRefKid112212212 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids11221221->GetElements()[1]);
                    CPPUNIT_ASSERT(pRefKid112212212);
                    auto pObject112212212 = pRefKid112212212->LookupObject();
                    CPPUNIT_ASSERT(pObject112212212);
                    auto pType112212212 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject112212212->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType112212212);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType112212212->GetValue());
                    auto pS112212212 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject112212212->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS112212212);
                    CPPUNIT_ASSERT_EQUAL("LBody"_ostr, pS112212212->GetValue());
                    // inner LBody has 1 children: paragraph
                    auto pKids112212212 = dynamic_cast<vcl::filter::PDFArrayElement*>(pObject112212212->Lookup("K"_ostr));
                    CPPUNIT_ASSERT(pKids112212212);
                    CPPUNIT_ASSERT_EQUAL(size_t(1), pKids112212212->GetElements().size());

                    auto pRefKid1122122121 = dynamic_cast<vcl::filter::PDFReferenceElement*>(pKids112212212->GetElements()[0]);
                    CPPUNIT_ASSERT(pRefKid1122122121);
                    auto pObject1122122121 = pRefKid1122122121->LookupObject();
                    CPPUNIT_ASSERT(pObject1122122121);
                    auto pType1122122121 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1122122121->Lookup("Type"_ostr));
                    CPPUNIT_ASSERT(pType1122122121);
                    CPPUNIT_ASSERT_EQUAL("StructElem"_ostr, pType1122122121->GetValue());
                    auto pS1122122121 = dynamic_cast<vcl::filter::PDFNameElement*>(pObject1122122121->Lookup("S"_ostr));
                    CPPUNIT_ASSERT(pS1122122121);
                    CPPUNIT_ASSERT_EQUAL("Standard"_ostr, pS1122122121->GetValue());
                }
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nL)>(1), nL);
}

CPPUNIT_TEST_FIXTURE(Test, testTableOfContentLinksHaveContentSet)
{
    // Test for tdf#167409

    // TOC is expected to have alt. text set (written to /Contents key), PDF/UA conformance tests
    // will fail. TOC links can't be set by the user.

    createSwDoc("SimpleTOC.odt");

    // Let's update TOC first
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pWrtShell->GetTOXCount());
    const SwTOXBase* pTOX = pWrtShell->GetTOX(0);
    CPPUNIT_ASSERT(pTOX);
    pWrtShell->UpdateTableOf(*pTOX);

    // Export as PDF
    comphelper::SequenceAsHashMap aMediaDescriptor;
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();

    // Non-NULL pPdfDocument means pdfium is available.
    if (!pPdfDocument)
        return;

    // The document has one page.
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // The page has one annotation.
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getAnnotationCount());
    std::unique_ptr<vcl::pdf::PDFiumAnnotation> pAnnotation = pPdfPage->getAnnotation(0);
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFAnnotationSubType::Link, pAnnotation->getSubType());
    CPPUNIT_ASSERT(pAnnotation->hasKey("Contents"_ostr));
    CPPUNIT_ASSERT_EQUAL(vcl::pdf::PDFObjectType::String,
                         pAnnotation->getValueType("Contents"_ostr));
    OUString aContent = pAnnotation->getString("Contents"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr, aContent);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143311)
{
    createSwDoc("tdf143311-1.docx");
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), u"Decorative"_ustr));
    {
        // add another one that's a SdrObject
        uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xShape(
            xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
        xShapeProps->setPropertyValue(u"Decorative"_ustr, uno::Any(true));
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage());
        xDrawPage->add(xShape);
    }
    // check DOCX filters
    saveAndReload(TestFilter::DOCX);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), u"Decorative"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(2), u"Decorative"_ustr));
    {
        // tdf#153925 not imported - check default and set it to test ODF filters
        uno::Reference<beans::XPropertySet> const xStyle(getStyles(u"FrameStyles"_ustr)->getByName(u"Formula"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xStyle, u"Decorative"_ustr));
        xStyle->setPropertyValue(u"Decorative"_ustr, uno::Any(true));
    }
    // check ODF filters
    saveAndReload(TestFilter::ODT);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), u"Decorative"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(2), u"Decorative"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getStyles(u"FrameStyles"_ustr)->getByName(u"Formula"_ustr), u"Decorative"_ustr));

    // check PDF export
    comphelper::SequenceAsHashMap aMediaDescriptor;
    // Enable PDF/UA
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "PDFUACompliance", uno::Any(true) } }));
    aMediaDescriptor[u"FilterData"_ustr] <<= aFilterData;
    save(TestFilter::PDF_WRITER, aMediaDescriptor.getAsConstPropertyValueList());

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    enum
    {
        Default,
        Artifact,
        Tagged
    } state
        = Default;

    auto nLine(0);
    auto nTagged(0);
    auto nArtifacts(0);
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n";
            if (line == "/Artifact BMC")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Artifact;
                ++nArtifacts;
            }
            else if (o3tl::starts_with(line, "/Standard<</MCID") && o3tl::ends_with(line, ">>BDC"))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Tagged;
                ++nTagged;
            }
            else if (line == "EMC")
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected end", state != Default);
                state = Default;
            }
            else if (nLine > 1) // first line is expected "0.1 w"
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected content outside MCS", state != Default);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("unclosed MCS", Default, state);
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nTagged)>(25), nTagged); // text in body
    // 1 decorative image + 1 decorative shape + 1 pre-existing rectangle border or something
    CPPUNIT_ASSERT(nArtifacts >= 3);
}

void Test::testTextFormField(TestFilter eFilterName)
{
    createSwDoc("text_form_field.odt");

    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check the document after round trip
    SwDoc* pDoc = getSwDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();

    // We have two text form fields
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(2), pMarkAccess->getAllMarksCount());

    // Check whether all fieldmarks are text form fields
    for(auto aIter = pMarkAccess->getAllMarksBegin(); aIter != pMarkAccess->getAllMarksEnd(); ++aIter)
    {
        ::sw::mark::Fieldmark* pFieldmark = dynamic_cast<::sw::mark::Fieldmark*>(*aIter);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pFieldmark);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), ODF_FORMTEXT, pFieldmark->GetFieldname());
    }

    // In the first paragraph we have an empty text form field with the placeholder spaces
    const uno::Reference< text::XTextRange > xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"TextFieldStart"_ustr, getProperty<OUString>(getRun(xPara, 1), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"TextFieldSeparator"_ustr, getProperty<OUString>(getRun(xPara, 2), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"Text"_ustr, getProperty<OUString>(getRun(xPara, 3), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), vEnSpaces, getRun(xPara, 3)->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"TextFieldEnd"_ustr, getProperty<OUString>(getRun(xPara, 4), u"TextPortionType"_ustr));

    // In the second paragraph we have a set text
    const uno::Reference< text::XTextRange > xPara2 = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"TextFieldStart"_ustr, getProperty<OUString>(getRun(xPara2, 1), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"TextFieldSeparator"_ustr, getProperty<OUString>(getRun(xPara2, 2), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"Text"_ustr, getProperty<OUString>(getRun(xPara2, 3), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"xxxxx"_ustr, getRun(xPara2, 3)->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"TextFieldEnd"_ustr, getProperty<OUString>(getRun(xPara2, 4), u"TextPortionType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTextFormField_ODT)
{
    testTextFormField(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testTextFormField_DOC)
{
    testTextFormField(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testTextFormField_DOCX)
{
    testTextFormField(TestFilter::DOCX);
}

void Test::testCheckBoxFormField(TestFilter eFilterName)
{
    createSwDoc("checkbox_form_field.odt");

    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check the document after round trip
    SwDoc* pDoc = getSwDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();

    // We have two check box form fields
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(2), pMarkAccess->getAllMarksCount());

    int nIndex = 0;
    for(auto aIter = pMarkAccess->getAllMarksBegin(); aIter != pMarkAccess->getAllMarksEnd(); ++aIter)
    {
        ::sw::mark::Fieldmark* pFieldmark = dynamic_cast<::sw::mark::Fieldmark*>(*aIter);

        if(eFilterName == TestFilter::DOCX) // OOXML import also generates bookmarks
        {
            if(!pFieldmark)
                continue;
        }

        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pFieldmark);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), ODF_FORMCHECKBOX, pFieldmark->GetFieldname());
        ::sw::mark::CheckboxFieldmark* pCheckBox = dynamic_cast< ::sw::mark::CheckboxFieldmark* >(pFieldmark);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pCheckBox);

        // The first one is unchecked, the other one is checked
        if(nIndex == 0)
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), !pCheckBox->IsChecked());
        else
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pCheckBox->IsChecked());
        ++nIndex;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), int(2), nIndex);
}

CPPUNIT_TEST_FIXTURE(Test, testCheckBoxFormField_ODT)
{
    testCheckBoxFormField(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testCheckBoxFormField_DOC)
{
    testCheckBoxFormField(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testCheckBoxFormField_DOCX)
{
    testCheckBoxFormField(TestFilter::DOCX);
}

void Test::testDropDownFormField(TestFilter eFilterName)
{
    createSwDoc("dropdown_form_field.odt");

    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check the document after round trip
    SwDoc* pDoc = getSwDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(2), pMarkAccess->getAllMarksCount());

    int nIndex = 0;
    for(auto aIter = pMarkAccess->getAllMarksBegin(); aIter != pMarkAccess->getAllMarksEnd(); ++aIter)
    {
        ::sw::mark::Fieldmark* pFieldmark = dynamic_cast<::sw::mark::Fieldmark*>(*aIter);

        if(!pFieldmark)
            continue;

        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pFieldmark);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), ODF_FORMDROPDOWN, pFieldmark->GetFieldname());

        // Check drop down field's parameters.
        const sw::mark::Fieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
        css::uno::Sequence<OUString> vListEntries;
        sal_Int32 nSelection = -1;
        auto pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
        if (pListEntries != pParameters->end())
        {
            pListEntries->second >>= vListEntries;

            if(vListEntries.hasElements())
            {
                auto pResult = pParameters->find(ODF_FORMDROPDOWN_RESULT);
                if (pResult != pParameters->end())
                {
                    pResult->second >>= nSelection;
                }
            }
        }

        // The first one is empty
        if(nIndex == 0)
        {
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), !vListEntries.hasElements());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(-1), nSelection);
        }
        else // The second one has list and also a selected item
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(4), vListEntries.getLength());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(1), nSelection);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"1000"_ustr, vListEntries[0]);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"2000"_ustr, vListEntries[1]);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"3000"_ustr, vListEntries[2]);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"4000"_ustr, vListEntries[3]);
        }
        ++nIndex;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), int(2), nIndex);
}

CPPUNIT_TEST_FIXTURE(Test, testDropDownFormField_ODT)
{
    testDropDownFormField(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testDropDownFormField_RTF)
{
    testDropDownFormField(TestFilter::RTF);
}

CPPUNIT_TEST_FIXTURE(Test, testDropDownFormField_DOC)
{
    testDropDownFormField(TestFilter::DOC);
}

CPPUNIT_TEST_FIXTURE(Test, testDropDownFormField_DOCX)
{
    testDropDownFormField(TestFilter::DOCX);
}

void Test::testDateFormField(TestFilter eFilterName)
{
    createSwDoc("date_form_field.odt");

    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check the document after round trip
    if (eFilterName == TestFilter::ODT)
    {
        SwDoc* pDoc = getSwDoc();
        IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();

        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(5), pMarkAccess->getAllMarksCount());

        int nIndex = 0;
        for(auto aIter = pMarkAccess->getAllMarksBegin(); aIter != pMarkAccess->getAllMarksEnd(); ++aIter)
        {
            ::sw::mark::DateFieldmark* pFieldmark = dynamic_cast<::sw::mark::DateFieldmark*>(*aIter);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pFieldmark);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), ODF_FORMDATE, pFieldmark->GetFieldname());

            // Check date form field's parameters.
            const sw::mark::Fieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
            OUString sDateFormat;
            auto pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT);
            if (pResult != pParameters->end())
            {
                pResult->second >>= sDateFormat;
            }

            OUString sLang;
            pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT_LANGUAGE);
            if (pResult != pParameters->end())
            {
                pResult->second >>= sLang;
            }

            OUString sCurrentDate = pFieldmark->GetContent();

            // The first one has the default field content
            if(nIndex == 0)
            {

                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"MM/DD/YY"_ustr, sDateFormat);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"en-US"_ustr, sLang);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), vEnSpaces, sCurrentDate);

                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), SwNodeOffset(9), pFieldmark->GetMarkStart().GetNodeIndex());
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(5), pFieldmark->GetMarkStart().GetContentIndex());
            }
            else if (nIndex == 1) // The second has the default format
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"MM/DD/YY"_ustr, sDateFormat);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"en-US"_ustr, sLang);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"06/12/19"_ustr, sCurrentDate);

                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), SwNodeOffset(9), pFieldmark->GetMarkStart().GetNodeIndex());
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(20), pFieldmark->GetMarkStart().GetContentIndex());
            }
            else if (nIndex == 2) // The third one has special format
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"[NatNum12 MMMM=abbreviation]YYYY\". \"MMMM D."_ustr, sDateFormat);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"hu-HU"_ustr, sLang);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"2019. febr. 12."_ustr, sCurrentDate);

                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), SwNodeOffset(9), pFieldmark->GetMarkStart().GetNodeIndex());
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(40), pFieldmark->GetMarkStart().GetContentIndex());

            }
            else if (nIndex == 3) // The fourth one has placeholder text
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"D, MMM YY"_ustr, sDateFormat);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"bm-ML"_ustr, sLang);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"[select date]"_ustr, sCurrentDate);

                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), SwNodeOffset(9), pFieldmark->GetMarkStart().GetNodeIndex());
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(62), pFieldmark->GetMarkStart().GetContentIndex());

            }
            else // The last one is really empty
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"MM/DD/YY"_ustr, sDateFormat);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"en-US"_ustr, sLang);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u""_ustr, sCurrentDate);

                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), SwNodeOffset(9), pFieldmark->GetMarkStart().GetNodeIndex());
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(82), pFieldmark->GetMarkStart().GetContentIndex());

            }
            ++nIndex;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), int(5), nIndex);
    }
    else
    {
        // Import from DOCX, so the fieldmark is now a content control.
        uno::Reference<container::XEnumerationAccess> xEnumAccess(getParagraph(1), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xTextPortions = xEnumAccess->createEnumeration();

        int nIndex = 0;
        while (xTextPortions->hasMoreElements())
        {
            uno::Reference<beans::XPropertySet> xTextPortion(xTextPortions->nextElement(), uno::UNO_QUERY);
            OUString aPortionType;
            xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
            if (aPortionType != "ContentControl")
            {
                continue;
            }

            uno::Reference<text::XTextContent> xContentControl;
            xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
            uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);

            bool bDate{};
            xContentControlProps->getPropertyValue(u"Date"_ustr) >>= bDate;
            CPPUNIT_ASSERT(bDate);

            // Check date form field's parameters.
            OUString sDateFormat;
            xContentControlProps->getPropertyValue(u"DateFormat"_ustr) >>= sDateFormat;

            OUString sLang;
            xContentControlProps->getPropertyValue(u"DateLanguage"_ustr) >>= sLang;

            uno::Reference<container::XEnumerationAccess> xContentControlEnumAccess(xContentControl,
                    uno::UNO_QUERY);
            uno::Reference<container::XEnumeration> xContentControlEnum
                = xContentControlEnumAccess->createEnumeration();
            uno::Reference<text::XTextRange> xContentControlTextPortion(xContentControlEnum->nextElement(), uno::UNO_QUERY);
            OUString sCurrentDate = xContentControlTextPortion->getString();

            // The first one has the default field content
            if(nIndex == 0)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"MM/DD/YY"_ustr, sDateFormat);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"en-US"_ustr, sLang);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), vEnSpaces, sCurrentDate);
            }
            else if (nIndex == 1) // The second has the default format
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"MM/DD/YY"_ustr, sDateFormat);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"en-US"_ustr, sLang);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"06/12/19"_ustr, sCurrentDate);
            }
            else if (nIndex == 2) // The third one has special format
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"[NatNum12 MMMM=abbreviation]YYYY\". \"MMMM D."_ustr, sDateFormat);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"hu-HU"_ustr, sLang);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"2019. febr. 12."_ustr, sCurrentDate);
            }
            else if (nIndex == 3) // The fourth one has placeholder text
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"D, MMM YY"_ustr, sDateFormat);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"bm-ML"_ustr, sLang);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"[select date]"_ustr, sCurrentDate);
            }
            else // The last one is really empty
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"MM/DD/YY"_ustr, sDateFormat);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"en-US"_ustr, sLang);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u""_ustr, sCurrentDate);
            }
            ++nIndex;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), int(5), nIndex);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDateFormField_ODT)
{
    testDateFormField(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testDateFormField_DOCX)
{
    testDateFormField(TestFilter::DOCX);
}

void Test::testDateFormFieldCharacterFormatting(TestFilter eFilterName)
{
    createSwDoc("date_form_field_char_formatting.odt");

    const OString sFailedMessage = OString::Concat("Failed on filter: ") + TestFilterNames.at(eFilterName).toUtf8();

    // Export the document and import again for a check
    saveAndReload(eFilterName);

    // Check the document after round trip
    if (eFilterName == TestFilter::ODT)
    {
        SwDoc* pDoc = getSwDoc();
        IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();

        // Check that we have the field at the right place
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(1), pMarkAccess->getAllMarksCount());
        ::sw::mark::DateFieldmark* pFieldmark = dynamic_cast<::sw::mark::DateFieldmark*>(*pMarkAccess->getAllMarksBegin());
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pFieldmark);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), ODF_FORMDATE, pFieldmark->GetFieldname());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(0), pFieldmark->GetMarkStart().GetContentIndex());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_Int32(11), pFieldmark->GetMarkEnd().GetContentIndex());

        // We have one date field, first half of the field has bold character weight and second part has red character color
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(1), 3), u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_AUTO, getProperty<Color>(getRun(getParagraph(1), 3), u"CharColor"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), awt::FontWeight::NORMAL, getProperty<float>(getRun(getParagraph(1), 4), u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_LIGHTRED, getProperty<Color>(getRun(getParagraph(1), 4), u"CharColor"_ustr));
    }
    else
    {
        uno::Reference<beans::XPropertySet> xTextPortion(getRun(getParagraph(1), 1), uno::UNO_QUERY);
        OUString aPortionType;
        xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), u"ContentControl"_ustr, aPortionType);

        uno::Reference<text::XTextContent> xContentControl;
        xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        bool bDate{};
        xContentControlProps->getPropertyValue(u"Date"_ustr) >>= bDate;
        CPPUNIT_ASSERT(bDate);

        uno::Reference<container::XEnumerationAccess> xContentControlEnumAccess(xContentControl,
                                                                                uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xContentControlEnum
            = xContentControlEnumAccess->createEnumeration();
        xTextPortion.set(xContentControlEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), awt::FontWeight::BOLD, getProperty<float>(xTextPortion, u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_AUTO, getProperty<Color>(xTextPortion, u"CharColor"_ustr));
        xTextPortion.set(xContentControlEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), awt::FontWeight::NORMAL, getProperty<float>(xTextPortion, u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), COL_LIGHTRED, getProperty<Color>(xTextPortion, u"CharColor"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDateFormFieldCharacterFormatting_ODT)
{
    testDateFormFieldCharacterFormatting(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testDateFormFieldCharacterFormatting_DOCX)
{
    testDateFormFieldCharacterFormatting(TestFilter::DOCX);
}

void Test::testSvgImageSupport(TestFilter eFilterName)
{
    // Use case to import a document containing a SVG image, export in target format, import and check if the
    // SVG image is present and as expected in the document

    // Import ODT file
    createSwDoc("SvgImageTest.odt");

    // Export the document in target format and import again
    saveAndReload(eFilterName);

    // Prepare fail message (writing which import/export filter was used)
    const OString sFailedMessage = "Failed on filter: "_ostr + TestFilterNames.at(eFilterName).toUtf8();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), 1, getShapes());

    // Get the image
    uno::Reference<drawing::XShape> xImage(getShape(1), uno::UNO_QUERY);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), VectorGraphicDataType::Svg, pVectorGraphic->getType());
}

CPPUNIT_TEST_FIXTURE(Test, testSvgImageSupport_ODT)
{
    testSvgImageSupport(TestFilter::ODT);
}

CPPUNIT_TEST_FIXTURE(Test, testSvgImageSupport_DOCX)
{
    testSvgImageSupport(TestFilter::DOCX);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
