/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <unotest/bootstrapfixturebase.hxx>
#include <tools/stream.hxx>
#include <svtools/HtmlWriter.hxx>

namespace
{

OString extractFromStream(SvMemoryStream& rStream)
{
    rStream.WriteChar('\0');
    rStream.Flush();
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    return OString(static_cast<const sal_Char*>(rStream.GetData()));
}

}

class Test: public CppUnit::TestFixture
{
};

CPPUNIT_TEST_FIXTURE(Test, testSingleElement)
{
    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.start("abc");
        aHtml.end();

        OString aString = extractFromStream(aStream);
        CPPUNIT_ASSERT_EQUAL(OString("<abc/>"), aString);
    }

    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.single("abc");

        OString aString = extractFromStream(aStream);

        CPPUNIT_ASSERT_EQUAL(OString("<abc/>"), aString);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testSingleElementWithAttributes)
{
    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.start("abc");
        aHtml.attribute("x", "y");
        aHtml.end();

        OString aString = extractFromStream(aStream);

        CPPUNIT_ASSERT_EQUAL(OString("<abc x=\"y\"/>"), aString);
    }

    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.start("abc");
        aHtml.attribute("x", "y");
        aHtml.attribute("q", "w");
        aHtml.end();

        OString aString = extractFromStream(aStream);

        CPPUNIT_ASSERT_EQUAL(OString("<abc x=\"y\" q=\"w\"/>"), aString);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testSingleElementWithContent)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc");
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL(OString("<abc/>"), aString);
}

CPPUNIT_TEST_FIXTURE(Test, testSingleElementWithContentAndAttributes)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc");
    aHtml.attribute("x", "y");
    aHtml.attribute("q", "w");
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL(OString("<abc x=\"y\" q=\"w\"/>"), aString);
}

CPPUNIT_TEST_FIXTURE(Test, testNested)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc");
    aHtml.start("xyz");
    aHtml.end();
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL(OString("<abc><xyz/></abc>"), aString);
}

CPPUNIT_TEST_FIXTURE(Test, testNamespace)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream, "reqif-xhtml");
    aHtml.prettyPrint(false);
    aHtml.single("br");

    OString aString = extractFromStream(aStream);

    // This was <br/>, namespace request was ignored.
    CPPUNIT_ASSERT_EQUAL(OString("<reqif-xhtml:br/>"), aString);
}

CPPUNIT_TEST_FIXTURE(Test, testAttributeValues)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc");
    aHtml.attribute("one", OString("one"));
    aHtml.attribute("two", OUString("two"));
    aHtml.attribute("three", sal_Int32(12));
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL(OString("<abc one=\"one\" two=\"two\" three=\"12\"/>"), aString);
}

CPPUNIT_TEST_FIXTURE(Test, testCharacters)
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc");
    aHtml.characters("hello");
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL(OString("<abc>hello</abc>"), aString);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
