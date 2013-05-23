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

#include <svx/svdotext.hxx>
#include <animations/animationnodehelper.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;

/// Impress import filters tests.
class SdFiltersTest : public SdModelTestBase
{
public:
    void testDocumentLayout();
    void testSmoketest();
    void testN759180();
    void testN778859();
    void testFdo64512();

    CPPUNIT_TEST_SUITE(SdFiltersTest);
    CPPUNIT_TEST(testDocumentLayout);
    CPPUNIT_TEST(testSmoketest);
    CPPUNIT_TEST(testN759180);
    CPPUNIT_TEST(testN778859);
    CPPUNIT_TEST(testFdo64512);
    CPPUNIT_TEST_SUITE_END();
};

/** Test document against a reference XML dump of shapes.

If you want to update one of these tests, set the nUpdateMe to the index of
the test, the dump XML's will be created (or rewritten) instead of checking.
Use with care - when the test is failing, first find out why, instead of just
updating .xml's blindly.

NOTE: This approach is suitable only for tests of fixes that actually change
the layout - best to check by reverting your fix locally after having added
the test, and re-running; it should break.
*/
void SdFiltersTest::testDocumentLayout()
{
    struct { const char *pInput, *pDump; } aFilesToCompare[] =
    {
        { "odp/shapes-test.odp", "xml/shapes-test_page" },
        { "pptx/fdo47434-all.pptx", "pptx/xml/fdo47434_page" },
        { "n758621.ppt", "xml/n758621_" },
        { "fdo64586.ppt", "xml/fdo64586_" },
        { "n819614.pptx", "xml/n819614_" },
        { "n820786.pptx", "xml/n820786_" },
    };

    for ( int i = 0; i < static_cast< int >( SAL_N_ELEMENTS( aFilesToCompare ) ); ++i )
    {
        int nUpdateMe = -1; // index of test we want to update; supposedly only when the test is created

        ::sd::DrawDocShellRef xDocShRef = loadURL( getURLFromSrc( "/sd/qa/unit/data/" ) + OUString::createFromAscii( aFilesToCompare[i].pInput ) );
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

CPPUNIT_TEST_SUITE_REGISTRATION(SdFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
