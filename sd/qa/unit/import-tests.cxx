/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_poppler.h>
#include <ostream>

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
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
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
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/style/NumberingType.hpp>

#include <stlpool.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <vcl/pngread.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/dibtools.hxx>
#include <sfx2/frame.hxx>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/XController2.hpp>

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
    void testTableBorderLineStyle();
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
    void testTdf62255();
    void testTdf93124();
    void testTdf99729();
    void testTdf89927();
    void testTdf93868();
    void testTdf95932();
    void testTdf99030();
    void testTdf49561();
    void testTdf103473();
    void testTdf103567();
    void testTdf103792();
    void testTdf103876();
    void testTdf104015();
    void testTdf104201();
    void testTdf103477();
    void testTdf104445();
    void testTdf105150();
    void testTdf105150PPT();

    bool checkPattern(sd::DrawDocShellRef& rDocRef, int nShapeNumber, std::vector<sal_uInt8>& rExpected);
    void testPatternImport();

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
    CPPUNIT_TEST(testTableBorderLineStyle);
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
    CPPUNIT_TEST(testTdf62255);
    CPPUNIT_TEST(testTdf93124);
    CPPUNIT_TEST(testTdf99729);
    CPPUNIT_TEST(testTdf89927);
    CPPUNIT_TEST(testTdf93868);
    CPPUNIT_TEST(testTdf95932);
    CPPUNIT_TEST(testTdf99030);
    CPPUNIT_TEST(testTdf49561);
    CPPUNIT_TEST(testTdf103473);
    CPPUNIT_TEST(testTdf103567);
    CPPUNIT_TEST(testTdf103792);
    CPPUNIT_TEST(testTdf103876);
    CPPUNIT_TEST(testTdf104015);
    CPPUNIT_TEST(testTdf104201);
    CPPUNIT_TEST(testTdf103477);
    CPPUNIT_TEST(testTdf104445);
    CPPUNIT_TEST(testTdf105150);
    CPPUNIT_TEST(testTdf105150PPT);
    CPPUNIT_TEST(testPatternImport);

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

        sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc( "/sd/qa/unit/data/" ) + OUString::createFromAscii( aFilesToCompare[i].pInput ), aFilesToCompare[i].nFormat );
        if( aFilesToCompare[i].nExportType >= 0 )
            xDocShRef = saveAndReload( xDocShRef.get(), aFilesToCompare[i].nExportType );
        compareWithShapesDump( xDocShRef,
                m_directories.getPathFromSrc( "/sd/qa/unit/data/" ) + OUString::createFromAscii( aFilesToCompare[i].pDump ),
                i == nUpdateMe );
    }
}

void SdImportTest::testSmoketest()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/smoketest.pptx"), PPTX);

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    // cf. SdrModel svx/svdmodel.hxx ...

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong page count", static_cast<sal_uInt16>(3), pDoc->GetPageCount());

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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/n759180.pptx"), PPTX);

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
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Para bottom spacing is wrong!", static_cast<sal_uInt16>(0), pULSpace->GetLower());
        aEdit.GetCharAttribs(1, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxFontHeightItem * pFontHeight = dynamic_cast<const SvxFontHeightItem *>((*it).pAttr);
            if(pFontHeight)
            {
                // nStart == 9
                // font height = 5 => 5*2540/72
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font height is wrong", static_cast<sal_uInt32>(176), pFontHeight->GetHeight() );
                break;
            }
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN862510_1()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/n862510_1.pptx"), PPTX );

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
            CPPUNIT_ASSERT_MESSAGE( "Baseline attribute not handled properly", !(pFontEscapement && pFontEscapement->GetProportionalHeight() != 100) );
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN862510_2()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/n862510_2.pptx"), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        SdrObjGroup *pGrpObj = dynamic_cast<SdrObjGroup *>( pPage->GetObj( 0 ) );
        CPPUNIT_ASSERT( pGrpObj );
        SdrObjCustomShape *pObj = dynamic_cast<SdrObjCustomShape *>( pGrpObj->GetSubList()->GetObj( 0 ) );
        CPPUNIT_ASSERT( pObj );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong Text Rotation!", 90.0, pObj->GetExtraTextRotation( true ) );
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN862510_4()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/n862510_4.pptx"), PPTX );

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
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/n828390_2.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrObject *pObj = pPage->GetObj(0);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT( pTxtObj );
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT_EQUAL(OUString("Linux  "), aEdit.GetText(0));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard Platform"), aEdit.GetText(1));

    xDocShRef->DoClose();
}

