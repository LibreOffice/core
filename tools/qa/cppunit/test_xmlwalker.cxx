/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <string>

#include <sal/types.h>
#include <cppunit/TestFixture.h>
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

    virtual void setUp() override { maBasePath = m_directories.getURLFromSrc("/tools/qa/data/"); }

    void testReadXML();

    CPPUNIT_TEST_SUITE(XmlWalkerTest);
    CPPUNIT_TEST(testReadXML);
    CPPUNIT_TEST_SUITE_END();
};

void XmlWalkerTest::testReadXML()
{
    OUString aXmlFilePath = maBasePath + "test.xml";

    tools::XmlWalker aWalker;
    SvFileStream aFileStream(aXmlFilePath, StreamMode::READ);
    CPPUNIT_ASSERT(aWalker.open(&aFileStream));
    CPPUNIT_ASSERT_EQUAL(OString("root"), aWalker.name());
    CPPUNIT_ASSERT_EQUAL(OString("Hello World"), aWalker.attribute("root-attr"));

    int nNumberOfChildNodes = 0;

    aWalker.children();
    while (aWalker.isValid())
    {
        if (aWalker.name() == "child")
        {
            nNumberOfChildNodes++;

            CPPUNIT_ASSERT_EQUAL(OString::number(nNumberOfChildNodes), aWalker.attribute("number"));

            if (nNumberOfChildNodes == 1) // only the first node has the attribute
                CPPUNIT_ASSERT_EQUAL(OString("123"), aWalker.attribute("attribute"));
            else
                CPPUNIT_ASSERT_EQUAL(OString(), aWalker.attribute("attribute"));

            aWalker.children();
            while (aWalker.isValid())
            {
                if (aWalker.name() == "grandchild")
                {
                    CPPUNIT_ASSERT_EQUAL(OString("ABC"), aWalker.attribute("attribute1"));
                    CPPUNIT_ASSERT_EQUAL(OString("CDE"), aWalker.attribute("attribute2"));
                    CPPUNIT_ASSERT_EQUAL(OString("Content"), aWalker.content());
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

CPPUNIT_TEST_SUITE_REGISTRATION(XmlWalkerTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
