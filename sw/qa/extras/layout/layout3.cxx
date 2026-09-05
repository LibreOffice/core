/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/metaact.hxx>
#include <editeng/unolingu.hxx>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <rootfrm.hxx>
#include <editeng/brushitem.hxx>
#include <IDocumentLayoutAccess.hxx>

namespace
{
/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter3 : public SwModelTestBase
{
public:
    SwLayoutWriter3()
        : SwModelTestBase(u"/sw/qa/extras/layout/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf134463)
{
    createSwDoc("tdf134463.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 621. The previous paragraph must have zero bottom border.
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/infos/prtBounds", "top", u"21");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf117188)
{
    createSwDoc("tdf117188.docx");
    saveAndReload(TestFilter::ODT);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    OUString sWidth = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "width");
    OUString sHeight = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "height");
    // The text box must have zero border distances
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "left", u"0");
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "top", u"0");
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "width", sWidth);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "height", sHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf117187)
{
    createSwDoc("tdf117187.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // there should be no fly portions
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[@nType='PortionType::Fly']", 0);
}
#if defined _WIN32 && defined _ARM64_
// skip for windows arm64 build
#else
CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf119875)
{
    createSwDoc("tdf119875.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page[2]/body/section[1]", "formatName", u"S10");
    assertXPath(pXmlDoc, "//page[2]/body/section[2]", "formatName", u"S11");
    assertXPath(pXmlDoc, "//page[2]/body/section[3]", "formatName", u"S13");
    assertXPath(pXmlDoc, "//page[2]/body/section[4]", "formatName", u"S14");
    // Sections "S10" and "S13" are hidden -> their frames are zero-height
    assertXPath(pXmlDoc, "//page[2]/body/section[1]/infos/bounds", "height", u"0");
    assertXPath(pXmlDoc, "//page[2]/body/section[3]/infos/bounds", "height", u"0");

    OUString S10Top = getXPath(pXmlDoc, "//page[2]/body/section[1]/infos/bounds", "top");
    OUString S11Top = getXPath(pXmlDoc, "//page[2]/body/section[2]/infos/bounds", "top");
    OUString S13Top = getXPath(pXmlDoc, "//page[2]/body/section[3]/infos/bounds", "top");
    OUString S14Top = getXPath(pXmlDoc, "//page[2]/body/section[4]/infos/bounds", "top");

    CPPUNIT_ASSERT_EQUAL(S10Top, S11Top);
    CPPUNIT_ASSERT_EQUAL(S13Top, S14Top);

    // Section "S11" had the same top value as section "S14", so they overlapped.
    CPPUNIT_ASSERT_LESS(S14Top.toInt32(), S11Top.toInt32());
}
#endif

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf137523)
{
    createSwDoc("tdf137523-1-min.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // the problem was that in the footer, the text frames below the table
    // had wrong height and were not visible
    assertXPath(pXmlDoc, "/root/page/footer/txt[1]/infos/bounds", "height", u"304");
    assertXPath(pXmlDoc, "/root/page/footer/txt[2]/infos/bounds", "height", u"191");
    assertXPath(pXmlDoc, "/root/page/footer/txt[3]/infos/bounds", "height", u"219");
    assertXPath(pXmlDoc, "/root/page/footer/tab/infos/bounds", "height", u"1378");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf120287)
{
    createSwDoc("tdf120287.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin Word-specific compat flag did not imply
    // default-in-Word printer-independent layout, resulting in an additional
    // line break.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testNestedSectionOverlap)
{
    createSwDoc("nested-sections.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    SwTwips nOuterSection1Bottom
        = getXPath(pXmlDoc, "/root/page[1]/body/section[1]/infos/bounds", "bottom").toInt32();
    SwTwips nInnerSection1Top
        = getXPath(pXmlDoc, "/root/page[1]/body/section[2]/infos/bounds", "top").toInt32();
    SwTwips nInnerSection1Bottom
        = getXPath(pXmlDoc, "/root/page[1]/body/section[2]/infos/bounds", "bottom").toInt32();
    SwTwips nOuterSection2Top
        = getXPath(pXmlDoc, "/root/page[1]/body/section[3]/infos/bounds", "top").toInt32();

    SwTwips nOuterSection1TextBottom
        = getXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[13]/infos/bounds", "bottom")
              .toInt32();
    CPPUNIT_ASSERT_EQUAL(nOuterSection1Bottom, nOuterSection1TextBottom);

    SwTwips nInnerSection1TextTop
        = getXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[1]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(nInnerSection1Top, nInnerSection1TextTop);
    SwTwips nInnerSection1TextBottom
        = getXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[2]/infos/bounds", "bottom")
              .toInt32();
    CPPUNIT_ASSERT_EQUAL(nInnerSection1Bottom, nInnerSection1TextBottom);

    SwTwips nOuterSection2TextTop
        = getXPath(pXmlDoc, "/root/page[1]/body/section[3]/txt[1]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_EQUAL(nOuterSection2Top, nOuterSection2TextTop);

    // the problem was that the outer section 2 was positioned above inner 1
    CPPUNIT_ASSERT_EQUAL(nOuterSection1Bottom + 1, nInnerSection1Top);
    CPPUNIT_ASSERT_EQUAL(nInnerSection1Bottom + 1, nOuterSection2Top);
}

auto getXPathIntAttributeValue(xmlXPathContextPtr pXmlXpathCtx, char const* const pXPath)
    -> sal_Int32
{
    xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(pXPath), pXmlXpathCtx);
    CPPUNIT_ASSERT(pXmlXpathObj->nodesetval);
    CPPUNIT_ASSERT_EQUAL(1, xmlXPathNodeSetGetLength(pXmlXpathObj->nodesetval));
    auto ret
        = sal_Int32(xmlXPathCastNodeToNumber(xmlXPathNodeSetItem(pXmlXpathObj->nodesetval, 0)));
    xmlXPathFreeObject(pXmlXpathObj);
    return ret;
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf128966)
{
    createSwDoc("tdf128966-2-min.odt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    xmlXPathObjectPtr pXmlObj
        = getXPathNode(pXmlDoc, "/root/page/body/tab/row/cell[@rowspan > 0][child::txt]");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT(pXmlNodes);
    CPPUNIT_ASSERT_GREATER(300, xmlXPathNodeSetGetLength(pXmlNodes)); // many...

    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc.get());
    registerNamespaces(pXmlXpathCtx);

    for (int i = 0; i < xmlXPathNodeSetGetLength(pXmlNodes); ++i)
    {
        xmlNodePtr pNode = xmlXPathNodeSetItem(pXmlNodes, i);
        xmlXPathSetContextNode(pNode, pXmlXpathCtx);

        OString msg("Cell nr.: " + OString::number(i)
                    + " id=" + OString::number(getXPathIntAttributeValue(pXmlXpathCtx, "@id")));

        auto nCellTop = getXPathIntAttributeValue(pXmlXpathCtx, "infos/bounds/@top");
        auto nCellHeight = getXPathIntAttributeValue(pXmlXpathCtx, "infos/bounds/@height");
        auto nCellCenter = nCellTop + (nCellHeight / 2);

        auto nContentTop
            = getXPathIntAttributeValue(pXmlXpathCtx, "txt[position()=1]/infos/bounds/@top");
        auto nContentBottom = getXPathIntAttributeValue(
            pXmlXpathCtx, "txt[position()=last()]/infos/bounds/@bottom");

        CPPUNIT_ASSERT_MESSAGE(msg.getStr(), nContentTop < nCellCenter);
        CPPUNIT_ASSERT_MESSAGE(msg.getStr(), nContentBottom > nCellCenter);
    }

    xmlXPathFreeContext(pXmlXpathCtx);
    xmlXPathFreeObject(pXmlObj);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTableMoveFwdBadFirstRowPos)
{
    createSwDoc("min2.fodt");

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->Down(false, 18);
    pWrtShell->SplitNode(false);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[2]/body/section/infos/bounds", "top", u"18086");
    assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/infos/bounds", "top", u"18086");
    // the problem was that the row top was at about 28469 and below bottom of the tab frame
    assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/row[1]/infos/bounds", "top", u"18086");
    assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/row[1]/cell[1]/infos/bounds", "top",
                u"18086");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf161718)
{
    createSwDoc("tdf161718.docx");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // everything on one page
    assertXPath(pXmlDoc, "/root/page/header", 1);
    assertXPath(pXmlDoc, "/root/page/header/txt/anchored", 1);
    assertXPath(pXmlDoc, "/root/page/footer", 1);
    assertXPath(pXmlDoc, "/root/page/ftncont/ftn", 1);
    assertXPath(pXmlDoc, "/root/page/ftncont/ftn/txt", 1);
    assertXPath(pXmlDoc, "/root/page/body/txt", 27);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored", 1);
    assertXPath(pXmlDoc, "/root/page", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf119908)
{
    createSwDoc("tdf130088.docx");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Keep real width of the exceeding line portions to calculate shrinking
    sal_Int32 nPortionWidth
        = getXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[2]/SwLinePortion[2]",
                   "width")
              .toInt32();
    // This was 5806 (not real portion width, but stripped to the line width)
    CPPUNIT_ASSERT_GREATER(sal_Int32(5840), nPortionWidth);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf119908_smart_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf119908_smart_hyphenation.odt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();

    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 14 hyphenations on 4 pages (hyphenation slider with default setting)

    // 2 hyphenations on page 1

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Whereas it is essential to promote the development of friendly relations between na");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Whereas a common understanding of these rights and freedoms is of the greatest im");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 2);

    // delete first page to update hyphenation on the next page

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2836, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 3 hyphenations on page 2

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
        u"No one shall be held in slavery or servitude; slavery and the slave trade shall be pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]", "portion",
        u"All are equal before the law and are entitled without any discrimination to equal pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[24]/SwParaPortion/SwLineLayout[2]", "portion",
        u"political crimes or from acts contrary to the purposes and principles of the United Na");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[24]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 3);

    // delete second page to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2336, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2336, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 5 hyphenations on page 3

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]", "portion",
        u"The family is the natural and fundamental group unit of society and is entitled to pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone has the right to freedom of thought, conscience and religion; this right in");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[2]", "portion",
        u"cludes freedom to change his religion or belief, and freedom, either alone or in com");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[3]", "portion",
                u"munity with others and in public or private, to manifest his religion or belief "
                u"in teach");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[3]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone has the right to freedom of opinion and expression; this right includes free");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 5);

    // delete second page to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2646, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2646, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 4 hyphenations on page 4

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone has the right to form and to join trade unions for the protection of his inter");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[2]", "portion",
        u"himself and of his family, including food, clothing, housing and medical care and nec");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[2]", "portion",
        u"and fundamental stages. Elementary education shall be compulsory. Technical and pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]", "portion",
        u"In the exercise of his rights and freedoms, everyone shall be subject only to such limi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf119908_DOCX_smart_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf119908_smart_hyphenation.docx");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();

    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 14 hyphenations on 4 pages (hyphenation slider with default setting)

    // 2 hyphenations on page 1

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Whereas it is essential to promote the development of friendly relations between na");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Whereas a common understanding of these rights and freedoms is of the greatest im");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 2);

    // delete first page to update hyphenation on the next page

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2836, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 3 hyphenations on page 2

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
        u"No one shall be held in slavery or servitude; slavery and the slave trade shall be pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]", "portion",
        u"All are equal before the law and are entitled without any discrimination to equal pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[24]/SwParaPortion/SwLineLayout[2]", "portion",
        u"political crimes or from acts contrary to the purposes and principles of the United Na");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[24]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 3);

    // delete second page to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2336, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2336, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 5 hyphenations on page 3

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]", "portion",
        u"The family is the natural and fundamental group unit of society and is entitled to pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone has the right to freedom of thought, conscience and religion; this right in");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[2]", "portion",
        u"cludes freedom to change his religion or belief, and freedom, either alone or in com");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[3]", "portion",
                u"munity with others and in public or private, to manifest his religion or belief "
                u"in teach");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[3]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone has the right to freedom of opinion and expression; this right includes free");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 5);

    // delete second page to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2646, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2646, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 4 hyphenations on page 4

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone has the right to form and to join trade unions for the protection of his inter");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[2]", "portion",
        u"himself and of his family, including food, clothing, housing and medical care and nec");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[2]", "portion",
        u"and fundamental stages. Elementary education shall be compulsory. Technical and pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]", "portion",
        u"In the exercise of his rights and freedoms, everyone shall be subject only to such limi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf172674_better_spacing)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf172674_better_spacing.odt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();

    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 22 hyphenations on 4 pages (hyphenation slider with maximum better spacing)

    // 2 hyphenations on page 1

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Whereas it is essential to promote the development of friendly relations between na");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Whereas a common understanding of these rights and freedoms is of the greatest im");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 2);

    // delete first page to update hyphenation on the next page

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2836, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 7 hyphenations on page 2

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
        u"No one shall be held in slavery or servitude; slavery and the slave trade shall be pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout[1]", "portion",
        u"No one shall be subjected to torture or to cruel, inhuman or degrading treatment or pun");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]", "portion",
        u"All are equal before the law and are entitled without any discrimination to equal pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[2]", "portion",
                u"tection of the law. All are entitled to equal protection against any "
                u"discrimination in vio");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[2]", "portion",
        u"impartial tribunal, in the determination of his rights and obligations and of any crimi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[23]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone has the right to seek and to enjoy in other countries asylum from persecu");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[23]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[24]/SwParaPortion/SwLineLayout[2]", "portion",
        u"political crimes or from acts contrary to the purposes and principles of the United Na");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[24]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 7);

    // delete second page to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2336, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2336, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 6 hyphenations on page 3

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]", "portion",
        u"The family is the natural and fundamental group unit of society and is entitled to pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone has the right to freedom of thought, conscience and religion; this right in");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[2]", "portion",
        u"cludes freedom to change his religion or belief, and freedom, either alone or in com");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[3]", "portion",
                u"munity with others and in public or private, to manifest his religion or belief "
                u"in teach");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[3]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone has the right to freedom of opinion and expression; this right includes free");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[21]/SwParaPortion/SwLineLayout[1]", "portion",
                u"Everyone, as a member of society, has the right to social security and is "
                u"entitled to real");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[21]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 6);

    // delete second page to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2646, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2646, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 7 hyphenations on page 4

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone has the right to form and to join trade unions for the protection of his inter");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[2]", "portion",
        u"himself and of his family, including food, clothing, housing and medical care and nec");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[2]", "portion",
        u"and fundamental stages. Elementary education shall be compulsory. Technical and pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]", "portion",
                u"Everyone has the right freely to participate in the cultural life of the "
                u"community, to en");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[15]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Everyone is entitled to a social and international order in which the rights and free");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]", "portion",
        u"In the exercise of his rights and freedoms, everyone shall be subject only to such limi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[21]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Nothing in this Declaration may be interpreted as implying for any State, group or per");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[21]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 7);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf172674_less_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf172674_less_hyphenation.odt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();

    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // only 8 hyphenations on 4 pages (hyphenation slider with maximum less hyphenation)

    // 0 hyphenation on page 1

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 0);

    // delete first page to update hyphenation on the next page

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2836, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 2 hyphenations on page 2

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"No one shall be held in slavery or servitude; slavery and the slave trade shall "
                u"be prohib");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]", "portion",
                u"All are equal before the law and are entitled without any discrimination to "
                u"equal protec");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 2);

    // delete second page to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2336, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2336, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 3 hyphenations on page 3

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]", "portion",
                u"The family is the natural and fundamental group unit of society and is entitled "
                u"to protec");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[2]", "portion",
        u"includes freedom to change his religion or belief, and freedom, either alone or in com");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[2]", "portion",
        u"freedom to hold opinions without interference and to seek, receive and impart informa");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[12]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 3);

    // delete second page to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2646, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2646, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 3 hyphenations on page 4

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[2]", "portion",
        u"himself and of his family, including food, clothing, housing and medical care and nec");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[2]", "portion",
        u"and fundamental stages. Elementary education shall be compulsory. Technical and pro");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]", "portion",
        u"In the exercise of his rights and freedoms, everyone shall be subject only to such limi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 3);
}