void SdImportTest::testN828390_3()
{
    bool bPassed = true;
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/n828390_3.pptx"), PPTX );
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
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/odp/masterpage_style_parent.odp"), ODP );
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odg/gradient-angle.fodg"), FODG);

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

    xDocShRef->DoClose();
}

void SdImportTest::testN778859()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/n778859.pptx"), PPTX);

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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/ppt/fdo68594.ppt"), PPT);

    const SdrPage *pPage = &(GetPage( 1, xDocShRef )->TRG_GetMasterPage());
    SdrObject *pObj = pPage->GetObj(1);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const SvxColorItem *pC = dynamic_cast<const SvxColorItem *>(&pTxtObj->GetMergedItem(EE_CHAR_COLOR));
    CPPUNIT_ASSERT_MESSAGE( "no color item", pC != nullptr);
    // Color should be black
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Placeholder color mismatch", static_cast<ColorData>(0), pC->GetValue().GetColor());

    xDocShRef->DoClose();
}

void SdImportTest::testFdo72998()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/cshapes.pptx"), PPTX);

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

void SdImportTest::testFdo77027()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/fdo77027.odp"), ODP);

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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/tdf97808.fodp"), FODP);

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

    xDocShRef->DoClose();
}

void SdImportTest::testFdo64512()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/fdo64512.odp"), ODP);

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "not exactly one page", static_cast<sal_Int32>(1), xDoc->getDrawPages()->getCount() );

    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "no exactly three shapes", static_cast<sal_Int32>(3), xPage->getCount() );

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "not 8 animation nodes", static_cast<std::size_t>(8), aAnimVector.size() );

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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/fdo71075.odp"), ODP);

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Invalid Series count", static_cast<sal_Int32>(1), aSeriesSeq.getLength() );
    uno::Reference< chart2::data::XDataSource > xSource( aSeriesSeq[0], uno::UNO_QUERY );
    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(xSource->getDataSequences());
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Invalid Series count", static_cast<sal_Int32>(1), aSeqCnt.getLength());
    uno::Reference< chart2::data::XDataSequence > xValueSeq( aSeqCnt[0]->getValues() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Invalid Data count", static_cast<sal_Int32>(sizeof(values)/(sizeof(double))), xValueSeq->getData().getLength());
    uno::Reference< chart2::data::XNumericalDataSequence > xNumSeq( xValueSeq, uno::UNO_QUERY );
    uno::Sequence< double > aValues( xNumSeq->getNumericalData());
    for(sal_Int32 i=0;i<xValueSeq->getData().getLength();i++)
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Invalid Series count", values[i], aValues.getConstArray()[i]);

    xDocShRef->DoClose();
}

void SdImportTest::testStrictOOXML()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/strict_ooxml.pptx"), PPTX);

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    xDocShRef->DoClose();
}

void SdImportTest::testBnc870237()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/bnc870237.pptx"), PPTX);
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/fdo71434.pptx"), PPTX);
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
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/bnc887225.pptx"), PPTX );
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/bnc480256.pptx"), PPTX);
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

    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/bnc584721_1_2.pptx"), PPTX);

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

    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/bnc584721_1_2.pptx"), PPTX);

    const SdrPage *pPage = &(GetPage( 1, xDocShRef )->TRG_GetMasterPage());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());

    xDocShRef->DoClose();
}

void SdImportTest::testBnc591147()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/bnc591147.pptx"), PPTX);

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

    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/bnc584721_4.pptx"), PPTX);
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/bnc904423.pptx"), PPTX);

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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/ShapeLineProperties.pptx"), PPTX);

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

