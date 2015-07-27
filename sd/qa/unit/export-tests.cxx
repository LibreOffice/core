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
#include <comphelper/processfactory.hxx>
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
#include <rtl/ustring.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
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
#include <com/sun/star/drawing/FillStyle.hpp>
 #include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>

#include <svx/svdotable.hxx>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>

#include <config_features.h>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

using namespace ::com::sun::star;

class SdExportTest : public SdModelTestBase
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
    void testLinkedGraphicRT();
    void testImageWithSpecialID();
    void testTableCellFillProperties();
    void testBulletStartNumber();
    void testLineStyle();
    void testCellLeftAndRightMargin();
    void testRightToLeftParaghraph();
    void testTableCellBorder();
    void testBulletColor();
    void testBulletCharAndFont();
    void testBulletMarginAndIndentation();
    void testParaMarginAndindentation();

    void testFdo90607();
#if !defined WNT
    void testBnc822341();
#endif

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
    CPPUNIT_TEST(testLinkedGraphicRT);
    CPPUNIT_TEST(testImageWithSpecialID);
    CPPUNIT_TEST(testTableCellFillProperties);
    CPPUNIT_TEST(testBulletStartNumber);
    CPPUNIT_TEST(testLineStyle);
    CPPUNIT_TEST(testCellLeftAndRightMargin);
    CPPUNIT_TEST(testRightToLeftParaghraph);
    CPPUNIT_TEST(testTableCellBorder);
    CPPUNIT_TEST(testBulletColor);
    CPPUNIT_TEST(testBulletCharAndFont);
    CPPUNIT_TEST(testBulletMarginAndIndentation);
    CPPUNIT_TEST(testParaMarginAndindentation);

#if !defined WNT
    CPPUNIT_TEST(testBnc822341);
#endif
    CPPUNIT_TEST_SUITE_END();
};

void SdExportTest::testN821567()
{
    OUString bgImage;
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n821567.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef, ODP );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "not exactly one page", xDoc->getDrawPages()->getCount() == 1 );
    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySet > xPropSet( xPage, uno::UNO_QUERY );
    uno::Any aAny = xPropSet->getPropertyValue( OUString("Background") );
    if(aAny.hasValue())
    {
        uno::Reference< beans::XPropertySet > aXBackgroundPropSet;
        aAny >>= aXBackgroundPropSet;
        aAny = aXBackgroundPropSet->getPropertyValue( OUString("FillBitmapName"));
        aAny >>= bgImage;
    }
    CPPUNIT_ASSERT_MESSAGE("Slide Background is not exported properly", !bgImage.isEmpty());

    xDocShRef->DoClose();
}

namespace {

void checkFontAttributes(const SdrTextObj* pObj, sal_uInt32 nColor,
        bool bCheckWeight, FontWeight eWeight, bool bCheckItalic, FontItalic eItalic)
{
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
    const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
    std::vector<EECharAttrib> rLst;
    aEdit.GetCharAttribs(0, rLst);
    for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
    {
        const SvxColorItem *pCharColor = dynamic_cast<const SvxColorItem *>((*it).pAttr);
        if( pCharColor )
        {
            CPPUNIT_ASSERT_EQUAL( nColor, pCharColor->GetValue().GetColor());
        }

        if(bCheckWeight)
        {
            const SvxWeightItem *pWeight = dynamic_cast<const SvxWeightItem *>((*it).pAttr);
            if( pWeight )
            {
                CPPUNIT_ASSERT_EQUAL( eWeight, pWeight->GetWeight());
            }
        }

        if(bCheckItalic)
        {
            const SvxPostureItem *pPosture = dynamic_cast<const SvxPostureItem *>((*it).pAttr);
            if( pPosture )
            {
                CPPUNIT_ASSERT_EQUAL( eItalic, pPosture->GetPosture());
            }
        }
    }

}

}

void SdExportTest::testBnc870233_1()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc870233_1.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // The problem was all shapes had the same font (the last parsed font attribues overwrote all previous ones)

    // First shape has red, bold font
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
        checkFontAttributes(pObj, sal_uInt32(0xff0000),
                true, WEIGHT_BOLD, true, ITALIC_NONE);
    }

    // Second shape has blue, italic font
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 1 ) );
        checkFontAttributes(pObj, sal_uInt32(0x0000ff),
                true, WEIGHT_NORMAL, true, ITALIC_NORMAL);
    }

    xDocShRef->DoClose();
}

