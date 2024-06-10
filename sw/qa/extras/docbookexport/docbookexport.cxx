/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

namespace
{
class DocbookExportTest : public SwModelTestBase
{
public:
    DocbookExportTest() :
        SwModelTestBase(u"/sw/qa/extras/docbookexport/data/"_ustr, u"DocBook File"_ustr)
    {}
};

CPPUNIT_TEST_FIXTURE(DocbookExportTest, testsimple)
{
    createSwDoc("simple.docx");
    save(mpFilter);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPathContent(pDoc, "/article/para"_ostr, u"aaaa"_ustr);
}

/* the test actually should crash with this file */
CPPUNIT_TEST_FIXTURE(DocbookExportTest, testtdf91095)
{
    createSwDoc("tdf91095.docx");
    save(mpFilter);
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
