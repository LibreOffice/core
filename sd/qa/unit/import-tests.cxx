/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <svl/stritem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>

#include <svx/svdotext.hxx>
#include <svx/svdoashp.hxx>
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

using namespace ::com::sun::star;

/// Impress import filters tests.
class SdFiltersTest : public SdModelTestBase
{
public:
    void testDocumentLayout();
    void testSmoketest();
    void testFdo77027();
    void testN759180();
    void testN778859();
    void testFdo64512();
    void testFdo71075();
    void testN828390();
    void testFdo71961();
    void testBnc870237();
    void testBnc870233_1();
    void testBnc870233_2();
    void testBnc880763();
    void testBnc862510_5();
    void testBnc480256();
    void testCreationDate();
    void testBnc584721_4();
    void testBnc904423();
    void testShapeLineStyle();
    void testBnc862510_6();

    CPPUNIT_TEST_SUITE(SdFiltersTest);
    CPPUNIT_TEST(testDocumentLayout);
    CPPUNIT_TEST(testSmoketest);
    CPPUNIT_TEST(testFdo77027);
    CPPUNIT_TEST(testN759180);
    CPPUNIT_TEST(testN778859);
    CPPUNIT_TEST(testFdo64512);
    CPPUNIT_TEST(testFdo71075);
    CPPUNIT_TEST(testN828390);
    CPPUNIT_TEST(testFdo71961);
    CPPUNIT_TEST(testBnc870237);
    CPPUNIT_TEST(testBnc870233_1);
    CPPUNIT_TEST(testBnc870233_2);
    CPPUNIT_TEST(testBnc880763);
    CPPUNIT_TEST(testBnc862510_5);
    CPPUNIT_TEST(testBnc480256);
    CPPUNIT_TEST(testCreationDate);
    CPPUNIT_TEST(testBnc584721_4);
    CPPUNIT_TEST(testBnc904423);
    CPPUNIT_TEST(testShapeLineStyle);
    CPPUNIT_TEST(testBnc862510_6);

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
void SdFiltersTest::testDocumentLayout()
{
    struct { const char *pInput, *pDump; sal_Int32 nExportType; } aFilesToCompare[] =
    {
        { "odp/shapes-test.odp", "xml/shapes-test_page", -1 },
        { "pptx/fdo47434-all.pptx", "pptx/xml/fdo47434_page", -1 },
        { "n758621.ppt", "xml/n758621_", -1 },
        { "fdo64586.ppt", "xml/fdo64586_", -1 },
        { "n819614.pptx", "xml/n819614_", -1 },
        { "n820786.pptx", "xml/n820786_", -1 },
        { "n762695.pptx", "xml/n762695_", -1 },
        { "n593612.pptx", "xml/n593612_", -1 },
        // { "pptx/n828390.pptx", "pptx/xml/n828390_", PPTX }, // Example
    };

    for ( int i = 0; i < static_cast< int >( SAL_N_ELEMENTS( aFilesToCompare ) ); ++i )
    {
        int nUpdateMe = -1; // index of test we want to update; supposedly only when the test is created

        ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc( "/sd/qa/unit/data/" ) + OUString::createFromAscii( aFilesToCompare[i].pInput ) );
        if( aFilesToCompare[i].nExportType >= 0 )
            xDocShRef = saveAndReload( xDocShRef, aFilesToCompare[i].nExportType );
        compareWithShapesDump( xDocShRef,
                getPathFromSrc( "/sd/qa/unit/data/" ) + OUString::createFromAscii( aFilesToCompare[i].pDump ),
                i == nUpdateMe );
    }
}

void SdFiltersTest::testSmoketest()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/smoketest.pptx"));
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );

    // cf. SdrModel svx/svdmodel.hxx ...

    CPPUNIT_ASSERT_MESSAGE( "wrong page count", pDoc->GetPageCount() == 3);

    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    sal_uIntPtr nObjs = pPage->GetObjCount();
    for (sal_uIntPtr i = 0; i < nObjs; i++)
    {
        SdrObject *pObj = pPage->GetObj(i);
        SdrObjKind eKind = (SdrObjKind) pObj->GetObjIdentifier();
        SdrTextObj *pTxt = dynamic_cast<SdrTextObj *>( pObj );
        (void)pTxt; (void)eKind;
    }

    CPPUNIT_ASSERT_MESSAGE( "changed", !pDoc->IsChanged() );
    xDocShRef->DoClose();
}

