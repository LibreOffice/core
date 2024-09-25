/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <unotest/bootstrapfixturebase.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <comphelper/scopeguard.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/event.hxx>
#include <vcl/scheduler.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/unolingu.hxx>
#include <svx/svdpage.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtcntnt.hxx>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <bodyfrm.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <ndtxt.hxx>
#include <frmatr.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <fldmgr.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/extras/layout/data/";

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter3 : public SwModelTestBase
{
protected:

    SwDoc* createDoc(const char* pName = nullptr);

    uno::Any executeMacro(const OUString& rScriptURL,
                          const uno::Sequence<uno::Any>& rParams = {});
};

SwDoc* SwLayoutWriter3::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

uno::Any SwLayoutWriter3::executeMacro(const OUString& rScriptURL,
                                  const uno::Sequence<uno::Any>& rParams)
{
    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;

    ErrCode result = SfxObjectShell::CallXScript(mxComponent, rScriptURL, rParams, aRet,
                                                 aOutParamIndex, aOutParam);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, result);

    return aRet;
}

static void lcl_dispatchCommand(const uno::Reference<lang::XComponent>& xComponent,
                                const OUString& rCommand,
                                const uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    uno::Reference<frame::XController> xController
        = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY_THROW)->getCurrentController();
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xContext));
    CPPUNIT_ASSERT(xDispatchHelper.is());

    xDispatchHelper->executeDispatch(xFrame, rCommand, OUString(), 0, rPropertyValues);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf117188)
{
    createDoc("tdf117188.docx");
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    }));
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    loadURL(aTempFile.GetURL(), "tdf117188.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    OUString sWidth = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "width");
    OUString sHeight = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "height");
    // The text box must have zero border distances
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "left", "0");
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "top", "0");
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "width", sWidth);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "height", sHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf117187)
{
    createDoc("tdf117187.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();

    // there should be no fly portions
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[@nType='PortionType::Fly']", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf119875)
{
    createDoc("tdf119875.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();

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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf120287)
{
    createDoc("tdf120287.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin Word-specific compat flag did not imply
    // default-in-Word printer-independent layout, resulting in an additional
    // line break.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf120287b)
{
    createDoc("tdf120287b.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 1418, TabOverMargin did the right split of the paragraph to two
    // lines, but then calculated a too large tab portion size on the first
    // line.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/Text[@nType='PortionType::TabRight']", "nWidth",
                "17");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf120287c)
{
    createDoc("tdf120287c.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 2, the second line was not broken into a 2nd and a 3rd one,
    // rendering text outside the paragraph frame.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf122878)
{
    createDoc("tdf122878.docx");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // FIXME: the XPath should be adjusted when the proper floating table would be imported
    const sal_Int32 nTblTop
        = getXPath(pXmlDoc, "/root/page[1]/footer/txt/anchored/fly/tab/infos/bounds", "top")
              .toInt32();
    const sal_Int32 nFirstPageParaCount
        = getXPathContent(pXmlDoc, "count(/root/page[1]/body/txt)").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(30), nFirstPageParaCount);
    for (sal_Int32 i = 1; i <= nFirstPageParaCount; ++i)
    {
        const OString xPath = "/root/page[1]/body/txt[" + OString::number(i) + "]/infos/bounds";
        const sal_Int32 nTxtBottom = getXPath(pXmlDoc, xPath.getStr(), "top").toInt32()
                                     + getXPath(pXmlDoc, xPath.getStr(), "height").toInt32();
        // No body paragraphs should overlap the table in the footer
        CPPUNIT_ASSERT_MESSAGE(OString("testing paragraph #" + OString::number(i)).getStr(),
                               nTxtBottom <= nTblTop);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf115094)
{
    createDoc("tdf115094.docx");
    xmlDocPtr pXmlDoc = parseLayoutDump();

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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf122607)
{
    createDoc("tdf122607.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/Text[1]",
                "nHeight", "253");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/Text[1]",
                "nWidth", "428");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/Text[1]",
                "Portion", "Fax:");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf122607_regression)
{
    discardDumpedLayout();
    if (mxComponent.is())
        mxComponent->dispose();

    OUString const pName("tdf122607_leerzeile.odt");

    OUString const url(m_directories.getURLFromSrc(DATA_DIRECTORY) + pName);

    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue("Hidden", -1, uno::Any(true), beans::PropertyState_DIRECT_VALUE) },
    };

    std::cout << pName << ":\n";

    // inline the loading because currently properties can't be passed...
    mxComponent = loadFromDesktop(url, "com.sun.star.text.TextDocument",
                                  comphelper::containerToSequence(aFilterOptions));

    CPPUNIT_ASSERT(mxComponent.is());

    uno::Sequence<beans::PropertyValue> props(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer_pdf_Export")) },
    }));
    utl::TempFile aTempFile;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), props);

    xmlDocPtr pXmlDoc = parseLayoutDump();
    // somehow these 2 rows overlapped in the PDF unless CalcLayout() runs
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds", "mbFixSize",
                "false");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds", "top", "2977");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds", "height", "241");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds", "mbFixSize",
                "true");
    // this was 3034, causing the overlap
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds", "top", "3218");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds", "height", "164");

    aTempFile.EnableKillingFile();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testBtlrCell)
{
    SwDoc* pDoc = createDoc("btlr-cell.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed, as
    // the orientation was 0 (layout did not take btlr direction request from
    // doc model).
    assertXPath(pXmlDoc, "//font[1]", "orientation", "900");

#if !defined(MACOSX) && !defined(_WIN32) // macOS fails with x == 2662 for some reason.
    // Without the accompanying fix in place, this test would have failed with 'Expected: 1915;
    // Actual  : 1756', i.e. the AAA1 text was too close to the left cell border due to an ascent vs
    // descent mismatch when calculating the baseline offset of the text portion.
    assertXPath(pXmlDoc, "//textarray[1]", "x", "1915");
    assertXPath(pXmlDoc, "//textarray[1]", "y", "2707");

    // Without the accompanying fix in place, this test would have failed with 'Expected: 1979;
    // Actual  : 2129', i.e. the gray background of the "AAA2." text was too close to the right edge
    // of the text portion. Now it's exactly behind the text portion.
    assertXPath(pXmlDoc, "//rect[@top='2159']", "left", "1979");

    // Without the accompanying fix in place, this test would have failed with 'Expected: 269;
    // Actual  : 0', i.e. the AAA2 frame was not visible due to 0 width.
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt[2]/infos/bounds", "width", "269");

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
    sal_uLong nNodeIndex = pWrtShell->GetCursor()->Start()->nNode.GetIndex();
    sal_Int32 nIndex = pWrtShell->GetCursor()->Start()->nContent.GetIndex();
    KeyEvent aKeyEvent(0, KEY_UP);
    SwEditWin& rEditWin = pShell->GetView()->GetEditWin();
    rEditWin.KeyInput(aKeyEvent);
    Scheduler::ProcessEventsToIdle();
    // Without the accompanying fix in place, this test would have failed: "up" was interpreted as
    // logical "left", which does nothing if you're at the start of the text anyway.
    CPPUNIT_ASSERT_EQUAL(nIndex + 1, pWrtShell->GetCursor()->Start()->nContent.GetIndex());

    // Test that pressing "right" goes to the next paragraph (logical "down").
    sal_Int32 nContentIndex = pWrtShell->GetCursor()->Start()->nContent.GetIndex();
    aKeyEvent = KeyEvent(0, KEY_RIGHT);
    rEditWin.KeyInput(aKeyEvent);
    Scheduler::ProcessEventsToIdle();
    // Without the accompanying fix in place, this test would have failed: the cursor went to the
    // paragraph after the table.
    CPPUNIT_ASSERT_EQUAL(nNodeIndex + 1, pWrtShell->GetCursor()->Start()->nNode.GetIndex());

    // Test that we have the correct character index after traveling to the next paragraph.
    // Without the accompanying fix in place, this test would have failed: char position was 5, i.e.
    // the cursor jumped to the end of the paragraph for no reason.
    CPPUNIT_ASSERT_EQUAL(nContentIndex, pWrtShell->GetCursor()->Start()->nContent.GetIndex());

    // Test that clicking "below" the second paragraph positions the cursor at the start of the
    // second paragraph.
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
    SwCursorMoveState aState(MV_NONE);
    pLayout->GetCursorOfst(&aPosition, aPoint, &aState);
    CPPUNIT_ASSERT_EQUAL(aCellStart.nNode.GetIndex() + 1, aPosition.nNode.GetIndex());
    // Without the accompanying fix in place, this test would have failed: character position was 5,
    // i.e. cursor was at the end of the paragraph.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aPosition.nContent.GetIndex());

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
        CPPUNIT_ASSERT_MESSAGE(ss.str(), aCellRect.IsInside(rRect));
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
    createDoc("tdf123898.odt");

    // Make sure spellchecker has done its job already
    Scheduler::ProcessEventsToIdle();

    xmlDocPtr pXmlDoc = parseLayoutDump();
    // Make sure that the arrow on the left is not there (there are 43 children if it's there)
    assertXPathChildren(pXmlDoc, "/root/page/body/txt/anchored/fly/txt", 42);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf123651)
{
    createDoc("tdf123651.docx");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with 'Expected: 7639;
    // Actual: 12926'. The shape was below the second "Lorem ipsum" text, not above it.
    assertXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "top", "7639");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf116501)
{
    //just care it doesn't freeze
    createDoc("tdf116501.odt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testAbi11870)
{
    //just care it doesn't assert
    createDoc("abi11870-2.odt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testOfz64109)
{
    //just care it doesn't assert
    createDoc("ofz64109-1.fodt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf118719)
{
    // Insert a page break.
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Enable hide whitespace mode.
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetHideWhitespaceMode(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    pWrtShell->Insert("first");
    pWrtShell->InsertPageBreak();
    pWrtShell->Insert("second");

    // Without the accompanying fix in place, this test would have failed, as the height of the
    // first page was 15840 twips, instead of the much smaller 276.
    sal_Int32 nOther = parseDump("/root/page[1]/infos/bounds", "height").toInt32();
    sal_Int32 nLast = parseDump("/root/page[2]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(nOther, nLast);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTabOverMargin)
{
    createDoc("tab-over-margin.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();

    // 2nd paragraph has a tab over the right margin, and with the TabOverMargin compat option,
    // there is enough space to have all content in a single line.
    // Without the accompanying fix in place, this test would have failed, there were 2 lines.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/LineBreak", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testImageComment)
{
    // Load a document that has "aaa" in it, then a commented image (4th char is the as-char image,
    // 5th char is the comment anchor).
    SwDoc* pDoc = createDoc("image-comment.odt");
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
    pTextFrame->GetCursorOfst(&aPosition, aPoint);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5
    // - Actual  : 4
    // i.e. the cursor got positioned between the image and its comment, so typing extended the
    // comment, instead of adding content after the commented image.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), aPosition.nContent.GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testScriptField)
{
    // Test clicking script field inside table ( tdf#141079 )
    SwDoc* pDoc = createDoc("tdf141079.odt");
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
    pCellTextFrame->GetCursorOfst(&aPosition, aPoint);
    // Position was 1 without the fix from tdf#141079
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aPosition.nContent.GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf64222)
{
    createDoc("tdf64222.docx");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/Special", "nHeight", "560");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf113014)
{
    SwDoc* pDoc = createDoc("tdf113014.fodt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if numbering of cell A1 is missing
    // (A1: left indent: 3 cm, first line indent: -3 cm
    // A2: left indent: 0 cm, first line indent: 0 cm)
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text", "1.");
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[3]/text", "2.");
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[5]/text", "3.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf127235)
{
    SwDoc* pDoc = createDoc("tdf127235.odt");
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testShapeAllowOverlap)
{
// Need to find out why this fails on macOS.
#ifndef MACOSX
    // Create an empty document with two, intentionally overlapping shapes.
    // Set their AllowOverlap property to false.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    awt::Point aPoint(1000, 1000);
    awt::Size aSize(2000, 2000);
    uno::Reference<drawing::XShape> xShape(
        xDocument->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xDocument, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("AllowOverlap", uno::makeAny(false));
    xShapeProperties->setPropertyValue("AnchorType",
                                       uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
    xDrawPageSupplier->getDrawPage()->add(xShape);

    aPoint = awt::Point(2000, 2000);
    xShape.set(xDocument->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    xShapeProperties.set(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("AllowOverlap", uno::makeAny(false));
    xShapeProperties->setPropertyValue("AnchorType",
                                       uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
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
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    awt::Point aPoint(1000, 1000);
    awt::Size aSize(2000, 2000);
    uno::Reference<drawing::XShape> xShape(
        xDocument->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xDocument, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("AllowOverlap", uno::makeAny(false));
    xShapeProperties->setPropertyValue("AnchorType",
                                       uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProperties->setPropertyValue("Surround", uno::makeAny(text::WrapTextMode_THROUGH));
    xDrawPageSupplier->getDrawPage()->add(xShape);

    aPoint = awt::Point(2000, 2000);
    xShape.set(xDocument->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    xShapeProperties.set(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("AllowOverlap", uno::makeAny(false));
    xShapeProperties->setPropertyValue("AnchorType",
                                       uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProperties->setPropertyValue("Surround", uno::makeAny(text::WrapTextMode_THROUGH));
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
    CPPUNIT_ASSERT(pSecond->GetObjRect().IsOver(pFirst->GetObjRect()));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf124600)
{
    createDoc("tdf124600.docx");
    xmlDocPtr pXmlDoc = parseLayoutDump();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the last line in the body text had 2 lines, while it should have 1, as Word does (as the
    // fly frame does not intersect with the print area of the paragraph.)
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/LineBreak", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf124601)
{
    // This is a testcase for the ContinuousEndnotes compat flag.
    // The document has 2 pages, the endnote anchor is on the first page.
    // The endnote should be on the 2nd page together with the last page content.
    createDoc("tdf124601.doc");
    xmlDocPtr pXmlDoc = parseLayoutDump();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. there was a separate endnote page, even when the ContinuousEndnotes compat option was
    // on.
    assertXPath(pXmlDoc, "/root/page", 2);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf124601b)
{
    // Table has an image, which is anchored in the first row, but its vertical position is large
    // enough to be rendered in the second row.
    // The shape has layoutInCell=1, so should match what Word does here.
    // Also the horizontal position should be in the last column, even if the anchor is in the
    // last-but-one column.
    createDoc("tdf124601b.doc");
    xmlDocPtr pXmlDoc = parseLayoutDump();

    sal_Int32 nFlyTop = getXPath(pXmlDoc, "//fly/infos/bounds", "top").toInt32();
    sal_Int32 nFlyLeft = getXPath(pXmlDoc, "//fly/infos/bounds", "left").toInt32();
    sal_Int32 nFlyRight = nFlyLeft + getXPath(pXmlDoc, "//fly/infos/bounds", "width").toInt32();
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
    SwDoc* pDoc = createDoc();
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
                        svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END - 1>{});
    SvxFontItem aFont(RES_CHRATR_FONT);
    aFont.SetFamilyName("Liberation Serif");
    aTextSet.Put(aFont);
    SvxFontHeightItem aHeight(400, 100, RES_CHRATR_FONTSIZE);
    aTextSet.Put(aHeight);
    SvxPostureItem aItalic(ITALIC_NORMAL, RES_CHRATR_POSTURE);
    aTextSet.Put(aItalic);
    pWrtShell->SetAttrSet(aTextSet);

    // Insert the text.
    pWrtShell->Insert2("HHH");

    xmlDocPtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the italic string was broken into 2 lines, while Word kept it in a single line.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testContinuousEndnotesInsertPageAtStart)
{
    // Create a new document with CONTINUOUS_ENDNOTES enabled.
    SwDoc* pDoc = createDoc();
    pDoc->getIDocumentSettingAccess().set(DocumentSettingId::CONTINUOUS_ENDNOTES, true);

    // Insert a second page, and an endnote on the 2nd page (both the anchor and the endnote is on
    // the 2nd page).
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();
    pWrtShell->InsertFootnote("endnote", /*bEndNote=*/true, /*bEdit=*/false);

    // Add a new page at the start of the document.
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->InsertPageBreak();

    // Make sure that the endnote is moved from the 2nd page to the 3rd one.
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 3);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the footnote container remained on page 2.
    assertXPath(pXmlDoc, "/root/page[3]/ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testContinuousEndnotesDeletePageAtStart)
{
    // Create a new document with CONTINUOUS_ENDNOTES enabled.
    SwDoc* pDoc = createDoc();
    pDoc->getIDocumentSettingAccess().set(DocumentSettingId::CONTINUOUS_ENDNOTES, true);

    // Insert a second page, and an endnote on the 2nd page (both the anchor and the endnote is on
    // the 2nd page).
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertPageBreak();
    pWrtShell->InsertFootnote("endnote", /*bEndNote=*/true, /*bEdit=*/false);

    // Remove the empty page at the start of the document.
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->DelRight();

    // Make sure that the endnote is moved from the 2nd page to the 1st one.
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the endnote remained on an (otherwise) empty 2nd page.
    assertXPath(pXmlDoc, "/root/page", 1);
    assertXPath(pXmlDoc, "/root/page[1]/ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf128399)
{
    SwDoc* pDoc = createDoc("tdf128399.docx");
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
    SwCursorMoveState aState(MV_NONE);
    pLayout->GetCursorOfst(&aPosition, aPoint, &aState);
    // Second row is +3: end node, start node and the first text node in the 2nd row.
    sal_uLong nExpected = aFirstRow.nNode.GetIndex() + 3;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14
    // - Actual  : 11
    // i.e. clicking on the center of the 2nd row placed the cursor in the 1st row.
    CPPUNIT_ASSERT_EQUAL(nExpected, aPosition.nNode.GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testSectionUnhide)
{
    createDoc("hiddensection.fodt");

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/section/txt/infos/bounds[@height='0']", 0);
        discardDumpedLayout();
    }

    // Hide the section
    uno::Reference<css::text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY_THROW);
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    uno::Reference<css::beans::XPropertySet> xSection(xSections->getByName("Section1"), uno::UNO_QUERY_THROW);
    xSection->setPropertyValue("IsVisible", css::uno::Any(false));
    calcLayout();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/section/txt/infos/bounds[@height='0']", 4);
        discardDumpedLayout();
    }

    xSection->setPropertyValue("IsVisible", css::uno::Any(true));
    calcLayout();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        // the problem was that 3 of the text frames had 0 height because Format was skipped
        assertXPath(pXmlDoc, "/root/page/body/section/txt/infos/bounds[@height='0']", 0);
        discardDumpedLayout();
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testHiddenSectionFlys)
{
    SwDoc* pDoc = createDoc("U-min.fodt");

    //NO! field update job masks if the visibility was created wrong when loading.
    //Scheduler::ProcessEventsToIdle();

    IDocumentDrawModelAccess const& rIDMA{ pDoc->getIDocumentDrawModelAccess() };
    SdrPage const* pDrawPage{ rIDMA.GetDrawModel()->GetPage(0) };
    int invisibleHeaven{ rIDMA.GetInvisibleHeavenId().get() };
    int visibleHeaven{ rIDMA.GetHeavenId().get() };

    // these are hidden by moving to invisible layer, they're still in layout
    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//anchored/fly", 6);
        discardDumpedLayout();

        CPPUNIT_ASSERT_EQUAL(size_t(6), pDrawPage->GetObjCount());
        for (int i = 0; i < 6; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(invisibleHeaven, int(pDrawPage->GetObj(i)->GetLayer().get()));
        }
    }

    // Show the section
    uno::Reference<css::text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY_THROW);
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    uno::Reference<css::beans::XPropertySet> xSection(xSections->getByName("Anlage"), uno::UNO_QUERY_THROW);
    xSection->setPropertyValue("IsVisible", css::uno::Any(true));
    calcLayout();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//anchored/fly", 6);
        discardDumpedLayout();

        CPPUNIT_ASSERT_EQUAL(size_t(6), pDrawPage->GetObjCount());
        for (int i = 0; i < 6; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(visibleHeaven, int(pDrawPage->GetObj(i)->GetLayer().get()));
        }
    }

    xSection->setPropertyValue("IsVisible", css::uno::Any(false));
    calcLayout();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//anchored/fly", 6);
        discardDumpedLayout();

        CPPUNIT_ASSERT_EQUAL(size_t(6), pDrawPage->GetObjCount());
        for (int i = 0; i < 6; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(invisibleHeaven, int(pDrawPage->GetObj(i)->GetLayer().get()));
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testHiddenSectionPageDescs)
{
    createDoc("hidden-sections-with-pagestyles.odt");

    // hide these just so that the height of the section is what is expected;
    // otherwise height depends on which tests run previously
    uno::Sequence<beans::PropertyValue> argsSH(
        comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", uno::Any(false) } }));
    lcl_dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", argsSH);
    uno::Sequence<beans::PropertyValue> args(
        comphelper::InitPropertySequence({ { "Fieldnames", uno::Any(false) } }));
    lcl_dispatchCommand(mxComponent, ".uno:Fieldnames", args);
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page", 2);
        assertXPath(pXmlDoc, "/root/page[1]", "formatName", "Hotti");
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]", "formatName",
                    u"Verfügung");
        assertXPath(pXmlDoc, "/root/page[2]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]", "formatName",
                    u"Verfügung");
        // should be > 0, no idea why it's different on Windows
#ifdef _WIN32
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "height",
                    "552");
#else
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "height",
                    "532");
#endif
        assertXPath(pXmlDoc, "/root/page[2]/body/section[2]", "formatName",
                    "Rueckantwort");
        assertXPath(pXmlDoc, "/root/page[2]/body/section[2]/infos/bounds", "height", "0");
        assertXPath(pXmlDoc, "/root/page[2]", "formatName", "Folgeseite");
        discardDumpedLayout();
    }

    // toggle one section hidden and other visible
    executeMacro(
        "vnd.sun.star.script:Standard.Module1.Main?language=Basic&location=document");
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page", 3);
        assertXPath(pXmlDoc, "/root/page[1]", "formatName", "Hotti");
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]", "formatName",
                    u"Verfügung");
        assertXPath(pXmlDoc, "/root/page[1]/body/section[2]", "formatName",
                    "Rueckantwort");
        assertXPath(pXmlDoc, "/root/page[2]", "formatName", "Empty Page");
        assertXPath(pXmlDoc, "/root/page[3]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]", "formatName",
                    "Rueckantwort");
        // should be > 0, no idea why it's different on Windows
#ifdef _WIN32
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/infos/bounds", "height",
                    "552");
#else
        assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/infos/bounds", "height",
                    "532");
#endif
        assertXPath(pXmlDoc, "/root/page[3]", "formatName", "RueckantwortRechts");
        discardDumpedLayout();
    }

    // toggle one section hidden and other visible
    executeMacro(
        "vnd.sun.star.script:Standard.Module1.Main?language=Basic&location=document");
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page", 2);
        assertXPath(pXmlDoc, "/root/page[1]", "formatName", "Hotti");
        assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/section[1]", "formatName",
                    u"Verfügung");
        assertXPath(pXmlDoc, "/root/page[2]/body/section", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]", "formatName",
                    u"Verfügung");
        // should be > 0, no idea why it's different on Windows
#ifdef _WIN32
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "height",
                    "552");
#else
        assertXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "height",
                    "532");
#endif
        assertXPath(pXmlDoc, "/root/page[2]/body/section[2]", "formatName",
                    "Rueckantwort");
        assertXPath(pXmlDoc, "/root/page[2]/body/section[2]/infos/bounds", "height", "0");
        assertXPath(pXmlDoc, "/root/page[2]", "formatName", "Folgeseite");
        discardDumpedLayout();
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTable0HeightRows)
{
    createDoc("table-0-height-rows.fodt");

    xmlDocPtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // the problem was that the table was erroneously split across 2 or 3 pages
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 28);
    // xpath doesn't work on this branch? assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/infos/bounds[@height='0']", 25);
    assertXPath(pXmlDoc, "/root/page", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf105481)
{
    createDoc("tdf105481.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed
    // because the vertical position of the as-char shape object and the
    // as-char math object will be wrong (below/beyond the text frame's bottom).

    SwTwips nTxtTop = getXPath(pXmlDoc,
                               "/root/page/anchored/fly/txt[2]"
                               "/infos/bounds",
                               "top")
                          .toInt32();
    SwTwips nTxtBottom = nTxtTop
                         + getXPath(pXmlDoc,
                                    "/root/page/anchored/fly/txt[2]"
                                    "/infos/bounds",
                                    "height")
                               .toInt32();

    SwTwips nFormula1Top = getXPath(pXmlDoc,
                                    "/root/page/anchored/fly/txt[2]"
                                    "/anchored/fly[1]/infos/bounds",
                                    "top")
                               .toInt32();
    SwTwips nFormula1Bottom = nFormula1Top
                              + getXPath(pXmlDoc,
                                         "/root/page/anchored/fly/txt[2]"
                                         "/anchored/fly[1]/infos/bounds",
                                         "height")
                                    .toInt32();

    SwTwips nFormula2Top = getXPath(pXmlDoc,
                                    "/root/page/anchored/fly/txt[2]"
                                    "/anchored/fly[2]/infos/bounds",
                                    "top")
                               .toInt32();
    SwTwips nFormula2Bottom = nFormula2Top
                              + getXPath(pXmlDoc,
                                         "/root/page/anchored/fly/txt[2]"
                                         "/anchored/fly[2]/infos/bounds",
                                         "height")
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
    SwDoc* pDocument = createDoc("tdf117982.docx");
    SwDocShell* pShell = pDocument->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text", "FOO AAA");
    //The first cell must be "FOO AAA". If not, this means the first cell content not visible in
    //the source document.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf121658)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    createDoc("tdf121658.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();

    // Only 2 hyphenated words should appear in the document (in the lowercase words).
    // Uppercase words should not be hyphenated.
    assertXPath(pXmlDoc, "//Special[@nType='PortionType::Hyphen']", 2);
}

static SwRect lcl_getVisibleFlyObjRect(SwWrtShell* pWrtShell)
{
    SwRootFrame* pRoot = pWrtShell->GetLayout();
    SwPageFrame* pPage = static_cast<SwPageFrame*>(pRoot->GetLower());
    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    SwSortedObjs* pDrawObjs = pPage->GetDrawObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDrawObjs->size());
    SwAnchoredObject* pDrawObj = (*pDrawObjs)[0];
    CPPUNIT_ASSERT_EQUAL(OUString("Rahmen8"), pDrawObj->GetFrameFormat().GetName());
    pPage = static_cast<SwPageFrame*>(pPage->GetNext());
    pDrawObjs = pPage->GetDrawObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDrawObjs->size());
    pDrawObj = (*pDrawObjs)[0];
    CPPUNIT_ASSERT_EQUAL(OUString("Rahmen123"), pDrawObj->GetFrameFormat().GetName());
    SwRect aFlyRect = pDrawObj->GetObjRect();
    CPPUNIT_ASSERT(pPage->getFrameArea().IsInside(aFlyRect));
    return aFlyRect;
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testStableAtPageAnchoredFlyPosition)
{
    // this doc has two page-anchored frames: one tiny on page 3 and one large on page 4.
    // it also has a style:master-page named "StandardEntwurf", which contains some fields.
    // if you add a break to page 2, or append some text to page 4 (or just toggle display field names),
    // the page anchored frame on page 4 vanishes, as it is incorrectly moved out of the page bounds.
    SwDoc* pDoc = createDoc("stable-at-page-anchored-fly-position.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // look up the layout position of the page-bound frame on page four
    SwRect aOrigRect = lcl_getVisibleFlyObjRect(pWrtShell);

    // append some text to the document to trigger bug / relayout
    pWrtShell->SttEndDoc(false);
    pWrtShell->Insert("foo");

    // get the current position of the frame on page four
    SwRect aRelayoutRect = lcl_getVisibleFlyObjRect(pWrtShell);

    // the anchored frame should not have moved
    CPPUNIT_ASSERT_EQUAL(aOrigRect, aRelayoutRect);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf134548)
{
    createDoc("tdf134548.odt");

    // Second paragraph has two non zero width tabs in beginning of line
    {
        OUString sNodeType = parseDump("/root/page/body/txt[2]/Text[1]", "nType");
        CPPUNIT_ASSERT_EQUAL(OUString("PortionType::TabLeft"), sNodeType);
        sal_Int32 nWidth = parseDump("/root/page/body/txt[2]/Text[1]", "nWidth").toInt32();
        CPPUNIT_ASSERT_GREATER(sal_Int32(0), nWidth);
    }
    {
        OUString sNodeType = parseDump("/root/page/body/txt[2]/Text[2]", "nType");
        CPPUNIT_ASSERT_EQUAL(OUString("PortionType::TabLeft"), sNodeType);
        sal_Int32 nWidth = parseDump("/root/page/body/txt[2]/Text[2]", "nWidth").toInt32();
        CPPUNIT_ASSERT_GREATER(sal_Int32(0), nWidth);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf154113)
{
    createDoc("three_sections.fodt");
    Scheduler::ProcessEventsToIdle();

    lcl_dispatchCommand(mxComponent, ".uno:GoToStartOfDoc", {});
    lcl_dispatchCommand(mxComponent, ".uno:GoToNextPara", {});
    lcl_dispatchCommand(mxComponent, ".uno:EndOfDocumentSel", {}); // to the end of current section!
    lcl_dispatchCommand(mxComponent, ".uno:EndOfDocumentSel", {}); // to the end of the document.

    css::uno::Reference <frame::XModel> xModel(mxComponent, css::uno::UNO_QUERY_THROW);
    css::uno::Reference <container::XIndexAccess> xSelected(xModel->getCurrentSelection(), css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSelected->getCount());
    css::uno::Reference <text::XTextRange> xRange (xSelected->getByIndex(0), css::uno::UNO_QUERY_THROW);

    //css::uno::Reference < css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("<-- Start selection here. Section1" SAL_NEWLINE_STRING
                                  "Section2" SAL_NEWLINE_STRING "Section3. End selection here -->"),
                         xRange->getString());

    lcl_dispatchCommand(mxComponent, ".uno:Cut", {});

    xSelected = css::uno::Reference <container::XIndexAccess> (xModel->getCurrentSelection(), css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSelected->getCount());
    xRange = css::uno::Reference <text::XTextRange> (xSelected->getByIndex(0), css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString(), xRange->getString());

    lcl_dispatchCommand(mxComponent, ".uno:Paste", {});

    xmlDocPtr pXml = parseLayoutDump();

    // Without the fix in place, this would fail with
    // - Expected: 3
    // - Actual  : 2
    assertXPath(pXml, "/root/page/body/section", 3);
    assertXPath(pXml, "/root/page/body/section[1]/txt/LineBreak", "Line",
                "<-- Start selection here. Section1");
    assertXPath(pXml, "/root/page/body/section[2]/txt/LineBreak", "Line",
                "Section2");
    assertXPath(pXml, "/root/page/body/section[3]/txt/LineBreak", "Line",
                "Section3. End selection here -->");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testLargeTopParaMarginAfterHiddenSection)
{
    // Given a large top margin in Standard paragraph style, and the first section hidden
    createDoc("largeTopMarginAndHiddenFirstSection.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // Make sure there is only one page and two sections, first hidden (zero-height)
    assertXPath(pXmlDoc, "//page", 1);
    assertXPath(pXmlDoc, "//page/body/section", 2);
    assertXPath(pXmlDoc, "//page/body/section[1]/infos/bounds", "height", u"0");
    // Check that the top margin (1 in = 1440 twip) is added to line height (12 pt = 240 twip)
    assertXPath(pXmlDoc, "//page/body/section[2]/infos/bounds", "height", u"1680");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testPageBreakInHiddenSection)
{
    // Given a paragraph with page-break-before with page style and page number
    createDoc("pageBreakInHiddenSection.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page", 4);
    assertXPath(pXmlDoc, "//section", 4);
    assertXPath(pXmlDoc, "//page[1]/body/txt", 1);
    // The page break inside the hidden section is ignored (otherwise, there would be one section
    // on the first page)
    assertXPath(pXmlDoc, "//page[1]/body/section", 2);
    // The first section is hidden
    assertXPath(pXmlDoc, "//page[1]/body/section[1]/infos/bounds", "height", u"0");

    // Page 2 is empty even page (generated by the next page's section with page-break-before)
    assertXPath(pXmlDoc, "//page[2]/body", 0);

    // The section on page 3 is not hidden, only text in it is, therefore its page break works
    assertXPath(pXmlDoc, "//page[3]/body/section", 1);
    assertXPath(pXmlDoc, "//page[3]/body/section/infos/bounds", "height", u"0");

    // The section on page 4 is hidden, thus page break in it is ignored (no further pages, where
    // the section would be moved to otherwise)
    assertXPath(pXmlDoc, "//page[4]/body/section", 1);
    assertXPath(pXmlDoc, "//page[4]/body/section/infos/bounds", "height", u"0");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf160958_page_break)
{
    // Given a document with a section with the first paragraph having a page break
    createDoc("tdf160958_page_break.fodt");
    auto pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 2);
    // A single paragraph on the first page, with 6 lines
    assertXPath(pExportDump, "//page[1]/body/txt", 1);
    assertXPath(pExportDump, "//page[1]/body/txt/LineBreak", 6);
    // A section with 7 paragraphs, and two more paragraphs after the section
    assertXPath(pExportDump, "//page[2]/body/section", 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt", 7);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[4]/LineBreak", 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[5]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[6]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[7]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/txt", 2);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/LineBreak", 7);
    assertXPath(pExportDump, "//page[2]/body/txt[2]/SwParaPortion", 0);

    // Hide the section
    uno::Reference<css::text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY_THROW);
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    uno::Reference<css::beans::XPropertySet> xSection(xSections->getByName(u"Section1"), uno::UNO_QUERY_THROW);
    xSection->setPropertyValue(u"IsVisible", css::uno::Any(false));

    discardDumpedLayout();
    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 1);
    // Three paragraphs and a hidden section on the first page
    assertXPath(pExportDump, "//page/body/txt", 3);
    assertXPath(pExportDump, "//page/body/section", 1);

    assertXPath(pExportDump, "//page/body/section/infos/bounds", "height", "0");
    assertXPath(pExportDump, "//page/body/txt[1]/LineBreak", 6);
    assertXPath(pExportDump, "//page/body/section/txt", 7);

    assertXPath(pExportDump, "//page/body/txt[2]/LineBreak", 7);
    assertXPath(pExportDump, "//page/body/txt[3]/SwParaPortion", 0);

    // Show the section again
    xSection->setPropertyValue(u"IsVisible", css::uno::Any(true));

    // Check that the layout has been restored
    discardDumpedLayout();
    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 2);
    assertXPath(pExportDump, "//page[1]/body/txt", 1);
    assertXPath(pExportDump, "//page[1]/body/txt/LineBreak", 6);
    assertXPath(pExportDump, "//page[2]/body/section", 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt", 7);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[4]/LineBreak", 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[5]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[6]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/section/txt[7]/SwParaPortion", 0);
    assertXPath(pExportDump, "//page[2]/body/txt", 2);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/LineBreak", 7);
    assertXPath(pExportDump, "//page[2]/body/txt[2]/SwParaPortion", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter3, testTdf160958_orphans)
{
    // Given a document with a section which moves to the next page as a whole, because of orphans
    createDoc("tdf160958_orphans_move_section.fodt");
    auto pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 2);
    // 21 paragraphs on the first page
    assertXPath(pExportDump, "//page[1]/body/txt", 21);
    assertXPath(pExportDump, "//page[1]/body/txt[1]/LineBreak", 6);
    assertXPath(pExportDump, "//page[1]/body/txt[2]/LineBreak", 5);
    assertXPath(pExportDump, "//page[1]/body/txt[3]/LineBreak", 7);
    assertXPath(pExportDump, "//page[1]/body/txt[4]/LineBreak", 16);
    assertXPath(pExportDump, "//page[1]/body/txt[5]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[6]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[7]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[8]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[9]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[10]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[11]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[12]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[13]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[14]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[15]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[16]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[17]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[18]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[19]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[20]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[21]/LineBreak", 1);
    // A section and one more paragraph after the section
    assertXPath(pExportDump, "//page[2]/body/section", 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt", 3);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/LineBreak", 6);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/LineBreak", 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/LineBreak", 7);
    assertXPath(pExportDump, "//page[2]/body/txt", 1);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/LineBreak", 1);

    // Hide the section
    uno::Reference<css::text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY_THROW);
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    uno::Reference<css::beans::XPropertySet> xSection(xSections->getByName(u"Section1"), uno::UNO_QUERY_THROW);
    xSection->setPropertyValue(u"IsVisible", css::uno::Any(false));

    discardDumpedLayout();
    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 1);
    assertXPath(pExportDump, "//page/body/txt", 22);
    assertXPath(pExportDump, "//page/body/section", 1);
    assertXPath(pExportDump, "//page/body/section/infos/bounds", "height", "0");

    // Show the section again
    xSection->setPropertyValue(u"IsVisible", css::uno::Any(true));

    // Check that the layout has been restored
    discardDumpedLayout();
    calcLayout();
    pExportDump = parseLayoutDump();
    assertXPath(pExportDump, "//page", 2);
    assertXPath(pExportDump, "//page[1]/body/txt", 21);
    assertXPath(pExportDump, "//page[1]/body/txt[1]/LineBreak", 6);
    assertXPath(pExportDump, "//page[1]/body/txt[2]/LineBreak", 5);
    assertXPath(pExportDump, "//page[1]/body/txt[3]/LineBreak", 7);
    assertXPath(pExportDump, "//page[1]/body/txt[4]/LineBreak", 16);
    assertXPath(pExportDump, "//page[1]/body/txt[5]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[6]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[7]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[8]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[9]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[10]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[11]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[12]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[13]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[14]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[15]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[16]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[17]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[18]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[19]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[20]/LineBreak", 1);
    assertXPath(pExportDump, "//page[1]/body/txt[21]/LineBreak", 1);
    assertXPath(pExportDump, "//page[2]/body/section", 1);
    assertXPath(pExportDump, "//page[2]/body/section/txt", 3);
    assertXPath(pExportDump, "//page[2]/body/section/txt[1]/LineBreak", 6);
    assertXPath(pExportDump, "//page[2]/body/section/txt[2]/LineBreak", 5);
    assertXPath(pExportDump, "//page[2]/body/section/txt[3]/LineBreak", 7);
    assertXPath(pExportDump, "//page[2]/body/txt", 1);
    assertXPath(pExportDump, "//page[2]/body/txt[1]/LineBreak", 1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
