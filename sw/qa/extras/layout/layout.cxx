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
#include <test/mtfxmldump.hxx>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <officecfg/Office/Common.hxx>
#include <comphelper/scopeguard.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtcntnt.hxx>
#include <wrtsh.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/extras/layout/data/";

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter : public SwModelTestBase
{
    void CheckRedlineFootnotesHidden();
    void CheckRedlineSectionsHidden();
    void CheckRedlineCharAttributesHidden();

public:
    void testRedlineFootnotes();
    void testRedlineFlysInBody();
    void testRedlineFlysInHeader();
    void testRedlineFlysInFootnote();
    void testRedlineFlysInFlys();
    void testRedlineFlysAtFlys();
    void testRedlineSections();
    void testRedlineTables();
    void testRedlineCharAttributes();
    void testTdf116830();
    void testTdf116925();
    void testTdf117028();
    void testTdf106390();
    void testTableExtrusion1();
    void testTableExtrusion2();
    void testTdf116848();
    void testTdf117245();
    void testTdf118672();
    void testTdf117923();
    void testTdf109077();
    void testUserFieldTypeLanguage();
    void testTdf109137();
    void testForcepoint72();
    void testTdf118058();
    void testTdf117188();
    void testTdf117187();
    void testTdf119875();
    void testTdf120287();
    void testTdf120287b();
    void testTdf120287c();

    CPPUNIT_TEST_SUITE(SwLayoutWriter);
    CPPUNIT_TEST(testRedlineFootnotes);
    CPPUNIT_TEST(testRedlineFlysInBody);
    CPPUNIT_TEST(testRedlineFlysInHeader);
    CPPUNIT_TEST(testRedlineFlysInFootnote);
    CPPUNIT_TEST(testRedlineFlysInFlys);
    CPPUNIT_TEST(testRedlineFlysAtFlys);
    CPPUNIT_TEST(testRedlineSections);
    CPPUNIT_TEST(testRedlineTables);
    CPPUNIT_TEST(testRedlineCharAttributes);
    CPPUNIT_TEST(testTdf116830);
    CPPUNIT_TEST(testTdf116925);
    CPPUNIT_TEST(testTdf117028);
    CPPUNIT_TEST(testTdf106390);
    CPPUNIT_TEST(testTableExtrusion1);
    CPPUNIT_TEST(testTableExtrusion2);
    CPPUNIT_TEST(testTdf116848);
    CPPUNIT_TEST(testTdf117245);
    CPPUNIT_TEST(testTdf118672);
    CPPUNIT_TEST(testTdf117923);
    CPPUNIT_TEST(testTdf109077);
    CPPUNIT_TEST(testUserFieldTypeLanguage);
    CPPUNIT_TEST(testTdf109137);
    CPPUNIT_TEST(testForcepoint72);
    CPPUNIT_TEST(testTdf118058);
    CPPUNIT_TEST(testTdf117188);
    CPPUNIT_TEST(testTdf117187);
    CPPUNIT_TEST(testTdf119875);
    CPPUNIT_TEST(testTdf120287);
    CPPUNIT_TEST(testTdf120287b);
    CPPUNIT_TEST(testTdf120287c);
    CPPUNIT_TEST_SUITE_END();

private:
    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* SwLayoutWriter::createDoc(const char* pName)
{
    load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

static void lcl_dispatchCommand(const uno::Reference<lang::XComponent>& xComponent,
                                const OUString& rCommand,
                                const uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    uno::Reference<frame::XController> xController
        = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY)->getCurrentController();
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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType", "POR_FTN");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "nType", "POR_FTN");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "rText", "5"); // TODO 2
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex", "13");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType", "POR_FTNNUM");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[1]", "Portion", "ac");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/merged", "paraPropsNodeIndex", "16");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "nType", "POR_FTNNUM");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "rText", "5"); // TODO 2
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "Portion", "mo");
}

