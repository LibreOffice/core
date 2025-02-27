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
#include <osl/process.h>
#include <comphelper/scopeguard.hxx>
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

#include <layouter.hxx>
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
class SwLayoutWriter : public SwModelTestBase
{
protected:
    void CheckRedlineFootnotesHidden();
    void CheckRedlineSectionsHidden();

    SwDoc* createDoc(const char* pName = nullptr);

    uno::Any executeMacro(const OUString& rScriptURL,
                          const uno::Sequence<uno::Any>& rParams = {});
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

uno::Any SwLayoutWriter::executeMacro(const OUString& rScriptURL,
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

// this is a member because our test classes have protected members :(
void SwLayoutWriter::CheckRedlineFootnotesHidden()
{
    discardDumpedLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf156077)
{
    createDoc("s4_min2.fodt");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of pages does not match!", 3, getPages());
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly", 3);
    assertXPath(pXmlDoc, "/root/page[2]/anchored/fly", 1);
    // this was 0, the at-page anchored flys were not displayed
    assertXPath(pXmlDoc, "/root/page[3]/anchored/fly", 3);
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

    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    discardDumpedLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();

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
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    discardDumpedLayout();
    CheckRedlineFootnotesHidden();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTableSplitBug)
{
    createDoc("table-split-bug.fodt");

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]//txt[1]/infos/bounds",
                    "height", u"276");
        assertXPath(pXmlDoc, "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]/infos/bounds",
                    "height", u"1274");
        discardDumpedLayout();
    }

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(false, 1);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Delete();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        // the problem was that the paragraph in the left cell had height 0
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]//txt[1]/infos/bounds",
                    "height", u"276");
        assertXPath(pXmlDoc, "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]/infos/bounds",
                    "height", u"1688");
        discardDumpedLayout();
    }

    pWrtShell->Undo();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        // the problem was that the paragraph in the left cell had height 0
        assertXPath(pXmlDoc,
                    "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]//txt[1]/infos/bounds",
                    "height", u"276");
        assertXPath(pXmlDoc, "/root/page[1]/body/section[3]/tab[1]/row[1]/cell[1]/infos/bounds",
                    "height", u"1274");
        discardDumpedLayout();
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTableInSectionTruncated)
{
    createDoc("table-in-section-truncated.fodt");

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt", 20);
        assertXPath(pXmlDoc, "/root/page[1]/body/section/txt", 0);
        assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row/cell/txt", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/row/cell/txt", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/section/txt", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt", 0);

        assertXPath(pXmlDoc, "/root/page[2]/body/section/infos/bounds", "bottom", "11032");
        discardDumpedLayout();
    }

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Down(false, 19);
    lcl_dispatchCommand(mxComponent, ".uno:InsertPagebreak", {});

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt", 20);
        assertXPath(pXmlDoc, "/root/page[1]/body/section/txt", 0);
        assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row/cell/txt", 0);
        assertXPath(pXmlDoc, "/root/page[2]/body/txt", 1);
        assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/row/cell/txt", 3);
        assertXPath(pXmlDoc, "/root/page[2]/body/section/txt", 1);

        assertXPath(pXmlDoc, "/root/page[2]/body/section/infos/bounds", "top", "10369");
        assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/infos/bounds", "top", "10369");
        assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/infos/bounds", "height", "940");
        assertXPath(pXmlDoc, "/root/page[2]/body/section/txt/infos/bounds", "bottom",
                    "11584");
        // problem was that the section bottom did not grow enough (only 11309)
        assertXPath(pXmlDoc, "/root/page[2]/body/section/infos/bounds", "bottom", "11584");
        discardDumpedLayout();
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTableInSectionSplitLoop)
{
    createDoc("table-in-section-split-loop.fodt");

    static OUString const var{ "TEST_NO_LOOP_CONTROLS" };
    osl_setEnvironment(var.pData, OUString("1").pData);
    comphelper::ScopeGuard g([] { osl_clearEnvironment(var.pData); });

    CPPUNIT_ASSERT_EQUAL(0, SwLayouter::GetLastLoopControlStage());

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Delete();

    CPPUNIT_ASSERT_EQUAL(0, SwLayouter::GetLastLoopControlStage());
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
    SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
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

    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "14");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "ahi");

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "14");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");

        { // hide: no anchored object shown
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//anchored");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "14");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "ahi");

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testFlyHiddenParagraph)
{
    SwDoc* pDoc = createDoc("fly_hidden_paragraph.fodt");
    CPPUNIT_ASSERT(pDoc);

    // first, disable both so para gets hidden
    uno::Sequence<beans::PropertyValue> argsSH(
        comphelper::InitPropertySequence({ { "ShowHiddenParagraphs", uno::Any(false) } }));
    lcl_dispatchCommand(mxComponent, ".uno:ShowHiddenParagraphs", argsSH);

    uno::Sequence<beans::PropertyValue> args(
        comphelper::InitPropertySequence({ { "Fieldnames", uno::Any(false) } }));

    lcl_dispatchCommand(mxComponent, ".uno:Fieldnames", args);
    Scheduler::ProcessEventsToIdle();

    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/txt/infos/bounds", "height", "0");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "height", "448");
    discardDumpedLayout();

    // the problem was that now the fly was the same height as before hiding
    lcl_dispatchCommand(mxComponent, ".uno:Fieldnames", {});
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/txt/infos/bounds", "height", "828");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "height", "1000");
    discardDumpedLayout();

    lcl_dispatchCommand(mxComponent, ".uno:Fieldnames", {});
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/txt/infos/bounds", "height", "0");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "height", "448");
    discardDumpedLayout();

    lcl_dispatchCommand(mxComponent, ".uno:Fieldnames", {});
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/txt/infos/bounds", "height", "828");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "height", "1000");
    discardDumpedLayout();

    // other test like testTdf143239 and testTdf159101 depend on this;
    // seems getting the previous value is only possible with a listener
    // so just hardcode it...
    lcl_dispatchCommand(mxComponent, ".uno:Fieldnames", args);
    Scheduler::ProcessEventsToIdle();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testFieldHideSection)
{
    SwDoc* pDoc = createDoc("field_hide_section.fodt");
    CPPUNIT_ASSERT(pDoc);

    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/row", 1);
    assertXPath(pXmlDoc, "/root/page", 2);
    discardDumpedLayout();

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    ::std::unique_ptr<SwField> pField(pWrtShell->GetCurField()->CopyField());
    SwFieldMgr manager(pWrtShell);

    pWrtShell->StartAllAction();
    manager.UpdateCurField(10000/*(?)*/, "Foo", "1", std::move(pField));
    pWrtShell->EndAllAction();
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row[1]/infos/bounds", "height", "0");
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row[2]/infos/bounds", "height", "0");
    assertXPath(pXmlDoc, "/root/page[1]/body/section/infos/bounds", "height", "0");
    // the problem was that there were 3 pages now
    assertXPath(pXmlDoc, "/root/page", 1);
    discardDumpedLayout();

    pWrtShell->StartAllAction();
    manager.UpdateCurField(10000/*(?)*/, "Foo", "0", std::move(pField));
    pWrtShell->EndAllAction();
    Scheduler::ProcessEventsToIdle();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/row", 1);
    assertXPath(pXmlDoc, "/root/page", 2);
    discardDumpedLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestI94666)
{
    SwDoc* pDoc = createDoc("i94666.odt");
    CPPUNIT_ASSERT(pDoc);

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page", 2);
        assertXPath(pXmlDoc, "/root/page[2]/body/section/txt[1]/Text[1]", "Portion", "pulled off ");
        discardDumpedLayout();
    }

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoPage(2, false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 11, /*bBasicCall=*/false);
    pWrtShell->SetMark();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 11, /*bBasicCall=*/false);
    pWrtShell->DelToEndOfPara();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        // the problem was that the last paragraph moved to page 3
        assertXPath(pXmlDoc, "/root/page[2]/body/section/txt[1]/Text[1]", "Portion", "Widows & orphans He heard quiet steps behind him. That didn't bode well. Who could be following");
        assertXPath(pXmlDoc, "/root/page[2]/body/section/txt[1]/Text[4]", "Portion", "pulled off ");
        assertXPath(pXmlDoc, "/root/page[2]/body/section/txt[2]/Text[1]", "Portion", "Moved paragraph");
        assertXPath(pXmlDoc, "/root/page[2]//txt", 3);
        assertXPath(pXmlDoc, "/root/page", 2);
        discardDumpedLayout();
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, TestTdf134272)
{
    SwDoc* pDoc = createDoc("tdf134472.odt");
    CPPUNIT_ASSERT(pDoc);
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/infos/bounds", "height", "843");
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
    pWrtShell->ChangeHeaderOrFooter("Default Style", /*bHeader*/ true, /*bOn*/ true, false);
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
    SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
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

    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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
    SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
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

    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();

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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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
    xmlDocPtr pLayout = parseLayoutDump();
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
    xmlDocPtr pLayout = parseLayoutDump();

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
    SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
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

    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "19");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");

        { // hide: no anchored object shown
            xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//anchored");
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
            CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
            xmlXPathFreeObject(pXmlObj);
        }

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
        CPPUNIT_ASSERT(pLayout->IsHideRedlines());
        discardDumpedLayout();
        xmlDocPtr pXmlDoc = parseLayoutDump();
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

        lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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
    SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
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

    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});
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

    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    discardDumpedLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
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

    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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

