/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vcl/scheduler.hxx>

#include <test/a11y/accessibletestbase.hxx>

using namespace css;
using namespace ::cpo;

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestImpressDefaultStructure)
{
    load(u"private:factory/simpress"_ustr);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(u"<SHAPE name=\"PageShape: Slide 1\"/>"
                         "<SHAPE name=\"PresentationTitle \">"
                         "<PARAGRAPH>"
                         "Click to add Title"
                         "</PARAGRAPH>"
                         "</SHAPE>"
                         "<SHAPE name=\"PresentationSubtitle \">"
                         "<PARAGRAPH>"
                         "Click to add Text"
                         "</PARAGRAPH>"
                         "</SHAPE>"_ustr,
                         collectText());
}

// a basic check of the content order for https://bugs.documentfoundation.org/show_bug.cgi?id=150064
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestImpressDefaultLayout)
{
    load(u"private:factory/simpress"_ustr);
    Scheduler::ProcessEventsToIdle();

    auto xDocumentContext = getDocumentAccessibleContext();

    /* The tree inside the document is a bit awkward, as the page is not the parent of the other
     * shapes, but well.  It looks like this:
     *  document_presentation
     *      shape (page)
     *      shape (title)
     *          paragraph
     *      shape (subtitle)
     *          paragraph
     */
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3), xDocumentContext->getAccessibleChildCount());
    CPPUNIT_ASSERT_EQUAL(
        u"PresentationTitle "_ustr,
        xDocumentContext->getAccessibleChild(1)->getAccessibleContext()->getAccessibleName());
    CPPUNIT_ASSERT_EQUAL(
        u"PresentationSubtitle "_ustr,
        xDocumentContext->getAccessibleChild(2)->getAccessibleContext()->getAccessibleName());
}

