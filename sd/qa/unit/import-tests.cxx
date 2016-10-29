/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_poppler.h>

#include "sdmodeltestbase.hxx"

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
#include <rsc/rscsfx.hxx>

#include <sfx2/sfxsids.hrc>
#include <svx/svdotext.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotable.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <animations/animationnodehelper.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
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
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/style/NumberingType.hpp>

#include <stlpool.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/pngread.hxx>
#include <vcl/bitmapaccess.hxx>

using namespace ::com::sun::star;

/// Impress import filters tests.
class SdImportTest : public SdModelTestBase
{
public:
    void testDocumentLayout();
    void testSmoketest();
    void testN759180();
    void testN778859();
    void testMasterPageStyleParent();
    void testGradientAngle();
    void testTdf97808();
    void testFdo64512();
    void testFdo71075();
    void testN828390_2();
    void testN828390_3();
    void testFdo68594();
    void testFdo72998();
    void testFdo77027();
    void testStrictOOXML();
    void testN862510_1();
    void testN862510_2();
    void testN862510_4();
    void testBnc870237();
    void testBnc887225();
    void testBnc480256();
    void testBnc591147();
    void testCreationDate();
    void testBnc584721_1();
    void testBnc584721_2();
    void testBnc584721_4();
    void testBnc904423();
    void testShapeLineStyle();
    void testBnc862510_6();
    void testBnc862510_7();
#if ENABLE_PDFIMPORT
    void testPDFImport();
    void testPDFImportSkipImages();
#endif
    void testBulletSuffix();
    void testBnc910045();
    void testRowHeight();
    void testTdf93830();
    void testTdf93097();
    void testTdf93124();
    void testTdf93868();
    void testTdf103473();
    void testTdf103792();
    void testTdf103876();
    void testTdf104015();
    void testTdf104201();
    void testTdf104445();

    CPPUNIT_TEST_SUITE(SdImportTest);

    CPPUNIT_TEST(testDocumentLayout);
    CPPUNIT_TEST(testSmoketest);
    CPPUNIT_TEST(testN759180);
    CPPUNIT_TEST(testN778859);
    CPPUNIT_TEST(testMasterPageStyleParent);
    CPPUNIT_TEST(testGradientAngle);
    CPPUNIT_TEST(testTdf97808);
    CPPUNIT_TEST(testFdo64512);
    CPPUNIT_TEST(testFdo71075);
    CPPUNIT_TEST(testN828390_2);
    CPPUNIT_TEST(testN828390_3);
    CPPUNIT_TEST(testFdo68594);
    CPPUNIT_TEST(testFdo72998);
    CPPUNIT_TEST(testFdo77027);
    CPPUNIT_TEST(testStrictOOXML);
    CPPUNIT_TEST(testN862510_1);
    CPPUNIT_TEST(testN862510_2);
    CPPUNIT_TEST(testN862510_4);
    CPPUNIT_TEST(testBnc870237);
    CPPUNIT_TEST(testBnc887225);
    CPPUNIT_TEST(testBnc480256);
    CPPUNIT_TEST(testBnc591147);
    CPPUNIT_TEST(testCreationDate);
    CPPUNIT_TEST(testBnc584721_1);
    CPPUNIT_TEST(testBnc584721_2);
    CPPUNIT_TEST(testBnc584721_4);
    CPPUNIT_TEST(testBnc904423);
    CPPUNIT_TEST(testShapeLineStyle);
    CPPUNIT_TEST(testBnc862510_6);
    CPPUNIT_TEST(testBnc862510_7);
#if ENABLE_PDFIMPORT
    CPPUNIT_TEST(testPDFImport);
    CPPUNIT_TEST(testPDFImportSkipImages);
#endif
    CPPUNIT_TEST(testBulletSuffix);
    CPPUNIT_TEST(testBnc910045);
    CPPUNIT_TEST(testRowHeight);
    CPPUNIT_TEST(testTdf93830);
    CPPUNIT_TEST(testTdf93097);
    CPPUNIT_TEST(testTdf93124);
    CPPUNIT_TEST(testTdf93868);
    CPPUNIT_TEST(testTdf103473);
    CPPUNIT_TEST(testTdf103792);
    CPPUNIT_TEST(testTdf103876);
    CPPUNIT_TEST(testTdf104015);
    CPPUNIT_TEST(testTdf104201);
    CPPUNIT_TEST(testTdf104445);

    CPPUNIT_TEST_SUITE_END();
};

