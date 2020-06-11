/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

char const DATA_DIRECTORY[] = "/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf133334_followPgStyle, "tdf133334_followPgStyle.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf133370_columnBreak, "tdf133370_columnBreak.odt")
{
    // Since non-DOCX formats ignores column breaks in non-column situtations, don't export to docx.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf131722, "tdf131722-date-contentcontrol-table.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport();
    // Check Date selector placeholder text is retained when at the beginning of the paragraph
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:sdt/w:sdtContent/w:r[2]/w:t", "Enter a date here!");
    // Check Date selector placeholder text retained when NOT at the beginning of the paragraph
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[3]/w:sdt/w:sdtContent/w:r[2]/w:t", "Enter a date here!");
    // Check Date selector placeholder text is retained when at the beginning of the paragraph in a table cell
    // Before the accompanying fix the first character was lost
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[1]/w:sdt/w:sdtContent/w:r[2]/w:t", "Enter a date here!");
    // Check Date selector placeholder text is retained when NOT at the beginning of the paragraph in a table cell
    // Before the accompanying fix the first character was lost
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[2]/w:tc[1]/w:p[1]/w:sdt/w:sdtContent/w:r[2]/w:t", "Enter a date here!");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
