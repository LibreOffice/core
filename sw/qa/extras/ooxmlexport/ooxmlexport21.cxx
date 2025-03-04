/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

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

CPPUNIT_TEST_FIXTURE(Test, testTdf161631)
{
    auto verify = [this](bool bIsExport = false) {
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

        // Without the fix in place, this test would have failed with
        // - Expected : Some text
        // - Actual:
        CPPUNIT_ASSERT_EQUAL(u"Some text"_ustr, getParagraph(1)->getString());

        if (bIsExport)
        {
            // tdf#164876 tdf#165117: don't add an empty paragraph every round-trip
            xmlDocUniquePtr pXmlDoc1 = parseExport(u"word/footer2.xml"_ustr);
            assertXPath(pXmlDoc1, "/w:ftr/w:p", 1);
        }
    };

    createSwDoc("tdf161631.docx");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

DECLARE_OOXMLEXPORT_TEST(testTdf158597, "tdf158597.docx")
{
    // test with 2 properties: font size, italic (toggle)
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(1));
        CPPUNIT_ASSERT_EQUAL(u"No style"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        CPPUNIT_ASSERT(!xProps->getPropertyValue(u"ListAutoFormat"_ustr).hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(2));
        CPPUNIT_ASSERT_EQUAL(u"Char style mark"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        comphelper::SequenceAsHashMap listAutoFormat(
            xProps->getPropertyValue(u"ListAutoFormat"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr,
                             listAutoFormat[u"CharStyleName"_ustr].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat[u"CharHeight"_ustr].hasValue());
        CPPUNIT_ASSERT(!listAutoFormat[u"CharPosture"_ustr].hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(3));
        CPPUNIT_ASSERT_EQUAL(u"Char style mark and text"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        comphelper::SequenceAsHashMap listAutoFormat(
            xProps->getPropertyValue(u"ListAutoFormat"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr,
                             listAutoFormat[u"CharStyleName"_ustr].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat[u"CharHeight"_ustr].hasValue());
        CPPUNIT_ASSERT(!listAutoFormat[u"CharPosture"_ustr].hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(4));
        CPPUNIT_ASSERT_EQUAL(u"Char style text"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        CPPUNIT_ASSERT(!xProps->getPropertyValue(u"ListAutoFormat"_ustr).hasValue());
    }

    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(5));
        CPPUNIT_ASSERT_EQUAL(u"Para style"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph E"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        CPPUNIT_ASSERT(!xProps->getPropertyValue(u"ListAutoFormat"_ustr).hasValue());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(6));
        CPPUNIT_ASSERT_EQUAL(u"Para style + Char style mark"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph E"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        comphelper::SequenceAsHashMap listAutoFormat(
            xProps->getPropertyValue(u"ListAutoFormat"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr,
                             listAutoFormat[u"CharStyleName"_ustr].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat[u"CharHeight"_ustr].hasValue());
        // bug was that here the toggle property was not overwritten
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             listAutoFormat[u"CharPosture"_ustr].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(7));
        CPPUNIT_ASSERT_EQUAL(u"Para style + Char style mark and text"_ustr,
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph E"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        comphelper::SequenceAsHashMap listAutoFormat(
            xProps->getPropertyValue(u"ListAutoFormat"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr,
                             listAutoFormat[u"CharStyleName"_ustr].get<OUString>());
        CPPUNIT_ASSERT(!listAutoFormat[u"CharHeight"_ustr].hasValue());
        // bug was that here the toggle property was not overwritten
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             listAutoFormat[u"CharPosture"_ustr].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(8));
        CPPUNIT_ASSERT_EQUAL(u"Para style + Char style text"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph E"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        CPPUNIT_ASSERT(!xProps->getPropertyValue(u"ListAutoFormat"_ustr).hasValue());
    }

    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(9));
        CPPUNIT_ASSERT_EQUAL(u"No style + direct mark"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        comphelper::SequenceAsHashMap listAutoFormat(
            xProps->getPropertyValue(u"ListAutoFormat"_ustr));
        CPPUNIT_ASSERT(!listAutoFormat[u"CharStyleName"_ustr].hasValue());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat[u"CharHeight"_ustr].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat[u"CharPosture"_ustr].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(10));
        CPPUNIT_ASSERT_EQUAL(u"Char style + direct mark"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        comphelper::SequenceAsHashMap listAutoFormat(
            xProps->getPropertyValue(u"ListAutoFormat"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr,
                             listAutoFormat[u"CharStyleName"_ustr].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat[u"CharHeight"_ustr].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat[u"CharPosture"_ustr].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(11));
        CPPUNIT_ASSERT_EQUAL(u"Char style + direct mark and text"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        comphelper::SequenceAsHashMap listAutoFormat(
            xProps->getPropertyValue(u"ListAutoFormat"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr,
                             listAutoFormat[u"CharStyleName"_ustr].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat[u"CharHeight"_ustr].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat[u"CharPosture"_ustr].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(12));
        CPPUNIT_ASSERT_EQUAL(u"Char style + direct text"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        CPPUNIT_ASSERT(!xProps->getPropertyValue(u"ListAutoFormat"_ustr).hasValue());
    }

    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(13));
        CPPUNIT_ASSERT_EQUAL(u"Para style + direct mark"_ustr, xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph E"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        comphelper::SequenceAsHashMap listAutoFormat(
            xProps->getPropertyValue(u"ListAutoFormat"_ustr));
        CPPUNIT_ASSERT(!listAutoFormat[u"CharStyleName"_ustr].hasValue());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat[u"CharHeight"_ustr].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat[u"CharPosture"_ustr].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(14));
        CPPUNIT_ASSERT_EQUAL(u"Para style + Char style + direct mark"_ustr,
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph E"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        comphelper::SequenceAsHashMap listAutoFormat(
            xProps->getPropertyValue(u"ListAutoFormat"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr,
                             listAutoFormat[u"CharStyleName"_ustr].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat[u"CharHeight"_ustr].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat[u"CharPosture"_ustr].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(15));
        CPPUNIT_ASSERT_EQUAL(u"Para style + Char style + direct mark and text"_ustr,
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph E"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        comphelper::SequenceAsHashMap listAutoFormat(
            xProps->getPropertyValue(u"ListAutoFormat"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr,
                             listAutoFormat[u"CharStyleName"_ustr].get<OUString>());
        CPPUNIT_ASSERT_EQUAL(16.f, listAutoFormat[u"CharHeight"_ustr].get<float>());
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             listAutoFormat[u"CharPosture"_ustr].get<awt::FontSlant>());
    }
    {
        uno::Reference<text::XTextRange> xParagraph(getParagraph(16));
        CPPUNIT_ASSERT_EQUAL(u"Para style + Char style + direct text"_ustr,
                             xParagraph->getString());
        uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"List Paragraph E"_ustr,
                             getProperty<OUString>(xProps, u"ParaStyleName"_ustr));
        uno::Reference<text::XTextRange> xRun(getRun(xParagraph, 1));
        CPPUNIT_ASSERT_EQUAL(u"Emphasis"_ustr, getProperty<OUString>(xRun, u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        CPPUNIT_ASSERT(!xProps->getPropertyValue(u"ListAutoFormat"_ustr).hasValue());
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf125469_singleSpacing, "tdf125469_singleSpacing.docx")
{
    // Given a document with 4 paragraphs of varying strange line spacing definitions,
    // and a DocDefault of single line spacing (AUTO 240pt) (240pt is 0.423 cm)

    // Paragraph 1 - DocDefaults specifies size 240 without a lineRule - default is AUTO(aka PROP)
    // Visually, this should clearly say "Single spacing"
    auto aSpacing = getProperty<style::LineSpacing>(getParagraph(1), u"ParaLineSpacing"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::PROP), aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), aSpacing.Height);

    // Paragraph 2 - paragraph style specifies atLeast 240, para overrides with only -240.
    // The negative value (always) turns the (inherited) "atLeast" into an "exact".
    // Visually, this is hardly readable (36pt font forced into 12pt space)
    aSpacing = getProperty<style::LineSpacing>(getParagraph(2), u"ParaLineSpacing"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::FIX), aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(423), aSpacing.Height);

    // Paragraph 3 - paragraph style specifies exact 240, para overrides with exact -240.
    // The negative value turns the non-inherited "exact" into an "atLeast".
    // Visually, this should clearly say "Negative exact"
    aSpacing = getProperty<style::LineSpacing>(getParagraph(3), u"ParaLineSpacing"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::MINIMUM), aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(423), aSpacing.Height);

    // Paragraph 4 - paragraph style specifies exact 240, para overrides with only -240.
    // The negative value does nothing to the inherited "exact".
    // Visually, this is hardly readable (36pt font forced into 12pt space)
    aSpacing = getProperty<style::LineSpacing>(getParagraph(4), u"ParaLineSpacing"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(style::LineSpacingMode::FIX), aSpacing.Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(423), aSpacing.Height);

    // all of this ends up being squeezed onto a single page
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf43767_caseMapNumbering)
{
    auto verify = [this](bool bIsExport = false) {
        // given a document with 2 numbered Lists [each entry restarts numbering for visual comparison]
        xmlDocUniquePtr pDump = parseLayoutDump();

        // using the relative width difference between "A)" and "a)" as the test comparison
        // since ListLabelString etc. does not output the actual string that is displayed on the screen

        // When the entire paragraph has a certain character attribute, that property is also applied
        // to the list numbering itself (with some differing exceptions) for both ODT and DOCX.

        // ESTABLISH A BASELINE: these baseline paragraphs have no special character attributes.
        // Paragraph 1/list 1(uppercase): no formatting applied to list numbering. Width is 253 for me
        const sal_Int32 nUpperCaseWidth
            = getXPath(pDump, "//body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion", "width")
                  .toInt32();
        // Paragraph 4/list 2(lowercase): no formatting applied to list numbering. Width is 186 for me.
        const sal_Int32 nLowerCaseWidth
            = getXPath(pDump, "//body/txt[5]/SwParaPortion/SwLineLayout/SwFieldPortion", "width")
                  .toInt32();

        // UPPERCASE LIST
        // Paragraph 2: ODF should honour "lowercase". MSO doesn't know about lowercase
        sal_Int32 nWidth
            = getXPath(pDump, "//body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion", "width")
                  .toInt32();
        CPPUNIT_ASSERT_EQUAL(bIsExport ? nUpperCaseWidth : nLowerCaseWidth, nWidth);

        // Paragraph 3: ODF should honour "superscript" (for consistency). MSO ignores superscript
        nWidth = getXPath(pDump, "//body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion", "width")
                     .toInt32();
        if (!bIsExport)
            CPPUNIT_ASSERT_LESS(nLowerCaseWidth, nWidth);
        else
            CPPUNIT_ASSERT_EQUAL(nUpperCaseWidth, nWidth);

        // LOWERCASE LIST
        //Paragraph 6: ODF should honour "titlecase". MSO doesn't know about titlecase
        nWidth = getXPath(pDump, "//body/txt[6]/SwParaPortion/SwLineLayout/SwFieldPortion", "width")
                     .toInt32();
        CPPUNIT_ASSERT_EQUAL(bIsExport ? nLowerCaseWidth : nUpperCaseWidth, nWidth);

        // Paragraph 7: ODF should honour "smallcaps". MSO apparently has an exception for small caps
        nWidth = getXPath(pDump, "//body/txt[7]/SwParaPortion/SwLineLayout/SwFieldPortion", "width")
                     .toInt32();
        if (!bIsExport)
        {
            CPPUNIT_ASSERT_GREATER(nLowerCaseWidth, nWidth);
            CPPUNIT_ASSERT_LESS(nUpperCaseWidth, nWidth);
        }
        else
            CPPUNIT_ASSERT_EQUAL(nLowerCaseWidth, nWidth);

        // Paragraph 8: ODF should honour "uppercase". MSO also honours uppercase
        nWidth = getXPath(pDump, "//body/txt[8]/SwParaPortion/SwLineLayout/SwFieldPortion", "width")
                     .toInt32();
        CPPUNIT_ASSERT_EQUAL(nUpperCaseWidth, nWidth);
    };

    createSwDoc("tdf43767_caseMapNumbering.odt");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156105_percentSuffix)
{
    auto verify = [this]() {
        // given a numbered list with a non-escaping percent symbol in the prefix and suffix
        CPPUNIT_ASSERT_EQUAL(u"(%)[%]"_ustr,
                             getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));

        // tdf#149258 - NONE number should not export separator since LO doesn't currently show it
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "showing levels 1, 2, and 4", u"(%)1.1.1[%]"_ustr,
            getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
    };

    createSwDoc("tdf156105_percentSuffix.odt");
    verify();
    saveAndReload(mpFilter);
    verify();

    xmlDocUniquePtr pXmlNum = parseExport(u"word/numbering.xml"_ustr);
    // The 3rd level is NONE. If we include the separator, MS Word will display it.
    assertXPath(pXmlNum, "/w:numbering/w:abstractNum[1]/w:lvl[4]/w:lvlText", "val",
                u"(%)%1.%2.%3%4[%]");
}

DECLARE_OOXMLEXPORT_TEST(testTdf160049_anchorMarginVML, "tdf160049_anchorMarginVML.docx")
{
    // given a VML (Word 2003) document with a LEFT "column/text" anchored image
    // (which will import as DML compat12 on the round-trip)

    // The image takes into account the margin, so it looks like it is in the middle of the doc,
    // which is "Paragraph text area"/PRINT_AREA/1, not "Entire paragraph area"/FRAME/0
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PRINT_AREA,
                         getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf160049_anchorMargin2, "tdf160049_anchorMargin2.docx")
{
    // given a DML compat14 (Word 2010) document with a LEFT "column/text" anchored shape

    // The shape takes into account the margin, so it looks like it is in the middle of the doc,
    // which is "Paragraph text area"/PRINT_AREA/1, not "Entire paragraph area"/FRAME/0
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PRINT_AREA,
                         getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf160049_anchorMargin14, "tdf160049_anchorMargin14.docx")
{
    // given a DML compat14 (Word 2010) document with a LEFT "column/text" anchored image
    // followed by a RIGHT column/text anchored image (with a 2cm paragraph right margin)
    // followed by a CENTER column/text anchored image (with a large left margin)
    // followed by a LEFT FROM column/text anchored image (which ignores the left margin)
    // followed by a LEFT "margin" anchored image (which always ignores the left margin)

    // The shape takes into account the left margin, looking like it is in the middle of the doc,
    // which is "Paragraph text area"/PRINT_AREA/1, not "Entire paragraph area"/FRAME/0
    uno::Reference<drawing::XShape> xShape(getShapeByName(u"Picture 2"));
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PRINT_AREA,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    // The shape takes into account the right margin, looking like it is in the middle of the doc,
    xShape.set(getShapeByName(u"Picture 3"));
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PRINT_AREA,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    // The third shape takes "center" very seriously, and ignores the margins
    xShape.set(getShapeByName(u"Picture 4"));
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    // The fourth shape takes "left by 123", and ignores the margins, acting just like "margin"
    xShape.set(getShapeByName(u"Picture 5"));
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    // The fifth shape shows how "left" works with "margin", which apparently means page margin
    xShape.set(getShapeByName(u"Picture 6"));
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf160049_anchorMargin15, "tdf160049_anchorMargin15.docx")
{
    // given a DML compat15 (Word 2013) document with a LEFT "column/text" anchored image

    // The image ignores the margin, so it is at the left for compat15 (but middle-ish for compat14)
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::FRAME,
                         getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf160077_layoutInCell, "tdf160077_layoutInCell.docx")
{
    // given an in-table, slightly rotated image vertically aligned -1cm (above) the top page margin
    // (which is actually forced to layoutInCell, so that becomes 1cm above the cell margin instead)

    // This document is particularly tricky. The image is in cell A1 with no special cell spacing
    // (no top/bottom margins), but Cell A2 has a custom top margin of 2cm,
    // so that effectively drops A1's print area down as well!

    xmlDocUniquePtr pDump = parseLayoutDump();
    const sal_Int32 nCellTop = getXPath(pDump, "//row[1]/cell[1]/infos/bounds", "top").toInt32();
    const sal_Int32 nImageTop
        = getXPath(pDump, "//row[1]/cell[1]/txt/anchored/SwAnchoredDrawObject/bounds", "top")
              .toInt32();
    // The image should be 1 cm above the 2cm cell margin (thus 1cm below the top of the cell)
    // 1cm is 567 twips. The numbers are not exactly what I would have expected, but close.
    // correct ImageTop: ~ 3588, while incorrect value was 1117. Cell top is 3051, ParaTop is 4195
    const SwTwips n1cm = o3tl::convert(tools::Long(1), o3tl::Length::cm, o3tl::Length::twip);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nCellTop + n1cm, nImageTop, 50); // +/- 4.4%

    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(getShape(1), u"VertOrientRelation"_ustr));

    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), u"IsFollowingTextFlow"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf160077_layoutInCellB, "tdf160077_layoutInCellB.docx")
{
    // given an in-table, group-shape vertically aligned -1.35 cm (above) the top page margin
    // (which is actually forced to layoutInCell, so that turns into 1.35cm above the cell margin)

    // This unit test is virtually the same idea as the previous one, with the main benefit being
    // that it causes an NS_ooxml::LN_Shape exception.

    xmlDocUniquePtr pDump = parseLayoutDump();
    const sal_Int32 nShapeTop
        = getXPath(pDump,
                   "//body/tab[1]/row[1]/cell[1]/txt[1]/anchored/SwAnchoredDrawObject/bounds",
                   "top")
              .toInt32();
    // The shape is approximately 1 cm below the top of the page, and ~0.5cm above the cell
    // correct ShapeTop: 888 TWIPS, while incorrect value was -480. Cell top is 1148, PageTop is 284
    CPPUNIT_ASSERT_DOUBLES_EQUAL(888, nShapeTop, 50);

    const auto xShape = getShapeByName(u"Group 1");
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));

    CPPUNIT_ASSERT(getProperty<bool>(xShape, u"IsFollowingTextFlow"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf160077_layoutInCellC, "tdf160077_layoutInCellC.docx")
{
    // given an in-table, slightly rotated image vertically aligned to top page margin
    // (which is actually forced to layoutInCell, so that becomes the top of the cell text area).
    // This test anchors the image on paragraph 5 - proving vertical cannot change to FRAME.

    xmlDocUniquePtr pDump = parseLayoutDump();
    const sal_Int32 nPara1Top
        = getXPath(pDump, "//row[1]/cell[2]/txt[1]/infos/bounds", "top").toInt32();
    const sal_Int32 nPara1Bottom
        = getXPath(pDump, "//row[1]/cell[2]/txt[1]/infos/bounds", "bottom").toInt32();
    const sal_Int32 nImageTop
        = getXPath(pDump, "//row[1]/cell[2]/txt[5]/anchored/SwAnchoredDrawObject/bounds", "top")
              .toInt32();
    // The image's top should be positioned at the start of the cell's text area (i.e. para1 top)
    // Before the fix, the image was positioned at the top of the cell.
    CPPUNIT_ASSERT_LESS(nPara1Bottom, nImageTop); // Image shouldn't start lower than para 1
    // The image must be limited to the top of the cell, not the page
    CPPUNIT_ASSERT_GREATEREQUAL(nPara1Top, nImageTop); // Image shouldn't start higher than para 1

    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(getShape(1), u"VertOrientRelation"_ustr));
    // LayoutInCell must be enforced, to keep the image inside the cell boundaries
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), u"IsFollowingTextFlow"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf160077_layoutInCellD, "tdf160077_layoutInCellD.docx")
{
    // given a table with two layoutInCell images, and cell A1 has 1/2 inch border padding (margin)
    // - A1 contains an image, vertically aligned to the bottom of the page (aka cell)
    // - B1 contains an image, vertically aligned to the bottom of the page margin (aka cell margin)

    // In Microsoft's layoutInCell implementation, vertical "page" is identical to "margin",
    // and everything (including bottom) actually is oriented to the top of the margin.

    xmlDocUniquePtr pDump = parseLayoutDump();
    // Cell A1
    sal_Int32 nShapeTop
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[1]/anchored/fly/SwAnchoredObject/bounds", "top")
              .toInt32();
    sal_Int32 nShapeBottom
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[1]/anchored/fly/SwAnchoredObject/bounds",
                   "bottom")
              .toInt32();
    // use paragraph 1 to indicate where the cell spacing/padding ends, and the text starts.
    sal_Int32 nPara1Top
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[1]/infos/bounds", "top").toInt32();
    // use paragraph 5 to prove the image is not at the bottom.
    CPPUNIT_ASSERT_EQUAL(OUString("Below logo"),
                         getXPathContent(pDump, "//tab/row[1]/cell[1]/txt[5]"));
    sal_Int32 nPara5Top
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[5]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(nShapeTop, nPara1Top);
    CPPUNIT_ASSERT(nPara5Top > nShapeBottom); // ShapeBottom is higher than Para5Top

    // In the file it is specified as "page" (PAGE_FRAME), but implemented as if it were "margin"
    // so on import we intentionally changed it to match the closest setting to the implementation.
    const auto xShape = getShapeByName(u"logo");
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));

    CPPUNIT_ASSERT(getProperty<bool>(xShape, u"IsFollowingTextFlow"_ustr));

    // Cell B1
    nShapeTop
        = getXPath(pDump, "//tab/row[1]/cell[2]/txt[1]/anchored/fly/SwAnchoredObject/bounds", "top")
              .toInt32();
    nShapeBottom
        = getXPath(pDump, "//tab/row[1]/cell[2]/txt[1]/anchored/fly/SwAnchoredObject/bounds",
                   "bottom")
              .toInt32();
    // use paragraph 1 to indicate where the cell spacing/padding ends, and the text starts.
    nPara1Top = getXPath(pDump, "//tab/row[1]/cell[2]/txt[1]/infos/bounds", "top").toInt32();
    // use paragraph 5 to prove the image is not at the bottom.
    CPPUNIT_ASSERT_EQUAL(OUString("Below image"),
                         getXPathContent(pDump, "//tab/row[1]/cell[2]/txt[5]"));
    nPara5Top = getXPath(pDump, "//tab[1]/row/cell[2]/txt[5]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(nShapeTop, nPara1Top);
    CPPUNIT_ASSERT(nPara5Top > nShapeBottom); // ShapeBottom is higher than Para5Top

    const auto xShape2 = getShapeByName(u"logoInverted");
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape2, u"VertOrientRelation"_ustr));

    CPPUNIT_ASSERT(getProperty<bool>(xShape2, u"IsFollowingTextFlow"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153909_followTextFlow, "tdf153909_followTextFlow.docx")
{
    // given a compat12 VML document with wrap-through blue rect that doesn't mention allowInCell

    // Although MSO's UI reports "layoutInCell" for the rectangle, it isn't specified or honored
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), u"IsFollowingTextFlow"_ustr));

    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nRectBottom
        = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds", "bottom").toInt32();
    sal_Int32 nTableTop = getXPath(pDump, "//tab/row/infos/bounds", "top").toInt32();
    // The entire table must be below the rectangle
    CPPUNIT_ASSERT(nTableTop > nRectBottom);

    // pre-emptive test: rectangle "margin" offset against cell, not outside-table-paragraph.
    // Since layoutInCell is true (as a non-defined default), the cell is the fly reference, thus
    // the rectangle should start at the paper's edge, 1.3cm to the left of the start of the table.
    sal_Int32 nRectLeft
        = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds", "left").toInt32();
    sal_Int32 nTableLeft = getXPath(pDump, "//tab/row/infos/bounds", "left").toInt32();
    CPPUNIT_ASSERT(nTableLeft > nRectLeft);
}

