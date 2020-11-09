/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <memory>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <vcl/filter/pdfdocument.hxx>

class PDFDocumentTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    PDFDocumentTest() = default;
};

char const DATA_DIRECTORY[] = "/vcl/qa/cppunit/data/";

CPPUNIT_TEST_FIXTURE(PDFDocumentTest, testParse)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "basic.pdf";
    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(aURL, StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aPages.size());

    vcl::filter::PDFObjectElement* pResources = aPages[0]->LookupObject("Resources");
    CPPUNIT_ASSERT(pResources);

    vcl::filter::PDFObjectElement* pTest = pResources->LookupObject("Test");
    CPPUNIT_ASSERT(pTest);

    vcl::filter::PDFObjectElement* pTestArray1 = pTest->LookupObject("TestArray1");
    CPPUNIT_ASSERT(pTestArray1);

    CPPUNIT_ASSERT_EQUAL(size_t(3), pTestArray1->GetArray()->GetElements().size());

    vcl::filter::PDFObjectElement* pTestArray2 = pTest->LookupObject("TestArray2");
    CPPUNIT_ASSERT(pTestArray2);

    CPPUNIT_ASSERT_EQUAL(size_t(2), pTestArray2->GetArray()->GetElements().size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
