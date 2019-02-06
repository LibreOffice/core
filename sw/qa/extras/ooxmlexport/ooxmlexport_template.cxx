/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <string>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "MS Word 2007 XML Template")
    {
    }

protected:
    bool mustTestImportOf(const char* filename) const override
    {
        return OString(filename).endsWith(".dotx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testSaveAsDotX, "sample.dotx")
{
    xmlDocPtr pXmlDocCT = parseExport("[Content_Types].xml");

    if (!pXmlDocCT)
        return;

    // Ensure that document has correct content type
    assertXPath(pXmlDocCT,
                "/ContentType:Types/ContentType:Override[@PartName='/word/document.xml']",
                "ContentType",
                "application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