void SdImportTest::testTableBorderLineStyle()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tableBorderLineStyle.pptx"), PPTX );

    // TODO: If you are working on improvement of table border line style
    // support, then expect this unit test to fail.

    const sal_Int16 nObjBorderLineStyles[] =
        {
            ::table::BorderLineStyle::DASHED,
            ::table::BorderLineStyle::DASH_DOT_DOT,
            ::table::BorderLineStyle::DASH_DOT,
            ::table::BorderLineStyle::DOTTED,
            ::table::BorderLineStyle::DASHED,
            ::table::BorderLineStyle::DOTTED,
            ::table::BorderLineStyle::DASHED,
            ::table::BorderLineStyle::DASH_DOT,
            ::table::BorderLineStyle::DASH_DOT,
            ::table::BorderLineStyle::SOLID,
            ::table::BorderLineStyle::NONE
        };

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(nObjBorderLineStyles), pPage->GetObjCount());

    sdr::table::SdrTableObj *pTableObj;
    uno::Reference< table::XCellRange > xTable;
    uno::Reference< beans::XPropertySet > xCell;
    table::BorderLine2 aBorderLine;

    for (size_t i = 0; i < SAL_N_ELEMENTS(nObjBorderLineStyles); i++)
    {
        pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(i));
        CPPUNIT_ASSERT( pTableObj );
        xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);
        xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
        xCell->getPropertyValue("TopBorder") >>= aBorderLine;
        if (aBorderLine.LineWidth > 0) {
            CPPUNIT_ASSERT_EQUAL(nObjBorderLineStyles[i], aBorderLine.LineStyle);
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest::testBnc862510_6()
{
    // Black text was imported instead of gray
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/bnc862510_6.pptx"), PPTX);
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/bnc862510_7.pptx"), PPTX);
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pdf/txtpic.pdf"), PDF);
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "no exactly two shapes", static_cast<sal_Int32>(2), xPage->getCount() );

    uno::Reference< beans::XPropertySet > xShape( getShape( 0, xPage ) );
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_MESSAGE( "not a text shape", xText.is() );

    xDocShRef->DoClose();
}

void SdImportTest::testPDFImportSkipImages()
{
    SfxAllItemSet *pParams = new SfxAllItemSet( SfxGetpApp()->GetPool() );
    pParams->Put( SfxStringItem ( SID_FILE_FILTEROPTIONS, OUString("SkipImages") ) );

    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pdf/txtpic.pdf"), PDF, pParams);
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "no exactly one shape", static_cast<sal_Int32>(1), xPage->getCount() );

    uno::Reference< drawing::XShape > xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "failed to load shape", xShape.is() );
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_MESSAGE( "not a text shape", xText.is() );

    xDocShRef->DoClose();
}

#endif

void SdImportTest::testBulletSuffix()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/n83889.pptx"), PPTX );

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
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/bnc910045.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCell;
    sal_Int32 nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5210557), nColor);

    xDocShRef->DoClose();
}

void SdImportTest::testRowHeight()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/n80340.pptx"), PPTX );
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

    xDocShRef->DoClose();
}
void SdImportTest::testTdf93830()
{
    // Text shape offset was ignored
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf93830.pptx"), PPTX);
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf93097.pptx"), PPTX);
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    CPPUNIT_ASSERT_EQUAL(OUString("ss"), xDocumentProperties->getTitle());
    xDocShRef->DoClose();
}

void SdImportTest::testTdf62255()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf62255.pptx"), PPTX);
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj;
    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    css::uno::Any aAny;
    uno::Reference< table::XCellRange > xTable;
    uno::Reference< beans::XPropertySet > xCell;
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    aAny = xCell->getPropertyValue("FillStyle");

    if (aAny.hasValue())
    {
        drawing::FillStyle aFillStyle;
        aAny >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, aFillStyle);
    }

    xDocShRef->DoClose();
}

void SdImportTest::testTdf93124()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/ppt/tdf93124.ppt"), PPT);
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

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    CPPUNIT_ASSERT_EQUAL(Size(320, 180), aBMPEx.GetSizePixel());
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        Bitmap::ScopedReadAccess pReadAccess(aBMP);
        int nNonWhiteCount = 0;
        // The word "Top" should be in rectangle 34,4 - 76,30. If text alignment is wrong, the rectangle will be white.
        for (long nY = 4; nY < (4 + 26); ++nY)
        {
            for (long nX = 34; nX < (34 + 43); ++nX)
            {
                const Color aColor = pReadAccess->GetColor(nY, nX);
                if ((aColor.GetRed() != 0xff) || (aColor.GetGreen() != 0xff) || (aColor.GetBlue() != 0xff))
                    ++nNonWhiteCount;
            }
        }
        CPPUNIT_ASSERT_MESSAGE("Tdf93124: vertical alignment of text is incorrect!", nNonWhiteCount>50);
    }
    xDocShRef->DoClose();
}

