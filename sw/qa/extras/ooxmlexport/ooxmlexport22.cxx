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
#include <comphelper/propertyvalue.hxx>
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

CPPUNIT_TEST_FIXTURE(Test, testTdf166201_simplePos)
{
    // Given a document with an image at the bottom-right placed there by simplePos

    loadAndSave("tdf166201_simplePos.docx");

    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
    // Without the fix, this was 0 - at the top left, instead of 10.5cm - at the bottom right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10478),
                         getProperty<sal_Int32>(getShape(1), u"HoriOrientPosition"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165492_exactWithBottomSpacing)
{
    // Given a document with "exact row height" of 2cm
    // and table bottom border padding of 1.5cm...

    loadAndSave("tdf165492_exactWithBottomSpacing.docx");

    // When laying out that document:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // the actual row heights should be exactly 3.5cm each (rounded up = 1985 twips)
    // giving a total table height of 5955 instead of 3402.
    SwTwips nTableHeight
        = getXPath(pXmlDoc, "//column[1]/body/tab/infos/prtBounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(5955), nTableHeight);

    // the table in the right column has no bottom border padding, so its height is 3402.
    nTableHeight = getXPath(pXmlDoc, "//column[2]/body/tab/infos/prtBounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwTwips>(3402), nTableHeight);
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

CPPUNIT_TEST_FIXTURE(Test, testAnnotationRef)
{
    loadAndSave("comment-annotationref.docx");

    // Check that the exported comment also has annotationRef
    xmlDocUniquePtr pXmlComments = parseExport(u"word/comments.xml"_ustr);
    CPPUNIT_ASSERT(pXmlComments);
    // Wihtout the fix it fails with
    // - Expected: 1
    // - Actual  : 0
    assertXPath(pXmlComments, "//w:comments/w:comment[1]/w:p[1]/w:r[1]/w:annotationRef");
}

CPPUNIT_TEST_FIXTURE(Test, testBadFormulaResult)
{
    // Given a loaded by not laid out document:
    uno::Sequence<beans::PropertyValue> aFilterOptions = {
        comphelper::makePropertyValue("Hidden", true),
    };
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(u"/sw/qa/extras/ooxmlexport/data/")
                                      + "formula-result.fodt",
                                  "com.sun.star.text.TextDocument", aFilterOptions);

    // When saving to DOCX:
    save(mpFilter);

    // Then make sure that the field result in the last row's last run is not empty:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6
    // - Actual  : 0
    // i.e. the SUM() field evaluated to an empty result.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc/w:p/w:r[4]/w:t", u"6");
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
