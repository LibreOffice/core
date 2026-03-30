/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

namespace
{
class XSLTImportTest : public SwModelTestBase
{
public:
    XSLTImportTest() :
        SwModelTestBase(u"/sw/qa/extras/xsltimport/data/"_ustr)
    {}
};

CPPUNIT_TEST_FIXTURE(XSLTImportTest, testReplaceSpaces)
{
    createSwDoc("testReplaceSpaces.xslt");
    uno::Reference<text::XTextRange> xPara(getParagraphOrTable(1), uno::UNO_QUERY_THROW);
    OUString sText = xPara->getString();
    CPPUNIT_ASSERT(sText.startsWith("  "));
    CPPUNIT_ASSERT(sText.endsWith("test"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10004), sText.getLength());
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */