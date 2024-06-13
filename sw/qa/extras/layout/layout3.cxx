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
#include <vcl/scheduler.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/editobj.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/propertyvalue.hxx>

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
#include <unoframe.hxx>
#include <drawdoc.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdotext.hxx>
#include <dcontact.hxx>
#include <frameformats.hxx>

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
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/infos/prtBounds"_ostr, "top"_ostr, u"21"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf117188)
{
    createSwDoc("tdf117188.docx");
    saveAndReload(u"writer8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    OUString sWidth
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds"_ostr, "width"_ostr);
    OUString sHeight
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds"_ostr, "height"_ostr);
    // The text box must have zero border distances
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds"_ostr, "left"_ostr,
                u"0"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds"_ostr, "top"_ostr,
                u"0"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds"_ostr, "width"_ostr,
                sWidth);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds"_ostr, "height"_ostr,
                sHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf117187)
{
    createSwDoc("tdf117187.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // there should be no fly portions
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[@nType='PortionType::Fly']"_ostr,
        0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf119875)
{
    createSwDoc("tdf119875.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page[2]/body/section[1]"_ostr, "formatName"_ostr, u"S10"_ustr);
    assertXPath(pXmlDoc, "//page[2]/body/section[2]"_ostr, "formatName"_ostr, u"S11"_ustr);
    assertXPath(pXmlDoc, "//page[2]/body/section[3]"_ostr, "formatName"_ostr, u"S13"_ustr);
    assertXPath(pXmlDoc, "//page[2]/body/section[4]"_ostr, "formatName"_ostr, u"S14"_ustr);
    // Sections "S10" and "S13" are hidden -> their frames are zero-height
    assertXPath(pXmlDoc, "//page[2]/body/section[1]/infos/bounds"_ostr, "height"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "//page[2]/body/section[3]/infos/bounds"_ostr, "height"_ostr, u"0"_ustr);

    OUString S10Top = getXPath(pXmlDoc, "//page[2]/body/section[1]/infos/bounds"_ostr, "top"_ostr);
    OUString S11Top = getXPath(pXmlDoc, "//page[2]/body/section[2]/infos/bounds"_ostr, "top"_ostr);
    OUString S13Top = getXPath(pXmlDoc, "//page[2]/body/section[3]/infos/bounds"_ostr, "top"_ostr);
    OUString S14Top = getXPath(pXmlDoc, "//page[2]/body/section[4]/infos/bounds"_ostr, "top"_ostr);

    CPPUNIT_ASSERT_EQUAL(S10Top, S11Top);
    CPPUNIT_ASSERT_EQUAL(S13Top, S14Top);

    // Section "S11" had the same top value as section "S14", so they overlapped.
    CPPUNIT_ASSERT_LESS(S14Top.toInt32(), S11Top.toInt32());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf137523)
{
    createSwDoc("tdf137523-1-min.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // the problem was that in the footer, the text frames below the table
    // had wrong height and were not visible
    assertXPath(pXmlDoc, "/root/page/footer/txt[1]/infos/bounds"_ostr, "height"_ostr, u"304"_ustr);
    assertXPath(pXmlDoc, "/root/page/footer/txt[2]/infos/bounds"_ostr, "height"_ostr, u"191"_ustr);
    assertXPath(pXmlDoc, "/root/page/footer/txt[3]/infos/bounds"_ostr, "height"_ostr, u"219"_ustr);
    assertXPath(pXmlDoc, "/root/page/footer/tab/infos/bounds"_ostr, "height"_ostr, u"1378"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf120287)
{
    createSwDoc("tdf120287.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin Word-specific compat flag did not imply
    // default-in-Word printer-independent layout, resulting in an additional
    // line break.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 1);
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
        = getXPathNode(pXmlDoc, "/root/page/body/tab/row/cell[@rowspan > 0][child::txt]"_ostr);
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf119908)
{
    createSwDoc("tdf130088.docx");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Keep real width of the exceeding line portions to calculate shrinking
    sal_Int32 nPortionWidth
        = getXPath(pXmlDoc,
                   "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[2]/SwLinePortion[2]"_ostr,
                   "width"_ostr)
              .toInt32();
    // This was 5806 (not real portion width, but stripped to the line width)
    CPPUNIT_ASSERT_GREATER(sal_Int32(5840), nPortionWidth);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf158333)
{
    createSwDoc("tdf130088.docx");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // shrink line 2
    assertXPath(
        pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[2]"_ostr, "portion"_ostr,
        u"viverra odio. Donec auctor molestie sem, sit amet tristique lectus hendrerit sed. "_ustr);

    // shrink line 7
    assertXPath(
        pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[7]"_ostr, "portion"_ostr,
        // This was "...diam ", not "...diam tempor "
        u"laoreet vel leo nec, volutpat facilisis eros. Donec consequat arcu ut diam tempor "_ustr);

    // shrink line 2 of paragraph 2
    assertXPath(
        pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[2]"_ostr, "portion"_ostr,
        // This was "...Cras ", not "...Cras sodales "
        u"Donec auctor molestie sem, sit amet tristique lectus hendrerit sed. Cras sodales "_ustr);

    // shrink line 2 of paragraph 4
    assertXPath(
        pXmlDoc, "/root/page/body/txt[4]/SwParaPortion/SwLineLayout[2]"_ostr, "portion"_ostr,
        // This was "...et ", not "...et magnis "
        u"consequat arcu ut diam tempor luctus. Cum sociis natoque penatibus et magnis "_ustr);

    // tdf#158776 don't shrink line 11 of paragraph 4
    assertXPath(pXmlDoc, "/root/page/body/txt[4]/SwParaPortion/SwLineLayout[11]"_ostr,
                "portion"_ostr,
                // This was "...quis curcus ", not "...quis "
                u"venenatis, quis commodo dolor posuere. Curabitur dignissim sapien quis "_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf159085)
{
    createSwDoc("tdf159085.fodt");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // This was "... cursus" instead of breaking the word at soft hyphen
    assertXPath(
        pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr, "portion"_ostr,
        u"venenatis, quis commodo dolor posuere. Curabitur dignissim sapien quis cur­"_ustr);

    // This was "... cursus" instead of breaking the word at soft hyphen
    assertXPath(
        pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]"_ostr, "portion"_ostr,
        u"venenatis, quis commodo dolor posuere. Curabitur dignissim sapien quis cur­"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf158419)
{
    createSwDoc("tdf130088.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // second paragraph.
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    SwPosition aPosition(*pWrtShell->GetCursor()->Start());
    SwTwips nSecondParaLeft
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds"_ostr, "left"_ostr).toInt32();
    SwTwips nSecondParaWidth
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds"_ostr, "width"_ostr).toInt32();
    SwTwips nSecondParaTop
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds"_ostr, "top"_ostr).toInt32();
    SwTwips nSecondParaHeight
        = getXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds"_ostr, "height"_ostr).toInt32();
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf132599_always)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf132599_always.fodt");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 2nd paragraph: hyphenated last full line
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[2]"_ostr,
                "portion"_ostr, u"ent to any other celes"_ustr);

    // hyphenation-keep-type='always'
    // 3rd paragraph: not hyphenated last full line of the hyphenated paragraph
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout[2]"_ostr,
                "portion"_ostr, u"ent to any other "_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf132599_frames_on_same_page_no_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf132599_frames_on_same_page_no_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="column"
    // 2nd frame: shifted hyphenated line (no hyphenation at the end of the first frame)
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/fly[2]/txt/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, u"space, ex"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf132599_frames_on_same_page_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf132599_frames_on_same_page_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="page"
    // 2nd frame: not shifted hyphenated line (hyphenation at the end of the first frame),
    // This was "space, ex" (bad shifting)
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/fly[2]/txt/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, u"cept that it "_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf132599_frames_on_right_pages_no_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf132599_frames_on_right_pages_no_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="spread"
    // 2nd frame: shifted hyphenated line
    // This was "cept that it" (missing shifting)
    assertXPath(pXmlDoc,
                "/root/page[3]/body/txt/anchored/fly/txt/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, u"space, ex"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf132599_frames_on_spread_hyphenation)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf132599_frames_on_spread_hyphenation.fodt");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // loext:hyphenation-keep-type="spread"
    // 2nd frame on left page and 3rd frame on right page -> not shifted hyphenated line
    // 2nd frame: not shifted hyphenated line (hyphenation at the end of the first frame),
    assertXPath(pXmlDoc,
                "/root/page[3]/body/txt/anchored/fly/txt/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, u"cept that it "_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf106234)
{
    createSwDoc("tdf106234.fodt");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // In justified paragraphs, there is justification between left tabulators and manual line breaks
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwGluePortion"_ostr,
                "type"_ostr, u"PortionType::Margin"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwGluePortion"_ostr,
                "width"_ostr, u"0"_ustr);
    // but not after centered, right and decimal tabulators
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwGluePortion"_ostr,
                "type"_ostr, u"PortionType::Margin"_ustr);
    // This was a justified line, without width
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwGluePortion"_ostr,
                "width"_ostr, u"7882"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf155324)
{
    createSwDoc("tox-update-wrong-pages.odt");

    dispatchCommand(mxComponent, u".uno:UpdateAllIndexes"_ustr, {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // the problem was that the first entry was on page 7, 2nd on page 9 etc.
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
        "portion"_ostr, u"Foo"_ustr);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
        "portion"_ostr, u"5"_ustr);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/section[2]/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
        "portion"_ostr, u"bar"_ustr);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/section[2]/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
        "portion"_ostr, u"7"_ustr);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
        "portion"_ostr, u"Three"_ustr);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
        "portion"_ostr, u"7"_ustr);

    // check first content page has the footnotes
    assertXPath(pXmlDoc, "/root/page[5]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, u"Foo"_ustr);
    assertXPath(pXmlDoc, "/root/page[4]/ftncont"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[5]/ftncont/ftn"_ostr, 5);
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
        "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabRight']"_ostr,
        "width"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf120287c)
{
    createSwDoc("tdf120287c.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 3, the second line was broken into a 2nd and a 3rd one,
    // not rendering text outside the paragraph frame like Word 2013 does.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf158658a)
{
    createSwDoc("tdf158658a.rtf");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Word 2013 puts all tabs into one line, the last 8 of them are off the page
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabCenter']"_ostr,
        1);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabRight']"_ostr,
        1);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabLeft']"_ostr,
        9);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf158658b)
{
    createSwDoc("tdf158658b.rtf");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Word 2013 puts all tabs and the field following into one line
    // but also puts the field off the page, while in Writer it's
    // aligned to the right margin; should be good enough for now...
    assertXPath(pXmlDoc, "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabCenter']"_ostr,
        1);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabRight']"_ostr,
        1);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/footer/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabLeft']"_ostr,
        0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf158658c)
{
    createSwDoc("tdf158658c.rtf");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Word 2013 puts all tabs into one line, the last 17 of them are off the page
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabCenter']"_ostr,
        1);
    // the right tab is exactly at the margin of the paragraph
    assertXPath(
        pXmlDoc,
        "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabRight']"_ostr,
        1);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabLeft']"_ostr,
        20);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf155177)
{
    createSwDoc("tdf155177-1-min.odt");

    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Body Text"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(210), getProperty<sal_Int32>(xStyle, u"ParaTopMargin"_ustr));

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[2]/body/txt"_ostr, 6);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[6]/SwParaPortion/SwLineLayout"_ostr, 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[6]/SwParaPortion/SwLineLayout[2]"_ostr,
                    "portion"_ostr, u"long as two lines."_ustr);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 3);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                    "portion"_ostr, u"This paragraph is even longer so that "_ustr);
        discardDumpedLayout();
    }

    // this should bring one line back
    xStyle->setPropertyValue(u"ParaTopMargin"_ustr, uno::Any(sal_Int32(200)));

    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[2]/body/txt"_ostr, 7);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[7]/SwParaPortion/SwLineLayout"_ostr, 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[7]/SwParaPortion/SwLineLayout[1]"_ostr,
                    "portion"_ostr, u"This paragraph is even longer so that "_ustr);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 2);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                    "portion"_ostr, u"it is now three lines long though "_ustr);
        discardDumpedLayout();
    }

    // this should bring second line back
    xStyle->setPropertyValue(u"ParaTopMargin"_ustr, uno::Any(sal_Int32(120)));

    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[2]/body/txt"_ostr, 7);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[7]/SwParaPortion/SwLineLayout"_ostr, 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[7]/SwParaPortion/SwLineLayout[1]"_ostr,
                    "portion"_ostr, u"This paragraph is even longer so that "_ustr);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[7]/SwParaPortion/SwLineLayout[2]"_ostr,
                    "portion"_ostr, u"it is now three lines long though "_ustr);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                    "portion"_ostr, u"containing a single sentence."_ustr);
        discardDumpedLayout();
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf122878)
{
    createSwDoc("tdf122878.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    const sal_Int32 nTblTop
        = getXPath(pXmlDoc, "/root/page[1]/footer/txt/anchored/fly/tab/infos/bounds"_ostr,
                   "top"_ostr)
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
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/tab/row[1]/cell[4]/infos/bounds"_ostr,
                   "top"_ostr)
              .toInt32();
    sal_Int32 nTopOfD1Anchored = getXPath(pXmlDoc,
                                          "/root/page/body/txt/anchored/fly/tab/row[1]/cell[4]/"
                                          "txt[2]/anchored/fly/infos/bounds"_ostr,
                                          "top"_ostr)
                                     .toInt32();
    CPPUNIT_ASSERT_LESS(nTopOfD1Anchored, nTopOfD1);
    sal_Int32 nTopOfB2
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/tab/row[2]/cell[2]/infos/bounds"_ostr,
                   "top"_ostr)
              .toInt32();
    sal_Int32 nTopOfB2Anchored = getXPath(pXmlDoc,
                                          "/root/page/body/txt/anchored/fly/tab/row[2]/cell[2]/"
                                          "txt[1]/anchored/fly/infos/bounds"_ostr,
                                          "top"_ostr)
                                     .toInt32();
    CPPUNIT_ASSERT_LESS(nTopOfB2Anchored, nTopOfB2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf112290)
{
    createSwDoc("tdf112290.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    auto pXml = parseLayoutDump();
    assertXPath(pXml, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]"_ostr, "portion"_ostr,
                u"Xxxx Xxxx"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testKeepWithNextPlusFlyFollowTextFlow)
{
    createSwDoc("keep-with-next-fly.fodt");

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // 3 text frames on page 1
        assertXPath(pXmlDoc, "/root/page[1]/body/infos/bounds"_ostr, "bottom"_ostr, u"7540"_ustr);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds"_ostr, "height"_ostr,
                    u"276"_ustr);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/bounds"_ostr, "height"_ostr,
                    u"276"_ustr);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly"_ostr, 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly/infos/bounds"_ostr, "top"_ostr,
                    u"1694"_ustr);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/infos/bounds"_ostr, "height"_ostr,
                    u"276"_ustr);
        assertXPath(pXmlDoc, "/root/page"_ostr, 1);
        discardDumpedLayout();
    }

    dispatchCommand(mxComponent, u".uno:Fieldnames"_ustr, {});

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // 1 text frame on page 1, and some empty space
        assertXPath(pXmlDoc, "/root/page[1]/body/infos/bounds"_ostr, "bottom"_ostr, u"7540"_ustr);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds"_ostr, "height"_ostr,
                    u"5796"_ustr);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds"_ostr, "bottom"_ostr,
                    u"7213"_ustr);
        // 2 text frames on page 2
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/infos/bounds"_ostr, "height"_ostr,
                    u"276"_ustr);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly"_ostr, 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly/infos/bounds"_ostr, "top"_ostr,
                    u"10093"_ustr);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/infos/bounds"_ostr, "height"_ostr,
                    u"276"_ustr);
        assertXPath(pXmlDoc, "/root/page"_ostr, 2);
        discardDumpedLayout();
    }

    dispatchCommand(mxComponent, u".uno:Fieldnames"_ustr, {});

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // 3 text frames on page 1
        assertXPath(pXmlDoc, "/root/page[1]/body/infos/bounds"_ostr, "bottom"_ostr, u"7540"_ustr);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds"_ostr, "height"_ostr,
                    u"276"_ustr);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/bounds"_ostr, "height"_ostr,
                    u"276"_ustr);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly"_ostr, 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly/infos/bounds"_ostr, "top"_ostr,
                    u"1694"_ustr);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/infos/bounds"_ostr, "height"_ostr,
                    u"276"_ustr);
        assertXPath(pXmlDoc, "/root/page"_ostr, 1);
        discardDumpedLayout();
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf122607)
{
    createSwDoc("tdf122607.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "height"_ostr, u"253"_ustr);
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "width"_ostr, u"428"_ustr);
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "portion"_ostr, u"Fax:"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf122607_regression)
{
    discardDumpedLayout();
    if (mxComponent.is())
        mxComponent->dispose();

    OUString const url(createFileURL(u"tdf122607_leerzeile.odt"));

    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue(u"Hidden"_ustr, -1, uno::Any(true),
                               beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    mxComponent = loadFromDesktop(url, u"com.sun.star.text.TextDocument"_ustr,
                                  comphelper::containerToSequence(aFilterOptions));
    uno::Sequence<beans::PropertyValue> props(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(u"writer_pdf_Export"_ustr) },
    }));
    utl::TempFileNamed aTempFile;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), props);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // somehow these 2 rows overlapped in the PDF unless CalcLayout() runs
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds"_ostr,
                "mbFixSize"_ostr, u"false"_ustr);
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds"_ostr, "top"_ostr,
                u"2977"_ustr);
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds"_ostr,
                "height"_ostr, u"241"_ustr);
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds"_ostr,
                "mbFixSize"_ostr, u"true"_ustr);
    // this was 3034, causing the overlap
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds"_ostr, "top"_ostr,
                u"3218"_ustr);
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds"_ostr,
                "height"_ostr, u"164"_ustr);

    aTempFile.EnableKillingFile();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, TestTdf150616)
{
    discardDumpedLayout();
    if (mxComponent.is())
        mxComponent->dispose();

    OUString const url(createFileURL(u"in_056132_mod.odt"));

    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue(u"Hidden"_ustr, -1, uno::Any(true),
                               beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    mxComponent = loadFromDesktop(url, u"com.sun.star.text.TextDocument"_ustr,
                                  comphelper::containerToSequence(aFilterOptions));
    uno::Sequence<beans::PropertyValue> props(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(u"writer_pdf_Export"_ustr) },
    }));
    utl::TempFileNamed aTempFile;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), props);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // this one was 0 height
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[2]/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, u"Important information here!"_ustr);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[2]/infos/bounds"_ostr,
                "height"_ostr, u"253"_ustr);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[2]/infos/bounds"_ostr,
                "top"_ostr, u"7925"_ustr);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[3]/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, u"xxx 111 "_ustr);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[3]/infos/bounds"_ostr,
                "height"_ostr, u"697"_ustr);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[3]/infos/bounds"_ostr,
                "top"_ostr, u"8178"_ustr);

    aTempFile.EnableKillingFile();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testBtlrCell)
{
    createSwDoc("btlr-cell.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed, as
    // the orientation was 0 (layout did not take btlr direction request from
    // doc model).
    assertXPath(pXmlDoc, "//font[1]"_ostr, "orientation"_ostr, u"900"_ustr);

#if !defined(MACOSX) && !defined(_WIN32) // macOS fails with x == 2662 for some reason.
    // Without the accompanying fix in place, this test would have failed with 'Expected: 1915;
    // Actual  : 1756', i.e. the AAA1 text was too close to the left cell border due to an ascent vs
    // descent mismatch when calculating the baseline offset of the text portion.
    assertXPath(pXmlDoc, "//textarray[1]"_ostr, "x"_ostr, u"1915"_ustr);
    assertXPath(pXmlDoc, "//textarray[1]"_ostr, "y"_ostr, u"2707"_ustr);

    // Without the accompanying fix in place, this test would have failed with 'Expected: 1979;
    // Actual  : 2129', i.e. the gray background of the "AAA2." text was too close to the right edge
    // of the text portion. Now it's exactly behind the text portion.
    assertXPath(pXmlDoc, "(//rect)[2]"_ostr, "left"_ostr, u"1979"_ustr);

    // Without the accompanying fix in place, this test would have failed with 'Expected: 269;
    // Actual  : 0', i.e. the AAA2 frame was not visible due to 0 width.
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds"_ostr, "width"_ostr,
                u"269"_ustr);

    // Test the position of the cursor after doc load.
    // We expect that it's inside the first text frame in the first cell.
    // More precisely, this is a bottom to top vertical frame, so we expect it's at the start, which
    // means it's at the lower half of the text frame rectangle (vertically).
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    const SwRect& rCharRect = pWrtShell->GetCharRect();
    SwTwips nFirstParaTop
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[1]/infos/bounds"_ostr, "top"_ostr)
              .toInt32();
    SwTwips nFirstParaHeight
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[1]/infos/bounds"_ostr,
                   "height"_ostr)
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
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwPosition aPosition(aCellStart);
    SwTwips nSecondParaLeft
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds"_ostr, "left"_ostr)
              .toInt32();
    SwTwips nSecondParaWidth
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds"_ostr,
                   "width"_ostr)
              .toInt32();
    SwTwips nSecondParaTop
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds"_ostr, "top"_ostr)
              .toInt32();
    SwTwips nSecondParaHeight
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds"_ostr,
                   "height"_ostr)
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
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds"_ostr, "left"_ostr)
              .toInt32();
    SwTwips nCellWidth
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds"_ostr, "width"_ostr)
              .toInt32();
    SwTwips nCellTop
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds"_ostr, "top"_ostr)
              .toInt32();
    SwTwips nCellHeight
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds"_ostr, "height"_ostr)
              .toInt32();
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

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure that the arrow on the left is not there (the first portion's type is
    // PortionType::Arrow if it's there)
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt/anchored/fly/txt/SwParaPortion/SwLineLayout[1]/child::*[1]"_ostr,
        "type"_ostr, u"PortionType::Para"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf123651)
{
    createSwDoc("tdf123651.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with 'Expected: 7639;
    // Actual: 12926'. The shape was below the second "Lorem ipsum" text, not above it.
    const sal_Int32 nTopValue
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "top"_ostr).toInt32();
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
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Enable hide whitespace mode.
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    pWrtShell->Insert(u"first"_ustr);
    pWrtShell->InsertPageBreak();
    pWrtShell->Insert(u"second"_ustr);

    // Without the accompanying fix in place, this test would have failed, as the height of the
    // first page was 15840 twips, instead of the much smaller 276.
    sal_Int32 nOther = parseDump("/root/page[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    sal_Int32 nLast = parseDump("/root/page[2]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(nOther, nLast);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTabOverMargin)
{
    createSwDoc("tab-over-margin.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 2nd paragraph has a tab over the right margin, and with the TabOverMargin compat option,
    // there is enough space to have all content in a single line.
    // Without the accompanying fix in place, this test would have failed, there were 2 lines.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testImageComment)
{
    // Load a document that has "aaa" in it, then a commented image (4th char is the as-char image,
    // 5th char is the comment anchor).
    createSwDoc("image-comment.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

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
    const SwRect& rDrawObjRect = pDrawObj->GetObjRect();
    Point aPoint = rDrawObjRect.Center();
    aPoint.setX(aPoint.getX() + rDrawObjRect.Width() / 2);

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
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

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
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

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
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

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
                "child::*[@type='PortionType::Number']/SwFont"_ostr,
                "height"_ostr, u"560"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf113014)
{
    createSwDoc("tdf113014.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if numbering of cell A1 is missing
    // (A1: left indent: 3 cm, first line indent: -3 cm
    // A2: left indent: 0 cm, first line indent: 0 cm)
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text"_ostr,
                       u"1."_ustr);
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[3]/text"_ostr,
                       u"2."_ustr);
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[5]/text"_ostr,
                       u"3."_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf130218)
{
    createSwDoc("tdf130218.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if hanging first line was hidden
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text"_ostr,
                       u"Text"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf127235)
{
    createSwDoc("tdf127235.odt");
    SwDoc* pDoc = getSwDoc();
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf138039)
{
    createSwDoc("tdf138039.odt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // there are 3 pages
    assertXPath(pXmlDoc, "/root/page"_ostr, 3);
    // table on first page
    assertXPath(pXmlDoc, "/root/page[1]/body/tab"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 0);
    // paragraph with large fly on second page
    assertXPath(pXmlDoc, "/root/page[2]/body/tab"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds"_ostr, "top"_ostr,
                u"17915"_ustr);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds"_ostr,
                "height"_ostr, u"15819"_ustr);
    // paragraph on third page
    assertXPath(pXmlDoc, "/root/page[3]/body/tab"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf134298)
{
    createSwDoc("tdf134298.ott");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // there are 2 pages
    assertXPath(pXmlDoc, "/root/page"_ostr, 2);
    // table and first para on first page
    assertXPath(pXmlDoc, "/root/page[1]/body/tab"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored"_ostr, 0);
    // paragraph with large fly on second page
    assertXPath(pXmlDoc, "/root/page[2]/body/tab"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds"_ostr, "top"_ostr,
                u"17897"_ustr);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds"_ostr,
                "height"_ostr, u"15819"_ustr);
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
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
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
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
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
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout"_ostr, 1);
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
    assertXPath(pXmlDoc, "/root/page"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[2]//ftncont"_ostr, 1);
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

    sal_Int32 nFlyTop = getXPath(pXmlDoc, "//anchored/fly/infos/bounds"_ostr, "top"_ostr).toInt32();
    sal_Int32 nFlyLeft
        = getXPath(pXmlDoc, "//anchored/fly/infos/bounds"_ostr, "left"_ostr).toInt32();
    sal_Int32 nFlyRight
        = nFlyLeft + getXPath(pXmlDoc, "//anchored/fly/infos/bounds"_ostr, "width"_ostr).toInt32();
    sal_Int32 nSecondRowTop
        = getXPath(pXmlDoc, "//tab/row[2]/infos/bounds"_ostr, "top"_ostr).toInt32();
    sal_Int32 nLastCellLeft
        = getXPath(pXmlDoc, "//tab/row[1]/cell[5]/infos/bounds"_ostr, "left"_ostr).toInt32();
    sal_Int32 nLastCellRight
        = nLastCellLeft
          + getXPath(pXmlDoc, "//tab/row[1]/cell[5]/infos/bounds"_ostr, "width"_ostr).toInt32();
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
    aLRSpace.SetLeft(1418);
    aLRSpace.SetRight(1418);
    rPageFormat.SetFormatAttr(aLRSpace);
    pDoc->ChgPageDesc(0, rPageDesc);

    // Set font to italic 20pt Liberation Serif.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
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
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testContinuousEndnotesInsertPageAtStart)
{
    // Create a new document with CONTINUOUS_ENDNOTES enabled.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    pDoc->getIDocumentSettingAccess().set(DocumentSettingId::CONTINUOUS_ENDNOTES, true);

    // Insert a second page, and an endnote on the 2nd page (both the anchor and the endnote is on
    // the 2nd page).
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();
    pWrtShell->InsertFootnote(u"endnote"_ustr, /*bEndNote=*/true, /*bEdit=*/false);

    // Add a new page at the start of the document.
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->InsertPageBreak();

    // Make sure that the endnote is moved from the 2nd page to the 3rd one.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page"_ostr, 3);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the footnote container remained on page 2.
    assertXPath(pXmlDoc, "/root/page[3]//ftncont"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testContinuousEndnotesDeletePageAtStart)
{
    // Create a new document with CONTINUOUS_ENDNOTES enabled.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    pDoc->getIDocumentSettingAccess().set(DocumentSettingId::CONTINUOUS_ENDNOTES, true);

    // Insert a second page, and an endnote on the 2nd page (both the anchor and the endnote is on
    // the 2nd page).
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
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
    assertXPath(pXmlDoc, "/root/page"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]//ftncont"_ostr, 1);
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

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
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
    discardDumpedLayout();
    if (mxComponent.is())
        mxComponent->dispose();

    OUString const url(createFileURL(u"fdo56797-2-min.odt"));

    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue(u"Hidden"_ustr, -1, uno::Any(true),
                               beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    mxComponent = loadFromDesktop(url, u"com.sun.star.text.TextDocument"_ustr,
                                  comphelper::containerToSequence(aFilterOptions));
    save(u"writer_pdf_Export"_ustr);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // both pages have a tab frame and one footnote
    assertXPath(pXmlDoc, "/root/page[1]/body/tab"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/ftncont"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/tab"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf156725)
{
    createSwDoc("tdf156725.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page"_ostr, 2);
    // the fly has 2 columns, the section in it has 2 columns, and is split
    // across the fly columns => 4 columns with 1 text frame each
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly/column"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly/column[1]/body/section/column"_ostr,
                2);
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/txt/anchored/fly/column[1]/body/section/column[1]/body/txt"_ostr, 1);
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/txt/anchored/fly/column[1]/body/section/column[2]/body/txt"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly/column[2]/body/section/column"_ostr,
                2);
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/txt/anchored/fly/column[2]/body/section/column[1]/body/txt"_ostr, 1);
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/txt/anchored/fly/column[2]/body/section/column[2]/body/txt"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf156419)
{
    createSwDoc("linked_frames_section_bug.odt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page"_ostr, 2);
    // there are 2 flys on page 1, and 1 on page 2, all linked
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[1]/section/column"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[1]/section/column[1]/body/txt"_ostr,
                11);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[1]/section/column[2]/body/txt"_ostr,
                11);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[2]/section/column"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[2]/section/column[1]/body/txt"_ostr,
                12);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[2]/section/column[2]/body/txt"_ostr,
                12);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly[1]/section/column"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly[1]/section/column[1]/body/txt"_ostr,
                2);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly[1]/section/column[2]/body/txt"_ostr,
                1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf145826)
{
    createSwDoc("tdf145826.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/root/page/body/section/column"_ostr, 2);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    assertXPath(pXmlDoc, "/root/page/body/section/column[1]/ftncont"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/section/column[2]/ftncont"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/section/column[1]/ftncont/ftn"_ostr, 3);
    assertXPath(pXmlDoc, "/root/page/body/section/column[2]/ftncont/ftn"_ostr, 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTable0HeightRows)
{
    createSwDoc("table-0-height-rows.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // the problem was that the table was erroneously split across 2 or 3 pages
    assertXPath(pXmlDoc, "/root/page[1]/body/tab"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 28);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/infos/bounds[@height='0']"_ostr, 25);
    assertXPath(pXmlDoc, "/root/page"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf105481)
{
    createSwDoc("tdf105481.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed
    // because the vertical position of the as-char shape object and the
    // as-char math object will be wrong (below/beyond the text frame's bottom).

    SwTwips nTxtTop = getXPath(pXmlDoc,
                               "/root/page/anchored/fly/txt[2]"
                               "/infos/bounds"_ostr,
                               "top"_ostr)
                          .toInt32();
    SwTwips nTxtBottom = nTxtTop
                         + getXPath(pXmlDoc,
                                    "/root/page/anchored/fly/txt[2]"
                                    "/infos/bounds"_ostr,
                                    "height"_ostr)
                               .toInt32();

    SwTwips nFormula1Top = getXPath(pXmlDoc,
                                    "/root/page/anchored/fly/txt[2]"
                                    "/anchored/fly[1]/infos/bounds"_ostr,
                                    "top"_ostr)
                               .toInt32();
    SwTwips nFormula1Bottom = nFormula1Top
                              + getXPath(pXmlDoc,
                                         "/root/page/anchored/fly/txt[2]"
                                         "/anchored/fly[1]/infos/bounds"_ostr,
                                         "height"_ostr)
                                    .toInt32();

    SwTwips nFormula2Top = getXPath(pXmlDoc,
                                    "/root/page/anchored/fly/txt[2]"
                                    "/anchored/fly[2]/infos/bounds"_ostr,
                                    "top"_ostr)
                               .toInt32();
    SwTwips nFormula2Bottom = nFormula2Top
                              + getXPath(pXmlDoc,
                                         "/root/page/anchored/fly/txt[2]"
                                         "/anchored/fly[2]/infos/bounds"_ostr,
                                         "height"_ostr)
                                    .toInt32();

    // Ensure that the two formula positions are at least between top and bottom of the text frame.
    // The below two are satisfied even without the fix.
    CPPUNIT_ASSERT_GREATEREQUAL(nTxtTop, nFormula1Top);
    CPPUNIT_ASSERT_GREATEREQUAL(nTxtTop, nFormula2Top);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than or equal to : 14423
    // - Actual  : 14828
    // that is, the formula is below the text-frame's y bound.
    CPPUNIT_ASSERT_LESSEQUAL(nTxtBottom, nFormula1Bottom);
    // Similarly for formula # 2 :
    // - Expected less than or equal to : 14423
    // - Actual  : 15035
    // that is, the formula is below the text-frame's y bound.
    CPPUNIT_ASSERT_LESSEQUAL(nTxtBottom, nFormula2Bottom);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf117982)
{
    createSwDoc("tdf117982.docx");
    SwDoc* pDocument = getSwDoc();
    SwDocShell* pShell = pDocument->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text"_ostr,
                       u"FOO AAA"_ustr);
    //The first cell must be "FOO AAA". If not, this means the first cell content not visible in
    //the source document.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf128959)
{
    // no orphan/widow control in table cells
    createSwDoc("tdf128959.docx");
    SwDoc* pDocument = getSwDoc();
    CPPUNIT_ASSERT(pDocument);
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // first two lines of the paragraph in the split table cell on the first page
    // (these lines were completely lost)
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
        "portion"_ostr,
        u"Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Maecenas porttitor congue "_ustr);
    assertXPath(
        pXmlDoc,
        "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout[2]"_ostr,
        "portion"_ostr,
        u"massa. Fusce posuere, magna sed pulvinar ultricies, purus lectus malesuada libero, sit "_ustr);
    // last line of the paragraph in the split table cell on the second page
    assertXPath(
        pXmlDoc,
        "/root/page[2]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
        "portion"_ostr, u"amet commodo magna eros quis urna."_ustr);

    // Also check that the widow control for the paragraph is not turned off:
    sw::TableFrameFormats& rTableFormats = *pDocument->GetTableFrameFormats();
    SwFrameFormat* pTableFormat = rTableFormats[0];
    SwTable* pTable = SwTable::FindTable(pTableFormat);
    const SwTableBox* pCell = pTable->GetTableBox(u"A1"_ustr);
    const SwStartNode* pStartNode = pCell->GetSttNd();
    SwNodeIndex aNodeIndex(*pStartNode);
    ++aNodeIndex;
    const SwTextNode* pTextNode = aNodeIndex.GetNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    // i.e. the original fix only worked as the entire widow / orphan control was switched off.
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(pTextNode->GetSwAttrSet().GetWidows().GetValue()));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf121658)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf121658.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 2 hyphenated words should appear in the document (in the lowercase words).
    // Uppercase words should not be hyphenated.
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf149420)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf149420.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 3 hyphenated words should appear in the document (last paragraph
    // has got a 1 cm hyphenation zone, removing two hyphenations, which visible
    // in the second paragraph).
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']"_ostr, 8);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf149324)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf149324.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 3 hyphenated words should appear in the document (last paragraph
    // has got a 7-character word limit for hyphenation, removing the
    // hyphenation "ex-cept".
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']"_ostr, 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf149248)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("tdf149248.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 1 hyphenated word should appear in the document (last word of the second
    // paragraph). Last word should not be hyphenated for the fourth paragraph
    // (the same paragraph, but with forbidden hyphenation of the last word).
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testWriterImageNoCapture)
{
    createSwDoc("writer-image-no-capture.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nPageLeft = getXPath(pXmlDoc, "//page/infos/bounds"_ostr, "left"_ostr).toInt32();
    sal_Int32 nImageLeft
        = getXPath(pXmlDoc, "//anchored/fly/infos/bounds"_ostr, "left"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than: 284
    // - Actual  : 284
    // i.e. the image position was modified to be inside the page frame ("captured"), even if Word
    // does not do that.
    CPPUNIT_ASSERT_LESS(nPageLeft, nImageLeft);
}

SwRect lcl_getVisibleFlyObjRect(SwWrtShell* pWrtShell)
{
    SwRootFrame* pRoot = pWrtShell->GetLayout();
    SwPageFrame* pPage = static_cast<SwPageFrame*>(pRoot->GetLower());
    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    SwSortedObjs* pDrawObjs = pPage->GetDrawObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDrawObjs->size());
    SwAnchoredObject* pDrawObj = (*pDrawObjs)[0];
    CPPUNIT_ASSERT_EQUAL(u"Rahmen8"_ustr, pDrawObj->GetFrameFormat()->GetName());
    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    pDrawObjs = pPage->GetDrawObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDrawObjs->size());
    pDrawObj = (*pDrawObjs)[0];
    CPPUNIT_ASSERT_EQUAL(u"Rahmen123"_ustr, pDrawObj->GetFrameFormat()->GetName());
    SwRect aFlyRect = pDrawObj->GetObjRect();
    CPPUNIT_ASSERT(pPage->getFrameArea().Contains(aFlyRect));
    return aFlyRect;
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testStableAtPageAnchoredFlyPosition)
{
    // this doc has two page-anchored frames: one tiny on page 3 and one large on page 4.
    // it also has a style:master-page named "StandardEntwurf", which contains some fields.
    // if you add a break to page 2, or append some text to page 4 (or just toggle display field names),
    // the page anchored frame on page 4 vanishes, as it is incorrectly moved out of the page bounds.
    createSwDoc("stable-at-page-anchored-fly-position.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // look up the layout position of the page-bound frame on page four
    SwRect aOrigRect = lcl_getVisibleFlyObjRect(pWrtShell);

    // append some text to the document to trigger bug / relayout
    pWrtShell->SttEndDoc(false);
    pWrtShell->Insert(u"foo"_ustr);

    // get the current position of the frame on page four
    SwRect aRelayoutRect = lcl_getVisibleFlyObjRect(pWrtShell);

    // the anchored frame should not have moved
    CPPUNIT_ASSERT_EQUAL(aOrigRect, aRelayoutRect);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf134548)
{
    createSwDoc("tdf134548.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Second paragraph has two non zero width tabs in beginning of line
    {
        OUString sNodeType = parseDump(
            "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[1]"_ostr, "type"_ostr);
        CPPUNIT_ASSERT_EQUAL(u"PortionType::TabLeft"_ustr, sNodeType);
        sal_Int32 nWidth
            = parseDump("/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[1]"_ostr,
                        "width"_ostr)
                  .toInt32();
        CPPUNIT_ASSERT_GREATER(sal_Int32(0), nWidth);
    }
    {
        OUString sNodeType = parseDump(
            "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[2]"_ostr, "type"_ostr);
        CPPUNIT_ASSERT_EQUAL(u"PortionType::TabLeft"_ustr, sNodeType);
        sal_Int32 nWidth
            = parseDump("/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[2]"_ostr,
                        "width"_ostr)
                  .toInt32();
        CPPUNIT_ASSERT_GREATER(sal_Int32(0), nWidth);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf124423)
{
    createSwDoc("tdf124423.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFly1Width
        = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    sal_Int32 nFly2Width
        = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    sal_Int32 nPageWidth = getXPath(pXmlDoc, "//page/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(nPageWidth, nFly2Width);
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly1Width);

    createSwDoc("tdf124423.odt");
    pXmlDoc = parseLayoutDump();
    nFly1Width
        = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    nFly2Width
        = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    nPageWidth = getXPath(pXmlDoc, "//page/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly2Width);
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly1Width);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf137185)
{
    // First load the sample bugdoc
    createSwDoc("tdf137185.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    // Get the doc shell
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());

    // Get the DrawObject from page
    auto pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
    CPPUNIT_ASSERT(pModel);
    auto pPage = pModel->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    auto pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);

    // Get the format of the draw object
    auto pShape = FindFrameFormat(pObj);
    CPPUNIT_ASSERT(pShape);

    // Check the text of the shape
    uno::Reference<text::XText> xTxt(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Align me!"_ustr, xTxt->getText()->getString());

    // Add a textbox to the shape
    SwTextBoxHelper::create(pShape, pShape->FindRealSdrObject(), true);

    // Check if the text moved from the shape to the frame
    auto pFormat = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));
    auto xTextFrame = SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat);

    CPPUNIT_ASSERT_EQUAL(u"Align me!"_ustr, xTextFrame->getText()->getString());
    SdrTextObj* pTextObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT(pTextObj);
    const auto& aOutStr = pTextObj->GetOutlinerParaObject()->GetTextObject();

    CPPUNIT_ASSERT(aOutStr.GetText(0).isEmpty());
    // Before the patch it failed, because the text appeared 2 times on each other.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf138782)
{
    createSwDoc("tdf138782.docx");
    auto pXml = parseLayoutDump();
    CPPUNIT_ASSERT(pXml);

    // Without the fix it failed because the 3rd shape was outside the page:
    // - Expected less than: 13327
    // - Actual  : 14469

    CPPUNIT_ASSERT_LESS(getXPath(pXml, "/root/page/infos/bounds"_ostr, "right"_ostr).toInt32(),
                        getXPath(pXml,
                                 "/root/page/body/txt[8]/anchored/SwAnchoredDrawObject/bounds"_ostr,
                                 "right"_ostr)
                            .toInt32());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf135035)
{
    createSwDoc("tdf135035.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFly1Width
        = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    sal_Int32 nFly2Width
        = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    sal_Int32 nFly3Width
        = getXPath(pXmlDoc, "(//anchored/fly)[3]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    sal_Int32 nParentWidth
        = getXPath(pXmlDoc, "(//txt)[1]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL(nParentWidth, nFly2Width);
    CPPUNIT_ASSERT_EQUAL(nParentWidth, nFly3Width);
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly1Width);

    createSwDoc("tdf135035.odt");
    pXmlDoc = parseLayoutDump();
    nFly1Width
        = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    nFly2Width
        = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    nFly3Width
        = getXPath(pXmlDoc, "(//anchored/fly)[3]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    nParentWidth = getXPath(pXmlDoc, "(//txt)[1]/infos/prtBounds"_ostr, "width"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly2Width);
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly1Width);
    CPPUNIT_ASSERT_GREATER(nParentWidth, nFly3Width);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf146704_EndnoteInSection)
{
    createSwDoc("tdf146704_EndnoteInSection.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the fix, the endnote placed to 2. page
    assertXPath(pXmlDoc, "/root/page"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf139336_ColumnsWithFootnoteDoNotOccupyEntirePage)
{
    createSwDoc("tdf139336_ColumnsWithFootnoteDoNotOccupyEntirePage.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the fix, it would be 5 pages, but with the fix the whole document
    // would fit into 1 page, but it will be 2 pages right now, because
    // when writer import (from docx) the last section with columns, then it does not set
    // the evenly distributed settings, and this settings is required for the fix now, to
    // avoid some regression.
    assertXPath(pXmlDoc, "/root/page"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf54465_ColumnsWithFootnoteDoNotOccupyEntirePage)
{
    // Old odt files should keep their original layout, as it was before Tdf139336 fix.
    // The new odt file is only 1 page long, while the old odt file (with the same content)
    // was more than 1 page long.
    // Note: Somewhy this test miscalculates the layout of the old odt file.
    // It will be 4 pages long, while opened in Writer it is 5 pages long.
    createSwDoc("tdf54465_ColumnsWithFootnoteDoNotOccupyEntirePage_Old.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    Scheduler::ProcessEventsToIdle();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/root/page"_ostr);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_GREATER(1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);

    discardDumpedLayout();
    createSwDoc("tdf54465_ColumnsWithFootnoteDoNotOccupyEntirePage_New.odt");
    pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf138124)
{
    // When the only portion after the footnote number is a FlyCnt, and it doesn't fit into
    // the page width, it should be moved to the next line without the footnote number, and
    // not loop, nor OOM, nor fail assertions.

    createSwDoc("wideBoxInFootnote.fodt");
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the layout would loop, creating new FootnoteNum portions
    // indefinitely, until OOM.
    // If the footnote paragraph had no orphan control, then the loop would finally end,
    // but an assertion in SwTextPainter::DrawTextLine would fail during paint.

    xmlDocUniquePtr pXml = parseLayoutDump();
    assertXPath(pXml, "/root/page"_ostr, 1);
    assertXPath(pXml, "/root/page/ftncont/ftn/txt/anchored"_ostr, 1);

    // And finally, if there were no assertion in SwTextPainter::DrawTextLine, it would have
    // produced multiple lines with FootnoteNum portions, failing the following check like
    // - Expected: 1
    // - Actual  : 49

    assertXPath(pXml,
                "/root/page/ftncont/ftn/txt//SwFieldPortion[@type='PortionType::FootnoteNum']"_ostr,
                1);
    assertXPath(pXml, "/root/page/ftncont/ftn/txt//SwLinePortion[@type='PortionType::FlyCnt']"_ostr,
                1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf154113)
{
    createSwDoc("three_sections.fodt");
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, u".uno:GoToStartOfDoc"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoToNextPara"_ustr, {});
    dispatchCommand(mxComponent, u".uno:EndOfDocumentSel"_ustr,
                    {}); // to the end of current section!
    dispatchCommand(mxComponent, u".uno:EndOfDocumentSel"_ustr, {}); // to the end of the document.

    auto xModel = mxComponent.queryThrow<frame::XModel>();
    auto xSelected = xModel->getCurrentSelection().queryThrow<container::XIndexAccess>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSelected->getCount());
    auto xRange = xSelected->getByIndex(0).queryThrow<text::XTextRange>();
    CPPUNIT_ASSERT_EQUAL(u"<-- Start selection here. Section1" SAL_NEWLINE_STRING
                         "Section2" SAL_NEWLINE_STRING "Section3. End selection here -->"_ustr,
                         xRange->getString());

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    xSelected = xModel->getCurrentSelection().queryThrow<container::XIndexAccess>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSelected->getCount());
    xRange = xSelected->getByIndex(0).queryThrow<text::XTextRange>();
    CPPUNIT_ASSERT_EQUAL(OUString(), xRange->getString());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    xmlDocUniquePtr pXml = parseLayoutDump();

    // Without the fix in place, this would fail with
    // - Expected: 3
    // - Actual  : 2
    assertXPath(pXml, "/root/page/body/section"_ostr, 3);
    assertXPath(pXml, "/root/page/body/section[1]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, u"<-- Start selection here. Section1"_ustr);
    assertXPath(pXml, "/root/page/body/section[2]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, u"Section2"_ustr);
    assertXPath(pXml, "/root/page/body/section[3]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, u"Section3. End selection here -->"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf155611)
{
    createSwDoc("tdf155611_table_and_nested_section.fodt");
    Scheduler::ProcessEventsToIdle();

    xmlDocUniquePtr pXml = parseLayoutDump();
    CPPUNIT_ASSERT(pXml);

    // Check the layout: single page, two section frames (no section frames after the one for Inner
    // section), correct table structure and content in the first section frame, including nested
    // table in the last cell, and the last section text.
    assertXPath(pXml, "/root/page"_ostr);
    // Without the fix in place, this would fail with
    // - Expected: 2
    // - Actual  : 3
    assertXPath(pXml, "/root/page/body/section"_ostr, 2);
    assertXPath(pXml, "/root/page/body/section[1]/tab"_ostr);
    assertXPath(pXml, "/root/page/body/section[1]/tab/row"_ostr);
    assertXPath(pXml, "/root/page/body/section[1]/tab/row/cell"_ostr, 2);
    assertXPath(pXml, "/root/page/body/section[1]/tab/row/cell[1]/txt/SwParaPortion/SwLineLayout/"
                      "SwParaPortion[@portion='foo']"_ostr);
    assertXPath(pXml, "/root/page/body/section[1]/tab/row/cell[2]/txt/SwParaPortion/SwLineLayout/"
                      "SwParaPortion[@portion='bar']"_ostr);
    assertXPath(pXml, "/root/page/body/section[1]/tab/row/cell[2]/tab/row/cell/txt/SwParaPortion/"
                      "SwLineLayout/SwParaPortion[@portion='baz']"_ostr);
    assertXPath(pXml, "/root/page/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/"
                      "SwParaPortion[@portion='abc']"_ostr);

    // Also must not crash on close because of a frame that accidentally fell off of the layout
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf152307)
{
    // Problem: On a given Writer document a table layout changed
    // after doing Tools -> Update -> Update All. The last table row on page 13
    // was bigger than the page size allowed and thus was hidden behind the footer.

    // load the document
    createSwDoc("tdf152307.odt");

    // do Tools -> Update -> Update All
    dispatchCommand(mxComponent, u".uno:UpdateAllIndexes"_ustr, {});

    // XML dump and some basic assertions
    sal_Int32 nPage = 7, nPages = 0;
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    nPages = countXPathNodes(pXmlDoc, "/root/page"_ostr);
    CPPUNIT_ASSERT_MESSAGE("tdf152307.odt / testTdf152307: Not enough pages.", nPage < nPages);
    assertXPath(pXmlDoc, "/root/page[" + OString::number(nPage) + "]/body/section", 1);

    // Actual test procedure:
    // On page 7, check:
    // How much tables do we have? How much rows does the last table have?
    int nTables
        = countXPathNodes(pXmlDoc, "/root/page[" + OString::number(nPage) + "]/body/section/tab");
    int nRowsLastTable
        = countXPathNodes(pXmlDoc, "/root/page[" + OString::number(nPage) + "]/body/section/tab["
                                       + OString::number(nTables) + "]/row");
    // What is the bottom value of the last table row?
    sal_Int32 nTabBottom = getXPath(pXmlDoc,
                                    "/root/page[" + OString::number(nPage) + "]/body/section/tab["
                                        + OString::number(nTables) + "]/row["
                                        + OString::number(nRowsLastTable) + "]/infos/bounds",
                                    "bottom"_ostr)
                               .toInt32();
    // Where does the footer start (footer/info/bounds/top)?
    sal_Int32 nFooterTop
        = getXPath(pXmlDoc, "/root/page[" + OString::number(nPage) + "]/footer/infos/bounds",
                   "top"_ostr)
              .toInt32();
    // Is the bottom value of the last row above the top value of the footer?
    OString aMsg = "tdf152307.odt / testTdf152307: Bottom value of last table row on page "
                   + OString::number(nPage) + " is below top value of footer: "
                   + OString::number(nTabBottom) + " > " + OString::number(nFooterTop);
    CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(), nTabBottom < nFooterTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf57187_Tdf158900)
{
    // Given a document with a single paragraph, having some long space runs and line breaks
    createSwDoc("space+break.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page, one paragraph, and five lines
    assertXPath(pXmlDoc, "/root/page"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion"_ostr, 1);
    // Without the fix in place, this would fail: there used to be 6 lines
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout"_ostr, 5);

    // tdf#57187: Check that relatively short lines have spaces not participating in layout.
    // First line has 11 spaces in the end, and then a manual line break. It is rather short:
    // without block justification, it is narrower than the available space.
    // It uses the "first check if everything fits to line" return path in SwTextGuess::Guess.
    // Check that the spaces are put into a Hole portion, thus not participating in layout.
    // Without the fix, this would fail: there were only 2 portions, no Hole nor Margin portions.
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*"_ostr, 4);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[1]"_ostr, "type"_ostr,
                u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[1]"_ostr,
                "length"_ostr, u"11"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[2]"_ostr, "type"_ostr,
                u"PortionType::Hole"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[2]"_ostr,
                "length"_ostr, u"11"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[3]"_ostr, "type"_ostr,
                u"PortionType::Break"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[4]"_ostr, "type"_ostr,
                u"PortionType::Margin"_ustr);
    // Second line has 101 spaces in the end, and then a manual line break.
    // It uses the "second check if everything fits to line" return path in SwTextGuess::Guess.
    // Check that the spaces are put into a Hole portion, thus not participating in layout.
    // Without the fix, this would fail: there were only 2 portions, no Hole portion.
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*"_ostr, 3);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*[1]"_ostr, "type"_ostr,
                u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*[1]"_ostr,
                "length"_ostr, u"11"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*[2]"_ostr, "type"_ostr,
                u"PortionType::Hole"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*[2]"_ostr,
                "length"_ostr, u"101"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]/*[3]"_ostr, "type"_ostr,
                u"PortionType::Break"_ustr);

    // tdf#158900: Check that the break after a long line with trailing spaces is kept on same line.
    // Without the fix in place, this would fail: the line had only 2 portions (text + hole),
    // and the break was on a separate third line
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]/*"_ostr, 3);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]/*[1]"_ostr, "type"_ostr,
                u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]/*[2]"_ostr, "type"_ostr,
                u"PortionType::Hole"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[3]/*[3]"_ostr, "type"_ostr,
                u"PortionType::Break"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf147666)
{
    createSwDoc("tdf147666.odt");

    // Move cursor into position to insert image
    dispatchCommand(mxComponent, u".uno:GoToEndOfPara"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});

    save(u"writer8"_ustr);
    sal_Int32 nNonInsertedViewTop
        = getXPathContent(parseExport(u"settings.xml"_ustr),
                          "//config:config-item[@config:name='ViewTop']"_ostr)
              .toInt32();

    // Insert image below the end of the paragraph on page one
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FileName"_ustr, createFileURL(u"tdf147666.png")),
    };
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aArgs);

    save(u"writer8"_ustr);
    sal_Int32 nInsertedViewTop
        = getXPathContent(parseExport(u"settings.xml"_ustr),
                          "//config:config-item[@config:name='ViewTop']"_ostr)
              .toInt32();

    // Without the fix in place this will fail with
    // nInsertedViewTop = nNonInsertedViewTop
    // i.e. when the image is inserted, the view doesn't
    // focus to the inserted graphic
    CPPUNIT_ASSERT_LESS(nInsertedViewTop, nNonInsertedViewTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf159050)
{
    // Given a document with a justified paragraph and a box with optimal wrapping
    createSwDoc("tdf159050-wrap-adjust.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page, one anchored object, one paragraph, and two lines
    assertXPath(pXmlDoc, "/root/page"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/SwAnchoredDrawObject"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout"_ostr, 2);

    // Without the fix, this would fail: there was an unexpected second fly portion.
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*"_ostr, 4);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[1]"_ostr, "type"_ostr,
                u"PortionType::Text"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[1]"_ostr,
                "length"_ostr, u"91"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[2]"_ostr, "type"_ostr,
                u"PortionType::Hole"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[2]"_ostr,
                "length"_ostr, u"1"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[3]"_ostr, "type"_ostr,
                u"PortionType::Fly"_ustr);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/*[4]"_ostr, "type"_ostr,
                u"PortionType::Margin"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf159271)
{
    // Given a document with a field with several spaces in a field content
    createSwDoc("fld-in-tbl.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page, one table with one row and two cells, and one paragraph
    assertXPath(pXmlDoc, "/root/page"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/tab"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/tab/row"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page/body/txt"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt/SwParaPortion"_ostr, 1);

    // Without the fix, this would fail:
    // - Expected: 1
    // - Actual  : 16
    // - In <>, XPath '/root/page/body/tab/row/cell[2]/txt//SwLineLayout' number of nodes is incorrect
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt//SwLineLayout"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt//SwFieldPortion"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf159259)
{
    // Given a document with a block sdt with a single field, having framePr aligned to right
    createSwDoc("sdt+framePr.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page and one paragraph with one line and one anchored object
    assertXPath(pXmlDoc, "/root/page"_ostr, 1);
    // Without the fix, this would fail: there were two paragraphs
    assertXPath(pXmlDoc, "/root/page/body/txt"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout"_ostr, 1);
    // Without the fix, this would fail: there was a field portion in the line
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, 0);
    // Without the fix, this would fail: there was no anchored objects
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly"_ostr, 1);

    const sal_Int32 paraRight
        = getXPath(pXmlDoc, "/root/page/body/txt/infos/bounds"_ostr, "right"_ostr).toInt32();
    const sal_Int32 paraHeight
        = getXPath(pXmlDoc, "/root/page/body/txt/infos/bounds"_ostr, "height"_ostr).toInt32();

    CPPUNIT_ASSERT_GREATER(sal_Int32(0), paraRight);
    CPPUNIT_ASSERT_GREATER(sal_Int32(0), paraHeight);

    const sal_Int32 flyRight
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds"_ostr, "right"_ostr)
              .toInt32();
    const sal_Int32 flyHeight
        = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds"_ostr, "height"_ostr)
              .toInt32();

    CPPUNIT_ASSERT_EQUAL(paraRight, flyRight); // The fly is right-aligned
    CPPUNIT_ASSERT_EQUAL(paraHeight, flyHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testLargeTopParaMarginAfterHiddenSection)
{
    // Given a large top margin in Standard paragraph style, and the first section hidden
    createSwDoc("largeTopMarginAndHiddenFirstSection.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page and two sections, first hidden (zero-height)
    assertXPath(pXmlDoc, "//page"_ostr, 1);
    assertXPath(pXmlDoc, "//page/body/section"_ostr, 2);
    assertXPath(pXmlDoc, "//page/body/section[1]/infos/bounds"_ostr, "height"_ostr, u"0"_ustr);
    // Check that the top margin (1 in = 1440 twip) is added to line height (12 pt = 240 twip)
    assertXPath(pXmlDoc, "//page/body/section[2]/infos/bounds"_ostr, "height"_ostr, u"1680"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testPageBreakInHiddenSection)
{
    // Given a paragraph with page-break-before with page style and page number
    createSwDoc("pageBreakInHiddenSection.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page"_ostr, 4);
    assertXPath(pXmlDoc, "//section"_ostr, 4);
    assertXPath(pXmlDoc, "//page[1]/body/txt"_ostr, 1);
    // The page break inside the hidden section is ignored (otherwise, there would be one section
    // on the first page)
    assertXPath(pXmlDoc, "//page[1]/body/section"_ostr, 2);
    // The first section is hidden
    assertXPath(pXmlDoc, "//page[1]/body/section[1]/infos/bounds"_ostr, "height"_ostr, u"0"_ustr);

    // Page 2 is empty even page (generated by the next page's section with page-break-before)
    assertXPath(pXmlDoc, "//page[2]/body"_ostr, 0);

    // The section on page 3 is not hidden, only text in it is, therefore its page break works
    assertXPath(pXmlDoc, "//page[3]/body/section"_ostr, 1);
    assertXPath(pXmlDoc, "//page[3]/body/section/infos/bounds"_ostr, "height"_ostr, u"0"_ustr);

    // The section on page 4 is hidden, thus page break in it is ignored (no further pages, where
    // the section would be moved to otherwise)
    assertXPath(pXmlDoc, "//page[4]/body/section"_ostr, 1);
    assertXPath(pXmlDoc, "//page[4]/body/section/infos/bounds"_ostr, "height"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf159443)
{
    // Given a document with chart, which have a datatable
    createSwDoc("tdf159443.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    //// Without the fix, this would fail:
    //// - Expected: DataSeries1
    //// - Actual  : 1.25
    //// - In <>, XPath contents of child does not match
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[47]/textarray/text"_ostr,
        u"DataSeries1"_ustr);
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[49]/textarray/text"_ostr,
        u"Category1"_ustr);
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[51]/textarray/text"_ostr,
        u"4.3"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf159422)
{
    // Given a document with chart, which have a datatable
    createSwDoc("charttable.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    //// Without the fix, this would fail:
    //// - Expected: 5877
    //// - Actual  : 5649
    //// - Delta   : 20
    sal_Int32 nYSymbol1
        = getXPath(
              pXmlDoc,
              "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[99]/polypolygon/polygon/point[1]"_ostr,
              "y"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5877, nYSymbol1, 20);
    sal_Int32 nYSymbol2
        = getXPath(
              pXmlDoc,
              "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[100]/polypolygon/polygon/point[1]"_ostr,
              "y"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6225, nYSymbol2, 20);
    sal_Int32 nYSymbol3
        = getXPath(
              pXmlDoc,
              "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[101]/polypolygon/polygon/point[1]"_ostr,
              "y"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6573, nYSymbol3, 20);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf159456)
{
    // Given a document with chart, which have a datatable
    createSwDoc("charttable.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    //// Without the fix, this would fail:
    //// - Expected: 1
    //// - Actual  : 1.5
    //// - In <>, XPath contents of child does not match
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[103]/textarray/text"_ostr,
        u"1"_ustr);
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push[104]/textarray/text"_ostr,
        u"2"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, test_i84870)
{
    // Given a document with a large as-char object, alone in its paragraph, shifted down by a
    // header object: it must not hang in a layout loop on import
    createSwDoc("i84870.fodt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf160549)
{
    // Given a document with a large as-char object, alone in its paragraph, shifted down by a
    // header object: it must not hang in a layout loop on import (similar to i84870, but not
    // fixed by its fix)
    createSwDoc("tdf160549.fodt");
    // The object is the first in the document; it must not move to the next page
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf160526)
{
    // Given a document with a large as-char object, alone in its paragraph, shifted down by
    // another body object
    createSwDoc("tdf160526.fodt");
    // It must move to the next page
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    auto pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page[2]/body/txt/anchored/SwAnchoredDrawObject"_ostr);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf160958_page_break)
{
    // Given a document with a section with the first paragraph having a page break
    createSwDoc("tdf160958_page_break.fodt");
    auto pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page"_ostr, 2);
    // A single paragraph on the first page, with 6 lines
    assertXPath(pExportDump, "//page[1]/body/txt"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt/SwParaPortion/SwLineLayout"_ostr, 6);
    // A section with 7 paragraphs, and two more paragraphs after the section
    assertXPath(pExportDump, "//page[2]/body/section"_ostr, 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt"_ostr, 7);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[4]/SwParaPortion/SwLineLayout"_ostr, 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[5]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[6]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[7]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/txt"_ostr, 2);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 7);
    assertXPath(pExportDump, "//page[2]/body/txt[2]/SwParaPortion"_ostr, 0);

    // Hide the section
    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection = xSections->getByName(u"Section1"_ustr).queryThrow<css::beans::XPropertySet>();
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(false));

    discardDumpedLayout();
    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page"_ostr, 1);
    // Three paragraphs and a hidden section on the first page
    assertXPath(pExportDump, "//page/body/txt"_ostr, 3);
    assertXPath(pExportDump, "//page/body/section"_ostr, 1);

    assertXPath(pExportDump, "//page/body/section/infos/bounds"_ostr, "height"_ostr, "0");
    assertXPath(pExportDump, "//page/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 6);
    assertXPath(pExportDump, "//page/body/section/txt"_ostr, 7);
    assertXPath(pExportDump, "//page/body/section/txt[1]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page/body/section/txt[2]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page/body/section/txt[3]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page/body/section/txt[4]/SwParaPortion/SwLineLayout"_ostr, 5);
    assertXPath(pExportDump, "//page/body/section/txt[5]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page/body/section/txt[6]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page/body/section/txt[7]/SwParaPortion"_ostr, 0);

    assertXPath(pExportDump, "//page/body/txt[2]/SwParaPortion/SwLineLayout"_ostr, 7);
    assertXPath(pExportDump, "//page/body/txt[3]/SwParaPortion"_ostr, 0);

    // Show the section again
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(true));

    // Check that the layout has been restored
    discardDumpedLayout();
    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page"_ostr, 2);
    assertXPath(pExportDump, "//page[1]/body/txt"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt/SwParaPortion/SwLineLayout"_ostr, 6);
    assertXPath(pExportDump, "//page[2]/body/section"_ostr, 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt"_ostr, 7);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[4]/SwParaPortion/SwLineLayout"_ostr, 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[5]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[6]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[7]/SwParaPortion"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/txt"_ostr, 2);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 7);
    assertXPath(pExportDump, "//page[2]/body/txt[2]/SwParaPortion"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf160958_orphans)
{
    // Given a document with a section which moves to the next page as a whole, because of orphans
    createSwDoc("tdf160958_orphans_move_section.fodt");
    auto pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page"_ostr, 2);
    // 21 paragraphs on the first page
    assertXPath(pExportDump, "//page[1]/body/txt"_ostr, 21);
    assertXPath(pExportDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 6);
    assertXPath(pExportDump, "//page[1]/body/txt[2]/SwParaPortion/SwLineLayout"_ostr, 5);
    assertXPath(pExportDump, "//page[1]/body/txt[3]/SwParaPortion/SwLineLayout"_ostr, 7);
    assertXPath(pExportDump, "//page[1]/body/txt[4]/SwParaPortion/SwLineLayout"_ostr, 16);
    assertXPath(pExportDump, "//page[1]/body/txt[5]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[6]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[7]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[8]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[9]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[10]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[11]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[12]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[13]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[14]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[15]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[16]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[17]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[18]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[19]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[20]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[21]/SwParaPortion/SwLineLayout"_ostr, 1);
    // A section and one more paragraph after the section
    assertXPath(pExportDump, "//page[2]/body/section"_ostr, 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt"_ostr, 3);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/SwParaPortion/SwLineLayout"_ostr, 6);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/SwParaPortion/SwLineLayout"_ostr, 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/SwParaPortion/SwLineLayout"_ostr, 7);
    assertXPath(pExportDump, "//page[2]/body/txt"_ostr, 1);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 1);

    // Hide the section
    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection = xSections->getByName(u"Section1"_ustr).queryThrow<css::beans::XPropertySet>();
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(false));

    discardDumpedLayout();
    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page"_ostr, 1);
    assertXPath(pExportDump, "//page/body/txt"_ostr, 22);
    assertXPath(pExportDump, "//page/body/section"_ostr, 1);
    assertXPath(pExportDump, "//page/body/section/infos/bounds"_ostr, "height"_ostr, "0");

    // Show the section again
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(true));

    // Check that the layout has been restored
    discardDumpedLayout();
    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page"_ostr, 2);
    assertXPath(pExportDump, "//page[1]/body/txt"_ostr, 21);
    assertXPath(pExportDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 6);
    assertXPath(pExportDump, "//page[1]/body/txt[2]/SwParaPortion/SwLineLayout"_ostr, 5);
    assertXPath(pExportDump, "//page[1]/body/txt[3]/SwParaPortion/SwLineLayout"_ostr, 7);
    assertXPath(pExportDump, "//page[1]/body/txt[4]/SwParaPortion/SwLineLayout"_ostr, 16);
    assertXPath(pExportDump, "//page[1]/body/txt[5]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[6]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[7]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[8]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[9]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[10]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[11]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[12]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[13]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[14]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[15]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[16]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[17]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[18]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[19]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[20]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[21]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[2]/body/section"_ostr, 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt"_ostr, 3);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/SwParaPortion/SwLineLayout"_ostr, 6);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/SwParaPortion/SwLineLayout"_ostr, 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/SwParaPortion/SwLineLayout"_ostr, 7);
    assertXPath(pExportDump, "//page[2]/body/txt"_ostr, 1);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf161368)
{
    // Given a document with a text body width of 116 mm, greater than 65535 twips (115.6 mm)
    createSwDoc("tdf161368.fodt");
    auto pExportDump = parseLayoutDump();
    // one page, three paragraphs, each one line (it was four pages, each paragraph split into
    // tens of short (<= 4 mm wide) lines)
    assertXPath(pExportDump, "//page"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt"_ostr, 3);
    assertXPath(pExportDump, "//page[1]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[2]/SwParaPortion/SwLineLayout"_ostr, 1);
    assertXPath(pExportDump, "//page[1]/body/txt[3]/SwParaPortion/SwLineLayout"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, TestCrashHyphenation)
{
    //just care it doesn't crash/assert
    createSwDoc("crashHyphen.fodt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, TestTdf161508)
{
    // This document must not hang on load.
    createSwDoc("tdf161508.fodt");
    auto pExportDump = parseLayoutDump();
    // The table must move completely to the second page
    assertXPath(pExportDump, "//page[1]/body/tab"_ostr, 0);
    assertXPath(pExportDump, "//page[2]/body/tab"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, TestTdf92091)
{
    // This test verifies that RTL text following an LTR footnote is measured correctly
    createSwDoc("tdf92091.fodt");
    auto pXmlDoc = parseLayoutDump();

    sal_Int32 nLayoutWidth
        = getXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout"_ostr,
                   "width"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(3210), nLayoutWidth);

    sal_Int32 nPor1Width
        = getXPath(pXmlDoc,
                   "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                   "width"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(55), nPor1Width);

    sal_Int32 nPor2Width
        = getXPath(pXmlDoc,
                   "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]"_ostr,
                   "width"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(75), nPor2Width);

    sal_Int32 nPor3Width
        = getXPath(pXmlDoc,
                   "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                   "width"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(2870), nPor3Width);

    sal_Int32 nPor4Width
        = getXPath(pXmlDoc,
                   "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]"_ostr,
                   "width"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(75), nPor4Width);

    sal_Int32 nPor5Width
        = getXPath(pXmlDoc,
                   "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                   "width"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(110), nPor5Width);
}

} // end of anonymous namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
