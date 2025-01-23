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
#include <vcl/scheduler.hxx>
#include <svx/svddef.hxx>

#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtcntnt.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <rootfrm.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <unoframe.hxx>
#include <fldmgr.hxx>

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter : public SwModelTestBase
{
public:
    SwLayoutWriter()
        : SwModelTestBase(u"/sw/qa/extras/layout/data/"_ustr)
    {
    }

protected:
    void CheckRedlineFootnotesHidden();
    void CheckRedlineSectionsHidden();
};

// this is a member because our test classes have protected members :(
void SwLayoutWriter::CheckRedlineFootnotesHidden()
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"24");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"1");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "type", u"PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "expand", u"2");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex", u"13");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"1");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"ac");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/merged", "paraPropsNodeIndex", u"16");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"2");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"mo");
}

void SwLayoutWriter::CheckRedlineSectionsHidden()
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"folah");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/merged", "paraPropsNodeIndex",
                u"20");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"folah");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf156077)
{
    createSwDoc("s4_min2.fodt");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of pages does not match!", 3, getPages());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly", 3);
    assertXPath(pXmlDoc, "/root/page[2]/anchored/fly", 1);
    // this was 0, the at-page anchored flys were not displayed
    assertXPath(pXmlDoc, "/root/page[3]/anchored/fly", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFootnotes)
{
    createSwDoc("redline_footnotes.odt");
    SwDoc* pDoc(getSwDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // verify after load
    CheckRedlineFootnotesHidden();

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // show: nothing is merged
    assertXPath(pXmlDoc, "//merged", 0);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"1");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "type", u"PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "expand", u"2");

    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"1");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"a");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"b");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"c");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"2");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"def");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"3");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"ar");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[3]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[3]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"3");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[3]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[3]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"ghi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"4");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "type", u"PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "expand", u"5");

    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[4]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[4]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"4");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[4]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[4]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"jkl");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", u"PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", u"5");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"m");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"n");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"o");

    // verify after hide
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    CheckRedlineFootnotesHidden();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf136588)
{
    createSwDoc("tdf136588.docx");
    auto pXMLLayoutDump = parseLayoutDump();
    CPPUNIT_ASSERT(pXMLLayoutDump);

    //there was a bad line break before, the correct break layout is this:
    assertXPath(pXMLLayoutDump, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[2]", "portion",
                u"effectively by modern-day small to ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTableSplitBug)
{
    createSwDoc("table-split-bug.fodt");

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]//txt[1]/infos/bounds",
                    "height", u"276");
        assertXPath(pXmlDoc, "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]/infos/bounds",
                    "height", u"1274");
    }

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(false, 1);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Delete();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // the problem was that the paragraph in the left cell had height 0
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]//txt[1]/infos/bounds",
                    "height", u"276");
        assertXPath(pXmlDoc, "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]/infos/bounds",
                    "height", u"1688");
    }

    pWrtShell->Undo();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // the problem was that the paragraph in the left cell had height 0
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]//txt[1]/infos/bounds",
                    "height", u"276");
        assertXPath(pXmlDoc, "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]/infos/bounds",
                    "height", u"1274");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysInBody)
{
    createSwDoc();
    SwDoc* pDoc(getSwDoc());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->Insert(u"foo"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"bar"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"baz"_ustr);
    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    pWrtShell->StartOfSection(false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    flySet.Put(anchor);
    SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);
    // move inside fly
    pWrtShell->GotoFly(pFly->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Insert(u"abc"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"def"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"ghi"_ustr);

    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    // delete redline inside fly
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->SttEndDoc(true); // note: SttDoc actually moves to start of fly?
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"14");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"foaz");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", u"6");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"ahi");

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"az");
    }

    // anchor to 2nd (deleted) paragraph
    pWrtShell->StartOfSection();
    pWrtShell->Down(false, 1);
    anchor.SetType(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"14");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"foaz");

        // hide: no anchored object shown
        assertXPath(pXmlDoc, "//anchored", 0);

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"az");
    }

    // anchor to 3rd paragraph
    pWrtShell->EndOfSection();
    anchor.SetType(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"14");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"foaz");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", u"6");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"ahi");

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"az");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"hi");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testFlyHiddenParagraph)
{
    createSwDoc("fly_hidden_paragraph.fodt");

    // first, disable both so para gets hidden
    uno::Sequence<beans::PropertyValue> argsSH(
        comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", uno::Any(false) } }));
    dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", argsSH);

    uno::Sequence<beans::PropertyValue> args(
        comphelper::InitPropertySequence({ { "Fieldnames", uno::Any(false) } }));

    dispatchCommand(mxComponent, ".uno:Fieldnames", args);
    Scheduler::ProcessEventsToIdle();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/txt/infos/bounds", "height", u"0");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "height", u"448");

    // the problem was that now the fly was the same height as before hiding
    dispatchCommand(mxComponent, ".uno:Fieldnames", {});
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/txt/infos/bounds", "height",
                u"828");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "height", u"1000");

    dispatchCommand(mxComponent, ".uno:Fieldnames", {});
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/txt/infos/bounds", "height", u"0");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "height", u"448");

    dispatchCommand(mxComponent, ".uno:Fieldnames", {});
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/txt/infos/bounds", "height",
                u"828");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "height", u"1000");

    // other test like testTdf143239 and testTdf159101 depend on this;
    // seems getting the previous value is only possible with a listener
    // so just hardcode it...
    dispatchCommand(mxComponent, ".uno:Fieldnames", args);
    Scheduler::ProcessEventsToIdle();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testFieldHideSection)
{
    createSwDoc("field_hide_section.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/row", 1);
    assertXPath(pXmlDoc, "/root/page", 2);

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    ::std::unique_ptr<SwField> pField(pWrtShell->GetCurField()->CopyField());
    SwFieldMgr manager(pWrtShell);

    pWrtShell->StartAllAction();
    manager.UpdateCurField(10000 /*(?)*/, "Foo", "1", std::move(pField));
    pWrtShell->EndAllAction();
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row[1]/infos/bounds", "height", u"0");
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row[2]/infos/bounds", "height", u"0");
    assertXPath(pXmlDoc, "/root/page[1]/body/section/infos/bounds", "height", u"0");
    // the problem was that there were 3 pages now
    assertXPath(pXmlDoc, "/root/page", 1);

    pWrtShell->StartAllAction();
    manager.UpdateCurField(10000 /*(?)*/, "Foo", "0", std::move(pField));
    pWrtShell->EndAllAction();
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/row", 1);
    assertXPath(pXmlDoc, "/root/page", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestI94666)
{
    createSwDoc("i94666.odt");

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page", 2);
        assertXPath(
            pXmlDoc,
            "/root/page[2]/body/section/txt[1]/SwParaPortion/SwLineLayout[1]/SwLinePortion[1]",
            "portion", u"pulled off ");
    }

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoPage(2, false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 11, /*bBasicCall=*/false);
    pWrtShell->SetMark();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 11, /*bBasicCall=*/false);
    pWrtShell->DelToEndOfPara();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // the problem was that the last paragraph moved to page 3
        assertXPath(
            pXmlDoc,
            "/root/page[2]/body/section/txt[1]/SwParaPortion/SwLineLayout[1]/SwLinePortion[1]",
            "portion",
            u"Widows & orphans He heard quiet steps behind him. That didn't bode well. Who could "
            u"be "
            "following");
        assertXPath(
            pXmlDoc,
            "/root/page[2]/body/section/txt[1]/SwParaPortion/SwLineLayout[3]/SwLinePortion[1]",
            "portion", u"pulled off ");
        assertXPath(
            pXmlDoc,
            "/root/page[2]/body/section/txt[2]/SwParaPortion/SwLineLayout[1]/SwParaPortion[1]",
            "portion", u"Moved paragraph");
        assertXPath(pXmlDoc, "/root/page[2]//txt", 3);
        assertXPath(pXmlDoc, "/root/page", 2);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf134272)
{
    createSwDoc("tdf134472.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/infos/bounds", "height", u"843");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/infos/bounds", "bottom", u"2819");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestNestedTableMoveFwd)
{
    createSwDoc("tabellen_test_windows_1.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // the row with the nested table should not be split but be the first row on page 2
    assertXPathContent(pXmlDoc, "/root/page[1]/body/tab[1]/row[last()]/cell[1]/txt[1]",
                       u"Tabelle 1");
    assertXPathContent(pXmlDoc,
                       "/root/page[2]/body/tab[1]/row[1]/cell[1]/tab[1]/row[1]/cell[1]/txt",
                       u"Tabelle 2");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf136613)
{
    createSwDoc("tdf136613.docx");
    SwDoc* pDoc = getSwDoc();

    //get the flys and the root frame
    const auto vFlyFormats = pDoc->GetFlyFrameFormats(FLYCNTTYPE_ALL, true);
    const auto vFrames = pDoc->GetAllLayouts();

    CPPUNIT_ASSERT(!vFrames.empty());
    CPPUNIT_ASSERT(!vFlyFormats.empty());

    //get the page frame from the root
    SwFrame* pPageFrame = vFrames[0]->Lower();
    CPPUNIT_ASSERT(pPageFrame);

    //get the rectangle of the page
    const SwRect& rPageRect = pPageFrame->getFrameArea();

    //check the flys and...
    for (auto pFlyFormat : vFlyFormats)
    {
        //...the rectangle of the fly location...
        const SwRect aRect = pFlyFormat->FindLayoutRect();
        CPPUNIT_ASSERT(!aRect.IsEmpty());

        //...if it is on the page. This will fail if not.
        CPPUNIT_ASSERT_MESSAGE("The pictures are outside the page!", rPageRect.Contains(aRect));
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf88496)
{
    createSwDoc("tdf88496.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 4, table fallback "switch off repeating header" started on a new page
    assertXPath(pXmlDoc, "/root/page", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysInHeader)
{
    createSwDoc();
    SwDoc* pDoc(getSwDoc());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->ChangeHeaderOrFooter(u"Default Page Style", /*bHeader*/ true, /*bOn*/ true, false);
    CPPUNIT_ASSERT(
        pWrtShell
            ->IsInHeaderFooter()); // assume this is supposed to put cursor in the new header...
    pWrtShell->Insert(u"foo"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"bar"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"baz"_ustr);
    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    pWrtShell->StartOfSection(false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    flySet.Put(anchor);
    SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);
    // move inside fly
    pWrtShell->GotoFly(pFly->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Insert(u"abc"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"def"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"ghi"_ustr);

    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    // delete redline inside fly
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->GotoHeaderText();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "length",
                    u"0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", u"6");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                    u"foaz");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", u"11");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout[1]",
            "type", u"PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout[1]",
            "portion", u"ahi");

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "length",
                    u"0");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"az");
    }

    // anchor to 2nd (deleted) paragraph
    pWrtShell->StartOfSection();
    pWrtShell->Down(false, 1);
    anchor.SetType(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // now the frame has no Text portion? not sure why it's a 0-length one first and now none?
        //        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type", "PortionType::Para");
        //        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "length", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", u"6");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"foaz");

        // hide: no anchored object shown
        assertXPath(pXmlDoc, "//anchored", 0);

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "length", u"0");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"az");
    }

    // anchor to 3rd paragraph
    pWrtShell->EndOfSection();
    anchor.SetType(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "length", u"0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", u"6");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"foaz");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", u"11");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"ahi");

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "length", u"0");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"az");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"hi");
    }
}

