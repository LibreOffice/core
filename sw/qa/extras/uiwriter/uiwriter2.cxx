/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <sstream>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/propertysequence.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/settings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svxids.hrc>
#include <view.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <wrtsh.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <flyfrm.hxx>
#include <pagefrm.hxx>
#include <fmtanchr.hxx>
#include <UndoManager.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <itabenum.hxx>
#include <fmtfsize.hxx>
#include <xmloff/odffields.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <svx/svxids.hrc>
#include <comphelper/lok.hxx>
#include <comphelper/scopeguard.hxx>
#include <editeng/acorrcfg.hxx>
#include <swacorr.hxx>
#include <txtfrm.hxx>
#include <redline.hxx>
#include <view.hxx>
#include <cmdid.h>
#include <AnnotationWin.hxx>
#include <PostItMgr.hxx>
#include <postithelper.hxx>
#include <fmtcntnt.hxx>
#include <shellio.hxx>

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/uiwriter/data2/";
char const FLOATING_TABLE_DATA_DIRECTORY[] = "/sw/qa/extras/uiwriter/data/floating_table/";
} // namespace

/// Second set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest2 : public SwModelTestBase
{
public:
    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        m_aSavedSettings = Application::GetSettings();
        if (OString(filename).indexOf("LocaleArabic") != -1)
        {
            std::unique_ptr<Resetter> pResetter(
                new Resetter([this]() { Application::SetSettings(this->m_aSavedSettings); }));
            AllSettings aSettings(m_aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("ar"));
            Application::SetSettings(aSettings);
            return pResetter;
        }
        return nullptr;
    }

protected:
    AllSettings m_aSavedSettings;
    SwDoc* createDoc(const char* pName = nullptr);
};

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

