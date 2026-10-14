/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <comphelper/propertyvalue.hxx>
#include <rtl/ustring.hxx>
#include <test/a11y/AccessibilityTools.hxx>
#include <test/a11y/accessibletestbase.hxx>
#include <unotest/macros_test.hxx>
#include <vcl/scheduler.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>

#include <vector>

using namespace css;

/** Base class for the tests around content that is laid out (and exposed) more
 * than once.
 *
 * A paragraph split over several pages, a repeated page header/footer and a
 * repeated table heading all refer to a single model node, but can exist in
 * multiple frames. Setting the caret on one of those objects must
 * move the cursor to that object's frame rather than to the first layout of the
 * shared model node, and only that object must report the caret.
 */
class SwA11yRepeatedContent : public test::AccessibleTestBase
{
protected:
    // Show two pages side by side, so that a page's content (and in
    // particular a repeated header/footer, or both fragments of a split
    // paragraph/table) is on screen instead of below the visible area.
    // TODO: this part can be removed once offscreen objects will be exposed (tdf#35652).
    void zoomOutToFitAllPages()
    {
        unotest::MacrosTest::dispatchCommand(
            mxDocument, ".uno:ViewLayout",
            { comphelper::makePropertyValue("ViewLayout.Columns", sal_Int16(2)),
              comphelper::makePropertyValue("ViewLayout.BookMode", false) });

        uno::Reference<frame::XModel> xModel(mxDocument, uno::UNO_QUERY_THROW);
        uno::Reference<view::XViewSettingsSupplier> xSupplier(xModel->getCurrentController(),
                                                              uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(xSupplier->getViewSettings(),
                                                      uno::UNO_SET_THROW);
        xSettings->setPropertyValue(u"ZoomType"_ustr,
                                    uno::Any(sal_Int16(view::DocumentZoomType::BY_VALUE)));
        // 20% is Writer's minimum zoom.
        xSettings->setPropertyValue(u"ZoomValue"_ustr, uno::Any(sal_Int16(20)));
        Scheduler::ProcessEventsToIdle();
    }

    // Get the accessibles that are direct children of the document filtered by their role.
    std::vector<uno::Reference<accessibility::XAccessibleContext>>
    getDocumentAccessiblesFromRole(sal_Int16 nRole)
    {
        std::vector<uno::Reference<accessibility::XAccessibleContext>> aResult;
        for (const auto& xChildContext : getAllChildren(getDocumentAccessibleContext()))
        {
            if (xChildContext.is() && xChildContext->getAccessibleRole() == nRole)
                aResult.push_back(xChildContext);
        }
        return aResult;
    }

    // Get the paragraph objects whose text is @p rText.
    std::vector<uno::Reference<accessibility::XAccessibleContext>>
    getParagraphsWithText(const std::u16string_view rText)
    {
        auto xCtx = getDocumentAccessibleContext();
        return AccessibilityTools::getAccessibleObjectsForPredicate(
            xCtx, [rText](const uno::Reference<accessibility::XAccessibleContext>& xObjCtx) {
                if (xObjCtx->getAccessibleRole() != accessibility::AccessibleRole::PARAGRAPH)
                    return false;
                uno::Reference<accessibility::XAccessibleText> xText(xObjCtx, uno::UNO_QUERY);
                return xText.is() && std::u16string_view(xText->getText()) == rText;
            });
    }

    // Check that setting the caret on @p rText's layout in the later frame
    // moves the cursor there, and that the earlier frame no longer reports it.
    void assertCaretGoesToLaterInstance(const OUString& rText)
    {
        // wait for both layouts of the paragraph to be exposed
        std::vector<css::uno::Reference<css::accessibility::XAccessibleContext>> aInstances;
        const bool bTwoInstances = AccessibilityTools::Await([this, &rText, &aInstances] {
            aInstances = getParagraphsWithText(rText);
            return aInstances.size() >= 2;
        });
        CPPUNIT_ASSERT_MESSAGE("expected the same text to be laid out twice; "
                               "are all pages visible?",
                               bTwoInstances);
        CPPUNIT_ASSERT_EQUAL(size_t(2), aInstances.size());

        uno::Reference<accessibility::XAccessibleText> xFirst(aInstances.front(),
                                                              uno::UNO_QUERY_THROW);
        uno::Reference<accessibility::XAccessibleText> xSecond(aInstances.back(),
                                                               uno::UNO_QUERY_THROW);

        // put the caret in the first instance
        CPPUNIT_ASSERT(xFirst->setCaretPosition(0));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xFirst->getCaretPosition());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), xSecond->getCaretPosition());

        // put the caret in the second instance: the cursor must move there and
        // the first instance must stop claiming the caret
        CPPUNIT_ASSERT(xSecond->setCaretPosition(0));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), xFirst->getCaretPosition());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSecond->getCaretPosition());

        // put the caret in the first instance again
        CPPUNIT_ASSERT(xFirst->setCaretPosition(0));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xFirst->getCaretPosition());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), xSecond->getCaretPosition());
    }
};

