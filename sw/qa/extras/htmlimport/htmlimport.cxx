/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <sfx2/linkmgr.hxx>

#include <docsh.hxx>
#include <editsh.hxx>

class HtmlImportTest : public SwModelTestBase
{
    public:
        HtmlImportTest() : SwModelTestBase("sw/qa/extras/htmlimport/data/", "HTML (StarWriter)") {}
};

#define DECLARE_HTMLIMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, HtmlImportTest)

DECLARE_HTMLIMPORT_TEST(testPictureImport, "picture.html")
{
    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTxtDoc);
    // The document contains one embeded picture and one stored as a link.
    const sfx2::LinkManager& rLinkManager = pTxtDoc->GetDocShell()->GetDoc()->GetEditShell()->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rLinkManager.GetLinks().size());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
