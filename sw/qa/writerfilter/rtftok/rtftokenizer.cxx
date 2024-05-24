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

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/rtftok/rtftokenizer.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/rtftok/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testInvalidHex)
{
    // Given a document with a markup like "\'3?":
    loadFromFile(u"invalid-hex.rtf");

    // Then make sure the result matches Word, rather than just refusing to import the document:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("xřx"), xTextDocument->getText()->getString());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
