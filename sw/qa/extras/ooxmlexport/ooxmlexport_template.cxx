/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <o3tl/string_view.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr,
                          u"MS Word 2007 XML Template"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSaveAsDotX)
{
    loadAndReload("sample.dotx");
    xmlDocUniquePtr pXmlDocCT = parseExport(u"[Content_Types].xml"_ustr);

    // Ensure that document has correct content type
    assertXPath(
        pXmlDocCT, "/ContentType:Types/ContentType:Override[@PartName='/word/document.xml']"_ostr,
        "ContentType"_ostr,
        u"application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml"_ustr);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
