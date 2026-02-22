/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

using namespace com::sun::star;

namespace
{
/// Covers xmloff/source/text/txtparae.cxx fixes.
class Test : public UnoApiTest
{
public:
    Test();
};

Test::Test()
    : UnoApiTest(u"/xmloff/qa/unit/text/data/"_ustr)
{
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineFormatCharStyleDirectExport)
{
    // Given a document that contains a format redline: both the character style and direct
    // formatting changes:
    loadFromFile(u"format-charstyle-direct.docx");

    // When exporting the document to ODT:
    save(TestFilter::ODT);

    // Then make sure the named style is not lost in the redline:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    OString aAutoName
        = getXPath(pXmlDoc, "//text:tracked-changes/text:changed-region/text:format-change",
                   "style-name")
              .toUtf8();
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//office:automatic-styles/style:style[@style:name='T2']' no attribute 'parent-style-name' exist
    // i.e. the named character style in the format redline was lost.
    OUString aParentName = getXPath(
        pXmlDoc, "//office:automatic-styles/style:style[@style:name='" + aAutoName + "']",
        "parent-style-name");
    CPPUNIT_ASSERT_EQUAL(u"Strong_20_Emphasis"_ustr, aParentName);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
