/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../sdmodeltestbase.hxx"
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
#include <sfx2/viewfrm.hxx>
#include <svl/srchitem.hxx>
#include <svl/stritem.hxx>
#include <ViewShellBase.hxx>
#include <ViewShell.hxx>
#include <unomodel.hxx>

#include <com/sun/star/frame/Desktop.hpp>

using namespace css;

class LOKitSearchTest : public SdModelTestBase, public XmlTestTools
{
private:
    static constexpr char DATA_DIRECTORY[] = "/sd/qa/unit/tiledrendering/data/";

public:
    LOKitSearchTest() {}

    virtual void setUp() override;
    virtual void tearDown() override;

    void testSearch();
    void testSearchAll();
    void testSearchAllSelections();
    void testSearchAllNotifications();
    void testSearchAllFollowedBySearch();
    void testDontSearchInMasterPages();

    CPPUNIT_TEST_SUITE(LOKitSearchTest);
    CPPUNIT_TEST(testSearch);
    CPPUNIT_TEST(testSearchAll);
    CPPUNIT_TEST(testSearchAllSelections);
    CPPUNIT_TEST(testSearchAllNotifications);
    CPPUNIT_TEST(testSearchAllFollowedBySearch);
    CPPUNIT_TEST(testDontSearchInMasterPages);
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
                                      + OUString::createFromAscii(pName),
                                  "com.sun.star.presentation.PresentationDocument");
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering(rArguments);
    return pImpressDocument;
}

namespace
{
void lcl_search(const OUString& rKey, bool bFindAll = false)
{
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(rKey) },
        { "SearchItem.Backward", uno::makeAny(false) },
        { "SearchItem.Command", uno::makeAny(static_cast<sal_uInt16>(
                                    bFindAll ? SvxSearchCmd::FIND_ALL : SvxSearchCmd::FIND)) },
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
}
}

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
    // This was 1: only the first match was highlighted.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), mpCallbackRecorder->m_aSelection.size());
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

CPPUNIT_TEST_SUITE_REGISTRATION(LOKitSearchTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
