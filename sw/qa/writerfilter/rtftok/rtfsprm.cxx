/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/rtftok/rtfsprm.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/rtftok/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testLeftMarginDedup)
{
    loadFromFile(u"left-margin-dedup.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xText->createEnumeration();
    uno::Reference<beans::XPropertySet> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);
    sal_Int32 nLeftMargin = 0;
    xParagraph->getPropertyValue("ParaLeftMargin") >>= nLeftMargin;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), nLeftMargin);

    uno::Reference<beans::XPropertyState> xParagraphState(xParagraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE,
                         xParagraphState->getPropertyState("ParaLeftMargin"));

    xParagraph.set(xParagraphs->nextElement(), uno::UNO_QUERY);
    nLeftMargin = 0;
    xParagraph->getPropertyValue("ParaLeftMargin") >>= nLeftMargin;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), nLeftMargin);

    xParagraphState.set(xParagraph, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0 (DIRECT_VALUE)
    // - Actual  : 1 (DEFAULT_VALUE)
    // i.e. the left margin was not a direct formatting, which means left margin from the numbering
    // was used instead.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE,
                         xParagraphState->getPropertyState("ParaLeftMargin"));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
