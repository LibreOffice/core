/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <officecfg/Office/Common.hxx>
#include "sdmodeltestbase.hxx"
#include "Outliner.hxx"
#include <test/xmltesttools.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <svl/stritem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/bulletitem.hxx>

#include <oox/drawingml/drawingmltypes.hxx>

#include <rsc/rscsfx.hxx>


#include <svx/svdoutl.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdoole2.hxx>
#include <svx/xflclit.hxx>
#include <animations/animationnodehelper.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>


#include <svx/svdotable.hxx>

#include <config_features.h>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

CPPUNIT_NS_BEGIN

template<> struct assertion_traits<Color>
{
    static bool equal( const Color& c1, const Color& c2 )
    {
        return c1 == c2;
    }

    static std::string toString( const Color& c )
    {
        OStringStream ost;
        ost << static_cast<unsigned int>(c.GetColor());
        return ost.str();
    }
};

CPPUNIT_NS_END

using namespace css;
using namespace css::animations;

class SdExportTest : public SdModelTestBase, public XmlTestTools
{
public:
    void testN821567();
    void testBnc870233_1();
    void testBnc870233_2();
    void testN828390_4();
    void testN828390_5();
    void testMediaEmbedding();
    void testFdo71961();
    void testFdo84043();
    void testN828390();
    void testBnc880763();
    void testBnc862510_5();
    void testBnc822347_EmptyBullet();
    void testFdo83751();
    void testFdo79731();
    void testSwappedOutImageExport();
    void testOOoXMLAnimations();
    void testTdf80020();
    void testLinkedGraphicRT();
    void testImageWithSpecialID();
    void testTableCellFillProperties();
    void testBulletStartNumber();
    void testLineStyle();
    void testCellLeftAndRightMargin();
    void testRightToLeftParaghraph();
    void testTextboxWithHyperlink();
    void testTableCellBorder();
    void testBulletColor();
    void testTdf62176();
    void testBulletCharAndFont();
    void testBulletMarginAndIndentation();
    void testParaMarginAndindentation();
    void testTransparentBackground();
    void testExportTransitionsPPTX();

    void testFdo90607();
    void testTdf91378();
    void testBnc822341();
    void testMathObject();
    void testMathObjectPPT2010();
    void testTdf80224();
    void testTdf92527();
    void testTdf99224();

    CPPUNIT_TEST_SUITE(SdExportTest);
    CPPUNIT_TEST(testFdo90607);
    CPPUNIT_TEST(testN821567);
    CPPUNIT_TEST(testBnc870233_1);
    CPPUNIT_TEST(testBnc870233_2);
    CPPUNIT_TEST(testN828390_4);
    CPPUNIT_TEST(testN828390_5);
    CPPUNIT_TEST(testMediaEmbedding);
    CPPUNIT_TEST(testFdo71961);
    CPPUNIT_TEST(testFdo84043);
    CPPUNIT_TEST(testN828390);
    CPPUNIT_TEST(testBnc880763);
    CPPUNIT_TEST(testBnc862510_5);
    CPPUNIT_TEST(testBnc822347_EmptyBullet);
    CPPUNIT_TEST(testFdo83751);
    CPPUNIT_TEST(testFdo79731);
    CPPUNIT_TEST(testSwappedOutImageExport);
    CPPUNIT_TEST(testOOoXMLAnimations);
    CPPUNIT_TEST(testTdf80020);
    CPPUNIT_TEST(testLinkedGraphicRT);
    CPPUNIT_TEST(testImageWithSpecialID);
    CPPUNIT_TEST(testTableCellFillProperties);
    CPPUNIT_TEST(testBulletStartNumber);
    CPPUNIT_TEST(testLineStyle);
    CPPUNIT_TEST(testCellLeftAndRightMargin);
    CPPUNIT_TEST(testRightToLeftParaghraph);
    CPPUNIT_TEST(testTextboxWithHyperlink);
    CPPUNIT_TEST(testTableCellBorder);
    CPPUNIT_TEST(testBulletColor);
    CPPUNIT_TEST(testTdf62176);
    CPPUNIT_TEST(testBulletCharAndFont);
    CPPUNIT_TEST(testBulletMarginAndIndentation);
    CPPUNIT_TEST(testParaMarginAndindentation);
    CPPUNIT_TEST(testTransparentBackground);
    CPPUNIT_TEST(testTdf91378);

    CPPUNIT_TEST(testBnc822341);
    CPPUNIT_TEST(testMathObject);
    CPPUNIT_TEST(testMathObjectPPT2010);
    CPPUNIT_TEST(testTdf80224);

    CPPUNIT_TEST(testExportTransitionsPPTX);
    CPPUNIT_TEST(testTdf92527);
    CPPUNIT_TEST(testTdf99224);

