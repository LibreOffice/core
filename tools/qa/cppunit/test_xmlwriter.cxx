/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/extensions/HelperMacros.h>
#include <test/bootstrapfixture.hxx>
#include <tools/stream.hxx>
#include <tools/XmlWriter.hxx>

namespace
{
class XmlWriterTest : public test::BootstrapFixture
{
public:
    XmlWriterTest()
        : BootstrapFixture(true, false)
    {
    }

    void setUp() override {}

    void testSimpleRoot();
    void testSpecialChars();
    void testAttributes();
    void testStartAndEndDocument();

    CPPUNIT_TEST_SUITE(XmlWriterTest);
    CPPUNIT_TEST(testSimpleRoot);
    CPPUNIT_TEST(testSpecialChars);
    CPPUNIT_TEST(testAttributes);
    CPPUNIT_TEST(testStartAndEndDocument);
    CPPUNIT_TEST_SUITE_END();
};

void XmlWriterTest::testStartAndEndDocument()
{
    SvMemoryStream aMemoryStream;
    tools::XmlWriter aWriter(&aMemoryStream);
    CPPUNIT_ASSERT(aWriter.startDocument());
    CPPUNIT_ASSERT(!aWriter.startDocument());
}

void XmlWriterTest::testSimpleRoot()
{
    SvMemoryStream aMemoryStream;

    tools::XmlWriter aWriter(&aMemoryStream);
    CPPUNIT_ASSERT(aWriter.startDocument(0, false));
    aWriter.startElement("test");
    aWriter.endElement();
    aWriter.endDocument();

    aMemoryStream.Seek(0);
    OString aString(static_cast<const char*>(aMemoryStream.GetData()), aMemoryStream.GetSize());
    CPPUNIT_ASSERT_EQUAL("<test/>"_ostr, aString);
}

void XmlWriterTest::testSpecialChars()
{
    SvMemoryStream aMemoryStream;

    tools::XmlWriter aWriter(&aMemoryStream);
    CPPUNIT_ASSERT(aWriter.startDocument(0, false));
    aWriter.startElement("test");
    aWriter.content("<>");
    aWriter.endElement();
    aWriter.endDocument();

    aMemoryStream.Seek(0);
    OString aString(static_cast<const char*>(aMemoryStream.GetData()), aMemoryStream.GetSize());
    CPPUNIT_ASSERT_EQUAL("<test>&lt;&gt;</test>"_ostr, aString);
}

void XmlWriterTest::testAttributes()
{
    SvMemoryStream aMemoryStream;

    tools::XmlWriter aWriter(&aMemoryStream);
    CPPUNIT_ASSERT(aWriter.startDocument(0, false));
    aWriter.startElement("test");
    aWriter.attribute("c", std::string_view("c"));
    aWriter.attribute("d", std::u16string_view(u"d"));
    aWriter.endElement();
    aWriter.endDocument();

    aMemoryStream.Seek(0);
    OString aString(static_cast<const char*>(aMemoryStream.GetData()), aMemoryStream.GetSize());
    CPPUNIT_ASSERT_EQUAL("<test c=\"c\" d=\"d\"/>"_ostr, aString);
}

CPPUNIT_TEST_SUITE_REGISTRATION(XmlWriterTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