void SwLayoutWriter::testRedlineFootnotes()
{
    // currently need experimental mode
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(false, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(true, pBatch);
    pBatch->commit();

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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType", "POR_FTN");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "nType", "POR_FTN");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "rText", "2");

    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType", "POR_FTNNUM");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[1]", "Portion", "a");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[2]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Text[3]", "Portion", "c");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "nType", "POR_FTNNUM");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "rText", "2");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "Portion", "def");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Special[1]", "nType", "POR_FTN");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Special[1]", "rText", "3");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "Portion", "ar");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[3]/txt[1]/Special[1]", "nType", "POR_FTNNUM");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[3]/txt[1]/Special[1]", "rText", "3");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[3]/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[3]/txt[1]/Text[1]", "Portion", "ghi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Special[1]", "nType", "POR_FTN");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Special[1]", "rText", "4");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Special[2]", "nType", "POR_FTN");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Special[2]", "rText", "5");

    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[4]/txt[1]/Special[1]", "nType", "POR_FTNNUM");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[4]/txt[1]/Special[1]", "rText", "4");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[4]/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[4]/txt[1]/Text[1]", "Portion", "jkl");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Special[1]", "nType", "POR_FTNNUM");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Special[1]", "rText", "5");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[1]", "Portion", "m");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[2]", "Portion", "n");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[5]/txt[1]/Text[3]", "Portion", "o");

    // verify after hide
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    discardDumpedLayout();
    CheckRedlineFootnotesHidden();
}

void SwLayoutWriter::testRedlineFlysInBody()
{
    // currently need experimental mode
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(false, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(true, pBatch);
    pBatch->commit();

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
    pWrtShell->SttDoc(false);
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
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_PARA");
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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "a");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "bc");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "def");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "g");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "hi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
    }

    // anchor to 2nd (deleted) paragraph
    pWrtShell->SttDoc();
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
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "a");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "bc");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "def");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "g");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "hi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
    }

    // anchor to 3rd paragraph
    pWrtShell->EndDoc();
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
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_PARA");
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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "a");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "bc");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "def");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "g");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "hi");
    }
}

void SwLayoutWriter::testRedlineFlysInHeader()
{
    // currently need experimental mode
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(false, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(true, pBatch);
    pBatch->commit();

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
    pWrtShell->SttDoc(false);
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
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "11");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_PARA");
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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "Portion", "az");
    }

    // anchor to 2nd (deleted) paragraph
    pWrtShell->SttDoc();
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
        //        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
        //        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "POR_PARA");
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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "Portion", "az");
    }

    // anchor to 3rd paragraph
    pWrtShell->EndDoc();
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
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/merged", "paraPropsNodeIndex", "6");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "foaz");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "11");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_PARA");
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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "0");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/Text[2]", "Portion", "az");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/header/txt[3]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
    }
}

void SwLayoutWriter::testRedlineFlysInFootnote()
{
    // currently need experimental mode
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(false, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(true, pBatch);
    pBatch->commit();

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

    pWrtShell->SttDoc(false);
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
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType", "POR_FTN");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    "7");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "17");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "ahi");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "POR_FTNNUM");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "2");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType", "POR_FTN");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "nType", "POR_FTN");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "quux");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "POR_FTNNUM");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/Text[2]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[2]/Text[1]",
                    "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/Text[1]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/Text[2]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "nType",
                    "POR_FTNNUM");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "nType", "POR_TXT");
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
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType", "POR_FTN");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    "7");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "POR_FTNNUM");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "2");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType", "POR_FTN");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "nType", "POR_FTN");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "quux");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "POR_FTNNUM");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "nType",
                    "POR_FTNNUM");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/Text[2]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[2]/Text[1]",
                    "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/Text[1]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/Text[2]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "Portion", "az");
    }

    // anchor to 3rd paragraph
    pWrtShell->EndDoc();
    pWrtShell->SttEndDoc(false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->GotoFootnoteText();
    pWrtShell->EndDoc();
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
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType", "POR_FTN");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/merged", "paraPropsNodeIndex",
                    "7");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/merged",
                    "paraPropsNodeIndex", "17");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "ahi");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "POR_FTNNUM");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "2");

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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "nType", "POR_FTN");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "nType", "POR_FTN");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Special[2]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "quux");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "nType",
                    "POR_FTNNUM");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[1]/txt[1]/Special[1]", "rText", "1");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "nType",
                    "POR_FTNNUM");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Special[1]", "rText", "2");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/Text[1]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/Text[1]",
                    "Portion", "a");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/Text[2]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[1]/Text[2]",
                    "Portion", "bc");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[2]/Text[1]",
                    "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[2]/Text[1]",
                    "Portion", "def");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/Text[1]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/Text[1]",
                    "Portion", "g");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/Text[2]",
                    "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/anchored/fly[1]/txt[3]/Text[2]",
                    "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn[2]/txt[3]/Text[2]", "Portion", "az");
    }
}