/** Test document against a reference XML dump of shapes.

If you want to update one of these tests, or add a new one, set the nUpdateMe
to the index of the test, and the dump XML's will be created (or rewritten)
instead of checking. Use with care - when the test is failing, first find out
why, instead of just updating .xml's blindly.

Example: Let's say you are adding a test called fdoABCD.pptx.  You'll place it
to the data/ subdirectory, and will add an entry to aFilesToCompare below,
the 3rd parameter is for export test - can be -1 (don't export), ODP, PPT or PPTX
like:

        { "fdoABCD.pptx", "xml/fdoABCD_", PPTX },

and will count the index in the aFilesToCompare structure (1st is 0, 2nd is 1,
etc.)  Temporarily you'll set nUpdateMe to this index (instead of -1), and run

make sd

This will generate the sd/qa/unit/data/xml/fdoABCD_*.xml for you.  Now you
will change nUpdateMe back to -1, and commit your fdoABCD.pptx test, the
xml/fdoABCD_*.xml dumps, and the aFilesToCompare addition in one commit.

As the last step, you will revert your fix and do 'make sd' again, to check
that without your fix, the unit test breaks.  Then clean up, and push :-)

NOTE: This approach is suitable only for tests of fixes that actually change
the layout - best to check by reverting your fix locally after having added
the test, and re-running; it should break.
*/
void SdImportTest::testDocumentLayout()
{
    struct { const char *pInput, *pDump; sal_Int32 nFormat; sal_Int32 nExportType; } aFilesToCompare[] =
    {
        { "odp/shapes-test.odp", "xml/shapes-test_page", ODP, -1 },
        { "fdo47434.pptx", "xml/fdo47434_", PPTX, -1 },
        { "n758621.ppt", "xml/n758621_", PPT, -1 },
        { "fdo64586.ppt", "xml/fdo64586_", PPT, -1 },
        { "n819614.pptx", "xml/n819614_", PPTX, -1 },
        { "n820786.pptx", "xml/n820786_", PPTX, -1 },
        { "n762695.pptx", "xml/n762695_", PPTX, -1 },
        { "n593612.pptx", "xml/n593612_", PPTX, -1 },
        { "fdo71434.pptx", "xml/fdo71434_", PPTX, -1 },
        { "n902652.pptx", "xml/n902652_", PPTX, -1 },
        { "tdf90403.pptx", "xml/tdf90403_", PPTX, -1 },
        { "tdf90338.odp", "xml/tdf90338_", ODP, PPTX },
        { "tdf92001.odp", "xml/tdf92001_", ODP, PPTX },
        // { "pptx/n828390.pptx", "pptx/xml/n828390_", PPTX, PPTX }, // Example
    };

    for ( int i = 0; i < static_cast< int >( SAL_N_ELEMENTS( aFilesToCompare ) ); ++i )
    {
        int nUpdateMe = -1; // index of test we want to update; supposedly only when the test is created

        sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc( "/sd/qa/unit/data/" ) + OUString::createFromAscii( aFilesToCompare[i].pInput ), aFilesToCompare[i].nFormat );
        if( aFilesToCompare[i].nExportType >= 0 )
            xDocShRef = saveAndReload( xDocShRef, aFilesToCompare[i].nExportType );
        compareWithShapesDump( xDocShRef,
                getPathFromSrc( "/sd/qa/unit/data/" ) + OUString::createFromAscii( aFilesToCompare[i].pDump ),
                i == nUpdateMe );
    }
}

void SdImportTest::testSmoketest()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/smoketest.pptx"), PPTX);

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    // cf. SdrModel svx/svdmodel.hxx ...

    CPPUNIT_ASSERT_MESSAGE( "wrong page count", pDoc->GetPageCount() == 3);

    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != nullptr );

    const size_t nObjs = pPage->GetObjCount();
    for (size_t i = 0; i < nObjs; i++)
    {
        SdrObject *pObj = pPage->GetObj(i);
        SdrObjKind eKind = (SdrObjKind) pObj->GetObjIdentifier();
        SdrTextObj *pTxt = dynamic_cast<SdrTextObj *>( pObj );
        (void)pTxt; (void)eKind;
    }

    CPPUNIT_ASSERT_MESSAGE( "changed", !pDoc->IsChanged() );

    xDocShRef->DoClose();
}