void SdImportTest::testTdf99729()
{
    const char* filenames[] = { "/sd/qa/unit/data/odp/tdf99729-new.odp", "/sd/qa/unit/data/odp/tdf99729-legacy.odp" };
    int nonwhitecounts[] = { 0, 0 };
    for (unsigned int i = 0; i < sizeof(filenames)/sizeof(filenames[0]); ++i)
    {
        // 1st check for new behaviour - having AnchoredTextOverflowLegacy compatibility flag set to false in settings.xml
        sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(filenames[i]), ODP);

        const css::uno::Reference< css::frame::XFrame > xFrame = SfxFrame::CreateBlankFrame();
        CPPUNIT_ASSERT(xFrame.is());
        const css::uno::Reference< css::frame::XModel2 > xModel(xDocShRef->GetModel(), css::uno::UNO_QUERY);
        CPPUNIT_ASSERT(xModel.is());
        const css::uno::Reference< css::frame::XController2 > xController(xModel->createViewController(
            "Default",
            css::uno::Sequence< css::beans::PropertyValue >(),
            xFrame
            ), css::uno::UNO_QUERY);
        CPPUNIT_ASSERT(xController.is());
        xController->attachModel(xModel.get());
        xModel->connectController(xController.get());
        xFrame->setComponent(xController->getComponentWindow(), xController.get());
        xController->attachFrame(xFrame);
        xModel->setCurrentController(xController.get());

        uno::Reference < uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        CPPUNIT_ASSERT(xContext.is());
        uno::Reference< drawing::XGraphicExportFilter > xGraphicExporter = drawing::GraphicExportFilter::create(xContext);
        CPPUNIT_ASSERT(xGraphicExporter.is());

        uno::Sequence< beans::PropertyValue > aFilterData(2);
        aFilterData[0].Name = "PixelWidth";
        aFilterData[0].Value <<= (sal_Int32)(320);
        aFilterData[1].Name = "PixelHeight";
        aFilterData[1].Value <<= (sal_Int32)(240);

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
        Bitmap::ScopedReadAccess pRead(aBMP);
        for (long nX = 154; nX < (154 + 12); ++nX)
        {
            for (long nY = 16; nY < (16 + 96); ++nY)
            {
                const Color aColor = pRead->GetColor(nY, nX);
                if ((aColor.GetRed() != 0xff) || (aColor.GetGreen() != 0xff) || (aColor.GetBlue() != 0xff))
                    ++nonwhitecounts[i];
            }
        }
        xController->dispose();
        xFrame->dispose();
    }
    // The numbers 1-9 should be above the Text Box in rectangle 154,16 - 170,112.
    // If text alignment is wrong, the rectangle will be white.
    CPPUNIT_ASSERT_MESSAGE("Tdf99729: vertical alignment of text is incorrect!", nonwhitecounts[0]>100); // it is 134 with cleartype disabled
    // The numbers 1-9 should be below the Text Box -> rectangle 154,16 - 170,112 should be white.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Tdf99729: legacy vertical alignment of text is incorrect!", 0, nonwhitecounts[1]);
}

void SdImportTest::testTdf89927()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf89927.pptx"), PPTX);
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference< text::XTextRange > xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< text::XTextRange > xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );

    sal_Int32 nCharColor;
    xPropSet->getPropertyValue( "CharColor" ) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0xFFFFFF), nCharColor );

    xDocShRef->DoClose();
}

void SdImportTest::testTdf93868()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf93868.pptx"), PPTX);

    const SdrPage *pPage = &(GetPage( 1, xDocShRef )->TRG_GetMasterPage());
    CPPUNIT_ASSERT_EQUAL(size_t(5), pPage->GetObjCount());
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, dynamic_cast<const XFillStyleItem&>(pPage->GetObj(0)->GetMergedItem(XATTR_FILLSTYLE)).GetValue());
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT, dynamic_cast<const XFillStyleItem&>(pPage->GetObj(1)->GetMergedItem(XATTR_FILLSTYLE)).GetValue());

    xDocShRef->DoClose();
}

void SdImportTest::testTdf95932()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf95932.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrObject *const pObj = pPage->GetObj(2);
    CPPUNIT_ASSERT(pObj);

    const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
        pObj->GetMergedItem(XATTR_FILLSTYLE));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
    const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
        pObj->GetMergedItem(XATTR_FILLCOLOR));
    CPPUNIT_ASSERT_EQUAL(ColorData(0x76bf3d), rColorItem.GetColorValue().GetColor());

    xDocShRef->DoClose();
}

void SdImportTest::testTdf99030()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf99030.pptx"), PPTX);

    uno::Reference< drawing::XMasterPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getMasterPages()->getByIndex( 0 ), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropSet( xPage, uno::UNO_QUERY );

    sal_Int32 nFillColor(0);
    uno::Any aAny = xPropSet->getPropertyValue( "Background" );
    if (aAny.hasValue())
    {
        uno::Reference< beans::XPropertySet > xBackgroundPropSet;
        aAny >>= xBackgroundPropSet;
        xBackgroundPropSet->getPropertyValue( "FillColor" ) >>= nFillColor;
    }
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0x676A55), nFillColor );

    xDocShRef->DoClose();
}