DECLARE_OOXMLEXPORT_TEST(testTdf162541, "tdf162541_notLayoutInCell_paraLeft.docx")
{
    // given cell B2 with a para-left para-fromTop image that is NOT layoutInCell
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nShapeLeft
        = getXPath(pDump, "//tab/row[2]/cell[2]/txt[8]/anchored/fly/SwAnchoredObject/bounds",
                   "left")
              .toInt32();
    sal_Int32 nParaLeft
        = getXPath(pDump, "//tab/row[2]/cell[2]/txt[8]/infos/bounds", "left").toInt32();
    sal_Int32 nTableLeft = getXPath(pDump, "//tab/infos/bounds", "left").toInt32();
    // The image uses the table-paragraph to orient to the left (bizarre MSO layout anomaly)
    CPPUNIT_ASSERT(nShapeLeft < nParaLeft); // shape is located in column A, not column B
    CPPUNIT_ASSERT_EQUAL(nTableLeft, nShapeLeft);

    // sal_Int32 nShapeBottom
    //     = getXPath(pDump, "//tab/row[2]/cell[2]/txt[8]/anchored/fly/SwAnchoredObject/bounds",
    //                "bottom")
    //           .toInt32();
    // sal_Int32 nPara8Top
    //     = getXPath(pDump, "//tab/row[2]/cell[2]/txt[8]/infos/bounds", "top").toInt32();
    // The image uses the table-paragraph to orient to the left (bizarre MSO layout anomaly)
    // CPPUNIT_ASSERT(nShapeBottom < nPara8Top); // shape is located at the top of the table para // tdf#133522

    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), u"IsFollowingTextFlow"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf162551, "tdf162551_notLayoutInCell_charLeft_fromTop.docx")
{
    // given cell B2 with a para-fromTop, char-left image that is NOT layoutInCell
    // (but Microsoft sees the CHAR orientation and triggers a layoutInCell anyway)
    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nShapeTop
        = getXPath(pDump, "//tab/row[2]/cell[2]/txt/anchored/fly/SwAnchoredObject/bounds", "top")
              .toInt32();
    sal_Int32 nPara1Top = getXPath(pDump, "//tab/row[2]/cell[2]/txt/infos/bounds", "top").toInt32();
    // The image is limited by the cell margin
    CPPUNIT_ASSERT_EQUAL(nPara1Top, nShapeTop); // tdf#162539

    // since in fact layoutInCell is supposed to be applied, we mark (and export) as layoutInCell
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), u"IsFollowingTextFlow"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159207_footerFramePrBorder)
{
    createSwDoc("tdf159207_footerFramePrBorder.docx"); // re-imports as editeng Frame/Shape

    // given a doc with footer paragraphs frame (with a top border, but no left border)
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame0(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    auto aBorder = getProperty<table::BorderLine2>(xFrame0, u"LeftBorder"_ustr);
    sal_uInt32 nBorderWidth
        = aBorder.OuterLineWidth + aBorder.InnerLineWidth + aBorder.LineDistance;
    // Without patch it failed with Expected 0, Actual 26
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Left border:", static_cast<sal_uInt32>(0), nBorderWidth);

    // TODO: there SHOULD BE a top border, and even if loaded, it would be lost on re-import...
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160814_commentOrder)
{
    // given a document with a comment and 5 replies
    loadAndSave("tdf160814_commentOrder.docx");

    // make sure the order of the comments is imported and exported correctly
    xmlDocUniquePtr pXmlComments = parseExport(u"word/comments.xml"_ustr);
    // This really should be "1. First comment", the 1. being list numbering...
    assertXPathContent(pXmlComments, "//w:comment[1]//w:t", u"First comment");
    assertXPathContent(pXmlComments, "//w:comment[2]//w:t", u"1.1 first reply.");
    assertXPathContent(pXmlComments, "//w:comment[4]//w:t", u"1.3");
    assertXPathContent(pXmlComments, "//w:comment[6]//w:t", u"1.5");
}

CPPUNIT_TEST_FIXTURE(Test, testPersonalMetaData)
{
    // 1. Remove all personal info
    auto pBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving::set(true, pBatch);
    pBatch->commit();
    loadAndSave("personalmetadata.docx");

    xmlDocUniquePtr pAppDoc = parseExport(u"docProps/app.xml"_ustr);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:Template", 0);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:TotalTime", 0);
    xmlDocUniquePtr pCoreDoc = parseExport(u"docProps/core.xml"_ustr);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:created", 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:modified", 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/dc:creator", 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastModifiedBy", 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastPrinted", 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:revision", 0);

    // 2. Remove personal information, keep user information
    officecfg::Office::Common::Security::Scripting::KeepDocUserInfoOnSaving::set(true, pBatch);
    pBatch->commit();
    loadAndSave("personalmetadata.docx");

    pAppDoc = parseExport(u"docProps/app.xml"_ustr);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:Template", 0);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:TotalTime", 0);
    pCoreDoc = parseExport(u"docProps/core.xml"_ustr);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:created", 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:modified", 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/dc:creator", 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastModifiedBy", 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastPrinted", 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:revision", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf126533_noPageBitmap, "tdf126533_noPageBitmap.docx")
{
    // given a document with a v:background bitmap, but no w:background fillcolor
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // the image (or any fill for that matter) should be ignored.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE,
                         getProperty<drawing::FillStyle>(xPageStyle, u"FillStyle"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf126533_pageGradient, "fill.docx")
{
    // given a document with a gradient page background
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xPageStyle, u"FillStyle"_ustr));

    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xPageStyle, u"FillGradient"_ustr);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RECT, aGradient.Style);

    // LO and MS handle radials very differently, so it is hard to say what
    // color1 (fill) and color2 should be. This test is only interested in making sure
    // that the colors are not alternating back and forth on each round-trip.
    CPPUNIT_ASSERT_EQUAL(Color(0xff8200), Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(Color(0x000082), Color(ColorTransparency, aGradient.EndColor));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126533_pageBitmap)
{
    auto verify = [this]() {
        // given a document with a page background image
        uno::Reference<beans::XPropertySet> xPageStyle(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP,
                             getProperty<drawing::FillStyle>(xPageStyle, u"FillStyle"_ustr));
    };

    createSwDoc("tdf126533_pageBitmap.docx");
    verify();
    saveAndReload(mpFilter);
    verify();

    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDocRels, "/rels:Relationships/rels:Relationship[@Target='media/image1.jpeg']",
                1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf131098_imageFill, "tdf131098_imageFill.docx")
{
    // given a document with an image background transparency (blue-white)
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(getShape(1), u"FillStyle"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x729fcf), getProperty<Color>(getShape(1), u"FillColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf154369, "tdf154369.docx")
{
    //Unit test for bug fix in tdf#154369
    // Docx file contains ordered list formatted with Heading 1 style, font color set as Accent 1 from theme
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Without the fix in place, this test would have failed with:
    // - Expected result: A & B bullets display same green color #527d55 as the paragraph
    // - Actual result: A & B bullets display black color, while the paragraph is green color #527d55
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"A.");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont",
                "color", u"00527d55");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"B.");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont",
                "color", u"00527d55");
}