void SdImportTest::testN759180()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/n759180.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    //sal_uIntPtr nObjs = pPage->GetObjCount();
    //for (sal_uIntPtr i = 0; i < nObjs; i++)
    {
        // Get the object
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT(pTxtObj);
        std::vector<EECharAttrib> rLst;
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        const SvxULSpaceItem *pULSpace = dynamic_cast<const SvxULSpaceItem *>(aEdit.GetParaAttribs(0).GetItem(EE_PARA_ULSPACE));
        CPPUNIT_ASSERT(pULSpace);
        CPPUNIT_ASSERT_MESSAGE( "Para bottom spacing is wrong!", pULSpace->GetLower() == 0 );
        aEdit.GetCharAttribs(1, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxFontHeightItem * pFontHeight = dynamic_cast<const SvxFontHeightItem *>((*it).pAttr);
            if(pFontHeight)
            {
                // nStart == 9
                // font height = 5 => 5*2540/72
                CPPUNIT_ASSERT_MESSAGE( "Font height is wrong", pFontHeight->GetHeight() == 176 );
                break;
            }
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN862510_1()
{
    sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n862510_1.pptx"), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        std::vector<EECharAttrib> rLst;
        SdrObject *pObj = pPage->GetObj( 0 );
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs( 0, rLst );
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxEscapementItem *pFontEscapement = dynamic_cast<const SvxEscapementItem *>( (*it).pAttr );
            CPPUNIT_ASSERT_MESSAGE( "Baseline attribute not handled properly", !( pFontEscapement && pFontEscapement->GetProp() != 100 ) );
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN862510_2()
{
    sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n862510_2.pptx"), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        SdrObjGroup *pGrpObj = dynamic_cast<SdrObjGroup *>( pPage->GetObj( 0 ) );
        CPPUNIT_ASSERT( pGrpObj );
        SdrObjCustomShape *pObj = dynamic_cast<SdrObjCustomShape *>( pGrpObj->GetSubList()->GetObj( 0 ) );
        CPPUNIT_ASSERT( pObj );
        CPPUNIT_ASSERT_MESSAGE( "Wrong Text Rotation!", pObj->GetExtraTextRotation( true ) == 90 );
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN862510_4()
{
    sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n862510_4.pptx"), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        std::vector<EECharAttrib> rLst;
        SdrObject *pObj = pPage->GetObj( 0 );
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs( 0, rLst );
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it != rLst.rend(); ++it )
        {
            const SvxColorItem *pC = dynamic_cast<const SvxColorItem *>( (*it).pAttr );
            CPPUNIT_ASSERT_MESSAGE( "gradfill for text color not handled!", !( pC && pC->GetValue().GetColor() == 0 ) );
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN828390_2()
{
    sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n828390_2.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrObject *pObj = pPage->GetObj(0);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT( pTxtObj );
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT(aEdit.GetText(0) == "Linux  ");
    CPPUNIT_ASSERT(aEdit.GetText(1) == "Standard Platform");

    xDocShRef->DoClose();
}

void SdImportTest::testN828390_3()
{
    bool bPassed = true;
    sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n828390_3.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrObject *pObj = pPage->GetObj(0);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT( pTxtObj );
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    std::vector<EECharAttrib> rLst;
    aEdit.GetCharAttribs(1, rLst);
    for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
    {
        const SvxEscapementItem *pFontEscapement = dynamic_cast<const SvxEscapementItem *>((*it).pAttr);
        if(pFontEscapement)
        {
            if( pFontEscapement->GetEsc() != 0 )
            {
                bPassed = false;
                break;
            }
        }
    }
    CPPUNIT_ASSERT_MESSAGE("CharEscapment not imported properly", bPassed);

    xDocShRef->DoClose();
}

void SdImportTest::testMasterPageStyleParent()
{
    sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/odp/masterpage_style_parent.odp"), ODP );
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    SdStyleSheetPool *const pPool(pDoc->GetSdStyleSheetPool());

    int parents(0);
    SfxStyleSheetIterator iter(pPool, SD_STYLE_FAMILY_MASTERPAGE);
    for (SfxStyleSheetBase * pStyle = iter.First(); pStyle; pStyle = iter.Next())
    {
        OUString const name(pStyle->GetName());
        OUString const parent(pStyle->GetParent());
        if (!parent.isEmpty())
        {
            ++parents;
            // check that parent exists
            SfxStyleSheetBase *const pParentStyle(
                    pPool->Find(parent, SD_STYLE_FAMILY_MASTERPAGE));
            CPPUNIT_ASSERT(pParentStyle);
            CPPUNIT_ASSERT_EQUAL(pParentStyle->GetName(), parent);
            // check that parent has the same master page as pStyle
            CPPUNIT_ASSERT(parent.indexOf(SD_LT_SEPARATOR) != -1);
            CPPUNIT_ASSERT(name.indexOf(SD_LT_SEPARATOR) != -1);
            CPPUNIT_ASSERT_EQUAL(
                    parent.copy(0, parent.indexOf(SD_LT_SEPARATOR)),
                    name.copy(0, name.indexOf(SD_LT_SEPARATOR)));
        }
    }
    // check that there are actually parents...
    CPPUNIT_ASSERT_EQUAL(16, parents);

    xDocShRef->DoClose();
}

void SdImportTest::testGradientAngle()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odg/gradient-angle.fodg"), FODG);

    uno::Reference<lang::XMultiServiceFactory> const xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);

    awt::Gradient gradient;
    uno::Reference<container::XNameAccess> const xGradients(
        xDoc->createInstance("com.sun.star.drawing.GradientTable"),
        uno::UNO_QUERY);

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 38") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), gradient.Angle); // was: 3600

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 10") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(270), gradient.Angle); // 27deg

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 11") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1145), gradient.Angle); // 2rad

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 12") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(900), gradient.Angle); // 100grad

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 13") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3599), gradient.Angle); // -1

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 14") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3028), gradient.Angle); // -1rad

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 15") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(300), gradient.Angle); // 3900

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 16") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(105), gradient.Angle); // 10.5deg

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 17") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1800), gradient.Angle); // \pi rad

    uno::Reference<container::XNameAccess> const xTranspGradients(
        xDoc->createInstance("com.sun.star.drawing.TransparencyGradientTable"),
        uno::UNO_QUERY);

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 2") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), gradient.Angle); // 1

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 1") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(900), gradient.Angle); // 90deg

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 3") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(572), gradient.Angle); // 1.0rad

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 4") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1800), gradient.Angle); // 1000grad
}

void SdImportTest::testN778859()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/n778859.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        // Get the object
        SdrObject *pObj = pPage->GetObj(1);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT(!pTxtObj->IsAutoFit());
    }

    xDocShRef->DoClose();
}

void SdImportTest::testFdo68594()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/ppt/fdo68594.ppt"), PPT);

    const SdrPage *pPage = &(GetPage( 1, xDocShRef )->TRG_GetMasterPage());
    SdrObject *pObj = pPage->GetObj(1);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const SvxColorItem *pC = dynamic_cast<const SvxColorItem *>(&pTxtObj->GetMergedItem(EE_CHAR_COLOR));
    CPPUNIT_ASSERT_MESSAGE( "no color item", pC != nullptr);
    // Color should be black
    CPPUNIT_ASSERT_MESSAGE( "Placeholder color mismatch", pC->GetValue().GetColor() == 0);

    xDocShRef->DoClose();
}

