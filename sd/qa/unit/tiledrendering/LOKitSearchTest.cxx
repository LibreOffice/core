/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../sdmodeltestbase.hxx"
#include <config_features.h>

#include "CallbackRecorder.hxx"

#include <test/bootstrapfixture.hxx>
#include <test/helper/transferable.hxx>
#include <test/xmltesttools.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/srchitem.hxx>
#include <svl/stritem.hxx>
#include <vcl/scheduler.hxx>
#include <ViewShellBase.hxx>
#include <ViewShell.hxx>
#include <unomodel.hxx>

#include <sdpage.hxx>
#include <svx/svdograf.hxx>

#include <com/sun/star/frame/Desktop.hpp>

using namespace css;

class LOKitSearchTest : public SdModelTestBase, public XmlTestTools
{
private:
    static constexpr char DATA_DIRECTORY[] = "/sd/qa/unit/tiledrendering/data/";

public:
    LOKitSearchTest() = default;

    virtual void setUp() override;
    virtual void tearDown() override;

    void testSearch();
    void testSearchAll();
    void testSearchAllSelections();
    void testSearchAllNotifications();
    void testSearchAllFollowedBySearch();
    void testDontSearchInMasterPages();
    void testSearchInPDFNonExisting();
    void testSearchInPDF();
    void testSearchInPDFOnePDFObject();
    void testSearchInPDFInMultiplePages();
    void testSearchInPDFInMultiplePagesBackwards();
    void testSearchIn2MixedObjects();
    void testSearchIn6MixedObjects();
    void testReplace();
    void testReplaceAll();
    void testReplaceCombined();

    CPPUNIT_TEST_SUITE(LOKitSearchTest);
    CPPUNIT_TEST(testSearch);
    CPPUNIT_TEST(testSearchAll);
    CPPUNIT_TEST(testSearchAllSelections);
    CPPUNIT_TEST(testSearchAllNotifications);
    CPPUNIT_TEST(testSearchAllFollowedBySearch);
    CPPUNIT_TEST(testDontSearchInMasterPages);
    CPPUNIT_TEST(testSearchInPDFNonExisting);
    CPPUNIT_TEST(testSearchInPDF);
    CPPUNIT_TEST(testSearchInPDFOnePDFObject);
    CPPUNIT_TEST(testSearchInPDFInMultiplePages);
    CPPUNIT_TEST(testSearchInPDFInMultiplePagesBackwards);
    CPPUNIT_TEST(testSearchIn2MixedObjects);
    CPPUNIT_TEST(testSearchIn6MixedObjects);
    CPPUNIT_TEST(testReplace);
    CPPUNIT_TEST(testReplaceAll);
    CPPUNIT_TEST(testReplaceCombined);
    CPPUNIT_TEST_SUITE_END();

private:
    SdXImpressDocument* createDoc(const char* pName,
                                  const uno::Sequence<beans::PropertyValue>& rArguments
                                  = uno::Sequence<beans::PropertyValue>());

    uno::Reference<lang::XComponent> mxComponent;
    std::unique_ptr<CallbackRecorder> mpCallbackRecorder;
};

void LOKitSearchTest::setUp()
{
    test::BootstrapFixture::setUp();

    // prevent showing warning message box
    setenv("OOX_NO_SMARTART_WARNING", "1", 1);
    comphelper::LibreOfficeKit::setActive(true);

    mxDesktop.set(
        css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    mpCallbackRecorder = std::make_unique<CallbackRecorder>();
}

void LOKitSearchTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    comphelper::LibreOfficeKit::setActive(false);

    test::BootstrapFixture::tearDown();
}

SdXImpressDocument*
LOKitSearchTest::createDoc(const char* pName, const uno::Sequence<beans::PropertyValue>& rArguments)
{
    if (mxComponent.is())
        mxComponent->dispose();

    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY)
                                  + OUString::createFromAscii(pName));

    CPPUNIT_ASSERT(mxComponent.is());
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering(rArguments);
    return pImpressDocument;
}

