/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vcl/scheduler.hxx>

#include <test/a11y/accessibletestbase.hxx>

using namespace css;

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestImpressDefaultStructure)
{
    load(u"private:factory/simpress"_ustr);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(u"<SHAPE name=\"PageShape: Slide 1\" description=\" \"/>"
                         "<SHAPE name=\"PresentationTitle \" description=\" \">"
                         "<PARAGRAPH>"
                         "Click to add Title"
                         "</PARAGRAPH>"
                         "</SHAPE>"
                         "<SHAPE name=\"PresentationSubtitle \" description=\" \">"
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

        CPPUNIT_ASSERT_EQUAL(u"<SHAPE name=\"PageShape: Slide 1\" description=\" \"/>"
                             "<SHAPE name=\"PresentationTitle \" description=\" \">"
                             "<PARAGRAPH>P1 title</PARAGRAPH>"
                             "</SHAPE>"
                             "<SHAPE name=\"PresentationSubtitle \" description=\" \">"
                             "<PARAGRAPH>Some text</PARAGRAPH>"
                             "</SHAPE>"
                             "<TABLE name=\"TableShape \" description=\" \">"
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
                             "</TABLE>"_ustr,
                             collectText());

        CPPUNIT_ASSERT(xDrawPages->getByIndex(1) >>= xDrawPage);
        xDrawView->setCurrentPage(xDrawPage);
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(u"<SHAPE name=\"PageShape: Slide 2\" description=\" \"/>"
                             "<SHAPE name=\"PresentationTitle \" description=\" \">"
                             "<PARAGRAPH>P2 title</PARAGRAPH>"
                             "</SHAPE>"
                             "<SHAPE name=\"PresentationOutliner \" description=\" \">"
                             "<PARAGRAPH>1</PARAGRAPH>"
                             "</SHAPE>"
                             "<SHAPE name=\"PresentationOutliner \" description=\" \">"
                             "<PARAGRAPH>2</PARAGRAPH>"
                             "</SHAPE>"
                             "<SHAPE name=\"PresentationOutliner \" description=\" \">"
                             "<PARAGRAPH>3</PARAGRAPH>"
                             "</SHAPE>"_ustr,
                             collectText());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();