void SdImportTest::testFdo72998()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/cshapes.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        SdrObjCustomShape *pObj = dynamic_cast<SdrObjCustomShape *>(pPage->GetObj(2));
        CPPUNIT_ASSERT( pObj );
        const SdrCustomShapeGeometryItem& rGeometryItem = static_cast<const SdrCustomShapeGeometryItem&>(pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ));
        const css::uno::Any* pViewBox = ((SdrCustomShapeGeometryItem&)rGeometryItem).GetPropertyValueByName( OUString( "ViewBox" ) );
        CPPUNIT_ASSERT_MESSAGE( "Missing ViewBox", pViewBox );
        css::awt::Rectangle aViewBox;
        CPPUNIT_ASSERT( (*pViewBox >>= aViewBox ) );
        CPPUNIT_ASSERT_MESSAGE( "Width should be zero - for forcing scale to 1", !aViewBox.Width );
        CPPUNIT_ASSERT_MESSAGE( "Height should be zero - for forcing scale to 1", !aViewBox.Height );
    }

    xDocShRef->DoClose();
}

// FIXME copypasta
std::ostream& operator<<(std::ostream& rStrm, const Color& rColor)
{
    rStrm << "Color: R:" << rColor.GetRed() << " G:" << rColor.GetGreen() << " B: " << rColor.GetBlue();
    return rStrm;
}

void SdImportTest::testFdo77027()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/fdo77027.odp"), ODP);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        SdrOle2Obj *const pObj = dynamic_cast<SdrOle2Obj*>(pPage->GetObj(0));
        CPPUNIT_ASSERT(pObj);

        // check that the fill style/color was actually imported
        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0xff6600), rColorItem.GetColorValue());
    }

    xDocShRef->DoClose();
}

namespace com { namespace sun { namespace star { namespace uno {

template<class T>
std::ostream& operator<<(std::ostream& rStrm, const uno::Reference<T>& xRef)
{
    rStrm << xRef.get();
    return rStrm;
}

} } } }

void SdImportTest::testTdf97808()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/tdf97808.fodp"), FODP);

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(
        xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("graphics"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName("objectwithoutfill"), uno::UNO_QUERY);
    OUString lineend;
    CPPUNIT_ASSERT(xStyle->getPropertyValue("LineEndName") >>= lineend);
    CPPUNIT_ASSERT_EQUAL(OUString("Arrow"), lineend);

    // the draw:marker-end="" did not override the style
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xLine(
        xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    //uno::Reference<style::XStyle> xParent;
    uno::Reference<beans::XPropertySet> xParent;
    CPPUNIT_ASSERT(xLine->getPropertyValue("Style") >>= xParent);
    CPPUNIT_ASSERT_EQUAL(xStyle, xParent);
    CPPUNIT_ASSERT(xLine->getPropertyValue("LineEndName") >>= lineend);
    CPPUNIT_ASSERT_EQUAL(OUString(), lineend);
}

void SdImportTest::testFdo64512()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo64512.odp"), ODP);

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "not exactly one page", xDoc->getDrawPages()->getCount() == 1 );

    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "no exactly three shapes", xPage->getCount() == 3 );

    uno::Reference< beans::XPropertySet > xConnectorShape(
        xPage->getByIndex(2), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no connector shape", xConnectorShape.is() );

    uno::Reference< beans::XPropertySet > xSvgShape(
        xConnectorShape->getPropertyValue("StartShape"), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no start shape", xSvgShape.is() );

    uno::Reference< beans::XPropertySet > xCustomShape(
        xConnectorShape->getPropertyValue("EndShape"), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no end shape", xCustomShape.is() );

    uno::Reference< animations::XAnimationNodeSupplier > xAnimNodeSupplier(
        xPage, uno::UNO_QUERY_THROW );
    uno::Reference< animations::XAnimationNode > xRootNode(
        xAnimNodeSupplier->getAnimationNode() );
    std::vector< uno::Reference< animations::XAnimationNode > > aAnimVector;
    anim::create_deep_vector(xRootNode, aAnimVector);
    CPPUNIT_ASSERT_MESSAGE( "not 8 animation nodes", aAnimVector.size() == 8 );

    uno::Reference< animations::XAnimate > xNode(
        aAnimVector[7], uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShape > xTargetShape(
        xNode->getTarget(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "inner node not referencing svg shape",
                            xTargetShape != xSvgShape );

    xDocShRef->DoClose();
}

// Unit test for importing charts
void SdImportTest::testFdo71075()
{
    double values[] = { 12.0, 13.0, 14.0 };
    css::uno::Any aAny;
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo71075.odp"), ODP);

    uno::Reference< beans::XPropertySet > xPropSet( getShapeFromPage( 0, 0, xDocShRef ) );
    aAny = xPropSet->getPropertyValue( "Model" );
    CPPUNIT_ASSERT_MESSAGE( "The shape doesn't have the property", aAny.hasValue() );

    uno::Reference< chart::XChartDocument > xChartDoc;
    aAny >>= xChartDoc;
    CPPUNIT_ASSERT_MESSAGE( "failed to load chart", xChartDoc.is() );
    uno::Reference< chart2::XChartDocument > xChart2Doc( xChartDoc, uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "failed to load chart", xChart2Doc.is() );

    uno::Reference< chart2::XCoordinateSystemContainer > xBCooSysCnt( xChart2Doc->getFirstDiagram(), uno::UNO_QUERY );
    uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > aCooSysSeq( xBCooSysCnt->getCoordinateSystems());
    uno::Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[0], uno::UNO_QUERY );

    uno::Reference< chart2::XDataSeriesContainer > xDSCnt( xCTCnt->getChartTypes()[0], uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "failed to load data series", xDSCnt.is() );
    uno::Sequence< uno::Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries());
    CPPUNIT_ASSERT_MESSAGE( "Invalid Series count", aSeriesSeq.getLength() == 1);
    uno::Reference< chart2::data::XDataSource > xSource( aSeriesSeq[0], uno::UNO_QUERY );
    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(xSource->getDataSequences());
    CPPUNIT_ASSERT_MESSAGE( "Invalid Series count", aSeqCnt.getLength() == 1);
    uno::Reference< chart2::data::XDataSequence > xValueSeq( aSeqCnt[0]->getValues() );
    CPPUNIT_ASSERT_MESSAGE( "Invalid Data count", xValueSeq->getData().getLength() == sizeof(values)/(sizeof(double)));
    uno::Reference< chart2::data::XNumericalDataSequence > xNumSeq( xValueSeq, uno::UNO_QUERY );
    uno::Sequence< double > aValues( xNumSeq->getNumericalData());
    for(sal_Int32 i=0;i<xValueSeq->getData().getLength();i++)
        CPPUNIT_ASSERT_MESSAGE( "Invalid Series count", aValues.getConstArray()[i] == values[i]);

    xDocShRef->DoClose();
}

void SdImportTest::testStrictOOXML()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/strict_ooxml.pptx"), PPTX);

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    xDocShRef->DoClose();
}

void SdImportTest::testBnc870237()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc870237.pptx"), PPTX);
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Simulate a:ext inside dsp:txXfrm with changing the lower distance
    const SdrObjGroup* pObj = dynamic_cast<SdrObjGroup*>( pPage->GetObj( 0 ) );
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrMetricItem& >(pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(9919), (static_cast< const SdrMetricItem& >(pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrMetricItem& >(pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrMetricItem& >(pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST))).GetValue());

    xDocShRef->DoClose();
}

void SdImportTest::testCreationDate()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo71434.pptx"), PPTX);
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    util::DateTime aDate = xDocumentProperties->getCreationDate();
    OUStringBuffer aBuffer;
    sax::Converter::convertDateTime(aBuffer, aDate, nullptr);
    // Metadata wasn't imported, this was 0000-00-00.
    CPPUNIT_ASSERT_EQUAL(OUString("2013-11-09T10:37:56"), aBuffer.makeStringAndClear());
    xDocShRef->DoClose();
}