    CPPUNIT_TEST_SUITE_END();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        struct { char const * pPrefix; char const * pURI; } namespaces[] =
        {
            // ODF
            { "anim", "urn:oasis:names:tc:opendocument:xmlns:animation:1.0" },
            { "draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" },
            { "fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" },
            { "number", "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" },
            { "office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0" },
            { "presentation", "urn:oasis:names:tc:opendocument:xmlns:presentation:1.0" },
            { "style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0" },
            { "svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" },
            { "table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0" },
            { "text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0" },
            { "xlink", "http://www.w3c.org/1999/xlink" },
            // OOXML
            { "ContentType", "http://schemas.openxmlformats.org/package/2006/content-types" },
            { "rels", "http://schemas.openxmlformats.org/package/2006/relationships" },
            { "mc", "http://schemas.openxmlformats.org/markup-compatibility/2006" },
            { "v", "urn:schemas-microsoft-com:vml" },
            { "a", "http://schemas.openxmlformats.org/drawingml/2006/main" },
            { "c", "http://schemas.openxmlformats.org/drawingml/2006/chart" },
            { "m", "http://schemas.openxmlformats.org/officeDocument/2006/math" },
            { "pic", "http://schemas.openxmlformats.org/drawingml/2006/picture" },
            { "wp", "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" },
            { "p", "http://schemas.openxmlformats.org/presentationml/2006/main" },
            { "w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main" },
            { "a14", "http://schemas.microsoft.com/office/drawing/2010/main" },
            { "wps", "http://schemas.microsoft.com/office/word/2010/wordprocessingShape" },
            { "wpg", "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" },
        };
        for (size_t i = 0; i < SAL_N_ELEMENTS(namespaces); ++i)
        {
            xmlXPathRegisterNs(pXmlXPathCtx,
                reinterpret_cast<xmlChar const *>(namespaces[i].pPrefix),
                reinterpret_cast<xmlChar const *>(namespaces[i].pURI));
        }
    }

    xmlDocPtr parseExport(utl::TempFile & rTempFile, OUString const& rStreamName)
    {
        OUString const url(rTempFile.GetURL());
        uno::Reference<packages::zip::XZipFileAccess2> const xZipNames(
            packages::zip::ZipFileAccess::createWithURL(
                comphelper::getComponentContext(m_xSFactory), url));
        uno::Reference<io::XInputStream> const xInputStream(
            xZipNames->getByName(rStreamName), uno::UNO_QUERY);
        std::unique_ptr<SvStream> const pStream(
            utl::UcbStreamHelper::CreateStream(xInputStream, true));
        xmlDocPtr const pXmlDoc = parseXmlStream(pStream.get());
        pXmlDoc->name = reinterpret_cast<char *>(xmlStrdup(
            reinterpret_cast<xmlChar const *>(OUStringToOString(url, RTL_TEXTENCODING_UTF8).getStr())));
        return pXmlDoc;
    }
};

void SdExportTest::testN821567()
{
    OUString bgImage;
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n821567.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef, ODP );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "not exactly one page", xDoc->getDrawPages()->getCount() == 1 );
    uno::Reference< drawing::XDrawPage > xPage( getPage( 0, xDocShRef ) );

    uno::Reference< beans::XPropertySet > xPropSet( xPage, uno::UNO_QUERY );
    uno::Any aAny = xPropSet->getPropertyValue( "Background" );
    if(aAny.hasValue())
    {
        uno::Reference< beans::XPropertySet > aXBackgroundPropSet;
        aAny >>= aXBackgroundPropSet;
        aAny = aXBackgroundPropSet->getPropertyValue( "FillBitmapName" );
        aAny >>= bgImage;
    }
    CPPUNIT_ASSERT_MESSAGE("Slide Background is not exported properly", !bgImage.isEmpty());

    xDocShRef->DoClose();
}

namespace {

template< typename ItemValue, typename ItemType >
void checkFontAttributes( const SdrTextObj* pObj, ItemValue nVal)
{
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
    std::vector<EECharAttrib> rLst;
    aEdit.GetCharAttribs(0, rLst);
    for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
    {
        const ItemType* pAttrib = dynamic_cast<const ItemType *>((*it).pAttr);
        if (pAttrib)
        {
            CPPUNIT_ASSERT_EQUAL( nVal, static_cast<ItemValue>(pAttrib->GetValue()));
        }
    }
}

}

void SdExportTest::testBnc870233_1()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc870233_1.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // The problem was all shapes had the same font (the last parsed font attribues overwrote all previous ones)

    // First shape has red, bold font
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0xff0000) );
        checkFontAttributes<FontWeight, SvxWeightItem>( pObj, WEIGHT_BOLD );
    }

    // Second shape has blue, italic font
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 1 ) );
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0x0000ff) );
        checkFontAttributes<FontItalic, SvxPostureItem>( pObj, ITALIC_NORMAL );
    }

    xDocShRef->DoClose();
}

void SdExportTest::testBnc870233_2()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc870233_2.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // The problem was in some SmartArts font color was wrong

    // First smart art has blue font color (direct formatting)
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0x0000ff) );
    }

    // Second smart art has "dk2" font color (style)
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 1 ) );
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0x1F497D) );
    }

    // Third smart art has white font color (style)
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 2 ) );
        checkFontAttributes<Color, SvxColorItem>( pObj, Color(0xffffff) );
    }

    xDocShRef->DoClose();
}

void SdExportTest::testN828390_4()
{
    bool bPassed = false;
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/n828390_4.odp"), ODP );

    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        std::vector<EECharAttrib> rLst;
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs(1, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxFontHeightItem * pFontHeight = dynamic_cast<const SvxFontHeightItem *>((*it).pAttr);
            if( pFontHeight )
                CPPUNIT_ASSERT_MESSAGE( "Font height is wrong", pFontHeight->GetHeight() == 1129 );
            const SvxFontItem *pFont = dynamic_cast<const SvxFontItem *>((*it).pAttr);
            if( pFont )
            {
                CPPUNIT_ASSERT_MESSAGE( "Font is wrong", pFont->GetFamilyName() == "Arial");
                bPassed = true;
            }
            const SvxWeightItem *pWeight = dynamic_cast<const SvxWeightItem *>((*it).pAttr);
            if( pWeight )
                CPPUNIT_ASSERT_MESSAGE( "Font Weight is wrong", pWeight->GetWeight() == WEIGHT_BOLD);
        }
    }
    CPPUNIT_ASSERT(bPassed);

    xDocShRef->DoClose();
}

void SdExportTest::testN828390_5()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/n828390_5.odp"), ODP );

    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        const SvxNumBulletItem *pNumFmt = dynamic_cast<const SvxNumBulletItem *>(aEdit.GetPool()->GetItem2(EE_PARA_NUMBULLET, 5));
        CPPUNIT_ASSERT( pNumFmt );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's relative size is wrong!", pNumFmt->GetNumRule()->GetLevel(1).GetBulletRelSize(), sal_uInt16(75) ); // != 25
    }

    xDocShRef->DoClose();
}

