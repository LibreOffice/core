/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/extensions/HelperMacros.h>
#include <test/bootstrapfixture.hxx>
#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <tools/XmlWalker.hxx>

namespace
{
class XmlWalkerTest : public test::BootstrapFixture
{
    OUString maBasePath;

public:
    XmlWalkerTest()
        : BootstrapFixture(true, false)
    {
    }

    virtual void setUp() override { maBasePath = m_directories.getURLFromSrc(u"/tools/qa/data/"); }

    void testReadXML();
    void testNullNames();

    CPPUNIT_TEST_SUITE(XmlWalkerTest);
    CPPUNIT_TEST(testReadXML);
    CPPUNIT_TEST(testNullNames);
    CPPUNIT_TEST_SUITE_END();
};

void XmlWalkerTest::testReadXML()
{
    OUString aXmlFilePath = maBasePath + "test.xml";

    tools::XmlWalker aWalker;
    SvFileStream aFileStream(aXmlFilePath, StreamMode::READ);
    CPPUNIT_ASSERT(aWalker.open(&aFileStream));
    CPPUNIT_ASSERT_EQUAL(std::string_view("root"), aWalker.name());
    CPPUNIT_ASSERT_EQUAL("Hello World"_ostr, aWalker.attribute("root-attr"_ostr));

    int nNumberOfChildNodes = 0;

    aWalker.children();
    while (aWalker.isValid())
    {
        if (aWalker.name() == "child")
        {
            nNumberOfChildNodes++;

            CPPUNIT_ASSERT_EQUAL(OString(OString::number(nNumberOfChildNodes)),
                                 aWalker.attribute("number"_ostr));

            if (nNumberOfChildNodes == 1) // only the first node has the attribute
                CPPUNIT_ASSERT_EQUAL("123"_ostr, aWalker.attribute("attribute"_ostr));
            else
                CPPUNIT_ASSERT_EQUAL(OString(), aWalker.attribute("attribute"_ostr));

            aWalker.children();
            while (aWalker.isValid())
            {
                if (aWalker.name() == "grandchild")
                {
                    CPPUNIT_ASSERT_EQUAL("ABC"_ostr, aWalker.attribute("attribute1"_ostr));
                    CPPUNIT_ASSERT_EQUAL("CDE"_ostr, aWalker.attribute("attribute2"_ostr));
                    CPPUNIT_ASSERT_EQUAL("Content"_ostr, aWalker.content());
                }
                aWalker.next();
            }
            aWalker.parent();
        }
        else if (aWalker.name() == "with-namespace")
        {
            CPPUNIT_ASSERT_EQUAL(std::string_view("adobe:ns:meta/"), aWalker.namespaceHref());
            CPPUNIT_ASSERT_EQUAL(std::string_view("xx"), aWalker.namespacePrefix());

            aWalker.children();
            while (aWalker.isValid())
            {
                if (aWalker.name() == "namespace-child")
                {
                    CPPUNIT_ASSERT_EQUAL(std::string_view("adobe:ns:meta/"),
                                         aWalker.namespaceHref());
                    CPPUNIT_ASSERT_EQUAL(std::string_view("xx"), aWalker.namespacePrefix());
                }
                aWalker.next();
            }
            aWalker.parent();
        }
        aWalker.next();
    }
    aWalker.parent();

    CPPUNIT_ASSERT_EQUAL(3, nNumberOfChildNodes);
}

void XmlWalkerTest::testNullNames()
{
    // An un-namespaced root, a CDATA child (no name in libxml2) and a default namespace
    // (no prefix) must not crash the accessors.
    const OString aXml = "<root><![CDATA[data]]><ns xmlns=\"urn:x\"/></root>"_ostr;
    SvMemoryStream aStream(const_cast<char*>(aXml.getStr()), aXml.getLength(), StreamMode::READ);

    tools::XmlWalker aWalker;
    CPPUNIT_ASSERT(aWalker.open(&aStream));
    CPPUNIT_ASSERT(aWalker.isElement());
    CPPUNIT_ASSERT_EQUAL(std::string_view(), aWalker.namespaceHref());
    CPPUNIT_ASSERT_EQUAL(std::string_view(), aWalker.namespacePrefix());

    aWalker.children();
    CPPUNIT_ASSERT(aWalker.isValid());
    CPPUNIT_ASSERT(!aWalker.isElement());
    CPPUNIT_ASSERT_EQUAL(std::string_view(), aWalker.name());

    aWalker.next();
    CPPUNIT_ASSERT(aWalker.isElement());
    CPPUNIT_ASSERT_EQUAL(std::string_view("ns"), aWalker.name());
    CPPUNIT_ASSERT_EQUAL(std::string_view("urn:x"), aWalker.namespaceHref());
    CPPUNIT_ASSERT_EQUAL(std::string_view(), aWalker.namespacePrefix());
    aWalker.parent();
}

CPPUNIT_TEST_SUITE_REGISTRATION(XmlWalkerTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
