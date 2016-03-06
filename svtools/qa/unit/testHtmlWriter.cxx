/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "cppunit/TestCase.h"
#include "cppunit/TestFixture.h"
#include "cppunit/TestSuite.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include <tools/stream.hxx>
#include <svtools/HtmlWriter.hxx>

namespace
{

OString extractFromStream(SvMemoryStream& rStream)
{
    rStream.WriteChar('\0');
    rStream.Flush();
    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    return OString(static_cast<const sal_Char*>(rStream.GetBuffer()));
}

}

class Test: public CppUnit::TestFixture
{

public:
    void testSingleElement();
    void testSingleElementWithAttributes();
    void testSingleElementWithContent();
    void testSingleElementWithContentAndAttributes();
    void testNested();
    void testAttributeValues();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSingleElement);
    CPPUNIT_TEST(testSingleElementWithAttributes);
    CPPUNIT_TEST(testSingleElementWithContent);
    CPPUNIT_TEST(testSingleElementWithContentAndAttributes);
    CPPUNIT_TEST(testNested);
    CPPUNIT_TEST(testAttributeValues);

    CPPUNIT_TEST_SUITE_END();
};

void Test::testSingleElement()
{
    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.start("abc");
        aHtml.end();

        OString aString = extractFromStream(aStream);
        CPPUNIT_ASSERT_EQUAL(aString, OString("<abc/>"));
    }

    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.single("abc");

        OString aString = extractFromStream(aStream);

        CPPUNIT_ASSERT_EQUAL(aString, OString("<abc/>"));
    }
}

void Test::testSingleElementWithAttributes()
{
    {
        SvMemoryStream aStream;

        HtmlWriter aHtml(aStream);
        aHtml.prettyPrint(false);
        aHtml.start("abc");
        aHtml.attribute("x", "y");
        aHtml.end();

        OString aString = extractFromStream(aStream);

        CPPUNIT_ASSERT_EQUAL(aString, OString("<abc x=\"y\"/>"));
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

        CPPUNIT_ASSERT_EQUAL(aString, OString("<abc x=\"y\" q=\"w\"/>"));
    }
}

void Test::testSingleElementWithContent()
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc");
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL(aString, OString("<abc/>"));
}

void Test::testSingleElementWithContentAndAttributes()
{
    SvMemoryStream aStream;

    HtmlWriter aHtml(aStream);
    aHtml.prettyPrint(false);
    aHtml.start("abc");
    aHtml.attribute("x", "y");
    aHtml.attribute("q", "w");
    aHtml.end();

    OString aString = extractFromStream(aStream);

    CPPUNIT_ASSERT_EQUAL(aString, OString("<abc x=\"y\" q=\"w\"/>"));
}

void Test::testNested()
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

void Test::testAttributeValues()
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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