void SdExportTest::testTransparentBackground()
{
   ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/transparent_background.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef, ODP );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    const SdrTextObj *pObj1 = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
    checkFontAttributes<Color, SvxBackgroundColorItem>( pObj1, Color(COL_TRANSPARENT) );

    const SdrTextObj *pObj2 = dynamic_cast<SdrTextObj *>( pPage->GetObj( 1 ) );
    checkFontAttributes<Color, SvxBackgroundColorItem>( pObj2, Color(COL_YELLOW));
}

void SdExportTest::testMediaEmbedding()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/media_embedding.odp"), ODP);

#if HAVE_FEATURE_GLTF
    xDocShRef = saveAndReload( xDocShRef, ODP );
#endif

    const SdrPage *pPage = GetPage( 1, xDocShRef );

#if HAVE_FEATURE_GLTF
    // First object is a glTF model
    SdrMediaObj *pModelObj = dynamic_cast<SdrMediaObj*>( pPage->GetObj( 2 ));
    CPPUNIT_ASSERT_MESSAGE( "missing model", pModelObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "vnd.sun.star.Package:Models/duck/duck.json" ), pModelObj->getMediaProperties().getURL());
    CPPUNIT_ASSERT_EQUAL( OUString( "model/vnd.gltf+json" ), pModelObj->getMediaProperties().getMimeType());
#else
    // If glTF is not supported, then the fallback image is imported
    SdrGrafObj *pGrafic = dynamic_cast<SdrGrafObj*>( pPage->GetObj( 2 ));
    CPPUNIT_ASSERT_MESSAGE( "Could not load glTF fallback image", pGrafic != NULL);
    CPPUNIT_ASSERT_EQUAL( OUString( "vnd.sun.star.Package:Models/Fallbacks/duck.png" ), pGrafic->GetGrafStreamURL());
#endif

    // Second object is a sound
    SdrMediaObj *pMediaObj = dynamic_cast<SdrMediaObj*>( pPage->GetObj( 3 ));
    CPPUNIT_ASSERT_MESSAGE( "missing media object", pMediaObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "vnd.sun.star.Package:Media/button-1.wav" ), pMediaObj->getMediaProperties().getURL());
    CPPUNIT_ASSERT_EQUAL( OUString( "application/vnd.sun.star.media" ), pMediaObj->getMediaProperties().getMimeType());

    xDocShRef->DoClose();
}

void SdExportTest::testFdo84043()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo84043.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef, ODP );

    // the bug was duplicate attributes, causing crash in a build with asserts
    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrObject const* pShape = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);
}

void SdExportTest::testFdo71961()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo71961.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef, PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Export to .pptx changes all text frames to custom shape objects, which obey TextWordWrap property
    // (which is false for text frames otherwise and is ignored). Check that frames that should wrap still do.
    SdrObjCustomShape *pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 1 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "Text to be always wrapped" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( true, (static_cast<const SdrOnOffItem&>(pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 2 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom shape non-wrapped text" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( false, (static_cast<const SdrOnOffItem&>(pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 3 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom shape wrapped text" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( true, (static_cast<const SdrOnOffItem&>(pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue());

    xDocShRef->DoClose();
}

void SdExportTest::testN828390()
{
    bool bPassed = false;
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n828390.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        std::vector<EECharAttrib> rLst;
        // Get the object
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs(0, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxEscapementItem *pFontEscapement = dynamic_cast<const SvxEscapementItem *>((*it).pAttr);
            if(pFontEscapement)
            {
                if( pFontEscapement->GetEsc() == -25 )
                {
                    bPassed = true;
                    break;
                }
            }
        }
    }
    CPPUNIT_ASSERT_MESSAGE("Subscript not exported properly", bPassed);

    xDocShRef->DoClose();
}

void SdExportTest::testBnc880763()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc880763.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Check z-order of the two shapes, use background color to identify them
    // First object in the background has blue background color
    const SdrObject *pObj = dynamic_cast<SdrObject *>( pPage->GetObj( 0 ) );
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( sal_uInt32(0x0000ff),(static_cast< const XColorItem& >(pObj->GetMergedItem(XATTR_FILLCOLOR))).GetColorValue().GetColor());

    // Second object at the front has green background color
    pObj = dynamic_cast<SdrObject *>( pPage->GetObj( 1 ) );
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( sal_uInt32(0x00ff00),(static_cast< const XColorItem& >(pObj->GetMergedItem(XATTR_FILLCOLOR))).GetColorValue().GetColor());

    xDocShRef->DoClose();
}

void SdExportTest::testBnc862510_5()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc862510_5.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Same as testBnc870237, but here we check the horizontal spacing
    const SdrObject* pObj = dynamic_cast<SdrObject*>( pPage->GetObj( 1 ) );
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrMetricItem& >(pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrMetricItem& >(pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(7510), (static_cast< const SdrMetricItem& >(pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrMetricItem& >(pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST))).GetValue());

    xDocShRef->DoClose();
}

// In numbering a bullet could be defined as empty (no character).
// When exporting to OOXML make sure that the bullet is ignored and
// not written into the file.
void SdExportTest::testBnc822347_EmptyBullet()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/bnc822347_EmptyBullet.odp"), ODP);
    xDocShRef = saveAndReload(xDocShRef, PPTX);

    SdDrawDocument* pDoc = xDocShRef->GetDoc();
    SdrOutliner* pOutliner = pDoc->GetInternalOutliner();
    const SdrPage* pPage = pDoc->GetPage(1);
    SdrObject* pObject = pPage->GetObj(0);
    SdrTextObj* pTextObject = dynamic_cast<SdrTextObj*>(pObject);
    CPPUNIT_ASSERT(pTextObject);

    OutlinerParaObject* pOutlinerParagraphObject = pTextObject->GetOutlinerParaObject();
    const EditTextObject& aEdit = pOutlinerParagraphObject->GetTextObject();

    OUString sText = aEdit.GetText(0);
    CPPUNIT_ASSERT_EQUAL(OUString("M3 Feature Test"), sText);

    pOutliner->SetText(*pOutlinerParagraphObject);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pOutliner->GetParagraphCount());

    const sal_Int16 nDepth = pOutliner->GetDepth(0);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(-1), nDepth); // depth >= 0 means that the paragraph has bullets enabled

    xDocShRef->DoClose();
}