#if !defined(MACOSX)
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf150606)
{
    createSwDoc("tdf150606-1-min.odt");

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/section/column[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/column[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/column[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/column[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[3]/body/section/column[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/section/column[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[4]/body/section/column[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[4]/body/section/column[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[4]/body/section/column", 2);
    // on page 5 the table is split across balanced columns
    assertXPath(pXmlDoc, "/root/page[5]/body/section/column[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[5]/body/section/column[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[5]/body/section/column", 2);
    assertXPath(pXmlDoc, "/root/page", 5);

    pWrtShell->Down(false, 1);
    dispatchCommand(mxComponent, u".uno:DeleteTable"_ustr, {});

    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/section/column/body/tab", 0);
    assertXPath(pXmlDoc, "/root/page", 1);

    pWrtShell->Undo();
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/section/column[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/column[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/column[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/column[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[3]/body/section/column[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/section/column[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[4]/body/section/column[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[4]/body/section/column[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[4]/body/section/column", 2);
    // on page 5 the table is split across balanced columns
    // (problem was that there were empty pages and table was on page 10)
    assertXPath(pXmlDoc, "/root/page[5]/body/section/column[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[5]/body/section/column[2]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[5]/body/section/column", 2);
    assertXPath(pXmlDoc, "/root/page", 5);
}
#endif

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf152983)
{
    //just care it doesn't crash/assert
    createSwDoc("tdf152983-1-min.docx");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf137025)
{
    // Check the padding of the textbox
    createSwDoc("tdf137025.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the layout xml
    // SDRATTR_TEXT_LEFTDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item"
                "[@whichId='"
                    + OString::number(SDRATTR_TEXT_LEFTDIST) + "']",
                "value", u"567");
    // SDRATTR_TEXT_RIGHTDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item"
                "[@whichId='"
                    + OString::number(SDRATTR_TEXT_RIGHTDIST) + "']",
                "value", u"1134");
    // SDRATTR_TEXT_UPPERDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item"
                "[@whichId='"
                    + OString::number(SDRATTR_TEXT_UPPERDIST) + "']",
                "value", u"1701");
    // SDRATTR_TEXT_LOWERDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item"
                "[@whichId='"
                    + OString::number(SDRATTR_TEXT_LOWERDIST) + "']",
                "value", u"2268");

    // Check the textbox-shape import too
    auto xShp = getShape(1);
    CPPUNIT_ASSERT(xShp);

    uno::Reference<beans::XPropertySet> xShapeProps(xShp, uno::UNO_QUERY);

    SwFrameFormat* pFrameFormat = SwTextBoxHelper::getOtherTextBoxFormat(xShp);
    CPPUNIT_ASSERT(pFrameFormat);

    // The shape has these values to copy to the associated text frame after modification::
    const tools::Long nLPaddng
        = xShapeProps->getPropertyValue(u"TextLeftDistance"_ustr).get<tools::Long>();
    const tools::Long nRPaddng
        = xShapeProps->getPropertyValue(u"TextRightDistance"_ustr).get<tools::Long>();
    const tools::Long nTPaddng
        = xShapeProps->getPropertyValue(u"TextUpperDistance"_ustr).get<tools::Long>();
    const tools::Long nBPaddng
        = xShapeProps->getPropertyValue(u"TextLowerDistance"_ustr).get<tools::Long>();

    CPPUNIT_ASSERT_EQUAL(tools::Long(1000), nLPaddng);
    CPPUNIT_ASSERT_EQUAL(tools::Long(2000), nRPaddng);
    CPPUNIT_ASSERT_EQUAL(tools::Long(3000), nTPaddng);
    CPPUNIT_ASSERT_EQUAL(tools::Long(4001), nBPaddng);

    // TODO: modify shape distance via UNO with text frame synchronization
    // Check the textbox as well:
    auto xTxFrm = SwXTextFrame::CreateXTextFrame(*pFrameFormat->GetDoc(), pFrameFormat);
    CPPUNIT_ASSERT(xTxFrm);

    const tools::Long nFrameLeftPaddng
        = xTxFrm->getPropertyValue(u"LeftBorderDistance"_ustr).get<tools::Long>();
    const tools::Long nFrameRightPaddng
        = xTxFrm->getPropertyValue(u"RightBorderDistance"_ustr).get<tools::Long>();
    const tools::Long nFrameTopPaddng
        = xTxFrm->getPropertyValue(u"TopBorderDistance"_ustr).get<tools::Long>();
    const tools::Long nFrameBottomPaddng
        = xTxFrm->getPropertyValue(u"BottomBorderDistance"_ustr).get<tools::Long>();

    // Check if the shape and frame have different setting
    CPPUNIT_ASSERT_EQUAL(nLPaddng, nFrameLeftPaddng);
    CPPUNIT_ASSERT_EQUAL(nRPaddng, nFrameRightPaddng);
    CPPUNIT_ASSERT_EQUAL(nTPaddng, nFrameTopPaddng);
    CPPUNIT_ASSERT_EQUAL(nBPaddng, nFrameBottomPaddng);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysInFootnote)
{
    createSwDoc();
    SwDoc* pDoc(getSwDoc());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->InsertFootnote(u""_ustr);
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());

    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
    SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    flySet.Put(anchor);
    // first fly is in first footnote that will be deleted
    /*  SwFrameFormat const* pFly1 =*/pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    pWrtShell->Insert(u"quux"_ustr);

    pWrtShell->SttEndDoc(false);

    pWrtShell->InsertFootnote(u""_ustr);
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());
    pWrtShell->Insert(u"foo"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"bar"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"baz"_ustr);

    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    flySet.Put(anchor);
    // second fly is in second footnote that is not deleted
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);
    // move inside fly
    pWrtShell->GotoFly(pFly->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Insert(u"abc"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"def"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"ghi"_ustr);

    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    // delete redline inside fly
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    //    pWrtShell->GotoFlyAnchor(); // sigh... why, now we're in the body...
    pWrtShell->SttEndDoc(false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->GotoFootnoteText();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();
    pWrtShell->EndSelect(); // ?
    // delete first footnote
    pWrtShell->SttEndDoc(true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Delete();

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"25");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote']",
                    "type", u"PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote']",
                    "expand", u"1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    u"7");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", u"17");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", u"ahi");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", u"PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", u"1");

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "type", u"PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "expand", u"1");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                    "type", u"PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                    "expand", u"2");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", u"quux");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", u"PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", u"1");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", u"a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", u"bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", u"hi");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", u"PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", u"2");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", u"fo");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", u"o");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "type", u"PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "portion", u"bar");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", u"b");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", u"az");
    }

    // anchor to 2nd (deleted) paragraph
    pWrtShell->SttEndDoc(false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->GotoFootnoteText();
    pWrtShell->Down(false, 1);
    anchor.SetType(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"25");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "type", u"PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "expand", u"1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    u"7");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", u"PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", u"1");

        // hide: no anchored object shown
        assertXPath(pXmlDoc, "//anchored", 0);

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "type", u"PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "expand", u"1");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                    "type", u"PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                    "expand", u"2");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", u"quux");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", u"PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", u"1");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", u"PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", u"2");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", u"fo");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", u"o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", u"a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", u"bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", u"hi");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "type", u"PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "portion", u"bar");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", u"b");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", u"az");
    }

    // anchor to 3rd paragraph
    pWrtShell->EndOfSection();
    pWrtShell->SttEndDoc(false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->GotoFootnoteText();
    pWrtShell->EndOfSection();
    anchor.SetType(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"25");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "type", u"PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "expand", u"1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    u"7");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", u"17");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", u"ahi");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", u"PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", u"1");

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote'][1]",
                    "type", u"PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote'][1]",
                    "expand", u"1");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote'][2]",
                    "type", u"PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote'][2]",
                    "expand", u"2");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", u"quux");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", u"PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", u"1");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", u"PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", u"2");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", u"fo");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", u"o");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "type", u"PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "portion", u"bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", u"a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", u"bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", u"hi");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", u"b");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", u"PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", u"az");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf143239)
{
    createSwDoc("tdf143239-1-min.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // These are unstable on macOS and Win64 builds,
    // so only test that they restore original values for now
    OUString p2txt1Left, p2txt2Left, p3txt1Left;

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "top",
                    u"18540");
        p2txt1Left
            = getXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "left");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly[1]/infos/bounds", "top",
                    u"23894");
        p2txt2Left
            = getXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly[1]/infos/bounds", "left");
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly[1]/infos/bounds", "top",
                    u"35662");
        p3txt1Left
            = getXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly[1]/infos/bounds", "left");
        assertXPath(pXmlDoc, "/root/page", 3);
    }

    pWrtShell->SelAll();
    pWrtShell->Delete();
    pWrtShell->Undo();
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // now the 1st fly was on page 1, and the fly on page 2 was the 2nd one
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "top",
                    u"18540");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "left",
                    p2txt1Left);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly[1]/infos/bounds", "top",
                    u"23894");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly[1]/infos/bounds", "left",
                    p2txt2Left);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly[1]/infos/bounds", "top",
                    u"35662");
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly[1]/infos/bounds", "left",
                    p3txt1Left);
        assertXPath(pXmlDoc, "/root/page", 3);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTableOverlapFooterFly)
{
    // Load a document that has a fly anchored in the footer.
    // It also has a table which initially overlaps with the fly, but then moves to the next page.
    createSwDoc("footer-fly-table.fodt");
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // no fly portions, was: 8
    assertXPath(pLayout,
                "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwFixPortion[@type='PortionType::Fly'][@type='PortionType::Fly']",
                0);
    // one line break, was: 5
    assertXPath(pLayout,
                "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/SwParaPortion/SwLineLayout", 1);
    // one text portion, was: 1
    assertXPath(
        pLayout,
        "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
        1);
    assertXPath(
        pLayout,
        "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
        "portion", u"Abc def ghi jkl mno pqr stu vwx yz.");
    // height was: 1517
    // tdf#134782 height was: 379
    assertXPath(pLayout, "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/infos/bounds", "height",
                u"253");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf134277)
{
    createSwDoc("tdf134277.docx");
    SwDocShell* pShell = getSwDocShell();

    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/metafile/push/push/push/layoutmode[2]", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf116486)
{
    createSwDoc("tdf116486.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    OUString aTop = getXPath(
        pXmlDoc,
        "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/child::*[@type='PortionType::Fly']",
        "height");
    CPPUNIT_ASSERT_EQUAL(u"4006"_ustr, aTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf142080)
{
    // this caused an infinite loop
    createSwDoc("fdo43573-2-min.docx");

    xmlDocUniquePtr pLayout = parseLayoutDump();
    // check the first paragraph on page 9 with its fly; the column was empty too
    assertXPath(pLayout,
                "/root/page[9]/body/section[1]/column[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/"
                "SwLinePortion",
                "portion",
                u"De kleur u (rood) in het rechtervlak (R), de kleur r (wit) beneden (D),");
    SwTwips nPage9Top = getXPath(pLayout, "/root/page[9]/infos/bounds", "top").toInt32();
    assertXPath(
        pLayout,
        "/root/page[9]/body/section[1]/column[1]/body/txt[1]/anchored/fly[1]/notxt/infos/bounds",
        "top", OUString::number(nPage9Top + 1460));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf152106)
{
    // this caused an infinite loop
    createSwDoc("tdf152106.odt");

    xmlDocUniquePtr pLayout = parseLayoutDump();

    // frame on page 3
    assertXPath(pLayout, "/root/page[3]/sorted_objs/fly", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf128198)
{
    createSwDoc("tdf128198-1.docx");
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // the problem was that line 5 was truncated at "this  "
    // due to the fly anchored in previous paragraph
    assertXPath(pLayout, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[5]", "portion",
                u"to access any service, any time, anywhere. From this  perspective, satellite "
                "boasts some ");
    assertXPath(pLayout, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[6]", "portion",
                u"significant advantages. ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testNoLineBreakAtSlash)
{
    createSwDoc("no-line-break-at-slash.fodt");
    xmlDocUniquePtr pLayout = parseLayoutDump();

    // the line break was between  "Foostrasse 13/c/" and "2"
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout", 2);
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/child::*", "type",
                u"PortionType::Para");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/child::*[1]",
                "type", u"PortionType::Text");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/child::*[2]",
                "type", u"PortionType::Blank");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/child::*[3]",
                "type", u"PortionType::Text");

    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"Blah blah bla bla bla ");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/SwLinePortion[1]",
                "portion", u"Foostrasse");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/SwLinePortion[2]",
                "portion", u"13/c/2, etc.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf106153)
{
    createSwDoc("tdf106153.docx");
    xmlDocUniquePtr pDump = parseLayoutDump();

    const sal_Int64 nPageValLeft = getXPath(pDump, "/root/page/infos/bounds", "left").toInt64();
    const sal_Int64 nPageValTop = getXPath(pDump, "/root/page/infos/bounds", "top").toInt64();
    const sal_Int64 nPageValRight = getXPath(pDump, "/root/page/infos/bounds", "right").toInt64();
    const sal_Int64 nPageValBottom = getXPath(pDump, "/root/page/infos/bounds", "bottom").toInt64();

    const sal_Int64 nShape1ValTop
        = getXPath(pDump, "/root/page/body/txt/anchored/fly[1]/infos/bounds", "top").toInt64();
    const sal_Int64 nShape2ValLeft
        = getXPath(pDump, "/root/page/body/txt/anchored/fly[2]/infos/bounds", "left").toInt64();
    const sal_Int64 nShape3ValRight
        = getXPath(pDump, "/root/page/body/txt/anchored/fly[3]/infos/bounds", "right").toInt64();
    const sal_Int64 nShape4ValBottom
        = getXPath(pDump, "/root/page/body/txt/anchored/fly[4]/infos/bounds", "bottom").toInt64();

    CPPUNIT_ASSERT_MESSAGE("The whole top textbox is inside the page!",
                           nPageValTop > nShape1ValTop);
    CPPUNIT_ASSERT_MESSAGE("The whole left textbox is inside the page!",
                           nPageValLeft > nShape2ValLeft);
    CPPUNIT_ASSERT_MESSAGE("The whole right textbox is inside the page!",
                           nPageValRight < nShape3ValRight);
    CPPUNIT_ASSERT_MESSAGE("The whole bottom textbox is inside the page!",
                           nPageValBottom < nShape4ValBottom);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysInFlys)
{
    createSwDoc();
    SwDoc* pDoc(getSwDoc());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->Insert(u"foo"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"bar"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"baz"_ustr);
    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
    SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    pWrtShell->StartOfSection(false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    SwFormatAnchor anchor1(RndStdIds::FLY_AT_CHAR);
    anchor1.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    flySet.Put(anchor1);
    SwFrameFormat const* pFly1 = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly1 != nullptr);
    // move inside fly1
    pWrtShell->GotoFly(pFly1->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Insert(u"abc"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"def"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"ghi"_ustr);

    SwFormatAnchor anchor2(RndStdIds::FLY_AT_CHAR);
    pWrtShell->StartOfSection(false); // start of fly...
    anchor2.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    flySet.Put(anchor2);
    SwFrameFormat const* pFly2 = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly2 != nullptr);
    // move inside fly2
    pWrtShell->GotoFly(pFly2->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Insert(u"jkl"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"mno"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"pqr"_ustr);

    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    // delete redline inside fly2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // delete redline inside fly1
    pWrtShell->GotoFly(pFly1->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->ClearMark(); // otherwise it refuses to leave the fly...
    pWrtShell->SttEndDoc(true); // note: SttDoc actually moves to start of fly?
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor1.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor1.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor1.SetAnchor(&pos);
            pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));
            anchor2.SetType(RndStdIds::FLY_AT_PARA);
            pos.nNode = *anchor2.GetAnchorNode();
            anchor2.SetAnchor(&pos);
            pDoc->SetAttr(anchor2, *const_cast<SwFrameFormat*>(pFly2));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"19");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", u"6");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/merged",
            "paraPropsNodeIndex", u"11");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"jqr");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwFixPortion[1]",
                    "type",
                    u"PortionType::Fly"); // remove???
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "type", u"PortionType::Lay");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "portion", u"abhi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"foaz");

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"j");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"kl");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"mno");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"p");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"qr");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwFixPortion[1]",
                    "type",
                    u"PortionType::Fly"); // remove???
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"ab");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"c");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"az");
    }

    // anchor to 2nd (deleted) paragraph
    // also, switch the in-fly anchoring to the other fly, for additional fun!
    pWrtShell->StartOfSection();
    pWrtShell->Down(false, 1);
    anchor2.SetType(RndStdIds::FLY_AT_CHAR);
    anchor2.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor2, *const_cast<SwFrameFormat*>(pFly2));
    pWrtShell->GotoFly(pFly2->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Down(false, 1);
    anchor1.SetType(RndStdIds::FLY_AT_CHAR);
    anchor1.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor1.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor1.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor1.SetAnchor(&pos);
            pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));
            anchor2.SetType(RndStdIds::FLY_AT_PARA);
            pos.nNode = *anchor2.GetAnchorNode();
            anchor2.SetAnchor(&pos);
            pDoc->SetAttr(anchor2, *const_cast<SwFrameFormat*>(pFly2));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"19");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"foaz");

        // hide: no anchored object shown
        assertXPath(pXmlDoc, "//anchored", 0);

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"j");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"kl");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"ab");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"c");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwFixPortion[1]",
                    "type",
                    u"PortionType::Fly"); // remove???
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "type", u"PortionType::Lay");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "portion", u"mno");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"p");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"qr");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"az");
    }

    // anchor to 3rd paragraph
    pWrtShell->SttEndDoc(false);
    anchor1.SetType(RndStdIds::FLY_AT_CHAR);
    anchor1.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));
    pWrtShell->GotoFly(pFly1->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->EndOfSection();
    anchor2.SetType(RndStdIds::FLY_AT_CHAR);
    anchor2.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor2, *const_cast<SwFrameFormat*>(pFly2));

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor1.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor1.GetAnchorNode());
            pos.nContent.Assign(nullptr, 0);
            anchor1.SetAnchor(&pos);
            pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));
            anchor2.SetType(RndStdIds::FLY_AT_PARA);
            pos.nNode = *anchor2.GetAnchorNode();
            anchor2.SetAnchor(&pos);
            pDoc->SetAttr(anchor2, *const_cast<SwFrameFormat*>(pFly2));
        }

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"19");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", u"6");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/merged",
            "paraPropsNodeIndex", u"11");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"jqr");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwFixPortion[1]",
                    "type",
                    u"PortionType::Fly"); // remove???
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "type", u"PortionType::Lay");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "portion", u"abhi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"foaz");

        dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        pXmlDoc = parseLayoutDump();

        // show: nothing is merged
        assertXPath(pXmlDoc, "//merged", 0);

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"ab");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"c");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", u"def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"j");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"kl");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", u"PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", u"mno");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"p");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"qr");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwFixPortion[1]",
                    "type",
                    u"PortionType::Fly"); // remove???
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", u"g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", u"hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", u"b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    u"PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", u"az");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysAtFlys)
{
    createSwDoc();
    SwDoc* pDoc(getSwDoc());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->Insert(u"foo"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"bar"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"baz"_ustr);
    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>);
    SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    pWrtShell->StartOfSection(false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    SwFormatAnchor anchor1(RndStdIds::FLY_AT_CHAR);
    anchor1.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    flySet.Put(anchor1);
    SwFrameFormat const* pFly1 = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly1 != nullptr);
    // move inside fly1
    pWrtShell->GotoFly(pFly1->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Insert(u"abc"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"def"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"ghi"_ustr);

    SwFormatAnchor anchor2(RndStdIds::FLY_AT_FLY);
    SwPosition pos(*pFly1->GetContent().GetContentIdx());
    anchor2.SetAnchor(&pos);
    flySet.Put(anchor2);
    SwFrameFormat const* pFly2 = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly2 != nullptr);
    // move inside fly2
    pWrtShell->GotoFly(pFly2->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Insert(u"jkl"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"mno"_ustr);
    pWrtShell->SplitNode(false);
    pWrtShell->Insert(u"pqr"_ustr);

    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    // delete redline inside fly2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // delete redline inside fly1
    pWrtShell->GotoFly(pFly1->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->ClearMark(); // otherwise it refuses to leave the fly...
    pWrtShell->SttEndDoc(true); // note: SttDoc actually moves to start of fly?
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"19");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                "paraPropsNodeIndex", u"6");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/merged",
                "paraPropsNodeIndex", u"11");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "portion", u"jqr");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwFixPortion[1]",
                "type",
                u"PortionType::Fly"); // remove???
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLineLayout[1]",
                "type", u"PortionType::Lay");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLineLayout[1]",
                "portion", u"abhi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"foaz");

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pXmlDoc = parseLayoutDump();

    // show: nothing is merged
    assertXPath(pXmlDoc, "//merged", 0);

    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "portion", u"j");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "portion", u"kl");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "portion", u"mno");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "portion", u"p");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "portion", u"qr");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwFixPortion[1]",
                "type",
                u"PortionType::Fly"); // remove???
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "portion", u"ab");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "portion", u"c");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "portion", u"def");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "portion", u"g");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "portion", u"hi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"az");

    // anchor to 2nd (deleted) paragraph
    pWrtShell->StartOfSection();
    pWrtShell->Down(false, 1);
    anchor1.SetType(RndStdIds::FLY_AT_CHAR);
    anchor1.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"19");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"foaz");

    // hide: no anchored object shown
    assertXPath(pXmlDoc, "//anchored", 0);

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pXmlDoc = parseLayoutDump();

    // show: nothing is merged
    assertXPath(pXmlDoc, "//merged", 0);

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"o");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "portion", u"j");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "portion", u"kl");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "portion", u"mno");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "portion", u"p");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "portion", u"qr");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwFixPortion[1]",
                "type",
                u"PortionType::Fly"); // remove???
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "portion", u"ab");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "portion", u"c");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "portion", u"def");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "portion", u"g");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "portion", u"hi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"az");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineSections)
{
    createSwDoc("redline_sections.fodt");
    SwDoc* pDoc(getSwDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // verify after load
    CheckRedlineSectionsHidden();

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    // why is this needed explicitly?
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // show: nothing is merged
    assertXPath(pXmlDoc, "//merged", 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[1]", "type",
                u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", u"fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[2]", "type",
                u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[2]",
                "portion", u"o");

    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/child::*[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/child::*[1]",
                "portion", u"FRAME");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout/child::*[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", u"bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[2]/SwParaPortion/SwLineLayout/child::*[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[2]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/child::*[1]", "type",
                u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", u"b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/child::*[2]", "type",
                u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/child::*[2]",
                "portion", u"lah");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/child::*[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", u"fo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/child::*[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/child::*[2]",
                "portion", u"o");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[2]/SwParaPortion/SwLineLayout/child::*[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[2]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", u"bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/child::*[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", u"b");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/child::*[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/child::*[2]",
                "portion", u"lah");

    // verify after hide
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    calcLayout();
    CheckRedlineSectionsHidden();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TDF69647_images)
{
    createSwDoc("tdf69647_images.odt");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of pages does not match!", 2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TDF69647_text)
{
    createSwDoc("tdf69647_text.docx");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of pages does not match!", 2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineTables)
{
    createSwDoc("redline_table.fodt");
    SwDoc* pDoc(getSwDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // verify after load
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"foar");

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    // why is this needed explicitly?
    calcLayout();
    pXmlDoc = parseLayoutDump();

    // show: nothing is merged
    assertXPath(pXmlDoc, "//merged", 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"o");

    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/anchored/fly[1]/txt[1]/"
                "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/anchored/fly[1]/txt[1]/"
                "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"FRAME");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "portion", u"aaa");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "portion", u"ddd");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"ar");

    // verify after hide
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    calcLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"foar");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf144057)
{
    createSwDoc("tdf144057.fodt");
    SwDoc* pDoc(getSwDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // show tracked row deletions
    assertXPath(pXmlDoc, "/root/page", 4);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"A6");
    assertXPath(pXmlDoc,
                "/root/page[2]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"A12");
    assertXPath(pXmlDoc,
                "/root/page[3]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"B6");
    assertXPath(pXmlDoc,
                "/root/page[4]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"B12");

    // hide tracked table and table row deletions
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    calcLayout();
    pXmlDoc = parseLayoutDump();

    // This was 4 (unhidden tracked table and table row deletions)
    assertXPath(pXmlDoc, "/root/page", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 5);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[5]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"B12");

    // show tracked table and table row deletions again
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    calcLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 4);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"A6");
    assertXPath(pXmlDoc,
                "/root/page[2]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"A12");
    assertXPath(pXmlDoc,
                "/root/page[3]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"B6");
    assertXPath(pXmlDoc,
                "/root/page[4]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"B12");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf144347)
{
    createSwDoc("tdf144057.fodt");
    SwDoc* pDoc(getSwDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());

    // enable redlining
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    // remove first table
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    for (int i = 0; i < 12; ++i)
        pEditShell->AcceptRedline(0);

    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // show tracked row deletions
    assertXPath(pXmlDoc, "/root/page", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);

    // select all the text, including the texts before and after the table
    // Note: this table contains tracked changes, which was a
    // problem for the original OOo implementation of track changes,
    // resulting empty tables after accepting the deletion of these tables.
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    calcLayout();
    pXmlDoc = parseLayoutDump();

    // table is deleted with change tracking: it still exists
    assertXPath(pXmlDoc, "/root/page", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);

    // accept all deletions, removing the table completely
    while (pEditShell->GetRedlineCount() > 0)
        pEditShell->AcceptRedline(0);

    calcLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page", 1);
    // This was 1 (bad empty table)
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf155345)
{
    createSwDoc("tdf144057.fodt");
    SwDoc* pDoc(getSwDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    // reject all deletions
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    // enable redlining
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // delete table column with track changes
    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // show tracked column deletions
    assertXPath(pXmlDoc, "/root/page", 4);

    // hide tracked table column deletions
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    calcLayout();
    pXmlDoc = parseLayoutDump();

    // This was 4 (unhidden tracked table column deletions)
    assertXPath(pXmlDoc, "/root/page", 2);

    // show tracked table column deletions again
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    calcLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf109137)
{
    createSwDoc("tdf109137.docx");
    // FIXME: Error: attribute "text:start-value" has a bad value
    skipValidation();
    saveAndReload(u"writer8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 0, the blue rectangle moved from the 1st to the 2nd page.
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly/notxt",
                /*nNumberOfNodes=*/1);
}

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint72) { createSwDoc("forcepoint72-1.rtf"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint75) { createSwDoc("forcepoint75-1.rtf"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testUXTSOREL) { createSwDoc("LIBREOFFICE-UXTSOREL.rtf"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepointFootnoteFrame)
{
    createSwDoc("forcepoint-swfootnoteframe-1.rtf");
}

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint76) { createSwDoc("forcepoint76-1.rtf"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testN4LA0OHZ) { createSwDoc("LIBREOFFICE-N4LA0OHZ.rtf"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint89) { createSwWebDoc("forcepoint89.html"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint90) { createSwDoc("forcepoint90.rtf"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint91) { createSwWebDoc("forcepoint91.html"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint92) { createSwDoc("forcepoint92.doc"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint93)
{
    createSwDoc("forcepoint93-1.rtf");
    createSwDoc("forcepoint93-2.rtf");
}

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint94) { createSwWebDoc("forcepoint94.html"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint98) { createSwWebDoc("forcepoint98.html"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint100) { createSwWebDoc("forcepoint100.html"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint102) { createSwDoc("forcepoint102.rtf"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint103) { createSwWebDoc("forcepoint103.html"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf147485Forcepoint)
{
    createSwDoc("tdf147485-forcepoint.docx");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf118058)
{
    createSwDoc("tdf118058.fodt");
    // This resulted in a layout loop.
    calcLayout();
}

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint99) { createSwWebDoc("forcepoint99.html"); }

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf128611)
{
    createSwDoc("tdf128611.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 14
    // i.e. there were multiple portions in the first paragraph of the A1 cell, which means that the
    // rotated text was broken into multiple lines without a good reason.
    assertXPathContent(pXmlDoc, "//tab/row/cell[1]/txt", u"Abcd efghijkl");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf159101)
{
    createSwDoc("tdf159101.odt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwFieldPortion",
                "expand", u"One");

    // Without the fix in place, this test would have failed with
    // - Expected: two
    // - Actual  :  two
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[2]/SwFieldPortion",
                "expand", u"two");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf157628)
{
    createSwDoc("tdf157628.docx");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwLinePortion",
                "portion", u"This is in first row");
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[2]/SwLinePortion",
                "portion", u"This is second row*");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf142694)
{
    createSwDoc("tdf142694-1.odt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // the problem with this table is that the 3rd text frame (inside a nested
    // table) contains an as-char fly that doesn't fit on a page with the
    // repeated header row; currently it's split so that the 3rd text frame is
    // on a page of its own and extends outside the table.

    assertXPathNoAttribute(pXmlDoc, "/root/page[4]/body/tab[1]", "precede");
    CPPUNIT_ASSERT(!getXPath(pXmlDoc, "/root/page[4]/body/tab[1]", "follow").isEmpty());
    assertXPath(pXmlDoc, "/root/page[4]/body/tab[1]/row[1]/cell[1]/tab", 1);
    assertXPath(pXmlDoc, "/root/page[4]/body/tab[1]/row[2]/cell[1]/txt", 2);
    CPPUNIT_ASSERT(!getXPath(pXmlDoc, "/root/page[5]/body/tab[1]", "precede").isEmpty());
    assertXPathNoAttribute(pXmlDoc, "/root/page[5]/body/tab[1]", "follow");
    assertXPath(pXmlDoc, "/root/page[5]/body/tab[1]/row[1]/cell[1]/tab", 1);
    assertXPath(pXmlDoc, "/root/page[5]/body/tab[1]/row[2]/cell[1]/tab", 1);
    assertXPathNoAttribute(pXmlDoc, "/root/page[5]/body/tab[1]/row[2]/cell[1]/tab", "follow");
    assertXPath(pXmlDoc, "/root/page[5]/body/tab[1]/row[2]/cell[1]/tab/row/cell/txt", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf152413)
{
    createSwDoc("tdf152413.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 2
    // - In <>, XPath '/root/page/body/txt/SwParaPortion/SwLineLayout/SwLinePortion' number of nodes is incorrect
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/SwLinePortion", "portion",
                u"صِرَ ٰطَ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf125893)
{
    createSwDoc("tdf125893.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 400. The paragraph must have zero top border.
    assertXPath(pXmlDoc, "/root/page/body/txt[4]/infos/prtBounds", "top", u"0");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
