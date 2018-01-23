/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <svtools/parhtml.hxx>
#include <tools/ref.hxx>
#include <tools/stream.hxx>

namespace
{
/// Subclass of HTMLParser that can sense the import result.
class TestHTMLParser : public HTMLParser
{
public:
    TestHTMLParser(SvStream& rStream);
    virtual void NextToken(HtmlTokenId nToken) override;

    OUString m_aDocument;
};

TestHTMLParser::TestHTMLParser(SvStream& rStream)
    : HTMLParser(rStream)
{
}

void TestHTMLParser::NextToken(HtmlTokenId nToken)
{
    if (nToken == HtmlTokenId::TEXTTOKEN)
        m_aDocument += aToken;
}

/// Tests HTMLParser.
class Test : public CppUnit::TestFixture
{
public:
    void testTdf114428();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testTdf114428);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testTdf114428()
{
    SvMemoryStream aStream;
    OString aDocument("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<html>hello</html>");
    aStream.WriteBytes(aDocument.getStr(), aDocument.getLength());
    aStream.Seek(0);

    tools::SvRef<TestHTMLParser> xParser = new TestHTMLParser(aStream);
    xParser->CallParser();

    // This was '<?xml version="1.0" encoding="utf-8"?> hello', XML declaration
    // was not ignored.
    CPPUNIT_ASSERT_EQUAL(OUString("hello"), xParser->m_aDocument.trim());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