//Bullets not having  any text following them are not getting exported to pptx correctly.
void SdExportTest::testFdo90607()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo90607.pptx"), PPTX);
    xDocShRef = saveAndReload(xDocShRef, PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(1) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    OutlinerParaObject* pOutlinerParagraphObject = pTxtObj->GetOutlinerParaObject();
    const sal_Int16 nDepth = pOutlinerParagraphObject->GetDepth(0);
    CPPUNIT_ASSERT_MESSAGE("not equal", nDepth != -1);
    xDocShRef->DoClose();
}

void SdExportTest::testFdo83751()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/fdo83751.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier( xDocShRef->GetModel(), uno::UNO_QUERY );
    uno::Reference<document::XDocumentProperties> xProps( xDocumentPropertiesSupplier->getDocumentProperties(), uno::UNO_QUERY );
    uno::Reference<beans::XPropertySet> xUDProps( xProps->getUserDefinedProperties(), uno::UNO_QUERY );
    OUString propValue;
    xUDProps->getPropertyValue("Testing") >>= propValue;
    CPPUNIT_ASSERT_EQUAL(OUString("Document"), propValue);
    xDocShRef->DoClose();
}

void SdExportTest::testFdo79731()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo79731.odp"), ODP);
    xDocShRef = saveAndReload(xDocShRef, PPTX);
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    xDocShRef->DoClose();
}

void SdExportTest::testSwappedOutImageExport()
{
    // Problem was with the swapped out images, which were not swapped in during export.
    const sal_Int32 vFormats[] = {
        ODP,
        PPT,
        PPTX,
    };

    // Set cache size to a very small value to make sure one of the images is swapped out
    std::shared_ptr< comphelper::ConfigurationChanges > xBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), xBatch);
    xBatch->commit();

    for( size_t nExportFormat = 0; nExportFormat < SAL_N_ELEMENTS(vFormats); ++nExportFormat )
    {
        // Load the original file with one image
        ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/document_with_two_images.odp"), ODP);
        const OString sFailedMessage = OString("Failed on filter: ") + OString(aFileFormats[vFormats[nExportFormat]].pFilterName);

        // Export the document and import again for a check
        uno::Reference< lang::XComponent > xComponent(xDocShRef->GetModel(), uno::UNO_QUERY);
        uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OStringToOUString(OString(aFileFormats[vFormats[nExportFormat]].pFilterName), RTL_TEXTENCODING_UTF8);

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        xComponent.set(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        xDocShRef = loadURL(aTempFile.GetURL(), nExportFormat);

        // Check whether graphic exported well after it was swapped out
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), xDrawPagesSupplier->getDrawPages()->getCount() == 2 );
        uno::Reference< drawing::XDrawPage > xDrawPage( xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );

        uno::Reference<drawing::XShape> xImage(xDrawPage->getByIndex(2), uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height );
        }

        // Second Image
        xDrawPage.set(xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW );
        xImage.set(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height );
        }
        xDocShRef->DoClose();
    }
}

void SdExportTest::testOOoXMLAnimations()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/sxi/ooo41061-1.sxi"), SXI);

    uno::Reference<lang::XComponent> xComponent(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OStringToOUString(OString(getFormat(ODP)->pFilterName), RTL_TEXTENCODING_UTF8);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    xDocShRef->DoClose();

    // the problem was that legacy OOoXML animations were lost if store
    // immediately follows load because they were "converted" async by a timer
    xmlDocPtr pXmlDoc = parseExport(aTempFile, "content.xml");
    assertXPath(pXmlDoc, "//anim:par[@presentation:node-type='timing-root']", 26);
    // currently getting 52 of these without the fix (depends on timing)
    assertXPath(pXmlDoc, "//anim:par", 223);
}

void SdExportTest::testTdf80020()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/tdf80020.odp"), ODP);
    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("graphics"), uno::UNO_QUERY);
        uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName("Test Style"), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("text"), xStyle->getParentStyle());
        xDocShRef = saveAndReload( xDocShRef, ODP );
    }
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("graphics"), uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xStyleFamily->getByName("Test Style"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("text"), xStyle->getParentStyle());
}

void SdExportTest::testLinkedGraphicRT()
{
    // Problem was with linked images
    const sal_Int32 vFormats[] = {
        ODP,
        PPT,
//      PPTX, -> this fails now, need a fix
    };

    for( size_t nExportFormat = 0; nExportFormat < SAL_N_ELEMENTS(vFormats); ++nExportFormat )
    {
        // Load the original file with one image
        ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/document_with_linked_graphic.odp"), ODP);
        const OString sFailedMessage = OString("Failed on filter: ") + OString(aFileFormats[vFormats[nExportFormat]].pFilterName);

        // Export the document and import again for a check
        uno::Reference< lang::XComponent > xComponent(xDocShRef->GetModel(), uno::UNO_QUERY);
        uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OStringToOUString(OString(aFileFormats[vFormats[nExportFormat]].pFilterName), RTL_TEXTENCODING_UTF8);

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        xComponent.set(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        xDocShRef = loadURL(aTempFile.GetURL(), nExportFormat);

        // Check whether graphic imported well after export
        SdDrawDocument *pDoc = xDocShRef->GetDoc();
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pDoc != nullptr );
        const SdrPage *pPage = pDoc->GetPage(1);
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pPage != nullptr );
        SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(2));
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pObject != nullptr );
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pObject->IsLinkedGraphic() );

        const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), !rGraphicObj.IsSwappedOut());
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), GRAPHIC_BITMAP, rGraphicObj.GetGraphic().GetType());
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), sal_uLong(864900), rGraphicObj.GetSizeBytes());

        xDocShRef->DoClose();
    }
}

