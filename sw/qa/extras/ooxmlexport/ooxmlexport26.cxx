/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <config_folders.h>
#include <osl/process.h>
#include <osl/file.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <wrtsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr)
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf160402, "StyleRef-DE.docx")
{
    xmlDocUniquePtr pLayout = parseLayoutDump();
    assertXPath(pLayout, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"Heading 1");
    assertXPath(pLayout, "/root/page[2]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"Nunc viverra imperdiet enim. Fusce est. Vivamus a tellus.");
    assertXPath(pLayout, "/root/page[3]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"Cras faucibus condimentum odio. Sed ac ligula. Aliquam at eros.");
    assertXPath(pLayout, "/root/page[4]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"Nunc viverra imperdiet enim. Fusce est. Vivamus a tellus.");
    assertXPath(pLayout, "/root/page[5]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"Aenean nec lorem. In porttitor. Donec laoreet nonummy augue.");
}

//test requires German user interface otherwise it will not detect the issue
CPPUNIT_TEST_FIXTURE(Test, testTdf166850)
{
    createSwDoc("tdf166850.docx");
    SwDoc* pDoc = getSwDoc();
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->UpdateFields();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[2]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"Heading 1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf166850_ODT)
{
    // test ODT import from version 24.2 where this worked
    // but only in the same locale that was used to import from DOCX, not in
    // a different one
    OUString instdir;
    osl_getEnvironment(u"INSTDIR"_ustr.pData, &instdir.pData);
    OUString url;
    CPPUNIT_ASSERT_EQUAL(::osl::FileBase::E_None,
                         ::osl::FileBase::getFileURLFromSystemPath(instdir, url));
    url += "/" LIBO_SHARE_RESOURCE_FOLDER "/de/LC_MESSAGES/sw.mo";
    ::osl::DirectoryItem item;
    if (::osl::DirectoryItem::get(url, item) == ::osl::FileBase::E_NOENT)
    {
        return;
    }
    createSwDoc("tdf166850_24.2.7.2.fodt");
    SwDoc* pDoc = getSwDoc();
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->UpdateFields();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"First heading");
    assertXPath(pXmlDoc, "/root/page[2]/header/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"Second heading");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"Second heading");
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