void SwLayoutWriter::CheckRedlineSectionsHidden()
{
    discardDumpedLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "folah");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/merged", "paraPropsNodeIndex", "20");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/Text[1]", "nType",
                "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/Text[1]", "Portion", "folah");
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

    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();

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
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    CheckRedlineSectionsHidden();
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
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foar");

    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
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
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foar");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf109137)
{
    createDoc("tdf109137.docx");
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
        { "FilterName", uno::Any(OUString("writer8")) },
    }));
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    loadURL(aTempFile.GetURL(), "tdf109137.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 0, the blue rectangle moved from the 1st to the 2nd page.
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly/notxt",
                /*nNumberOfNodes=*/1);
}

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint72) { createDoc("forcepoint72-1.rtf"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint75) { createDoc("forcepoint75-1.rtf"); }

// FIXME: apparently infinite loop on Mac
#ifndef MACOSX
//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testUXTSOREL) { createDoc("LIBREOFFICE-UXTSOREL.rtf"); }
#endif

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepointFootnoteFrame)
{
    createDoc("forcepoint-swfootnoteframe-1.rtf");
}

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint76)
{
    createDoc("forcepoint76-1.rtf");
}

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testN4LA0OHZ) { createDoc("LIBREOFFICE-N4LA0OHZ.rtf"); }

