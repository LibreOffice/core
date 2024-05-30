/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CallbackRecorder.hxx"
#include <test/unoapi_test.hxx>

#include <test/helper/transferable.hxx>

#include <comphelper/propertysequence.hxx>
#include <comphelper/lok.hxx>
#include <svl/srchitem.hxx>
#include <vcl/scheduler.hxx>
#include <ViewShellBase.hxx>
#include <ViewShell.hxx>
#include <unomodel.hxx>

#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <svx/svdograf.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace css;

class LOKitSearchTest : public UnoApiTest
{
public:
    LOKitSearchTest()
        : UnoApiTest(u"/sd/qa/unit/tiledrendering/data/"_ustr)
    {
    }

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
    void lcl_search(const OUString& rKey, bool bFindAll = false, bool bBackwards = false);
    void lcl_replace(const OUString& rKey, const OUString& rReplace, bool bAll = false);

    std::unique_ptr<CallbackRecorder> mpCallbackRecorder;
};

void LOKitSearchTest::setUp()
{
    UnoApiTest::setUp();

    // prevent showing warning message box
    setenv("OOX_NO_SMARTART_WARNING", "1", 1);
    comphelper::LibreOfficeKit::setActive(true);

    mpCallbackRecorder = std::make_unique<CallbackRecorder>();
}

void LOKitSearchTest::tearDown()
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }

    comphelper::LibreOfficeKit::setActive(false);

    UnoApiTest::tearDown();
}

SdXImpressDocument*
LOKitSearchTest::createDoc(const char* pName, const uno::Sequence<beans::PropertyValue>& rArguments)
{
    loadFromFile(OUString::createFromAscii(pName));
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering(rArguments);
    return pImpressDocument;
}

void LOKitSearchTest::lcl_search(const OUString& rKey, bool bFindAll, bool bBackwards)
{
    Scheduler::ProcessEventsToIdle();
    SvxSearchCmd eSearch = bFindAll ? SvxSearchCmd::FIND_ALL : SvxSearchCmd::FIND;

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::Any(rKey) },
        { "SearchItem.Backward", uno::Any(bBackwards) },
        { "SearchItem.Command", uno::Any(sal_uInt16(eSearch)) },
    }));

    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aPropertyValues);
}

void LOKitSearchTest::lcl_replace(const OUString& rKey, const OUString& rReplace, bool bAll)
{
    Scheduler::ProcessEventsToIdle();

    SvxSearchCmd eSearch = bAll ? SvxSearchCmd::REPLACE_ALL : SvxSearchCmd::REPLACE;

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::Any(rKey) },
        { "SearchItem.ReplaceString", uno::Any(rReplace) },
        { "SearchItem.Command", uno::Any(sal_uInt16(eSearch)) },
    }));

    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aPropertyValues);
}

namespace
{
SdrObject* lclGetSelectedObject(sd::ViewShell* pViewShell)
{
    SdrView* pSdrView = pViewShell->GetView();
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rMarkList.GetMarkCount());
    SdrObject* pObject = rMarkList.GetMark(0)->GetMarkedSdrObj();
    return pObject;
}

} // end anonymous namespace

void LOKitSearchTest::testSearch()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());
    uno::Reference<container::XIndexAccess> xDrawPage(
        pXImpressDocument->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    xShape->setString(u"Aaa bbb."_ustr);

    lcl_search(u"bbb"_ustr);

    SdrView* pView = pViewShell->GetView();
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    // Did we indeed manage to select the second word?
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, rEditView.GetSelected());

    // Did the selection callback fire?
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), mpCallbackRecorder->m_aSelection.size());

    // Search for something on the second slide, and make sure that the set-part callback fired.
    lcl_search(u"bbb"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), mpCallbackRecorder->m_nPart);
    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    // This was 0; should be 1 match for "find".
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1),
                         mpCallbackRecorder->m_aSearchResultSelection.size());
    // Result is on the second slide.
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_aSearchResultPart[0]);

    // This should trigger the not-found callback.
    lcl_search(u"ccc"_ustr);
    CPPUNIT_ASSERT_EQUAL(false, mpCallbackRecorder->m_bFound);
}

void LOKitSearchTest::testSearchAll()
{
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    lcl_search(u"match"_ustr, /*bFindAll=*/true);

    // This was empty: find-all did not highlight the first match.
    CPPUNIT_ASSERT_EQUAL("match"_ostr,
                         apitest::helper::transferable::getTextSelection(
                             pXImpressDocument->getSelection(), "text/plain;charset=utf-8"_ostr));

    // We're on the first slide, search for something on the second slide and make sure we get a SET_PART.
    mpCallbackRecorder->m_nPart = 0;
    lcl_search(u"second"_ustr, /*bFindAll=*/true);
    // This was 0: no SET_PART was emitted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), mpCallbackRecorder->m_nPart);
}

void LOKitSearchTest::testSearchAllSelections()
{
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    lcl_search(u"third"_ustr, /*bFindAll=*/true);
    // Make sure this is found on the 3rd slide.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), mpCallbackRecorder->m_nPart);
    // This was 1: only the first match was highlighted.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), mpCallbackRecorder->m_aSelection.size());
}

