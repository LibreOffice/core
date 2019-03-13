/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_QA_API_XTEXTCONTENTTEST_HXX
#define INCLUDED_SW_QA_API_XTEXTCONTENTTEST_HXX

#include "ApiTestBase.hxx"

#include <cppunit/TestAssert.h>

#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace apitest
{
class XTextContentTest : public ApiTestBase
{
public:
    virtual css::uno::Reference<css::text::XTextDocument> getTextDocument() = 0;
    /**
     * Tries to attach the text content to the test range
     * gotten with getAnchor(). If relations are found
     * then they are used for testing. <p>
     *
     * The test is OK if the method works without error.
     */
    void testAttach()
    {
        css::uno::Reference<css::text::XTextContent> xTextContent(init(),
                                                                  css::uno::UNO_QUERY_THROW);
        auto xTextCursor = getTextDocument()->getText()->createTextCursor();
        xTextCursor->gotoEnd(false);
        css::uno::Reference<css::lang::XMultiServiceFactory> xMSF(getTextDocument(),
                                                                  css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::text::XDocumentIndex> xDocumentIndex(
            xMSF->createInstance("com.sun.star.text.DocumentIndex"), css::uno::UNO_QUERY_THROW);
        xDocumentIndex->attach(xTextCursor);
    }

    void testGetAnchor()
    {
        css::uno::Reference<css::text::XTextContent> xTextContent(init(),
                                                                  css::uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(OUString(""), xTextContent->getAnchor()->getString());
    }
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