void SdImportTest::testTdf49561()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/ppt/tdf49561.ppt"), PPT);

    uno::Reference< drawing::XMasterPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getMasterPages()->getByIndex( 0 ), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(5), xPage->getCount() );

    uno::Reference< beans::XPropertySet > xShape( getShape( 3, xPage ) );
    uno::Reference<text::XTextRange> xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph (0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet(xRun , uno::UNO_QUERY_THROW );

    float fCharHeight = 0;
    CPPUNIT_ASSERT(xPropSet->getPropertyValue("CharHeight") >>= fCharHeight);
    CPPUNIT_ASSERT_EQUAL(12.f, fCharHeight);

    OUString aCharFontName;
    CPPUNIT_ASSERT(xPropSet->getPropertyValue("CharFontName") >>= aCharFontName);
    CPPUNIT_ASSERT_EQUAL(OUString("Stencil"), aCharFontName);

    xDocShRef->DoClose();
}

void SdImportTest::testTdf103473()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf103473.pptx"), PPTX);

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

void SdImportTest::testTdf103567()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf103567.odp"), ODP);
    for (int i = 0; i < 4; ++i)
    {
        uno::Reference<beans::XPropertySet> const xShape(getShapeFromPage(i, 0, xDocShRef));
        uno::Reference<document::XEventsSupplier> const xEventsSupplier(xShape, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> const xEvents(xEventsSupplier->getEvents());
        OString const msg("shape " + OString::number(i) + ": ");

        CPPUNIT_ASSERT(xEvents->hasByName("OnClick"));
        uno::Sequence<beans::PropertyValue> props;
        xEvents->getByName("OnClick") >>= props;
        comphelper::SequenceAsHashMap const map(props);
        {
            auto iter(map.find("EventType"));
            CPPUNIT_ASSERT_MESSAGE(OString(msg + "no EventType").getStr(), iter != map.end());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), OUString("Presentation"), iter->second.get<OUString>());
        }
        {
            auto iter(map.find("ClickAction"));
            CPPUNIT_ASSERT_MESSAGE(OString(msg + "no ClickAction").getStr(), iter != map.end());
            if (i % 2 == 0)
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), css::presentation::ClickAction_DOCUMENT, iter->second.get<css::presentation::ClickAction>());
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), css::presentation::ClickAction_NEXTPAGE, iter->second.get<css::presentation::ClickAction>());
            }
        }
        if (i % 2 == 0)
        {
            auto iter(map.find("Bookmark"));
            CPPUNIT_ASSERT_MESSAGE(OString(msg + "no Bookmark").getStr(), iter != map.end());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.getStr(), OUString("http://example.com/"), iter->second.get<OUString>());
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest::testTdf103792()
{
    // Title text shape on the actual slide contained no text neither a placeholder text.
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf103792.pptx"), PPTX);

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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf103876.pptx"), PPTX);
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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf104015.pptx"), PPTX);

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
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf104201.pptx"), PPTX);

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
    // Second shape has blue fill, but this should be overwritten by green group fill
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
}

void SdImportTest::testTdf103477()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pptx/tdf103477.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(6) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr );

    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = dynamic_cast<const SvxNumBulletItem *>( aEdit.GetParaAttribs(0).GetItem(EE_PARA_NUMBULLET) );
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's color is wrong!", sal_uInt32(0x000000), pNumFmt->GetNumRule()->GetLevel(1).GetBulletColor().GetColor());

    xDocShRef->DoClose();
}

void SdImportTest::testTdf105150()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf105150.pptx"), PPTX);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    const SdrObject* pObj = pPage->GetObj(1);
    auto& rFillStyleItem = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
    // This was drawing::FillStyle_NONE, <p:sp useBgFill="1"> was ignored when
    // the slide didn't have an explicit background fill.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rFillStyleItem.GetValue());
    xDocShRef->DoClose();
}

void SdImportTest::testTdf105150PPT()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/ppt/tdf105150.ppt"), PPT);
    const SdrPage* pPage = GetPage(1, xDocShRef);
    const SdrObject* pObj = pPage->GetObj(1);
    // This was drawing::FillStyle_NONE, the shape's mso_fillBackground was
    // ignored when the slide didn't have an explicit background fill.
    auto& rFillStyleItem = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rFillStyleItem.GetValue());
    xDocShRef->DoClose();
}

