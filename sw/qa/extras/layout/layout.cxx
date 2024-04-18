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
#include <vcl/event.hxx>
#include <vcl/scheduler.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/editobj.hxx>
#include <comphelper/sequence.hxx>

#include <fmtfsize.hxx>
#include <fmtcntnt.hxx>
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
#include <IDocumentRedlineAccess.hxx>
#include <unoframe.hxx>
#include <drawdoc.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdotext.hxx>
#include <dcontact.hxx>

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter : public SwModelTestBase
{
public:
    SwLayoutWriter()
        : SwModelTestBase("/sw/qa/extras/layout/data/")
    {
    }

protected:
    void CheckRedlineFootnotesHidden();
    void CheckRedlineSectionsHidden();
};

// this is a member because our test classes have protected members :(
void SwLayoutWriter::CheckRedlineFootnotesHidden()
{
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "24");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "1");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "foaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "type", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "expand", "2");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex", "13");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "1");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "ac");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/merged", "paraPropsNodeIndex", "16");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "2");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "mo");
}

void SwLayoutWriter::CheckRedlineSectionsHidden()
{
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", "folah");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/merged", "paraPropsNodeIndex", "20");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", "folah");
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
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // verify after load
    CheckRedlineFootnotesHidden();

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // show: nothing is merged
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "1");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "type", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "expand", "2");

    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "1");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "a");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "b");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", "c");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "2");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "def");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "3");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "ar");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[3]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[3]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "3");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[3]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[3]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "ghi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "4");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "type", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                "expand", "5");

    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[4]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[4]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "4");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[4]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[4]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "jkl");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "type", "PortionType::FootnoteNum");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                "expand", "5");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "m");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "n");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/ftncont/ftn[5]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", "o");

    // verify after hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    discardDumpedLayout();
    CheckRedlineFootnotesHidden();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf136588)
{
    createSwDoc("tdf136588.docx");
    auto pXMLLayoutDump = parseLayoutDump();
    CPPUNIT_ASSERT(pXMLLayoutDump);

    //there was a bad line break before, the correct break layout is this:
    assertXPath(pXMLLayoutDump, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[2]", "portion",
                "effectively by modern-day small to ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysInBody)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("baz");
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
    pWrtShell->Insert("abc");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("def");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("ghi");

    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "14");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "ahi");

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "az");
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "14");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "foaz");

        { // hide: no anchored object shown
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//anchored");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "az");
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "14");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "ahi");

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "az");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "hi");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf134272)
{
    createSwDoc("tdf134472.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/infos/bounds", "height", "843");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/infos/bounds", "bottom", "2819");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestNestedTableMoveFwd)
{
    createSwDoc("tabellen_test_windows_1.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // the row with the nested table should not be split but be the first row on page 2
    assertXPathContent(pXmlDoc, "/root/page[1]/body/tab[1]/row[last()]/cell[1]/txt[1]",
                       "Tabelle 1");
    assertXPathContent(
        pXmlDoc, "/root/page[2]/body/tab[1]/row[1]/cell[1]/tab[1]/row[1]/cell[1]/txt", "Tabelle 2");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf136613)
{
    createSwDoc("tdf136613.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

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
        const SwRect& rRect = pFlyFormat->FindLayoutRect();
        CPPUNIT_ASSERT(!rRect.IsEmpty());

        //...if it is on the page. This will fail if not.
        CPPUNIT_ASSERT_MESSAGE("The pictures are outside the page!", rPageRect.Contains(rRect));
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf88496)
{
    createSwDoc("tdf88496.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 4, table fallback "switch off repeating header" started on a new page
    assertXPath(pXmlDoc, "/root/page", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysInHeader)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->ChangeHeaderOrFooter(u"Default Page Style", /*bHeader*/ true, /*bOn*/ true, false);
    CPPUNIT_ASSERT(
        pWrtShell
            ->IsInHeaderFooter()); // assume this is supposed to put cursor in the new header...
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("baz");
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
    pWrtShell->Insert("abc");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("def");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("ghi");

    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "length",
                    "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                    "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "11");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout[1]",
            "type", "PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout[1]",
            "portion", "ahi");

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "length",
                    "0");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "az");
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // now the frame has no Text portion? not sure why it's a 0-length one first and now none?
        //        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type", "PortionType::Para");
        //        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "length", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "foaz");

        { // hide: no anchored object shown
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//anchored");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "length", "0");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "az");
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "length", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "11");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "ahi");

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "length", "0");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "az");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "hi");
    }
}