void SdFiltersTest::testN759180()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/n759180.pptx"));
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

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
}

void SdFiltersTest::testN828390()
{
    bool bPassed = false;
    ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc("/sd/qa/unit/data/pptx/n828390.pptx") );
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );

    xDocShRef = saveAndReload( xDocShRef, PPTX );
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "in destruction", !xDocShRef->IsInDestruction() );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );
    {
        std::vector<EECharAttrib> rLst;
        // Get the object
        SdrObject *pObj = pPage->GetObj(0);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
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
}

void SdFiltersTest::testN778859()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/n778859.pptx"));
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );
    {
        // Get the object
        SdrObject *pObj = pPage->GetObj(1);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT(!pTxtObj->IsAutoFit());
    }
}

// FIXME copypasta
std::ostream& operator<<(std::ostream& rStrm, const Color& rColor)
{
    rStrm << "Color: R:" << rColor.GetRed() << " G:" << rColor.GetGreen() << " B: << rColor.GetBlue()";
    return rStrm;
}

void SdFiltersTest::testFdo77027()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/fdo77027.odp"));

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );
    {
        SdrOle2Obj *const pObj = dynamic_cast<SdrOle2Obj*>(pPage->GetObj(0));
        CPPUNIT_ASSERT(pObj);

        // check that the fill style/color was actually imported
        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(XFILL_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0xff6600), rColorItem.GetColorValue());
    }

    xDocShRef->DoClose();
}

void SdFiltersTest::testFdo64512()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo64512.odp"));
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

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
}

// Unit test for importing charts
void SdFiltersTest::testFdo71075()
{
    double values[] = { 12.0, 13.0, 14.0 };
    ::com::sun::star::uno::Any aAny;
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo71075.odp"));
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShape > xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "failed to load shape", xShape.is() );

    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY );
    aAny = xPropSet->getPropertyValue( OUString("Model") );
    CPPUNIT_ASSERT_MESSAGE( "failed to load shape", aAny.hasValue() );

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
}

