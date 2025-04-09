/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>

#include <comphelper/configuration.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <officecfg/Office/Common.hxx>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

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

CPPUNIT_TEST_FIXTURE(Test, testTdf165642_glossaryFootnote)
{
    loadAndSave("tdf165642_glossaryFootnote.docx");
    // round-trip'ing the settings.xml file as is, it contains footnote/endnote references
    xmlDocUniquePtr pXmlSettings = parseExport(u"word/glossary/settings.xml"_ustr);
    assertXPath(pXmlSettings, "//w:endnotePr", 1);
    assertXPath(pXmlSettings, "//w:footnotePr", 1);

    // thus, the footnote and endnote files must also be round-tripped
    parseExport(u"word/glossary/endnotes.xml"_ustr);
    parseExport(u"word/glossary/footnotes.xml"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165933_noDelTextOnMove)
{
    loadAndSave("tdf165933.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Wihtout the fix it fails with
    // - Expected: 0
    // - Actual  : 1
    // a w:delText is created inside a w:moveFrom, which is invalid
    assertXPath(pXmlDoc, "//w:moveFrom/w:r/w:delText", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165492_atLeastWithBottomSpacing)
{
    // Given a document with "minimum row height" of 2cm
    // and table bottom border padding of 1.5cm...

    loadAndSave("tdf165492_atLeastWithBottomSpacing.docx");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // the actual row heights should be at least 3.5cm each (rounded up = 1985 twips)
    // giving a total table height of 5955 instead of 3402.
    SwTwips nTableHeight
        = getXPath(pXmlDoc, "//column[1]/body/tab/infos/prtBounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(5955), nTableHeight);

    // the table in the right column has no bottom border padding, so its height is 3402.
    nTableHeight = getXPath(pXmlDoc, "//column[2]/body/tab/infos/prtBounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(3402), nTableHeight);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165047_consolidatedTopMargin)
{
    // Given a two page document with a section page break
    // which is preceded by a paragraph with a lot of lower spacing
    // and followed by a paragraph with even more upper spacing...
    loadAndSave("tdf165047_consolidatedTopMargin.docx");

    // the upper spacing is mostly "absorbed" by the preceding lower spacing, and is barely noticed
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // the effective top margin should be 60pt - 50pt = 10pt (0.36cm) after the page break
    SwTwips nParaTopMargin
        = getXPath(pXmlDoc, "/root/page[2]/body/section/infos/prtBounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(200), nParaTopMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165047_contextualSpacingTopMargin)
{
    // Given a two page document with a section page break
    // which is preceded by a paragraph with a lot of lower spacing
    // and followed by a paragraph with a lot of upper spacing,
    // but that paragraph says "don't add space between identical paragraph styles...
    loadAndSave("tdf165047_contextualSpacingTopMargin.docx");

    // the upper spacing is ignored since the paragraph styles are the same
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // The effective top margin (after the page break) must be 0
    SwTwips nParaTopMargin
        = getXPath(pXmlDoc, "/root/page[2]/body/section/infos/prtBounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(0), nParaTopMargin);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
