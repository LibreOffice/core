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
    assertXPath(pXmlSettings, "//w:endnotePr"_ostr, 1);
    assertXPath(pXmlSettings, "//w:footnotePr"_ostr, 1);

    // thus, the footnote and endnote files must also be round-tripped
    parseExport(u"word/glossary/endnotes.xml"_ustr);
    parseExport(u"word/glossary/footnotes.xml"_ustr);
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

CPPUNIT_TEST_FIXTURE(Test, testTdf165933_noDelTextOnMove)
{
    loadAndSave("tdf165933.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Wihtout the fix it fails with
    // - Expected: 0
    // - Actual  : 1
    // a w:delText is created inside a w:moveFrom, which is invalid
    assertXPath(pXmlDoc, "//w:moveFrom/w:r/w:delText"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf166975)
{
    createSwDoc("WordOK.docx");

    CPPUNIT_ASSERT_EQUAL(u"a)"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));
    // this was a%6%)
    CPPUNIT_ASSERT_EQUAL(u"aa)"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));
    // this was a%7%%7%)
    CPPUNIT_ASSERT_EQUAL(u"aaa)"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"bbb)"_ustr,
                         getProperty<OUString>(getParagraph(5), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"ccc)"_ustr,
                         getProperty<OUString>(getParagraph(6), u"ListLabelString"_ustr));

    saveAndReload(mpFilter);

    CPPUNIT_ASSERT_EQUAL(u"a)"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr));
    // this was aa%)
    CPPUNIT_ASSERT_EQUAL(u"aa)"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));
    // this was aa%a%)
    CPPUNIT_ASSERT_EQUAL(u"aaa)"_ustr,
                         getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"bbb)"_ustr,
                         getProperty<OUString>(getParagraph(5), u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"ccc)"_ustr,
                         getProperty<OUString>(getParagraph(6), u"ListLabelString"_ustr));
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
    assertXPath(pXmlComments, "//w:comments/w:comment[1]/w:p[1]/w:r[1]/w:annotationRef"_ostr);
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

        const int aDrawingNodes1
            = countXPathNodes(pXmlDoc, "/w:ftr/w:p/w:r/mc:AlternateContent"_ostr);
        for (int i = 1; i <= aDrawingNodes1; i++)
        {
            OUString aId = getXPath(pXmlDoc,
                                    "/w:ftr/w:p/w:r/mc:AlternateContent[" + OString::number(i)
                                        + "]/mc:Choice/w:drawing/wp:anchor/wp:docPr",
                                    "id"_ostr);
            aDiffIds.insert(aId.toInt32());
            nIds++;
            const OString aFallbackXPath
                = "/w:ftr/w:p/w:r/mc:AlternateContent[" + OString::number(i)
                  + "]/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:r/"
                    "w:drawing/wp:inline/wp:docPr";
            if (countXPathNodes(pXmlDoc, aFallbackXPath) > 0)
            {
                aId = getXPath(pXmlDoc, aFallbackXPath, "id"_ostr);
                aDiffIds.insert(aId.toInt32());
                nIds++;
            }
        }
        const int aDrawingNodes2 = countXPathNodes(pXmlDoc, "/w:ftr/w:p/w:r/w:drawing"_ostr);
        for (int i = 1; i <= aDrawingNodes2; i++)
        {
            OUString aId = getXPath(
                pXmlDoc, "/w:ftr/w:p/w:r/w:drawing[" + OString::number(i) + "]/wp:anchor/wp:docPr",
                "id"_ostr);
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
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[4]/w:t"_ostr, "Text1");
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