void SdFiltersTest::testFdo71961()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/fdo71961.odp"));
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

    xDocShRef = saveAndReload( xDocShRef, PPTX );
    CPPUNIT_ASSERT_MESSAGE( "failed to load", xDocShRef.Is() );
    CPPUNIT_ASSERT_MESSAGE( "not in destruction", !xDocShRef->IsInDestruction() );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // Export to .pptx changes all text frames to custom shape objects, which obey TextWordWrap property
    // (which is false for text frames otherwise and is ignored). Check that frames that should wrap still do.
    SdrObjCustomShape *pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 1 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != NULL);
    CPPUNIT_ASSERT_EQUAL( OUString( "Text to be always wrapped" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( true, (static_cast<const SdrTextWordWrapItem&>(pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 2 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != NULL);
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom shape non-wrapped text" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( false, (static_cast<const SdrTextWordWrapItem&>(pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue());

    pTxtObj = dynamic_cast<SdrObjCustomShape *>( pPage->GetObj( 3 ));
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != NULL);
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom shape wrapped text" ), pTxtObj->GetOutlinerParaObject()->GetTextObject().GetText(0));
    CPPUNIT_ASSERT_EQUAL( true, (static_cast<const SdrTextWordWrapItem&>(pTxtObj->GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue());
}

void SdFiltersTest::testBnc870237()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc870237.pptx"));

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // Simulate a:ext inside dsp:txXfrm with changing the lower distance
    const SdrObjGroup* pObj = dynamic_cast<SdrObjGroup*>( pPage->GetObj( 0 ) );
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrTextUpperDistItem& >(pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(9919), (static_cast< const SdrTextLowerDistItem& >(pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrTextRightDistItem& >(pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrTextLeftDistItem& >(pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST))).GetValue());

    xDocShRef->DoClose();
}

void SdFiltersTest::testCreationDate()
{
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/n593612.pptx"));
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    util::DateTime aDate = xDocumentProperties->getCreationDate();
    OUStringBuffer aBuffer;
    sax::Converter::convertDateTime(aBuffer, aDate, 0);
    // Metadata wasn't imported, this was 0000-00-00.
    CPPUNIT_ASSERT_EQUAL(OUString("2008-12-10T19:12:35"), aBuffer.makeStringAndClear());
}

void SdFiltersTest::testBnc870233_1()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc870233_1.pptx"));
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // The problem was all shapes had the same font (the last parsed font attribues overwrote all previous ones)

    // First shape has red, bold font
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
        const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
        std::vector<EECharAttrib> rLst;
        aEdit.GetCharAttribs(0, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxColorItem *pCharColor = dynamic_cast<const SvxColorItem *>((*it).pAttr);
            if( pCharColor )
            {
                CPPUNIT_ASSERT_EQUAL( sal_uInt32(0xff0000), pCharColor->GetValue().GetColor());
            }
            const SvxWeightItem *pWeight = dynamic_cast<const SvxWeightItem *>((*it).pAttr);
            if( pWeight )
            {
                CPPUNIT_ASSERT_EQUAL( WEIGHT_BOLD, pWeight->GetWeight());
            }
            const SvxPostureItem *pPosture = dynamic_cast<const SvxPostureItem *>((*it).pAttr);
            if( pPosture )
            {
                CPPUNIT_ASSERT_EQUAL( ITALIC_NONE, pPosture->GetPosture());
            }
        }
    }

    // Second shape has blue, italic font
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 1 ) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
        const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
        std::vector<EECharAttrib> rLst;
        aEdit.GetCharAttribs(0, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxColorItem *pCharColor = dynamic_cast<const SvxColorItem *>((*it).pAttr);
            if( pCharColor )
            {
                CPPUNIT_ASSERT_EQUAL( sal_uInt32(0x0000ff), pCharColor->GetValue().GetColor());
            }
            const SvxWeightItem *pWeight = dynamic_cast<const SvxWeightItem *>((*it).pAttr);
            if( pWeight )
            {
                CPPUNIT_ASSERT_EQUAL( WEIGHT_NORMAL, pWeight->GetWeight());
            }
            const SvxPostureItem *pPosture = dynamic_cast<const SvxPostureItem *>((*it).pAttr);
            if( pPosture )
            {
                CPPUNIT_ASSERT_EQUAL( ITALIC_NORMAL, pPosture->GetPosture());
            }
        }
    }

    xDocShRef->DoClose();
}

void SdFiltersTest::testBnc870233_2()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc870233_2.pptx"));
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // The problem was in some SmartArts font color was wrong

    // First smart art has blue font color (direct formatting)
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 0 ) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
        const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
        std::vector<EECharAttrib> rLst;
        aEdit.GetCharAttribs(0, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxColorItem *pCharColor = dynamic_cast<const SvxColorItem *>((*it).pAttr);
            if( pCharColor )
            {
                CPPUNIT_ASSERT_EQUAL( sal_uInt32(0x0000ff), pCharColor->GetValue().GetColor());
            }
        }
    }

    // Second smart art has "dk2" font color (style)
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 1 ) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
        const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
        std::vector<EECharAttrib> rLst;
        aEdit.GetCharAttribs(0, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxColorItem *pCharColor = dynamic_cast<const SvxColorItem *>((*it).pAttr);
            if( pCharColor )
            {
                CPPUNIT_ASSERT_EQUAL( sal_uInt32(0x1F497D), pCharColor->GetValue().GetColor());
            }
        }
    }

    // Third smart art has white font color (style)
    {
        const SdrTextObj *pObj = dynamic_cast<SdrTextObj *>( pPage->GetObj( 2 ) );
        CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
        const EditTextObject& aEdit = pObj->GetOutlinerParaObject()->GetTextObject();
        std::vector<EECharAttrib> rLst;
        aEdit.GetCharAttribs(0, rLst);
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxColorItem *pCharColor = dynamic_cast<const SvxColorItem *>((*it).pAttr);
            if( pCharColor )
            {
                CPPUNIT_ASSERT_EQUAL( sal_uInt32(0xffffff), pCharColor->GetValue().GetColor());
            }
        }
    }

    xDocShRef->DoClose();
}

void SdFiltersTest::testBnc880763()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc880763.pptx"));
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