void SdImportTest::testBnc887225()
{
    sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/bnc887225.pptx"), PPTX );
    // In the document, lastRow and lastCol table properties are used.
    // Make sure styles are set properly for individual cells.

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(2));
    CPPUNIT_ASSERT( pTableObj );
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCell;
    sal_Int32 nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6003669), nColor);

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6003669), nColor);

    xCell.set(xTable->getCellByPosition(1, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(13754095), nColor);

    xCell.set(xTable->getCellByPosition(1, 2), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15331319), nColor);

    xCell.set(xTable->getCellByPosition(1, 4), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6003669), nColor);

    xCell.set(xTable->getCellByPosition(3, 2), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6003669), nColor);

    xCell.set(xTable->getCellByPosition(3, 4), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6003669), nColor);

    xDocShRef->DoClose();
}

void SdImportTest::testBnc480256()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc480256.pptx"), PPTX);
    // In the document, there are two tables with table background properties.
    // Make sure colors are set properly for individual cells.

    // TODO: If you are working on improving table background support, expect
    // this unit test to fail. In that case, feel free to change the numbers.

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj;
    uno::Reference< table::XCellRange > xTable;
    uno::Reference< beans::XPropertySet > xCell;
    sal_Int32 nColor;
    table::BorderLine2 aBorderLine;

    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10208238), nColor);
    xCell->getPropertyValue("LeftBorder") >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(util::Color(5609427), aBorderLine.Color);

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(13032959), nColor);
    xCell->getPropertyValue("TopBorder") >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(util::Color(5609427), aBorderLine.Color);

    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(1));
    CPPUNIT_ASSERT( pTableObj );
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7056614), nColor);
    xCell->getPropertyValue("LeftBorder") >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(util::Color(12505062), aBorderLine.Color);

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4626400), nColor);

    xCell.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("BottomBorder") >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(util::Color(0), aBorderLine.Color);

    xDocShRef->DoClose();
}

void SdImportTest::testBnc584721_1()
{
    // Title text shape on the master page contained wrong text.

    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc584721_1_2.pptx"), PPTX);

    const SdrPage *pPage = &(GetPage( 1, xDocShRef )->TRG_GetMasterPage());
    SdrObject *pObj = pPage->GetObj(0);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT_EQUAL(OUString("Click to edit Master title style"), aEdit.GetText(0));
    xDocShRef->DoClose();
}

void SdImportTest::testBnc584721_2()
{
    // Import created an extra/unneeded outliner shape on the master slide next to the imported title shape.

    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc584721_1_2.pptx"), PPTX);

    const SdrPage *pPage = &(GetPage( 1, xDocShRef )->TRG_GetMasterPage());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());

    xDocShRef->DoClose();
}

