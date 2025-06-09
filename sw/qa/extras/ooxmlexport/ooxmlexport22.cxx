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

#include <set>

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

CPPUNIT_TEST_FIXTURE(Test, testTdf166201_simplePosCM)
{
    // Given a document with an image at the bottom-right placed there by simplePos - in cm

    loadAndSave("tdf166201_simplePosCM.docx");

    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), u"VertOrientRelation"_ustr));
    // Without the fix, this was 0 - at the top left, instead of 5cm - at the bottom right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001),
                         getProperty<sal_Int32>(getShape(1), u"VertOrientPosition"_ustr));
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

CPPUNIT_TEST_FIXTURE(Test, testDelThenFormatDocxExport)
{
    // Given a document with <del>A<format>B</format>C</del> style redlines:
    // When exporting that document:
    loadAndSave("del-then-format.docx");

    // Then make sure delete "under" format uses the <w:delText> markup:
    // Without the accompanying fix in place, this test would have failed with:
    // - In <>, XPath '/w:document/w:body/w:p/w:del[2]/w:r/w:delText' not found
    // i.e. <w:t> was used, not <w:delText>.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:del[2]/w:r/w:delText", u"BBB");
}

DECLARE_OOXMLEXPORT_TEST(testTdf139418, "tdf139418.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);

    sal_Int32 nBaseWidth;
    xPropertySet->getPropertyValue(u"GridBaseWidth"_ustr) >>= nBaseWidth;
    // Without the fix, this will fail with
    // - Expected: ~795
    // - Actual  : 848
    CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(794), nBaseWidth);
    CPPUNIT_ASSERT_LESSEQUAL(sal_Int32(796), nBaseWidth);

    auto pXmlDoc = parseLayoutDump();

    // Vertical DOCX should insert kern portions to align text to the grid
    sal_Int32 nPorLen1 = getXPath(pXmlDoc, "(//SwLinePortion)[1]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nPorLen1);

    sal_Int32 nPorLen2 = getXPath(pXmlDoc, "(//SwLinePortion)[2]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(42), nPorLen2);

    sal_Int32 nPorLen3 = getXPath(pXmlDoc, "(//SwLinePortion)[3]", "length").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nPorLen3);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf166511)
{
    loadAndSave("tdf166511.docx");

    // The reproducer for tdf#166511 depends on result having footer2 and footer3.xml
    // Original bugdoc only has footer1.xml, and the result only started having multiples
    //  after tdf#136472's fix
    // This test uses a "good" roundtripped version of the bugdoc after the fix already
    //  having the extra footer XMLs, so it doesn't depend on the change coming from tdf#136472

    // Count total docPr ids and collect unique docPr ids to be counted later from various parts
    //  of the doc, in the end both counts must be the same, as ids have to be unique
    std::set<sal_Int32> aDiffIds;
    size_t nIds = 0;
    const OUString aFooters[] = { u"word/footer2.xml"_ustr, u"word/footer3.xml"_ustr };
    for (const OUString& footer : aFooters)
    {
        xmlDocUniquePtr pXmlDoc = parseExport(footer);
        CPPUNIT_ASSERT(pXmlDoc);

        const int aDrawingNodes1 = countXPathNodes(pXmlDoc, "/w:ftr/w:p/w:r/mc:AlternateContent");
        for (int i = 1; i <= aDrawingNodes1; i++)
        {
            OUString aId = getXPath(pXmlDoc,
                                    "/w:ftr/w:p/w:r/mc:AlternateContent[" + OString::number(i)
                                        + "]/mc:Choice/w:drawing/wp:anchor/wp:docPr",
                                    "id");
            aDiffIds.insert(aId.toInt32());
            nIds++;
            const OString aFallbackXPath
                = "/w:ftr/w:p/w:r/mc:AlternateContent[" + OString::number(i)
                  + "]/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:r/"
                    "w:drawing/wp:inline/wp:docPr";
            if (countXPathNodes(pXmlDoc, aFallbackXPath) > 0)
            {
                aId = getXPath(pXmlDoc, aFallbackXPath, "id");
                aDiffIds.insert(aId.toInt32());
                nIds++;
            }
        }
        const int aDrawingNodes2 = countXPathNodes(pXmlDoc, "/w:ftr/w:p/w:r/w:drawing");
        for (int i = 1; i <= aDrawingNodes2; i++)
        {
            OUString aId = getXPath(
                pXmlDoc, "/w:ftr/w:p/w:r/w:drawing[" + OString::number(i) + "]/wp:anchor/wp:docPr",
                "id");
            aDiffIds.insert(aId.toInt32());
            nIds++;
        }
    }
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected : 7
    // - Actual : 8
    CPPUNIT_ASSERT_EQUAL(aDiffIds.size(), nIds);
}

CPPUNIT_TEST_FIXTURE(Test, testLineBreakInRef)
{
    loadAndSave("line-break-in-ref.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Without the fix if fails with
    // assertion failed
    // - Expression: xmlXPathNodeSetGetLength(pXmlNodes) > 0
    // - In <>, XPath '/w:document/w:body/w:p[1]/w:r[4]/w:t' not found
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[4]/w:t", u"Text1");
}

DECLARE_OOXMLEXPORT_TEST(testFieldMarkFormat, "fontsize-field-separator.docx")
{
    uno::Reference<text::XTextRange> xRun(getRun(getParagraph(1), 1));

    // Without the fix it fails with
    // - Expected: 12
    // - Actual  : 42
    // i.e. the field content has the properties fo the field marks
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(xRun, u"CharHeight"_ustr));
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