void SdExportTest::testImageWithSpecialID()
{
    // Check how LO handles when the imported graphic's ID is different from that one
    // which is generated by LO.
    const sal_Int32 vFormats[] = {
        ODP,
        PPT,
        PPTX,
    };

    // Trigger swap out mechanism to test swapped state factor too.
    std::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), batch);
    batch->commit();

    for( size_t nExportFormat = 0; nExportFormat < SAL_N_ELEMENTS(vFormats); ++nExportFormat )
    {
        // Load the original file
        ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/images_with_special_IDs.odp"), ODP);
        const OString sFailedMessage = OString("Failed on filter: ") + OString(aFileFormats[vFormats[nExportFormat]].pFilterName);

        // Export the document and import again for a check
        uno::Reference< lang::XComponent > xComponent(xDocShRef->GetModel(), uno::UNO_QUERY);
        uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= OStringToOUString(OString(aFileFormats[vFormats[nExportFormat]].pFilterName), RTL_TEXTENCODING_UTF8);

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        xComponent.set(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        xDocShRef = loadURL(aTempFile.GetURL(), nExportFormat);

        // Check whether graphic was exported well
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), xDrawPagesSupplier->getDrawPages()->getCount() == 2 );
        uno::Reference< drawing::XDrawPage > xDrawPage( xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );

        uno::Reference<drawing::XShape> xImage(xDrawPage->getByIndex(2), uno::UNO_QUERY);
        uno::Reference< beans::XPropertySet > XPropSet( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381), xBitmap->getSize().Height );
        }

        // Second Image
        xDrawPage.set(xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW );
        xImage.set(xDrawPage->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set( xImage, uno::UNO_QUERY_THROW );
        // Check URL
        {
            OUString sURL;
            XPropSet->getPropertyValue("GraphicURL") >>= sURL;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), sURL != "vnd.sun.star.GraphicObject:00000000000000000000000000000000");
        }
        // Check size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900), xBitmap->getSize().Width );
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600), xBitmap->getSize().Height );
        }
        xDocShRef->DoClose();
    }
}
void SdExportTest::testTableCellFillProperties()
{
    std::shared_ptr< comphelper::ConfigurationChanges > batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(sal_Int32(1), batch);
    batch->commit();

    // Load the original file
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/Table_with_Cell_Fill.odp"), ODP);

    // Export the document and import again for a check
    uno::Reference< lang::XComponent > xComponent(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<frame::XStorable> xStorable(xComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OStringToOUString(OString(aFileFormats[PPTX].pFilterName), RTL_TEXTENCODING_UTF8);

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    xComponent.set(xStorable, uno::UNO_QUERY);
    xComponent->dispose();
    xDocShRef = loadURL(aTempFile.GetURL(), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCell;

    // Test Solid fill color
    sal_Int32 nColor;
    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6750207), nColor);

    // Test Picture fill type for cell
    drawing::FillStyle aFillStyle( drawing::FillStyle_NONE );
    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, aFillStyle);

    // Test Gradient fill type for cell
    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillStyle") >>= aFillStyle;
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, aFillStyle);

    xDocShRef->DoClose();
}

void SdExportTest::testBulletStartNumber()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n90255.pptx"), PPTX );
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = dynamic_cast<const SvxNumBulletItem *>(aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET));
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's start number is wrong!", sal_Int16(pNumFmt->GetNumRule()->GetLevel(0).GetStart()), sal_Int16(3) );
    xDocShRef->DoClose();
}

void SdExportTest::testLineStyle()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/lineStyle.pptx"), PPTX);
     xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrObject const* pShape = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);

    const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pShape->GetMergedItem(XATTR_LINESTYLE));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong style",drawing::LineStyle_SOLID, rStyleItem.GetValue());

    xDocShRef->DoClose();
}

void SdExportTest::testRightToLeftParaghraph()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/rightToLeftParagraph.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef, PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    sal_Int16 nWritingMode = 0;
    xPropSet->getPropertyValue( "WritingMode" ) >>= nWritingMode;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong paragraph WritingMode", text::WritingMode2::RL_TB, nWritingMode);

    xDocShRef->DoClose();
}

void SdExportTest::testTextboxWithHyperlink()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/hyperlinktest.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef, PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );

    // first chunk of text
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );

    uno::Reference<text::XTextField> xField;
    xPropSet->getPropertyValue("TextField") >>= xField;
    CPPUNIT_ASSERT_MESSAGE("Where is the text field?", xField.is() );

    xPropSet.set(xField, uno::UNO_QUERY);
    OUString aURL;
    xPropSet->getPropertyValue("URL") >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", OUString("http://www.xkcd.com/"), aURL);

    xDocShRef->DoClose();
}

void SdExportTest::testBulletColor()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/bulletColor.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = dynamic_cast<const SvxNumBulletItem *>(aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET));
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's color is wrong!", sal_uInt32(0xff0000),pNumFmt->GetNumRule()->GetLevel(0).GetBulletColor().GetColor());
}