CPPUNIT_TEST_FIXTURE(Test, testScreenTip)
{
    loadAndSave("tdf159897.docx");

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // Hyperlink with ScreenTip
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:hyperlink", "tooltip",
                u"This is a hyperlink");
}

CPPUNIT_TEST_FIXTURE(Test, testEmptyObjectRange)
{
    // Before the fix, this failed an assertion like this:
    // Assertion failed: isBetween(n, (SAL_MIN_INT64 + d / 2) / m, (SAL_MAX_INT64 - d / 2) / m),
    // file C:\lo\core\include\o3tl/unit_conversion.hxx, line 75
    loadAndSave("cloud.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf161509)
{
    loadAndReload("special_styles.docx");
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    CPPUNIT_ASSERT(pXmlStyles);

    // Check the mapping of standard style names to their IDs
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:name", "val", u"Normal");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:name", "val", u"heading 1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading2']/w:name", "val", u"heading 2");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading3']/w:name", "val", u"heading 3");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading4']/w:name", "val", u"heading 4");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading5']/w:name", "val", u"heading 5");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading6']/w:name", "val", u"heading 6");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading7']/w:name", "val", u"heading 7");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading8']/w:name", "val", u"heading 8");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading9']/w:name", "val", u"heading 9");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index1']/w:name", "val", u"index 1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index2']/w:name", "val", u"index 2");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index3']/w:name", "val", u"index 3");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index4']/w:name", "val", u"index 4");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index5']/w:name", "val", u"index 5");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index6']/w:name", "val", u"index 6");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index7']/w:name", "val", u"index 7");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index8']/w:name", "val", u"index 8");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index9']/w:name", "val", u"index 9");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC1']/w:name", "val", u"toc 1");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC2']/w:name", "val", u"toc 2");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC3']/w:name", "val", u"toc 3");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC4']/w:name", "val", u"toc 4");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC5']/w:name", "val", u"toc 5");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC6']/w:name", "val", u"toc 6");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC7']/w:name", "val", u"toc 7");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC8']/w:name", "val", u"toc 8");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC9']/w:name", "val", u"toc 9");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NormalIndent']/w:name", "val",
                u"Normal Indent");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='FootnoteText']/w:name", "val",
                u"footnote text");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='CommentText']/w:name", "val",
                u"annotation text");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Header']/w:name", "val", u"header");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Footer']/w:name", "val", u"footer");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='IndexHeading']/w:name", "val",
                u"index heading");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Caption']/w:name", "val", u"caption");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableofFigures']/w:name", "val",
                u"table of figures");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='EnvelopeAddress']/w:name", "val",
                u"envelope address");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='EnvelopeReturn']/w:name", "val",
                u"envelope return");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='FootnoteReference']/w:name", "val",
                u"footnote reference");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='CommentReference']/w:name", "val",
                u"annotation reference");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='LineNumber']/w:name", "val",
                u"line number");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='PageNumber']/w:name", "val",
                u"page number");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='EndnoteReference']/w:name", "val",
                u"endnote reference");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='EndnoteText']/w:name", "val",
                u"endnote text");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableofAuthorities']/w:name", "val",
                u"table of authorities");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MacroText']/w:name", "val", u"macro");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOCHeading']/w:name", "val",
                u"TOC Heading");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='List']/w:name", "val", u"List");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListBullet']/w:name", "val",
                u"List Bullet");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListNumber']/w:name", "val",
                u"List Number");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='List2']/w:name", "val", u"List 2");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='List3']/w:name", "val", u"List 3");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='List4']/w:name", "val", u"List 4");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='List5']/w:name", "val", u"List 5");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListBullet2']/w:name", "val",
                u"List Bullet 2");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListBullet3']/w:name", "val",
                u"List Bullet 3");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListBullet4']/w:name", "val",
                u"List Bullet 4");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListBullet5']/w:name", "val",
                u"List Bullet 5");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListNumber2']/w:name", "val",
                u"List Number 2");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListNumber3']/w:name", "val",
                u"List Number 3");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListNumber4']/w:name", "val",
                u"List Number 4");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListNumber5']/w:name", "val",
                u"List Number 5");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Title']/w:name", "val", u"Title");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Closing']/w:name", "val", u"Closing");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Signature']/w:name", "val",
                u"Signature");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='DefaultParagraphFont']/w:name", "val",
                u"Default Paragraph Font");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyText']/w:name", "val", u"Body Text");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyTextIndent']/w:name", "val",
                u"Body Text Indent");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListContinue']/w:name", "val",
                u"List Continue");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListContinue2']/w:name", "val",
                u"List Continue 2");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListContinue3']/w:name", "val",
                u"List Continue 3");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListContinue4']/w:name", "val",
                u"List Continue 4");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListContinue5']/w:name", "val",
                u"List Continue 5");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MessageHeader']/w:name", "val",
                u"Message Header");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Subtitle']/w:name", "val", u"Subtitle");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Salutation']/w:name", "val",
                u"Salutation");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Date']/w:name", "val", u"Date");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyTextFirstIndent']/w:name", "val",
                u"Body Text First Indent");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyTextFirstIndent2']/w:name", "val",
                u"Body Text First Indent 2");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoteHeading']/w:name", "val",
                u"Note Heading");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyText2']/w:name", "val",
                u"Body Text 2");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyText3']/w:name", "val",
                u"Body Text 3");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyTextIndent2']/w:name", "val",
                u"Body Text Indent 2");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyTextIndent3']/w:name", "val",
                u"Body Text Indent 3");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BlockText']/w:name", "val",
                u"Block Text");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Hyperlink']/w:name", "val",
                u"Hyperlink");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='FollowedHyperlink']/w:name", "val",
                u"FollowedHyperlink");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Strong']/w:name", "val", u"Strong");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Emphasis']/w:name", "val", u"Emphasis");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='DocumentMap']/w:name", "val",
                u"Document Map");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='PlainText']/w:name", "val",
                u"Plain Text");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf162370)
{
    // This must not crash on save; without the fix, it would fail with
    // "Assertion failed: vector subscript out of range"
    loadAndSave("too_many_styles.odt");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128460)
{
    auto verify = [this]() {
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(
            xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);

        CPPUNIT_ASSERT_EQUAL(u""_ustr, xField->getPresentation(false));
        CPPUNIT_ASSERT_EQUAL(u"Input field"_ustr, xField->getPresentation(true));

        xField.set(xFields->nextElement(), uno::UNO_QUERY);

        CPPUNIT_ASSERT_EQUAL(u"text"_ustr, xField->getPresentation(false));
        CPPUNIT_ASSERT_EQUAL(u"Input field"_ustr, xField->getPresentation(true));
    };
    createSwDoc("tdf128460.odt");
    verify();

    saveAndReload(u"Office Open XML Text"_ustr);
    // Without the fix in place, the third ASSERT fails with
    // Expected: text
    // Actual:
    // i.e. The second Field is blank when it should have the content of `text`
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131288)
{
    // Given a document with an embedded chart
    createSwDoc("tdf131288.docx");

    // Edit the document and save **twice**
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    xText->insertString(xCursor, u"test"_ustr, false);
    save(u"Office Open XML Text"_ustr);

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(
        xNameAccess->getByName(u"word/embeddings/Microsoft_Excel-munkalap11.xlsx"_ustr),
        uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

    CPPUNIT_ASSERT_EQUAL(sal_uInt64(9041), pStream->remainingSize());

    xText->insertString(xCursor, u"more testing"_ustr, false);

    // Save again
    save(u"Office Open XML Text"_ustr);

    xNameAccess = packages::zip::ZipFileAccess::createWithURL(
        comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    xInputStream.set(
        xNameAccess->getByName(u"word/embeddings/Microsoft_Excel-munkalap11.xlsx"_ustr),
        uno::UNO_QUERY);
    pStream = utl::UcbStreamHelper::CreateStream(xInputStream, true);

    // Without the fix we get a zero length(empty) word/embeddings/Microsoft_Excel-munkalap11.xlsx
    // With the fix in place word/embeddings/Microsoft_Excel-munkalap11.xlsx contains data
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(9041), pStream->remainingSize());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf89731)
{
    // Without the fix in place this crashes on opening
    loadAndSave("tdf89731.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf61309)
{
    // Don't crash on import
    loadAndSave("tdf61309.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf162746)
{
    // Without the fix in place this hangs (and eventually OOMs) on opening
    loadAndSave("tdf162746.docx");
    // tdf#162781: test the page body table vertical offset and width
    xmlDocUniquePtr pDump = parseLayoutDump();
    // Without the fix, this would be 0 - i.e., the page body table didn't shift down
    // below the header's floating table
    assertXPath(pDump, "//page[1]/body/tab/infos/prtBounds", "top", u"35");
    // Without the fix, this would be 100, because the page body table only used tiny space
    // to the left of the header's floating table
    assertXPath(pDump, "//page[1]/body/tab/infos/prtBounds", "width", u"9360");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf61000)
{
    // Without the fix in place this crashes on opening
    loadAndSave("tdf61000.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    assertXPath(pXmlDoc,
                "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:numFmt",
                "val", u"bullet");
    // Without the fix in place, this would be -540, and the abstractNumId is 4
    // The negative value of the tab stop is the culprit for the crash
    assertXPath(
        pXmlDoc,
        "//w:numbering/w:abstractNum[@w:abstractNumId='1']/w:lvl[@w:ilvl='0']/w:pPr/w:tabs/w:tab",
        "pos", u"0");
}

CPPUNIT_TEST_FIXTURE(Test, testCommentWithChildrenTdf163092)
{
    loadAndSave("comment_with_children.odt");
    // commentsExtended should exist
    xmlDocUniquePtr pXmlCommExt = parseExport("word/commentsExtended.xml");
    CPPUNIT_ASSERT(pXmlCommExt);
    // And it should contain the same parent-child relations
    OUString sExChild1 = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx[1]", "paraId");
    OUString sExParent1 = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx[1]", "paraIdParent");
    OUString sExChild2 = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx[2]", "paraId");
    OUString sExParent2 = getXPath(pXmlCommExt, "/w15:commentsEx/w15:commentEx[2]", "paraIdParent");
    std::map<OUString, OUString> parents;
    parents[sExChild1] = sExParent1;
    parents[sExChild2] = sExParent2;
    xmlDocUniquePtr pXmlComments = parseExport("word/comments.xml");
    OUString sComment1Id = getXPath(pXmlComments, "/w:comments/w:comment[1]/w:p[1]", "paraId");
    OUString sComment2Id = getXPath(pXmlComments, "/w:comments/w:comment[2]/w:p[1]", "paraId");
    OUString sComment3Id = getXPath(pXmlComments, "/w:comments/w:comment[3]/w:p[1]", "paraId");
    CPPUNIT_ASSERT_EQUAL(parents[sComment2Id], sComment1Id);
    CPPUNIT_ASSERT_EQUAL(parents[sComment3Id], sComment2Id);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146269)
{
    // Given a document with a field in a redline:
    createSwDoc("deleted_pageref.docx");
    // It must open unmodified:
    {
        auto xModifiable = mxComponent.queryThrow<util::XModifiable>();
        CPPUNIT_ASSERT(!xModifiable->isModified());
    }

    // Test also after save-and-reload:
    saveAndReload(u"Office Open XML Text"_ustr);
    {
        auto xModifiable = mxComponent.queryThrow<util::XModifiable>();
        CPPUNIT_ASSERT(!xModifiable->isModified());
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf164176, "tdf164176.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    uno::Reference<text::XTextField> xEnumerationAccess1(xFields->nextElement(), uno::UNO_QUERY);
    rtl::OUString sPresentation = xEnumerationAccess1->getPresentation(true).trim();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), sPresentation.indexOf("_x000d_"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), sPresentation.indexOf("_x000a_"));
}

CPPUNIT_TEST_FIXTURE(Test, testMsWordUlTrailSpace)
{
    createSwDoc("UnderlineTrailingSpace.docx");
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(true),
                             xSettings->getPropertyValue(u"MsWordUlTrailSpace"_ustr));
    }

    // Test also after save-and-reload:
    saveAndReload(u"Office Open XML Text"_ustr);
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(true),
                             xSettings->getPropertyValue(u"MsWordUlTrailSpace"_ustr));
    }

    // Check that the compat setting is exported in OOXML
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:ulTrailSpace");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165059_moveFromTo)
{
    loadAndSave("tdf165059_broken.docx");
    // Without the fix, exported contains w:move* ouside of move ranges
    // Outside move range tags ins/del are valid
    xmlDocUniquePtr p_XmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(p_XmlDoc);
    assertXPath(p_XmlDoc, "//w:moveTo"_ostr, 0);
    assertXPath(p_XmlDoc, "//w:ins"_ostr, 1);
    assertXPath(p_XmlDoc, "//w:moveFrom"_ostr, 0);
    assertXPath(p_XmlDoc, "//w:del"_ostr, 1);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
