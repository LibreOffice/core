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
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>

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

DECLARE_OOXMLEXPORT_TEST(testTdf161631, "tdf161631.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // Without the fix in place, this test would have failed with
    // - Expected: Some text
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(u"Some text"_ustr, getParagraph(1)->getString());
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

DECLARE_OOXMLEXPORT_TEST(testTdf43767_caseMapNumbering, "tdf43767_caseMapNumbering.odt")
{
    // given a document with 2 numbered Lists [each entry restarts numbering for visual comparison]
    xmlDocUniquePtr pDump = parseLayoutDump();

    // using the relative width difference between "A)" and "a)" as the test comparison
    // since ListLabelString etc. does not output the actual string that is displayed on the screen

    // When the entire paragraph has a certain character attribute, that property is also applied
    // to the list numbering itself (with some differing exceptions) for both ODT and DOCX.

    // ESTABLISH A BASELINE: these baseline paragraphs have no special character attributes.
    // Paragraph 1/list 1(uppercase): no formatting applied to list numbering. Width is 253 for me
    const sal_Int32 nUpperCaseWidth
        = getXPath(pDump, "//body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                   "width"_ostr)
              .toInt32();
    // Paragraph 4/list 2(lowercase): no formatting applied to list numbering. Width is 186 for me.
    const sal_Int32 nLowerCaseWidth
        = getXPath(pDump, "//body/txt[5]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                   "width"_ostr)
              .toInt32();

    // UPPERCASE LIST
    // Paragraph 2: ODF should honour "lowercase". MSO doesn't know about lowercase
    sal_Int32 nWidth
        = getXPath(pDump, "//body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                   "width"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_EQUAL(isExported() ? nUpperCaseWidth : nLowerCaseWidth, nWidth);

    // Paragraph 3: ODF should honour "superscript" (for consistency). MSO ignores superscript
    nWidth = getXPath(pDump, "//body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                      "width"_ostr)
                 .toInt32();
    if (!isExported())
        CPPUNIT_ASSERT_LESS(nLowerCaseWidth, nWidth);
    else
        CPPUNIT_ASSERT_EQUAL(nUpperCaseWidth, nWidth);

    // LOWERCASE LIST
    //Paragraph 6: ODF should honour "titlecase". MSO doesn't know about titlecase
    nWidth = getXPath(pDump, "//body/txt[6]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                      "width"_ostr)
                 .toInt32();
    CPPUNIT_ASSERT_EQUAL(isExported() ? nLowerCaseWidth : nUpperCaseWidth, nWidth);

    // Paragraph 7: ODF should honour "smallcaps". MSO apparently has an exception for small caps
    nWidth = getXPath(pDump, "//body/txt[7]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                      "width"_ostr)
                 .toInt32();
    if (!isExported())
    {
        CPPUNIT_ASSERT_GREATER(nLowerCaseWidth, nWidth);
        CPPUNIT_ASSERT_LESS(nUpperCaseWidth, nWidth);
    }
    else
        CPPUNIT_ASSERT_EQUAL(nLowerCaseWidth, nWidth);

    // Paragraph 8: ODF should honour "uppercase". MSO also honours uppercase
    nWidth = getXPath(pDump, "//body/txt[8]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                      "width"_ostr)
                 .toInt32();
    CPPUNIT_ASSERT_EQUAL(nUpperCaseWidth, nWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf156105_percentSuffix, "tdf156105_percentSuffix.odt")
{
    // given a numbered list with a non-escaping percent symbol in the prefix and suffix
    CPPUNIT_ASSERT_EQUAL(u"(%)[%]"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ListLabelString"_ustr));

    // tdf#149258 - NONE number should not export separator since LO doesn't currently show it
    CPPUNIT_ASSERT_EQUAL_MESSAGE("showing levels 1, 2, and 4", u"(%)1.1.1[%]"_ustr,
                                 getProperty<OUString>(getParagraph(4), u"ListLabelString"_ustr));
    if (isExported())
    {
        xmlDocUniquePtr pXmlNum = parseExport(u"word/numbering.xml"_ustr);
        // The 3rd level is NONE. If we include the separator, MS Word will display it.
        assertXPath(pXmlNum, "/w:numbering/w:abstractNum[1]/w:lvl[4]/w:lvlText"_ostr, "val"_ostr,
                    u"(%)%1.%2.%3%4[%]"_ustr);
    }
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

    // xmlDocUniquePtr pDump = parseLayoutDump();
    // const sal_Int32 nCellTop
    //     = getXPath(pDump, "//row[1]/cell[1]/infos/bounds"_ostr, "top"_ostr).toInt32();
    // const sal_Int32 nParaTop
    //     = getXPath(pDump, "//row[1]/cell[1]/txt/infos/bounds"_ostr, "top"_ostr).toInt32();
    // const sal_Int32 nImageTop
    //     = getXPath(pDump, "//row[1]/cell[1]/txt/anchored/SwAnchoredDrawObject/bounds"_ostr,
    //                 "top"_ostr)
    //             .toInt32();
    // // The image is approximately half-way between cell top and the start of the text
    // // correct ImageTop: 3588, while incorrect value was 1117. Cell top is 3051, ParaTop is 4195
    // const sal_Int32 nHalfway = nCellTop + (nParaTop - nCellTop) / 2;
    // CPPUNIT_ASSERT_DOUBLES_EQUAL(nHalfway, nImageTop, 50); // +/- 4.4%

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

    // xmlDocUniquePtr pDump = parseLayoutDump();
    // const sal_Int32 nShapeTop
    //     = getXPath(pDump,
    //                 "//body/tab[1]/row[1]/cell[1]/txt[1]/anchored/SwAnchoredDrawObject/bounds"_ostr,
    //                 "top"_ostr)
    //             .toInt32();
    // // The shape is approximately 1 cm below the top of the page, and ~0.5cm above the cell
    // // correct ShapeTop: 888 TWIPS, while incorrect value was -480. Cell top is 1148, PageTop is 284
    // CPPUNIT_ASSERT_DOUBLES_EQUAL(888, nShapeTop, 50);

    const auto& xShape = getShapeByName(u"Group 1");
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
        = getXPath(pDump, "//row[1]/cell[2]/txt[1]/infos/bounds"_ostr, "top"_ostr).toInt32();
    const sal_Int32 nPara1Bottom
        = getXPath(pDump, "//row[1]/cell[2]/txt[1]/infos/bounds"_ostr, "bottom"_ostr).toInt32();
    const sal_Int32 nImageTop
        = getXPath(pDump, "//row[1]/cell[2]/txt[5]/anchored/SwAnchoredDrawObject/bounds"_ostr,
                   "top"_ostr)
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
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[1]/anchored/fly/SwAnchoredObject/bounds"_ostr,
                   "top"_ostr)
              .toInt32();
    sal_Int32 nShapeBottom
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[1]/anchored/fly/SwAnchoredObject/bounds"_ostr,
                   "bottom"_ostr)
              .toInt32();
    // use paragraph 1 to indicate where the cell spacing/padding ends, and the text starts.
    sal_Int32 nPara1Top
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[1]/infos/bounds"_ostr, "top"_ostr).toInt32();
    // use paragraph 5 to prove the image is not at the bottom.
    CPPUNIT_ASSERT_EQUAL(OUString("Below logo"),
                         getXPathContent(pDump, "//tab/row[1]/cell[1]/txt[5]"_ostr));
    sal_Int32 nPara5Top
        = getXPath(pDump, "//tab/row[1]/cell[1]/txt[5]/infos/bounds"_ostr, "top"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(nShapeTop, nPara1Top);
    CPPUNIT_ASSERT(nPara5Top > nShapeBottom); // ShapeBottom is higher than Para5Top

    // In the file it is specified as "page" (PAGE_FRAME), but implemented as if it were "margin"
    // so on import we intentionally changed it to match the closest setting to the implementation.
    const auto& xShape = getShapeByName(u"logo");
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));

    CPPUNIT_ASSERT(getProperty<bool>(xShape, u"IsFollowingTextFlow"_ustr));

    // Cell B1
    nShapeTop
        = getXPath(pDump, "//tab/row[1]/cell[2]/txt[1]/anchored/fly/SwAnchoredObject/bounds"_ostr,
                   "top"_ostr)
              .toInt32();
    nShapeBottom
        = getXPath(pDump, "//tab/row[1]/cell[2]/txt[1]/anchored/fly/SwAnchoredObject/bounds"_ostr,
                   "bottom"_ostr)
              .toInt32();
    // use paragraph 1 to indicate where the cell spacing/padding ends, and the text starts.
    nPara1Top
        = getXPath(pDump, "//tab/row[1]/cell[2]/txt[1]/infos/bounds"_ostr, "top"_ostr).toInt32();
    // use paragraph 5 to prove the image is not at the bottom.
    CPPUNIT_ASSERT_EQUAL(OUString("Below image"),
                         getXPathContent(pDump, "//tab/row[1]/cell[2]/txt[5]"_ostr));
    nPara5Top
        = getXPath(pDump, "//tab[1]/row/cell[2]/txt[5]/infos/bounds"_ostr, "top"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(nShapeTop, nPara1Top);
    CPPUNIT_ASSERT(nPara5Top > nShapeBottom); // ShapeBottom is higher than Para5Top

    const auto& xShape2 = getShapeByName(u"logoInverted");
    CPPUNIT_ASSERT_EQUAL(css::text::RelOrientation::PAGE_PRINT_AREA,
                         getProperty<sal_Int16>(xShape2, u"VertOrientRelation"_ustr));

    CPPUNIT_ASSERT(getProperty<bool>(xShape2, u"IsFollowingTextFlow"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf153909_followTextFlow, "tdf153909_followTextFlow.docx")
{
    // given a compat12 VML document with wrap-through blue rect that doesn't mention allowInCell

    // Although MSO's UI reports "layoutInCell" for the rectangle, it isn't specified or honored
    CPPUNIT_ASSERT_EQUAL(isExported(), getProperty<bool>(getShape(1), u"IsFollowingTextFlow"_ustr));

    xmlDocUniquePtr pDump = parseLayoutDump();
    sal_Int32 nRectBottom
        = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "bottom"_ostr).toInt32();
    sal_Int32 nTableTop = getXPath(pDump, "//tab/row/infos/bounds"_ostr, "top"_ostr).toInt32();
    // The entire table must be below the rectangle
    CPPUNIT_ASSERT(nTableTop > nRectBottom);

    // pre-emptive test: rectangle "margin" offset against cell, not outside-table-paragraph.
    // Since layoutInCell is true (as a non-defined default), the cell is the fly reference, thus
    // the rectangle should start at the paper's edge, 1.3cm to the left of the start of the table.
    sal_Int32 nRectLeft
        = getXPath(pDump, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "left"_ostr).toInt32();
    sal_Int32 nTableLeft = getXPath(pDump, "//tab/row/infos/bounds"_ostr, "left"_ostr).toInt32();
    CPPUNIT_ASSERT(nTableLeft > nRectLeft);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159207_footerFramePrBorder)
{
    loadFromFile(u"tdf159207_footerFramePrBorder.docx"); // re-imports as editeng Frame/Shape

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
    assertXPathContent(pXmlComments, "//w:comment[1]//w:t"_ostr, u"First comment"_ustr);
    assertXPathContent(pXmlComments, "//w:comment[2]//w:t"_ostr, u"1.1 first reply."_ustr);
    assertXPathContent(pXmlComments, "//w:comment[4]//w:t"_ostr, u"1.3"_ustr);
    assertXPathContent(pXmlComments, "//w:comment[6]//w:t"_ostr, u"1.5"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testPersonalMetaData)
{
    // 1. Remove all personal info
    auto pBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving::set(true, pBatch);
    pBatch->commit();
    loadAndSave("personalmetadata.docx");

    xmlDocUniquePtr pAppDoc = parseExport(u"docProps/app.xml"_ustr);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:Template"_ostr, 0);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:TotalTime"_ostr, 0);
    xmlDocUniquePtr pCoreDoc = parseExport(u"docProps/core.xml"_ustr);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:created"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:modified"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/dc:creator"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastModifiedBy"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastPrinted"_ostr, 0);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:revision"_ostr, 0);

    // 2. Remove personal information, keep user information
    officecfg::Office::Common::Security::Scripting::KeepDocUserInfoOnSaving::set(true, pBatch);
    pBatch->commit();
    loadAndSave("personalmetadata.docx");

    pAppDoc = parseExport(u"docProps/app.xml"_ustr);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:Template"_ostr, 0);
    assertXPath(pAppDoc, "/extended-properties:Properties/extended-properties:TotalTime"_ostr, 0);
    pCoreDoc = parseExport(u"docProps/core.xml"_ustr);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:created"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/dcterms:modified"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/dc:creator"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastModifiedBy"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:lastPrinted"_ostr, 1);
    assertXPath(pCoreDoc, "/cp:coreProperties/cp:revision"_ostr, 0);
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
}

DECLARE_OOXMLEXPORT_TEST(testTdf126533_pageBitmap, "tdf126533_pageBitmap.docx")
{
    // given a document with a page background image
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP,
                         getProperty<drawing::FillStyle>(xPageStyle, u"FillStyle"_ustr));

    if (!isExported())
        return;

    xmlDocUniquePtr pXmlDocRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDocRels,
                "/rels:Relationships/rels:Relationship[@Target='media/image1.jpeg']"_ostr, 1);
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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                "expand"_ostr, u"A."_ustr);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
                "color"_ostr, u"00527d55"_ustr);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
                "expand"_ostr, u"B."_ustr);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
                "color"_ostr, u"00527d55"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testScreenTip)
{
    loadAndSave("tdf159897.docx");

    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);

    // Hyperlink with ScreenTip
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:hyperlink"_ostr, "tooltip"_ostr,
                u"This is a hyperlink"_ustr);
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
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Normal']/w:name"_ostr, "val"_ostr,
                u"Normal"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading1']/w:name"_ostr, "val"_ostr,
                u"heading 1"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading2']/w:name"_ostr, "val"_ostr,
                u"heading 2"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading3']/w:name"_ostr, "val"_ostr,
                u"heading 3"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading4']/w:name"_ostr, "val"_ostr,
                u"heading 4"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading5']/w:name"_ostr, "val"_ostr,
                u"heading 5"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading6']/w:name"_ostr, "val"_ostr,
                u"heading 6"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading7']/w:name"_ostr, "val"_ostr,
                u"heading 7"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading8']/w:name"_ostr, "val"_ostr,
                u"heading 8"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Heading9']/w:name"_ostr, "val"_ostr,
                u"heading 9"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index1']/w:name"_ostr, "val"_ostr,
                u"index 1"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index2']/w:name"_ostr, "val"_ostr,
                u"index 2"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index3']/w:name"_ostr, "val"_ostr,
                u"index 3"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index4']/w:name"_ostr, "val"_ostr,
                u"index 4"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index5']/w:name"_ostr, "val"_ostr,
                u"index 5"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index6']/w:name"_ostr, "val"_ostr,
                u"index 6"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index7']/w:name"_ostr, "val"_ostr,
                u"index 7"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index8']/w:name"_ostr, "val"_ostr,
                u"index 8"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Index9']/w:name"_ostr, "val"_ostr,
                u"index 9"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC1']/w:name"_ostr, "val"_ostr,
                u"toc 1"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC2']/w:name"_ostr, "val"_ostr,
                u"toc 2"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC3']/w:name"_ostr, "val"_ostr,
                u"toc 3"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC4']/w:name"_ostr, "val"_ostr,
                u"toc 4"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC5']/w:name"_ostr, "val"_ostr,
                u"toc 5"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC6']/w:name"_ostr, "val"_ostr,
                u"toc 6"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC7']/w:name"_ostr, "val"_ostr,
                u"toc 7"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC8']/w:name"_ostr, "val"_ostr,
                u"toc 8"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOC9']/w:name"_ostr, "val"_ostr,
                u"toc 9"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NormalIndent']/w:name"_ostr, "val"_ostr,
                u"Normal Indent"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='FootnoteText']/w:name"_ostr, "val"_ostr,
                u"footnote text"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='CommentText']/w:name"_ostr, "val"_ostr,
                u"annotation text"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Header']/w:name"_ostr, "val"_ostr,
                u"header"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Footer']/w:name"_ostr, "val"_ostr,
                u"footer"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='IndexHeading']/w:name"_ostr, "val"_ostr,
                u"index heading"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Caption']/w:name"_ostr, "val"_ostr,
                u"caption"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableofFigures']/w:name"_ostr,
                "val"_ostr, u"table of figures"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='EnvelopeAddress']/w:name"_ostr,
                "val"_ostr, u"envelope address"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='EnvelopeReturn']/w:name"_ostr,
                "val"_ostr, u"envelope return"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='FootnoteReference']/w:name"_ostr,
                "val"_ostr, u"footnote reference"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='CommentReference']/w:name"_ostr,
                "val"_ostr, u"annotation reference"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='LineNumber']/w:name"_ostr, "val"_ostr,
                u"line number"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='PageNumber']/w:name"_ostr, "val"_ostr,
                u"page number"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='EndnoteReference']/w:name"_ostr,
                "val"_ostr, u"endnote reference"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='EndnoteText']/w:name"_ostr, "val"_ostr,
                u"endnote text"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TableofAuthorities']/w:name"_ostr,
                "val"_ostr, u"table of authorities"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MacroText']/w:name"_ostr, "val"_ostr,
                u"macro"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='TOCHeading']/w:name"_ostr, "val"_ostr,
                u"TOC Heading"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='List']/w:name"_ostr, "val"_ostr,
                u"List"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListBullet']/w:name"_ostr, "val"_ostr,
                u"List Bullet"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListNumber']/w:name"_ostr, "val"_ostr,
                u"List Number"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='List2']/w:name"_ostr, "val"_ostr,
                u"List 2"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='List3']/w:name"_ostr, "val"_ostr,
                u"List 3"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='List4']/w:name"_ostr, "val"_ostr,
                u"List 4"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='List5']/w:name"_ostr, "val"_ostr,
                u"List 5"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListBullet2']/w:name"_ostr, "val"_ostr,
                u"List Bullet 2"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListBullet3']/w:name"_ostr, "val"_ostr,
                u"List Bullet 3"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListBullet4']/w:name"_ostr, "val"_ostr,
                u"List Bullet 4"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListBullet5']/w:name"_ostr, "val"_ostr,
                u"List Bullet 5"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListNumber2']/w:name"_ostr, "val"_ostr,
                u"List Number 2"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListNumber3']/w:name"_ostr, "val"_ostr,
                u"List Number 3"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListNumber4']/w:name"_ostr, "val"_ostr,
                u"List Number 4"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListNumber5']/w:name"_ostr, "val"_ostr,
                u"List Number 5"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Title']/w:name"_ostr, "val"_ostr,
                u"Title"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Closing']/w:name"_ostr, "val"_ostr,
                u"Closing"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Signature']/w:name"_ostr, "val"_ostr,
                u"Signature"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='DefaultParagraphFont']/w:name"_ostr,
                "val"_ostr, u"Default Paragraph Font"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyText']/w:name"_ostr, "val"_ostr,
                u"Body Text"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyTextIndent']/w:name"_ostr,
                "val"_ostr, u"Body Text Indent"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListContinue']/w:name"_ostr, "val"_ostr,
                u"List Continue"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListContinue2']/w:name"_ostr, "val"_ostr,
                u"List Continue 2"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListContinue3']/w:name"_ostr, "val"_ostr,
                u"List Continue 3"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListContinue4']/w:name"_ostr, "val"_ostr,
                u"List Continue 4"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='ListContinue5']/w:name"_ostr, "val"_ostr,
                u"List Continue 5"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='MessageHeader']/w:name"_ostr, "val"_ostr,
                u"Message Header"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Subtitle']/w:name"_ostr, "val"_ostr,
                u"Subtitle"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Salutation']/w:name"_ostr, "val"_ostr,
                u"Salutation"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Date']/w:name"_ostr, "val"_ostr,
                u"Date"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyTextFirstIndent']/w:name"_ostr,
                "val"_ostr, u"Body Text First Indent"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyTextFirstIndent2']/w:name"_ostr,
                "val"_ostr, u"Body Text First Indent 2"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NoteHeading']/w:name"_ostr, "val"_ostr,
                u"Note Heading"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyText2']/w:name"_ostr, "val"_ostr,
                u"Body Text 2"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyText3']/w:name"_ostr, "val"_ostr,
                u"Body Text 3"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyTextIndent2']/w:name"_ostr,
                "val"_ostr, u"Body Text Indent 2"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BodyTextIndent3']/w:name"_ostr,
                "val"_ostr, u"Body Text Indent 3"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='BlockText']/w:name"_ostr, "val"_ostr,
                u"Block Text"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Hyperlink']/w:name"_ostr, "val"_ostr,
                u"Hyperlink"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='FollowedHyperlink']/w:name"_ostr,
                "val"_ostr, u"FollowedHyperlink"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Strong']/w:name"_ostr, "val"_ostr,
                u"Strong"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Emphasis']/w:name"_ostr, "val"_ostr,
                u"Emphasis"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='DocumentMap']/w:name"_ostr, "val"_ostr,
                u"Document Map"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='PlainText']/w:name"_ostr, "val"_ostr,
                u"Plain Text"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf162370)
{
    // This must not crash on save; without the fix, it would fail with
    // "Assertion failed: vector subscript out of range"
    loadAndSave("too_many_styles.odt");
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