void SdExportTest::testTdf62176()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/Tdf62176.odp"), ODP);
    uno::Reference<drawing::XDrawPage> xPage( getPage( 0, xDocShRef ) );

    //there should be only *one* shape
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage->getCount());

    uno::Reference<beans::XPropertySet> xShape( getShape( 0, xPage ) );
    //checking Paragraph's Left Margin with expected value
    sal_Int32 nParaLeftMargin = 0;
    xShape->getPropertyValue("ParaLeftMargin") >>= nParaLeftMargin;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nParaLeftMargin);
    //checking Paragraph's First Line Indent with expected value
    sal_Int32 nParaFirstLineIndent = 0;
    xShape->getPropertyValue("ParaFirstLineIndent") >>= nParaFirstLineIndent;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1300), nParaFirstLineIndent);

    //Checking the *Text* in TextBox
    uno::Reference<text::XTextRange> xParagraph( getParagraphFromShape( 0, xShape ) );
    CPPUNIT_ASSERT_EQUAL(OUString("Hello World"), xParagraph->getString());

    //Saving and Reloading the file
    xDocShRef = saveAndReload(xDocShRef, ODP);
    uno::Reference<drawing::XDrawPage> xPage2( getPage(0, xDocShRef ) );
    //there should be only *one* shape
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage2->getCount());

    uno::Reference<beans::XPropertySet> xShape2( getShape( 0, xPage2 ) );
    //checking Paragraph's Left Margin with expected value
    sal_Int32 nParaLeftMargin2 = 0;
    xShape2->getPropertyValue("ParaLeftMargin") >>= nParaLeftMargin2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), nParaLeftMargin2);
    //checking Paragraph's First Line Indent with expected value
    sal_Int32 nParaFirstLineIndent2 = 0;
    xShape2->getPropertyValue("ParaFirstLineIndent") >>= nParaFirstLineIndent2;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1300), nParaFirstLineIndent2);

    //Checking the *Text* in TextBox
    uno::Reference<text::XTextRange> xParagraph2( getParagraphFromShape( 0, xShape2 ) );
    CPPUNIT_ASSERT_EQUAL(OUString("Hello World"), xParagraph2->getString());
}

void SdExportTest::testBulletCharAndFont()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/odp/bulletCharAndFont.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    uno::Reference<container::XIndexAccess> xLevels(xPropSet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    OUString    sBulletChar(sal_Unicode(0xf06c));
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "BulletChar")
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "BulletChar incorrect.", sBulletChar ,rProp.Value.get<OUString>());
        if (rProp.Name == "BulletFont")
        {
            awt::FontDescriptor aFontDescriptor;
            rProp.Value >>= aFontDescriptor;
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "BulletFont incorrect.", OUString("Wingdings"),aFontDescriptor.Name);
        }
    }
    xDocShRef->DoClose();
}

void SdExportTest::testTdf91378()
{

    //Check For Import and Export Both
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/tdf91378.pptx"), PPTX);
    for( sal_uInt32 i=0;i<2;i++)
    {
      SdDrawDocument *pDoc = xDocShRef->GetDoc();
      CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
      uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier( xDocShRef->GetModel(), uno::UNO_QUERY );
      uno::Reference<document::XDocumentProperties> xProps( xDocumentPropertiesSupplier->getDocumentProperties(), uno::UNO_QUERY );
      uno::Reference<beans::XPropertySet> xUDProps( xProps->getUserDefinedProperties(), uno::UNO_QUERY );
      OUString propValue;
      xUDProps->getPropertyValue("Testing") >>= propValue;
      CPPUNIT_ASSERT(propValue.isEmpty());
      xDocShRef = saveAndReload( xDocShRef, PPTX );
    }
    xDocShRef->DoClose();
}

void SdExportTest::testBnc822341()
{
    // Check import / export of embedded text document
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/odp/bnc822341.odp"), ODP);
    utl::TempFile tempFile1;
    xDocShRef = saveAndReload( xDocShRef, PPTX, &tempFile1 );

    // Export an LO specific ole object (imported from an ODP document)
    {
        xmlDocPtr pXmlDocCT = parseExport(tempFile1, "[Content_Types].xml");
        assertXPath(pXmlDocCT,
                    "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.wordprocessingml.document']",
                    "PartName",
                    "/ppt/embeddings/oleObject1.docx");

        xmlDocPtr pXmlDocRels = parseExport(tempFile1, "ppt/slides/_rels/slide1.xml.rels");
        assertXPath(pXmlDocRels,
            "/rels:Relationships/rels:Relationship[@Target='../embeddings/oleObject1.docx']",
            "Type",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");

        xmlDocPtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
        assertXPath(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/p:oleObj",
            "progId",
            "Word.Document.12");

        const SdrPage *pPage = GetPage( 1, xDocShRef );

        const SdrObject* pObj = dynamic_cast<SdrObject*>( pPage->GetObj(0) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier() );
    }

    utl::TempFile tempFile2;
    xDocShRef = saveAndReload( xDocShRef, PPTX, &tempFile2 );

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocPtr pXmlDocCT = parseExport(tempFile2, "[Content_Types].xml");
        assertXPath(pXmlDocCT,
                    "/ContentType:Types/ContentType:Override[@ContentType='application/vnd.openxmlformats-officedocument.wordprocessingml.document']",
                    "PartName",
                    "/ppt/embeddings/oleObject1.docx");

        xmlDocPtr pXmlDocRels = parseExport(tempFile2, "ppt/slides/_rels/slide1.xml.rels");
        assertXPath(pXmlDocRels,
            "/rels:Relationships/rels:Relationship[@Target='../embeddings/oleObject1.docx']",
            "Type",
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");

        xmlDocPtr pXmlDocContent = parseExport(tempFile2, "ppt/slides/slide1.xml");
        assertXPath(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/p:oleObj",
            "progId",
            "Word.Document.12");

        SdDrawDocument *pDoc = xDocShRef->GetDoc();
        CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
        const SdrPage *pPage = pDoc->GetPage(1);
        CPPUNIT_ASSERT_MESSAGE( "no page", pPage != nullptr );

        const SdrObject* pObj = dynamic_cast<SdrObject*>( pPage->GetObj(0) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier() );
    }

    xDocShRef->DoClose();
}

void SdExportTest::testMathObject()
{
    // Check import / export of math object
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/odp/math.odp"), ODP);
    utl::TempFile tempFile1;
    xDocShRef = saveAndReload(xDocShRef, PPTX, &tempFile1);

    // Export an LO specific ole object (imported from an ODP document)
    {
        xmlDocPtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
        assertXPath(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice",
            "Requires",
            "a14");
        assertXPathContent(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice/p:sp/p:txBody/a:p/a14:m/m:oMath/m:r[1]/m:t",
            "a");

        // TODO can't import yet
    }

    xDocShRef->DoClose();
}

void SdExportTest::testMathObjectPPT2010()
{
    // Check import / export of math object
#if 0 // disabled on release branch, needs 332a796366b7cb91dff41de4b9ffb17843112a3e
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/Math.pptx"), PPTX);
    utl::TempFile tempFile1;
    xDocShRef = saveAndReload(xDocShRef, PPTX, &tempFile1);

    // Export an MS specific ole object (imported from a PPTX document)
    {
        xmlDocPtr pXmlDocContent = parseExport(tempFile1, "ppt/slides/slide1.xml");
        assertXPath(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice",
            "Requires",
            "a14");
        assertXPathContent(pXmlDocContent,
            "/p:sld/p:cSld/p:spTree/mc:AlternateContent/mc:Choice/p:sp/p:txBody/a:p/a14:m/m:oMath/m:sSup/m:e/m:r[1]/m:t",
            OUString::fromUtf8("\xf0\x9d\x91\x8e")); // non-BMP char

        const SdrPage *pPage = GetPage(1, xDocShRef);
        const SdrObject* pObj = dynamic_cast<SdrObject*>(pPage->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier());
    }

    xDocShRef->DoClose();
#endif
}

void SdExportTest::testBulletMarginAndIndentation()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/bulletMarginAndIndent.pptx"), PPTX );
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = dynamic_cast<const SvxNumBulletItem *>(aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET));
    CPPUNIT_ASSERT(pNumFmt);

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's left margin is wrong!", sal_uInt32(1000),sal_uInt32(pNumFmt->GetNumRule()->GetLevel(0).GetAbsLSpace()) ); // left margin is 0.79 cm
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's indentation is wrong!", sal_Int32(-998),sal_Int32(pNumFmt->GetNumRule()->GetLevel(0). GetFirstLineOffset()));
}