void SwLayoutWriter::testRedlineFlysInFlys()
{
    // currently need experimental mode
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(false, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(true, pBatch);
    pBatch->commit();

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
    pWrtShell->SttDoc(false);
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
    pWrtShell->SttDoc(false); // start of fly...
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
            "nType", "POR_PARA");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "Portion", "jqr");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                    "POR_FLY"); // remove???
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_LAY");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "abhi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
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
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "Portion", "j");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "Portion", "kl");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "nType", "POR_PARA");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "Portion", "mno");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "Portion", "p");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "Portion", "qr");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                    "POR_FLY"); // remove???
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "ab");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "c");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "def");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "g");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "hi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
    }

    // anchor to 2nd (deleted) paragraph
    // also, switch the in-fly anchoring to the other fly, for additional fun!
    pWrtShell->SttDoc();
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
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "j");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "kl");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "Portion", "ab");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "Portion", "c");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "nType", "POR_PARA");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "Portion", "def");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "Portion", "g");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "Portion", "hi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Special[1]", "nType",
                    "POR_FLY"); // remove???
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "POR_LAY");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "mno");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "p");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "qr");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
    }

    // anchor to 3rd paragraph
    pWrtShell->SttEndDoc(false);
    anchor1.SetType(RndStdIds::FLY_AT_CHAR);
    anchor1.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));
    pWrtShell->GotoFly(pFly1->GetName(), FLYCNTTYPE_FRM, /*bSelFrame=*/false);
    pWrtShell->EndDoc();
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
            "nType", "POR_PARA");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "Portion", "jqr");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                    "POR_FLY"); // remove???
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_LAY");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "abhi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
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

        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                    "ab");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                    "c");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                    "POR_PARA");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                    "def");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/Text[1]",
            "Portion", "j");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[1]/Text[2]",
            "Portion", "kl");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "nType", "POR_PARA");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[2]/Text[1]",
            "Portion", "mno");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/Text[1]",
            "Portion", "p");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "nType", "POR_TXT");
        assertXPath(
            pXmlDoc,
            "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/anchored[1]/fly[1]/txt[3]/Text[2]",
            "Portion", "qr");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Special[1]", "nType",
                    "POR_FLY"); // remove???
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                    "g");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                    "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                    "hi");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
    }
}