void LOKitSearchTest::testSearchAllNotifications()
{
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    lcl_search(u"third"_ustr, /*bFindAll=*/true);
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

    lcl_search(u"third"_ustr, /*bFindAll=*/true);
    lcl_search(u"match"_ustr /*,bFindAll=false*/);

    // This used to give wrong result: 'search' after 'search all' still
    // returned 'third'
    CPPUNIT_ASSERT_EQUAL("match"_ostr,
                         apitest::helper::transferable::getTextSelection(
                             pXImpressDocument->getSelection(), "text/plain;charset=utf-8"_ostr));
}

void LOKitSearchTest::testDontSearchInMasterPages()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    // This should trigger the not-found callback ("date" is present only on
    // the master page)
    lcl_search(u"date"_ustr);
    CPPUNIT_ASSERT_EQUAL(false, mpCallbackRecorder->m_bFound);
}

void LOKitSearchTest::testSearchInPDFNonExisting()
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

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
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

    lcl_search(u"NonExisting"_ustr);

    CPPUNIT_ASSERT_EQUAL(false, mpCallbackRecorder->m_bFound);
}

void LOKitSearchTest::testSearchInPDF()
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

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
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

    // Search
    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL("3763, 1331, 1432, 483"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(3763, 1331), Size(1433, 484)),
                         mpCallbackRecorder->m_aSelection[0]);

    // Search again - same result
    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL("3763, 1331, 1432, 483"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(3763, 1331), Size(1433, 484)),
                         mpCallbackRecorder->m_aSelection[0]);
}

void LOKitSearchTest::testSearchInPDFOnePDFObject()
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

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
    CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());

    // Search down
    lcl_search(u"ABC"_ustr, false, false);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    // Search up
    lcl_search(u"ABC"_ustr, false, true); // This caused a crash

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);
}

void LOKitSearchTest::testSearchInPDFInMultiplePages()
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

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
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());
    }

    // Search for "him"
    lcl_search(u"him"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL("9463, 3382, 1099, 499"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search(u"him"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL("5592, 5038, 1100, 499"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search(u"him"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(3, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL("9463, 1308, 1099, 499"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search(u"him"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(4, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL("5592, 2964, 1100, 499"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him" - back to start
    lcl_search(u"him"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(5, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL("9463, 3382, 1099, 499"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);
}

void LOKitSearchTest::testSearchInPDFInMultiplePagesBackwards()
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

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
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());
    }

    // Expected for backwards search is:
    // - Start with Page 1
    //   + search backwards through objects
    //   + inside objects search backwards through text
    // - Switch to Page 2
    //   + search backwards through objects
    //   + inside objects search backwards through text

    // Search for "him"
    lcl_search(u"him"_ustr, /*FindAll*/ false, /*Backwards*/ true);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL("5592, 5038, 1100, 499"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search(u"him"_ustr, /*FindAll*/ false, /*Backwards*/ true);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL("9463, 3382, 1099, 499"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search(u"him"_ustr, /*FindAll*/ false, /*Backwards*/ true);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(3, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL("5592, 2964, 1100, 499"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him"
    lcl_search(u"him"_ustr, /*FindAll*/ false, /*Backwards*/ true);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(4, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL("9463, 1308, 1099, 499"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search for "him" - back to start
    lcl_search(u"him"_ustr, /*FindAll*/ false, /*Backwards*/ true);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(5, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL(0, mpCallbackRecorder->m_aSearchResultPart[0]);
    CPPUNIT_ASSERT_EQUAL("5592, 5038, 1100, 499"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);
}

// Test searching in document with mixed objects.
// We have 2 objects: 1. Text Object, 2. Graphic Object with PDF
void LOKitSearchTest::testSearchIn2MixedObjects()
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

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

        CPPUNIT_ASSERT_EQUAL(SdrObjKind::Text, pObject->GetObjIdentifier());
    }

    // Check Object 2
    {
        SdrObject* pObject = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObject);

        CPPUNIT_ASSERT_EQUAL(SdrObjKind::Graphic, pObject->GetObjIdentifier());

        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);

        Graphic aGraphic = pGraphicObject->GetGraphic();
        auto const& pVectorGraphicData = aGraphic.getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);

        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());
    }

    // Let's try to search now

    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL("3545, 3174, 740, 402"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search next

    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL("8412, 6385, 519, 174"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);

    // Search next again - we should get the first object again

    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(3, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());

    CPPUNIT_ASSERT_EQUAL("3545, 3174, 740, 402"_ostr,
                         mpCallbackRecorder->m_aSearchResultSelection[0]);
}

// Test searching in document with mixed objects. We have 6 objects.
void LOKitSearchTest::testSearchIn6MixedObjects()
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

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
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::Text, pObject->GetObjIdentifier());
    }

    // Check Object 2
    {
        SdrObject* pObject = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::Graphic, pObject->GetObjIdentifier());
        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);
        auto const& pVectorGraphicData = pGraphicObject->GetGraphic().getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());
    }

    // Check Object 3
    {
        SdrObject* pObject = pPage->GetObj(2);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::CustomShape, pObject->GetObjIdentifier());
    }

    // Check Object 4
    {
        SdrObject* pObject = pPage->GetObj(3);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::CustomShape, pObject->GetObjIdentifier());
    }

    // Check Object 5
    {
        SdrObject* pObject = pPage->GetObj(4);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::Graphic, pObject->GetObjIdentifier());
        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);
        auto const& pVectorGraphicData = pGraphicObject->GetGraphic().getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Svg, pVectorGraphicData->getType());
    }

    // Check Object 6
    {
        SdrObject* pObject = pPage->GetObj(5);
        CPPUNIT_ASSERT(pObject);
        CPPUNIT_ASSERT_EQUAL(SdrObjKind::Graphic, pObject->GetObjIdentifier());
        SdrGrafObj* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        CPPUNIT_ASSERT(pGraphicObject);
        auto const& pVectorGraphicData = pGraphicObject->GetGraphic().getVectorGraphicData();
        CPPUNIT_ASSERT(pVectorGraphicData);
        CPPUNIT_ASSERT_EQUAL(VectorGraphicDataType::Pdf, pVectorGraphicData->getType());
    }

    // Search "ABC" which is in all objects (2 times in Object 3)

    // Object 1
    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(1, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(0), lclGetSelectedObject(pViewShell));

    // Object 2
    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(2, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(1), lclGetSelectedObject(pViewShell));

    // Object 3
    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(3, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(2), lclGetSelectedObject(pViewShell));

    // Object 3 again
    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(4, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(2), lclGetSelectedObject(pViewShell));

    // Object 4
    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(5, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(3), lclGetSelectedObject(pViewShell));

    // Object 5
    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(6, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(4), lclGetSelectedObject(pViewShell));

    // Object 6
    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(7, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(5), lclGetSelectedObject(pViewShell));

    // Loop to Object 1 again
    lcl_search(u"ABC"_ustr);

    CPPUNIT_ASSERT_EQUAL(true, mpCallbackRecorder->m_bFound);
    CPPUNIT_ASSERT_EQUAL(8, mpCallbackRecorder->m_nSearchResultCount);

    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultSelection.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), mpCallbackRecorder->m_aSearchResultPart.size());
    CPPUNIT_ASSERT_EQUAL(pPage->GetObj(0), lclGetSelectedObject(pViewShell));
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

    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Bbb bbb bbb bbb"_ustr, getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 4, 0));

    lcl_replace(u"bbb"_ustr, u"aaa"_ustr, false); // select

    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Bbb bbb bbb bbb"_ustr, getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 4, 0));

    lcl_replace(u"bbb"_ustr, u"aaa"_ustr, false); // replace

    CPPUNIT_ASSERT_EQUAL(u"aaa"_ustr, getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Bbb bbb bbb bbb"_ustr, getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 4, 0));
}