void SdExportTest::testParaMarginAndindentation()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/paraMarginAndIndentation.pptx"), PPTX);

    xDocShRef = saveAndReload( xDocShRef, PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    sal_Int32 nParaLeftMargin = 0;
    xPropSet->getPropertyValue( "ParaLeftMargin" ) >>= nParaLeftMargin;
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1000), sal_uInt32(nParaLeftMargin));

    sal_Int32 nParaFirstLineIndent = 0;
    xPropSet->getPropertyValue( "ParaFirstLineIndent" ) >>= nParaFirstLineIndent;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1268), sal_Int32(nParaFirstLineIndent));

    xDocShRef->DoClose();
}

void SdExportTest::testCellLeftAndRightMargin()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/n90223.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef, PPTX );
    sal_Int32 nLeftMargin, nRightMargin;

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    uno::Reference< css::table::XTable > xTable (pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< css::table::XMergeableCell > xCell( xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xCellPropSet(xCell, uno::UNO_QUERY_THROW);

    uno::Any aLeftMargin = xCellPropSet->getPropertyValue("TextLeftDistance");
    aLeftMargin >>= nLeftMargin ;

    uno::Any aRightMargin = xCellPropSet->getPropertyValue("TextRightDistance");
    aRightMargin >>= nRightMargin ;

    // Convert values to EMU
    nLeftMargin  =  oox::drawingml::convertHmmToEmu( nLeftMargin );
    nRightMargin =  oox::drawingml::convertHmmToEmu( nRightMargin );

    CPPUNIT_ASSERT_EQUAL(sal_Int32(45720), nLeftMargin);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(45720), nRightMargin);

    xDocShRef->DoClose();
}

void SdExportTest::testTableCellBorder()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/n90190.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    table::BorderLine2 aBorderLine;

    uno::Reference< table::XTable > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< css::table::XMergeableCell > xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCellPropSet (xCell, uno::UNO_QUERY_THROW);

    xCellPropSet->getPropertyValue("LeftBorder") >>= aBorderLine;
    sal_Int32 nLeftBorder = aBorderLine.LineWidth ;
// While importing the table cell border line width, it converts EMU->Hmm then divided result by 2.
// To get original value of LineWidth need to multiple by 2.
    nLeftBorder = nLeftBorder * 2 ;
    nLeftBorder = oox::drawingml::convertHmmToEmu( nLeftBorder );
    CPPUNIT_ASSERT(nLeftBorder);
    CPPUNIT_ASSERT_EQUAL(util::Color(45296), aBorderLine.Color);

    xCellPropSet->getPropertyValue("RightBorder") >>= aBorderLine;
    sal_Int32 nRightBorder = aBorderLine.LineWidth ;
    nRightBorder = nRightBorder * 2 ;
    nRightBorder = oox::drawingml::convertHmmToEmu( nRightBorder );
    CPPUNIT_ASSERT(nRightBorder);
    CPPUNIT_ASSERT_EQUAL(util::Color(16777215), aBorderLine.Color);

    xCellPropSet->getPropertyValue("TopBorder") >>= aBorderLine;
    sal_Int32 nTopBorder = aBorderLine.LineWidth ;
    nTopBorder = nTopBorder * 2 ;
    nTopBorder = oox::drawingml::convertHmmToEmu( nTopBorder );
    CPPUNIT_ASSERT(nTopBorder);
    CPPUNIT_ASSERT_EQUAL(util::Color(45296), aBorderLine.Color);


    xCellPropSet->getPropertyValue("BottomBorder") >>= aBorderLine;
    sal_Int32 nBottomBorder = aBorderLine.LineWidth ;
    nBottomBorder = nBottomBorder * 2 ;
    nBottomBorder = oox::drawingml::convertHmmToEmu( nBottomBorder );
    CPPUNIT_ASSERT(nBottomBorder);
    CPPUNIT_ASSERT_EQUAL(util::Color(45296), aBorderLine.Color);

    xDocShRef->DoClose();
}