void SdImportTest::testTdf104445()
{
    // Extra bullets were added to the first shape
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/pptx/tdf104445.pptx"), PPTX);

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

namespace
{

bool checkPatternValues(std::vector<sal_uInt8>& rExpected, Bitmap& rBitmap)
{
    bool bResult = true;

    Color aFGColor(0xFF0000);
    Color aBGColor(0xFFFFFF);

    Bitmap::ScopedReadAccess pAccess(rBitmap);
    for (long y = 0; y < pAccess->Height(); ++y)
    {
        for (long x = 0; x < pAccess->Width(); ++x)
        {
            Color aColor = pAccess->GetPixel(y, x);
            sal_uInt8 aValue = rExpected[y*8+x];

            if (aValue == 1 && aColor != aFGColor)
                bResult = false;
            else if (aValue == 0 && aColor != aBGColor)
                bResult = false;
        }
    }

    return bResult;
}

} // end anonymous namespace

bool SdImportTest::checkPattern(sd::DrawDocShellRef& rDocRef, int nShapeNumber, std::vector<sal_uInt8>& rExpected)
{
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(nShapeNumber, 0, rDocRef));
    CPPUNIT_ASSERT_MESSAGE("Not a shape", xShape.is());

    Bitmap aBitmap;
    if (xShape.is())
    {
        uno::Any aBitmapAny = xShape->getPropertyValue("FillBitmap");
        uno::Reference<awt::XBitmap> xBitmap;
        if (aBitmapAny >>= xBitmap)
        {
            uno::Sequence<sal_Int8> aBitmapSequence(xBitmap->getDIB());
            SvMemoryStream aBitmapStream(aBitmapSequence.getArray(),
                                         aBitmapSequence.getLength(),
                                         StreamMode::READ);
            ReadDIB(aBitmap, aBitmapStream, true);
        }
    }
    CPPUNIT_ASSERT_EQUAL(8L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(8L, aBitmap.GetSizePixel().Height());
    return checkPatternValues(rExpected, aBitmap);
}

/* Test checks that importing a PPT file with all supported fill patterns is
 * correctly imported as a tiled fill bitmap with the expected pattern.
 */
void SdImportTest::testPatternImport()
{
    sd::DrawDocShellRef xDocRef = loadURL(m_directories.getURLFromSrc("sd/qa/unit/data/ppt/FillPatterns.ppt"), PPT);

    std::vector<sal_uInt8> aExpectedPattern1 = {
        1,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    };
    std::vector<sal_uInt8> aExpectedPattern2 = {
        1,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,1,0,0,0,
        0,0,0,0,0,0,0,0,
        1,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,1,0,0,0,
        0,0,0,0,0,0,0,0,
    };
    std::vector<sal_uInt8> aExpectedPattern3 = {
        1,0,0,0,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,0,0,0,1,0,
        0,0,0,0,0,0,0,0,
        1,0,0,0,1,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,0,0,0,1,0,
        0,0,0,0,0,0,0,0,
    };
    std::vector<sal_uInt8> aExpectedPattern4 = {
        1,0,0,0,1,0,0,0,
        0,0,1,0,0,0,1,0,
        1,0,0,0,1,0,0,0,
        0,0,1,0,0,0,1,0,
        1,0,0,0,1,0,0,0,
        0,0,1,0,0,0,1,0,
        1,0,0,0,1,0,0,0,
        0,0,1,0,0,0,1,0,
    };
    std::vector<sal_uInt8> aExpectedPattern5 = {
        1,0,1,0,1,0,1,0,
        0,1,0,0,0,1,0,0,
        1,0,1,0,1,0,1,0,
        0,0,0,1,0,0,0,1,
        1,0,1,0,1,0,1,0,
        0,1,0,0,0,1,0,0,
        1,0,1,0,1,0,1,0,
        0,0,0,1,0,0,0,1,
    };
    std::vector<sal_uInt8> aExpectedPattern6 = {
        1,0,1,0,1,0,1,0,
        0,1,0,1,0,1,0,1,
        1,0,1,0,1,0,1,0,
        0,1,0,1,0,0,0,1,
        1,0,1,0,1,0,1,0,
        0,1,0,1,0,1,0,1,
        1,0,1,0,1,0,1,0,
        0,0,0,1,0,1,0,1,
    };
    std::vector<sal_uInt8> aExpectedPattern7 = {
        1,0,1,0,1,0,1,0,
        0,1,0,1,0,1,0,1,
        1,0,1,0,1,0,1,0,
        0,1,0,1,0,1,0,1,
        1,0,1,0,1,0,1,0,
        0,1,0,1,0,1,0,1,
        1,0,1,0,1,0,1,0,
        0,1,0,1,0,1,0,1,
    };
    std::vector<sal_uInt8> aExpectedPattern8 = {
        1,1,1,0,1,1,1,0,
        0,1,0,1,0,1,0,1,
        1,0,1,1,1,0,1,1,
        0,1,0,1,0,1,0,1,
        1,1,1,0,1,1,1,0,
        0,1,0,1,0,1,0,1,
        1,0,1,1,1,0,1,1,
        0,1,0,1,0,1,0,1,
    };
    std::vector<sal_uInt8> aExpectedPattern9 = {
        0,1,1,1,0,1,1,1,
        1,1,0,1,1,1,0,1,
        0,1,1,1,0,1,1,1,
        1,1,0,1,1,1,0,1,
        0,1,1,1,0,1,1,1,
        1,1,0,1,1,1,0,1,
        0,1,1,1,0,1,1,1,
        1,1,0,1,1,1,0,1,
    };
    std::vector<sal_uInt8> aExpectedPattern10 = {
        0,1,1,1,0,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,1,1,1,0,1,
        1,1,1,1,1,1,1,1,
        0,1,1,1,0,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,0,1,1,1,0,1,
        1,1,1,1,1,1,1,1,
    };
    std::vector<sal_uInt8> aExpectedPattern11 = {
        1,1,1,0,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,1,1,
        1,1,1,0,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,0,
        1,1,1,1,1,1,1,1,
    };
    std::vector<sal_uInt8> aExpectedPattern12 = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,0,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,1,1,1,1,1,1,1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine1 = {
        1,0,0,0,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,0,0,0,1,0,
        0,0,0,1,0,0,0,1,
        1,0,0,0,1,0,0,0,
        0,1,0,0,0,1,0,0,
        0,0,1,0,0,0,1,0,
        0,0,0,1,0,0,0,1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine2 = {
        0,0,0,1,0,0,0,1,
        0,0,1,0,0,0,1,0,
        0,1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,0,
        0,0,0,1,0,0,0,1,
        0,0,1,0,0,0,1,0,
        0,1,0,0,0,1,0,0,
        1,0,0,0,1,0,0,0,
    };
    std::vector<sal_uInt8> aExpectedPatternLine3 = {
        1,1,0,0,1,1,0,0,
        0,1,1,0,0,1,1,0,
        0,0,1,1,0,0,1,1,
        1,0,0,1,1,0,0,1,
        1,1,0,0,1,1,0,0,
        0,1,1,0,0,1,1,0,
        0,0,1,1,0,0,1,1,
        1,0,0,1,1,0,0,1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine4 = {
        0,0,1,1,0,0,1,1,
        0,1,1,0,0,1,1,0,
        1,1,0,0,1,1,0,0,
        1,0,0,1,1,0,0,1,
        0,0,1,1,0,0,1,1,
        0,1,1,0,0,1,1,0,
        1,1,0,0,1,1,0,0,
        1,0,0,1,1,0,0,1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine5 = {
        1,1,0,0,0,0,0,1,
        1,1,1,0,0,0,0,0,
        0,1,1,1,0,0,0,0,
        0,0,1,1,1,0,0,0,
        0,0,0,1,1,1,0,0,
        0,0,0,0,1,1,1,0,
        0,0,0,0,0,1,1,1,
        1,0,0,0,0,0,1,1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine6 = {
        1,0,0,0,0,0,1,1,
        0,0,0,0,0,1,1,1,
        0,0,0,0,1,1,1,0,
        0,0,0,1,1,1,0,0,
        0,0,1,1,1,0,0,0,
        0,1,1,1,0,0,0,0,
        1,1,1,0,0,0,0,0,
        1,1,0,0,0,0,0,1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine7 = {
        1,0,0,0,1,0,0,0,
        1,0,0,0,1,0,0,0,
        1,0,0,0,1,0,0,0,
        1,0,0,0,1,0,0,0,
        1,0,0,0,1,0,0,0,
        1,0,0,0,1,0,0,0,
        1,0,0,0,1,0,0,0,
        1,0,0,0,1,0,0,0,
    };
    std::vector<sal_uInt8> aExpectedPatternLine8 = {
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    };
    std::vector<sal_uInt8> aExpectedPatternLine9 = {
        0,1,0,1,0,1,0,1,
        0,1,0,1,0,1,0,1,
        0,1,0,1,0,1,0,1,
        0,1,0,1,0,1,0,1,
        0,1,0,1,0,1,0,1,
        0,1,0,1,0,1,0,1,
        0,1,0,1,0,1,0,1,
        0,1,0,1,0,1,0,1,
    };
    std::vector<sal_uInt8> aExpectedPatternLine10 = {
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,
    };
    std::vector<sal_uInt8> aExpectedPatternLine11 = {
        1,1,0,0,1,1,0,0,
        1,1,0,0,1,1,0,0,
        1,1,0,0,1,1,0,0,
        1,1,0,0,1,1,0,0,
        1,1,0,0,1,1,0,0,
        1,1,0,0,1,1,0,0,
        1,1,0,0,1,1,0,0,
        1,1,0,0,1,1,0,0,
    };
    std::vector<sal_uInt8> aExpectedPatternLine12 = {
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
    };

    CPPUNIT_ASSERT_MESSAGE("Pattern1 - 5%" ,  checkPattern(xDocRef, 0,  aExpectedPattern1));
    CPPUNIT_ASSERT_MESSAGE("Pattern2 - 10%",  checkPattern(xDocRef, 1,  aExpectedPattern2));
    CPPUNIT_ASSERT_MESSAGE("Pattern3 - 20%",  checkPattern(xDocRef, 2,  aExpectedPattern3));
    CPPUNIT_ASSERT_MESSAGE("Pattern4 - 25%",  checkPattern(xDocRef, 3,  aExpectedPattern4));
    CPPUNIT_ASSERT_MESSAGE("Pattern5 - 30%",  checkPattern(xDocRef, 4,  aExpectedPattern5));
    CPPUNIT_ASSERT_MESSAGE("Pattern6 - 40%",  checkPattern(xDocRef, 5,  aExpectedPattern6));
    CPPUNIT_ASSERT_MESSAGE("Pattern7 - 50%",  checkPattern(xDocRef, 6,  aExpectedPattern7));
    CPPUNIT_ASSERT_MESSAGE("Pattern8 - 60%",  checkPattern(xDocRef, 7,  aExpectedPattern8));
    CPPUNIT_ASSERT_MESSAGE("Pattern9 - 70%",  checkPattern(xDocRef, 8,  aExpectedPattern9));
    CPPUNIT_ASSERT_MESSAGE("Pattern10 - 75%", checkPattern(xDocRef, 9,  aExpectedPattern10));
    CPPUNIT_ASSERT_MESSAGE("Pattern11 - 80%", checkPattern(xDocRef, 10, aExpectedPattern11));
    CPPUNIT_ASSERT_MESSAGE("Pattern12 - 90%", checkPattern(xDocRef, 11, aExpectedPattern12));

    CPPUNIT_ASSERT_MESSAGE("Pattern13 - Light downward diagonal", checkPattern(xDocRef, 12, aExpectedPatternLine1));
    CPPUNIT_ASSERT_MESSAGE("Pattern14 - Light upward diagonal",   checkPattern(xDocRef, 13, aExpectedPatternLine2));
    CPPUNIT_ASSERT_MESSAGE("Pattern15 - Dark downward diagonal",  checkPattern(xDocRef, 14, aExpectedPatternLine3));
    CPPUNIT_ASSERT_MESSAGE("Pattern16 - Dark upward diagonal",    checkPattern(xDocRef, 15, aExpectedPatternLine4));
    CPPUNIT_ASSERT_MESSAGE("Pattern17 - Wide downward diagonal",  checkPattern(xDocRef, 16, aExpectedPatternLine5));
    CPPUNIT_ASSERT_MESSAGE("Pattern18 - Wide upward diagonal",    checkPattern(xDocRef, 17, aExpectedPatternLine6));

    CPPUNIT_ASSERT_MESSAGE("Pattern19 - Light vertical",    checkPattern(xDocRef, 18, aExpectedPatternLine7));
    CPPUNIT_ASSERT_MESSAGE("Pattern20 - Light horizontal",  checkPattern(xDocRef, 19, aExpectedPatternLine8));
    CPPUNIT_ASSERT_MESSAGE("Pattern21 - Narrow vertical",   checkPattern(xDocRef, 20, aExpectedPatternLine9));
    CPPUNIT_ASSERT_MESSAGE("Pattern22 - Narrow horizontal", checkPattern(xDocRef, 21, aExpectedPatternLine10));
    CPPUNIT_ASSERT_MESSAGE("Pattern23 - Dark vertical",     checkPattern(xDocRef, 22, aExpectedPatternLine11));
    CPPUNIT_ASSERT_MESSAGE("Pattern24 - Dark horizontal",   checkPattern(xDocRef, 23, aExpectedPatternLine12));

    // TODO: other patterns in the test document

    xDocRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