void SdImportTest::testBnc591147()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc591147.pptx"), PPTX);

    // In the document, there are two slides with media files.
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(2), xDoc->getDrawPages()->getCount() );

    // First page has video file inserted
    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(1), xPage->getCount() );

    //uno::Reference< drawing::XShape > xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropSet( getShape( 0, xPage ) );
    OUString sVideoURL("emptyURL");
    bool bSucess = xPropSet->getPropertyValue("MediaURL") >>= sVideoURL;
    CPPUNIT_ASSERT_MESSAGE( "MediaURL property is not set", bSucess );
    CPPUNIT_ASSERT_MESSAGE("MediaURL is empty", !sVideoURL.isEmpty());

    // Second page has audio file inserted
    xPage.set( xDoc->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(1), xPage->getCount() );

    xPropSet.set( getShape( 0, xPage ) );
    OUString sAudioURL("emptyURL");
    bSucess = xPropSet->getPropertyValue("MediaURL") >>= sAudioURL;
    CPPUNIT_ASSERT_MESSAGE( "MediaURL property is not set", bSucess );
    CPPUNIT_ASSERT_MESSAGE("MediaURL is empty", !sAudioURL.isEmpty());

    CPPUNIT_ASSERT_MESSAGE( "sAudioURL and sVideoURL should not be equal", sAudioURL != sVideoURL );

    xDocShRef->DoClose();
}

void SdImportTest::testBnc584721_4()
{
    // Black text was imported as white because of wrong caching mechanism

    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc584721_4.pptx"), PPTX);
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 1, 1, xDocShRef ) );

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph (0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );
    sal_Int32 nCharColor;
    xPropSet->getPropertyValue( "CharColor" ) >>= nCharColor;

    // Color should be black
    CPPUNIT_ASSERT_EQUAL( sal_Int32(COL_BLACK), nCharColor );

    xDocShRef->DoClose();
}

void SdImportTest::testBnc904423()
{
    // Here the problem was that different fill properties were applied in wrong order on the shape
    // Right order: 1) master slide fill style, 2) theme, 3) direct formatting
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/bnc904423.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    // First shape's background color is defined on master slide
    {
        SdrObject *const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0x00CC99), rColorItem.GetColorValue().GetColor());
    }

    // Second shape's background color is defined by theme
    {
        SdrObject *const pObj = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0x3333CC), rColorItem.GetColorValue().GetColor());
    }

    // Third shape's background color is defined by direct formatting
    {
        SdrObject *const pObj = pPage->GetObj(2);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0xFF0000), rColorItem.GetColorValue().GetColor());
    }

    xDocShRef->DoClose();
}

void SdImportTest::testShapeLineStyle()
{
    // Here the problem was that different line properties were applied in wrong order on the shape
    // Right order: 1) master slide line style, 2) theme, 3) direct formatting
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/ShapeLineProperties.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    // First shape's line style is defined on master slide
    {
        SdrObject *const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pObj->GetMergedItem(XATTR_LINESTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_DASH, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = dynamic_cast<const XLineColorItem&>(
                pObj->GetMergedItem(XATTR_LINECOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0xFF0000), rColorItem.GetColorValue().GetColor());

        const XLineWidthItem& rWidthItem = dynamic_cast<const XLineWidthItem&>(
                pObj->GetMergedItem(XATTR_LINEWIDTH));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(132), rWidthItem.GetValue());
    }

    // Second shape's line style is defined by theme
    {
        SdrObject *const pObj = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pObj->GetMergedItem(XATTR_LINESTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = dynamic_cast<const XLineColorItem&>(
                pObj->GetMergedItem(XATTR_LINECOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0x3333CC), rColorItem.GetColorValue().GetColor());

        const XLineWidthItem& rWidthItem = dynamic_cast<const XLineWidthItem&>(
                pObj->GetMergedItem(XATTR_LINEWIDTH));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(35), rWidthItem.GetValue());
    }

    // Third shape's line style is defined by direct formatting
    {
        SdrObject *const pObj = pPage->GetObj(2);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pObj->GetMergedItem(XATTR_LINESTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = dynamic_cast<const XLineColorItem&>(
                pObj->GetMergedItem(XATTR_LINECOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0x7030A0), rColorItem.GetColorValue().GetColor());

        const XLineWidthItem& rWidthItem = dynamic_cast<const XLineWidthItem&>(
                pObj->GetMergedItem(XATTR_LINEWIDTH));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(176), rWidthItem.GetValue());
    }

    xDocShRef->DoClose();
}

void SdImportTest::testBnc862510_6()
{
    // Black text was imported instead of gray
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc862510_6.pptx"), PPTX);
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );
    sal_Int32 nCharColor;
    xPropSet->getPropertyValue( "CharColor" ) >>= nCharColor;

    // Color should be gray
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0x8B8B8B), nCharColor );

    xDocShRef->DoClose();
}

void SdImportTest::testBnc862510_7()
{
    // Title shape's text was aligned to left instead of center.
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc862510_7.pptx"), PPTX);
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    sal_Int16 nParaAdjust = 0;
    xPropSet->getPropertyValue( "ParaAdjust" ) >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(nParaAdjust));

    xDocShRef->DoClose();
}

#if ENABLE_PDFIMPORT

void SdImportTest::testPDFImport()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pdf/txtpic.pdf"), PDF);
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "no exactly two shapes", xPage->getCount() == 2 );

    uno::Reference< beans::XPropertySet > xShape( getShape( 0, xPage ) );
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_MESSAGE( "not a text shape", xText.is() );

    xDocShRef->DoClose();
}

