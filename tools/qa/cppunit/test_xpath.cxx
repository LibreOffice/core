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
#include <rtl/ustring.hxx>
#include <test/xmldocptr.hxx>
#include <tools/stream.hxx>
#include <tools/XPath.hxx>

namespace
{
class XPathTest : public test::BootstrapFixture
{
    OUString maBasePath;

public:
    XPathTest()
        : BootstrapFixture(true, false)
    {
    }

    virtual void setUp() override { maBasePath = m_directories.getURLFromSrc(u"/tools/qa/data/"); }

    void testXPath()
    {
        OUString aXmlFilePath = maBasePath + "test.xml";
        SvFileStream aFileStream(aXmlFilePath, StreamMode::READ);
        std::size_t nSize = aFileStream.remainingSize();
        std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nSize + 1]);
        pBuffer[nSize] = 0;
        aFileStream.ReadBytes(pBuffer.get(), nSize);
        auto pCharBuffer = reinterpret_cast<xmlChar*>(pBuffer.get());

        xmlDocUniquePtr pXmlDoc(
            xmlReadDoc(pCharBuffer, nullptr, nullptr, XML_PARSE_NODICT | XML_PARSE_HUGE));

        tools::XPath aXPath(pXmlDoc.get());
        auto aNonExistentPath = aXPath.create("/nonexistent");
        CPPUNIT_ASSERT(aNonExistentPath);
        CPPUNIT_ASSERT_EQUAL(0, aNonExistentPath->count());

        auto aRootResult = aXPath.create("/root");
        CPPUNIT_ASSERT(aRootResult);
        CPPUNIT_ASSERT_EQUAL(1, aRootResult->count());
        CPPUNIT_ASSERT_EQUAL(u"Hello World"_ustr, aRootResult->attribute("root-attr"));
        {
            auto aRootElement = aRootResult->at(0);
            CPPUNIT_ASSERT_EQUAL(std::string_view("root"), aRootElement->name());
            CPPUNIT_ASSERT_EQUAL(4, aRootElement->countChildren());
            CPPUNIT_ASSERT_EQUAL(std::string_view("child"), aRootElement->at(0)->name());
            CPPUNIT_ASSERT_EQUAL(std::string_view("child"), aRootElement->at(1)->name());
            CPPUNIT_ASSERT_EQUAL(std::string_view("child"), aRootElement->at(2)->name());
            CPPUNIT_ASSERT_EQUAL(std::string_view("with-namespace"), aRootElement->at(3)->name());
        }

        auto aChildResult = aXPath.create(aRootResult, "/child");
        CPPUNIT_ASSERT(aChildResult);
        CPPUNIT_ASSERT_EQUAL(3, aChildResult->count());

        auto aChildElement = aChildResult->at(0);
        CPPUNIT_ASSERT_EQUAL(std::string_view("child"), aChildElement->name());
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, aChildElement->attribute("number"));
        CPPUNIT_ASSERT_EQUAL(u"123"_ustr, aChildElement->attribute("attribute"));

        auto aGrandChildElement = aChildElement->at(0);
        CPPUNIT_ASSERT(aGrandChildElement);
        CPPUNIT_ASSERT_EQUAL(std::string_view("grandchild"), aGrandChildElement->name());

        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, aChildResult->at(1)->attribute("number"));
        CPPUNIT_ASSERT_EQUAL(u"3"_ustr, aChildResult->at(2)->attribute("number"));

        auto aGrandChildResult = aXPath.create(aRootResult, "/child[1]/grandchild");
        CPPUNIT_ASSERT(aGrandChildResult);
        CPPUNIT_ASSERT_EQUAL(1, aGrandChildResult->count());
        CPPUNIT_ASSERT_EQUAL(u"Content"_ustr, aGrandChildResult->content());
        CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, aGrandChildResult->attribute("attribute1"));
        CPPUNIT_ASSERT_EQUAL(u"CDE"_ustr, aGrandChildResult->attribute("attribute2"));
    }

    CPPUNIT_TEST_SUITE(XPathTest);
    CPPUNIT_TEST(testXPath);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(XPathTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