// check smart hyphenation and compound-based smart hyphenation
CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf119908_smart_hyphenation_hu)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"hu"_ustr, u"HU"_ustr, OUString())))
        return;

    // compound-based hyphenation needs Hunspell morphological analysis
    // (and an up-to-date Hungarian dictionary for full testing, which
    // contains compound splitting information. E.g. dictionary version 1.7
    // didn't contain data for the words "valamint" and "elismerésének")
    uno::Reference<linguistic2::XSpellChecker1> xSpell = LinguMgr::GetSpellChecker();
    LanguageType eLang
        = LanguageTag::convertToLanguageType(lang::Locale(u"hu"_ustr, u"HU"_ustr, OUString()));
    if (!xSpell.is() || !xSpell->hasLanguage(static_cast<sal_uInt16>(eLang)))
        return;

    std::vector<std::pair<OUString, OUString>> aCompound = {
        { u"jogorvoslatért"_ustr, "hy:3" }, { u"valamint"_ustr, "hy:4" },
        { u"alapfokon"_ustr, "pa:fokon" },  { u"jótéteményekben"_ustr, "hy:2" },
        { u"elismerésének"_ustr, "hy:2" },
    };

    // tests supported by the actual spelling dictionary
    bool aTests[5];

    // check Hunspell-based morphological analysis & hyphenation
    bool bHunspell = xSpell->isValid("<?xml?>", static_cast<sal_uInt16>(eLang),
                                     uno::Sequence<beans::PropertyValue>());
    if (bHunspell)
    {
        // check dictionary support of compound splitting
        for (size_t i = 0; i < aCompound.size(); ++i)
        {
            aTests[i] = false;
            // get morphological analysis of the test word
            uno::Reference<css::linguistic2::XSpellAlternatives> xTmpRes = xSpell->spell(
                "<?xml?><query type='analyze'><word>" + aCompound[i].first + "</word></query>",
                static_cast<sal_uInt16>(eLang), uno::Sequence<beans::PropertyValue>());
            if (xTmpRes.is())
            {
                uno::Sequence<OUString> seq = xTmpRes->getAlternatives();
                if (seq.hasElements())
                {
                    sal_Int32 nEndOfFirstAnalysis = seq[0].indexOf("</a>");
                    // use only the first analysis, like the lingucomponent implementation
                    OUString morph(seq[0].copy(0, nEndOfFirstAnalysis));

                    // check compound data
                    aTests[i] = morph.indexOf(aCompound[i].second) > -1;
                }
            }
        }
    }

    createSwDoc("tdf119908_smart_hyphenation_hu.odt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();

    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 53 hyphenations on 5 pages using compound-based smart hyphenation
    // This was 67 without smart hyphenation and 53 without compound-based smart hyphenation
    // (using only the plain or interoperable smart hyphenation).

    // 13 hyphenations on page 1

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Tekintettel arra, hogy az emberiség családja minden egyes tagja méltóságának, vala");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    // "smart" hyphenation: disabled hyphenation of "emberiség" by shrinking
    // This was "emberi-ség"

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Tekintettel arra, hogy az emberi jogok el nem ismerése és semmibevevése az emberiség ");

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Tekintettel annak fontosságára, hogy az emberi jogokat a jog uralma védelmezze, ne");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[2]", "portion",
        u"hogy az ember végső szükségében a zsarnokság és az elnyomás elleni lázadásra kény");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Tekintettel arra, hogy igen lényeges a nemzetek közötti baráti kapcsolatok kifejlődésé");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    // "smart" hyphenation: disabled hyphenation of "tettek" by shrinking
    // This was "tet-tek"

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Tekintettel arra, hogy az Alapokmányban az Egyesült Nemzetek népei újból hitet tettek ");

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[3]", "portion",
        u"egyenjogúsága mellett, valamint kinyilvánították azt az elhatározásukat, hogy elősegí");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[3]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Tekintettel arra, hogy a tagállamok kötelezték magukat arra, hogy az Egyesült Nemze");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout[1]", "portion",
                u"A KÖZGYŰLÉS kinyilvánítja az EMBERI JOGOK EGYETEMES NYILATKOZA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    // "smart" hyphenation: disabled hyphenation of "törekednie" by shifting
    // This was "tö-rekednie"

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout[2]", "portion",
                u"TÁT, mint azt a közös eszményt, amelynek elérésére minden népnek és nemzetnek ");

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Minden emberi lény szabadnak születik, és egyenlő méltósága és joga van. Az embe");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[2]", "portion",
        u"nyelvre, vallásra, politikai vagy más véleményre, nemzeti vagy társadalmi szárma");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[3]", "portion",
                u"zásra, vagyonra, születésre vagy más körülményre vonatkozó mindennemű megkülön");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[3]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[4]", "portion",
        u"böztetés nélkül mindenkit megilletnek. Ezenfelül nem lehet semmiféle megkülönböz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[4]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[5]", "portion",
        u"tetést tenni annak az országnak vagy területnek a politikai, jogi vagy nemzetközi hely");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[5]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[7]", "portion",
        u"terület független, gyámság alatt áll, nem autonóm vagy szuverenitása bármely vonat");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[7]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 13);

    // delete page 1 to update hyphenation on the next page

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2868, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 13 hyphenations on page 2

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Senkit sem lehet rabszolgaságban vagy szolgaságban tartani; a rabszolgaság és a rab");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Senkit sem lehet kínvallatásnak, avagy kegyetlen, embertelen vagy lealacsonyító bá");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[1]", "portion",
        u"A törvény előtt mindenki egyenlő, és minden megkülönböztetés nélkül joga van a tör");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[3]", "portion",
        u"sértő minden megkülönböztetéssel és minden ilyen megkülönböztetésre irányuló fel");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[3]/SwHyphPortion",
                1);

    // compound-based "smart" hyphenation of the compound "jog|orvoslat"
    // This was hyphenated as "jogor-voslat" previously, now "jog-orvoslat"

    if (bHunspell && aTests[0])
    {
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[2]", "portion",
                    u"alapvető jogokat sértő eljárások ellen az illetékes hazai bíróságokhoz "
                    u"tényleges jog");
    }
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[14]/SwParaPortion/SwLineLayout[2]", "portion",
        u"bíróság méltányosan és nyilvánosan tárgyalja, s ez határozzon egyrészt jogai és köte");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[14]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[16]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Minden büntetendő cselekménnyel vádolt személyt ártatlannak kell vélelmezni mind");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[16]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[16]/SwParaPortion/SwLineLayout[2]", "portion",
        u"addig, amíg bűnösségét nyilvánosan lefolytatott perben, a védelméhez szükséges vala");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[16]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[17]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Senkit sem szabad elítélni oly cselekményért vagy mulasztásért, amely elkövetése pil");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[17]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[17]/SwParaPortion/SwLineLayout[3]", "portion",
        u"Ugyancsak nem szabad súlyosabb büntetést kiszabni, mint amely a büntetendő cselek");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[17]/SwParaPortion/SwLineLayout[3]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[19]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Senkinek magánéletébe, családi ügyeibe, otthonába vagy levelezésébe nem szabad ön");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[19]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    // "smart" hyphenation: disabled hyphenation of "minden" by shrinking
    // This was "Min-den"

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[19]/SwParaPortion/SwLineLayout[2]", "portion",
        u"kényesen beavatkozni, sem pedig becsületében vagy jó hírnevében megsérteni. Minden ");

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[21]/SwParaPortion/SwLineLayout[1]", "portion",
                u"Az államon belül minden személynek joga van szabadon mozogni és lakóhelyét sza");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[21]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    // compound-based "smart" hyphenation of the compound "vala|mint"
    // This was hyphenated as "va-lamint" previously)

    if (bHunspell && aTests[1])
    {
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[22]/SwParaPortion/SwLineLayout[1]", "portion",
                    u"Minden személynek joga van minden országot, ideértve saját hazáját is, "
                    u"elhagyni, vala");
    }
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[22]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 13);

    // delete page 2 to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2297, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2297, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 10 hyphenations on page 3

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Erre a jogra nem lehet hivatkozni közönséges bűncselekmény miatti, kellőképpen meg");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]", "portion",
        u"alapozott üldözés, sem pedig az Egyesült Nemzetek céljaival és elveivel ellentétes te");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Senkit sem lehet sem állampolgárságától, sem állampolgársága megváltoztatásának jo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Mind a férfinak, mind a nőnek a nagykorúság elérésétől kezdve joga van fajon, nem");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[2]", "portion",
        u"zetiségen vagy valláson alapuló korlátozás nélkül házasságot kötni és családot alapí");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]", "portion",
                u"Minden személynek joga van a tulajdonhoz, mind egyénileg, mind másokkal együtte");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    // "smart" hyphenation: disabled hyphenation of "valamint" by shifting
    // This was "va-lamint"

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[2]", "portion",
        u"a jog magában foglalja a vallás és a meggyőződés megváltoztatásának szabadságát, ");

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[15]/SwParaPortion/SwLineLayout[1]", "portion",
                u"Minden személynek joga van a vélemény és a kifejezés szabadságához, amely magá");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[15]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[15]/SwParaPortion/SwLineLayout[2]", "portion",
        u"ban foglalja azt a jogot, hogy véleménye miatt ne szenvedjen zaklatást, és hogy hatá");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[15]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    // "smart" hyphenation: disabled hyphenation of "információkat" by shrinking
    // This was "információ-kat"

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[15]/SwParaPortion/SwLineLayout[3]", "portion",
                u"rokra való tekintet nélkül kutathasson, tájékozódhasson és terjeszthessen "
                u"információkat ");

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[22]/SwParaPortion/SwLineLayout[2]", "portion",
        u"rendszeres, az általános és egyenlő szavazati jogot és a titkos szavazást biztosító vá");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[22]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[24]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Minden személynek mint a társadalom tagjának joga van a szociális biztonsághoz, to");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[24]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 10);

    // delete page 3 to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2566, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2566, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 12 hyphenations on page 4

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout[1]", "portion",
                u"Minden személynek joga van a munkához, a munka szabad megválasztásához, a mél");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]", "portion",
                u"Mindenkinek joga van egyenlő munkáért egyenlő bért kapni, bárminemű megkülön");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Minden személynek joga van a pihenéshez, a szabadidőhöz, beleértve a munkaidő ész");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Minden személynek joga van saját maga és családja egészségének és jólétének bizto");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[3]", "portion",
        u"ellátáshoz, valamint a szükséges szociális szolgáltatásokhoz, továbbá joga van a szoci");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout[3]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout[2]", "portion",
        u"gyermek, akár házasságból, akár házasságon kívül született, azonos szociális védelem");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    // compound-based "smart" hyphenation of the compound "alap|fokon"
    // This was hyphenated as "alapfo-kon" previously, now "alap-fokon"

    if (bHunspell && aTests[2])
    {
        assertXPath(
            pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[1]", "portion",
            u"Minden személynek joga van az oktatáshoz. Az oktatásnak legalábbis elemi és alap");
    }
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[13]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[14]/SwParaPortion/SwLineLayout[1]", "portion",
        u"A nevelésnek az emberi személyiség teljes kibontakoztatására, valamint az emberi jo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[14]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[14]/SwParaPortion/SwLineLayout[4]", "portion",
        u"közötti megértést, türelmet és barátságot, valamint támogatnia kell az Egyesült Nem");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[14]/SwParaPortion/SwLineLayout[4]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[15]/SwParaPortion/SwLineLayout[1]", "portion",
        u"A szülőket elsőbbségi jog illeti meg a gyermekeiknek adandó oktatás megválasztásá");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[15]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    // compound-based "smart" hyphenation of the compound "jó|téteményekben"
    // This wasn't hyphenated previously, now "jó-téteményekben"

    if (bHunspell && aTests[3])
    {
        assertXPath(
            pXmlDoc, "/root/page[1]/body/txt[17]/SwParaPortion/SwLineLayout[2]", "portion",
            u"a művészetek élvezéséhez, valamint a tudomány haladásában és az abból származó jó");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[17]/SwParaPortion/SwLineLayout[2]/SwHyphPortion", 1);
    }

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Mindenkinek joga van minden általa alkotott tudományos, irodalmi és művészeti ter");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[18]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 13);

    // delete page 4 to update hyphenation on the next page

    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2766, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2766, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // 5 hyphenations on page 5

    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"Minden személynek joga van ahhoz, hogy mind a társadalmi, mind a nemzetközi vi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]", "portion",
        u"szonyok tekintetében olyan rendszer uralkodjék, amelyben a jelen Nyilatkozatban ki");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/SwHyphPortion",
                1);

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Jogainak gyakorlásában és szabadságainak élvezetében mindenki csak olyan korláto");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    // compound-based "smart" hyphenation of the compound "el|ismerésének"
    // This wasn't hyphenated by the plain, i.e. not compound-based "smart" hyphenation previously.

    if (bHunspell && aTests[4])
    {
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[2]", "portion",
                    u"zásoknak lehet alávetve, amelyeket a törvény kizárólag mások jogai és "
                    u"szabadságai el");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout[2]/SwHyphPortion", 1);
    }

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]", "portion",
        u"Ezeket a jogokat és szabadságokat semmi esetre sem lehet az Egyesült Nemzetek cél");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout[1]/SwHyphPortion",
                1);

    // "smart" hyphenation: disabled hyphenation of "állam" by shifting
    // This was "ál-lam"

    assertXPath(
        pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout[1]", "portion",
        u"A jelen Nyilatkozat egyetlen rendelkezése sem értelmezhető úgy, hogy az valamely ");

    if (bHunspell && aTests[4])
        assertXPath(pXmlDoc, "/root/page[1]/body/txt/SwParaPortion/SwLineLayout/SwHyphPortion", 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf158333)
{
    createSwDoc("tdf130088.docx");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // shrink line 2
    assertXPath(
        pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[2]", "portion",
        u"viverra odio. Donec auctor molestie sem, sit amet tristique lectus hendrerit sed. ");

    // shrink line 7
    assertXPath(
        pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[7]", "portion",
        // This was "...diam ", not "...diam tempor "
        u"laoreet vel leo nec, volutpat facilisis eros. Donec consequat arcu ut diam tempor ");

    // shrink line 2 of paragraph 2
    assertXPath(
        pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[2]", "portion",
        // This was "...Cras ", not "...Cras sodales "
        u"Donec auctor molestie sem, sit amet tristique lectus hendrerit sed. Cras sodales ");

    // shrink line 2 of paragraph 4
    assertXPath(pXmlDoc, "/root/page/body/txt[4]/SwParaPortion/SwLineLayout[2]", "portion",
                // This was "...et ", not "...et magnis "
                u"consequat arcu ut diam tempor luctus. Cum sociis natoque penatibus et magnis ");

    // tdf#158776 don't shrink line 11 of paragraph 4
    assertXPath(pXmlDoc, "/root/page/body/txt[4]/SwParaPortion/SwLineLayout[11]", "portion",
                // This was "...quis curcus ", not "...quis "
                u"venenatis, quis commodo dolor posuere. Curabitur dignissim sapien quis ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf159085)
{
    createSwDoc("tdf159085.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // This was "... cursus" instead of breaking the word at soft hyphen
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"venenatis, quis commodo dolor posuere. Curabitur dignissim sapien quis cur­");

    // This was "... cursus" instead of breaking the word at soft hyphen
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]", "portion",
                u"venenatis, quis commodo dolor posuere. Curabitur dignissim sapien quis cur­");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf158419)
{
    createSwDoc("tdf130088.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = getSwDocShell();

    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // second paragraph.
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    SwPosition aPosition(*pWrtShell->GetCursor()->Start());
    SwTwips nSecondParaLeft
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds", "left").toInt32();
    SwTwips nSecondParaWidth
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds", "width").toInt32();
    SwTwips nSecondParaTop
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds", "top").toInt32();
    SwTwips nSecondParaHeight
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds", "height").toInt32();
    Point aPoint;

    // click at the end of the second line of the second paragraph
    // (a line shrunk by the new justification)

    aPoint.setX(nSecondParaLeft + nSecondParaWidth);
    aPoint.setY(nSecondParaTop + (nSecondParaHeight / 6) * 1.5);
    SwCursorMoveState aState(CursorMoveState::NONE);
    pLayout->GetModelPositionForViewPoint(&aPosition, aPoint, &aState);
    // Without the accompanying fix in place, this test would have failed: character position was 155,
    // i.e. cursor was before the end of the paragraph.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(156), aPosition.GetContentIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf163042)
{
    createSwDoc("tdf163042.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = getSwDocShell();

    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 1-line paragraph
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    SwPosition aPosition(*pWrtShell->GetCursor()->Start());
    SwTwips nParaLeft = getXPath(pXmlDoc, "/root/page/body/txt[1]/infos/bounds", "left").toInt32();
    SwTwips nParaWidth
        = getXPath(pXmlDoc, "/root/page/body/txt[1]/infos/bounds", "width").toInt32();
    SwTwips nParaTop = getXPath(pXmlDoc, "/root/page/body/txt[1]/infos/bounds", "top").toInt32();
    SwTwips nParaHeight
        = getXPath(pXmlDoc, "/root/page/body/txt[1]/infos/bounds", "height").toInt32();
    Point aPoint;

    // click before the last but one character of the paragraph
    // (in a line shrunk by the new space shrinking justification)

    aPoint.setX(nParaLeft + nParaWidth - 2 * nParaWidth / 160);
    aPoint.setY(nParaTop + nParaHeight * 0.5);
    SwCursorMoveState aState(CursorMoveState::NONE);
    pLayout->GetModelPositionForViewPoint(&aPosition, aPoint, &aState);
    // Without the accompanying fix in place, this test would have failed: character position was 160,
    // i.e. cursor was at the end of the paragraph instead of the last but one character
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(158), aPosition.GetContentIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf163060)
{
    createSwDoc("tdf163060.fodt");

    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // There is only a single shrunk line 1, without breaking the last word
    // before the last text portion "i"

    // This ends in "dolorsi" (not "dolors", as before)
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"Quis pretium semper. Proin luctus orci a neque venenatis, quis commodo dolorsi");

    // no second line (there was a second line with the text portion "i").
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf162109)
{
    createSwDoc("tdf162109.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // There was no SwGluePortion, because of missing justification of the last paragraph line,
    // despite it is a full line with shrunk spaces
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwGluePortion");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf162220)
{
    createSwDoc("tdf162220.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // There was no SwGluePortion, because of missing justification of the last paragraph line,
    // despite it is a full line with shrunk spaces
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwGluePortion");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf163720)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf163720.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first text array action
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 101 chars on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(101), pDXArray.size());

            // Assert we are using the expected position for the last char
            // This was 10093, now 10003, according to the less shrinking,
            // than needed for the extra hyphen glyph at hyphenation
            CPPUNIT_ASSERT_LESS(sal_Int32(10010), sal_Int32(pDXArray[100]));
            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf162725)
{
    createSwDoc("tdf162725.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // There was no SwGluePortion, because of missing justification of the last paragraph line,
    // despite it is a full line with shrunk spaces
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwGluePortion");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testHiddenParagraphMarkPerLineProperties)
{
    createSwDoc("min-wtf.rtf");

    // ensure paragraphs are merged by hidden formatting
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetViewMetaChars(false);
    pWrtShell->ApplyViewOptions(aViewOptions);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/header/txt", 3);
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[1]", "height",
                u"184");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[2]", "height",
                u"184");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[3]", "height",
                u"184");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[4]", "height",
                u"184");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[5]", "height",
                u"184");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout", 5);
    // the problem was that this was 1656 due to using wrong line spacing for last line
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/infos/bounds", "height", u"1499");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[4]/SwFixPortion",
                "type", u"PortionType::TabLeft");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[4]/SwFixPortion",
                "width", u"532");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[5]/SwFixPortion",
                "type", u"PortionType::TabLeft");
    // the problem was that this was 626 (same tab position as previous line)
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[5]/SwFixPortion",
                "width", u"2755");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout", 0);
    // this one was also wrong (1968) due to wrong spacing-below
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/infos/bounds", "height", u"411");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout[1]", "height",
                u"265");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout", 1);
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/infos/bounds", "height", u"492");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf161810)
{
    createSwDoc("tdf161810.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first text array action
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 70 chars on the first line
            // (tdf#164499 no space shrinking in lines with tabulation)
            CPPUNIT_ASSERT_EQUAL(size_t(70), pDXArray.size());

            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf164905)
{
    createSwDoc("tdf164905.docx");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 9 (resulting broken ToC layout)
    assertXPath(pXmlDoc, "//SwGluePortion", 3);
    // For example, it was an unnecessary glue portion here
    assertXPath(pXmlDoc,
                "/root/page/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/SwGluePortion", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf163149)
{
    createSwDoc("tdf163149.docx");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the text array action for the second non-empty (shrunk) line
    bool bFirst = true;
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // skip empty paragraphs
            if (pDXArray.size() <= 1)
                continue;

            // skip first non-empty line
            if (bFirst)
            {
                bFirst = false;
                continue;
            }

            // There should be 46 chars on the second line
            CPPUNIT_ASSERT_EQUAL(size_t(46), pDXArray.size());

            // Assert we are using the expected position for the last char
            // This was 4673, now 4163, according to the fixed space shrinking
            CPPUNIT_ASSERT_LESS(sal_Int32(4250), sal_Int32(pDXArray[45]));
            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf167648)
{
    createSwDoc("tdf167648.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first text array action
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 27 characters on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(27), pDXArray.size());

            // Assert we are using the expected position for the
            // second character of the first word with enlarged letter-spacing
            // This was 286, now 320, according to the 25% maximum letter spacing
            CPPUNIT_ASSERT_GREATER(sal_Int32(315), sal_Int32(pDXArray[1]));
            CPPUNIT_ASSERT_LESS(sal_Int32(325), sal_Int32(pDXArray[1]));

            // first character of the second word nearer to the left side
            // because of the narrower spaces
            // This was 977, now 965, according to the 25% maximum letter spacing
            CPPUNIT_ASSERT_LESS(sal_Int32(970), sal_Int32(pDXArray[5]));
            CPPUNIT_ASSERT_GREATER(sal_Int32(960), sal_Int32(pDXArray[5]));
            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf167648_minimum)
{
    createSwDoc("tdf167648_minimum.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first text array action
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 39 characters on the first line
            // This was 27 characters, but setting minimum letter spacing
            // to -25% allows more words in the line
            CPPUNIT_ASSERT_EQUAL(size_t(39), pDXArray.size());

            // Assert we are using the expected position for the
            // second character of the first word with enlarged letter-spacing
            // This was 286, now 266, according to the -25% minimum letter spacing
            CPPUNIT_ASSERT_LESS(sal_Int32(270), sal_Int32(pDXArray[1]));

            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf169168)
{
    createSwDoc("tdf169168.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first text array action
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 11 characters in the first portion on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(11), pDXArray.size());

            // Assert we are using the expected position for the
            // second character of the first word with enlarged letter-spacing
            // This was 286, now 320, according to the 25% maximum letter spacing
            CPPUNIT_ASSERT_GREATER(sal_Int32(315), sal_Int32(pDXArray[1]));
            CPPUNIT_ASSERT_LESS(sal_Int32(325), sal_Int32(pDXArray[1]));

            // first character of the second word nearer to the left side
            // because of the narrower spaces
            // This was 977, now 965, according to the 25% maximum letter spacing
            CPPUNIT_ASSERT_LESS(sal_Int32(970), sal_Int32(pDXArray[5]));
            CPPUNIT_ASSERT_GREATER(sal_Int32(960), sal_Int32(pDXArray[5]));
            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf169168_minimum)
{
    createSwDoc("tdf169168_minimum.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first text array action
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 5 characters in the first portion on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(5), pDXArray.size());

            // Assert we are using the expected position for the
            // second character of the first word with enlarged letter-spacing
            // This was 286, now 266, according to the -25% minimum letter spacing
            CPPUNIT_ASSERT_LESS(sal_Int32(270), sal_Int32(pDXArray[1]));

            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf168251)
{
    createSwDoc("tdf168251.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first text array action
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 39 characters on the first line
            // This was 27 characters, but setting minimum glyph scaling
            // to 99% allows more words in the line
            CPPUNIT_ASSERT_EQUAL(size_t(39), pDXArray.size());

            // Assert we are using the expected position for the
            // second character of the first word with enlarged letter-spacing
            // This was 286, now 266, according to the -25% minimum letter spacing
            CPPUNIT_ASSERT_LESS(sal_Int32(270), sal_Int32(pDXArray[1]));

            break;
        }
    }

    // Find the fourth text array action
    int nLine = 0;
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            if (++nLine < 6)
                continue;

            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 35 characters on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(35), pDXArray.size());

            // Assert we are using the expected position for the
            // second character of the first word with enlarged glyph width
            // This was 238, now 251, according to the 110% maximum glyph scaling
            // (and no changes in letter spacing)
            CPPUNIT_ASSERT_GREATER(sal_Int32(245), sal_Int32(pDXArray[1]));

            // Assert we are using the expected position for the
            // first character of the last word with enlarged glyph width
            // This was 3689, now 3667, according to the 110% maximum glyph scaling
            CPPUNIT_ASSERT_LESS(sal_Int32(3675), sal_Int32(pDXArray[30]));

            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf171161)
{
    createSwDoc("tdf171161.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first text array action
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 2 characters in the first portion on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(2), pDXArray.size());

            // Assert we are using the expected position for the
            // second character of the first word with narrow scale width.
            // This was 17490, now 8745, according to the 50% letter scaling
            CPPUNIT_ASSERT_LESS(sal_Int32(8800), sal_Int32(pDXArray[1]));

            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf38159)
{
    createSwDoc("tdf38159.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Test paragraph composer without limiting maximum word spacing
    // (Desired and Maximum word spacing are not different)
    //
    //     BEFORE                      AFTER
    //
    //     Lorem ipsum dolor sit  ->   Lorem   ipsum   dolor
    //     amet,     consectetur       sit amet, consectetur
    //
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"Lorem ipsum dolor ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                u"sit amet, consectetur ");

    //     BEFORE                            AFTER
    //
    //     semper. Proin luctus orci ac  ->  semper. Proin luctus orci
    //     neque     venenatis,    quis      ac neque venenatis,  quis
    //
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[5]", "portion",
                u"semper. Proin luctus orci ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[6]", "portion",
                u"ac neque venenatis, quis ");

    //     BEFORE                            AFTER
    //
    //     cursus mauris vitae ligula  ->    cursus   mauris    vitae
    //     pellentesque,          non        ligula pellentesque, non
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[14]", "portion",
                u"cursus mauris vitae ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[15]", "portion",
                u"ligula pellentesque, non ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf38159_disabled)
{
    createSwDoc("tdf38159_disabled.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Test disabled paragraph composer
    //
    //     DISABLED                    ENABLED
    //
    //     Lorem ipsum dolor sit  ->   Lorem   ipsum   dolor
    //     amet,     consectetur       sit amet, consectetur
    //
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"Lorem ipsum dolor sit ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                u"amet, consectetur ");

    //     DISABLED                          ENABLED
    //
    //     semper. Proin luctus orci ac  ->  semper. Proin luctus orci
    //     neque     venenatis,    quis      ac neque venenatis,  quis
    //
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[5]", "portion",
                u"semper. Proin luctus orci ac ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[6]", "portion",
                u"neque venenatis, quis ");

    //     DISABLED                          ENABLED
    //
    //     cursus mauris vitae ligula  ->    cursus   mauris    vitae
    //     pellentesque,          non        ligula pellentesque, non
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[14]", "portion",
                u"cursus mauris vitae ligula ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[15]", "portion",
                u"pellentesque, non ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf38159_limited)
{
    createSwDoc("tdf38159_limited.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Test paragraph composer limited by maximum word spacing
    // (Maximum word spacing is greater than the desired word spacing)
    //
    //     LIMITED BY MAXIMUM WORD SPACING=300%
    //
    //     Lorem ipsum dolor sit  <-   Lorem   ipsum   dolor
    //     amet,     consectetur       sit amet, consectetur
    //
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"Lorem ipsum dolor sit ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                u"amet, consectetur ");

    //                                       NOT LIMITED BY MAXIMUM WORD SPACING=300%
    //
    //     semper. Proin luctus orci ac  ->  semper. Proin luctus orci
    //     neque     venenatis,    quis      ac neque venenatis,  quis
    //
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[5]", "portion",
                u"semper. Proin luctus orci ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[6]", "portion",
                u"ac neque venenatis, quis ");

    //     LIMITED BY MAXIMUM WORD SPACING=300%
    //
    //     cursus mauris vitae ligula  <-    cursus   mauris    vitae
    //     pellentesque,          non        ligula pellentesque, non
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[14]", "portion",
                u"cursus mauris vitae ligula ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[15]", "portion",
                u"pellentesque, non ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf168705)
{
    createSwDoc("tdf168705.fodt");
    SwDoc* pDoc(getSwDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // check multi-column section in Normal View
    assertXPath(pXmlDoc, "/root/page/body/section/column", 3);

    // set Draft View and check single-column section
    dispatchCommand(mxComponent, u".uno:DraftView"_ustr, {});
    calcLayout();
    pXmlDoc = parseLayoutDump();

    // check single-column sections in Draft View
    assertXPath(pXmlDoc, "/root/page/body/section", 1);
    assertXPath(pXmlDoc, "/root/page/body/section/column", 0);

    // set Normal View again and check multi-column section
    dispatchCommand(mxComponent, u".uno:PrintLayout"_ustr, {});
    calcLayout();
    pXmlDoc = parseLayoutDump();

    // check multi-column section in Normal View
    assertXPath(pXmlDoc, "/root/page/body/section/column", 3);

    // set Web View and check multi-column section
    dispatchCommand(mxComponent, u".uno:BrowseView"_ustr, {});
    calcLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/section/column", 3);

    // set Draft View again and check single-column section
    dispatchCommand(mxComponent, u".uno:DraftView"_ustr, {});
    calcLayout();
    pXmlDoc = parseLayoutDump();

    // check single-column sections in Draft View
    assertXPath(pXmlDoc, "/root/page/body/section", 1);
    assertXPath(pXmlDoc, "/root/page/body/section/column", 0);

    // set Web View again and check multi-column section
    dispatchCommand(mxComponent, u".uno:BrowseView"_ustr, {});
    calcLayout();
    pXmlDoc = parseLayoutDump();

    // check multi-column section in Normal View
    assertXPath(pXmlDoc, "/root/page/body/section/column", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf169168_scaling)
{
    createSwDoc("tdf169168_scaling.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first text array action
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 11 characters in the first portion on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(11), pDXArray.size());

            // Assert we are using the expected position for the
            // second character of the first word with enlarged letter-spacing
            // This was 286, now 266, according to the -25% minimum letter spacing
            CPPUNIT_ASSERT_LESS(sal_Int32(270), sal_Int32(pDXArray[1]));

            break;
        }
    }

    // Find the fourth text array action
    int nLine = 0;
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            if (++nLine < 9)
                continue;

            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 35 characters on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(35), pDXArray.size());

            // Assert we are using the expected position for the
            // second character of the first word with enlarged glyph width
            // This was 238, now 251, according to the 110% maximum glyph scaling
            // (and no changes in letter spacing)
            CPPUNIT_ASSERT_GREATER(sal_Int32(245), sal_Int32(pDXArray[1]));

            // Assert we are using the expected position for the
            // first character of the last word with enlarged glyph width
            // This was 3689, now 3667, according to the 110% maximum glyph scaling
            CPPUNIT_ASSERT_LESS(sal_Int32(3675), sal_Int32(pDXArray[30]));

            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf168351)
{
    createSwDoc("tdf168351.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first text array action
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 38 characters on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(38), pDXArray.size());

            // This was 830, now 789, according to the applied negative letter spacing
            CPPUNIT_ASSERT_LESS(sal_Int32(800), sal_Int32(pDXArray[7]));

            // restore desired word spacing between the first two words
            // This was -6.5 (missing word spacing), now 57,
            // according to the applied letter spacing
            CPPUNIT_ASSERT_GREATER(sal_Int32(50), sal_Int32(pDXArray[9]) - sal_Int32(pDXArray[8]));

            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf168448)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf168448.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first two text array actions (associated to the first text line)
    bool bFirstArray = true;
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);

        // check letter spacing in the first line (in the first text array)
        if (bFirstArray && pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 25 characters on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(25), pDXArray.size());

            // Assert we are using the expected position for the
            // last character of the first word with enlarged letter-spacing
            // This was 750, now 786, according to the enabled maximum letter spacing
            CPPUNIT_ASSERT_GREATER(sal_Int32(770), sal_Int32(pDXArray[4]));

            // first character of the second word is there after a space
            CPPUNIT_ASSERT_GREATER(sal_Int32(877), sal_Int32(pDXArray[5]));

            bFirstArray = false;
            continue;
        }

        // check hyphen position of the first line (in the second text array)
        if (!bFirstArray && pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 1 character, the hyphen of the first line
            CPPUNIT_ASSERT_EQUAL(size_t(1), pDXArray.size());

            // This was 3662 (at enabled letter spacing for the hyphenated line),
            // now 4149, according to the fixed hyphen position
            auto nX = pTextArrayAction->GetPoint().X();
            CPPUNIT_ASSERT_GREATER(sal_Int32(4100), sal_Int32(nX));

            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf169168_hyphen)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf169168_hyphen.fodt");
    // Ensure that all text portions are calculated before testing.
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Find the first two text array actions (associated to the first text line)
    int nArray = 0;
    for (size_t nAction = 0; nAction < xMetaFile->GetActionSize(); nAction++)
    {
        auto pAction = xMetaFile->GetAction(nAction);

        // check letter spacing in the first line (in the first text array)
        if (nArray == 0 && pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 11 characters in the first portion on the first line
            CPPUNIT_ASSERT_EQUAL(size_t(11), pDXArray.size());

            // Assert we are using the expected position for the
            // last character of the first word with enlarged letter-spacing
            // This was 750, now 786, according to the enabled maximum letter spacing
            CPPUNIT_ASSERT_GREATER(sal_Int32(770), sal_Int32(pDXArray[4]));

            // first character of the second word is there after a space
            CPPUNIT_ASSERT_GREATER(sal_Int32(877), sal_Int32(pDXArray[5]));
        }

        // check hyphen position of the first line (in the forth text array)
        if (pAction->GetType() == MetaActionType::TEXTARRAY)
        {
            if (++nArray < 7)
                continue;

            auto pTextArrayAction = static_cast<MetaTextArrayAction*>(pAction);
            auto pDXArray = pTextArrayAction->GetDXArray();

            // There should be 1 character, the hyphen of the first line
            CPPUNIT_ASSERT_EQUAL(size_t(1), pDXArray.size());

            // This was 3662 (at enabled letter spacing for the hyphenated line),
            // now 4149, according to the fixed hyphen position
            auto nX = pTextArrayAction->GetPoint().X();
            CPPUNIT_ASSERT_GREATER(sal_Int32(4100), sal_Int32(nX));

            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf164499)
{
    createSwDoc("tdf164499.docx");

    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // no shrinking in tabulated text lines

    // This was 1 (no line break in heading 2.5.5)
    assertXPath(pXmlDoc, "/root/page[1]/body/section/txt[18]/SwParaPortion/SwLineLayout", 2);
    // line break in heading 2.5.5: the second line contains only the page number
    assertXPath(pXmlDoc, "/root/page[1]/body/section/txt[18]/SwParaPortion/SwLineLayout[2]",
                "portion", u"*1");
}

// FIXME The test passes on most machines. Need to figure out what goes wrong and fix that
// (at least, replacing Times New Roman with the metric-equivalent Liberation).
#if 0
CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf132599_always)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf132599_always.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 2nd paragraph: hyphenated last full line
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[2]", "portion",
                u"ent to any other celes");

    // hyphenation-keep-type='always'
    // 3rd paragraph: not hyphenated last full line of the hyphenated paragraph
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout[2]", "portion",
                u"ent to any other ");
}
#endif

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf132599_frames_on_same_page_no_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf132599_frames_on_same_page_no_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="column"
    // 2nd frame: shifted hyphenated line (no hyphenation at the end of the first frame)
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly[2]/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"space, ex");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf132599_frames_on_same_page_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf132599_frames_on_same_page_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="page"
    // 2nd frame: not shifted hyphenated line (hyphenation at the end of the first frame),
    // This was "space, ex" (bad shifting)
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly[2]/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"cept that it ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf132599_frames_on_right_pages_no_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf132599_frames_on_right_pages_no_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="spread"
    // 2nd frame: shifted hyphenated line
    // This was "cept that it" (missing shifting)
    assertXPath(pXmlDoc, "/root/page[3]/body/txt/anchored/fly/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"space, ex");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf132599_frames_on_spread_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf132599_frames_on_spread_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="spread"
    // 2nd frame on left page and 3rd frame on right page -> not shifted hyphenated line
    // 2nd frame: not shifted hyphenated line (hyphenation at the end of the first frame),
    assertXPath(pXmlDoc, "/root/page[3]/body/txt/anchored/fly/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"cept that it ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_page)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_page.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-line="true"
    // first: shifted hyphenated word
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout[9]", "portion",
                u"except that it has an ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_spread)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_spread.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-line="true"
    // first: shifted hyphenated word at end of the spread (right page)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout[9]", "portion",
                u"except that it has an ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_spread_left_page)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_spread-left-page.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-line="true"
    // first: no shifted hyphenated word at the end of the first page of the spread (left page)
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/SwParaPortion/SwLineLayout[9]", "portion",
                u"except that it has an at");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_column)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_column.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="column" hyphenation-keep-line="true"
    // 2nd frame: shifted hyphenated word (no hyphenation at the end of the first column)
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section/column[2]/body/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"iner");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_page_in_last_column)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_page_in_last_column.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="page" hyphenation-keep-line="true"
    // the end line of the first page is a column boundary,
    // but at the page boundary, too, so disable its hyphenation
    // 2nd frame: shifted hyphenated word (no hyphenation at the end of the first column)
    assertXPath(pXmlDoc,
                "/root/page[2]/body/section/column[1]/body/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"iner");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_page_in_not_last_column)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_page_in_not_last_column.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="page" hyphenation-keep-line="true"
    // 2nd frame: no shifted hyphenated word (hyphenation at the end of the first column)
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section/column[2]/body/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"tially. ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_page_in_table)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_page_in_table.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="page" hyphenation-keep-line="true"
    // 2nd frame: no shifted hyphenated word (hyphenation at the end of the first column)
    assertXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"atmosphere. The Earth ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_frames_on_same_page_no_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_frames_on_same_page_no_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="column" hyphenation-keep-line="true"
    // 2nd frame: shifted hyphenated word (no hyphenation at the end of the first frame)
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly[2]/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"except that ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_frames_on_same_page_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_frames_on_same_page_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="page" hyphenation-keep-line="true"
    // 2nd frame: not shifted hyphenated word (hyphenation at the end of the first frame),
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly[2]/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"cept that it ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_frames_on_spread_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_frames_on_spread_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="spread" hyphenation-keep-line="true"
    // 2nd frame on left page and 3rd frame on right page -> not shifted hyphenated word
    // 2nd frame: not shifted hyphenated word (hyphenation at the end of the first frame),
    assertXPath(pXmlDoc, "/root/page[3]/body/txt/anchored/fly/txt/SwParaPortion/SwLineLayout[1]",
                "portion", u"cept that it ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, tdf165354_frames_on_right_pages_no_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_frames_on_right_pages_no_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-line="true"
    // It was "space, ex": missing layout update of the last line with disabled hyphenation
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly/txt/SwParaPortion/SwLineLayout[12]",
                "portion", u"space, ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_long_paragraph)
{
    // disabled hyphenation on page 1 (no hyphenation at all on page 2,
    // only in first line of page 3, which resulted broken layout)
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_long_paragraph.fodt");
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

    assertXPath(pXmlDoc, "/root/page[2]/body/txt/SwParaPortion/SwLineLayout[12]", "portion",
                u"of the Earth is space, ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf165354_long_paragraph_2)
{
    // disabled hyphenation on page 1 and page 2
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf165354_long_paragraph_2.fodt");
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

    // disabled hyphenation by loext:hyphenation-keep-type="page"
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/SwParaPortion/SwLineLayout[12]", "portion",
                u"of the Earth is space ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testDropFlyOverlap)
{
    // Load the overlapping fly frame document
    createSwDoc("drop_fly_overlap.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert that the layout engine successfully processed the Drop Portion
    assertXPath(pXmlDoc,
                "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/"
                "SwLinePortion[@type='PortionType::Drop']",
                1);

    // Extract the calculated widths as integers
    sal_Int32 nDropWidth = getXPath(pXmlDoc,
                                    "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/"
                                    "SwLinePortion[@type='PortionType::Drop']",
                                    "width")
                               .toInt32();
    sal_Int32 nFlyWidth
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/SwAnchoredDrawObject/bounds", "width")
              .toInt32();

    // Assert that the Drop Cap calculation was safely constrained by the intersecting shape.
    CPPUNIT_ASSERT_GREATER(sal_Int32(600), nDropWidth);
    CPPUNIT_ASSERT_LESS(sal_Int32(700), nDropWidth);

    CPPUNIT_ASSERT_GREATER(sal_Int32(600), nFlyWidth);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testDropAsianWord)
{
    // Trigger the ASIAN script case in GetDropLen
    createSwDoc("drop_asian_word.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert that the drop portion exists
    assertXPath(pXmlDoc,
                "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/"
                "SwLinePortion[@type='PortionType::Drop']",
                1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testDropComplexWord)
{
    // Trigger the COMPLEX script case in GetDropLen
    createSwDoc("drop_complex_word.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert that the drop portion exists
    assertXPath(pXmlDoc,
                "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/"
                "SwLinePortion[@type='PortionType::Drop']",
                1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testDropVertical)
{
    // Trigger the IsVertical() layout calculations for Top-to-Bottom typography
    createSwDoc("drop_vertical.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert the Drop Cap was calculated in the vertical layout tree
    // (Since we put it in a frame, we search inside the //fly node)
    assertXPath(pXmlDoc,
                "//fly/txt/SwParaPortion/SwLineLayout[1]/SwLinePortion[@type='PortionType::Drop']",
                1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testHeaderImageAlignment)
{
    createSwDoc("testHeaderImageAlignment.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // The logo is a floating text frame anchored in the first header paragraph.
    assertXPath(pXmlDoc, "/root/page/header/txt", 4);
    assertXPath(pXmlDoc, "/root/page/header/txt[1]/anchored/fly", 1);

    // Without the fix there was no fly portion and the text ran under the logo.
    assertXPath(pXmlDoc,
                "/root/page/header/txt[1]/SwParaPortion/SwLineLayout/"
                "SwFixPortion[@type='PortionType::Fly']",
                1);

    const sal_Int32 nFlyGap = getXPath(pXmlDoc,
                                       "/root/page/header/txt[1]/SwParaPortion/SwLineLayout/"
                                       "SwFixPortion[@type='PortionType::Fly']",
                                       "width")
                                  .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(1000), nFlyGap);

    // The separator paragraph stays BELOW the logo instead of riding up across it.
    const sal_Int32 nLogoBottom
        = getXPath(pXmlDoc, "/root/page/header/txt[1]/anchored/fly/infos/bounds", "bottom")
              .toInt32();
    const sal_Int32 nSeparatorTop
        = getXPath(pXmlDoc, "/root/page/header/txt[4]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_GREATER(nLogoBottom, nSeparatorTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testSmallCapsLigature)
{
    // Trigger bCaseMapLengthDiffers layout math using the ﬄ -> FFL character
    createSwDoc("smallcaps_ligature.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // The first line must contain the ffl ligature intact.
    // A regression in bCaseMapLengthDiffers would corrupt this portion string.
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion", u"Maﬄb");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testHiddenTextFieldExpansion)
{
    // Trigger the base class SwExpandPortion logic
    // A Hidden Text field that evaluates to false is replaced with an empty SwExpandPortion.
    createSwDoc("hidden_text_field.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // We expect the layout engine to process the field as a generic Portion
    // Because it is hidden, its width must be zero.
    assertXPath(pXmlDoc,
                "/root/page/body/txt/SwParaPortion/SwLineLayout"
                "/SwFieldPortion[@type='PortionType::Hidden' and @width='0']",
                1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testDoubleLineBrackets)
{
    // Trigger SwDoubleLinePortion initialization for Asian "Two Lines in One" layout.
    createSwDoc("double_line_bracket.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert that the layout engine successfully created the Double Line Portion
    // for the full "Hello World" run.
    assertXPath(pXmlDoc, "//SwMultiPortion[contains(@symbol, 'SwDoubleLinePortion')]", 1);

    // Verify it correctly split the content into its 2 stacked SwLineLayout rows
    // ("Hello " / "World"), confirming the two-lines-in-one layout was built.
    assertXPath(pXmlDoc, "//SwMultiPortion[contains(@symbol, 'SwDoubleLinePortion')]/SwLineLayout",
                2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testHardBlankOverflow)
{
    createSwDoc("hard_blank_overflow.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert that the layout engine successfully processed the overflow
    // by breaking it into at least two SwLineLayout lines
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testDropAdjustCenter)
{
    createSwDoc("drop_adjust_center.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert that the layout engine successfully processed the Drop Portion
    assertXPath(pXmlDoc,
                "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/"
                "SwLinePortion[@type='PortionType::Drop']",
                1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testCombinedCharacters5)
{
    // Trigger SwCombinedPortion::Format for > 4 characters
    createSwDoc("combined_chars_5.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert that the layout engine successfully processed the Combined Portion
    assertXPath(pXmlDoc,
                "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/"
                "SwFieldPortion[@type='PortionType::Combined']",
                1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testWarichuInterrupted)
{
    createSwDoc("warichu_interrupted.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // The ruby portion sandwiched between the two double-line portions must have
    // successfully formatted its nested field portion.
    assertXPath(pXmlDoc, "//SwMultiPortion[contains(@symbol, 'SwRubyPortion')]//SwFieldPortion", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testWarichuToggleInterrupted)
{
    createSwDoc("warichu_toggle.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // "Middle Portion " is correctly split into 2 stacked SwLineLayout rows.
    assertXPath(pXmlDoc,
                "//SwMultiPortion[contains(@symbol, 'SwDoubleLinePortion')][@portion='Middle "
                "Portion ']/SwLineLayout",
                2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testRotateWarichuInterrupted)
{
    createSwDoc("rotate_warichu_interrupted.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // Both rotated portions survive alongside the double-line portion.
    assertXPath(pXmlDoc, "//SwMultiPortion[contains(@symbol, 'SwRotatedPortion')]", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testWarichuFieldWrapRestPortion)
{
    createSwDoc("warichu_field_wrap.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // Trigger SwTextFormatter::MakeRestPortion for a Warichu (double-line)
    // block wrapping a long field across lines.
    assertXPath(
        pXmlDoc,
        "//SwMultiPortion[contains(@symbol, 'SwDoubleLinePortion')][@length='44']/SwLineLayout", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testRubyWrapRestPortion)
{
    createSwDoc("ruby_wrap.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // separate ruby portions (one per wrapped word) survive the wrap.
    assertXPath(pXmlDoc, "//SwMultiPortion[contains(@symbol, 'SwRubyPortion')]", 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testJustifyWarichu)
{
    createSwDoc("justify_warichu.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Both Warichu blocks received their own justification glue portion, proving
    // CalcAdjustLine correctly stretched content inside the double-line portions.
    assertXPath(pXmlDoc,
                "//SwMultiPortion[contains(@symbol, 'SwDoubleLinePortion')]//SwGluePortion", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testCenterFlyWarichuTab)
{
    createSwDoc("center_fly_warichu_tab.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // The tab inside the Warichu block is correctly resolved on both of its
    // stacked internal lines despite the fly frame intersecting the paragraph.
    assertXPath(pXmlDoc,
                "//SwMultiPortion[contains(@symbol, "
                "'SwDoubleLinePortion')]//SwFixPortion[@type='PortionType::TabLeft']",
                2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testThaiJustifyPortxt)
{
    // Trigger Thai/CTL justification handling in portxt.cxx. Thai script has no
    // spaces to break on, so justified Thai text must be measured/compressed as a
    // single run rather than incorrectly split into word-like portions.
    createSwDoc("thai_justify_portxt.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // The full 21-character Thai string stays intact as one SwTextPortion.
    assertXPath(pXmlDoc,
                "//SwLineLayout/SwLinePortion[contains(@symbol, 'SwTextPortion')][@length='21']",
                1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testRefPageGetField)
{
    createSwDoc("ref_page_get_field.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // The "Get" field must resolve and expand to its computed value ("1"),
    // proving RefPageGet correctly evaluated the set/get + page-adjust logic
    // rather than rendering blank or unresolved.
    assertXPath(pXmlDoc, "//SwFieldPortion[contains(@symbol, 'SwFieldPortion')][@expand='1']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testInputFieldName)
{
    // Load document with an Input Field
    createSwDoc("input_field_name.fodt");

    // Grab the Writer Shell to modify the view options
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Simulate pressing Ctrl+F9 (View -> Field Names)
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetFieldName(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    // Dump the layout. The engine MUST evaluate IsFieldName() == true
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // With field names shown, the input field must render its name ("Input field")
    // instead of the typed content ("*User Typed This*").
    assertXPath(pXmlDoc,
                "//SwFieldPortion[contains(@symbol, 'SwFieldPortion')][@expand='Input field']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTextNodeGetDropLenAsian)
{
    // Hit the ASIAN script branch in SwTextNode::GetDropLen.
    // The paragraph is a single 7-character katakana word (リブレオフィス) with a
    // word-length drop cap, so GetDropLen must return the full word length.
    createSwDoc("drop_asian_word.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwTextNode* pNode = pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode();
    CPPUNIT_ASSERT(pNode);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pNode->GetDropLen(0));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTextNodeGetDropLenComplex)
{
    // Hit the COMPLEX (CTL) script branch in SwTextNode::GetDropLen.
    // The paragraph is a single Bengali word (লিব্রেঅফিস, 10 UTF-16 code units)
    // with a word-length drop cap; GetDropLen returns the full code-unit length.
    createSwDoc("drop_complex_word.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwTextNode* pNode = pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode();
    CPPUNIT_ASSERT(pNode);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), pNode->GetDropLen(0));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTextNodeGetDropLenDefault)
{
    // Hit the DEFAULT (Latin/English) script branch in SwTextNode::GetDropLen.
    // Word-length drop cap over the paragraph's first word, "Proin" (5 chars).
    createSwDoc("drop_fly_overlap.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwTextNode* pNode = pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode();
    CPPUNIT_ASSERT(pNode);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pNode->GetDropLen(0));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTextNodeGetDropSize)
{
    // Direct C++ unit test for SwTextNode::GetDropSize.
    createSwDoc("drop_asian_word.fodt");

    // Ensure that all text portions are calculated before testing, so that
    // GetDropSize() below is deterministic.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwTextNode* pNode = pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode();
    CPPUNIT_ASSERT(pNode);

    int nFontHeight = 0;
    int nDropHeight = 0;
    int nDropDescent = 0;
    bool bHasDropSize = pNode->GetDropSize(nFontHeight, nDropHeight, nDropDescent);

    // With layout fully reformatted, GetDropSize should reliably succeed and
    // report a real, positive font height with a non-negative drop height.
    CPPUNIT_ASSERT(bHasDropSize);
    CPPUNIT_ASSERT_GREATER(0, nFontHeight);
    CPPUNIT_ASSERT_GREATEREQUAL(0, nDropHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTextNodeGetDropSizeUnformatted)
{
    // Force the fallback "guessing" branch in GetDropSize by
    // querying it before the frame has ever been formatted.
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwTextNode* pNode = pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode();
    CPPUNIT_ASSERT(pNode);

    SwFormatDrop aDrop;
    aDrop.SetLines(3);
    aDrop.SetChars(1);
    pNode->SetAttr(aDrop);

    int nFontHeight = 0;
    int nDropHeight = 0;
    int nDropDescent = 0;

    // Deliberately no parseLayoutDump()/formatting call here — the frame must
    // stay unformatted so rFontHeight/rDropHeight start at 0, forcing the
    // fallback estimation logic to run instead of reading real frame metrics.
    bool bRet = pNode->GetDropSize(nFontHeight, nDropHeight, nDropDescent);

    // Returns false to signal it's an estimate, not a measured value...
    CPPUNIT_ASSERT(!bRet);
    // ...but still produces a usable non-zero guessed height.
    CPPUNIT_ASSERT_GREATER(0, nDropHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testBackgroundAttrChangeNotification)
{
    // Trigger the pAttrSetChangeHint notification containing RES_BACKGROUND,
    // forcing SwTextFrame to react to a paragraph background color change.
    createSwDoc("drop_fly_overlap.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Change the paragraph's background color; this sends the RES_BACKGROUND hint.
    SvxBrushItem aBrush(COL_RED, RES_BACKGROUND);
    pWrtShell->SetAttrItem(aBrush);

    // Re-dump the layout to confirm the invalidation/reformat completes cleanly.
    pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // The paragraph's attribute set now actually carries the new background
    // color — proving the change notification was correctly applied to the
    // model, not just silently accepted or lost during the reformat.
    SwTextNode* pNode = pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode();
    CPPUNIT_ASSERT(pNode);

    const SvxBrushItem* pBrush = pNode->GetSwAttrSet().GetItem<SvxBrushItem>(RES_BACKGROUND);
    CPPUNIT_ASSERT(pBrush);
    CPPUNIT_ASSERT_EQUAL(COL_RED, pBrush->GetColor());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testErgoSumFootnoteContinuation)
{
    // Trigger a footnote splitting across two pages, forming the follow/master
    // frame chain that SwErgoSumPortion construction depends on.
    createSwDoc("ergo_sum_footnote.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // The footnote body is split into a master frame (page 1) and a linked
    // follow frame (page 2), proving the footnote successfully broke across pages.
    assertXPath(pXmlDoc, "//ftn[contains(@symbol, 'SwFootnoteFrame')]", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testQuoVadisFootnoteContinuation)
{
    // Trigger SwTextFormatter::FormatQuoVadis by forcing a footnote to break
    // across a short page, forming the same follow/master frame chain.
    createSwDoc("quo_vadis_footnote.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//ftn[contains(@symbol, 'SwFootnoteFrame')]", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testLinkPortion)
{
    createSwDoc("link.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // The hyperlink run must show up as its own text portion with the
    // expanded text "link".
    assertXPath(pXmlDoc, "//SwLinePortion[@portion='link']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testPageNumberFieldPortion)
{
    createSwDoc("pagenumber.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // The page-number field portion must expand to "1".
    assertXPath(pXmlDoc, "//SwFieldPortion[@expand='1']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testFootnoteBodyPortion)
{
    createSwDoc("footnote.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // The anchor text in the body paragraph must be laid out correctly.
    assertXPath(pXmlDoc, "//body/txt//SwLinePortion[@portion='This is a footnote']", 1);
    // The footnote body text must be laid out in the footnote container.
    assertXPath(pXmlDoc, "//ftncont/ftn/txt//SwLinePortion[@portion='test']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testRubyPortion)
{
    createSwDoc("ruby.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // The ruby base/text pair must be formatted as a SwMultiPortion backed
    // by a SwRubyPortion, with base text "Ruby".
    assertXPath(pXmlDoc, "//SwMultiPortion[contains(@symbol, 'SwRubyPortion')][@portion='Ruby']",
                1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testAsCharShapePortion)
{
    createSwDoc("shape.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // Shape's SdrObject must survive layout, regardless of container.
    assertXPath(pXmlDoc, "//SdrObject[@name='Shape 1']", 1);
}

} // end of anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
