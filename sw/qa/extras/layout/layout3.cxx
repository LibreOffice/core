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
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <vcl/event.hxx>
#include <vcl/metaact.hxx>
#include <vcl/scheduler.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/unolingu.hxx>
#include <comphelper/sequence.hxx>

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
#include <unotxdoc.hxx>
#include <rootfrm.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <svx/svdpage.hxx>

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
    saveAndReload(u"writer8"_ustr);
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
            CPPUNIT_ASSERT_LESS(sal_Int32(4200), sal_Int32(pDXArray[45]));
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf106234)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf155324)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf120287b)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf120287c)
{
    createSwDoc("tdf120287c.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 3, the second line was broken into a 2nd and a 3rd one,
    // not rendering text outside the paragraph frame like Word 2013 does.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf158658a)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf158658b)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf158658c)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf155177)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf122878)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf115094)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf112290)
{
    createSwDoc("tdf112290.docx");
    auto pXml = parseLayoutDump();
    assertXPath(pXml, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion", u"Xxxx Xxxx");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testKeepWithNextPlusFlyFollowTextFlow)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf122607)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf122607_regression)
{
    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue(u"Hidden"_ustr, -1, uno::Any(true),
                               beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    OUString const url(createFileURL(u"tdf122607_leerzeile.odt"));
    loadWithParams(url, comphelper::containerToSequence(aFilterOptions));
    save(u"writer_pdf_Export"_ustr);

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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, TestTdf150616)
{
    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue(u"Hidden"_ustr, -1, uno::Any(true),
                               beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    OUString const url(createFileURL(u"in_056132_mod.odt"));
    loadWithParams(url, comphelper::containerToSequence(aFilterOptions));
    save(u"writer_pdf_Export"_ustr);

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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testBtlrCell)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf123898)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf123651)
{
    createSwDoc("tdf123651.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with 'Expected: 7639;
    // Actual: 12926'. The shape was below the second "Lorem ipsum" text, not above it.
    const sal_Int32 nTopValue
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "top").toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7639, nTopValue, 10);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf116501)
{
    //just care it doesn't freeze
    createSwDoc("tdf116501.odt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf123163)
{
    //just care it doesn't assert
    createSwDoc("tdf123163-1.docx");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testAbi11870)
{
    //just care it doesn't assert
    createSwDoc("abi11870-2.odt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testOfz64109)
{
    //just care it doesn't assert
    createSwDoc("ofz64109-1.fodt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf118719)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTabOverMargin)
{
    createSwDoc("tab-over-margin.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 2nd paragraph has a tab over the right margin, and with the TabOverMargin compat option,
    // there is enough space to have all content in a single line.
    // Without the accompanying fix in place, this test would have failed, there were 2 lines.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testImageComment)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testScriptField)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testCommentCursorPosition)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testCombiningCharacterCursorPosition)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf64222)
{
    createSwDoc("tdf64222.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc,
                "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/"
                "child::*[@type='PortionType::Number']/SwFont",
                "height", u"560");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf113014)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf130218)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf127235)
{
    createSwDoc("tdf127235.odt");
    // This resulted in a layout loop.
    calcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf138039)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf134298)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testShapeAllowOverlap)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testShapeAllowOverlapWrap)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf124600)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf124601)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf124601b)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf124770)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testContinuousEndnotesInsertPageAtStart)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testContinuousEndnotesDeletePageAtStart)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf128399)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf156724)
{
    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue(u"Hidden"_ustr, -1, uno::Any(true),
                               beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    OUString const url(createFileURL(u"fdo56797-2-min.odt"));
    loadWithParams(url, comphelper::containerToSequence(aFilterOptions));
    save(u"writer_pdf_Export"_ustr);

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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testHiddenParagraphFollowFrame)
{
    createSwDoc("hidden-para-follow-frame.fodt");

    uno::Any aOldValue{ queryDispatchStatus(mxComponent, m_xContext, ".uno:ShowHiddenParagraphs") };

    Resetter g([this, aOldValue] {
        uno::Sequence<beans::PropertyValue> argsSH(
            comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", aOldValue } }));
        dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", argsSH);
    });

    uno::Sequence<beans::PropertyValue> argsSH(
        comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", uno::Any(true) } }));
    dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", argsSH);
    uno::Sequence<beans::PropertyValue> args(
        comphelper::InitPropertySequence({ { "Fieldnames", uno::Any(false) } }));
    dispatchCommand(mxComponent, ".uno:Fieldnames", args);
    Scheduler::ProcessEventsToIdle();

    {
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testHiddenParagraphFlys)
{
    createSwDoc("hidden-para-as-char-fly.fodt");

    uno::Any aOldValue{ queryDispatchStatus(mxComponent, m_xContext, ".uno:ShowHiddenParagraphs") };

    Resetter g([this, aOldValue] {
        uno::Sequence<beans::PropertyValue> argsSH(
            comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", aOldValue } }));
        dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", argsSH);
    });

    uno::Sequence<beans::PropertyValue> argsSH(
        comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", uno::Any(true) } }));
    dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", argsSH);
    uno::Sequence<beans::PropertyValue> args(
        comphelper::InitPropertySequence({ { "Fieldnames", uno::Any(false) } }));
    dispatchCommand(mxComponent, ".uno:Fieldnames", args);
    Scheduler::ProcessEventsToIdle();

    {
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testSectionUnhide)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testHiddenSectionFlys)
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

} // end of anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