void SdExportTest::testBnc870233_2()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc870233_2.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // The problem was in some SmartArts font color was wrong

    // First smart art has blue font color (direct formatting)
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
        checkFontAttributes(pObj, sal_uInt32(0x0000ff),
                false, WEIGHT_DONTKNOW, false, ITALIC_NONE);
    }

    // Second smart art has "dk2" font color (style)
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 1 ) );
        checkFontAttributes(pObj, sal_uInt32(0x1F497D),
                false, WEIGHT_DONTKNOW, false, ITALIC_NONE);
    }

    // Third smart art has white font color (style)
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 2 ) );
        checkFontAttributes(pObj, sal_uInt32(0xffffff),
                false, WEIGHT_DONTKNOW, false, ITALIC_NONE);
    }

    xDocShRef->DoClose();
}

void SdExportTest::testN828390_4()
{
    bool bPassed = false;
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/n828390_4.odp"), ODP );

    xDocShRef = saveAndReload( xDocShRef, PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );
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

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );
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

void SdExportTest::testMediaEmbedding()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/media_embedding.odp"), ODP);

#if HAVE_FEATURE_GLTF
    xDocShRef = saveAndReload( xDocShRef, ODP );
#endif

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

#if HAVE_FEATURE_GLTF
    // First object is a glTF model
    SdrMediaObj *pModelObj = dynamic_cast<SdrMediaObj*>( pPage->GetObj( 2 ));
    CPPUNIT_ASSERT_MESSAGE( "missing model", pModelObj != NULL);
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
    CPPUNIT_ASSERT_MESSAGE( "missing media object", pMediaObj != NULL);
    CPPUNIT_ASSERT_EQUAL( OUString( "vnd.sun.star.Package:Media/button-1.wav" ), pMediaObj->getMediaProperties().getURL());
    CPPUNIT_ASSERT_EQUAL( OUString( "application/vnd.sun.star.media" ), pMediaObj->getMediaProperties().getMimeType());

    xDocShRef->DoClose();
}

void SdExportTest::testFdo84043()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo84043.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef, ODP );

    // the bug was duplicate attributes, causing crash in a build with asserts
    SdDrawDocument const* pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE("no document", pDoc != nullptr);
    SdrPage const* pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("no page", pPage != nullptr);
    SdrObject const* pShape = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no shape", pShape != nullptr);
}

void SdExportTest::testFdo71961()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo71961.odp"), ODP);

    xDocShRef = saveAndReload( xDocShRef, PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // Export to .pptx changes all text frames to custom shape objects, which obey TextWordWrap property
    // (which is false for text frames otherwise and is ignored). Check that frames that should wrap still do.
    SdrObjCustomShape *pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 1 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != NULL);
    CPPUNIT_ASSERT_EQUAL( OUString( "Text to be always wrapped" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( true, (static_cast<const SdrOnOffItem&>(pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 2 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != NULL);
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom shape non-wrapped text" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( false, (static_cast<const SdrOnOffItem&>(pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 3 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != NULL);
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom shape wrapped text" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( true, (static_cast<const SdrOnOffItem&>(pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue());

    xDocShRef->DoClose();
}

void SdExportTest::testN828390()
{
    bool bPassed = false;
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n828390.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef, PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );
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

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // Check z-order of the two shapes, use background color to identify them
    // First object in the background has blue background color
    const SdrObject *pObj = dynamic_cast<SdrObject *>( pPage->GetObj( 0 ) );
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
    CPPUNIT_ASSERT_EQUAL( sal_uInt32(0x0000ff),(static_cast< const XColorItem& >(pObj->GetMergedItem(XATTR_FILLCOLOR))).GetColorValue().GetColor());

    // Second object at the front has green background color
    pObj = dynamic_cast<SdrObject *>( pPage->GetObj( 1 ) );
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
    CPPUNIT_ASSERT_EQUAL( sal_uInt32(0x00ff00),(static_cast< const XColorItem& >(pObj->GetMergedItem(XATTR_FILLCOLOR))).GetColorValue().GetColor());

    xDocShRef->DoClose();
}

void SdExportTest::testBnc862510_5()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc862510_5.pptx"), PPTX);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // Same as testBnc870237, but here we check the horizontal spacing
    const SdrObject* pObj = dynamic_cast<SdrObject*>( pPage->GetObj( 1 ) );
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
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

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );

    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
     SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );

    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(1) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != NULL);
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
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );

    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier( xDocShRef->GetModel(), uno::UNO_QUERY );
    uno::Reference<document::XDocumentProperties> xProps( xDocumentPropertiesSupplier->getDocumentProperties(), uno::UNO_QUERY );
    uno::Reference<beans::XPropertySet> xUDProps( xProps->getUserDefinedProperties(), uno::UNO_QUERY );
    OUString propValue;
    xUDProps->getPropertyValue(OUString("Testing")) >>= propValue;
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
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pDoc != NULL );
        const SdrPage *pPage = pDoc->GetPage(1);
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pPage != NULL );
        SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(2));
        CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pObject != NULL );
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

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

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

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );

    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );

    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != NULL);
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

    SdDrawDocument const* pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE("no document", pDoc != nullptr);
    SdrPage const* pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("no page", pPage != nullptr);
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

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
    xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );

    uno::Reference< drawing::XDrawPage > xPage(
    xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySet > xShape(
    xPage->getByIndex(0), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no shape", xShape.is() );

    // Get first paragraph
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_MESSAGE( "not a text shape", xText.is() );
    uno::Reference<container::XEnumerationAccess> paraEnumAccess;
    paraEnumAccess.set(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> const xParagraph(paraEnum->nextElement(),
                uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    sal_Int16 nWritingMode = 0;
    xPropSet->getPropertyValue( "WritingMode" ) >>= nWritingMode;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong paragraph WritingMode", text::WritingMode2::RL_TB, nWritingMode);

    xDocShRef->DoClose();
}

void SdExportTest::testBulletColor()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/bulletColor.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef, PPTX );

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );

    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );

    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != NULL);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = dynamic_cast<const SvxNumBulletItem *>(aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET));
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's color is wrong!", sal_uInt32(0xff0000),pNumFmt->GetNumRule()->GetLevel(0).GetBulletColor().GetColor());
}

