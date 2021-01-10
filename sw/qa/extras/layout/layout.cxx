/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <comphelper/scopeguard.hxx>

#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtcntnt.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <rootfrm.hxx>
#include <docsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <textboxhelper.hxx>
#include <unoframe.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/layout/data/";

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter : public SwModelTestBase
{
protected:
    void CheckRedlineFootnotesHidden();
    void CheckRedlineSectionsHidden();

    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* SwLayoutWriter::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

// this is a member because our test classes have protected members :(
void SwLayoutWriter::CheckRedlineFootnotesHidden()
{
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "24");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "nType", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "rText", "2");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex", "13");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                "PortionType::FootnoteNum");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[1]", "Portion", "ac");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/merged", "paraPropsNodeIndex", "16");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "nType",
                "PortionType::FootnoteNum");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "rText", "2");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "Portion", "mo");
}

void SwLayoutWriter::CheckRedlineSectionsHidden()
{
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "folah");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/merged", "paraPropsNodeIndex", "20");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/Text[1]", "nType",
                "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/Text[1]", "Portion", "folah");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFootnotes)
{
    createDoc("redline_footnotes.odt");
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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "nType", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "rText", "2");

    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                "PortionType::FootnoteNum");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[1]", "Portion", "a");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[2]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[2]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[3]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[3]", "Portion", "c");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "nType",
                "PortionType::FootnoteNum");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "rText", "2");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "Portion", "def");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Special[1]", "nType", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Special[1]", "rText", "3");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "Portion", "ar");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[3]/txt[1]/Special[1]", "nType",
                "PortionType::FootnoteNum");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[3]/txt[1]/Special[1]", "rText", "3");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[3]/txt[1]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[3]/txt[1]/Text[1]", "Portion", "ghi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Special[1]", "nType", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Special[1]", "rText", "4");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Special[2]", "nType", "PortionType::Footnote");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Special[2]", "rText", "5");

    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[4]/txt[1]/Special[1]", "nType",
                "PortionType::FootnoteNum");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[4]/txt[1]/Special[1]", "rText", "4");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[4]/txt[1]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[4]/txt[1]/Text[1]", "Portion", "jkl");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Special[1]", "nType",
                "PortionType::FootnoteNum");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Special[1]", "rText", "5");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[1]", "Portion", "m");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[2]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[2]", "Portion", "n");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[3]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[3]", "Portion", "o");

    // verify after hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    discardDumpedLayout();
    CheckRedlineFootnotesHidden();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf136588)
{
    load(DATA_DIRECTORY, "tdf136588.docx");
    auto pXMLLayoutDump = parseLayoutDump();
    CPPUNIT_ASSERT(pXMLLayoutDump);

    //there was a bad line break before, the correct break layout is this:
    assertXPath(pXMLLayoutDump, "/root/page/body/txt[2]/LineBreak[2]", "Line",
                "effectively by modern-day small to ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysInBody)
{
    loadURL("private:factory/swriter", nullptr);
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
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    pWrtShell->StartOfSection(false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
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
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->SttEndDoc(true); // note: SttDoc actually moves to start of fly?
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "14");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "ahi");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "a");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "bc");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "def");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "g");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "hi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
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
            SwPosition pos(*anchor.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "14");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "a");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "bc");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "def");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "g");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "hi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
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
            SwPosition pos(*anchor.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "14");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "ahi");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "a");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "bc");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "def");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "g");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "hi");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf134272)
{
    SwDoc* pDoc = createDoc("tdf134472.odt");
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/infos/bounds", "height", "843");
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/infos/bounds", "bottom", "2819");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf136613)
{
    SwDoc* pDoc = createDoc("tdf136613.docx");
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
        CPPUNIT_ASSERT_MESSAGE("The pictures are outside the page!", rPageRect.IsInside(rRect));
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf88496)
{
    SwDoc* pDoc = createDoc("tdf88496.docx");
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 4, table fallback "switch off repeating header" started on a new page
    assertXPath(pXmlDoc, "/root/page", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysInHeader)
{
    loadURL("private:factory/swriter", nullptr);
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
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    pWrtShell->StartOfSection(false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
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
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->GotoHeaderText();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "11");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "ahi");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "Portion", "az");
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
            SwPosition pos(*anchor.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        // now the frame has no Text portion? not sure why it's a 0-length one first and now none?
        //        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        //        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "foaz");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "Portion", "az");
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
            SwPosition pos(*anchor.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "11");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "ahi");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "Portion", "az");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf137025)
{
    // Check the padding of the textbox
    SwDoc* pDoc = createDoc("tdf137025.docx");
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the layout xml
    // SDRATTR_TEXT_LEFTDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item[@whichId='1071']",
                "value", "567");
    // SDRATTR_TEXT_RIGHTDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item[@whichId='1072']",
                "value", "1134");
    // SDRATTR_TEXT_UPPERDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item[@whichId='1073']",
                "value", "1701");
    // SDRATTR_TEXT_LOWERDIST
    assertXPath(pXmlDoc,
                "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObject"
                "/DefaultProperties/SfxItemSet/SdrMetricItem/SfxInt32Item[@whichId='1074']",
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
    uno::Reference<beans::XPropertySet> xFrameProps(xTxFrm, uno::UNO_QUERY);

    const tools::Long nFrameLeftPaddng
        = xFrameProps->getPropertyValue("LeftBorderDistance").get<tools::Long>();
    const tools::Long nFrameRightPaddng
        = xFrameProps->getPropertyValue("RightBorderDistance").get<tools::Long>();
    const tools::Long nFrameTopPaddng
        = xFrameProps->getPropertyValue("TopBorderDistance").get<tools::Long>();
    const tools::Long nFrameBottomPaddng
        = xFrameProps->getPropertyValue("BottomBorderDistance").get<tools::Long>();

    // Check if the shape and frame have different setting
    CPPUNIT_ASSERT_EQUAL(nLPaddng, nFrameLeftPaddng);
    CPPUNIT_ASSERT_EQUAL(nRPaddng, nFrameRightPaddng);
    CPPUNIT_ASSERT_EQUAL(nTPaddng, nFrameTopPaddng);
    CPPUNIT_ASSERT_EQUAL(nBPaddng, nFrameBottomPaddng);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysInFootnote)
{
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->InsertFootnote("");
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());

    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
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
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
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
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    //    pWrtShell->GotoFlyAnchor(); // sigh... why, now we're in the body...
    pWrtShell->SttEndDoc(false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->GotoFootnoteText();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();
    pWrtShell->EndSelect(); // ?
    // delete first footnote
    pWrtShell->SttEndDoc(true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Delete();

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "25");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType",
                    "PortionType::Footnote");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    "7");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "17");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "ahi");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "PortionType::FootnoteNum");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType",
                    "PortionType::Footnote");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "nType",
                    "PortionType::Footnote");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "quux");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "PortionType::FootnoteNum");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/Text[2]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[2]/Text[1]",
                    "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/Text[1]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/Text[2]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "nType",
                    "PortionType::FootnoteNum");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "Portion", "az");
    }

    // anchor to 2nd (deleted) paragraph
    pWrtShell->SttEndDoc(false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
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
            SwPosition pos(*anchor.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "25");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType",
                    "PortionType::Footnote");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    "7");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "PortionType::FootnoteNum");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType",
                    "PortionType::Footnote");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "nType",
                    "PortionType::Footnote");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "quux");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "PortionType::FootnoteNum");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "nType",
                    "PortionType::FootnoteNum");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/Text[2]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[2]/Text[1]",
                    "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/Text[1]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/Text[2]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "Portion", "az");
    }

    // anchor to 3rd paragraph
    pWrtShell->EndOfSection();
    pWrtShell->SttEndDoc(false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
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
            SwPosition pos(*anchor.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor.SetAnchor(&pos);
            pDoc->SetAttr(anchor, *const_cast<SwFrameFormat*>(pFly));
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "25");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType",
                    "PortionType::Footnote");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    "7");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "17");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "ahi");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "PortionType::FootnoteNum");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType",
                    "PortionType::Footnote");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "nType",
                    "PortionType::Footnote");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "quux");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "PortionType::FootnoteNum");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "nType",
                    "PortionType::FootnoteNum");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/Text[2]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[2]/Text[1]",
                    "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/Text[1]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/Text[2]",
                    "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "Portion", "az");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTableOverlapFooterFly)
{
    // Load a document that has a fly anchored in the footer.
    // It also has a table which initially overlaps with the fly, but then moves to the next page.
    load(DATA_DIRECTORY, "footer-fly-table.fodt");
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // no fly portions, was: 8
    assertXPath(
        pLayout,
        "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/Special[@nType='PortionType::Fly']", 0);
    // one line break, was: 5
    assertXPath(pLayout, "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/LineBreak", 1);
    // one text portion, was: 1
    assertXPath(pLayout, "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/Text", 1);
    assertXPath(pLayout, "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/Text", "Portion",
                "Abc def ghi jkl mno pqr stu vwx yz.");
    // height was: 1517
    // tdf#134782 height was: 379
    assertXPath(pLayout, "/root/page[2]/body/tab[1]/row[5]/cell[5]/txt[1]/infos/bounds", "height",
                "253");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf134277)
{
    SwDoc* pDoc = createDoc("tdf134277.docx");
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
    SwDoc* pDoc = createDoc("tdf116486.docx");
    CPPUNIT_ASSERT(pDoc);
    OUString aTop = parseDump("/root/page/body/txt/Special", "nHeight");
    CPPUNIT_ASSERT_EQUAL(OUString("4006"), aTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf128198)
{
    SwDoc* pDoc = createDoc("tdf128198-1.docx");
    CPPUNIT_ASSERT(pDoc);
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // the problem was that line 5 was truncated at "this  "
    // due to the fly anchored in previous paragraph
    assertXPath(pLayout, "/root/page/body/txt[2]/LineBreak[5]", "Line",
                "to access any service, any time, anywhere. From this  perspective, satellite "
                "boasts some ");
    assertXPath(pLayout, "/root/page/body/txt[2]/LineBreak[6]", "Line", "significant advantages. ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testNoLineBreakAtSlash)
{
    load(DATA_DIRECTORY, "no-line-break-at-slash.fodt");
    xmlDocUniquePtr pLayout = parseLayoutDump();

    // the line break was between  "Foostrasse 13/c/" and "2"
    xmlXPathObjectPtr pXmlObj = getXPathNode(pLayout, "/root/page[1]/body/txt[1]/child::*[2]");
    CPPUNIT_ASSERT_EQUAL(std::string("Text"), std::string(reinterpret_cast<char const*>(
                                                  pXmlObj->nodesetval->nodeTab[0]->name)));
    xmlXPathFreeObject(pXmlObj);
    pXmlObj = getXPathNode(pLayout, "/root/page[1]/body/txt[1]/child::*[3]");
    CPPUNIT_ASSERT_EQUAL(std::string("LineBreak"), std::string(reinterpret_cast<char const*>(
                                                       pXmlObj->nodesetval->nodeTab[0]->name)));
    xmlXPathFreeObject(pXmlObj);
    pXmlObj = getXPathNode(pLayout, "/root/page[1]/body/txt[1]/child::*[4]");
    CPPUNIT_ASSERT_EQUAL(std::string("Text"), std::string(reinterpret_cast<char const*>(
                                                  pXmlObj->nodesetval->nodeTab[0]->name)));
    xmlXPathFreeObject(pXmlObj);
    pXmlObj = getXPathNode(pLayout, "/root/page[1]/body/txt[1]/child::*[5]");
    CPPUNIT_ASSERT_EQUAL(std::string("Special"), std::string(reinterpret_cast<char const*>(
                                                     pXmlObj->nodesetval->nodeTab[0]->name)));
    xmlXPathFreeObject(pXmlObj);
    pXmlObj = getXPathNode(pLayout, "/root/page[1]/body/txt[1]/child::*[6]");
    CPPUNIT_ASSERT_EQUAL(std::string("Text"), std::string(reinterpret_cast<char const*>(
                                                  pXmlObj->nodesetval->nodeTab[0]->name)));
    xmlXPathFreeObject(pXmlObj);
    pXmlObj = getXPathNode(pLayout, "/root/page[1]/body/txt[1]/child::*[7]");
    CPPUNIT_ASSERT_EQUAL(std::string("LineBreak"), std::string(reinterpret_cast<char const*>(
                                                       pXmlObj->nodesetval->nodeTab[0]->name)));
    xmlXPathFreeObject(pXmlObj);
    pXmlObj = getXPathNode(pLayout, "/root/page[1]/body/txt[1]/child::*[8]");
    CPPUNIT_ASSERT_EQUAL(std::string("Finish"), std::string(reinterpret_cast<char const*>(
                                                    pXmlObj->nodesetval->nodeTab[0]->name)));
    xmlXPathFreeObject(pXmlObj);

    assertXPath(pLayout, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "Blah blah bla bla bla ");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "Foostrasse");
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/Text[3]", "Portion", "13/c/2, etc.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf106153)
{
    load(DATA_DIRECTORY, "tdf106153.docx");
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
    loadURL("private:factory/swriter", nullptr);
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
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
    SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    pWrtShell->StartOfSection(false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
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
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // delete redline inside fly1
    pWrtShell->GotoFly(pFly1->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->ClearMark(); // otherwise it refuses to leave the fly...
    pWrtShell->SttEndDoc(true); // note: SttDoc actually moves to start of fly?
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    for (int i = 0; i < 2; ++i)
    {
        if (i == 1) // secondly, try with different anchor type
        {
            anchor1.SetType(RndStdIds::FLY_AT_PARA);
            SwPosition pos(*anchor1.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor1.SetAnchor(&pos);
            pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));
            anchor2.SetType(RndStdIds::FLY_AT_PARA);
            pos.nNode = anchor2.GetContentAnchor()->nNode;
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
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "nType", "PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "Portion", "jqr");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                    "PortionType::Fly"); // remove???
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Lay");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "abhi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");

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

        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "Portion", "j");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "Portion", "kl");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "nType", "PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "Portion", "mno");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "Portion", "p");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "Portion", "qr");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                    "PortionType::Fly"); // remove???
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "ab");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "c");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "def");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "g");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "hi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
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
            SwPosition pos(*anchor1.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor1.SetAnchor(&pos);
            pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));
            anchor2.SetType(RndStdIds::FLY_AT_PARA);
            pos.nNode = anchor2.GetContentAnchor()->nNode;
            anchor2.SetAnchor(&pos);
            pDoc->SetAttr(anchor2, *const_cast<SwFrameFormat*>(pFly2));
        }

        dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "19");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "j");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "kl");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "Portion", "ab");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "Portion", "c");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "nType", "PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "Portion", "def");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "Portion", "g");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Special[1]", "nType",
                    "PortionType::Fly"); // remove???
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "PortionType::Lay");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "mno");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "p");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "qr");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
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
            SwPosition pos(*anchor1.GetContentAnchor());
            pos.nContent.Assign(nullptr, 0);
            anchor1.SetAnchor(&pos);
            pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));
            anchor2.SetType(RndStdIds::FLY_AT_PARA);
            pos.nNode = anchor2.GetContentAnchor()->nNode;
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
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "nType", "PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "Portion", "jqr");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                    "PortionType::Fly"); // remove???
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Lay");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "abhi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "ab");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "c");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "def");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "Portion", "j");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "Portion", "kl");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "nType", "PortionType::Para");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "Portion", "mno");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "Portion", "p");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "nType", "PortionType::Text");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "Portion", "qr");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Special[1]", "nType",
                    "PortionType::Fly"); // remove???
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "g");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "hi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineFlysAtFlys)
{
    loadURL("private:factory/swriter", nullptr);
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
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
    SwFormatFrameSize size(SwFrameSize::Minimum, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    pWrtShell->StartOfSection(false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
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
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // delete redline inside fly1
    pWrtShell->GotoFly(pFly1->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->ClearMark(); // otherwise it refuses to leave the fly...
    pWrtShell->SttEndDoc(true); // note: SttDoc actually moves to start of fly?
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 7, /*bBasicCall=*/false);
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
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
                "nType", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
                "Portion", "jqr");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                "PortionType::Fly"); // remove???
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                "PortionType::Lay");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                "abhi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");

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
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
                "nType", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
                "Portion", "j");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
                "nType", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
                "Portion", "kl");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
                "nType", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
                "Portion", "mno");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
                "nType", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
                "Portion", "p");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
                "nType", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
                "Portion", "qr");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                "PortionType::Fly"); // remove???
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                "ab");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                "c");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                "def");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                "g");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                "hi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");

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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");

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

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
                "nType", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
                "Portion", "j");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
                "nType", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
                "Portion", "kl");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
                "nType", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
                "Portion", "mno");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
                "nType", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
                "Portion", "p");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
                "nType", "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
                "Portion", "qr");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                "PortionType::Fly"); // remove???
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                "ab");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                "c");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                "def");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                "g");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                "hi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineSections)
{
    createDoc("redline_sections.fodt");
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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");

    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                "Portion", "FRAME");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/Text[1]", "nType",
                "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/Text[1]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[2]/Text[1]", "nType",
                "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[2]/Text[1]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "Portion", "lah");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[1]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[1]/Text[2]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[1]/Text[2]", "Portion", "o");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[2]/Text[1]", "nType",
                "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[2]/Text[1]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[3]/Text[1]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[3]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[3]/Text[2]", "nType",
                "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[3]/Text[2]", "Portion", "lah");

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
    createDoc("tdf69647_images.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of pages does not match!", 2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TDF69647_text)
{
    createDoc("tdf69647_text.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of pages does not match!", 2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testRedlineTables)
{
    createDoc("redline_table.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // verify after load
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foar");

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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");

    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                "nType", "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                "Portion", "FRAME");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/Text[1]", "nType",
                "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/Text[1]", "Portion",
                "aaa");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[2]/cell[2]/txt[1]/Text[1]", "nType",
                "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[2]/cell[2]/txt[1]/Text[1]", "Portion",
                "ddd");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "Portion", "ar");

    // verify after hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foar");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