void LOKitSearchTest::testReplaceAll()
{
    SdXImpressDocument* pXImpressDocument = createDoc("ReplaceTest.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Bbb bbb bbb bbb"_ustr, getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 4, 0));

    lcl_replace(u"bbb"_ustr, u"ccc"_ustr, true);

    CPPUNIT_ASSERT_EQUAL(u"ccc"_ustr, getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"ccc ccc ccc ccc"_ustr, getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"ccc"_ustr, getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"ccc"_ustr, getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(u"ccc"_ustr, getShapeText(pXImpressDocument, 4, 0));

    lcl_replace(u"ccc"_ustr, u"bbb"_ustr, true);

    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb bbb bbb bbb"_ustr, getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 4, 0));
}

void LOKitSearchTest::testReplaceCombined()
{
    SdXImpressDocument* pXImpressDocument = createDoc("ReplaceTest.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    mpCallbackRecorder->registerCallbacksFor(pViewShell->GetViewShellBase());

    lcl_replace(u"bbb"_ustr, u"aaa"_ustr, false); // select
    lcl_replace(u"bbb"_ustr, u"aaa"_ustr, false); // replace

    CPPUNIT_ASSERT_EQUAL(u"aaa"_ustr, getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"Bbb bbb bbb bbb"_ustr, getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getShapeText(pXImpressDocument, 4, 0));

    lcl_replace(u"bbb"_ustr, u"ccc"_ustr, true);

    CPPUNIT_ASSERT_EQUAL(u"aaa"_ustr, getShapeText(pXImpressDocument, 0, 0));
    CPPUNIT_ASSERT_EQUAL(u"ccc ccc ccc ccc"_ustr, getShapeText(pXImpressDocument, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"ccc"_ustr, getShapeText(pXImpressDocument, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"ccc"_ustr, getShapeText(pXImpressDocument, 3, 0));
    CPPUNIT_ASSERT_EQUAL(u"ccc"_ustr, getShapeText(pXImpressDocument, 4, 0));
}

CPPUNIT_TEST_SUITE_REGISTRATION(LOKitSearchTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