CPPUNIT_TEST_FIXTURE(SwA11yRepeatedContent, TestRepeatedHeaderCaret)
{
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/RepeatedHeader.fodt"_ustr);
    zoomOutToFitAllPages();
    assertCaretGoesToLaterInstance(u"This is a header"_ustr);
}

// grabFocus() on the header object itself must also put the cursor in that header's
// frame, not in the first header of the document.
CPPUNIT_TEST_FIXTURE(SwA11yRepeatedContent, TestHeaderContainerGrabFocus)
{
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/RepeatedHeader.fodt"_ustr);
    zoomOutToFitAllPages();

    // wait for both layouts of the header to be exposed
    std::vector<uno::Reference<accessibility::XAccessibleContext>> aHeaders;
    const bool bTwoHeaders = AccessibilityTools::Await([this, &aHeaders] {
        aHeaders = getDocumentAccessiblesFromRole(accessibility::AccessibleRole::HEADER);
        return aHeaders.size() >= 2;
    });

    CPPUNIT_ASSERT_MESSAGE("expected at least two headers to be exposed", bTwoHeaders);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aHeaders.size());
    auto xHeader1 = aHeaders[0];
    auto xHeader2 = aHeaders[1];
    CPPUNIT_ASSERT(xHeader1.is());
    CPPUNIT_ASSERT(xHeader2.is());

    // the paragraph inside each header is the object that reports the caret
    auto xParagraph1 = xHeader1->getAccessibleChild(0);
    auto xParagraph2 = xHeader2->getAccessibleChild(0);
    CPPUNIT_ASSERT(xParagraph1.is());
    CPPUNIT_ASSERT(xParagraph2.is());

    uno::Reference<accessibility::XAccessibleText> xPara1(xParagraph1->getAccessibleContext(),
                                                          uno::UNO_QUERY_THROW);
    uno::Reference<accessibility::XAccessibleText> xPara2(xParagraph2->getAccessibleContext(),
                                                          uno::UNO_QUERY_THROW);

    // grabbing focus on the HEADER object itself, not on its paragraph child,
    // should move the caret to the paragraph inside it. This is to ensure
    // the grabFocus() implementation for headers moves the caret correctly.
    uno::Reference<accessibility::XAccessibleComponent> xComponent(xHeader2, uno::UNO_QUERY_THROW);
    xComponent->grabFocus();
    Scheduler::ProcessEventsToIdle();

    // the cursor must be in the header paragraph on page 2
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xPara2->getCaretPosition());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), xPara1->getCaretPosition());
}

CPPUNIT_TEST_FIXTURE(SwA11yRepeatedContent, TestRepeatedFooterCaret)
{
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/RepeatedFooter.fodt"_ustr);
    zoomOutToFitAllPages();
    assertCaretGoesToLaterInstance(u"This is a footer"_ustr);
}

// A table whose heading row is repeated on the second page: the heading row's
// cell text exists once in the model but twice in the tree.
CPPUNIT_TEST_FIXTURE(SwA11yRepeatedContent, TestRepeatedTableHeadingCaret)
{
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/SplitTableOverTwoPages.fodt"_ustr);
    zoomOutToFitAllPages();
    assertCaretGoesToLaterInstance(u"Header 1"_ustr);
}

// A paragraph split over two pages is exposed as two paragraphs. Setting the
// caret in the second one must put it there, rather than stay in the
// first part, and only the part the cursor is in must report the caret.
CPPUNIT_TEST_FIXTURE(SwA11yRepeatedContent, TestSplitParagraphCaret)
{
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/SplitParagraphOverTwoPages.fodt"_ustr);
    zoomOutToFitAllPages();

    // wait for both layouts of the paragraph to be exposed
    std::vector<uno::Reference<accessibility::XAccessibleContext>> aInstances;
    const bool bTwoInstances = AccessibilityTools::Await([this, &aInstances] {
        aInstances = getDocumentAccessiblesFromRole(accessibility::AccessibleRole::PARAGRAPH);
        return aInstances.size() >= 2;
    });
    CPPUNIT_ASSERT_MESSAGE("expected the paragraph to be split over two pages", bTwoInstances);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aInstances.size());

    uno::Reference<accessibility::XAccessibleText> xFirst(aInstances[0], uno::UNO_QUERY_THROW);
    uno::Reference<accessibility::XAccessibleText> xSecond(aInstances[1], uno::UNO_QUERY_THROW);

    // put the caret in the first instance
    CPPUNIT_ASSERT(xFirst->setCaretPosition(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xFirst->getCaretPosition());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), xSecond->getCaretPosition());

    // put the caret in the second instance: the cursor must move there and
    // the first instance must stop claiming the caret
    CPPUNIT_ASSERT(xSecond->setCaretPosition(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), xFirst->getCaretPosition());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSecond->getCaretPosition());

    // put the caret in the first instance again
    CPPUNIT_ASSERT(xFirst->setCaretPosition(0));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xFirst->getCaretPosition());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), xSecond->getCaretPosition());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
