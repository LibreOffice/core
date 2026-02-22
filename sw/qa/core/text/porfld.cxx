/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>

#include <vcl/gdimtf.hxx>

#include <docsh.hxx>
#include <wrtsh.hxx>

namespace
{
/// Covers sw/source/core/text/porfld.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/text/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testNumberPortionRedlineRenderMode)
{
    // Given a document with redlines, the "2." number portion is inserted:
    createSwDoc("redline-number-portion.docx");

    // When redline render mode is standard:
    SwDocShell* pDocShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();

    // Then make sure we paint an underline:
    MetafileXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);
    OUString aContent = getXPathContent(pXmlDoc, "(//textarray)[3]/text");
    CPPUNIT_ASSERT_EQUAL(u"2."_ustr, aContent);
    OUString aUnderline
        = getXPath(pXmlDoc, "(//textarray)[3]/preceding-sibling::font[1]", "underline");
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, aUnderline);

    // And given "omit inserts" redline render mode:
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwViewOption aOpt(*pWrtShell->GetViewOptions());
    aOpt.SetRedlineRenderMode(SwRedlineRenderMode::OmitInserts);
    pWrtShell->ApplyViewOptions(aOpt);

    // When rendering:
    xMetaFile = pDocShell->GetPreviewMetaFile();

    // Then make sure we don't paint an underline:
    pXmlDoc = dumpAndParse(aDumper, *xMetaFile);
    aContent = getXPathContent(pXmlDoc, "(//textarray)[3]/text");
    CPPUNIT_ASSERT_EQUAL(u"2."_ustr, aContent);
    aUnderline = getXPath(pXmlDoc, "(//textarray)[3]/preceding-sibling::font[1]", "underline");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. there was an unexpected underline.
    CPPUNIT_ASSERT_EQUAL(u"0"_ustr, aUnderline);
}

CPPUNIT_TEST_FIXTURE(Test, testTabPortionRedlineRenderMode)
{
    // Given a document with redlines, the tab number portion is deleted:
    createSwDoc("redline-bullet.docx");
    SwDocShell* pDocShell = getSwDocShell();

    // When redline render mode is standard:
    std::shared_ptr<GDIMetaFile> xMetaFile = pDocShell->GetPreviewMetaFile();

    // Then make sure we paint a strikeout:
    MetafileXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);
    assertXPath(pXmlDoc, "//stretchtext", 1);
    OUString aStrikeout
        = getXPath(pXmlDoc, "(//stretchtext)[1]/preceding-sibling::font[1]", "strikeout");
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, aStrikeout);

    // And given "omit inserts" redline render mode:
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwViewOption aOpt(*pWrtShell->GetViewOptions());
    aOpt.SetRedlineRenderMode(SwRedlineRenderMode::OmitInserts);
    pWrtShell->ApplyViewOptions(aOpt);

    // When rendering:
    xMetaFile = pDocShell->GetPreviewMetaFile();

    // Then make sure we don't paint a strikeout:
    pXmlDoc = dumpAndParse(aDumper, *xMetaFile);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the stretched text was painted, which used a strikeout font.
    assertXPath(pXmlDoc, "//stretchtext", 0);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