namespace
{
void lcl_search(const OUString& rKey, bool bFindAll = false, bool bBackwards = false)
{
    Scheduler::ProcessEventsToIdle();
    SvxSearchCmd eSearch = bFindAll ? SvxSearchCmd::FIND_ALL : SvxSearchCmd::FIND;

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(rKey) },
        { "SearchItem.Backward", uno::makeAny(bBackwards) },
        { "SearchItem.Command", uno::makeAny(sal_uInt16(eSearch)) },
    }));

    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
}

void lcl_replace(const OUString& rKey, const OUString& rReplace, bool bAll = false)
{
    Scheduler::ProcessEventsToIdle();

    SvxSearchCmd eSearch = bAll ? SvxSearchCmd::REPLACE_ALL : SvxSearchCmd::REPLACE;

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(rKey) },
        { "SearchItem.ReplaceString", uno::makeAny(rReplace) },
        { "SearchItem.Command", uno::makeAny(sal_uInt16(eSearch)) },
    }));

    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
}

#if HAVE_FEATURE_PDFIUM
SdrObject* lclGetSelectedObject(sd::ViewShell* pViewShell)
{
    SdrView* pSdrView = pViewShell->GetView();
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rMarkList.GetMarkCount());
    SdrObject* pObject = rMarkList.GetMark(0)->GetMarkedSdrObj();
    return pObject;
}
#endif

} // end anonymous namespace

void LOKitSearchTest::testSearch()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());
    uno::Reference<container::XIndexAccess> xDrawPage(
        pXImpressDocument->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    xShape->setString("Aaa bbb.");

    lcl_search("bbb");

    SdrView* pView = pViewShell->GetView();
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    // Did we indeed manage to select the second word?
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), rEditView.GetSelected());

    // Did the selection callback fire?
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), mpCallbackRecorder->m_aSelection.size());

    // Search for something on the second slide, and make sure that the set-part callback fired.
    lcl_search("bbb");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), mpCallbackRecorder->m_nPart);
    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    // This was 0; should be 1 match for "find".
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1),
                         mpCallbackRecorder->m_aSearchResultSelection.size());
    // Result is on the second slide.
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_aSearchResultPart[0]);

    // This should trigger the not-found callback.
    lcl_search("ccc");
    CPPUNIT_ASSERT_EQUAL(false, mpCallbackRecorder->m_bFound);
}

void LOKitSearchTest::testSearchAll()
{
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    lcl_search("match", /*bFindAll=*/true);

    // This was empty: find-all did not highlight the first match.
    CPPUNIT_ASSERT_EQUAL(OString("match"),
                         apitest::helper::transferable::getTextSelection(
                             pXImpressDocument->getSelection(), "text/plain;charset=utf-8"));

    // We're on the first slide, search for something on the second slide and make sure we get a SET_PART.
    mpCallbackRecorder->m_nPart = 0;
    lcl_search("second", /*bFindAll=*/true);
    // This was 0: no SET_PART was emitted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), mpCallbackRecorder->m_nPart);
}

void LOKitSearchTest::testSearchAllSelections()
{
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    lcl_search("third", /*bFindAll=*/true);
    // Make sure this is found on the 3rd slide.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), mpCallbackRecorder->m_nPart);

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), mpCallbackRecorder->m_aSelection.size());
}

void LOKitSearchTest::testSearchAllNotifications()
{
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    lcl_search("third", /*bFindAll=*/true);
    // Make sure that we get no notifications about selection changes during search.
    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_nSelectionBeforeSearchResult);
    // But we do get the selection of the first hit.
    CPPUNIT_ASSERT(mpCallbackRecorder->m_nSelectionAfterSearchResult > 0);
}