void SdImportTest::testPDFImportSkipImages()
{
    SfxAllItemSet *pParams = new SfxAllItemSet( SfxGetpApp()->GetPool() );
    pParams->Put( SfxStringItem ( SID_FILE_FILTEROPTIONS, OUString("SkipImages") ) );

    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pdf/txtpic.pdf"), PDF, pParams);
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "no exactly one shape", xPage->getCount() == 1 );

    uno::Reference< drawing::XShape > xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "failed to load shape", xShape.is() );
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_MESSAGE( "not a text shape", xText.is() );

    xDocShRef->DoClose();
}

#endif

void SdImportTest::testBulletSuffix()
{
    sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n83889.pptx"), PPTX );

    // check suffix of the char bullet
    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = dynamic_cast<const SvxNumBulletItem *>(aEdit.GetParaAttribs(1).GetItem(EE_PARA_NUMBULLET));
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's suffix is wrong!", OUString(pNumFmt->GetNumRule()->GetLevel(0).GetSuffix()), OUString("") );
    xDocShRef->DoClose();
}

void SdImportTest::testBnc910045()
{
    // Problem with table style which defines cell color with fill style
    sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/bnc910045.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCell;
    sal_Int32 nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5210557), nColor);
}

void SdImportTest::testRowHeight()
{
    sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n80340.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    sal_Int32 nHeight;
    const OUString sHeight("Height");
    uno::Reference< css::table::XTable > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< css::table::XTableRows > xRows( xTable->getRows(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xRefRow( xRows->getByIndex(0), uno::UNO_QUERY_THROW );
    xRefRow->getPropertyValue( sHeight ) >>= nHeight;
    CPPUNIT_ASSERT_EQUAL( sal_Int32(507), nHeight);
}
void SdImportTest::testTdf93830()
{
    // Text shape offset was ignored
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/tdf93830.pptx"), PPTX);
    uno::Reference< drawing::XDrawPage > xPage( getPage( 0, xDocShRef ) );

    // Get the first text box from group shape
    uno::Reference< container::XIndexAccess > xShape( xPage->getByIndex(0), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xPropSet( xShape->getByIndex(2), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no textbox shape", xPropSet.is() );

    sal_Int32 nTextLeftDistance = 0;
    xPropSet->getPropertyValue( "TextLeftDistance" ) >>= nTextLeftDistance;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4152), nTextLeftDistance);

    xDocShRef->DoClose();
}

void SdImportTest::testTdf93097()
{
    // Throwing metadata import aborted the filter, check that metadata is now imported.
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/tdf93097.pptx"), PPTX);
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    CPPUNIT_ASSERT_EQUAL(OUString("ss"), xDocumentProperties->getTitle());
    xDocShRef->DoClose();
}

void SdImportTest::testTdf93124()
{
#if 0
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/ppt/tdf93124.ppt"), PPT);
    uno::Reference < uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    uno::Reference< drawing::XGraphicExportFilter > xGraphicExporter = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence< beans::PropertyValue > aFilterData(2);
    aFilterData[0].Name = "PixelWidth";
    aFilterData[0].Value <<= (sal_Int32)(320);
    aFilterData[1].Name = "PixelHeight";
    aFilterData[1].Value <<= (sal_Int32)(180);

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();

    uno::Sequence< beans::PropertyValue > aDescriptor(3);
    aDescriptor[0].Name = "URL";
    aDescriptor[0].Value <<= aTempFile.GetURL();
    aDescriptor[1].Name = "FilterName";
    aDescriptor[1].Value <<= OUString("PNG");
    aDescriptor[2].Name = "FilterData";
    aDescriptor[2].Value <<= aFilterData;

    uno::Reference< lang::XComponent > xPage(getPage(0, xDocShRef), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    vcl::PNGReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.Read();
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapReadAccess* pRead = aBMP.AcquireReadAccess();
    int nNonWhiteCount = 0;
    // The word "Top" should be in rectangle 34,4 - 76,30. If text alignment is wrong, the rectangle will be white.
    for (long nX = 34; nX < (34 + 43); ++nX)
        for (long nY = 4; nY < (4 + 26); ++nY)
        {
            const Color aColor = pRead->GetColor(nY, nX);
            if ((aColor.GetRed() != 0xff) || (aColor.GetGreen() != 0xff) || (aColor.GetBlue() != 0xff))
                ++nNonWhiteCount;
        }
    CPPUNIT_ASSERT_MESSAGE("Tdf93124: vertical alignment of text is incorrect!", nNonWhiteCount>100);
    xDocShRef->DoClose();
#endif
}

void SdImportTest::testTdf93868()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/tdf93868.pptx"), PPTX);

    const SdrPage *pPage = &(GetPage( 1, xDocShRef )->TRG_GetMasterPage());
    CPPUNIT_ASSERT_EQUAL(size_t(5), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, dynamic_cast<const XFillStyleItem&>(pPage->GetObj(0)->GetMergedItem(XATTR_FILLSTYLE)).GetValue());
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, dynamic_cast<const XFillStyleItem&>(pPage->GetObj(1)->GetMergedItem(XATTR_FILLSTYLE)).GetValue());

    xDocShRef->DoClose();
}


void SdImportTest::testTdf103792()
{
    // Title text shape on the actual slide contained no text neither a placeholder text.
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/tdf103792.pptx"), PPTX);

    const SdrPage *pPage = GetPage(1, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("No page found", pPage != nullptr);
    SdrObject *pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>(pObj);
    CPPUNIT_ASSERT_MESSAGE("Not a text object", pTxtObj != nullptr);

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT_EQUAL(OUString("Click to add Title"), aEdit.GetText(0));

    xDocShRef->DoClose();
}

void SdImportTest::testTdf103876()
{
    // Title text shape's placeholder text did not inherit the corresponding text properties
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/tdf103876.pptx"), PPTX);
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Check paragraph alignment
    sal_Int16 nParaAdjust = 0;
    xShape->getPropertyValue( "ParaAdjust" ) >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(nParaAdjust));

    // Check character color
    sal_Int32 nCharColor;
    xShape->getPropertyValue( "CharColor" ) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0xFF0000), nCharColor );

    xDocShRef->DoClose();
}