void SwLayoutWriter::testRedlineFlysAtFlys()
{
    // currently need experimental mode
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(false, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(true, pBatch);
    pBatch->commit();

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
    pWrtShell->SttDoc(false);
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
                "nType", "POR_PARA");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
                "Portion", "jqr");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                "POR_FLY"); // remove???
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                "POR_LAY");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                "abhi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
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
                "nType", "POR_TXT");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
                "Portion", "j");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
                "nType", "POR_TXT");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
                "Portion", "kl");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
                "nType", "POR_PARA");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
                "Portion", "mno");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
                "nType", "POR_TXT");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
                "Portion", "p");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
                "nType", "POR_TXT");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
                "Portion", "qr");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                "POR_FLY"); // remove???
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                "ab");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                "c");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                "def");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                "g");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                "hi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");

    // anchor to 2nd (deleted) paragraph
    pWrtShell->SttDoc();
    pWrtShell->Down(false, 1);
    anchor1.SetType(RndStdIds::FLY_AT_CHAR);
    anchor1.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    pDoc->SetAttr(anchor1, *const_cast<SwFrameFormat*>(pFly1));

    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "19");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
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

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
                "nType", "POR_TXT");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[1]",
                "Portion", "j");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
                "nType", "POR_TXT");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[1]/Text[2]",
                "Portion", "kl");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
                "nType", "POR_PARA");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[2]/Text[1]",
                "Portion", "mno");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
                "nType", "POR_TXT");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[1]",
                "Portion", "p");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
                "nType", "POR_TXT");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/anchored/fly[1]/anchored[1]/fly[1]/txt[3]/Text[2]",
                "Portion", "qr");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "nType",
                "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Special[1]", "nType",
                "POR_FLY"); // remove???
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[1]", "Portion",
                "ab");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "nType",
                "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[1]/Text[2]", "Portion",
                "c");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "nType",
                "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[2]/Text[1]", "Portion",
                "def");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "nType",
                "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[1]", "Portion",
                "g");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "nType",
                "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly[1]/txt[3]/Text[2]", "Portion",
                "hi");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "az");
}

void SwLayoutWriter::CheckRedlineSectionsHidden()
{
    discardDumpedLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "folah");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/merged", "paraPropsNodeIndex", "20");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/Text[1]", "Portion", "folah");
}

void SwLayoutWriter::testRedlineSections()
{
    // currently need experimental mode
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(false, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(true, pBatch);
    pBatch->commit();

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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");

    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                "Portion", "FRAME");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/Text[1]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[2]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[2]/Text[1]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "Portion", "lah");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[1]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[1]/Text[2]", "Portion", "o");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[2]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[2]/Text[1]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[3]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[3]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[3]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt[3]/Text[2]", "Portion", "lah");

    // verify after hide
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    CheckRedlineSectionsHidden();
}

void SwLayoutWriter::testRedlineTables()
{
    // currently need experimental mode
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(false, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(true, pBatch);
    pBatch->commit();

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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "o");

    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                "nType", "POR_PARA");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/anchored/fly[1]/txt[1]/Text[1]",
                "Portion", "FRAME");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/Text[1]", "nType",
                "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/Text[1]", "Portion",
                "aaa");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[2]/cell[2]/txt[1]/Text[1]", "nType",
                "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[2]/cell[2]/txt[1]/Text[1]", "Portion",
                "ddd");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "Portion", "ar");

    // verify after hide
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foar");
}

// this mainly tests that splitting portions across redlines in SwAttrIter works
void SwLayoutWriter::CheckRedlineCharAttributesHidden()
{
    discardDumpedLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "9");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/merged", "paraPropsNodeIndex", "10");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/merged", "paraPropsNodeIndex", "11");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/merged", "paraPropsNodeIndex", "13");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/merged", "paraPropsNodeIndex", "14");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/merged", "paraPropsNodeIndex", "15");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/merged", "paraPropsNodeIndex", "16");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/merged", "paraPropsNodeIndex", "17");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[1]", "Portion", "foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/merged", "paraPropsNodeIndex", "18");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[2]", "Portion", "ob");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[3]", "Portion", "az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/merged", "paraPropsNodeIndex", "19");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[1]", "nType", "POR_PARA");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[1]", "Portion", "foobaz");
}