#if !defined(MACOSX)
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf150606)
{
    createSwDoc("tdf150606-1-min.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

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
    dispatchCommand(mxComponent, ".uno:DeleteTable", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/section/column/body/tab", 0);
    assertXPath(pXmlDoc, "/root/page", 1);

    pWrtShell->Undo();
    Scheduler::ProcessEventsToIdle();

    discardDumpedLayout();
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
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the layout xml
    // SDRATTR_TEXT_LEFTDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item[@whichId='1072']",
                "value", "567");
    // SDRATTR_TEXT_RIGHTDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item[@whichId='1073']",
                "value", "1134");
    // SDRATTR_TEXT_UPPERDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item[@whichId='1074']",
                "value", "1701");
    // SDRATTR_TEXT_LOWERDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item[@whichId='1075']",
                "value", "2268");

    // Check the textbox-shape import too
    auto xShp = getShape(1);
    CPPUNIT_ASSERT(xShp);

    uno::Reference<beans::XPropertySet> xShapeProps(xShp, uno::UNO_QUERY);

    SwFrameFormat* pFrameFormat = SwTextBoxHelper::getOtherTextBoxFormat(xShp);
    CPPUNIT_ASSERT(pFrameFormat);

    // The shape has these values to copy to the associated text frame after modification::
    const tools::Long nLPaddng
        = xShapeProps->getPropertyValue("TextLeftDistance").get<tools::Long>();
    const tools::Long nRPaddng
        = xShapeProps->getPropertyValue("TextRightDistance").get<tools::Long>();
    const tools::Long nTPaddng
        = xShapeProps->getPropertyValue("TextUpperDistance").get<tools::Long>();
    const tools::Long nBPaddng
        = xShapeProps->getPropertyValue("TextLowerDistance").get<tools::Long>();

    CPPUNIT_ASSERT_EQUAL(tools::Long(1000), nLPaddng);
    CPPUNIT_ASSERT_EQUAL(tools::Long(2000), nRPaddng);
    CPPUNIT_ASSERT_EQUAL(tools::Long(3000), nTPaddng);
    CPPUNIT_ASSERT_EQUAL(tools::Long(4001), nBPaddng);

    // TODO: modify shape distance via UNO with text frame synchronization
    // Check the textbox as well:
    auto xTxFrm = SwXTextFrame::CreateXTextFrame(*pFrameFormat->GetDoc(), pFrameFormat);
    CPPUNIT_ASSERT(xTxFrm);

    const tools::Long nFrameLeftPaddng
        = xTxFrm->getPropertyValue("LeftBorderDistance").get<tools::Long>();
    const tools::Long nFrameRightPaddng
        = xTxFrm->getPropertyValue("RightBorderDistance").get<tools::Long>();
    const tools::Long nFrameTopPaddng
        = xTxFrm->getPropertyValue("TopBorderDistance").get<tools::Long>();
    const tools::Long nFrameBottomPaddng
        = xTxFrm->getPropertyValue("BottomBorderDistance").get<tools::Long>();

    // Check if the shape and frame have different setting
    CPPUNIT_ASSERT_EQUAL(nLPaddng, nFrameLeftPaddng);
    CPPUNIT_ASSERT_EQUAL(nRPaddng, nFrameRightPaddng);
    CPPUNIT_ASSERT_EQUAL(nTPaddng, nFrameTopPaddng);
    CPPUNIT_ASSERT_EQUAL(nBPaddng, nFrameBottomPaddng);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysInFootnote)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->InsertFootnote("");
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
    pWrtShell->Insert("quux");

    pWrtShell->SttEndDoc(false);

    pWrtShell->InsertFootnote("");
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("baz");

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
    pWrtShell->Insert("abc");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("def");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("ghi");

    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "25");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote']",
                    "type", "PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote']",
                    "expand", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    "7");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "17");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", "ahi");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", "PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", "1");

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "type", "PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "expand", "1");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                    "type", "PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                    "expand", "2");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", "quux");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", "PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", "1");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", "a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", "bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", "hi");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", "PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", "2");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", "fo");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", "o");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "type", "PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "portion", "bar");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", "b");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", "az");
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "25");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "type", "PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "expand", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    "7");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", "PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", "1");

        { // hide: no anchored object shown
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//anchored");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "type", "PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "expand", "1");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                    "type", "PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[2]",
                    "expand", "2");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", "quux");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", "PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", "1");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", "PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", "2");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", "fo");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", "o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", "a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", "bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", "hi");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "type", "PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "portion", "bar");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", "b");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", "az");
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "25");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "type", "PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
                    "expand", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    "7");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "17");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", "ahi");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", "PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", "1");

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote'][1]",
                    "type", "PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote'][1]",
                    "expand", "1");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote'][2]",
                    "type", "PortionType::Footnote");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/"
                    "child::*[@type='PortionType::Footnote'][2]",
                    "expand", "2");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", "quux");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", "PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[1]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", "1");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "type", "PortionType::FootnoteNum");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwFieldPortion[1]",
            "expand", "2");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", "fo");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", "o");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "type", "PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
            "portion", "bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", "a");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", "bc");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/"
                    "SwLineLayout/SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/"
                    "SwLineLayout/SwLinePortion[2]",
                    "portion", "hi");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
            "portion", "b");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "type", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/ftncont/ftn[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
            "portion", "az");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf143239)
{
    createSwDoc("tdf143239-1-min.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // These are unstable on macOS and Win64 builds,
    // so only test that they restore original values for now
    OUString p2txt1Left, p2txt2Left, p3txt1Left;

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "top",
                    "18540");
        p2txt1Left
            = getXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "left");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly[1]/infos/bounds", "top",
                    "23894");
        p2txt2Left
            = getXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly[1]/infos/bounds", "left");
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly[1]/infos/bounds", "top",
                    "35662");
        p3txt1Left
            = getXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly[1]/infos/bounds", "left");
        assertXPath(pXmlDoc, "/root/page", 3);
        discardDumpedLayout();
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
                    "18540");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "left",
                    p2txt1Left);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly[1]/infos/bounds", "top",
                    "23894");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/anchored/fly[1]/infos/bounds", "left",
                    p2txt2Left);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly[1]/infos/bounds", "top",
                    "35662");
        assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored/fly[1]/infos/bounds", "left",
                    p3txt1Left);
        assertXPath(pXmlDoc, "/root/page", 3);
        discardDumpedLayout();
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
        "portion", "Abc def ghi jkl mno pqr stu vwx yz.");
    // height was: 1517
    // tdf#134782 height was: 379
    assertXPath(pLayout, "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/infos/bounds", "height",
                "253");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf134277)
{
    createSwDoc("tdf134277.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwDocShell* pShell = pDoc->GetDocShell();

    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/metafile/push/push/push/layoutmode[2]");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bad position of shape in page break!", 0,
                                 xmlXPathNodeSetGetLength(pXmlNodes));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf116486)
{
    createSwDoc("tdf116486.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    OUString aTop = parseDump(
        "/root/page/body/txt/SwParaPortion/SwLineLayout[1]/child::*[@type='PortionType::Fly']",
        "height");
    CPPUNIT_ASSERT_EQUAL(OUString("4006"), aTop);
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
                "De kleur u (rood) in het rechtervlak (R), de kleur r (wit) beneden (D),");
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
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // the problem was that line 5 was truncated at "this  "
    // due to the fly anchored in previous paragraph
    assertXPath(pLayout, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[5]", "portion",
                "to access any service, any time, anywhere. From this  perspective, satellite "
                "boasts some ");
    assertXPath(pLayout, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[6]", "portion",
                "significant advantages. ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testNoLineBreakAtSlash)
{
    createSwDoc("no-line-break-at-slash.fodt");
    xmlDocUniquePtr pLayout = parseLayoutDump();

    // the line break was between  "Foostrasse 13/c/" and "2"
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout", 2);
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]/child::*", "type",
                "PortionType::Para");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/child::*[1]",
                "type", "PortionType::Text");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/child::*[2]",
                "type", "PortionType::Blank");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/child::*[3]",
                "type", "PortionType::Text");

    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                "Blah blah bla bla bla ");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/SwLinePortion[1]",
                "portion", "Foostrasse");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]/SwLinePortion[2]",
                "portion", "13/c/2, etc.");
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
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("baz");
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
    pWrtShell->Insert("abc");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("def");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("ghi");

    SwFormatAnchor anchor2(RndStdIds::FLY_AT_CHAR);
    pWrtShell->StartOfSection(false); // start of fly...
    anchor2.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    flySet.Put(anchor2);
    SwFrameFormat const* pFly2 = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly2 != nullptr);
    // move inside fly2
    pWrtShell->GotoFly(pFly2->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Insert("jkl");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("mno");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("pqr");

    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "19");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "6");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/merged",
            "paraPropsNodeIndex", "11");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "jqr");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwFixPortion[1]",
                    "type",
                    "PortionType::Fly"); // remove???
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "type", "PortionType::Lay");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "portion", "abhi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "foaz");

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "j");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "kl");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "mno");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "p");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "qr");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwFixPortion[1]",
                    "type",
                    "PortionType::Fly"); // remove???
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "ab");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "c");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "az");
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "19");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "foaz");

        { // hide: no anchored object shown
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//anchored");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "j");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "kl");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "ab");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "c");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwFixPortion[1]",
                    "type",
                    "PortionType::Fly"); // remove???
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "type", "PortionType::Lay");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "portion", "mno");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "p");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "qr");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "az");
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

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "19");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "6");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/merged",
            "paraPropsNodeIndex", "11");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "jqr");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwFixPortion[1]",
                    "type",
                    "PortionType::Fly"); // remove???
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "type", "PortionType::Lay");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLineLayout[1]",
                    "portion", "abhi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "foaz");

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
        discardDumpedLayout();
        pXmlDoc = parseLayoutDump();

        { // show: nothing is merged
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "fo");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "o");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]", "type",
                    "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "bar");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "ab");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "c");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion[1]",
                    "portion", "def");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "j");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "kl");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "type", "PortionType::Para");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[2]/"
                    "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                    "portion", "mno");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "p");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/"
                    "SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "qr");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwFixPortion[1]",
                    "type",
                    "PortionType::Fly"); // remove???
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[1]",
                    "portion", "g");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "type", "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                    "SwLinePortion[2]",
                    "portion", "hi");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                    "portion", "b");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]", "type",
                    "PortionType::Text");
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                    "portion", "az");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysAtFlys)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("baz");
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
    pWrtShell->Insert("abc");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("def");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("ghi");

    SwFormatAnchor anchor2(RndStdIds::FLY_AT_FLY);
    SwPosition pos(*pFly1->GetContent().GetContentIdx());
    anchor2.SetAnchor(&pos);
    flySet.Put(anchor2);
    SwFrameFormat const* pFly2 = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly2 != nullptr);
    // move inside fly2
    pWrtShell->GotoFly(pFly2->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Insert("jkl");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("mno");
    pWrtShell->SplitNode(false);
    pWrtShell->Insert("pqr");

    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
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

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "19");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                "paraPropsNodeIndex", "6");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/merged",
                "paraPropsNodeIndex", "11");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "portion", "jqr");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwFixPortion[1]",
                "type",
                "PortionType::Fly"); // remove???
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLineLayout[1]",
                "type", "PortionType::Lay");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLineLayout[1]",
                "portion", "abhi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", "foaz");

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    { // show: nothing is merged
        xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
        xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
        CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
        xmlXPathFreeObject(pXmlObj);
    }

    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "portion", "j");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "portion", "kl");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "portion", "mno");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "portion", "p");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "portion", "qr");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwFixPortion[1]",
                "type",
                "PortionType::Fly"); // remove???
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "portion", "ab");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "portion", "c");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "portion", "def");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "portion", "g");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "portion", "hi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "az");

    // anchor to 2nd (deleted) paragraph
    pWrtShell->StartOfSection();
    pWrtShell->Down(false, 1);
    anchor1.SetType(RndStdIds::FLY_AT_CHAR);
    anchor1.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "19");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", "foaz");

    { // hide: no anchored object shown
        xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//anchored");
        xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
        CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
        xmlXPathFreeObject(pXmlObj);
    }

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    { // show: nothing is merged
        xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
        xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
        CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
        xmlXPathFreeObject(pXmlObj);
    }

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "o");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "portion", "j");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "portion", "kl");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/SwParaPortion/"
                "SwLineLayout/SwParaPortion[1]",
                "portion", "mno");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[1]",
                "portion", "p");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/SwParaPortion/"
                "SwLineLayout/SwLinePortion[2]",
                "portion", "qr");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwFixPortion[1]",
                "type",
                "PortionType::Fly"); // remove???
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "portion", "ab");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "portion", "c");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "portion", "def");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[1]",
                "portion", "g");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/SwParaPortion/SwLineLayout/"
                "SwLinePortion[2]",
                "portion", "hi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "az");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineSections)
{
    createSwDoc("redline_sections.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // verify after load
    CheckRedlineSectionsHidden();

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // show: nothing is merged
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[1]", "type",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[2]", "type",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[2]",
                "portion", "o");

    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/child::*[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/anchored/fly[1]/txt[1]/SwParaPortion/"
                "SwLineLayout/child::*[1]",
                "portion", "FRAME");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout/child::*[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[2]/SwParaPortion/SwLineLayout/child::*[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[1]/txt[2]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/child::*[1]", "type",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/child::*[2]", "type",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/child::*[2]",
                "portion", "lah");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/child::*[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", "fo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/child::*[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/child::*[2]",
                "portion", "o");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[2]/SwParaPortion/SwLineLayout/child::*[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[2]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/child::*[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", "b");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/child::*[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/child::*[2]",
                "portion", "lah");

    // verify after hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    CheckRedlineSectionsHidden();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TDF69647_images)
{
    createSwDoc("tdf69647_images.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of pages does not match!", 2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TDF69647_text)
{
    createSwDoc("tdf69647_text.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of pages does not match!", 2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineTables)
{
    createSwDoc("redline_table.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // verify after load
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", "foar");

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    // show: nothing is merged
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//merged");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "o");

    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/anchored/fly[1]/txt[1]/"
                "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/anchored/fly[1]/txt[1]/"
                "SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", "FRAME");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "portion", "aaa");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[2]/cell[2]/txt[1]/SwParaPortion/SwLineLayout/"
                "SwParaPortion[1]",
                "portion", "ddd");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "ar");

    // verify after hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", "foar");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf144057)
{
    createSwDoc("tdf144057.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // show tracked row deletions
    assertXPath(pXmlDoc, "/root/page", 4);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", "A6");
    assertXPath(pXmlDoc,
                "/root/page[2]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", "A12");
    assertXPath(pXmlDoc,
                "/root/page[3]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", "B6");
    assertXPath(pXmlDoc,
                "/root/page[4]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", "B12");

    // hide tracked table and table row deletions
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    // This was 4 (unhidden tracked table and table row deletions)
    assertXPath(pXmlDoc, "/root/page", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 5);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[5]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", "B12");

    // show tracked table and table row deletions again
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 4);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", "A6");
    assertXPath(pXmlDoc,
                "/root/page[2]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", "A12");
    assertXPath(pXmlDoc,
                "/root/page[3]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", "B6");
    assertXPath(pXmlDoc,
                "/root/page[4]/body/tab/row[6]/cell/txt/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", "B12");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf144347)
{
    createSwDoc("tdf144057.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());

    // enable redlining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    // remove first table
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    for (int i = 0; i < 12; ++i)
        pEditShell->AcceptRedline(0);

    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // show tracked row deletions
    assertXPath(pXmlDoc, "/root/page", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);

    // select all the text, including the texts before and after the table
    // Note: this table contains tracked changes, which was a
    // problem for the original OOo implementation of track changes,
    // resulting empty tables after accepting the deletion of these tables.
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    // table is deleted with change tracking: it still exists
    assertXPath(pXmlDoc, "/root/page", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);

    // accept all deletions, removing the table completely
    while (pEditShell->GetRedlineCount() > 0)
        pEditShell->AcceptRedline(0);

    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page", 1);
    // This was 1 (bad empty table)
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf109137)
{
    createSwDoc("tdf109137.docx");
    // FIXME: it's not possible to use 'reload' here because the validation fails with
    // Error: attribute "text:start-value" has a bad value
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    }));
    xStorable->storeToURL(maTempFile.GetURL(), aDescriptor);
    mxComponent = loadFromDesktop(maTempFile.GetURL(), "com.sun.star.text.TextDocument");
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
    SwDoc* pDoc = getSwDoc();
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
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
    assertXPathContent(pXmlDoc, "//tab/row/cell[1]/txt", "Abcd efghijkl");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf125893)
{
    createSwDoc("tdf125893.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 400. The paragraph must have zero top border.
    assertXPath(pXmlDoc, "/root/page/body/txt[4]/infos/prtBounds", "top", "0");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf134463)
{
    createSwDoc("tdf134463.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 621. The previous paragraph must have zero bottom border.
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/infos/prtBounds", "top", "21");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf117188)
{
    createSwDoc("tdf117188.docx");
    reload("writer8", "tdf117188.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    OUString sWidth = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "width");
    OUString sHeight = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "height");
    // The text box must have zero border distances
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "left", "0");
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "top", "0");
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "width", sWidth);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "height", sHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf117187)
{
    createSwDoc("tdf117187.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // there should be no fly portions
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[@nType='PortionType::Fly']", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf119875)
{
    createSwDoc("tdf119875.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFirstTop
        = getXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "top").toInt32();
    sal_Int32 nSecondTop
        = getXPath(pXmlDoc, "/root/page[2]/body/section[2]/infos/bounds", "top").toInt32();
    // The first section had the same top value as the second one, so they
    // overlapped.
    CPPUNIT_ASSERT_LESS(nSecondTop, nFirstTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf120287)
{
    createSwDoc("tdf120287.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin Word-specific compat flag did not imply
    // default-in-Word printer-independent layout, resulting in an additional
    // line break.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf106234)
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
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwGluePortion",
                "type", "PortionType::Margin");
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwGluePortion",
                "width", "0");
    // but not after centered, right and decimal tabulators
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwGluePortion",
                "type", "PortionType::Margin");
    // This was a justified line, without width
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwGluePortion",
                "width", "7882");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf120287b)
{
    createSwDoc("tdf120287b.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 1418, TabOverMargin did the right split of the paragraph to two
    // lines, but then calculated a too large tab portion size on the first
    // line.
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::TabRight']",
        "width", "19");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf120287c)
{
    createSwDoc("tdf120287c.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2, the second line was not broken into a 2nd and a 3rd one,
    // rendering text outside the paragraph frame.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf122878)
{
    createSwDoc("tdf122878.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf115094)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf112290)
{
    createSwDoc("tdf112290.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    auto pXml = parseLayoutDump();
    assertXPath(pXml, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion", "Xxxx Xxxx");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testKeepWithNextPlusFlyFollowTextFlow)
{
    createSwDoc("keep-with-next-fly.fodt");

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // 3 text frames on page 1
        assertXPath(pXmlDoc, "/root/page[1]/body/infos/bounds", "bottom", "7540");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "height", "276");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/bounds", "height", "276");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly/infos/bounds", "top", "1694");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/infos/bounds", "height", "276");
        assertXPath(pXmlDoc, "/root/page", 1);
        discardDumpedLayout();
    }

    dispatchCommand(mxComponent, ".uno:Fieldnames", {});
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // 1 text frame on page 1, and some empty space
        assertXPath(pXmlDoc, "/root/page[1]/body/infos/bounds", "bottom", "7540");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "height", "5796");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "bottom", "7213");
        // 2 text frames on page 2
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/infos/bounds", "height", "276");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly/infos/bounds", "top", "10093");
        assertXPath(pXmlDoc, "/root/page[2]/body/txt[2]/infos/bounds", "height", "276");
        assertXPath(pXmlDoc, "/root/page", 2);
        discardDumpedLayout();
    }

    dispatchCommand(mxComponent, ".uno:Fieldnames", {});
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // 3 text frames on page 1
        assertXPath(pXmlDoc, "/root/page[1]/body/infos/bounds", "bottom", "7540");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "height", "276");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/infos/bounds", "height", "276");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly", 1);
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly/infos/bounds", "top", "1694");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/infos/bounds", "height", "276");
        assertXPath(pXmlDoc, "/root/page", 1);
        discardDumpedLayout();
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf122607)
{
    createSwDoc("tdf122607.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/SwParaPortion/SwLineLayout/child::*[1]",
                "height", "253");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/SwParaPortion/SwLineLayout/child::*[1]",
                "width", "427");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/txt[1]/anchored/fly/tab/row[2]/cell/txt[7]/anchored/"
                "fly/txt/SwParaPortion/SwLineLayout/child::*[1]",
                "portion", "Fax:");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf122607_regression)
{
    discardDumpedLayout();
    if (mxComponent.is())
        mxComponent->dispose();

    OUString const url(createFileURL(u"tdf122607_leerzeile.odt"));

    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue("Hidden", -1, uno::Any(true), beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    mxComponent = loadFromDesktop(url, "com.sun.star.text.TextDocument",
                                  comphelper::containerToSequence(aFilterOptions));
    uno::Sequence<beans::PropertyValue> props(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer_pdf_Export")) },
    }));
    utl::TempFileNamed aTempFile;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), props);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf150616)
{
    discardDumpedLayout();
    if (mxComponent.is())
        mxComponent->dispose();

    OUString const url(createFileURL(u"in_056132_mod.odt"));

    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue("Hidden", -1, uno::Any(true), beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    mxComponent = loadFromDesktop(url, "com.sun.star.text.TextDocument",
                                  comphelper::containerToSequence(aFilterOptions));
    uno::Sequence<beans::PropertyValue> props(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer_pdf_Export")) },
    }));
    utl::TempFileNamed aTempFile;
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), props);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // this one was 0 height
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[2]/SwParaPortion/SwLineLayout",
                "portion", "Important information here!");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[2]/infos/bounds", "height",
                "253");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[2]/infos/bounds", "top",
                "7925");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[3]/SwParaPortion/SwLineLayout",
                "portion", "xxx 111 ");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[3]/infos/bounds", "height",
                "697");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[3]/row[2]/cell[2]/txt[3]/infos/bounds", "top",
                "8178");

    aTempFile.EnableKillingFile();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testBtlrCell)
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
    assertXPath(pXmlDoc, "(//rect)[2]", "left", "1979");

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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf123898)
{
    createSwDoc("tdf123898.odt");

    // Make sure spellchecker has done its job already
    Scheduler::ProcessEventsToIdle();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure that the arrow on the left is not there (the first portion's type is
    // PortionType::Arrow if it's there)
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/fly/txt/SwParaPortion/SwLineLayout[1]/child::*[1]",
                "type", "PortionType::Para");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf123651)
{
    createSwDoc("tdf123651.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with 'Expected: 7639;
    // Actual: 12926'. The shape was below the second "Lorem ipsum" text, not above it.
    const sal_Int32 nTopValue
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "top").toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7639, nTopValue, 10);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf116501)
{
    //just care it doesn't freeze
    createSwDoc("tdf116501.odt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf123163)
{
    //just care it doesn't assert
    createSwDoc("tdf123163-1.docx");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testAbi11870)
{
    //just care it doesn't assert
    createSwDoc("abi11870-2.odt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf118719)
{
    // Insert a page break.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTabOverMargin)
{
    createSwDoc("tab-over-margin.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 2nd paragraph has a tab over the right margin, and with the TabOverMargin compat option,
    // there is enough space to have all content in a single line.
    // Without the accompanying fix in place, this test would have failed, there were 2 lines.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testImageComment)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testScriptField)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testCommentCursorPosition)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testCombiningCharacterCursorPosition)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf64222)
{
    createSwDoc("tdf64222.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']",
        "font-height", "560");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf113014)
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
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text", "1.");
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[3]/text", "2.");
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[5]/text", "3.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf130218)
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
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text", "Text");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf127235)
{
    createSwDoc("tdf127235.odt");
    SwDoc* pDoc = getSwDoc();
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf138039)
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
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "top", "17915");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "height",
                "15819");
    // paragraph on third page
    assertXPath(pXmlDoc, "/root/page[3]/body/tab", 0);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/anchored", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf134298)
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
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "top", "17897");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly[1]/infos/bounds", "height",
                "15819");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testShapeAllowOverlap)
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
        xDocument->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xDocument, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("AllowOverlap", uno::Any(false));
    xShapeProperties->setPropertyValue("AnchorType",
                                       uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xDrawPageSupplier->getDrawPage()->add(xShape);

    aPoint = awt::Point(2000, 2000);
    xShape.set(xDocument->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    xShapeProperties.set(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("AllowOverlap", uno::Any(false));
    xShapeProperties->setPropertyValue("AnchorType",
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testShapeAllowOverlapWrap)
{
    // Create an empty document with two, intentionally overlapping shapes.
    // Set their AllowOverlap property to false and their wrap to through.
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    awt::Point aPoint(1000, 1000);
    awt::Size aSize(2000, 2000);
    uno::Reference<drawing::XShape> xShape(
        xDocument->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xDocument, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("AllowOverlap", uno::Any(false));
    xShapeProperties->setPropertyValue("AnchorType",
                                       uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProperties->setPropertyValue("Surround", uno::Any(text::WrapTextMode_THROUGH));
    xDrawPageSupplier->getDrawPage()->add(xShape);

    aPoint = awt::Point(2000, 2000);
    xShape.set(xDocument->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setPosition(aPoint);
    xShape->setSize(aSize);
    xShapeProperties.set(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("AllowOverlap", uno::Any(false));
    xShapeProperties->setPropertyValue("AnchorType",
                                       uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProperties->setPropertyValue("Surround", uno::Any(text::WrapTextMode_THROUGH));
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf124600)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf124601)
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
    assertXPath(pXmlDoc, "/root/page[2]/ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf124601b)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf124770)
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
    aFont.SetFamilyName("Liberation Serif");
    aTextSet.Put(aFont);
    SvxFontHeightItem aHeight(400, 100, RES_CHRATR_FONTSIZE);
    aTextSet.Put(aHeight);
    SvxPostureItem aItalic(ITALIC_NORMAL, RES_CHRATR_POSTURE);
    aTextSet.Put(aItalic);
    pWrtShell->SetAttrSet(aTextSet);

    // Insert the text.
    pWrtShell->Insert2("HHH");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the italic string was broken into 2 lines, while Word kept it in a single line.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testContinuousEndnotesInsertPageAtStart)
{
    // Create a new document with CONTINUOUS_ENDNOTES enabled.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 3);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the footnote container remained on page 2.
    assertXPath(pXmlDoc, "/root/page[3]/ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testContinuousEndnotesDeletePageAtStart)
{
    // Create a new document with CONTINUOUS_ENDNOTES enabled.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the endnote remained on an (otherwise) empty 2nd page.
    assertXPath(pXmlDoc, "/root/page", 1);
    assertXPath(pXmlDoc, "/root/page[1]/ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf128399)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf145826)
{
    createSwDoc("tdf145826.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/root/page/body/section/column", 2);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    assertXPath(pXmlDoc, "/root/page/body/section/column[1]/ftncont", 1);
    assertXPath(pXmlDoc, "/root/page/body/section/column[2]/ftncont", 1);
    assertXPath(pXmlDoc, "/root/page/body/section/column[1]/ftncont/ftn", 3);
    assertXPath(pXmlDoc, "/root/page/body/section/column[2]/ftncont/ftn", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf105481)
{
    createSwDoc("tdf105481.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf117982)
{
    createSwDoc("tdf117982.docx");
    SwDoc* pDocument = getSwDoc();
    SwDocShell* pShell = pDocument->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text", "FOO AAA");
    //The first cell must be "FOO AAA". If not, this means the first cell content not visible in
    //the source document.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf128959)
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
        pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout[1]",
        "portion",
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Maecenas porttitor congue ");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout[2]",
        "portion",
        "massa. Fusce posuere, magna sed pulvinar ultricies, purus lectus malesuada libero, sit ");
    // last line of the paragraph in the split table cell on the second page
    assertXPath(pXmlDoc,
                "/root/page[2]/body/tab[1]/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout[1]",
                "portion", "amet commodo magna eros quis urna.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf121658)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    createSwDoc("tdf121658.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 2 hyphenated words should appear in the document (in the lowercase words).
    // Uppercase words should not be hyphenated.
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf149420)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    createSwDoc("tdf149420.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 3 hyphenated words should appear in the document (last paragraph
    // has got a 1 cm hyphenation zone, removing two hyphenations, which visible
    // in the second paragraph).
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']", 8);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf149324)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    createSwDoc("tdf149324.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 3 hyphenated words should appear in the document (last paragraph
    // has got a 7-character word limit for hyphenation, removing the
    // hyphenation "ex-cept".
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf149248)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    createSwDoc("tdf149248.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 1 hyphenated word should appear in the document (last word of the second
    // paragraph). Last word should not be hyphenated for the fourth paragraph
    // (the same paragraph, but with forbidden hyphenation of the last word).
    assertXPath(pXmlDoc, "//SwLineLayout/child::*[@type='PortionType::Hyphen']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testWriterImageNoCapture)
{
    createSwDoc("writer-image-no-capture.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nPageLeft = getXPath(pXmlDoc, "//page/infos/bounds", "left").toInt32();
    sal_Int32 nImageLeft = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "left").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less than: 284
    // - Actual  : 284
    // i.e. the image position was modified to be inside the page frame ("captured"), even if Word
    // does not do that.
    CPPUNIT_ASSERT_LESS(nPageLeft, nImageLeft);
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
    CPPUNIT_ASSERT(pPage->getFrameArea().Contains(aFlyRect));
    return aFlyRect;
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testStableAtPageAnchoredFlyPosition)
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
    pWrtShell->Insert("foo");

    // get the current position of the frame on page four
    SwRect aRelayoutRect = lcl_getVisibleFlyObjRect(pWrtShell);

    // the anchored frame should not have moved
    CPPUNIT_ASSERT_EQUAL(aOrigRect, aRelayoutRect);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf134548)
{
    createSwDoc("tdf134548.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Second paragraph has two non zero width tabs in beginning of line
    {
        OUString sNodeType = parseDump(
            "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[1]", "type");
        CPPUNIT_ASSERT_EQUAL(OUString("PortionType::TabLeft"), sNodeType);
        sal_Int32 nWidth
            = parseDump("/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[1]",
                        "width")
                  .toInt32();
        CPPUNIT_ASSERT_GREATER(sal_Int32(0), nWidth);
    }
    {
        OUString sNodeType = parseDump(
            "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[2]", "type");
        CPPUNIT_ASSERT_EQUAL(OUString("PortionType::TabLeft"), sNodeType);
        sal_Int32 nWidth
            = parseDump("/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFixPortion[2]",
                        "width")
                  .toInt32();
        CPPUNIT_ASSERT_GREATER(sal_Int32(0), nWidth);
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf124423)
{
    createSwDoc("tdf124423.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFly1Width
        = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds", "width").toInt32();
    sal_Int32 nFly2Width
        = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds", "width").toInt32();
    sal_Int32 nPageWidth = getXPath(pXmlDoc, "//page/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_EQUAL(nPageWidth, nFly2Width);
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly1Width);

    createSwDoc("tdf124423.odt");
    pXmlDoc = parseLayoutDump();
    nFly1Width = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds", "width").toInt32();
    nFly2Width = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds", "width").toInt32();
    nPageWidth = getXPath(pXmlDoc, "//page/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly2Width);
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly1Width);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf137185)
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
    CPPUNIT_ASSERT_EQUAL(OUString("Align me!"), xTxt->getText()->getString());

    // Add a textbox to the shape
    SwTextBoxHelper::create(pShape, pShape->FindRealSdrObject(), true);

    // Check if the text moved from the shape to the frame
    auto pFormat = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));
    auto xTextFrame = SwXTextFrame::CreateXTextFrame(*pFormat->GetDoc(), pFormat);

    CPPUNIT_ASSERT_EQUAL(OUString("Align me!"), xTextFrame->getText()->getString());
    SdrTextObj* pTextObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT(pTextObj);
    const auto& aOutStr = pTextObj->GetOutlinerParaObject()->GetTextObject();

    CPPUNIT_ASSERT(aOutStr.GetText(0).isEmpty());
    // Before the patch it failed, because the text appeared 2 times on each other.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf138782)
{
    createSwDoc("tdf138782.docx");
    auto pXml = parseLayoutDump();
    CPPUNIT_ASSERT(pXml);

    // Without the fix it failed because the 3rd shape was outside the page:
    // - Expected less than: 13327
    // - Actual  : 14469

    CPPUNIT_ASSERT_LESS(
        getXPath(pXml, "/root/page/infos/bounds", "right").toInt32(),
        getXPath(pXml, "/root/page/body/txt[8]/anchored/SwAnchoredDrawObject/bounds", "right")
            .toInt32());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf135035)
{
    createSwDoc("tdf135035.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFly1Width
        = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds", "width").toInt32();
    sal_Int32 nFly2Width
        = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds", "width").toInt32();
    sal_Int32 nFly3Width
        = getXPath(pXmlDoc, "(//anchored/fly)[3]/infos/prtBounds", "width").toInt32();
    sal_Int32 nParentWidth = getXPath(pXmlDoc, "(//txt)[1]/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_EQUAL(nParentWidth, nFly2Width);
    CPPUNIT_ASSERT_EQUAL(nParentWidth, nFly3Width);
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly1Width);

    createSwDoc("tdf135035.odt");
    pXmlDoc = parseLayoutDump();
    nFly1Width = getXPath(pXmlDoc, "(//anchored/fly)[1]/infos/prtBounds", "width").toInt32();
    nFly2Width = getXPath(pXmlDoc, "(//anchored/fly)[2]/infos/prtBounds", "width").toInt32();
    nFly3Width = getXPath(pXmlDoc, "(//anchored/fly)[3]/infos/prtBounds", "width").toInt32();
    nParentWidth = getXPath(pXmlDoc, "(//txt)[1]/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly2Width);
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly1Width);
    CPPUNIT_ASSERT_GREATER(nParentWidth, nFly3Width);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf146704_EndnoteInSection)
{
    createSwDoc("tdf146704_EndnoteInSection.odt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the fix, the endnote placed to 2. page
    assertXPath(pXmlDoc, "/root/page", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf139336_ColumnsWithFootnoteDoNotOccupyEntirePage)
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
    assertXPath(pXmlDoc, "/root/page", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf54465_ColumnsWithFootnoteDoNotOccupyEntirePage)
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
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/root/page");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_GREATER(1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);

    discardDumpedLayout();
    createSwDoc("tdf54465_ColumnsWithFootnoteDoNotOccupyEntirePage_New.odt");
    pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf155324)
{
    createSwDoc("tox-update-wrong-pages.odt");

    dispatchCommand(mxComponent, ".uno:UpdateAllIndexes", {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // the problem was that the first entry was on page 7, 2nd on page 9 etc.
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "Foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "5");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "7");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", "Three");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/section[2]/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", "7");

    // check first content page has the footnotes
    assertXPath(pXmlDoc, "/root/page[5]/body/txt[1]/SwParaPortion/SwLineLayout", "portion", "Foo");
    assertXPath(pXmlDoc, "/root/page[4]/ftncont", 0);
    assertXPath(pXmlDoc, "/root/page[5]/ftncont/ftn", 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf156419)
{
    createSwDoc("linked_frames_section_bug.odt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 2);
    // there are 2 flys on page 1, and 1 on page 2, all linked
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[1]/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[1]/section/column[1]/body/txt", 11);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[1]/section/column[2]/body/txt", 11);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[2]/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[2]/section/column[1]/body/txt", 12);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly[2]/section/column[2]/body/txt", 12);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly[1]/section/column", 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly[1]/section/column[1]/body/txt", 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/fly[1]/section/column[2]/body/txt", 1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