void SdExportTest::testTdf80224()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/odp/tdf80224.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef, PPTX );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph->getStart(), uno::UNO_QUERY_THROW );

    sal_Int32 nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6644396), nCharColor);
    xDocShRef->DoClose();
}

bool checkTransitionOnPage(uno::Reference<drawing::XDrawPagesSupplier> xDoc, sal_Int32 nSlideNumber,
                           sal_Int16 nExpectedTransitionType, sal_Int16 nExpectedTransitionSubType,
                           bool bExpectedDirection = true)
{
    sal_Int32 nSlideIndex = nSlideNumber - 1;

    CPPUNIT_ASSERT_MESSAGE("Slide/Page index out of range", nSlideIndex < xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(nSlideIndex), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY);

    sal_Int16 nTransitionType = 0;
    xPropSet->getPropertyValue("TransitionType") >>= nTransitionType;

    if (nExpectedTransitionType != nTransitionType)
    {
        std::cerr << "Transition type: " << nTransitionType << " " << nExpectedTransitionType << std::endl;
        return false;
    }

    sal_Int16 nTransitionSubtype = 0;
    xPropSet->getPropertyValue("TransitionSubtype") >>= nTransitionSubtype;
    if (nExpectedTransitionSubType != nTransitionSubtype)
    {
        std::cerr << "Transition Subtype: " << nTransitionSubtype << " " << nExpectedTransitionSubType << std::endl;
        return false;
    }

    bool bDirection = false;
    xPropSet->getPropertyValue("TransitionDirection") >>= bDirection;

    if (bExpectedDirection != bDirection)
    {
        std::cerr << "Transition Direction: " << (bExpectedDirection ? "normal" : "reversed")
                  << " "                      << (bDirection ? "normal" : "reversed") << std::endl;
        return false;
    }

    return true;
}

void SdExportTest::testExportTransitionsPPTX()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/AllTransitions.odp"), ODP);
    xDocShRef = saveAndReload(xDocShRef, PPTX);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);

    // WIPE TRANSITIONS
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 01, TransitionType::BARWIPE, TransitionSubType::TOPTOBOTTOM, false));
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 02, TransitionType::BARWIPE, TransitionSubType::LEFTTORIGHT));
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 03, TransitionType::BARWIPE, TransitionSubType::LEFTTORIGHT, false));
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 04, TransitionType::BARWIPE, TransitionSubType::TOPTOBOTTOM));

    // CUT THROUGH BLACK
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 31, TransitionType::BARWIPE, TransitionSubType::FADEOVERCOLOR));

    // COMB
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 41, TransitionType::PUSHWIPE, TransitionSubType::COMBHORIZONTAL));
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 42, TransitionType::PUSHWIPE, TransitionSubType::COMBVERTICAL));

    // OUTSIDE TURNING CUBE
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 57, TransitionType::MISCSHAPEWIPE, TransitionSubType::CORNERSOUT));
    // INSIDE TURNING CUBE
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 60, TransitionType::MISCSHAPEWIPE, TransitionSubType::CORNERSIN));

    // FALL
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 61, TransitionType::MISCSHAPEWIPE, TransitionSubType::LEFTTORIGHT));

    // VORTEX
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 70, TransitionType::MISCSHAPEWIPE, TransitionSubType::VERTICAL));

    // RIPPLE
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 71, TransitionType::MISCSHAPEWIPE, TransitionSubType::HORIZONTAL));

    // GLITTER
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 72, TransitionType::MISCSHAPEWIPE, TransitionSubType::DIAMOND));

    // HONEYCOMB
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 73, TransitionType::MISCSHAPEWIPE, TransitionSubType::HEART));

    // NEWSFLASH
    CPPUNIT_ASSERT(checkTransitionOnPage(xDoc, 74, TransitionType::ZOOM, TransitionSubType::ROTATEIN));
}

void SdExportTest::testTdf92527()
{
    // We draw a diamond in an empty document. A newly created diamond shape does not have
    // CustomShapeGeometry - Path - Segments property, and previously DrawingML exporter
    // did not export custom shapes which did not have CustomShapeGeometry - Path - Segments property.
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/empty.fodp"), FODG );
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape1(xFactory->createInstance("com.sun.star.drawing.CustomShape"), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc1(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage1(xDoc1->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);
    xPage1->add(xShape1);
    xShape1->setSize(awt::Size(10000, 10000));
    xShape1->setPosition(awt::Point(1000, 1000));
    uno::Sequence<beans::PropertyValue> aShapeGeometry(comphelper::InitPropertySequence(
        {
            {"Type", uno::makeAny(OUString("diamond"))},
        }));
    uno::Reference<beans::XPropertySet> xPropertySet1(xShape1, uno::UNO_QUERY);
    xPropertySet1->setPropertyValue("CustomShapeGeometry", uno::makeAny(aShapeGeometry));

    xDocShRef = saveAndReload(xDocShRef, PPTX);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc2(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage2(xDoc2->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape2(xPage2->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xPropertySet2( xShape2, uno::UNO_QUERY_THROW );
    uno::Sequence<beans::PropertyValue> aProps;
    xPropertySet2->getPropertyValue("CustomShapeGeometry") >>= aProps;
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get< uno::Sequence<beans::PropertyValue> >();
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Coordinates")
            aCoordinates = rProp.Value.get< uno::Sequence<drawing::EnhancedCustomShapeParameterPair> >();
    }

    // 5 coordinate pairs, 1 MoveTo, 4 LineTo
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aCoordinates.getLength());
    xDocShRef->DoClose();
}

void SdExportTest::testTdf99224()
{
    sd::DrawDocShellRef xShell = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/tdf99224.odp"), ODP);
    xShell = saveAndReload(xShell, PPTX);
    uno::Reference<drawing::XDrawPage> xPage = getPage(0, xShell);
    // This was 0: the image with text was lost on export.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xPage->getCount());
    xShell->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