void SwLayoutWriter::testRedlineCharAttributes()
{
    // currently need experimental mode
    Resetter _([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::ExperimentalMode::set(false, pBatch);
        return pBatch->commit();
    });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::ExperimentalMode::set(true, pBatch);
    pBatch->commit();

    createDoc("redline_charatr.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // verify after load
    CheckRedlineCharAttributesHidden();

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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[2]", "Portion", "ba");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[3]", "Portion", "r");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[4]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[4]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[2]", "Portion", "o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[3]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[4]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[4]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[5]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[5]", "Portion", "az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[1]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[2]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[2]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[3]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[3]", "Portion", "a");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[4]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[4]", "Portion", "r");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[5]", "nType", "POR_TXT");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[5]", "Portion", "baz");

    // verify after hide
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    CheckRedlineCharAttributesHidden();
}

void SwLayoutWriter::testTdf116830()
{
    SwDoc* pDoc = createDoc("tdf116830.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert that the yellow rectangle (cell background) is painted after the
    // polypolygon (background shape).
    // Background shape: 1.1.1.2
    // Cell background: 1.1.1.3
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[2]/push[1]/push[1]/fillcolor[@color='#729fcf']", 1);
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[2]/push[1]/push[1]/polypolygon",
                1);

    // This failed: cell background was painted before the background shape.
    assertXPath(pXmlDoc,
                "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/fillcolor[@color='#ffff00']", 1);
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/rect", 1);
}

void SwLayoutWriter::testTdf116925()
{
    SwDoc* pDoc = createDoc("tdf116925.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc,
                       "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[3]/textarray/text",
                       "hello");
    // This failed, text color was #000000.
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[3]/textcolor[@color='#ffffff']", 1);
}