void SdImportTest::testTdf104015()
{
    // Shape fill, line and effect properties were not inherited from master slide shape
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/tdf104015.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    CPPUNIT_ASSERT_MESSAGE("No page found", pPage != nullptr);
    SdrObject *const pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
    // Should have a red fill color
    {
        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0xFF0000), rColorItem.GetColorValue().GetColor());
    }
    // Should have a blue line
    {
        const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pObj->GetMergedItem(XATTR_LINESTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = dynamic_cast<const XLineColorItem&>(
                pObj->GetMergedItem(XATTR_LINECOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0x0000FF), rColorItem.GetColorValue().GetColor());
    }
    // Should have some shadow
    {
        const SdrOnOffItem& rShadowItem = dynamic_cast<const SdrOnOffItem&>(
                pObj->GetMergedItem(SDRATTR_SHADOW));
        CPPUNIT_ASSERT(rShadowItem.GetValue());
    }

    xDocShRef->DoClose();
}

void SdImportTest::testTdf104201()
{
    // Group shape properties did not overwrite the child shapes' fill
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/tdf104201.pptx"), PPTX);

    const SdrPage *pPage = GetPage(1, xDocShRef);
    CPPUNIT_ASSERT_MESSAGE("No page found", pPage != nullptr);

    // First shape has red fill, but this should be overwritten by green group fill
    {
        SdrObject *const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
            pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
            pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0x00FF00), rColorItem.GetColorValue().GetColor());
    }
    // Scond shape has blue fill, but this should be overwritten by green group fill
    {
        SdrObject *const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT_MESSAGE("Wrong object", pObj != nullptr);
        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
            pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
            pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0x00FF00), rColorItem.GetColorValue().GetColor());
    }

    xDocShRef->DoClose();
}

void SdImportTest::testTdf104445()
{
    // Extra bullets were added to the first shape
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/tdf104445.pptx"), PPTX);

    // First shape should not have bullet
   {
        uno::Reference< beans::XPropertySet > xShape(getShapeFromPage(0, 0, xDocShRef));
        uno::Reference< text::XText > xText = uno::Reference< text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
        CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());
        uno::Reference< beans::XPropertySet > xPropSet(xText, uno::UNO_QUERY_THROW);

        uno::Reference< container::XIndexAccess > xNumRule;
        xPropSet->getPropertyValue("NumberingRules") >>= xNumRule;
        uno::Sequence<beans::PropertyValue> aBulletProps;
        xNumRule->getByIndex(0) >>= aBulletProps;

        for (int i = 0; i < aBulletProps.getLength(); ++i)
        {
            const beans::PropertyValue& rProp = aBulletProps[i];
            if(rProp.Name == "NumberingType")
                CPPUNIT_ASSERT_EQUAL(sal_Int16(style::NumberingType::NUMBER_NONE), rProp.Value.get<sal_Int16>());
            if(rProp.Name == "LeftMargin")
                CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rProp.Value.get<sal_Int32>());
        }
    }
    // Second shape should have bullet set
    {
        uno::Reference< beans::XPropertySet > xShape(getShapeFromPage(1, 0, xDocShRef));
        uno::Reference< text::XText > xText = uno::Reference< text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
        CPPUNIT_ASSERT_MESSAGE("Not a text shape", xText.is());
        uno::Reference< beans::XPropertySet > xPropSet(xText, uno::UNO_QUERY_THROW);

        uno::Reference< container::XIndexAccess > xNumRule;
        xPropSet->getPropertyValue("NumberingRules") >>= xNumRule;
        uno::Sequence<beans::PropertyValue> aBulletProps;
        xNumRule->getByIndex(0) >>= aBulletProps;

        for(int i = 0; i < aBulletProps.getLength(); ++i)
        {
            const beans::PropertyValue& rProp = aBulletProps[i];
            if(rProp.Name == "NumberingType")
                CPPUNIT_ASSERT_EQUAL(sal_Int16(style::NumberingType::CHAR_SPECIAL), rProp.Value.get<sal_Int16>());
            if(rProp.Name == "LeftMargin")
                CPPUNIT_ASSERT_EQUAL(sal_Int32(635), rProp.Value.get<sal_Int32>());
        }
    }
    xDocShRef->DoClose();
}

void SdImportTest::testTdf103473()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/tdf103473.pptx"), PPTX);

    const SdrPage *pPage = GetPage(1, xDocShRef);
    SdrTextObj *const pObj = dynamic_cast<SdrTextObj *const>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObj);
    Rectangle aRect = pObj->GetGeoRect();
    CPPUNIT_ASSERT_EQUAL(3629L, aRect.Left());
    CPPUNIT_ASSERT_EQUAL(4431L, aRect.Top());
    CPPUNIT_ASSERT_EQUAL(8353L, aRect.Right());
    CPPUNIT_ASSERT_EQUAL(9155L, aRect.Bottom());

    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
