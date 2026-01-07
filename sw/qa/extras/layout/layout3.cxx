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
#include <vcl/metaact.hxx>
#include <editeng/unolingu.hxx>

#include <wrtsh.hxx>
#include <rootfrm.hxx>
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

} // end of anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
