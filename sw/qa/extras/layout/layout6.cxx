/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <vcl/event.hxx>
#include <vcl/scheduler.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/unolingu.hxx>
#include <comphelper/sequence.hxx>
#include <test/commontesttools.hxx>

#include <anchoredobject.hxx>
#include <fmtfsize.hxx>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <bodyfrm.hxx>
#include <sortedobjs.hxx>
#include <ndtxt.hxx>
#include <frmatr.hxx>
#include <IDocumentSettingAccess.hxx>
#include <rootfrm.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <svx/svdpage.hxx>

#include <officecfg/Office/Common.hxx>

namespace
{
/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter6 : public SwModelTestBase
{
public:
    SwLayoutWriter6()
        : SwModelTestBase(u"/sw/qa/extras/layout/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf165354_long_paragraph_3)
{
    // disabled hyphenation on page 1, enabled on page 2
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_long_paragraph_3.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 3-page paragraph, loext:hyphenation-keep-line="true"
    // This started with "tially" (not disabled hyphenation, because of
    // the first hyphenated line on the third page)
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"inertially. Even just one ");

    // not disabled hyphenation by loext:hyphenation-keep-type="spread"
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/SwParaPortion/SwLineLayout[12]", "portion",
                u"of the Earth is space ex");
}

// FIXME The test passes on most machines. Need to figure out what goes wrong and fix that
// (at least, replacing Times New Roman with the metric-equivalent Liberation).
#if 0
CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf165984)
{
    // enabled hyphenation on page 1, disabled on page 2 by hyphenation-zone-page
    // (no hyphenation, if the word part is completely inside the Page end zone:
    // "iner-tially", but "except" and not "ex-cept")
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165984.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // paragraph with loext:hyphenation-zone-paragraph="7.62cm"
    // This was "tially. Even" (now disabled hyphenation in the last full paragraph line)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout[6]", "portion",
                u"inertially. Even");

    // 3-page paragraph, loext:hyphenation-zone-page="5.08cm"
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"tially. Even just one ");

    // disabled hyphenation by hyphenation-zone-page="5.08cm"
    // This ended with "ex-"
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/SwParaPortion/SwLineLayout[12]", "portion",
                u"of the Earth is space ");
}
#endif

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf126154)
{
    // minimum, desired and maximum word spacing
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf126154.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // only desired word space: 100%, 100%, 100%
    // 5 lines are hyphenated
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout[1]", "portion",
        u",, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bulum ");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti ");

    // also minimum word space: 80%, 100%, 100%
    // only a single line was hyphenated from the previous ones
    // TODO: fix possible interoperability issues, allow optional limitation of hyphenation again
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]", "portion",
        u",, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bulum ");
    // This was "... bulum c" (more shrinking, than needed to remove hyphenation)
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bulum ");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bulum ");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bu");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[14]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[15]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[16]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti ");

    // minimum, desired and maximum word spacing: 80%, 100%, 133%
    // no hyphenation in the same text: hyphenation of all the short words were limited
    // by the minimum and maximum word spacing settings
    // TODO: fix possible interoperability issues, allow optional limitation of hyphenation again
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]", "portion",
        u",, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bulum ");
    // This was "... bulum c" (more shrinking, than needed to remove hyphenation)
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[19]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bulum ");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[20]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bulum ");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[21]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bu");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[22]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[23]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti ");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[24]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf106234)
{
    createSwDoc("tdf106234.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // In justified paragraphs, there is justification between left tabulators and manual line breaks
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwGluePortion",
                "type", u"PortionType::Margin");
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwGluePortion",
                "width", u"0");
    // but not after centered, right and decimal tabulators
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwGluePortion",
                "type", u"PortionType::Margin");
    // This was a justified line, without width
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwGluePortion",
                "width", u"7882");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf126154_minimum_shrinking)
{
    // minimum, desired and maximum word spacing
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf126154_minimum_shrinking.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // only desired word space: 100%, 100%, 100%
    // 5 lines are hyphenated
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout[1]", "portion",
        u",, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bulum ");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti ");

    // also minimum word space: 80%, 100%, 100%
    // only a single line was hyphenated from the previous ones
    // TODO: fix possible interoperability issues, allow optional limitation of hyphenation again
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]", "portion",
        u",, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bulum ");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bulum ");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bulum ");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bu");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[14]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[15]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[16]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti ");

    // minimum, desired and maximum word spacing: 80%, 100%, 133%
    // no hyphenation in the same text: hyphenation of all the short words were limited
    // by the minimum and maximum word spacing settings
    // TODO: fix possible interoperability issues, allow optional limitation of hyphenation again
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]", "portion",
        u",, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bulum ");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[19]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti bulum ");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[20]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bulum ");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[21]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bu");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[22]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
                u"Vesti bu");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[23]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti ");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[24]/SwParaPortion/SwLineLayout[1]", "portion",
        u",,,,,,,, , , , , , , , Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vesti ");
}