// test a11y tree order is stable (https://bugs.documentfoundation.org/show_bug.cgi?id=150064)
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, tdf150064)
{
    loadFromSrc(u"/sd/qa/unit/a11y/data/tdf150064.fodp"_ustr);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxDocument,
                                                                   uno::UNO_QUERY_THROW);
    uno::Reference<frame::XModel> xModel(mxDocument, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawView> xDrawView(xModel->getCurrentController(),
                                                 uno::UNO_QUERY_THROW);

    auto xDrawPages = xDrawPagesSupplier->getDrawPages();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDrawPages->getCount());

    // Loop a couple times to make sure output is stable when loading and when switching pages
    for (int i = 0; i < 2; i++)
    {
        uno::Reference<drawing::XDrawPage> xDrawPage;

        CPPUNIT_ASSERT(xDrawPages->getByIndex(0) >>= xDrawPage);
        xDrawView->setCurrentPage(xDrawPage);
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(u"<SHAPE name=\"PageShape: Slide 1\"/>"
                             "<SHAPE name=\"PresentationTitle \">"
                             "<PARAGRAPH>P1 title</PARAGRAPH>"
                             "</SHAPE>"
                             "<SHAPE name=\"PresentationSubtitle \">"
                             "<PARAGRAPH>Some text</PARAGRAPH>"
                             "</SHAPE>"
                             "<TABLE name=\"TableShape \">"
                             "<TABLE_CELL name=\"A1\">"
                             "<PARAGRAPH>1</PARAGRAPH>"
                             "</TABLE_CELL>"
                             "<TABLE_CELL name=\"B1\">"
                             "<PARAGRAPH>2</PARAGRAPH>"
                             "</TABLE_CELL>"
                             "<TABLE_CELL name=\"C1\">"
                             "<PARAGRAPH>3</PARAGRAPH>"
                             "</TABLE_CELL>"
                             "<TABLE_CELL name=\"D1\">"
                             "<PARAGRAPH>4</PARAGRAPH>"
                             "</TABLE_CELL>"
                             "<TABLE_CELL name=\"E1\">"
                             "<PARAGRAPH>5</PARAGRAPH>"
                             "</TABLE_CELL>"
                             "<TABLE_CELL name=\"A2\">"
                             "<PARAGRAPH>6</PARAGRAPH>"
                             "</TABLE_CELL>"
                             "<TABLE_CELL name=\"B2\">"
                             "<PARAGRAPH>7</PARAGRAPH>"
                             "</TABLE_CELL>"
                             "<TABLE_CELL name=\"C2\">"
                             "<PARAGRAPH>8</PARAGRAPH>"
                             "</TABLE_CELL>"
                             "<TABLE_CELL name=\"D2\">"
                             "<PARAGRAPH>9</PARAGRAPH>"
                             "</TABLE_CELL>"
                             "<TABLE_CELL name=\"E2\">"
                             "<PARAGRAPH>10</PARAGRAPH>"
                             "</TABLE_CELL>"
                             "</TABLE>"
                             "<SHAPE name=\"MyRectangle\" description=\"Some Rectangle\">"
                             "<PARAGRAPH>Text inside Shape</PARAGRAPH>"
                             "</SHAPE>"_ustr,
                             collectText());

        CPPUNIT_ASSERT(xDrawPages->getByIndex(1) >>= xDrawPage);
        xDrawView->setCurrentPage(xDrawPage);
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(u"<SHAPE name=\"PageShape: Slide 2\"/>"
                             "<SHAPE name=\"PresentationTitle \">"
                             "<PARAGRAPH>P2 title</PARAGRAPH>"
                             "</SHAPE>"
                             "<SHAPE name=\"PresentationOutliner \">"
                             "<PARAGRAPH>1</PARAGRAPH>"
                             "</SHAPE>"
                             "<SHAPE name=\"PresentationOutliner \">"
                             "<PARAGRAPH>2</PARAGRAPH>"
                             "</SHAPE>"
                             "<SHAPE name=\"PresentationOutliner \">"
                             "<PARAGRAPH>3</PARAGRAPH>"
                             "</SHAPE>"_ustr,
                             collectText());
    }
}

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, testBulletedParagraphRuns)
{
    // Given a text box with a numbered "plain BOLD end" and a bulleted "abc", "c" bold:
    loadFromSrc(u"/sd/qa/unit/a11y/data/bulleted-runs.fodp"_ustr);
    Scheduler::ProcessEventsToIdle();
    uno::Reference<accessibility::XAccessibleContext> xBox;
    for (const auto& xShape : getAllChildren(getDocumentAccessibleContext()))
    {
        if (xShape->getAccessibleChildCount() == 2)
            xBox = xShape;
    }
    CPPUNIT_ASSERT(xBox.is());

    // When the runs of each paragraph and their attributes are asked for:
    const uno::Sequence<OUString> aNames{ u"CharWeight"_ustr };
    // the characters whose run is bold, and the run of the last one
    auto boldCharacters = [&](sal_Int64 nParagraph, accessibility::TextSegment& rLastRun) {
        const auto xParagraph = xBox->getAccessibleChild(nParagraph)->getAccessibleContext();
        const auto xText = xParagraph.queryThrow<accessibility::XAccessibleText>();
        const auto xAttributes = xParagraph.queryThrow<accessibility::XAccessibleTextAttributes>();
        OUStringBuffer aBold;
        for (sal_Int32 i = 0; i < xText->getCharacterCount(); ++i)
        {
            float fWeight = awt::FontWeight::NORMAL;
            for (const beans::PropertyValue& rValue : xAttributes->getRunAttributes(i, aNames))
                rValue.Value >>= fWeight;
            if (fWeight == awt::FontWeight::BOLD)
                aBold.append(xText->getCharacter(i));
            rLastRun = xText->getTextAtIndex(i, accessibility::AccessibleTextType::ATTRIBUTE_RUN);
        }
        return aBold.makeStringAndClear();
    };

    // Then they are those of the characters at the given positions, not after the bullet text:
    accessibility::TextSegment aRun;
    // Without the fix, it was "LD e": each run was 2 characters early, the length of "1."
    CPPUNIT_ASSERT_EQUAL(u"BOLD"_ustr, boldCharacters(0, aRun));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aRun.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aRun.SegmentEnd);
    // Without the fix, it was empty
    CPPUNIT_ASSERT_EQUAL(u"c"_ustr, boldCharacters(1, aRun));
    // Without the fix, the run of "c" was the one of "ab"
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRun.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aRun.SegmentEnd);
    // Without the fix, an IndexOutOfBoundsException
    const auto xText = xBox->getAccessibleChild(0)
                           ->getAccessibleContext()
                           .queryThrow<accessibility::XAccessibleText>();
    float fWeight = 0;
    for (const beans::PropertyValue& rValue : xText->getCharacterAttributes(13, aNames))
        rValue.Value >>= fWeight;
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, fWeight);

    // Without the fix, it was "plain " at [2,8)
    aRun = xText->getTextBehindIndex(0, accessibility::AccessibleTextType::ATTRIBUTE_RUN);
    CPPUNIT_ASSERT_EQUAL(u"BOLD"_ustr, aRun.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aRun.SegmentStart);
    // Without the fix, it was empty at [0,2), the range of "1."
    aRun = xText->getTextBeforeIndex(7, accessibility::AccessibleTextType::ATTRIBUTE_RUN);
    CPPUNIT_ASSERT_EQUAL(u"plain "_ustr, aRun.SegmentText);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRun.SegmentStart);
    // Without the fix, it was " end", not the empty run after the end
    aRun = xText->getTextAtIndex(14, accessibility::AccessibleTextType::ATTRIBUTE_RUN);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aRun.SegmentStart);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aRun.SegmentEnd);
}

CPPUNIT_PLUGIN_IMPLEMENT();