//just care it doesn't crash/assert
#if 0 // no createSwWebDoc
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint89)
{
    createSwWebDoc(DATA_DIRECTORY, "forcepoint89.html");
}
#endif

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint90)
{
    createDoc("forcepoint90.rtf");
}

//just care it doesn't crash/assert
#if 0 // no createSwWebDoc
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint91)
{
    createSwWebDoc(DATA_DIRECTORY, "forcepoint91.html");
}
#endif

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint92) { createDoc("forcepoint92.doc"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint93)
{
//fails to load    createDoc("forcepoint93-1.rtf");
//layout loop    createDoc("forcepoint93-2.rtf");
}

#if 0 // no createSwWebDoc
//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint94)
{
    createSwWebDoc(DATA_DIRECTORY, "forcepoint94.html");
}
#endif

//just care it doesn't crash/assert
#if 0 // no createSwWebDoc
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint98)
{
    createSwWebDoc(DATA_DIRECTORY, "forcepoint98.html");
}
#endif

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint102)
{
    createDoc("forcepoint102.rtf");
}

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf147485Forcepoint)
{
    createDoc("tdf147485-forcepoint.doc");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf118058)
{
    SwDoc* pDoc = createDoc("tdf118058.fodt");
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

//just care it doesn't crash/assert
#if 0 // no createSwWebDoc
CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testForcepoint99)
{
    createSwWebDoc(DATA_DIRECTORY, "forcepoint99.html");
}
#endif

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testTdf128611)
{
    createDoc("tdf128611.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 14
    // i.e. there were multiple portions in the first paragraph of the A1 cell, which means that the
    // rotated text was broken into multiple lines without a good reason.
    assertXPath(pXmlDoc, "//tab/row/cell[1]/txt/Text", "Portion", "Abcd efghijkl");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testKeepWithNextPlusFlyFollowTextFlow)
{
    createDoc("keep-with-next-fly.fodt");

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
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

    lcl_dispatchCommand(mxComponent, ".uno:Fieldnames", {});
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
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

    lcl_dispatchCommand(mxComponent, ".uno:Fieldnames", {});
    Scheduler::ProcessEventsToIdle();

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testBtlrTableRowSpan)
{
    // Load a document which has a table. The A1 cell has btlr text direction, and the A1..A3 cells
    // are merged.
    load(DATA_DIRECTORY, "btlr-table-row-span.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump aDumper;
    xmlDocPtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: USA
    // - Actual  : West
    // i.e. the "USA" text completely disappeared.
    assertXPathContent(pXmlDoc, "//textarray[1]/text", "USA");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter, testContinuousEndnotesMoveBackwards)
{
    // Load a document with the ContinuousEndnotes flag turned on.
    load(DATA_DIRECTORY, "continuous-endnotes-move-backwards.doc");
    xmlDocPtr pLayout = parseLayoutDump();
    // We have 2 pages.
    assertXPath(pLayout, "/root/page", 2);
    // No endnote container on page 1.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. there were unexpected endnotes on page 1.
    assertXPath(pLayout, "/root/page[1]/ftncont", 0);
    // All endnotes are in a container on page 2.
    assertXPath(pLayout, "/root/page[2]/ftncont", 1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