void SdFiltersTest::testBnc862510_5()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc862510_5.pptx"));
    xDocShRef = saveAndReload( xDocShRef, PPTX );

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // Same as testBnc870237, but here we check the horizontal spacing
    const SdrObject* pObj = dynamic_cast<SdrObject*>( pPage->GetObj( 1 ) );
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != NULL);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrTextUpperDistItem& >(pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrTextLowerDistItem& >(pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(7510), (static_cast< const SdrTextRightDistItem& >(pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST))).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), (static_cast< const SdrTextLeftDistItem& >(pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST))).GetValue());

    xDocShRef->DoClose();
}

void SdFiltersTest::testBnc480256()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc480256.pptx"));
    // In the document, there are two tables with table background properties.
    // Make sure colors are set properly for individual cells.

    // TODO: If you are working on improving table background support, expect
    // this unit test to fail. In that case, feel free to change the numbers.

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

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

void SdFiltersTest::testBnc584721_4()
{
    // Black text was imported as white because of wrong caching mechanism

    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc584721_4.pptx"));

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );

    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySet > xShape(
        xPage->getByIndex(1), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no text shape", xShape.is() );

    // Get first paragraph of the text
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_MESSAGE( "no text shape", xText.is() );
    uno::Reference<container::XEnumerationAccess> paraEnumAccess;
    paraEnumAccess.set(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> const xParagraph(paraEnum->nextElement(),
                uno::UNO_QUERY_THROW);

    // Get first run of the paragraph
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );
    sal_Int32 nCharColor;
    xPropSet->getPropertyValue( "CharColor" ) >>= nCharColor;

    // Color should be black
    CPPUNIT_ASSERT_EQUAL( sal_Int32(COL_BLACK), nCharColor );

    xDocShRef->DoClose();
}

void SdFiltersTest::testBnc904423()
{
    // Here the problem was that different fill properties were applied in wrong order on the shape
    // Right order: 1) master slide fill style, 2) theme, 3) direct formatting
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/bnc904423.pptx"));

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // First shape's background color is defined on master slide
    {
        SdrObject *const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(XFILL_SOLID, rStyleItem.GetValue());
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
        CPPUNIT_ASSERT_EQUAL(XFILL_SOLID, rStyleItem.GetValue());
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
        CPPUNIT_ASSERT_EQUAL(XFILL_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0xFF0000), rColorItem.GetColorValue().GetColor());
    }

    xDocShRef->DoClose();
}

void SdFiltersTest::testShapeLineStyle()
{
    // Here the problem was that different line properties were applied in wrong order on the shape
    // Right order: 1) master slide line style, 2) theme, 3) direct formatting
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("sd/qa/unit/data/pptx/ShapeLineProperties.pptx"));

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != NULL );
    const SdrPage *pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != NULL );

    // First shape's line style is defined on master slide
    {
        SdrObject *const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pObj->GetMergedItem(XATTR_LINESTYLE));
        CPPUNIT_ASSERT_EQUAL(XLINE_DASH, rStyleItem.GetValue());

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
        CPPUNIT_ASSERT_EQUAL(XLINE_SOLID, rStyleItem.GetValue());

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
        CPPUNIT_ASSERT_EQUAL(XLINE_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = dynamic_cast<const XLineColorItem&>(
                pObj->GetMergedItem(XATTR_LINECOLOR));
        CPPUNIT_ASSERT_EQUAL(ColorData(0x7030A0), rColorItem.GetColorValue().GetColor());

        const XLineWidthItem& rWidthItem = dynamic_cast<const XLineWidthItem&>(
                pObj->GetMergedItem(XATTR_LINEWIDTH));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(176), rWidthItem.GetValue());
    }

    xDocShRef->DoClose();
}

void SdFiltersTest::testBnc862510_6()
{
    // Black text was imported instead of gray
    ::sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/pptx/bnc862510_6.pptx"));

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );

    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySet > xShape(
        xPage->getByIndex(0), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no shape", xShape.is() );

    // Get first paragraph of the text
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_MESSAGE( "not a text shape", xText.is() );
    uno::Reference<container::XEnumerationAccess> paraEnumAccess;
    paraEnumAccess.set(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> const xParagraph(paraEnum->nextElement(),
                uno::UNO_QUERY_THROW);

    // Get first run of the paragraph
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );
    sal_Int32 nCharColor;
    xPropSet->getPropertyValue( "CharColor" ) >>= nCharColor;

    // Color should be gray
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0x8B8B8B), nCharColor );

    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