void LOKitSearchTest::testSearchAllFollowedBySearch()
{
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    lcl_search("third", /*bFindAll=*/true);
    lcl_search("match" /*,bFindAll=false*/);

    // This used to give wrong result: 'search' after 'search all' still
    // returned 'third'
    CPPUNIT_ASSERT_EQUAL(OString("match"),
                         apitest::helper::transferable::getTextSelection(
                             pXImpressDocument->getSelection(), "text/plain;charset=utf-8"));
}

void LOKitSearchTest::testDontSearchInMasterPages()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    // This should trigger the not-found callback ("date" is present only on
    // the master page)
    lcl_search("date");
    CPPUNIT_ASSERT_EQUAL(false, mpCallbackRecorder->m_bFound);
}

void LOKitSearchTest::testSearchInPDFNonExisting()
{
#if HAVE_FEATURE_PDFIUM
    SdXImpressDocument* pXImpressDocument = createDoc("PDFSearch.pdf");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);

    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObject);

    SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
    CPPUNIT_ASSERT(pGraphicObject);

    Graphic aGraphic = pGraphicObject->GetGraphic();
    auto const& pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                         pVectorGraphicData->getVectorGraphicDataType());

    lcl_search("NonExisting");

    CPPUNIT_ASSERT_EQUAL(false, mpCallbackRecorder->m_bFound);
#endif
}

void LOKitSearchTest::testSearchInPDF()
{
#if HAVE_FEATURE_PDFIUM
    SdXImpressDocument* pXImpressDocument = createDoc("PDFSearch.pdf");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);

    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObject);

    SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
    CPPUNIT_ASSERT(pGraphicObject);

    Graphic aGraphic = pGraphicObject->GetGraphic();
    auto const& pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                         pVectorGraphicData->getVectorGraphicDataType());

    // Search
    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(OString("3763, 1331, 1432, 483"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(3763, 1331), Size(1433, 484)),
                         mpCallbackRecorder->m_aSelection[0]);

    // Search again - same result
    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(OString("3763, 1331, 1432, 483"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(3763, 1331), Size(1433, 484)),
                         mpCallbackRecorder->m_aSelection[0]);
#endif
}

void LOKitSearchTest::testSearchInPDFOnePDFObject()
{
#if HAVE_FEATURE_PDFIUM
    SdXImpressDocument* pXImpressDocument = createDoc("OnePDFObject.odg");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);

    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObject);

    SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
    CPPUNIT_ASSERT(pGraphicObject);

    Graphic aGraphic = pGraphicObject->GetGraphic();
    auto const& pVectorGraphicData = aGraphic.getVectorGraphicData();
    CPPUNIT_ASSERT(pVectorGraphicData);
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                         pVectorGraphicData->getVectorGraphicDataType());

    // Search down
    lcl_search("ABC", false, false);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    // Search up
    lcl_search("ABC", false, true); // This caused a crash

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);
#endif
}

