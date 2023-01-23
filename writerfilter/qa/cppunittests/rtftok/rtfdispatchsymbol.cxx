/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/rtftok/rtfdispatchsymbol.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/writerfilter/qa/cppunittests/rtftok/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testPage)
{
    // Given a file with a \page and 2 \par tokens:
    loadFromURL(u"page.rtf");

    // Then make sure we get exactly two paragraphs:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xText->createEnumeration();
    xParagraphs->nextElement();
    xParagraphs->nextElement();
    // Without the accompanying fix in place, this test would have failed, the document had 3
    // paragraphs, not 2.
    CPPUNIT_ASSERT(!xParagraphs->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(Test, testCenterAfterPage)
{
    // Given a file with a \page, followed by a \qc:
    // When loading that file:
    loadFromURL(u"center-after-page.rtf");

    // Then make sure that the last paragraph is centered:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xText->createEnumeration();
    xParagraphs->nextElement();
    xParagraphs->nextElement();
    uno::Reference<beans::XPropertySet> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);
    sal_Int16 eActual{};
    CPPUNIT_ASSERT(xParagraph->getPropertyValue("ParaAdjust") >>= eActual);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3 (CENTER)
    // - Actual  : 0 (LEFT)
    // i.e. the paragraph alignment on the second page was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(style::ParagraphAdjust_CENTER), eActual);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