void SwLayoutWriter::testTdf117028()
{
    SwDoc* pDoc = createDoc("tdf117028.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // The only polypolygon in the rendering result was the white background we
    // want to avoid.
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//polypolygon");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);

    // Make sure the text is still rendered.
    assertXPathContent(pXmlDoc, "//textarray/text", "Hello");
}

void SwLayoutWriter::testTdf106390()
{
    SwDoc* pDoc = createDoc("tdf106390.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nBottom = getXPath(pXmlDoc, "//sectrectclipregion", "bottom").toInt32();

    // No end point of line segments shall go below the bottom of the clipping area.
    const OString sXPath = "//polyline/point[@y>" + OString::number(nBottom) + "]";

    assertXPath(pXmlDoc, sXPath, 0);
}

void SwLayoutWriter::testTableExtrusion1()
{
    SwDoc* pDoc = createDoc("table-extrusion1.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nRight = getXPath(pXmlDoc, "//sectrectclipregion", "right").toInt32();
    sal_Int32 nLeft = (nRight + getXPath(pXmlDoc, "(//rect)[1]", "right").toInt32()) / 2;

    // Expect table borders in right page margin.
    const OString sXPath = "//polyline/point[@x>" + OString::number(nLeft) + " and @x<"
                           + OString::number(nRight) + "]";

    assertXPath(pXmlDoc, sXPath, 4);
}

void SwLayoutWriter::testTableExtrusion2()
{
    SwDoc* pDoc = createDoc("table-extrusion2.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocPtr pXmlDoc = dumper.dumpAndParse(*xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // End point position of the outer table.
    sal_Int32 nX = getXPath(pXmlDoc, "(//polyline[1]/point)[2]", "x").toInt32();

    // Do not allow inner table extrude outer table.
    const OString sXPath = "//polyline/point[@x>" + OString::number(nX) + "]";

    assertXPath(pXmlDoc, sXPath, 0);
}

void SwLayoutWriter::testTdf116848()
{
    SwDoc* pDoc = createDoc("tdf116848.odt");
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

void SwLayoutWriter::testTdf117245()
{
    createDoc("tdf117245.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin did not use a single line when there was
    // enough space for the text.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 1);

    // This was 2, same problem elsewhere due to code duplication.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/LineBreak", 1);
}

void SwLayoutWriter::testTdf118672()
{
    createDoc("tdf118672.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();

    // Check if we have hyphenation support, otherwise can't test SwHyphPortion.
    uno::Reference<linguistic2::XLinguServiceManager2> xLinguServiceManager
        = linguistic2::LinguServiceManager::create(comphelper::getProcessComponentContext());
    uno::Sequence<lang::Locale> aLocales
        = xLinguServiceManager->getAvailableLocales("com.sun.star.linguistic2.Hyphenator");
    if (std::none_of(aLocales.begin(), aLocales.end(), [](const lang::Locale& rLocale) {
            return rLocale.Language == "en" && rLocale.Country == "US";
        }))
        return;

    OUString aLine1("He heard quiet steps behind him. That didn't bode well. Who could be fol*1 2 "
                    "3 4 5 6 7 8 9 10con-");
    // This ended as "fol*1 2 3 4 5 6 7 8 9", i.e. "10con-" was moved to the next line.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak[1]", "Line", aLine1);
    OUString aLine2("setetur");
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak[2]", "Line", aLine2);
}

void SwLayoutWriter::testTdf117923()
{
    createDoc("tdf117923.doc");
    xmlDocPtr pXmlDoc = parseLayoutDump();

    // Check that we actually test the line we need
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]", "GHI GHI GHI GHI");
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nType", "POR_NUMBER");
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "rText", "2.");
    // The numbering height was 960.
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nHeight", "220");
}

void SwLayoutWriter::testTdf109077()
{
    createDoc("tdf109077.docx");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "top").toInt32();
    sal_Int32 nTextBoxTop = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "top").toInt32();
    // This was 281: the top of the shape and its textbox should match, though
    // tolerate differences <= 1px (about 15 twips).
    CPPUNIT_ASSERT_LESS(static_cast<sal_Int32>(15), nTextBoxTop - nShapeTop);
}

void SwLayoutWriter::testUserFieldTypeLanguage()
{
    // Set the system locale to German, the document will be English.
    SvtSysLocaleOptions aOptions;
    aOptions.SetLocaleConfigString("de-DE");
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions] {
        aOptions.SetLocaleConfigString(OUString());
        aOptions.Commit();
    });

    SwDoc* pDoc = createDoc("user-field-type-language.fodt");
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->UpdateFields();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was "123,456.00", via a buggy 1234.56 -> 1234,56 -> 123456 ->
    // 123,456.00 transform chain.
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[@nType='POR_FLD']", "rText", "1,234.56");
}

void SwLayoutWriter::testTdf109137()
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

//just care it doesn't crash
void SwLayoutWriter::testForcepoint72() { createDoc("forcepoint72-1.rtf"); }

void SwLayoutWriter::testTdf118058()
{
    SwDoc* pDoc = createDoc("tdf118058.fodt");
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

void SwLayoutWriter::testTdf117188()
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

void SwLayoutWriter::testTdf117187()
{
    createDoc("tdf117187.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();

    // there should be no fly portions
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[@nType='POR_FLY']", 0);
}

void SwLayoutWriter::testTdf119875()
{
    createDoc("tdf119875.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFirstTop
        = getXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "top").toInt32();
    sal_Int32 nSecondTop
        = getXPath(pXmlDoc, "/root/page[2]/body/section[2]/infos/bounds", "top").toInt32();
    // The first section had the same top value as the second one, so they
    // overlapped.
    CPPUNIT_ASSERT_LESS(nSecondTop, nFirstTop);
}

void SwLayoutWriter::testTdf120287()
{
    createDoc("tdf120287.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin Word-specific compat flag did not imply
    // default-in-Word printer-independent layout, resulting in an additional
    // line break.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 1);
}

void SwLayoutWriter::testTdf120287b()
{
    createDoc("tdf120287b.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 1418, TabOverMargin did the right split of the paragraph to two
    // lines, but then calculated a too large tab portion size on the first
    // line.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/Text[@nType='POR_TABRIGHT']", "nWidth", "17");
}

void SwLayoutWriter::testTdf120287c()
{
    createDoc("tdf120287c.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    // This was 2, the second line was not broken into a 2nd and a 3rd one,
    // rendering text outside the paragraph frame.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 3);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwLayoutWriter);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