void LOKitSearchTest::testSearchInPDFInMultiplePages()
{
#if HAVE_FEATURE_PDFIUM
    SdXImpressDocument* pXImpressDocument = createDoc("PDFSearch.pdf");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);

    {
        SdrObject* pObject = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObject);

        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);

        Graphic aGraphic = pGraphicObject->GetGraphic();
        auto const& pVectorGraphicData = aGraphic.getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                             pVectorGraphicData->getVectorGraphicDataType());
    }

    // Search for "him"
    lcl_search("him");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL(OString("9463, 3382, 1099, 499"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search("him");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL(OString("5592, 5038, 1100, 499"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search("him");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(3, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL(OString("9463, 1308, 1099, 499"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search("him");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(4, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL(OString("5592, 2964, 1100, 499"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him" - back to start
    lcl_search("him");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(5, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL(OString("9463, 3382, 1099, 499"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);
#endif
}

void LOKitSearchTest::testSearchInPDFInMultiplePagesBackwards()
{
#if HAVE_FEATURE_PDFIUM
    SdXImpressDocument* pXImpressDocument = createDoc("PDFSearch.pdf");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);

    {
        SdrObject* pObject = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObject);

        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);

        Graphic aGraphic = pGraphicObject->GetGraphic();
        auto const& pVectorGraphicData = aGraphic.getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                             pVectorGraphicData->getVectorGraphicDataType());
    }

    // Expected for backwards search is:
    // - Start with Page 1
    //   + search backwards through objects
    //   + inside objects search backwards through text
    // - Switch to Page 2
    //   + search backwards through objects
    //   + inside objects search backwards through text

    // Search for "him"
    lcl_search("him", /*FindAll*/ false, /*Backwards*/ true);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL(OString("5592, 5038, 1100, 499"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search("him", /*FindAll*/ false, /*Backwards*/ true);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL(OString("9463, 3382, 1099, 499"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search("him", /*FindAll*/ false, /*Backwards*/ true);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(3, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL(OString("5592, 2964, 1100, 499"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search("him", /*FindAll*/ false, /*Backwards*/ true);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(4, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL(OString("9463, 1308, 1099, 499"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him" - back to start
    lcl_search("him", /*FindAll*/ false, /*Backwards*/ true);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(5, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL(OString("5592, 5038, 1100, 499"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);
#endif
}

// Test searching in document with mixed objects.
// We have 2 objects: 1. Text Object, 2. Graphic Object with PDF
void LOKitSearchTest::testSearchIn2MixedObjects()
{
#if HAVE_FEATURE_PDFIUM
    SdXImpressDocument* pXImpressDocument = createDoc("MixedTest1.odg");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    SdDrawDocument* pDocument = pXImpressDocument->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDocument);
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    // Check we have one page
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pDocument->GetSdPageCount(PageKind::Standard));

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);

    // Check page has 2 objects only
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());

    // Check Object 1
    {
        SdrObject* pObject = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObject);

        CPPUNIT_ASSERT_EQUAL(sal_uInt16(OBJ_TEXT), pObject->GetObjIdentifier());
    }

    // Check Object 2
    {
        SdrObject* pObject = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObject);

        CPPUNIT_ASSERT_EQUAL(sal_uInt16(OBJ_GRAF), pObject->GetObjIdentifier());

        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);

        Graphic aGraphic = pGraphicObject->GetGraphic();
        auto const& pVectorGraphicData = aGraphic.getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);

        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                             pVectorGraphicData->getVectorGraphicDataType());
    }

    // Let's try to search now

    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(OString("3546, 3174, 738, 402"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search next

    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(OString("8412, 6385, 519, 174"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search next again - we should get the first object again

    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(3, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(OString("3546, 3174, 738, 402"),
                         mpCallbackRecorder->m_aSearchResultSelection[0]);
#endif
}

// Test searching in document with mixed objects. We have 6 objects.
void LOKitSearchTest::testSearchIn6MixedObjects()
{
#if HAVE_FEATURE_PDFIUM
    SdXImpressDocument* pXImpressDocument = createDoc("MixedTest2.odg");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    SdDrawDocument* pDocument = pXImpressDocument->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDocument);
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    // Check we have one page
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pDocument->GetSdPageCount(PageKind::Standard));

    SdPage* pPage = pViewShell->GetActualPage();
    CPPUNIT_ASSERT(pPage);

    // Check page has 6 objects only
    CPPUNIT_ASSERT_EQUAL(size_t(6), pPage->GetObjCount());

    // Check we have the right objects that we expect

    // Check Object 1
    {
        SdrObject* pObject = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(OBJ_TEXT), pObject->GetObjIdentifier());
    }

    // Check Object 2
    {
        SdrObject* pObject = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(OBJ_GRAF), pObject->GetObjIdentifier());
        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);
        auto const& pVectorGraphicData = pGraphicObject->GetGraphic().getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                             pVectorGraphicData->getVectorGraphicDataType());
    }

    // Check Object 3
    {
        SdrObject* pObject = pPage->GetObj(2);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(OBJ_CUSTOMSHAPE), pObject->GetObjIdentifier());
    }

    // Check Object 4
    {
        SdrObject* pObject = pPage->GetObj(3);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(OBJ_CUSTOMSHAPE), pObject->GetObjIdentifier());
    }

    // Check Object 5
    {
        SdrObject* pObject = pPage->GetObj(4);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(OBJ_GRAF), pObject->GetObjIdentifier());
        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);
        auto const& pVectorGraphicData = pGraphicObject->GetGraphic().getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Svg,
                             pVectorGraphicData->getVectorGraphicDataType());
    }

    // Check Object 6
    {
        SdrObject* pObject = pPage->GetObj(5);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(OBJ_GRAF), pObject->GetObjIdentifier());
        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);
        auto const& pVectorGraphicData = pGraphicObject->GetGraphic().getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf,
                             pVectorGraphicData->getVectorGraphicDataType());
    }

    // Search "ABC" which is in all objects (2 times in Object 3)

    // Object 1
    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(0), lclGetSelectedObject(pViewShell));

    // Object 2
    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(1), lclGetSelectedObject(pViewShell));

    // Object 3
    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(3, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(2), lclGetSelectedObject(pViewShell));

    // Object 3 again
    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(4, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(2), lclGetSelectedObject(pViewShell));

    // Object 4
    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(5, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(3), lclGetSelectedObject(pViewShell));

    // Object 5
    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(6, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(4), lclGetSelectedObject(pViewShell));

    // Object 6
    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(7, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(5), lclGetSelectedObject(pViewShell));

    // Loop to Object 1 again
    lcl_search("ABC");

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(8, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(0), lclGetSelectedObject(pViewShell));
#endif
}
namespace
{
OUString getShapeText(SdXImpressDocument* pXImpressDocument, sal_uInt32 nPage, sal_uInt32 nShape)
{
    uno::Reference<container::XIndexAccess> xDrawPage;
    xDrawPage.set(pXImpressDocument->getDrawPages()->getByIndex(nPage), uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(nShape), uno::UNO_QUERY);
    return xShape->getString();
}
}