void SdExportTest::testBulletCharAndFont()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/odp/bulletCharAndFont.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef, PPTX );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc( xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage( xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShape > xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    uno::Reference<container::XEnumerationAccess> paraEnumAccess;
    paraEnumAccess.set(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> const xParagraph(paraEnum->nextElement(), uno::UNO_QUERY_THROW);
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

#if !defined WNT

void SdExportTest::testBnc822341()
{
    // Check import / export of embedded text document
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/odp/bnc822341.odp"), ODP);
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    // Export an LO specific ole object (imported from an ODP document)
    {
        SdDrawDocument *pDoc = xDocShRef->GetDoc();
        CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
        const SdrPage *pPage = pDoc->GetPage(1);
        CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

        const SdrObject* pObj = dynamic_cast<SdrObject*>( pPage->GetObj(0) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier() );
    }

    xDocShRef = saveAndReload( xDocShRef, PPTX );

    // Export an MS specific ole object (imported from a PPTX document)
    {
        SdDrawDocument *pDoc = xDocShRef->GetDoc();
        CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
        const SdrPage *pPage = pDoc->GetPage(1);
        CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

        const SdrObject* pObj = dynamic_cast<SdrObject*>( pPage->GetObj(0) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(OBJ_OLE2), pObj->GetObjIdentifier() );
    }

    xDocShRef->DoClose();
}

#endif

void SdExportTest::testBulletMarginAndIndentation()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/bulletMarginAndIndent.pptx"), PPTX );

    xDocShRef = saveAndReload( xDocShRef, PPTX );

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
    xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(
    xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );

    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != NULL);

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

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
    xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );

    uno::Reference< drawing::XDrawPage > xPage(
    xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySet > xShape(
    xPage->getByIndex(0), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no shape", xShape.is() );

    // Get first paragraph
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_MESSAGE( "not a text shape", xText.is() );
    uno::Reference<container::XEnumerationAccess> paraEnumAccess;
    paraEnumAccess.set(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> const xParagraph(paraEnum->nextElement(),
                uno::UNO_QUERY_THROW);
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

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );

    uno::Reference< drawing::XDrawPage > xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );

    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    uno::Reference< com::sun::star::table::XTable > xTable (pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< com::sun::star::table::XMergeableCell > xCell( xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
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

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );

    uno::Reference< drawing::XDrawPage > xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );

    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    table::BorderLine2 aBorderLine;

    uno::Reference< table::XTable > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< com::sun::star::table::XMergeableCell > xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
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

CPPUNIT_TEST_SUITE_REGISTRATION(SdExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
