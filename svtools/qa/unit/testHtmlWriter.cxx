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
#include <cppunit/plugin/TestPlugIn.h>

#include <unotest/bootstrapfixturebase.hxx>
#include <tools/stream.hxx>
#include <svtools/HtmlWriter.hxx>

namespace
{
OString extractFromStream(SvMemoryStream& rStream)
{
    rStream.WriteChar('\0');
    rStream.FlushBuffer();
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    return static_cast<const char*>(rStream.GetData());
}
}

class Test : public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(Test, testSingleElement)
{
    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.start("abc"_ostr);
        aHtml.end();

        OString aString = extractFromStream(aStream);
        CPPUNIT_ASSERT_EQUAL("<abc/>"_ostr, aString);
    }

    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.single("abc"_ostr);

        OString aString = extractFromStream(aStream);

        CPPUNIT_ASSERT_EQUAL("<abc/>"_ostr, aString);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testSingleElementWithAttributes)
{
    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.start("abc"_ostr);
        aHtml.attribute("x", "y");
        aHtml.end();

        OString aString = extractFromStream(aStream);

        CPPUNIT_ASSERT_EQUAL("<abc x=\"y\"/>"_ostr, aString);
    }

    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.start("abc"_ostr);
        aHtml.attribute("x", "y");
        aHtml.attribute("q", "w");
        aHtml.end();

        OString aString = extractFromStream(aStream);

        CPPUNIT_ASSERT_EQUAL("<abc x=\"y\" q=\"w\"/>"_ostr, aString);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testSingleElementWithContent)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc"_ostr);
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL("<abc/>"_ostr, aString);
}

CPPUNIT_TEST_FIXTURE(Test, testSingleElementWithContentAndAttributes)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc"_ostr);
    aHtml.attribute("x", "y");
    aHtml.attribute("q", "w");
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL("<abc x=\"y\" q=\"w\"/>"_ostr, aString);
}

CPPUNIT_TEST_FIXTURE(Test, testNested)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc"_ostr);
    aHtml.start("xyz"_ostr);
    aHtml.end();
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL("<abc><xyz/></abc>"_ostr, aString);
}

CPPUNIT_TEST_FIXTURE(Test, testNamespace)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream, "reqif-xhtml:");
    aHtml.prettyPrint(false);
    aHtml.single("br"_ostr);

    OString aString = extractFromStream(aStream);

    // This was <br/>, namespace request was ignored.
    CPPUNIT_ASSERT_EQUAL("<reqif-xhtml:br/>"_ostr, aString);
}

CPPUNIT_TEST_FIXTURE(Test, testAttributeValues)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc"_ostr);
    aHtml.attribute("one", "one");
    aHtml.attribute("two", u"two"_ustr);
    aHtml.attribute("three", sal_Int32(12));
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL("<abc one=\"one\" two=\"two\" three=\"12\"/>"_ostr, aString);
}

CPPUNIT_TEST_FIXTURE(Test, testCharacters)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc"_ostr);
    aHtml.characters("hello");
    aHtml.end();
    aHtml.characters(" "); // Should not try to close a not opened tag
    aHtml.start("abc"_ostr);
    aHtml.characters("world"); // Should close opening tag
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL("<abc>hello</abc> <abc>world</abc>"_ostr, aString);
}

CPPUNIT_TEST_FIXTURE(Test, testExactElementEnd)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("start"_ostr);
    aHtml.start("a"_ostr);
    CPPUNIT_ASSERT(aHtml.end("a"_ostr));
    aHtml.start("b"_ostr);
    CPPUNIT_ASSERT(!aHtml.end("c"_ostr));
    CPPUNIT_ASSERT(aHtml.end("start"_ostr));

    OString aString = extractFromStream(aStream);
    CPPUNIT_ASSERT_EQUAL("<start><a/><b/></start>"_ostr, aString);
}

CPPUNIT_TEST_FIXTURE(Test, testAttributeValueEncode)
{
    // Given a HTML writer:
    SvMemoryStream aStream;
    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);

    // When writing an attribute with a value that needs encoding:
    aHtml.start("element"_ostr);
    aHtml.attribute("attribute", "a&b");
    aHtml.end();

    // Then make sure that the encoding is performed:
    OString aString = extractFromStream(aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: <element attribute="a&amp;b"/>
    // - Actual  : <element attribute="a&b"/>
    // i.e. attribute value was not encoded in HTML, but it was in e.g. XML.
    CPPUNIT_ASSERT_EQUAL("<element attribute=\"a&amp;b\"/>"_ostr, aString);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