void LOKitSearchTest::testReplace()
{
    SdXImpressDocument* pXImpressDocument = createDoc("ReplaceTest.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Bbb bbb bbb bbb"), getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 4, 0));

    lcl_replace("bbb", "aaa", false); // select

    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Bbb bbb bbb bbb"), getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 4, 0));

    lcl_replace("bbb", "aaa", false); // replace

    CPPUNIT_ASSERT_EQUAL(OUString("aaa"), getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Bbb bbb bbb bbb"), getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 4, 0));
}

void LOKitSearchTest::testReplaceAll()
{
    SdXImpressDocument* pXImpressDocument = createDoc("ReplaceTest.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Bbb bbb bbb bbb"), getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 4, 0));

    lcl_replace("bbb", "ccc", true);

    CPPUNIT_ASSERT_EQUAL(OUString("ccc"), getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("ccc ccc ccc ccc"), getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("ccc"), getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("ccc"), getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("ccc"), getShapeText(pXImpressDocument, 4, 0));

    lcl_replace("ccc", "bbb", true);

    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb bbb bbb bbb"), getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 4, 0));
}

void LOKitSearchTest::testReplaceCombined()
{
    SdXImpressDocument* pXImpressDocument = createDoc("ReplaceTest.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    lcl_replace("bbb", "aaa", false); // select
    lcl_replace("bbb", "aaa", false); // replace

    CPPUNIT_ASSERT_EQUAL(OUString("aaa"), getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Bbb bbb bbb bbb"), getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getShapeText(pXImpressDocument, 4, 0));

    lcl_replace("bbb", "ccc", true);

    CPPUNIT_ASSERT_EQUAL(OUString("aaa"), getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("ccc ccc ccc ccc"), getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("ccc"), getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("ccc"), getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("ccc"), getShapeText(pXImpressDocument, 4, 0));
}

CPPUNIT_TEST_SUITE_REGISTRATION(LOKitSearchTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