SwDoc* SwUiWriterTest2::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf47471_paraStyleBackground)
{
    SwDoc* pDoc = createDoc("tdf47471_paraStyleBackground.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14729933), getProperty<sal_Int32>(getParagraph(2), "FillColor"));

    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    lcl_dispatchCommand(mxComponent, ".uno:ResetAttributes", {});

    // the background color should revert to the color for 00Background style
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14605542), getProperty<sal_Int32>(getParagraph(2), "FillColor"));
    // the paragraph style should not be reset
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));

    // Save it and load it back.
    reload("writer8", "tdf47471_paraStyleBackgroundRT.odt");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(14605542), getProperty<sal_Int32>(getParagraph(2), "FillColor"));
    // on round-trip, the paragraph style name was lost
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("00Background"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf131684)
{
    load(DATA_DIRECTORY, "tdf131684.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    //Use selectAll 3 times in a row
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});

    lcl_dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    lcl_dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // without the fix, it crashes
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // check that the text frame has the correct upper
    xmlDocPtr pXmlDoc = parseLayoutDump();
    OUString const sectionId = getXPath(pXmlDoc, "/root/page[1]/body/section[7]", "id");
    OUString const sectionLower = getXPath(pXmlDoc, "/root/page[1]/body/section[7]", "lower");
    OUString const textId = getXPath(pXmlDoc, "/root/page[1]/body/section[7]/txt[1]", "id");
    OUString const textUpper = getXPath(pXmlDoc, "/root/page[1]/body/section[7]/txt[1]", "upper");
    CPPUNIT_ASSERT_EQUAL(textId, sectionLower);
    CPPUNIT_ASSERT_EQUAL(sectionId, textUpper);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdfChangeNumberingListAutoFormat)
{
    createDoc("tdf117923.docx");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocPtr pXmlDoc = parseLayoutDump();

    // Check that we actually test the line we need
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]", "GHI GHI GHI GHI");
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nType",
                "PortionType::Number");
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "rText", "2.");
    // The numbering height was 960 in DOC format.
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nHeight", "220");

    // tdf#127606: now it's possible to change formatting of numbering
    // increase font size (220 -> 260)
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
    lcl_dispatchCommand(mxComponent, ".uno:Grow", {});
    pViewShell->Reformat();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nHeight", "260");

    // save it to DOCX
    reload("Office Open XML Text", "tdf117923.docx");
    pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->Reformat();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // this was 220
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nHeight", "260");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf101534)
{
    // Copy the first paragraph of the document.
    load(DATA_DIRECTORY, "tdf101534.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara(/*bSelect=*/true);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Copy();

    // Go to the second paragraph, assert that we have margins as direct
    // formatting.
    pWrtShell->Down(/*bSelect=*/false);
    SfxItemSet aSet(pWrtShell->GetAttrPool(), svl::Items<RES_LR_SPACE, RES_LR_SPACE>{});
    pWrtShell->GetCurAttr(aSet);
    CPPUNIT_ASSERT(aSet.HasItem(RES_LR_SPACE));

    // Make sure that direct formatting is preserved during paste.
    pWrtShell->EndPara(/*bSelect=*/false);
    TransferableDataHelper aHelper(pTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);
    aSet.ClearItem();
    pWrtShell->GetCurAttr(aSet);
    // This failed, direct formatting was lost.
    CPPUNIT_ASSERT(aSet.HasItem(RES_LR_SPACE));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testExtendedSelectAllHang)
{
    SwDoc* const pDoc = createDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->InsertFootnote("");
    pWrtShell->StartOfSection();
    SwView* pView = pDoc->GetDocShell()->GetView();
    SfxStringItem aLangString(SID_LANGUAGE_STATUS, "Default_Spanish (Bolivia)");
    // this looped
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS, SfxCallMode::SYNCHRON,
                                                        { &aLangString });
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineMoveInsertInDelete)
{
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* const pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert(" foo");
    pWrtShell->SttEndDoc(true);
    pWrtShell->InsertFootnote("");
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());
    RedlineFlags const mode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(mode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(mode);
    // insert redline
    pWrtShell->Insert("bar");
    // first delete redline, logically containing the insert redline
    // (note: Word apparently allows similar things...)
    pWrtShell->SttEndDoc(true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Delete(); // the footnote
    // second delete redline, following the first one
    pWrtShell->EndOfSection(false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 3, /*bBasicCall=*/false);
    pWrtShell->Delete(); // "foo"

    // hiding used to copy the 2nd delete redline "foo", but not delete it
    pWrtShell->SetRedlineFlags(mode & ~RedlineFlags::ShowDelete); // hide
    CPPUNIT_ASSERT_EQUAL(
        OUString(" "),
        pWrtShell->GetCursor()->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
    pWrtShell->SetRedlineFlags(mode); // show again
    CPPUNIT_ASSERT_EQUAL(
        OUString(u"\u0001 foo"),
        pWrtShell->GetCursor()->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineInHiddenSection)
{
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* const pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    pWrtShell->SplitNode();
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->Insert("baz");

    RedlineFlags const mode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(mode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(mode);

    // delete paragraph "bar"
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 8, /*bBasicCall=*/false);
    pWrtShell->Delete();

    pWrtShell->StartOfSection();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndOfSection(true);

    SwSectionData section(CONTENT_SECTION, pWrtShell->GetUniqueSectionName());
    section.SetHidden(true);
    SwSection const* pSection = pWrtShell->InsertSection(section, nullptr);

    SwSectionNode const* pNode = pSection->GetFormat()->GetSectionNode();

    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 1]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 2]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 3]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(pNode->GetNodes()[pNode->GetIndex() + 4]->IsEndNode());

    pWrtShell->SetRedlineFlags(mode & ~RedlineFlags::ShowDelete); // hide

    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 1]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(pNode->GetNodes()[pNode->GetIndex() + 2]->IsEndNode());

    pWrtShell->SetRedlineFlags(mode); // show again

    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 1]->GetTextNode()->getLayoutFrame(nullptr));
    // there was a frame created here
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 2]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(
        !pNode->GetNodes()[pNode->GetIndex() + 3]->GetTextNode()->getLayoutFrame(nullptr));
    CPPUNIT_ASSERT(pNode->GetNodes()[pNode->GetIndex() + 4]->IsEndNode());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRedlineSplitContentNode)
{
    load(DATA_DIRECTORY, "try2.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* const pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    // these are required so that IsBlank() is true
    aViewOptions.SetBlank(true);
    aViewOptions.SetViewMetaChars(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    // enable redlining
    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    // hide
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    SwDocShell* const pDocShell = pTextDoc->GetDocShell();
    SwDoc* const pDoc = pDocShell->GetDoc();
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    pWrtShell->CalcLayout();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 18, /*bBasicCall=*/false);
    pWrtShell->SplitNode(true);
    rUndoManager.Undo();
    // crashed
    pWrtShell->SplitNode(true);
    rUndoManager.Undo();
    rUndoManager.Redo();
    rUndoManager.Undo();
    rUndoManager.Redo();
    rUndoManager.Undo();
    pWrtShell->SplitNode(true);
    rUndoManager.Undo();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137318)
{
    SwDoc* const pDoc = createDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert("A");

    // enable redlining
    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    // hide
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pWrtShell->DelLine();
    pWrtShell->StartOfSection(false);
    pWrtShell->SplitNode(true);
    pWrtShell->SplitNode(true);

    xmlDocPtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text", 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text", 0);
    // not sure why there's an empty text portion here, but it's not a problem
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1][@Portion]", 0);

    pWrtShell->Undo();

    // the problem was that here the "A" showed up again
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text", 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1][@Portion]", 0);

    pWrtShell->Undo();

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1][@Portion]", 0);

    pWrtShell->Undo();

    // now the "A" is no longer deleted
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1][@Portion]", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nLength", "1");

    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "A");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf136452)
{
    SwDoc* const pDoc(createDoc("tdf136452.fodt"));

    auto const nNodes(pDoc->GetNodes().Count());

    SwWrtShell* const pWrtShell(pDoc->GetDocShell()->GetWrtShell());

    // first deletion spanning 2 sections
    pWrtShell->SttEndDoc(false);
    pWrtShell->SetMark();
    pWrtShell->Up(true, 2);
    pWrtShell->Delete();

    // 2 paragraphs deleted, last section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 4, pDoc->GetNodes().Count());

    // second deletion spanning 2 sections
    pWrtShell->SetMark();
    pWrtShell->Up(true, 3);
    pWrtShell->Delete();

    // 3 paragraphs deleted, 2nd section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 9, pDoc->GetNodes().Count());

    pWrtShell->Undo();

    // 2 paragraphs deleted, last section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 4, pDoc->GetNodes().Count());

    // this crashed
    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(nNodes, pDoc->GetNodes().Count());

    pWrtShell->Redo();

    // 2 paragraphs deleted, last section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 4, pDoc->GetNodes().Count());

    pWrtShell->Redo();

    // 3 paragraphs deleted, 2nd section is gone
    CPPUNIT_ASSERT_EQUAL(nNodes - 9, pDoc->GetNodes().Count());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf137245)
{
    SwDoc* const pDoc(createDoc());
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());
    corr.GetSwFlags().bSetBorder = true;
    // sigh, it's a global... err i mean Singleton design pattern *eyeroll*
    SvxSwAutoFormatFlags flags(*SwEditShell::GetAutoFormatFlags());
    comphelper::ScopeGuard const g([=]() { SwEditShell::SetAutoFormatFlags(&flags); });
    flags.bSetBorder = true;
    SwEditShell::SetAutoFormatFlags(&flags);

    {
        SwFormatAnchor anchor(RndStdIds::FLY_AT_PARA);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
        flySet.Put(anchor);
        SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }
    {
        SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
        flySet.Put(anchor);
        SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }
    // move cursor back to body
    pWrtShell->SttEndDoc(false);
    // keep first paragraph empty so that its flys may be deleted too
    //pWrtShell->Insert("abc");
    pWrtShell->SplitNode(false);

    {
        SwFormatAnchor anchor(RndStdIds::FLY_AT_PARA);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
        flySet.Put(anchor);
        SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }
    {
        SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
        flySet.Put(anchor);
        SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }

    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rFormats.size());

    // move cursor back to body
    pWrtShell->SttEndDoc(false);
    pWrtShell->Insert("---");
    pWrtShell->SplitNode(true);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rFormats.size());

    // check that the AutoFormat did something
    pWrtShell->SttEndDoc(true);
    SfxItemSet set{ pDoc->GetAttrPool(), svl::Items<RES_BOX, RES_BOX>{} };
    pWrtShell->GetCurParAttr(set);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, set.GetItemState(RES_BOX, false));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf139922)
{
    SwDoc* const pDoc = createDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert("this _is_ a SEntence. this _is_ a SEntence.");

    CPPUNIT_ASSERT_EQUAL(OUString("this _is_ a SEntence. this _is_ a SEntence."),
                         getParagraph(2)->getString());

    //apply autocorrect StartAutoCorrect
    lcl_dispatchCommand(mxComponent, ".uno:AutoFormatApply", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: This is a Sentence. This is a Sentence.
    // - Actual  : this is a Sentence. This is a Sentence.
    CPPUNIT_ASSERT_EQUAL(OUString("This is a Sentence. This is a Sentence."),
                         getParagraph(2)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf135061)
{
    createDoc("tdf135061.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    lcl_dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        lcl_dispatchCommand(mxComponent, ".uno:Paste", {});
        Scheduler::ProcessEventsToIdle();
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(20, getShapes());

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        // Without the fix in place, this test would have crashed here
        lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
        Scheduler::ProcessEventsToIdle();
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf132236)
{
    load(DATA_DIRECTORY, "tdf132236.odt");

    SwXTextDocument* const pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // select everything and delete
    SwWrtShell* const pWrtShell(pTextDoc->GetDocShell()->GetWrtShell());
    pWrtShell->Down(true);
    pWrtShell->Down(true);
    pWrtShell->Down(true);
    pWrtShell->Delete();
    SwDoc* const pDoc(pWrtShell->GetDoc());
    sw::UndoManager& rUndoManager(pDoc->GetUndoManager());
    rUndoManager.Undo();

    // check that the text frames exist inside their sections
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section[2]/txt", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf131912)
{
    SwDoc* const pDoc = createDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    sw::UnoCursorPointer pCursor(
        pDoc->CreateUnoCursor(SwPosition(SwNodeIndex(pDoc->GetNodes().GetEndOfContent(), -1))));

    pDoc->getIDocumentContentOperations().InsertString(*pCursor, "foo");

    {
        SfxItemSet flySet(pDoc->GetAttrPool(),
                          svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
        SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
        pWrtShell->StartOfSection(false);
        pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        flySet.Put(anchor);
        SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
        flySet.Put(size); // set a size, else we get 1 char per line...
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));

    pCursor->SetMark();
    pCursor->GetMark()->nContent.Assign(pCursor->GetContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetContentNode(), 3);

    // replace with more text
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, "blahblah", false);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("blahblah"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("blahblah"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    pCursor->GetMark()->nContent.Assign(pCursor->GetContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetContentNode(), 3);

    // replace with less text
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, "x", false);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("x"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("x"), pCursor->GetNode().GetTextNode()->GetText());

    rUndoManager.Undo();

    pCursor->GetMark()->nContent.Assign(pCursor->GetContentNode(), 0);
    pCursor->GetPoint()->nContent.Assign(pCursor->GetContentNode(), 3);

    // regex replace with paragraph breaks
    pDoc->getIDocumentContentOperations().ReplaceRange(*pCursor, "xyz\\n\\nquux\\n", true);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetNode().GetTextNode()->GetText());
    pWrtShell->StartOfSection(false);
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    // regex replace with paragraph join
    pWrtShell->StartOfSection(false);
    pWrtShell->Down(true);
    pDoc->getIDocumentContentOperations().ReplaceRange(*pWrtShell->GetCursor(), "bar", true);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("bar\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("bar\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    pWrtShell->StartOfSection(false);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz"),
                         pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("xyz\n\nquux\n"), pWrtShell->GetCursor()->GetText());

    rUndoManager.Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pCursor->GetNode().GetTextNode()->GetText());
    pWrtShell->StartOfSection(false);
    pWrtShell->EndOfSection(true);
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pWrtShell->GetCursor()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf140007)
{
    typedef sal_uLong SwNodeOffset;
    SwDoc* const pDoc = createDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->Insert("baz");
    CPPUNIT_ASSERT_EQUAL(sal_uLong(13), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(11)]->GetTextNode()->GetText());

    pWrtShell->SttEndDoc(true);
    pWrtShell->EndPara(false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Replace(" ", true);
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->SttEndDoc(true);
    pWrtShell->EndPara(false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    pWrtShell->Replace(" ", true);
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar baz"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(11), pDoc->GetNodes().Count());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(13), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(11)]->GetTextNode()->GetText());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(OUString("foo bar baz"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(11), pDoc->GetNodes().Count());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(12), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(13), pDoc->GetNodes().Count());
    CPPUNIT_ASSERT_EQUAL(OUString("foo"),
                         pDoc->GetNodes()[SwNodeOffset(9)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("bar"),
                         pDoc->GetNodes()[SwNodeOffset(10)]->GetTextNode()->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("baz"),
                         pDoc->GetNodes()[SwNodeOffset(11)]->GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf139982)
{
    SwDoc* const pDoc = createDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // turn on redlining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    pWrtShell->Insert("helloo");

    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    {
        SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
        anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
        SfxItemSet flySet(pDoc->GetAttrPool(), svl::Items<RES_ANCHOR, RES_ANCHOR>{});
        flySet.Put(anchor);
        SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
        CPPUNIT_ASSERT(pFly != nullptr);
    }

    pWrtShell->SttEndDoc(true);
    pWrtShell->EndPara(/*bSelect=*/true);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));

    pWrtShell->Replace("hello", true);

    // the problem was that a redline delete with the same author as redline
    // insert has its text deleted immediately, including anchored flys.
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf135976)
{
    SwDoc* const pDoc = createDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert("foobar");

    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    SfxItemSet flySet(pDoc->GetAttrPool(), svl::Items<RES_ANCHOR, RES_ANCHOR>{});
    flySet.Put(anchor);
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);

    // turn on redlining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pFly->GetAnchor().GetContentAnchor()->nContent.GetIndex());

    pWrtShell->UnSelectFrame();
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    pWrtShell->DelLeft();
    pWrtShell->DelLeft();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    // the problem was that the fly was deleted from the layout
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    // check that the anchor was moved outside the redline
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pFly->GetAnchor().GetContentAnchor()->nContent.GetIndex());

    pWrtShell->Undo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    // check that the anchor was restored
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pFly->GetAnchor().GetContentAnchor()->nContent.GetIndex());

    pWrtShell->Redo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pFly->GetAnchor().GetContentAnchor()->nContent.GetIndex());

    pWrtShell->Undo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pFly->GetAnchor().GetContentAnchor()->nContent.GetIndex());

    // now again in the other direction:

    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 3, /*bBasicCall=*/false);

    pWrtShell->DelRight();
    pWrtShell->DelRight();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    // the problem was that the fly was deleted from the layout
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pFly->GetAnchor().GetContentAnchor()->nContent.GetIndex());

    pWrtShell->Undo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pFly->GetAnchor().GetContentAnchor()->nContent.GetIndex());

    pWrtShell->Redo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pFly->GetAnchor().GetContentAnchor()->nContent.GetIndex());

    pWrtShell->Undo(2);

    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetLayout()->GetLastPage()->GetSortedObjs()->size());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), pFly->GetAnchor().GetContentAnchor()->nContent.GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf54819)
{
    load(DATA_DIRECTORY, "tdf54819.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    //turn on red-lining and hide changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove first paragraph with paragraph break
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // remaining paragraph keeps its original style
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf54819_keep_numbering_with_Undo)
{
    load(DATA_DIRECTORY, "tdf54819b.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // heading

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    OUString sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    //turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove heading with paragraph break
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // solved problem: changing paragraph style after deletion
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    sNumName = getProperty<OUString>(getParagraph(2), "NumberingStyleName");
    // solved problem: lost numbering
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // accept deletion, remaining (now second) paragraph: still bulleted list item
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(2), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // solved problem: Undo with the workaround
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();

    // heading, manual test is correct
    // TODO: it works well, but the test fails...
    // SwWrtShell* const pWrtShell2 = pTextDoc->GetDocShell()->GetWrtShell();
    // CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
    //                     getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    // CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
    //                     getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf141175)
{
    createDoc("tdf141175.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    //Use selectAll 3 times in a row
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    //Without the fix in place, this test would have crashed here
    lcl_dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    lcl_dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119571_keep_numbering_with_Undo)
{
    // as the previous test, but with partial paragraph deletion:
    // all deleted paragraphs get the formatting of the first (the partially deleted) one
    load(DATA_DIRECTORY, "tdf54819b.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // heading

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    OUString sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(4), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Bad numbering", sNumName.isEmpty());

    //turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove only end part of the heading and the next numbered paragraph with paragraph break
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 6, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // solved problem: changing paragraph style after deletion
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // solved problem: apply numbering
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // accept deletion
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // Joined paragraph 2 and paragraph 4: Fusce...nunc.
    CPPUNIT_ASSERT(getParagraph(2)->getString().startsWith("Fusce"));
    CPPUNIT_ASSERT(getParagraph(2)->getString().endsWith("nunc."));
    // Remaining (now second) paragraph: it is still heading
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // solved problem: Undo with the workaround
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();

    // heading

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(4), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Bad numbering", sNumName.isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119571_keep_numbering_with_Reject)
{
    // as the previous test, but with partial paragraph deletion:
    // all deleted paragraphs get the formatting of the first (the partially deleted) one
    load(DATA_DIRECTORY, "tdf54819b.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // heading

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    OUString sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(4), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Bad numbering", sNumName.isEmpty());

    //turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove only end part of the heading and the next numbered paragraph with paragraph break
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 6, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // solved problem: changing paragraph style after deletion
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // solved problem: apply numbering
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // reject deletion
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);

    // heading

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Outline"),
                         getProperty<OUString>(getParagraph(2), "NumberingStyleName"));

    // next paragraph: bulleted list item

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Missing numbering style", !sNumName.isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Not a bulleted list item", sNumName != "Outline");

    // third paragraph: normal text without numbering

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(4), "ParaStyleName"));
    sNumName = getProperty<OUString>(getParagraph(4), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Bad numbering", sNumName.isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf140077)
{
    SwDoc* const pDoc = createDoc();

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // hide
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    pWrtShell->Insert("a");
    pWrtShell->SplitNode();
    pWrtShell->Insert("b");
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    // enable
    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pWrtShell->Delete();
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    // crashed in layout
    pWrtShell->SplitNode();

    pWrtShell->Undo();
    pWrtShell->Redo();
    pWrtShell->Undo();
    pWrtShell->Redo();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf109376_redline)
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    // need 2 paragraphs to get to the bMoveNds case
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->StartOfSection(false);

    // add AT_PARA fly at 1st to be deleted node
    SwFormatAnchor anchor(RndStdIds::FLY_AT_PARA);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
    flySet.Put(anchor);
    SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);

    pWrtShell->SttEndDoc(false);
    SwInsertTableOptions tableOpt(SwInsertTableFlags::DefaultBorder, 0);
    const SwTable& rTable = pWrtShell->InsertTable(tableOpt, 1, 1);

    pWrtShell->StartOfSection(false);
    SwPaM pam(*pWrtShell->GetCursor()->GetPoint());
    pam.SetMark();
    pam.GetPoint()->nNode = *rTable.GetTableNode();
    pam.GetPoint()->nContent.Assign(nullptr, 0);
    pam.Exchange(); // same selection direction as in doc compare...

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);
    rIDRA.AppendRedline(new SwRangeRedline(RedlineType::Delete, pam), true);
    // this used to assert/crash with m_pAnchoredFlys mismatch because the
    // fly was not deleted but its anchor was moved to the SwTableNode
    rIDRA.AcceptAllRedline(true);

    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf109376)
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    // need 2 paragraphs to get to the bMoveNds case
    pWrtShell->Insert("foo");
    pWrtShell->SplitNode();
    pWrtShell->Insert("bar");
    pWrtShell->SplitNode();
    pWrtShell->StartOfSection(false);

    // add AT_PARA fly at 1st to be deleted node
    SwFormatAnchor anchor(RndStdIds::FLY_AT_PARA);
    anchor.SetAnchor(pWrtShell->GetCursor()->GetPoint());
    SfxItemSet flySet(pDoc->GetAttrPool(),
                      svl::Items<RES_FRM_SIZE, RES_FRM_SIZE, RES_ANCHOR, RES_ANCHOR>{});
    flySet.Put(anchor);
    SwFormatFrameSize size(ATT_MIN_SIZE, 1000, 1000);
    flySet.Put(size); // set a size, else we get 1 char per line...
    SwFrameFormat const* pFly = pWrtShell->NewFlyFrame(flySet, /*bAnchValid=*/true);
    CPPUNIT_ASSERT(pFly != nullptr);

    pWrtShell->SttEndDoc(false);
    SwInsertTableOptions tableOpt(SwInsertTableFlags::DefaultBorder, 0);
    const SwTable& rTable = pWrtShell->InsertTable(tableOpt, 1, 1);

    pWrtShell->StartOfSection(false);
    SwPaM pam(*pWrtShell->GetCursor()->GetPoint());
    pam.SetMark();
    pam.GetPoint()->nNode = *rTable.GetTableNode();
    pam.GetPoint()->nContent.Assign(nullptr, 0);
    pam.Exchange(); // same selection direction as in doc compare...

    // this used to assert/crash with m_pAnchoredFlys mismatch because the
    // fly was not deleted but its anchor was moved to the SwTableNode
    pDoc->getIDocumentContentOperations().DeleteRange(pam);
    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Redo();
    CPPUNIT_ASSERT_EQUAL(size_t(0), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pWrtShell->GetFlyCount(FLYCNTTYPE_FRM));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf147414)
{
    SwDoc* const pDoc(createDoc());
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwAutoCorrect corr(*SvxAutoCorrCfg::Get().GetAutoCorrect());

    pWrtShell->Insert("Abc");

    // hide and enable
    lcl_dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    CPPUNIT_ASSERT(pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT(
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // backspace
    pWrtShell->DelLeft();
    pWrtShell->AutoCorrect(corr, u' ');

    // problem was this was 1 i.e. before the deleted "b" while " " was inserted after
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         pWrtShell->getShellCursor(false)->GetPoint()->nContent.GetIndex());
    CPPUNIT_ASSERT_EQUAL(
        OUString("Ab c"),
        pWrtShell->getShellCursor(false)->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf147310)
{
    SwDoc* pDoc = createDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // somehow bug happens only with 2 tables
    SwInsertTableOptions tableOpt(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(tableOpt, 1, 1);
    pWrtShell->InsertTable(tableOpt, 1, 1);

    pWrtShell->SttEndDoc(/*bStart=*/true);

    pWrtShell->DeleteRow(false);
    pWrtShell->DeleteRow(false);

    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/tab", 0);
        discardDumpedLayout();
    }
    pWrtShell->Undo();
    // this did not create frames for the table
    pWrtShell->Undo();
    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        // there are 2 tables
        assertXPath(pXmlDoc, "/root/page/body/tab", 2);
        discardDumpedLayout();
    }
    pWrtShell->Redo();
    pWrtShell->Redo();
    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/tab", 0);
        discardDumpedLayout();
    }
    pWrtShell->Undo();
    pWrtShell->Undo();
    {
        xmlDocPtr pXmlDoc = parseLayoutDump();
        // there are 2 tables
        assertXPath(pXmlDoc, "/root/page/body/tab", 2);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf64242_optimizeTable)
{
    SwDoc* pDoc = createDoc("tdf64242_optimizeTable.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    double origWidth = getProperty<double>(xTextTable, "Width");
    sal_Int32 nToleranceW = origWidth * .01;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table Width", double(17013), origWidth, nToleranceW);

    pWrtShell->SelTable(); //select the whole table

    lcl_dispatchCommand(mxComponent, ".uno:SetOptimalColumnWidth", {});
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table Width: optimize", origWidth,
                                         getProperty<double>(xTextTable, "Width"), nToleranceW);

    lcl_dispatchCommand(mxComponent, ".uno:SetMinimalColumnWidth", {});
    CPPUNIT_ASSERT_MESSAGE("Table Width: minimized",
                           (origWidth - nToleranceW) > getProperty<double>(xTextTable, "Width"));

    double origRowHeight = getProperty<double>(xTableRows->getByIndex(2), "Height");
    sal_Int32 nToleranceH = origRowHeight * .01;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Row Height", double(3441), origRowHeight, nToleranceH);

    lcl_dispatchCommand(mxComponent, ".uno:SetOptimalRowHeight", {});
    double optimalRowHeight = getProperty<double>(xTableRows->getByIndex(2), "Height");
    CPPUNIT_ASSERT_MESSAGE("Row Height: optimized",
                           (origRowHeight - nToleranceH) > optimalRowHeight);

    lcl_dispatchCommand(mxComponent, ".uno:SetMinimalRowHeight", {});
    double minimalRowHeight = getProperty<double>(xTableRows->getByIndex(2), "Height");
    CPPUNIT_ASSERT_MESSAGE("Row Height: minimized",
                           (optimalRowHeight - nToleranceH) > minimalRowHeight);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Row set to auto-height", double(0), minimalRowHeight);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf126784_distributeSelectedColumns)
{
    SwDoc* pDoc = createDoc("tdf126784_distributeSelectedColumns.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    auto aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(xTableRows->getByIndex(0),
                                                                       "TableColumnSeparators");
    sal_Int16 nOrigCol2Pos = aSeq[0].Position;
    sal_Int16 nOrigCol3Pos = aSeq[1].Position;

    //Select column 1 and 2
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    lcl_dispatchCommand(mxComponent, ".uno:DistributeColumns", {});

    aSeq = getProperty<uno::Sequence<text::TableColumnSeparator>>(xTableRows->getByIndex(0),
                                                                  "TableColumnSeparators");
    CPPUNIT_ASSERT_MESSAGE("Second column should shrink", nOrigCol2Pos < aSeq[0].Position);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Last column shouldn't change", nOrigCol3Pos, aSeq[1].Position);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf108687_tabstop)
{
    SwDoc* pDoc = createDoc("tdf108687_tabstop.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    sal_Int32 nStartIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), nStartIndex);

    // Now pressing 'tab' should jump to the radio buttons.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();
    //sal_Int32 nEndIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(11), nEndIndex);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119571)
{
    load(DATA_DIRECTORY, "tdf54819.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    //turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // join paragraphs by removing the end of the first one with paragraph break
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // second paragraph changes its style in "Show changes" mode
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119019)
{
    // check handling of overlapping redlines
    load(DATA_DIRECTORY, "tdf119019.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Nunc viverra imperdiet enim. Fusce est. Vivamus a tellus."),
                         getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(2), 1)->getString());
    // second paragraph has got a tracked paragraph formatting at this point
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 1), "RedlineType"));

    // delete last word of the second paragraph to remove tracked paragraph formatting
    // of this paragraph to track and show word deletion correctly.
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 7, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // check tracked text deletion
    CPPUNIT_ASSERT_EQUAL(OUString("tellus."), getRun(getParagraph(2), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(2), 2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 2), "RedlineType"));

    // make sure that the tracked paragraph formatting is removed
    CPPUNIT_ASSERT(!hasProperty(getRun(getParagraph(2), 1), "RedlineType"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf119824)
{
    // check handling of overlapping redlines with Redo
    SwDoc* pDoc = createDoc("tdf119019.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "
                                  "et netus et malesuada fames ac turpis egestas. "
                                  "Proin pharetra nonummy pede. Mauris et orci."),
                         getParagraph(3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 1)->getString());
    // third paragraph has got a tracked paragraph formatting at this point
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), "RedlineType"));

    // and a tracked text deletion at the beginning of the paragraph
    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "),
                         getRun(getParagraph(3), 3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 2), "RedlineType"));

    // delete last word of the third paragraph to remove tracked paragraph formatting
    // of this paragraph to track and show word deletion correctly.
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 5, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // check tracking of the new text deletion
    CPPUNIT_ASSERT_EQUAL(OUString("orci."), getRun(getParagraph(3), 7)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 6)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 6), "RedlineType"));

    // make sure that the tracked paragraph formatting is removed (tracked deletion is in the second run)
    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "),
                         getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), "RedlineType"));

    // tdf#119824 check redo
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();
    rUndoManager.Redo();
    rUndoManager.Redo();

    // check again the first tracked text deletion (we lost this before the redo fix)
    CPPUNIT_ASSERT_EQUAL(OUString("Pellentesque habitant morbi tristique senectus "),
                         getRun(getParagraph(3), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 1), "RedlineType"));

    // check redo of the new tracked text deletion
    CPPUNIT_ASSERT_EQUAL(OUString("orci."), getRun(getParagraph(3), 7)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getRun(getParagraph(3), 6)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(3), 6), "RedlineType"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf105413)
{
    load(DATA_DIRECTORY, "tdf105413.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // all paragraphs have got Standard paragraph style
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                             getProperty<OUString>(getParagraph(i), "ParaStyleName"));
    }

    // turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Set Heading 1 paragraph style in the 3th paragraph.
    // Because of the tracked deleted region between them,
    // this sets also the same style in the first paragraph automatically
    // to keep the changed paragraph style at hiding tracked changes or saving the document
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::makeAny(OUString("Heading 1")) },
        { "FamilyName", uno::makeAny(OUString("ParagraphStyles")) },
    });
    lcl_dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    // first paragraph gets the same heading style
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testUnfloatButtonSmallTable)
{
    // The floating table in the test document is too small, so we don't provide an unfloat button
    load(FLOATING_TABLE_DATA_DIRECTORY, "small_floating_table.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    const SwSortedObjs* pAnchored
        = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
    CPPUNIT_ASSERT(pAnchored);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnchored->size());
    SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];

    SwFlyFrame* pFlyFrame = dynamic_cast<SwFlyFrame*>(pAnchoredObj);
    CPPUNIT_ASSERT(pFlyFrame);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));

    SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
    CPPUNIT_ASSERT(pObj);
    pWrtShell->SelectObj(Point(), 0, pObj);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testUnfloatButton)
{
    // Different use cases where unfloat button should be visible
    const std::vector<OUString> aTestFiles = {
        "unfloatable_floating_table.odt", // Typical use case of multipage floating table
        "unfloatable_floating_table.docx", // Need to test the DOCX import whether we detect the floating table correctly
        "unfloatable_floating_table.doc", // Also the DOC import
        "unfloatable_small_floating_table.docx" // Atypical use case, when the table is small, but because of it's position is it broken to two pages
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        OString sTestFileName = OUStringToOString(aTestFile, RTL_TEXTENCODING_UTF8);
        OString sFailureMessage = OStringLiteral("Failure in the test file: ") + sTestFileName;

        load(FLOATING_TABLE_DATA_DIRECTORY, sTestFileName.getStr());
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pTextDoc);
        SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pWrtShell);

        const SwSortedObjs* pAnchored;
        if (sTestFileName == "unfloatable_small_floating_table.docx")
            pAnchored = pWrtShell->GetLayout()
                            ->GetLower()
                            ->GetLower()
                            ->GetLower()
                            ->GetNext()
                            ->GetDrawObjs();
        else
            pAnchored = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pAnchored);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), static_cast<size_t>(1),
                                     pAnchored->size());
        SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];

        // The unfloat button is not visible until it gets selected
        SwFlyFrame* pFlyFrame = dynamic_cast<SwFlyFrame*>(pAnchoredObj);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                               !pFlyFrame->IsShowUnfloatButton(pWrtShell));

        SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pObj);
        pWrtShell->SelectObj(Point(), 0, pObj);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame->IsShowUnfloatButton(pWrtShell));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testUnfloatButtonReadOnlyMode)
{
    // In read only mode we don't show the unfloat button even if we have a multipage floating table
    load(FLOATING_TABLE_DATA_DIRECTORY, "unfloatable_floating_table.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->SetReadonlyOption(true);

    const SwSortedObjs* pAnchored
        = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
    CPPUNIT_ASSERT(pAnchored);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pAnchored->size());
    SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];

    SwFlyFrame* pFlyFrame = dynamic_cast<SwFlyFrame*>(pAnchoredObj);
    CPPUNIT_ASSERT(pFlyFrame);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));

    SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
    CPPUNIT_ASSERT(pObj);
    pWrtShell->SelectObj(Point(), 0, pObj);
    CPPUNIT_ASSERT(!pFlyFrame->IsShowUnfloatButton(pWrtShell));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testUnfloating)
{
    // Test unfloating with tables imported from different file formats
    const std::vector<OUString> aTestFiles = {
        "unfloatable_floating_table.odt",
        "unfloatable_floating_table.docx",
        "unfloatable_floating_table.doc",
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        OString sTestFileName = OUStringToOString(aTestFile, RTL_TEXTENCODING_UTF8);
        OString sFailureMessage = OStringLiteral("Failure in the test file: ") + sTestFileName;

        // Test what happens when pushing the unfloat button
        load(FLOATING_TABLE_DATA_DIRECTORY, "unfloatable_floating_table.docx");
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pTextDoc);
        SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pWrtShell);

        SwFlyFrame* pFlyFrame;

        // Before unfloating we have only one page with a fly frame
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), SwFrameType::Page,
                                         pWrtShell->GetLayout()->GetLower()->GetType());
            CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                                   !pWrtShell->GetLayout()->GetLower()->GetNext());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                sFailureMessage.getStr(), SwFrameType::Txt,
                pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetType());
            const SwSortedObjs* pAnchored
                = pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetDrawObjs();
            CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pAnchored);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), static_cast<size_t>(1),
                                         pAnchored->size());
            SwAnchoredObject* pAnchoredObj = (*pAnchored)[0];
            pFlyFrame = dynamic_cast<SwFlyFrame*>(pAnchoredObj);
            CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame);
        }

        // Select the floating table
        SdrObject* pObj = pFlyFrame->GetFormat()->FindRealSdrObject();
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pObj);
        pWrtShell->SelectObj(Point(), 0, pObj);
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(), pFlyFrame->IsShowUnfloatButton(pWrtShell));

        // Push the unfloat button
        pFlyFrame->ActiveUnfloatButton(pWrtShell);
        Scheduler::ProcessEventsToIdle();

        // After unfloating we have two pages with one table frame on each page
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                               pWrtShell->GetLayout()->GetLower()->GetNext());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailureMessage.getStr(), SwFrameType::Page,
                                     pWrtShell->GetLayout()->GetLower()->GetNext()->GetType());
        CPPUNIT_ASSERT_MESSAGE(sFailureMessage.getStr(),
                               !pWrtShell->GetLayout()->GetLower()->GetNext()->GetNext());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            sFailureMessage.getStr(), SwFrameType::Tab,
            pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower()->GetType());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            sFailureMessage.getStr(), SwFrameType::Tab,
            pWrtShell->GetLayout()->GetLower()->GetNext()->GetLower()->GetLower()->GetType());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testRTLparaStyle_LocaleArabic)
{
    // New documents, created in RTL locales, were not round-tripping the paragraph style as RTL.
    // Set the locale to "ar" for this test - see preTest() at the top of this file.
    std::unique_ptr<Resetter> const pChanges(preTest("LocaleArabic"));

    createDoc(); // new, empty doc - everything defaults to RTL with Arabic locale

    // Save it and load it back.
    reload("Office Open XML Text", "tdf116404_paraStyleFrameDir.docx");

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles("ParagraphStyles")->getByName("Default Style"), uno::UNO_QUERY_THROW);
    // Test the text Direction value for the -none- based paragraph styles
    CPPUNIT_ASSERT_EQUAL_MESSAGE("RTL Writing Mode", sal_Int32(1),
                                 getProperty<sal_Int32>(xPageStyle, "WritingMode"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122893)
{
    load(DATA_DIRECTORY, "tdf105413.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // all paragraphs are left-aligned with preset single line spacing
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(i), "ParaAdjust"));
        lcl_dispatchCommand(mxComponent, ".uno:SpacePara1", {});
    }

    // turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Set center-aligned paragraph with preset double line spacing in the 3th paragraph.
    // Because of the tracked deleted region between them,
    // this sets also the same formatting in the first paragraph automatically
    // to keep the changed paragraph formatting at hiding tracked changes or saving the document
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    lcl_dispatchCommand(mxComponent, ".uno:CenterPara", {});
    lcl_dispatchCommand(mxComponent, ".uno:SpacePara2", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(3), "ParaAdjust")); // center-aligned
    CPPUNIT_ASSERT_EQUAL(sal_Int16(200),
                         getProperty<style::LineSpacing>(getParagraph(3), "ParaLineSpacing")
                             .Height); // double line spacing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(2), "ParaAdjust")); // left-aligned
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<style::LineSpacing>(getParagraph(2), "ParaLineSpacing")
                             .Height); // single line spacing
    // first paragraph is also center-aligned with double line spacing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty<sal_Int32>(getParagraph(1), "ParaAdjust"));
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(200), getProperty<style::LineSpacing>(getParagraph(1), "ParaLineSpacing").Height);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122901)
{
    load(DATA_DIRECTORY, "tdf105413.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // all paragraphs with zero borders
    for (int i = 1; i < 4; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(getParagraph(i), "ParaTopMargin"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(getParagraph(i), "ParaBottomMargin"));
    }

    // turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Increase paragraph borders in the 3th paragraph, similar to the default icon of the UI
    // "Increase Paragraph Spacing". Because of the tracked deleted region between them,
    // this sets also the same formatting in the first paragraph automatically
    // to keep the changed paragraph formatting at hiding tracked changes or saving the document
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara(/*bSelect=*/false);

    lcl_dispatchCommand(mxComponent, ".uno:ParaspaceIncrease", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(101), getProperty<sal_Int32>(getParagraph(3), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(101),
                         getProperty<sal_Int32>(getParagraph(3), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(2), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));

    // first paragraph is also center-aligned with double line spacing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(101), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(101),
                         getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf122942)
{
    load(DATA_DIRECTORY, "tdf122942.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

    // Do the moral equivalent of mouse button down, move and up.
    // Start creating a custom shape that overlaps with the rounded rectangle
    // already present in the document.
    Point aStartPos(8000, 3000);
    pWrtShell->BeginCreate(static_cast<sal_uInt16>(OBJ_CUSTOMSHAPE), aStartPos);

    // Set its size.
    Point aMovePos(10000, 5000);
    pWrtShell->MoveCreate(aMovePos);

    // Finish creation.
    pWrtShell->EndCreate(SdrCreateCmd::ForceEnd);

    // Make sure that the shape is inserted.
    SwDoc* pDoc = pWrtShell->GetDoc();
    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFormats.size());

    reload("writer8", "tdf122942.odt");
    pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pDoc = pWrtShell->GetDoc();
    const SwFrameFormats& rFormats2 = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFormats2.size());

    // Make sure the top of the inserted shape does not move outside the existing shape, even after
    // reload.
    SdrObject* pObject1 = rFormats2[0]->FindSdrObject();
    CPPUNIT_ASSERT(pObject1);
    const tools::Rectangle& rOutRect1 = pObject1->GetLastBoundRect();
    SdrObject* pObject2 = rFormats2[1]->FindSdrObject();
    CPPUNIT_ASSERT(pObject2);
    const tools::Rectangle& rOutRect2 = pObject2->GetLastBoundRect();
    CPPUNIT_ASSERT(rOutRect2.Top() > rOutRect1.Top() && rOutRect2.Top() < rOutRect1.Bottom());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf52391)
{
    load(DATA_DIRECTORY, "tdf52391.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    lcl_dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    const uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1);
    // this was "Portion1", because the tracked background color of Portion1 was
    // accepted for "Reject All". Now rejection clears formatting of the text
    // in format-only changes, concatenating the text portions in the first paragraph.
    CPPUNIT_ASSERT_EQUAL(OUString("Portion1Portion2"), xRun->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf126206)
{
    load(DATA_DIRECTORY, "tdf126206.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // normal text (it was bold)
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 4)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
    }

    // reject tracked changes
    lcl_dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 3)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf101873)
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);

    SwDocShell* pDocShell = pDoc->GetDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Insert some content.
    pWrtShell->Insert("something");

    // Search for something which does not exist, twice.
    uno::Sequence<beans::PropertyValue> aFirst(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(OUString("fig")) },
        { "SearchItem.Backward", uno::makeAny(false) },
    }));
    lcl_dispatchCommand(mxComponent, ".uno:ExecuteSearch", aFirst);
    lcl_dispatchCommand(mxComponent, ".uno:ExecuteSearch", aFirst);

    uno::Sequence<beans::PropertyValue> aSecond(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(OUString("something")) },
        { "SearchItem.Backward", uno::makeAny(false) },
    }));
    lcl_dispatchCommand(mxComponent, ".uno:ExecuteSearch", aSecond);

    // Without the accompanying fix in place, this test would have failed with "Expected: something;
    // Actual:", i.e. searching for "something" failed, even if it was inserted above.
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("something"), pShellCursor->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTableWidth)
{
    load(DATA_DIRECTORY, "frame_size_export.docx");

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // after exporting: table width was overwritten in the doc model
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<sal_Int16>(xTables->getByIndex(0), "RelativeWidth"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTextFormFieldInsertion)
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a text form field
    lcl_dispatchCommand(mxComponent, ".uno:TextFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMTEXT), pFieldmark->GetFieldname());

    // The text form field has the placeholder text in it
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());

    // Undo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    xPara.set(getParagraph(1));
    CPPUNIT_ASSERT(xPara->getString().isEmpty());

    // Redo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    xPara.set(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testCheckboxFormFieldInsertion)
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a checkbox form field
    lcl_dispatchCommand(mxComponent, ".uno:CheckBoxFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMCHECKBOX), pFieldmark->GetFieldname());
    // The checkbox is not checked by default
    ::sw::mark::ICheckboxFieldmark* pCheckBox
        = dynamic_cast<::sw::mark::ICheckboxFieldmark*>(pFieldmark);
    CPPUNIT_ASSERT(pCheckBox);
    CPPUNIT_ASSERT(!pCheckBox->IsChecked());

    // Undo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMCHECKBOX), pFieldmark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDropDownFormFieldInsertion)
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a drop-down form field
    lcl_dispatchCommand(mxComponent, ".uno:DropDownFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDROPDOWN), pFieldmark->GetFieldname());
    // Check drop down field's parameters. By default these params are not set
    const sw::mark::IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
    auto pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
    CPPUNIT_ASSERT(bool(pListEntries == pParameters->end()));
    auto pResult = pParameters->find(ODF_FORMDROPDOWN_RESULT);
    CPPUNIT_ASSERT(bool(pResult == pParameters->end()));

    // Undo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDROPDOWN), pFieldmark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testMixedFormFieldInsertion)
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert fields
    lcl_dispatchCommand(mxComponent, ".uno:TextFormField", {});
    lcl_dispatchCommand(mxComponent, ".uno:CheckBoxFormField", {});
    lcl_dispatchCommand(mxComponent, ".uno:DropDownFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMarkAccess->getAllMarksCount());

    // Undo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMarkAccess->getAllMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf124261)
{
#if !defined(WNT) && !defined(MACOSX)
    // Make sure that pressing a key in a btlr cell frame causes an immediate, correct repaint.
    SwDoc* pDoc = createDoc("tdf124261.docx");
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
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

    // Make sure that the text frame's area and the paint rectangle match.
    // Without the accompanying fix in place, this test would have failed with 'Expected: 1721;
    // Actual: 1547', i.e. an area other than the text frame was invalidated for a single-line
    // paragraph.
    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pFrame);
    SwRect aRect = pTextFrame->GetPaintSwRect();
    CPPUNIT_ASSERT_EQUAL(pTextFrame->getFrameArea().Top(), aRect.Top());
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDocxAttributeTableExport)
{
    createDoc("floating-table-position.docx");

    // get the table frame, set new values and dismiss the references
    {
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

        // change the properties
        // 8133 -> 8000
        xShape->setPropertyValue("VertOrientPosition", uno::makeAny(static_cast<sal_Int32>(8000)));
        // 5964 -> 5000
        xShape->setPropertyValue("HoriOrientPosition", uno::makeAny(static_cast<sal_Int32>(5000)));
        // 0 (frame) -> 8 (page print area)
        xShape->setPropertyValue("VertOrientRelation", uno::makeAny(static_cast<sal_Int16>(8)));
        // 8 (page print area) -> 0 (frame)
        xShape->setPropertyValue("HoriOrientRelation", uno::makeAny(static_cast<sal_Int16>(0)));
    }
    // save it to docx
    reload("Office Open XML Text", "floating-table-position.docx");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    // test the new values
    sal_Int32 nValue = getProperty<sal_Int32>(xShape, "VertOrientPosition");
    CPPUNIT_ASSERT(sal_Int32(7999) <= nValue && nValue <= sal_Int32(8001));
    nValue = getProperty<sal_Int32>(xShape, "HoriOrientPosition");
    CPPUNIT_ASSERT(sal_Int32(4999) <= nValue && nValue <= sal_Int32(5001));

    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), getProperty<sal_Int16>(xShape, "VertOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf118699_redline_numbering)
{
    load(DATA_DIRECTORY, "tdf118699.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    uno::Reference<beans::XPropertySet> xProps(getParagraph(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());

    CPPUNIT_ASSERT_MESSAGE(
        "first paragraph after the second deletion: missing numbering",
        getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(5), "NumberingRules")
            .is());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf125881_redline_list_level)
{
    load(DATA_DIRECTORY, "tdf125881.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<beans::XPropertySet> xProps(getParagraph(8), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("deleted paragraph: erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());

    // deleted paragraph gets the numbering of the next paragraph
    uno::Reference<beans::XPropertySet> xProps2(getParagraph(9), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: missing numbering",
                           xProps2->getPropertyValue("NumberingRules").hasValue());

    // check numbering level at deletion (1 instead of 0)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(getParagraph(9), "NumberingLevel"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf125916_redline_restart_numbering)
{
    load(DATA_DIRECTORY, "tdf125916.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // check unnecessary numbering
    uno::Reference<beans::XPropertySet> xProps(getParagraph(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf125310)
{
    load(DATA_DIRECTORY, "tdf125310.fodt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // paragraph join
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // copied paragraph style
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // without copying the page break
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf125310b)
{
    SwDoc* pDoc = createDoc("tdf125310b.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // remove second paragraph with the page break
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Up(/*bSelect=*/true);
    pWrtShell->DelLeft();

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // losing the page break, as without redlining
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf120336)
{
    load(DATA_DIRECTORY, "tdf120336.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // turn on red-lining and show changes
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    CPPUNIT_ASSERT_EQUAL(2, getPages());

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // keep page break, as without redlining
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf106843)
{
    load(DATA_DIRECTORY, "tdf106843.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();

    // try to turn off red-lining
    lcl_dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    // but the protection doesn't allow it
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testImageComment)
{
    // Load a document with an as-char image in it.
    SwDoc* pDoc = createDoc("image-comment.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();

    // Test document has "before<image>after", remove the content before the image.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // Select the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

    // Insert a comment while the image is selected.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT, SfxCallMode::SYNCHRON);

    // Verify that the comment is around the image.
    // Without the accompanying fix in place, this test would have failed, as FN_POSTIT was disabled
    // in the frame shell.
    // Then this test would have failed, as in case the as-char anchored image was at the start of
    // the paragraph, the comment of the image covered the character after the image, not the image.
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(xPara, 3), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 4), "TextPortionType"));

    // Insert content to the comment, and select the image again.
    SfxStringItem aItem(FN_INSERT_STRING, "x");
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(FN_INSERT_STRING, SfxCallMode::SYNCHRON,
                                                        { &aItem });
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

#if !defined(MACOSX)
    // Calc the left edge of the as-char frame.
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwFrame* pPage = pLayout->GetLower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pTextFrame = pBody->GetLower();
    CPPUNIT_ASSERT(pTextFrame->GetDrawObjs());
    const SwSortedObjs& rAnchored = *pTextFrame->GetDrawObjs();
    CPPUNIT_ASSERT_GREATER(static_cast<size_t>(0), rAnchored.size());
    SwAnchoredObject* pObject = rAnchored[0];
    long nFrameLeft = pObject->GetObjRect().Left();
    long nFrameTop = pObject->GetObjRect().Top();

    // Make sure that the anchor points to the bottom left corner of the image.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less or equal than: 1418
    // - Actual: 2442
    // The anchor pointed to the bottom right corner, so as-char and at-char was inconsistent.
    Scheduler::ProcessEventsToIdle();
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    for (const auto& pItem : *pPostItMgr)
    {
        const SwRect& rAnchor = pItem->pPostIt->GetAnchorRect();
        CPPUNIT_ASSERT_EQUAL(nFrameLeft, rAnchor.Left());
    }

    // Test the comment anchor we expose via the LOK API.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1418, 1418, 0, 0
    // - Actual  : 1418, 1418, 1024, 1024
    // I.e. the anchor position had a non-empty size, which meant different rendering via tiled
    // rendering and on the desktop.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    OUString aPostits = pTextDoc->getPostIts();
    std::stringstream aStream(aPostits.toUtf8().getStr());
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("comments"))
    {
        const boost::property_tree::ptree& rComment = rValue.second;
        OString aAnchorPos(rComment.get<std::string>("anchorPos").c_str());
        OString aExpected
            = OString::number(nFrameLeft) + ", " + OString::number(nFrameTop) + ", 0, 0";
        CPPUNIT_ASSERT_EQUAL(aExpected, aAnchorPos);
    }

#endif

    // Now delete the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the comment of the image was not deleted when the image was deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testImageCommentAtChar)
{
    // Load a document with an at-char image in it.
    SwDoc* pDoc = createDoc("image-comment-at-char.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();

    // Select the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

    // Insert a comment while the image is selected.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT, SfxCallMode::SYNCHRON);

    // Verify that the comment is around the image.
    // Without the accompanying fix in place, this test would have failed, as the comment was
    // anchored at the end of the paragraph, it was not around the image.
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(xPara, 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 3), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(xPara, 4), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 5), "TextPortionType"));

    // Insert content to the comment, and select the image again.
    SfxStringItem aItem(FN_INSERT_STRING, "x");
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(FN_INSERT_STRING, SfxCallMode::SYNCHRON,
                                                        { &aItem });
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Now delete the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the comment of the image was not deleted when the image was deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());

    // Undo the deletion and move the image down, so the anchor changes.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_UNDO, SfxCallMode::SYNCHRON);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    Point aNewAnchor = pWrtShell->GetFlyRect().TopLeft();
    aNewAnchor.Move(0, 600);
    pWrtShell->SetFlyPos(aNewAnchor);

    // Get the image anchor doc model position.
    SwFlyFrame* pFly = pWrtShell->GetCurrFlyFrame(false);
    CPPUNIT_ASSERT(pFly);
    SwFrameFormat& rFlyFormat = pFly->GetFrameFormat();
    const SwPosition* pImageAnchor = rFlyFormat.GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT(pImageAnchor);

    // Get the annotation mark doc model start.
    auto it = pDoc->getIDocumentMarkAccess()->getAnnotationMarksBegin();
    CPPUNIT_ASSERT(it != pDoc->getIDocumentMarkAccess()->getAnnotationMarksEnd());
    const sw::mark::IMark* pMark = *it;
    const SwPosition& rAnnotationMarkStart = pMark->GetMarkPos();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: SwPosition (node 14, offset 15)
    // - Actual  : SwPosition (node 12, offset 3)
    // This means moving the image anchor did not move the comment anchor / annotation mark, so the
    // image and its comment got out of sync.
    CPPUNIT_ASSERT_EQUAL(*pImageAnchor, rAnnotationMarkStart);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf120338)
{
    load(DATA_DIRECTORY, "tdf120338.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(2), "ParaAdjust")); // right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(3), "ParaAdjust")); // right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(4), "ParaAdjust")); // left
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(5), "ParaAdjust")); // right

    CPPUNIT_ASSERT_EQUAL(OUString(""),
                         getProperty<OUString>(getParagraph(7), "NumberingStyleName"));

    CPPUNIT_ASSERT_EQUAL(OUString("WWNum2"),
                         getProperty<OUString>(getParagraph(8), "NumberingStyleName"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(10), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(11), "ParaStyleName"));

    // reject tracked paragraph adjustments
    lcl_dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(2), "ParaAdjust")); // left
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(3), "ParaAdjust")); // center
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(4), "ParaAdjust")); // center
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(5), "ParaAdjust")); // left

    // tdf#126245 revert numbering changes
    CPPUNIT_ASSERT_EQUAL(OUString("WWNum2"),
                         getProperty<OUString>(getParagraph(7), "NumberingStyleName"));

    CPPUNIT_ASSERT_EQUAL(OUString(""),
                         getProperty<OUString>(getParagraph(8), "NumberingStyleName"));

    // tdf#126243 revert paragraph styles
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(10), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 3"),
                         getProperty<OUString>(getParagraph(11), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf120338_multiple_paragraph_join)
{
    load(DATA_DIRECTORY, "redline-para-join.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));

    // reject tracked paragraph styles
    lcl_dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 3"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testShapePageMove)
{
    // Load a document with 2 pages, shape on the first page.
    SwDoc* pDoc = createDoc("shape-page-move.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();
    // Make sure that the 2nd page is below the 1st one.
    pView->SetViewLayout(/*nColumns=*/1, /*bBookMode=*/false);
    calcLayout();

    // Select the shape.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();

    // Move the shape down to the 2nd page.
    SfxInt32Item aXItem(SID_ATTR_TRANSFORM_POS_X, 4000);
    SfxInt32Item aYItem(SID_ATTR_TRANSFORM_POS_Y, 12000);
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM, SfxCallMode::SYNCHRON,
                                                        { &aXItem, &aYItem });

    // Check if the shape anchor was moved to the 2nd page as well.
    SwFrameFormats* pShapeFormats = pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(!pShapeFormats->empty());
    auto it = pShapeFormats->begin();
    SwFrameFormat* pShapeFormat = *it;
    const SwPosition* pAnchor = pShapeFormat->GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT(pAnchor);

    // Find out the node index of the 1st para on the 2nd page.
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pFirstPage = pLayout->Lower();
    SwFrame* pSecondPage = pFirstPage->GetNext();
    CPPUNIT_ASSERT(pSecondPage->IsLayoutFrame());
    SwFrame* pBodyFrame = static_cast<SwLayoutFrame*>(pSecondPage)->GetLower();
    CPPUNIT_ASSERT(pBodyFrame->IsLayoutFrame());
    SwFrame* pTextFrame = static_cast<SwLayoutFrame*>(pBodyFrame)->GetLower();
    CPPUNIT_ASSERT(pTextFrame->IsTextFrame());
    sal_uLong nNodeIndex = static_cast<SwTextFrame*>(pTextFrame)->GetTextNodeFirst()->GetIndex();

    // Without the accompanying fix in place, this test would have failed with "Expected: 13;
    // Actual: 12", i.e. the shape was anchored to the last paragraph of the 1st page, not to a
    // paragraph on the 2nd page.
    CPPUNIT_ASSERT_EQUAL(nNodeIndex, pAnchor->nNode.GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDateFormFieldInsertion)
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    lcl_dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // The date form field has the placeholder text in it
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());

    // Undo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    lcl_dispatchCommand(mxComponent, ".uno:Redo", {});
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDateFormFieldContentOperations)
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    lcl_dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IDateFieldmark* pFieldmark = dynamic_cast<::sw::mark::IDateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // Check the default content added by insertion
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), pFieldmark->GetContent());

    // Set content to empty string
    pFieldmark->ReplaceContent("");
    CPPUNIT_ASSERT_EQUAL(OUString(""), pFieldmark->GetContent());

    // Replace empty string with a valid content
    pFieldmark->ReplaceContent("2019-10-23");
    CPPUNIT_ASSERT_EQUAL(OUString("2019-10-23"), pFieldmark->GetContent());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDateFormFieldCurrentDateHandling)
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    lcl_dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IDateFieldmark* pFieldmark = dynamic_cast<::sw::mark::IDateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // The default content is not a valid date
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), pFieldmark->GetContent());
    std::pair<bool, double> aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(!aResult.first);

    // Check empty string
    pFieldmark->ReplaceContent("");
    aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(!aResult.first);

    // Check valid date
    // Set date format first
    sw::mark::IFieldmark::parameter_map_t* pParameters = pFieldmark->GetParameters();
    (*pParameters)[ODF_FORMDATE_DATEFORMAT] <<= OUString("YYYY/MM/DD");
    (*pParameters)[ODF_FORMDATE_DATEFORMAT_LANGUAGE] <<= OUString("en-US");

    // Set date value and check whether the content is formatted correctly
    pFieldmark->SetCurrentDate(48000.0);
    aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(aResult.first);
    CPPUNIT_ASSERT_EQUAL(48000.0, aResult.second);
    CPPUNIT_ASSERT_EQUAL(OUString("2031/06/01"), pFieldmark->GetContent());
    // Current date param contains date in a "standard format"
    OUString sCurrentDate;
    auto pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    CPPUNIT_ASSERT_EQUAL(OUString("2031-06-01"), sCurrentDate);
}