// FIXME Need to figure out what goes wrong and fix that
// (at least, replacing Times New Roman with the metric-equivalent Liberation).
#if 0
CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf126154_portion)
{
    // text portions with word spacing, paragraph end zone and hyphenation zone
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf126154_portion.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // only maximum word spacing: 133%
    // This was "... Vesti bu" (not disabled hyphenation because of text portion)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,,,,,,,,,,,,,,,,,,, , , , , , , , , , , , , , , , Lorem ipsum dolor sit amet, "
                u"consectetur adipiscing elit. Vesti ");

    // minimum: 80%, desired: 100%, maximum word spacing: 133%
    // prefer maximum word spacing, not minimum word spacing, if the weighted word space
    // is nearer to the desired word space
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]", "portion",
                u",,,,,,,,,,,,,,,,,,,,, , , , , , , , , , , , , , , , Lorem ipsum dolor sit amet, "
                u"consectetur adipiscing elit. Vesti ");

    // paragraph end zone
    // This was "... other celes" (not disabled hyphenation because of text portion)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[8]", "portion",
                u"cally atmospherically atmospherically atmospherically. The Earth is no different "
                u"to any other ");

    // hyphenation
    // This was "... other celes" (not disabled hyphenation because of text portion)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[8]", "portion",
                u"cally atmospherically atmospherically atmospherically. The Earth is no different "
                u"to any other ");
}
#endif

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf155324)
{
    createSwDoc("tox-update-wrong-pages.odt");

    dispatchCommand(mxComponent, u".uno:UpdateAllIndexes"_ustr, {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // the problem was that the first entry was on page 7, 2nd on page 9 etc.
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"Foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"5");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"7");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"Three");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"7");

    // check first content page has the footnotes
    assertXPath(pXmlDoc, "/root/page[5]/body/txt[1]/SwParaPortion/SwLineLayout", "portion", u"Foo");
    assertXPath(pXmlDoc, "/root/page[4]/ftncont", 0);
    assertXPath(pXmlDoc, "/root/page[5]/ftncont/ftn", 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf120287b)
{
    createSwDoc("tdf120287b.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 1418, TabOverMargin did the right split of the paragraph to two
    // lines, but then calculated a too large tab portion size on the first
    // line.
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabRight']",
        "width", u"1");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf120287c)
{
    createSwDoc("tdf120287c.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 3, the second line was broken into a 2nd and a 3rd one,
    // not rendering text outside the paragraph frame like Word 2013 does.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf158658a)
{
    createSwDoc("tdf158658a.rtf");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Word 2013 puts all tabs into one line, the last 8 of them are off the page
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pXmlDoc,
                "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::TabCenter']",
                1);
    assertXPath(pXmlDoc,
                "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::TabRight']",
                1);
    assertXPath(pXmlDoc,
                "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::TabLeft']",
                9);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf158658b)
{
    createSwDoc("tdf158658b.rtf");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Word 2013 puts all tabs and the field following into one line
    // and also puts the field off the page
    assertXPath(pXmlDoc, "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pXmlDoc,
                "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::TabCenter']",
                1);
    assertXPath(pXmlDoc,
                "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::TabRight']",
                1);
    assertXPath(pXmlDoc,
                "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::TabRight']",
                "width", u"4446"); // was very small: 24
    assertXPath(pXmlDoc,
                "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::TabLeft']",
                0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf158658c)
{
    createSwDoc("tdf158658c.rtf");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Word 2013 puts all tabs into one line, the last 17 of them are off the page
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pXmlDoc,
                "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::TabCenter']",
                1);
    // the right tab is exactly at the margin of the paragraph
    assertXPath(pXmlDoc,
                "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::TabRight']",
                1);
    assertXPath(pXmlDoc,
                "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::TabLeft']",
                20);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf155177)
{
    createSwDoc("tdf155177-1-min.odt");

    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Text body"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(210), getProperty<sal_Int32>(xStyle, u"ParaTopMargin"_ustr));

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[2]/body/txt", 6);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[6]/SwParaPortion/SwLineLayout", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[6]/SwParaPortion/SwLineLayout[2]", "portion",
                    u"long as two lines.");
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout", 3);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                    u"This paragraph is even longer so that ");
    }

    // this should bring one line back
    xStyle->setPropertyValue(u"ParaTopMargin"_ustr, uno::Any(sal_Int32(200)));

    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[2]/body/txt", 7);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[7]/SwParaPortion/SwLineLayout", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[7]/SwParaPortion/SwLineLayout[1]", "portion",
                    u"This paragraph is even longer so that ");
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout", 2);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                    u"it is now three lines long though ");
    }

    // this should bring second line back
    xStyle->setPropertyValue(u"ParaTopMargin"_ustr, uno::Any(sal_Int32(120)));

    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[2]/body/txt", 7);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[7]/SwParaPortion/SwLineLayout", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[7]/SwParaPortion/SwLineLayout[1]", "portion",
                    u"This paragraph is even longer so that ");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[7]/SwParaPortion/SwLineLayout[2]", "portion",
                    u"it is now three lines long though ");
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                    u"containing a single sentence.");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf122878)
{
    createSwDoc("tdf122878.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    const sal_Int32 nTblTop
        = getXPath(pXmlDoc, "/root/page[1]/footer/txt/anchored/fly/tab/infos/bounds", "top")
              .toInt32();
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    SwFrame* pBody = pPage1->FindBodyCont();
    for (SwFrame* pFrame = pBody->GetLower(); pFrame; pFrame = pFrame->GetNext())
    {
        const sal_Int32 nTxtBottom = pFrame->getFrameArea().Bottom();
        // No body paragraphs should overlap the table in the footer
        CPPUNIT_ASSERT_MESSAGE(
            OString("testing paragraph #" + OString::number(pFrame->GetFrameId())).getStr(),
            nTxtBottom <= nTblTop);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf115094)
{
    createSwDoc("tdf115094.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    sal_Int32 nTopOfD1
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/tab/row[1]/cell[4]/infos/bounds",
                   "top")
              .toInt32();
    sal_Int32 nTopOfD1Anchored = getXPath(pXmlDoc,
                                          "/root/page/body/txt/anchored/fly/tab/row[1]/cell[4]/"
                                          "txt[2]/anchored/fly/infos/bounds",
                                          "top")
                                     .toInt32();
    CPPUNIT_ASSERT_LESS(nTopOfD1Anchored, nTopOfD1);
    sal_Int32 nTopOfB2
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/tab/row[2]/cell[2]/infos/bounds",
                   "top")
              .toInt32();
    sal_Int32 nTopOfB2Anchored = getXPath(pXmlDoc,
                                          "/root/page/body/txt/anchored/fly/tab/row[2]/cell[2]/"
                                          "txt[1]/anchored/fly/infos/bounds",
                                          "top")
                                     .toInt32();
    CPPUNIT_ASSERT_LESS(nTopOfB2Anchored, nTopOfB2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf112290)
{
    createSwDoc("tdf112290.docx");
    auto pXml = parseLayoutDump();
    assertXPath(pXml, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion", u"Xxxx Xxxx");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testKeepWithNextPlusFlyFollowTextFlow)
{
    createSwDoc("keep-with-next-fly.fodt");

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // 3 text frames on page 1
        assertXPath(pXmlDoc, "/root/page[1]/body/infos/bounds", "bottom", u"7540");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "height", u"276");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/bounds", "height", u"276");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly/infos/bounds", "top", u"1694");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/infos/bounds", "height", u"276");
        assertXPath(pXmlDoc, "/root/page", 1);
    }

    // disable Field Names warning dialog
    ScopedConfigValue<officecfg::Office::Common::Misc::QueryShowFieldName> aCfg(false);

    dispatchCommand(mxComponent, u".uno:Fieldnames"_ustr, {});

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // 1 text frame on page 1, and some empty space
        assertXPath(pXmlDoc, "/root/page[1]/body/infos/bounds", "bottom", u"7540");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "height", u"5796");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "bottom", u"7213");
        // 2 text frames on page 2
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/infos/bounds", "height", u"276");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly/infos/bounds", "top",
                    u"10093");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/infos/bounds", "height", u"276");
        assertXPath(pXmlDoc, "/root/page", 2);
    }

    dispatchCommand(mxComponent, u".uno:Fieldnames"_ustr, {});

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // 3 text frames on page 1
        assertXPath(pXmlDoc, "/root/page[1]/body/infos/bounds", "bottom", u"7540");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "height", u"276");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/bounds", "height", u"276");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly/infos/bounds", "top", u"1694");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/infos/bounds", "height", u"276");
        assertXPath(pXmlDoc, "/root/page", 1);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf122607)
{
    createSwDoc("tdf122607.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/SwParaPortion/SwLineLayout/child::*[1]",
                "height", u"253");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/SwParaPortion/SwLineLayout/child::*[1]",
                "width", u"427");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", u"Fax:");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf122607_regression)
{
    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue(u"Hidden"_ustr, -1, uno::Any(true),
                               beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    createSwDoc("tdf122607_leerzeile.odt", comphelper::containerToSequence(aFilterOptions));
    save(TestFilter::PDF_WRITER);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // somehow these 2 rows overlapped in the PDF unless CalcLayout() runs
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds", "mbFixSize",
                u"false");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds", "top", u"2977");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds", "height", u"241");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds", "mbFixSize",
                u"true");
    // this was 3034, causing the overlap
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds", "top", u"3218");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds", "height", u"164");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, TestTdf150616)
{
    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue(u"Hidden"_ustr, -1, uno::Any(true),
                               beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    createSwDoc("in_056132_mod.odt", comphelper::containerToSequence(aFilterOptions));
    save(TestFilter::PDF_WRITER);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // this one was 0 height
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[2]/SwParaPortion/SwLineLayout",
                "portion", u"Important information here!");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[2]/infos/bounds", "height",
                u"253");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[2]/infos/bounds", "top",
                u"7925");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[3]/SwParaPortion/SwLineLayout",
                "portion", u"xxx 111 ");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[3]/infos/bounds", "height",
                u"697");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[3]/infos/bounds", "top",
                u"8178");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testBtlrCell)
{
    createSwDoc("btlr-cell.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed, as
    // the orientation was 0 (layout did not take btlr direction request from
    // doc model).
    assertXPath(pXmlDoc, "//font[1]", "orientation", u"900");

#if !defined(MACOSX) && !defined(_WIN32) // macOS fails with x == 2662 for some reason.
    // Without the accompanying fix in place, this test would have failed with 'Expected: 1915;
    // Actual  : 1756', i.e. the AAA1 text was too close to the left cell border due to an ascent vs
    // descent mismatch when calculating the baseline offset of the text portion.
    assertXPath(pXmlDoc, "//textarray[1]", "x", u"1915");
    assertXPath(pXmlDoc, "//textarray[1]", "y", u"2707");

    // Without the accompanying fix in place, this test would have failed with 'Expected: 1979;
    // Actual  : 2129', i.e. the gray background of the "AAA2." text was too close to the right edge
    // of the text portion. Now it's exactly behind the text portion.
    assertXPath(pXmlDoc, "(//rect)[2]", "left", u"1979");

    // Without the accompanying fix in place, this test would have failed with 'Expected: 269;
    // Actual  : 0', i.e. the AAA2 frame was not visible due to 0 width.
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds", "width", u"269");

    // Test the position of the cursor after doc load.
    // We expect that it's inside the first text frame in the first cell.
    // More precisely, this is a bottom to top vertical frame, so we expect it's at the start, which
    // means it's at the lower half of the text frame rectangle (vertically).
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    const SwRect& rCharRect = pWrtShell->GetCharRect();
    SwTwips nFirstParaTop
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[1]/infos/bounds", "top").toInt32();
    SwTwips nFirstParaHeight
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[1]/infos/bounds", "height")
              .toInt32();
    SwTwips nFirstParaMiddle = nFirstParaTop + nFirstParaHeight / 2;
    SwTwips nFirstParaBottom = nFirstParaTop + nFirstParaHeight;
    // Without the accompanying fix in place, this test would have failed: the lower half (vertical)
    // range was 2273 -> 2835, the good vertical position is 2730, the bad one was 1830.
    CPPUNIT_ASSERT_GREATER(nFirstParaMiddle, rCharRect.Top());
    CPPUNIT_ASSERT_LESS(nFirstParaBottom, rCharRect.Top());

    // Save initial cursor position.
    SwPosition aCellStart = *pWrtShell->GetCursor()->Start();

    // Test that pressing "up" at the start of the cell goes to the next character position.
    SwNodeOffset nNodeIndex = pWrtShell->GetCursor()->Start()->GetNodeIndex();
    sal_Int32 nIndex = pWrtShell->GetCursor()->Start()->GetContentIndex();
    KeyEvent aKeyEvent(0, KEY_UP);
    SwEditWin& rEditWin = pShell->GetView()->GetEditWin();
    rEditWin.KeyInput(aKeyEvent);
    Scheduler::ProcessEventsToIdle();
    // Without the accompanying fix in place, this test would have failed: "up" was interpreted as
    // logical "left", which does nothing if you're at the start of the text anyway.
    CPPUNIT_ASSERT_EQUAL(nIndex + 1, pWrtShell->GetCursor()->Start()->GetContentIndex());

    // Test that pressing "right" goes to the next paragraph (logical "down").
    sal_Int32 nContentIndex = pWrtShell->GetCursor()->Start()->GetContentIndex();
    aKeyEvent = KeyEvent(0, KEY_RIGHT);
    rEditWin.KeyInput(aKeyEvent);
    Scheduler::ProcessEventsToIdle();
    // Without the accompanying fix in place, this test would have failed: the cursor went to the
    // paragraph after the table.
    CPPUNIT_ASSERT_EQUAL(nNodeIndex + 1, pWrtShell->GetCursor()->Start()->GetNodeIndex());

    // Test that we have the correct character index after traveling to the next paragraph.
    // Without the accompanying fix in place, this test would have failed: char position was 5, i.e.
    // the cursor jumped to the end of the paragraph for no reason.
    CPPUNIT_ASSERT_EQUAL(nContentIndex, pWrtShell->GetCursor()->Start()->GetContentIndex());

    // Test that clicking "below" the second paragraph positions the cursor at the start of the
    // second paragraph.
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwPosition aPosition(aCellStart);
    SwTwips nSecondParaLeft
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds", "left")
              .toInt32();
    SwTwips nSecondParaWidth
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds", "width")
              .toInt32();
    SwTwips nSecondParaTop
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds", "top").toInt32();
    SwTwips nSecondParaHeight
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds", "height")
              .toInt32();
    Point aPoint;
    aPoint.setX(nSecondParaLeft + nSecondParaWidth / 2);
    aPoint.setY(nSecondParaTop + nSecondParaHeight - 100);
    SwCursorMoveState aState(CursorMoveState::NONE);
    pLayout->GetModelPositionForViewPoint(&aPosition, aPoint, &aState);
    CPPUNIT_ASSERT_EQUAL(aCellStart.GetNodeIndex() + 1, aPosition.GetNodeIndex());
    // Without the accompanying fix in place, this test would have failed: character position was 5,
    // i.e. cursor was at the end of the paragraph.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aPosition.GetContentIndex());

    // Test that the selection rectangles are inside the cell frame if we select all the cell
    // content.
    SwTwips nCellLeft
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds", "left").toInt32();
    SwTwips nCellWidth
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds", "width").toInt32();
    SwTwips nCellTop
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds", "top").toInt32();
    SwTwips nCellHeight
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds", "height").toInt32();
    SwRect aCellRect(Point(nCellLeft, nCellTop), Size(nCellWidth, nCellHeight));
    pWrtShell->SelAll();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(/*bBlock=*/false);
    CPPUNIT_ASSERT(!pShellCursor->empty());
    // Without the accompanying fix in place, this test would have failed with:
    // selection rectangle 269x2573@(1970,2172) is not inside cell rectangle 3207x1134@(1593,1701)
    // i.e. the selection went past the bottom border of the cell frame.
    for (const auto& rRect : *pShellCursor)
    {
        std::stringstream ss;
        ss << "selection rectangle " << rRect << " is not inside cell rectangle " << aCellRect;
        CPPUNIT_ASSERT_MESSAGE(ss.str(), aCellRect.Contains(rRect));
    }

    // Make sure that the correct rectangle gets repainted on scroll.
    SwFrame* pPageFrame = pLayout->GetLower();
    CPPUNIT_ASSERT(pPageFrame->IsPageFrame());

    SwFrame* pBodyFrame = pPageFrame->GetLower();
    CPPUNIT_ASSERT(pBodyFrame->IsBodyFrame());

    SwFrame* pTabFrame = pBodyFrame->GetLower();
    CPPUNIT_ASSERT(pTabFrame->IsTabFrame());

    SwFrame* pRowFrame = pTabFrame->GetLower();
    CPPUNIT_ASSERT(pRowFrame->IsRowFrame());

    SwFrame* pCellFrame = pRowFrame->GetLower();
    CPPUNIT_ASSERT(pCellFrame->IsCellFrame());

    SwFrame* pFrame = pCellFrame->GetLower();
    CPPUNIT_ASSERT(pFrame->IsTextFrame());

    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pFrame);
    pTextFrame->SwapWidthAndHeight();
    // Mimic what normally SwTextFrame::PaintSwFrame() does:
    SwRect aRect(4207, 2273, 269, 572);
    pTextFrame->SwitchVerticalToHorizontal(aRect);
    // Without the accompanying fix in place, this test would have failed with:
    // Expected: 572x269@(1691,4217)
    // Actual  : 572x269@(2263,4217)
    // i.e. the paint rectangle position was incorrect, text was not painted on scrolling up.
    CPPUNIT_ASSERT_EQUAL(SwRect(1691, 4217, 572, 269), aRect);
#endif
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf123898)
{
    createSwDoc("tdf123898.odt");

    // Make sure spellchecker has done its job already
    Scheduler::ProcessEventsToIdle();

    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("de", "DE", OUString())))
        return;

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure that the arrow on the left is not there (the first portion's type is
    // PortionType::Arrow if it's there)
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/fly/txt/SwParaPortion/SwLineLayout[1]/child::*[1]",
                "type", u"PortionType::Text");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf123651)
{
    createSwDoc("tdf123651.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with 'Expected: 7639;
    // Actual: 12926'. The shape was below the second "Lorem ipsum" text, not above it.
    const sal_Int32 nTopValue
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "top").toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7639, nTopValue, 10);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf116501)
{
    //just care it doesn't freeze
    createSwDoc("tdf116501.odt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf123163)
{
    //just care it doesn't assert
    createSwDoc("tdf123163-1.docx");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testAbi11870)
{
    //just care it doesn't assert
    createSwDoc("abi11870-2.odt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testOfz64109)
{
    //just care it doesn't assert
    createSwDoc("ofz64109-1.fodt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf118719)
{
    // Insert a page break.
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // Enable hide whitespace mode.
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    pWrtShell->Insert(u"first"_ustr);
    pWrtShell->InsertPageBreak();
    pWrtShell->Insert(u"second"_ustr);

    // Without the accompanying fix in place, this test would have failed, as the height of the
    // first page was 15840 twips, instead of the much smaller 276.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nOther = getXPath(pXmlDoc, "/root/page[1]/infos/bounds", "height").toInt32();
    sal_Int32 nLast = getXPath(pXmlDoc, "/root/page[2]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(nOther, nLast);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTabOverMargin)
{
    createSwDoc("tab-over-margin.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 2nd paragraph has a tab over the right margin, and with the TabOverMargin compat option,
    // there is enough space to have all content in a single line.
    // Without the accompanying fix in place, this test would have failed, there were 2 lines.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testImageComment)
{
    // Load a document that has "aaa" in it, then a commented image (4th char is the as-char image,
    // 5th char is the comment anchor).
    createSwDoc("image-comment.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // Look up a layout position which is on the right of the image.
    SwRootFrame* pRoot = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pRoot->GetLower()->IsPageFrame());
    SwPageFrame* pPage = static_cast<SwPageFrame*>(pRoot->GetLower());
    CPPUNIT_ASSERT(pPage->GetLower()->IsBodyFrame());
    SwBodyFrame* pBody = static_cast<SwBodyFrame*>(pPage->GetLower());
    CPPUNIT_ASSERT(pBody->GetLower()->IsTextFrame());
    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pBody->GetLower());
    CPPUNIT_ASSERT(pTextFrame->GetDrawObjs());
    SwSortedObjs& rDrawObjs = *pTextFrame->GetDrawObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rDrawObjs.size());
    SwAnchoredObject* pDrawObj = rDrawObjs[0];
    const SwRect aDrawObjRect = pDrawObj->GetObjRect();
    Point aPoint = aDrawObjRect.Center();
    aPoint.setX(aPoint.getX() + aDrawObjRect.Width() / 2);

    // Ask for the doc model pos of this layout point.
    SwPosition aPosition(*pTextFrame->GetTextNodeForFirstText());
    pTextFrame->GetModelPositionForViewPoint(&aPosition, aPoint);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5
    // - Actual  : 4
    // i.e. the cursor got positioned between the image and its comment, so typing extended the
    // comment, instead of adding content after the commented image.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), aPosition.GetContentIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testScriptField)
{
    // Test clicking script field inside table ( tdf#141079 )
    createSwDoc("tdf141079.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // Look up layout position which is the first cell in the table
    SwRootFrame* pRoot = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pRoot->GetLower()->IsPageFrame());
    SwPageFrame* pPage = static_cast<SwPageFrame*>(pRoot->GetLower());
    CPPUNIT_ASSERT(pPage->GetLower()->IsBodyFrame());
    SwBodyFrame* pBody = static_cast<SwBodyFrame*>(pPage->GetLower());
    CPPUNIT_ASSERT(pBody->GetLower()->IsTextFrame());
    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pBody->GetLower());
    CPPUNIT_ASSERT(pTextFrame->GetNext()->IsTabFrame());
    SwFrame* pTable = pTextFrame->GetNext();
    SwFrame* pRow1 = pTable->GetLower();
    CPPUNIT_ASSERT(pRow1->GetLower()->IsCellFrame());
    SwFrame* pCell1 = pRow1->GetLower();
    CPPUNIT_ASSERT(pCell1->GetLower()->IsTextFrame());
    SwTextFrame* pCellTextFrame = static_cast<SwTextFrame*>(pCell1->GetLower());
    const SwRect& rCellRect = pCell1->getFrameArea();
    Point aPoint = rCellRect.Center();
    aPoint.setX(aPoint.getX() - rCellRect.Width() / 2);

    // Ask for the doc model pos of this layout point.
    SwPosition aPosition(*pCellTextFrame->GetTextNodeForFirstText());
    pCellTextFrame->GetModelPositionForViewPoint(&aPosition, aPoint);

    // Position was 1 without the fix from tdf#141079
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aPosition.GetContentIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testCommentCursorPosition)
{
    // Load a document that has "aaa" in it, followed by three comments.
    createSwDoc("endOfLineComments.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    SwRootFrame* pRoot = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pRoot->GetLower()->IsPageFrame());
    SwPageFrame* pPage = static_cast<SwPageFrame*>(pRoot->GetLower());
    CPPUNIT_ASSERT(pPage->GetLower()->IsBodyFrame());
    SwBodyFrame* pBody = static_cast<SwBodyFrame*>(pPage->GetLower());
    CPPUNIT_ASSERT(pBody->GetLower()->IsTextFrame());
    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pBody->GetLower());

    // Set a point in the whitespace past the end of the first line.
    Point aPoint = pWrtShell->getShellCursor(false)->GetSttPos();
    aPoint.setX(aPoint.getX() + 10000);

    // Ask for the doc model pos of this layout point.
    SwPosition aPosition(*pTextFrame->GetTextNodeForFirstText());
    pTextFrame->GetModelPositionForViewPoint(&aPosition, aPoint);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6
    // - Actual  : 3 or 4
    // i.e. the cursor got positioned before the comments,
    // so typing extended the first comment instead of adding content after the comments.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6), aPosition.GetContentIndex());
    // The second line is also important, but can't be auto-tested
    // since the failing situation depends on GetViewWidth which is zero in the headless tests.
    // bb<comment>|   - the cursor should move behind the |, not before it.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testCombiningCharacterCursorPosition)
{
    // Load a document that has "a" in it, followed by a combining acute in a separate rext span
    createSwDoc("tdf138592-a-acute.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    SwRootFrame* pRoot = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pRoot->GetLower()->IsPageFrame());
    SwPageFrame* pPage = static_cast<SwPageFrame*>(pRoot->GetLower());
    CPPUNIT_ASSERT(pPage->GetLower()->IsBodyFrame());
    SwBodyFrame* pBody = static_cast<SwBodyFrame*>(pPage->GetLower());
    CPPUNIT_ASSERT(pBody->GetLower()->IsTextFrame());
    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pBody->GetLower());

    // Set a point in the whitespace past the end of the first line.
    Point aPoint = pWrtShell->getShellCursor(false)->GetSttPos();
    aPoint.AdjustX(10000);

    // Ask for the doc model pos of this layout point.
    SwPosition aPosition(*pTextFrame->GetTextNodeForFirstText());
    pTextFrame->GetModelPositionForViewPoint(&aPosition, aPoint);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the cursor got positioned before the acute, so typing shifted the acute (applying it
    // to newly typed characters) instead of adding content after it.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aPosition.GetContentIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf64222)
{
    createSwDoc("tdf64222.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc,
                "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::Number']/SwFont",
                "height", u"560");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf113014)
{
    createSwDoc("tdf113014.fodt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if numbering of cell A1 is missing
    // (A1: left indent: 3 cm, first line indent: -3 cm
    // A2: left indent: 0 cm, first line indent: 0 cm)
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text", u"1.");
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[3]/text", u"2.");
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[5]/text", u"3.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf130218)
{
    createSwDoc("tdf130218.fodt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if hanging first line was hidden
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text", u"Text");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf127235)
{
    createSwDoc("tdf127235.odt");
    // This resulted in a layout loop.
    calcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf138039)
{
    createSwDoc("tdf138039.odt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // there are 3 pages
    assertXPath(pXmlDoc, "/root/page", 3);
    // table on first page
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 0);
    // paragraph with large fly on second page
    assertXPath(pXmlDoc, "/root/page[2]/body/tab", 0);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "top", u"17915");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "height",
                u"15819");
    // paragraph on third page
    assertXPath(pXmlDoc, "/root/page[3]/body/tab", 0);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf134298)
{
    createSwDoc("tdf134298.ott");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // there are 2 pages
    assertXPath(pXmlDoc, "/root/page", 2);
    // table and first para on first page
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored", 0);
    // paragraph with large fly on second page
    assertXPath(pXmlDoc, "/root/page[2]/body/tab", 0);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "top", u"17897");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "height",
                u"15819");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testShapeAllowOverlap)
{
// Need to find out why this fails on macOS and why this is unstable on Windows.
#if !defined(MACOSX) && !defined(_WIN32)
    // Create an empty document with two, intentionally overlapping shapes.
    // Set their AllowOverlap property to false.
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    awt::Point aPoint(1000, 1000);
    awt::Size aSize(2000, 2000);
    uno::Reference<drawing::XShape> xShape(
        xDocument->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xDocument, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue(u"AllowOverlap"_ustr, uno::Any(false));
    xShapeProperties->setPropertyValue(u"AnchorType"_ustr,
                                       uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xDrawPageSupplier->getDrawPage()->add(xShape);

    aPoint = awt::Point(2000, 2000);
    xShape.set(xDocument->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr),
               uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    xShapeProperties.set(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue(u"AllowOverlap"_ustr, uno::Any(false));
    xShapeProperties->setPropertyValue(u"AnchorType"_ustr,
                                       uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // Now verify that the rectangle of the anchored objects don't overlap.
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPageFrame = pLayout->GetLower();
    SwFrame* pBodyFrame = pPageFrame->GetLower();
    SwFrame* pTextFrame = pBodyFrame->GetLower();
    CPPUNIT_ASSERT(pTextFrame->GetDrawObjs());
    SwSortedObjs& rObjs = *pTextFrame->GetDrawObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rObjs.size());
    SwAnchoredObject* pFirst = rObjs[0];
    SwAnchoredObject* pSecond = rObjs[1];
    // Without the accompanying fix in place, this test would have failed: the layout dump was
    // <bounds left="1984" top="1984" width="1137" height="1137"/>
    // <bounds left="2551" top="2551" width="1137" height="1137"/>
    // so there was a clear vertical overlap. (Allow for 1px tolerance.)
    OString aMessage = "Unexpected overlap: first shape's bottom is "
                       + OString::number(pFirst->GetObjRect().Bottom()) + ", second shape's top is "
                       + OString::number(pSecond->GetObjRect().Top());
    CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(),
                           std::abs(pFirst->GetObjRect().Bottom() - pSecond->GetObjRect().Top())
                               < 15);
#endif
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testShapeAllowOverlapWrap)
{
    // Create an empty document with two, intentionally overlapping shapes.
    // Set their AllowOverlap property to false and their wrap to through.
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    awt::Point aPoint(1000, 1000);
    awt::Size aSize(2000, 2000);
    uno::Reference<drawing::XShape> xShape(
        xDocument->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xDocument, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue(u"AllowOverlap"_ustr, uno::Any(false));
    xShapeProperties->setPropertyValue(u"AnchorType"_ustr,
                                       uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProperties->setPropertyValue(u"Surround"_ustr, uno::Any(text::WrapTextMode_THROUGH));
    xDrawPageSupplier->getDrawPage()->add(xShape);

    aPoint = awt::Point(2000, 2000);
    xShape.set(xDocument->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr),
               uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    xShapeProperties.set(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue(u"AllowOverlap"_ustr, uno::Any(false));
    xShapeProperties->setPropertyValue(u"AnchorType"_ustr,
                                       uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProperties->setPropertyValue(u"Surround"_ustr, uno::Any(text::WrapTextMode_THROUGH));
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // Now verify that the rectangle of the anchored objects do overlap.
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPageFrame = pLayout->GetLower();
    SwFrame* pBodyFrame = pPageFrame->GetLower();
    SwFrame* pTextFrame = pBodyFrame->GetLower();
    CPPUNIT_ASSERT(pTextFrame->GetDrawObjs());
    SwSortedObjs& rObjs = *pTextFrame->GetDrawObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rObjs.size());
    SwAnchoredObject* pFirst = rObjs[0];
    SwAnchoredObject* pSecond = rObjs[1];
    // Without the accompanying fix in place, this test would have failed: AllowOverlap=no had
    // priority over Surround=through (which is bad for Word compat).
    CPPUNIT_ASSERT(pSecond->GetObjRect().Overlaps(pFirst->GetObjRect()));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf124600)
{
    createSwDoc("tdf124600.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the last line in the body text had 2 lines, while it should have 1, as Word does (as the
    // fly frame does not intersect with the print area of the paragraph.)
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf124601)
{
    // This is a testcase for the ContinuousEndnotes compat flag.
    // The document has 2 pages, the endnote anchor is on the first page.
    // The endnote should be on the 2nd page together with the last page content.
    createSwDoc("tdf124601.doc");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. there was a separate endnote page, even when the ContinuousEndnotes compat option was
    // on.
    assertXPath(pXmlDoc, "/root/page", 2);
    assertXPath(pXmlDoc, "/root/page[2]//ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf124601b)
{
    // Table has an image, which is anchored in the first row, but its vertical position is large
    // enough to be rendered in the second row.
    // The shape has layoutInCell=1, so should match what Word does here.
    // Also the horizontal position should be in the last column, even if the anchor is in the
    // last-but-one column.
    createSwDoc("tdf124601b.doc");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    sal_Int32 nFlyTop = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "top").toInt32();
    sal_Int32 nFlyLeft = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "left").toInt32();
    sal_Int32 nFlyRight
        = nFlyLeft + getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "width").toInt32();
    sal_Int32 nSecondRowTop = getXPath(pXmlDoc, "//tab/row[2]/infos/bounds", "top").toInt32();
    sal_Int32 nLastCellLeft
        = getXPath(pXmlDoc, "//tab/row[1]/cell[5]/infos/bounds", "left").toInt32();
    sal_Int32 nLastCellRight
        = nLastCellLeft + getXPath(pXmlDoc, "//tab/row[1]/cell[5]/infos/bounds", "width").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 3736
    // - Actual  : 2852
    // i.e. the image was still inside the first row.
    CPPUNIT_ASSERT_GREATER(nSecondRowTop, nFlyTop);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 9640
    // - Actual  : 9639
    // i.e. the right edge of the image was not within the bounds of the last column, the right edge
    // was in the last-but-one column.
    CPPUNIT_ASSERT_GREATER(nLastCellLeft, nFlyRight);
    CPPUNIT_ASSERT_LESS(nLastCellRight, nFlyRight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf124770)
{
    // Enable content over margin.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    pDoc->getIDocumentSettingAccess().set(DocumentSettingId::TAB_OVER_MARGIN, true);

    // Set page width.
    SwPageDesc& rPageDesc = pDoc->GetPageDesc(0);
    SwFrameFormat& rPageFormat = rPageDesc.GetMaster();
    const SwAttrSet& rPageSet = rPageFormat.GetAttrSet();
    SwFormatFrameSize aPageSize = rPageSet.GetFrameSize();
    aPageSize.SetWidth(3703);
    rPageFormat.SetFormatAttr(aPageSize);

    // Set left and right margin.
    SvxLRSpaceItem aLRSpace = rPageSet.GetLRSpace();
    aLRSpace.SetLeft(SvxIndentValue::twips(1418));
    aLRSpace.SetRight(SvxIndentValue::twips(1418));
    rPageFormat.SetFormatAttr(aLRSpace);
    pDoc->ChgPageDesc(0, rPageDesc);

    // Set font to italic 20pt Liberation Serif.
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SfxItemSet aTextSet(pWrtShell->GetView().GetPool(),
                        svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END - 1>);
    SvxFontItem aFont(RES_CHRATR_FONT);
    aFont.SetFamilyName(u"Liberation Serif"_ustr);
    aTextSet.Put(aFont);
    SvxFontHeightItem aHeight(400, 100, RES_CHRATR_FONTSIZE);
    aTextSet.Put(aHeight);
    SvxPostureItem aItalic(ITALIC_NORMAL, RES_CHRATR_POSTURE);
    aTextSet.Put(aItalic);
    pWrtShell->SetAttrSet(aTextSet);

    // Insert the text.
    pWrtShell->Insert2(u"HHH"_ustr);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the italic string was broken into 2 lines, while Word kept it in a single line.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testContinuousEndnotesInsertPageAtStart)
{
    // Create a new document with CONTINUOUS_ENDNOTES enabled.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    pDoc->getIDocumentSettingAccess().set(DocumentSettingId::CONTINUOUS_ENDNOTES, true);

    // Insert a second page, and an endnote on the 2nd page (both the anchor and the endnote is on
    // the 2nd page).
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();
    pWrtShell->InsertFootnote(u"endnote"_ustr, /*bEndNote=*/true, /*bEdit=*/false);

    // Add a new page at the start of the document.
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->InsertPageBreak();

    // Make sure that the endnote is moved from the 2nd page to the 3rd one.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 3);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the footnote container remained on page 2.
    assertXPath(pXmlDoc, "/root/page[3]//ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testContinuousEndnotesDeletePageAtStart)
{
    // Create a new document with CONTINUOUS_ENDNOTES enabled.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    pDoc->getIDocumentSettingAccess().set(DocumentSettingId::CONTINUOUS_ENDNOTES, true);

    // Insert a second page, and an endnote on the 2nd page (both the anchor and the endnote is on
    // the 2nd page).
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();
    pWrtShell->InsertFootnote(u"endnote"_ustr, /*bEndNote=*/true, /*bEdit=*/false);

    // Remove the empty page at the start of the document.
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->DelRight();

    // Make sure that the endnote is moved from the 2nd page to the 1st one.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the endnote remained on an (otherwise) empty 2nd page.
    assertXPath(pXmlDoc, "/root/page", 1);
    assertXPath(pXmlDoc, "/root/page[1]//ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf128399)
{
    createSwDoc("tdf128399.docx");
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->GetLower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pTable = pBody->GetLower();
    SwFrame* pRow1 = pTable->GetLower();
    SwFrame* pRow2 = pRow1->GetNext();
    const SwRect& rRow2Rect = pRow2->getFrameArea();
    Point aPoint = rRow2Rect.Center();

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwPosition aPosition = *pWrtShell->GetCursor()->Start();
    SwPosition aFirstRow(aPosition);
    SwCursorMoveState aState(CursorMoveState::NONE);
    pLayout->GetModelPositionForViewPoint(&aPosition, aPoint, &aState);
    // Second row is +3: end node, start node and the first text node in the 2nd row.
    SwNodeOffset nExpected = aFirstRow.GetNodeIndex() + 3;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14
    // - Actual  : 11
    // i.e. clicking on the center of the 2nd row placed the cursor in the 1st row.
    CPPUNIT_ASSERT_EQUAL(nExpected, aPosition.GetNodeIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf156724)
{
    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue(u"Hidden"_ustr, -1, uno::Any(true),
                               beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    createSwDoc("fdo56797-2-min.odt", comphelper::containerToSequence(aFilterOptions));
    save(TestFilter::PDF_WRITER);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // both pages have a tab frame and one footnote
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/ftncont", 1);
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont", 1);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn", 1);
    assertXPath(pXmlDoc, "/root/page", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testHiddenParagraphFollowFrame)
{
    createSwDoc("hidden-para-follow-frame.fodt");

    comphelper::ScopeGuard g(
        [ this, old = queryDispatchStatus(mxComponent, m_xContext, ".uno:ShowHiddenParagraphs") ] {
            auto args(comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", old } }));
            dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", args);
        });

    {
        // disable Field Names warning dialog
        ScopedConfigValue<officecfg::Office::Common::Misc::QueryShowFieldName> aCfg(false);

        uno::Sequence<beans::PropertyValue> argsSH(
            comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", uno::Any(true) } }));
        dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", argsSH);
        uno::Sequence<beans::PropertyValue> args(
            comphelper::InitPropertySequence({ { "Fieldnames", uno::Any(false) } }));
        dispatchCommand(mxComponent, ".uno:Fieldnames", args);
        Scheduler::ProcessEventsToIdle();

        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page", 2);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt", 2);
    }

    dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", {});

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // the problem was that the 3rd paragraph didn't move to page 1
        assertXPath(pXmlDoc, "/root/page", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt", 3);
    }

    dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", {});

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page", 2);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt", 2);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testHiddenParagraphFlys)
{
    createSwDoc("hidden-para-as-char-fly.fodt");

    comphelper::ScopeGuard g(
        [ this, old = queryDispatchStatus(mxComponent, m_xContext, ".uno:ShowHiddenParagraphs") ] {
            auto args(comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", old } }));
            dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", args);
        });

    {
        // disable Field Names warning dialog
        ScopedConfigValue<officecfg::Office::Common::Misc::QueryShowFieldName> aCfg(false);

        uno::Sequence<beans::PropertyValue> argsSH(
            comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", uno::Any(true) } }));
        dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", argsSH);
        uno::Sequence<beans::PropertyValue> args(
            comphelper::InitPropertySequence({ { "Fieldnames", uno::Any(false) } }));
        dispatchCommand(mxComponent, ".uno:Fieldnames", args);
        Scheduler::ProcessEventsToIdle();

        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/txt[3]/anchored/fly/infos/bounds", "height", u"724");
    }

    dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", {});

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // the problem was that this did not shrink
        assertXPath(pXmlDoc, "/root/page/body/txt[3]/anchored/fly/infos/bounds", "height", u"448");
    }

    dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", {});

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/txt[3]/anchored/fly/infos/bounds", "height", u"724");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testSectionUnhide)
{
    createSwDoc("hiddensection.fodt");

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/section/txt/infos/bounds[@height='0']", 0);
    }

    // Hide the section
    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection = xSections->getByName(u"Section1"_ustr).queryThrow<css::beans::XPropertySet>();
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(false));
    calcLayout();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/section/txt/infos/bounds[@height='0']", 4);
    }

    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(true));
    calcLayout();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // the problem was that 3 of the text frames had 0 height because Format was skipped
        assertXPath(pXmlDoc, "/root/page/body/section/txt/infos/bounds[@height='0']", 0);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testHiddenSectionFlys)
{
    createSwDoc("U-min.fodt");

    //NO! field update job masks if the visibility was created wrong when loading.
    //Scheduler::ProcessEventsToIdle();

    SwDoc* pDoc = getSwDoc();
    IDocumentDrawModelAccess const& rIDMA{ pDoc->getIDocumentDrawModelAccess() };
    SdrPage const* pDrawPage{ rIDMA.GetDrawModel()->GetPage(0) };
    int invisibleHeaven{ rIDMA.GetInvisibleHeavenId().get() };
    int visibleHeaven{ rIDMA.GetHeavenId().get() };

    // these are hidden by moving to invisible layer, they're still in layout
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//anchored/fly", 6);

        CPPUNIT_ASSERT_EQUAL(size_t(6), pDrawPage->GetObjCount());
        for (int i = 0; i < 6; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(invisibleHeaven, int(pDrawPage->GetObj(i)->GetLayer().get()));
        }
    }

    // Show the section
    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection = xSections->getByName(u"Anlage"_ustr).queryThrow<css::beans::XPropertySet>();
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(true));
    calcLayout();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//anchored/fly", 6);

        CPPUNIT_ASSERT_EQUAL(size_t(6), pDrawPage->GetObjCount());
        for (int i = 0; i < 6; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(visibleHeaven, int(pDrawPage->GetObj(i)->GetLayer().get()));
        }
    }

    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(false));
    calcLayout();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//anchored/fly", 6);

        CPPUNIT_ASSERT_EQUAL(size_t(6), pDrawPage->GetObjCount());
        for (int i = 0; i < 6; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(invisibleHeaven, int(pDrawPage->GetObj(i)->GetLayer().get()));
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf169399)
{
    createSwDoc("tdf169399.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the fix, this failed, because there were two pages:
    assertXPath(pXmlDoc, "/root/page", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf155306)
{
    // Given a document defining footnotes counted per page, with a 2-column section containing
    // a footnote:
    createSwDoc("footnote-in-2-column-section.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // The footnote must have the correct number. Without the fix, the following tests both failed,
    // because the number was 0:
    assertXPath(pXmlDoc, "//body/section/column[1]//SwFieldPortion[@type='PortionType::Footnote']",
                "expand", u"2");
    assertXPath(pXmlDoc, "//ftncont/ftn[2]//SwFieldPortion[@type='PortionType::FootnoteNum']",
                "expand", u"2");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf170381_split_float_table_in_normal_table)
{
    // Given a document with a normal table containing a floating table which is split across
    // pages:
    createSwDoc("tdf170381-split-float-table-in-normal-table.docx");

    // 1. It must not hang.
    // 2. Check some correct layout aspects:

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Exactly two pages:
    assertXPath(pXmlDoc, "//page", 2);

    // Exactly one object anchored at each page:
    assertXPath(pXmlDoc, "//page[1]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "//page[2]/sorted_objs/fly", 1);

    // Get the ids of the two flys (for both pages):
    OString f1 = getXPath(pXmlDoc, "//page[1]/sorted_objs/fly", "ptr").toUtf8();
    OString f2 = getXPath(pXmlDoc, "//page[2]/sorted_objs/fly", "ptr").toUtf8();
    CPPUNIT_ASSERT(f1 != f2);
    assertXPath(pXmlDoc, "//anchored/fly", 2);
    OString aP1FlyTab = "//anchored/fly[@ptr='" + f1 + "']/tab";
    OString aP2FlyTab = "//anchored/fly[@ptr='" + f2 + "']/tab";

    // Exactly one normal (master / follow) table on each page:
    assertXPath(pXmlDoc, "//page[1]/body/tab", 1);
    assertXPath(pXmlDoc, "//page[2]/body/tab", 1);
    assertXPathNoAttribute(pXmlDoc, "//page[1]/body/tab", "precede");
    assertXPath(pXmlDoc, "//page[1]/body/tab", "follow",
                getXPath(pXmlDoc, "//page[2]/body/tab", "id"));
    assertXPathNoAttribute(pXmlDoc, "//page[2]/body/tab", "follow");
    assertXPath(pXmlDoc, "//page[2]/body/tab", "precede",
                getXPath(pXmlDoc, "//page[1]/body/tab", "id"));

    // Exactly two rows in the first page's normal table:
    assertXPath(pXmlDoc, "//page[1]/body/tab/row", 2);

    // Check the text of the first (repeating) row's cell text:
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[1]/cell", 1);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[1]/cell/txt[1]//SwLineLayout", "portion",
                u"elit ipsum lorem dolor");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[1]/cell/txt[2]//SwLineLayout", "portion",
                u"amet elit amet sit adipiscing adipiscing consectetur consectetur elit dolor");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[1]/cell/txt[3]//SwLineLayout", "portion", u"");
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[1]/cell/txt[4]//SwLineLayout", "portion", u"");

    // The second row's cell has a single master paragraph with two anchored flys:
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[2]/cell", 1);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[2]/cell/txt", 1);
    OUString followId = getXPath(pXmlDoc, "//page[1]/body/tab/row[2]/cell/txt", "follow");
    CPPUNIT_ASSERT_GREATER(sal_Int32(0), followId.toInt32());
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[2]/cell/txt/anchored/fly", 2);

    // Exactly four rows in the second page's normal table:
    assertXPath(pXmlDoc, "//page[2]/body/tab/row", 4);

    // Check the text of the first (repeating) row's cell text:
    assertXPath(pXmlDoc, "//page[2]/body/tab/row[1]/cell", 1);
    assertXPath(pXmlDoc, "//page[2]/body/tab/row[1]/cell/txt[1]//SwLineLayout", "portion",
                u"elit ipsum lorem dolor");
    assertXPath(pXmlDoc, "//page[2]/body/tab/row[1]/cell/txt[2]//SwLineLayout", "portion",
                u"amet elit amet sit adipiscing adipiscing consectetur consectetur elit dolor");
    assertXPath(pXmlDoc, "//page[2]/body/tab/row[1]/cell/txt[3]//SwLineLayout", "portion", u"");
    assertXPath(pXmlDoc, "//page[2]/body/tab/row[1]/cell/txt[4]//SwLineLayout", "portion", u"");

    // The second row's cell has a single follow paragraph:
    assertXPath(pXmlDoc, "//page[2]/body/tab/row[2]/cell", 1);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row[2]/cell/txt", 1);
    assertXPath(pXmlDoc, "//page[2]/body/tab/row[2]/cell/txt", "id", followId);
    assertXPath(pXmlDoc, "//page[2]/body/tab/row[2]/cell/txt", "precede",
                getXPath(pXmlDoc, "//page[1]/body/tab/row[2]/cell/txt", "id"));

    // Test floating tables' content (the line split must be correct).

    auto assertCellLines
        = [&](int page, int row, std::initializer_list<std::u16string_view> lines) {
              OString base = (page == 1 ? aP1FlyTab : aP2FlyTab) + "/row[" + OString::number(row)
                             + "]/cell/txt/SwParaPortion/SwLineLayout[";
              int i = 1;
              for (const auto& line : lines)
                  assertXPath(pXmlDoc, base + OString::number(i++) + "]", "portion", line);
          };

    // Page 1's floating table:
    // NB: the *intended correct layout* is, when the first page's floating table has 5 rows!
    // Currently asserting 6 rows on page 1, but row 6 must move to page 2, when fixed properly.

    std::initializer_list<std::u16string_view> page1cells[] = {
        { u"amet sit consectetur ", u"elit" },
        { u"" },
        { u"dolor dolor dolor ", u"ipsum" },
        { u"amet ipsum amet dolor ", u"elit sit" },
        { u"ipsum consectetur ", u"consectetur amet ", u"adipiscing ipsum" },
        // NB: this must move to the follow!
        { u"" },
    };

    assertXPath(pXmlDoc, aP1FlyTab + "/row", std::size(page1cells));
    for (size_t r = 0; r < std::size(page1cells); ++r)
        assertCellLines(1, r + 1, page1cells[r]);

    // Page 2's floating table:

    std::initializer_list<std::u16string_view> page2cells[] = {
        { u"adipiscing ipsum elit ", u"lorem" },
        { u"lorem adipiscing sit sit ", u"lorem lorem" },
        { u"ipsum lorem ", u"consectetur amet amet ", u"ipsum" },
        { u"" },
        { u"sit consectetur ", u"adipiscing sit" },
        { u"elit consectetur lorem ", u"consectetur ", u"consectetur lorem sit ",
          u"sit dolor elit adipiscing ", u"consectetur sit" },
        { u"consectetur dolor ", u"dolor sit elit lorem ", u"consectetur dolor ", u"lorem ipsum" },
    };

    assertXPath(pXmlDoc, aP2FlyTab + "/row", std::size(page2cells));
    for (size_t r = 0; r < std::size(page2cells); ++r)
        assertCellLines(2, r + 1, page2cells[r]);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf170381_split_float_table_in_float_table)
{
    // Given a document with a floating table containing another floating table which is split
    // across pages:
    createSwDoc("tdf170381-split-float-table-in-float-table.docx");

    // 1. It must not hang.
    // 2. Check some correct layout aspects:

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Exactly two pages:
    assertXPath(pXmlDoc, "//page", 2);

    // Exactly two objects anchored at each page:
    assertXPath(pXmlDoc, "//page[1]/sorted_objs/fly", 2);
    assertXPath(pXmlDoc, "//page[2]/sorted_objs/fly", 2);

    // Exactly one (master/follow) paragraph on each page:
    assertXPath(pXmlDoc, "//page[1]/body/txt", 1);
    assertXPath(pXmlDoc, "//page[2]/body/txt", 1);
    assertXPath(pXmlDoc, "//page[1]/body/txt", "follow",
                getXPath(pXmlDoc, "//page[2]/body/txt", "id"));
    assertXPath(pXmlDoc, "//page[2]/body/txt", "precede",
                getXPath(pXmlDoc, "//page[1]/body/txt", "id"));

    // Page 1's paragraph has two anchored flys:
    assertXPath(pXmlDoc, "//page[1]/body/txt/anchored/fly", 2);

    // Get the ids of the two outer flys.
    // Page 1:
    OString f1 = getXPath(pXmlDoc, "//page[1]/sorted_objs/fly[1]", "ptr").toUtf8();
    OString f2 = getXPath(pXmlDoc, "//page[1]/sorted_objs/fly[2]", "ptr").toUtf8();
    CPPUNIT_ASSERT(f1 != f2);
    OString filter1 = "@ptr='" + f1 + "' or @ptr='" + f2 + "'";
    OUString id = getXPath(pXmlDoc, "//page[1]/body/txt/anchored/fly[" + filter1 + "]", "id");
    OString aP1OuterFlyTab = "//anchored/fly[@id='" + id.toUtf8() + "']/tab";

    // Page 2:
    f1 = getXPath(pXmlDoc, "//page[2]/sorted_objs/fly[1]", "ptr").toUtf8();
    f2 = getXPath(pXmlDoc, "//page[2]/sorted_objs/fly[2]", "ptr").toUtf8();
    CPPUNIT_ASSERT(f1 != f2);
    OString filter2 = "@ptr='" + f1 + "' or @ptr='" + f2 + "'";
    id = getXPath(pXmlDoc, "//page[1]/body/txt/anchored/fly[" + filter2 + "]", "id");
    OString aP2OuterFlyTab = "//anchored/fly[@id='" + id.toUtf8() + "']/tab";

    // Exactly one row in both top-level floating tables:
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row", 1);
    assertXPath(pXmlDoc, aP2OuterFlyTab + "/row", 1);

    // Exactly one cell in both top-level floating tables:
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell", 1);
    assertXPath(pXmlDoc, aP2OuterFlyTab + "/row/cell", 1);

    // First page's top-level floating table's cell has three paragraphs:
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell/txt", 3);
    // Check text in the first two paragraphs:
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell/txt[1]//SwLineLayout", "portion",
                u"Table1 A1 dolor elit");
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell/txt[2]//SwLineLayout", "portion",
                u"adipiscing dolor adipiscing amet ipsum elit sit elit lorem elit adipiscing "
                "dolor ipsum");
    // The third paragraph has two attached inner floating tables:
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell/txt[3]/anchored/fly", 2);

    // Get the ids of the two inner flys.
    // Page 1:
    id = getXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell/txt[3]/anchored/fly[" + filter1 + "]", "id");
    OString aP1InnerFlyTab = "//anchored/fly[@id='" + id.toUtf8() + "']/tab";

    // Page 2:
    id = getXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell/txt[3]/anchored/fly[" + filter2 + "]", "id");
    OString aP2InnerFlyTab = "//anchored/fly[@id='" + id.toUtf8() + "']/tab";

    // Check the layout of the inner tables (splitting of lines and rows).

    auto assertCellLines
        = [&](int page, int row, std::initializer_list<std::u16string_view> lines) {
              OString base = (page == 1 ? aP1InnerFlyTab : aP2InnerFlyTab) + "/row["
                             + OString::number(row) + "]/cell[1]/txt/SwParaPortion/SwLineLayout[";
              int i = 1;
              for (const auto& line : lines)
                  assertXPath(pXmlDoc, base + OString::number(i++) + "]", "portion", line);
          };

    // Page 1's inner table:
    // NB: the *intended correct layout* is, when the first page's inner floating table is split
    // after row 21! Currently part of row 22 is on page 1, but must move to page 2, when fixed.

    std::initializer_list<std::u16string_view> page1cells[] = {
        { u"Table2 A1 sit amet ", u"ipsum consectetur ", u"ipsum amet ", u"adipiscing amet elit ",
          u"dolor consectetur" },
        { u"Table2 A2 ", u"consectetur ", u"adipiscing adipiscing ", u"consectetur dolor sit ",
          u"amet lorem" },
        { u"Table2 A3 dolor elit ", u"amet ipsum ", u"adipiscing ipsum ", u"dolor lorem" },
        { u"Table2 A4" },
        { u"Table2 A5 amet dolor ", u"elit consectetur lorem ", u"dolor sit amet" },
        { u"Table2 A6 sit dolor ", u"elit consectetur elit sit ", u"dolor adipiscing" },
        { u"Table2 A7" },
        { u"Table2 A8 ", u"consectetur ipsum ", u"dolor adipiscing ", u"ipsum dolor dolor ",
          u"sit elit consectetur ", u"adipiscing" },
        { u"Table2 A9 adipiscing ", u"amet dolor amet ", u"lorem elit sit amet" },
        { u"Table2 A10 amet ", u"lorem elit elit elit ", u"adipiscing elit sit" },
        { u"Table2 A11" },
        { u"Table2 A12 sit ", u"adipiscing adipiscing ", u"consectetur sit ipsum ",
          u"consectetur ipsum" },
        { u"Table2 A13 amet ", u"dolor consectetur ", u"amet dolor ipsum sit ", u"sit" },
        { u"Table2 A14" },
        { u"Table2 A15 dolor ", u"dolor elit dolor ", u"dolor ipsum ", u"consectetur amet ",
          u"elit sit" },
        { u"Table2 A16 ipsum ", u"lorem adipiscing sit ", u"sit dolor lorem elit" },
        { u"Table2 A17 sit dolor ", u"adipiscing ", u"consectetur elit ", u"ipsum lorem sit" },
        { u"Table2 A18" },
        { u"Table2 A19 sit ", u"adipiscing ", u"consectetur ", u"adipiscing lorem ",
          u"ipsum amet elit" },
        { u"Table2 A20 ipsum ", u"amet consectetur elit ", u"amet amet sit sit ", u"adipiscing" },
        { u"Table2 A21" },
        // NB: this must merge to the first row of the follow!
        { u"Table2 A22 elit ", u"ipsum elit elit sit elit " },
    };

    assertXPath(pXmlDoc, aP1InnerFlyTab + "/row", std::size(page1cells));
    for (size_t r = 0; r < std::size(page1cells); ++r)
        assertCellLines(1, r + 1, page1cells[r]);

    // Page 2's inner table:

    std::initializer_list<std::u16string_view> page2cells[] = {
        { u"sit consectetur ", u"amet sit" },
        { u"Table2 A23 ", u"consectetur amet ", u"lorem consectetur elit ", u"dolor sit elit" },
        { u"Table2 A24 ipsum ", u"amet ipsum amet ", u"consectetur lorem ", u"amet sit" },
        { u"Table2 A25" },
        { u"Table2 A26 ", u"consectetur dolor ", u"consectetur ", u"adipiscing dolor dolor ",
          u"lorem adipiscing" },
        { u"Table2 A27 dolor sit ", u"elit dolor ipsum lorem ", u"dolor elit" },
        { u"Table2 A28" },
        { u"Table2 A29 ipsum ", u"ipsum amet ipsum" },
        { u"Table2 A30 amet ", u"ipsum lorem ", u"consectetur ipsum ", u"ipsum lorem ipsum ",
          u"ipsum sit consectetur ", u"consectetur" },
        { u"Table2 A31 ", u"consectetur elit sit ", u"ipsum adipiscing ", u"ipsum ipsum ",
          u"consectetur" },
    };

    assertXPath(pXmlDoc, aP2InnerFlyTab + "/row", std::size(page2cells));
    for (size_t r = 0; r < std::size(page2cells); ++r)
        assertCellLines(2, r + 1, page2cells[r]);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf170477)
{
    // This document must not hang on layout:
    createSwDoc("tdf170477.docx");
    calcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf170620_float_table_after_keep_with_next_para)
{
    // Given a document with a keep-with-next paragraph, followed by floating table containing
    // another floating table which is split across pages:
    createSwDoc("tdf170620.docx");

    // The keep-with-next paragraph and the floating table must start on page 1:

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Exactly two pages:
    assertXPath(pXmlDoc, "//page", 2);

    // "Keep-with-next paragraph" is on the first page:
    assertXPath(pXmlDoc, "//page[1]/body/txt[2]//SwLineLayout", "portion",
                u"Keep-with-next paragraph");

    // Exactly two objects anchored at each page:
    assertXPath(pXmlDoc, "//page[1]/sorted_objs/fly", 2);
    assertXPath(pXmlDoc, "//page[2]/sorted_objs/fly", 2);

    // Get master/follow paragraph ids:
    assertXPath(pXmlDoc, "//page[1]/body/txt", 3);
    assertXPath(pXmlDoc, "//page[2]/body/txt", 1);
    assertXPath(pXmlDoc, "//page[1]/body/txt[3]", "follow",
                getXPath(pXmlDoc, "//page[2]/body/txt", "id"));
    assertXPath(pXmlDoc, "//page[2]/body/txt", "precede",
                getXPath(pXmlDoc, "//page[1]/body/txt[3]", "id"));

    // Page 1's paragraph 3 has two anchored flys:
    assertXPath(pXmlDoc, "//page[1]/body/txt[3]/anchored/fly", 2);

    // Get the ids of the two outer flys.
    // Page 1:
    OString f1 = getXPath(pXmlDoc, "//page[1]/sorted_objs/fly[1]", "ptr").toUtf8();
    OString f2 = getXPath(pXmlDoc, "//page[1]/sorted_objs/fly[2]", "ptr").toUtf8();
    CPPUNIT_ASSERT(f1 != f2);
    OString filter1 = "@ptr='" + f1 + "' or @ptr='" + f2 + "'";
    OUString id = getXPath(pXmlDoc, "//page[1]/body/txt[3]/anchored/fly[" + filter1 + "]", "id");
    OString aP1OuterFlyTab = "//anchored/fly[@id='" + id.toUtf8() + "']/tab";

    // Page 2:
    f1 = getXPath(pXmlDoc, "//page[2]/sorted_objs/fly[1]", "ptr").toUtf8();
    f2 = getXPath(pXmlDoc, "//page[2]/sorted_objs/fly[2]", "ptr").toUtf8();
    CPPUNIT_ASSERT(f1 != f2);
    OString filter2 = "@ptr='" + f1 + "' or @ptr='" + f2 + "'";
    id = getXPath(pXmlDoc, "//page[1]/body/txt[3]/anchored/fly[" + filter2 + "]", "id");
    OString aP2OuterFlyTab = "//anchored/fly[@id='" + id.toUtf8() + "']/tab";

    // One row in top-level floating table on page 1, four rows on page 2:
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row", 1);
    assertXPath(pXmlDoc, aP2OuterFlyTab + "/row", 4);

    // One cell in top-level floating table on page 1, four cells on page 2:
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell", 1);
    assertXPath(pXmlDoc, aP2OuterFlyTab + "/row/cell", 4);

    // First page's top-level floating table's cell has two paragraphs:
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell/txt", 2);
    // Check text in the first paragraph:
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell/txt[1]//SwLineLayout", "portion",
                u"Something");
    // The second paragraph has two attached inner floating tables:
    assertXPath(pXmlDoc, aP1OuterFlyTab + "/row/cell/txt[2]/anchored/fly", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf170630)
{
    // Given a document with a keep-with-next paragraph, followed by floating table containing
    // two keep-with-next paragraphs and another floating table which is split across pages:
    createSwDoc("tdf170630.docx");

    // The keep-with-next paragraph and the floating table must start on page 1:

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Exactly two pages (without the fix, there was only one):
    assertXPath(pXmlDoc, "//page", 2);

    // "Keep-with-next paragraph" is on the first page:
    assertXPath(pXmlDoc, "//page[1]/body/txt[2]//SwLineLayout", "portion",
                u"Keep-with-next paragraph");

    // Exactly two objects anchored at each page (without the fix, the first page had three: one
    // outer, and two inner, where the follow frame never moved forward):
    assertXPath(pXmlDoc, "//page[1]/sorted_objs/fly", 2);
    assertXPath(pXmlDoc, "//page[2]/sorted_objs/fly", 2);

    // Page 1's paragraph 3 has two anchored flys (without the fix, it was one: the outer table
    // never split):
    assertXPath(pXmlDoc, "//page[1]/body/txt[3]/anchored/fly", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf167946)
{
    // Given a document with a sequence of non-breaking spaces after a text and a space, which
    // sequence doesn't fit the rest of the line. Before the fix, only the comma was moved to the
    // second line, and all NBSPs were elided as a hole portion at the end of the first line:
    createSwDoc("nbsp-moved-to-new-line.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // One page:
    assertXPath(pXmlDoc, "//page", 1);

    // One paragraph:
    assertXPath(pXmlDoc, "//txt", 1);

    // Two lines in the paragraph:
    assertXPath(pXmlDoc, "//txt/SwParaPortion/SwLineLayout", 2);
    assertXPath(pXmlDoc, "//SwLineLayout", 2);

    // First line consists of a text portion with, and a hole portion for the following space:
    assertXPath(pXmlDoc, "//SwLineLayout[1]/child::*[1]", "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "//SwLineLayout[1]/child::*[1]", "portion",
                u"Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    assertXPath(pXmlDoc, "//SwLineLayout[1]/child::*[2]", "type", u"PortionType::Hole");
    assertXPath(pXmlDoc, "//SwLineLayout[1]/child::*[2]", "portion", u" ");

    // Second line has blank portions for the leading NBSPs, followed by a text portion with comma:
    assertXPathChildren(pXmlDoc, "//SwLineLayout[2]", 81);
    for (int i = 1; i <= 80; ++i)
    {
        OString aXPath = "//SwLineLayout[2]/child::*[" + OString::number(i) + "]";
        assertXPath(pXmlDoc, aXPath, "type", u"PortionType::Blank");
        assertXPath(pXmlDoc, aXPath, "portion", u"\xA0");
    }
    assertXPath(pXmlDoc, "//SwLineLayout[2]/child::*[81]", "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "//SwLineLayout[2]/child::*[81]", "portion", u",");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf169607)
{
    // Given a document with a sequence of huge letters, each of which don't fit to page vertically:
    createSwDoc("tdf169607-big-letters.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // There must be three pages, because the sequence must split two characters per page. Before
    // the fix, there was only one page:
    assertXPath(pXmlDoc, "//page", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf170811)
{
    // This used to fail assertion
    createSwDoc("tdf170811.fodt");
    // This must succeed
    parseLayoutDump();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf169999)
{
    // Open a document with a section with a paragraph with a footnote
    createSwDoc("tdf169999.fodt");

    // Initially, the hide condition evaluates to false, so footnote is visible:
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//ftn", 1);
    }

    // Set hide condition to "1"
    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection = xSections->getByName(u"Section1"_ustr).queryThrow<css::beans::XPropertySet>();
    xSection->setPropertyValue(u"Condition"_ustr, css::uno::Any(u"1"_ustr));
    Scheduler::ProcessEventsToIdle();

    // The footnote must get hidden - without the fix, the text failed, because there was a ftn
    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//ftn", 0);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf170846_1)
{
    // In this document, the whole floating table must move to page 2
    createSwDoc("tdf170846_1.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the fix, the next test failed:
    assertXPath(pXmlDoc, "//page[1]//tab", 0); // No tables on page 1
    assertXPath(pXmlDoc, "//page[2]//tab", 1); // One table on page 2
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter6, testTdf170846_2)
{
    // In this document, the whole floating table must move to page 2
    createSwDoc("tdf170846_2.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the fix, the next test failed:
    assertXPath(pXmlDoc, "//page[1]//tab", 0); // No tables on page 1
    assertXPath(pXmlDoc, "//page[2]//tab", 3); // Three tables on page 2
}

} // end of anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
