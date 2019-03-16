/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/extensions/HelperMacros.h>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <test/unoapi_property_testers.hxx>
#include <test/text/xdocumentindex.hxx>

namespace apitest
{
XDocumentIndex::~XDocumentIndex() {}
/**
 * Gets the document from relation and insert a new index mark.
 * Then it stores the text content of document index before
 * update and after.<p>
 *
 * Has <b> OK </b> status if index content is changed and
 * new index contains index mark inserted. <p>
 */
void XDocumentIndex::testUpdate()
{
    css::uno::Reference<css::text::XDocumentIndex> xDocumentIndex(init(),
                                                                  css::uno::UNO_QUERY_THROW);

    bool bOK = true;
    try
    {
        auto xText = getTextDocument()->getText();
        auto xTextRange = xText->getEnd();
        xTextRange->setString("IndexMark");
        css::uno::Reference<css::lang::XMultiServiceFactory> xFactory(getTextDocument(),
                                                                      css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::text::XTextContent> xTextContentMark(
            xFactory->createInstance("com.sun.star.text.DocumentIndexMark"),
            css::uno::UNO_QUERY_THROW);
        xText->insertTextContent(xTextRange, xTextContentMark, true);
    }
    catch (css::uno::Exception /*exception*/)
    {
        bOK = false;
    }

    CPPUNIT_ASSERT_MESSAGE("Couldn't create the document index mark", bOK);

    OUString sContentBefore = xDocumentIndex->getAnchor()->getString();
    xDocumentIndex->update();
    OUString sContentAfter = xDocumentIndex->getAnchor()->getString();

    CPPUNIT_ASSERT_MESSAGE("Before and after shouldn't be equal", sContentBefore != sContentAfter);
    CPPUNIT_ASSERT_MESSAGE("Content after should contain string 'IndexMark'",
                           sContentAfter.indexOf("IndexMark") >= 0);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