#if !defined(_WIN32)
CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testDateFormFieldCurrentDateInvalidation)
{
    SwDoc* pDoc = createDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    lcl_dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IDateFieldmark* pFieldmark = dynamic_cast<::sw::mark::IDateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // Set a date first
    sw::mark::IFieldmark::parameter_map_t* pParameters = pFieldmark->GetParameters();
    pFieldmark->SetCurrentDate(48000.0);
    std::pair<bool, double> aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(aResult.first);
    CPPUNIT_ASSERT_EQUAL(48000.0, aResult.second);

    // Do the layouting to trigger invalidation
    // Since we have the current date consistent with the field content
    // This invalidation won't change anything
    calcLayout();
    Scheduler::ProcessEventsToIdle();

    // Current date param contains date in a "standard format"
    OUString sCurrentDate;
    auto pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    // We have the current date parameter set
    CPPUNIT_ASSERT_EQUAL(OUString("2031-06-01"), sCurrentDate);

    // Now change the content of the field
    pFieldmark->ReplaceContent("[select date]");
    // Do the layouting to trigger invalidation
    calcLayout();
    Scheduler::ProcessEventsToIdle();

    sCurrentDate.clear();
    pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    CPPUNIT_ASSERT_EQUAL(OUString(""), sCurrentDate);
}
#endif

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testOleSaveWhileEdit)
{
    // Enable LOK mode, otherwise OCommonEmbeddedObject::SwitchStateTo_Impl() will throw when it
    // finds out that the test runs headless.
    comphelper::LibreOfficeKit::setActive();

    // Load a document with a Draw doc in it.
    SwDoc* pDoc = createDoc("ole-save-while-edit.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoObj(/*bNext=*/true, GotoObjFlags::Any);

    // Select the frame and switch to the frame shell.
    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->StopShellTimer();

    // Start editing the OLE object.
    pWrtShell->LaunchOLEObj();

    // Save the document without existing the OLE edit.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), {});

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    // Without the accompanying fix in place, this test would have failed: the OLE object lost its
    // replacement on save if the edit was active while saving.
    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Object 1"));

    // Dispose the document while LOK is still active to avoid leaks.
    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf105330)
{
    load(DATA_DIRECTORY, "tdf105330.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();
    SfxUInt16Item aRows(SID_ATTR_TABLE_ROW, 1);
    SfxUInt16Item aColumns(SID_ATTR_TABLE_COLUMN, 1);
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(FN_INSERT_TABLE, SfxCallMode::SYNCHRON,
                                                        { &aRows, &aColumns });

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();

    // Without the accompanying fix in place, height was only 1 twips (practically invisible).
    // Require at least 12pt height (font size under the cursor), in twips.
    CPPUNIT_ASSERT_GREATEREQUAL(
        static_cast<long>(240),
        pWrtShell->GetVisibleCursor()->GetTextCursor().GetSize().getHeight());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf118311)
{
    load(DATA_DIRECTORY, "tdf118311.fodt");

    SwXTextDocument* pDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDoc);

    SwDocShell* pDocShell = pDoc->GetDocShell();
    CPPUNIT_ASSERT(pDocShell);

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Jump to the first cell, selecting its content
    uno::Sequence<beans::PropertyValue> aSearch(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(OUString("a")) },
        { "SearchItem.Backward", uno::makeAny(false) },
    }));
    lcl_dispatchCommand(mxComponent, ".uno:ExecuteSearch", aSearch);

    //  .uno:Cut doesn't remove the table, only the selected content of the first cell
    lcl_dispatchCommand(mxComponent, ".uno:Cut", {});

    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // .uno:SelectAll selects the whole table, and UNO command Cut cuts it
    lcl_dispatchCommand(mxComponent, ".uno:SelectAll", {});
    lcl_dispatchCommand(mxComponent, ".uno:Cut", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf128335)
{
    // Load the bugdoc, which has 3 textboxes.
    SwDoc* pDoc = createDoc("tdf128335.odt");

    // Select the 3rd textbox.
    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // Cut it.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);

    // Paste it: this makes the 3rd textbox anchored in the 2nd one.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_PASTE, SfxCallMode::SYNCHRON);

    // Select all shapes.
    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(
        pXTextDocument->getCurrentController(), uno::UNO_QUERY);
    xSelectionSupplier->select(pXTextDocument->getDrawPages()->getByIndex(0));

    // Cut them.
    // Without the accompanying fix in place, this test would have crashed as the textboxes were
    // deleted in an incorrect order.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf128603)
{
    // Load the bugdoc, which has 3 textboxes.
    SwDoc* pDoc = createDoc("tdf128603.odt");

    // Select the 3rd textbox.
    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // Cut it.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);

    // Paste it: this makes the 3rd textbox anchored in the 2nd one.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_PASTE, SfxCallMode::SYNCHRON);

    // Undo all of this.
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();

    // Make sure the content indexes still match.
    const SwFrameFormats& rSpzFrameFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), rSpzFrameFormats.size());
    const SwNodeIndex* pIndex4 = rSpzFrameFormats[4]->GetContent().GetContentIdx();
    CPPUNIT_ASSERT(pIndex4);
    const SwNodeIndex* pIndex5 = rSpzFrameFormats[5]->GetContent().GetContentIdx();
    CPPUNIT_ASSERT(pIndex5);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 11
    // - Actual  : 14
    // i.e. the shape content index and the frame content index did not match after undo, even if
    // their "other text box format" pointers pointed to each other.
    CPPUNIT_ASSERT_EQUAL(pIndex4->GetIndex(), pIndex5->GetIndex());
}

// only care that it doesn't assert/crash
CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testOfz18563)
{
    OUString sURL(m_directories.getURLFromSrc("/sw/qa/extras/uiwriter/data2/ofz18563.docx"));
    SvFileStream aFileStream(sURL, StreamMode::READ);
    TestImportDOCX(aFileStream);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf129655)
{
    createDoc("tdf129655-vtextbox.odt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//fly/txt[@WritingMode='Vertical']", 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
