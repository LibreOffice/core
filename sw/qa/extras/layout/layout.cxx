/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <test/mtfxmldump.hxx>
#include <comphelper/scopeguard.hxx>
#include <unotools/syslocaleoptions.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/extras/layout/data/";

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter : public SwModelTestBase
{
public:
    void testUserFieldTypeLanguage();

    CPPUNIT_TEST_SUITE(SwLayoutWriter);
    CPPUNIT_TEST(testUserFieldTypeLanguage);
    CPPUNIT_TEST_SUITE_END();

private:
    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* SwLayoutWriter::createDoc(const char* pName)
{
    load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

void SwLayoutWriter::testUserFieldTypeLanguage()
{
    // Set the system locale to German, the document will be English.
    SvtSysLocaleOptions aOptions;
    aOptions.SetLocaleConfigString("de-DE");
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions] {
        aOptions.SetLocaleConfigString(OUString());
        aOptions.Commit();
    });

    SwDoc* pDoc = createDoc("user-field-type-language.fodt");
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->UpdateFields();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was "123,456.00", via a buggy 1234.56 -> 1234,56 -> 123456 ->
    // 123,456.00 transform chain.
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[@nType='POR_FLD']", "rText", "1,234.56");
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwLayoutWriter);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
