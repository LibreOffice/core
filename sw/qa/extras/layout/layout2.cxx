/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>

#include <comphelper/scopeguard.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <editeng/unolingu.hxx>
#include <svx/svdobj.hxx>

#include <unotxdoc.hxx>
#include <rootfrm.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <textboxhelper.hxx>
#include <frameformats.hxx>
#include <unotextrange.hxx>
#include <fmtanchr.hxx>
#include <editsh.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/layout/data/";

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter2 : public SwModelTestBase
{
protected:
    void CheckRedlineCharAttributesHidden();
};

// this mainly tests that splitting portions across redlines in SwAttrIter works
void SwLayoutWriter2::CheckRedlineCharAttributesHidden()
{
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", "9");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/merged", "paraPropsNodeIndex", "10");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/merged", "paraPropsNodeIndex", "11");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/merged", "paraPropsNodeIndex", "12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/merged", "paraPropsNodeIndex", "13");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/merged", "paraPropsNodeIndex", "14");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/merged", "paraPropsNodeIndex", "15");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/merged", "paraPropsNodeIndex", "16");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[2]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/merged", "paraPropsNodeIndex", "17");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[1]", "Portion", "foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/merged", "paraPropsNodeIndex", "18");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[2]", "Portion", "ob");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[3]", "Portion", "az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/merged", "paraPropsNodeIndex", "19");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[1]", "nType", "PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[1]", "Portion", "foobaz");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineCharAttributes)
{
    createSwDoc(DATA_DIRECTORY, "redline_charatr.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // verify after load
    CheckRedlineCharAttributesHidden();

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
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[2]", "Portion", "ba");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[3]", "Portion", "r");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[4]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/Text[4]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[2]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/Text[3]", "Portion", "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[1]", "Portion", "fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[2]", "Portion", "o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[3]", "Portion", "bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[4]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[4]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[5]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/Text[5]", "Portion", "az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[1]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[1]", "Portion", "foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[2]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[2]", "Portion", "b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[3]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[3]", "Portion", "a");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[4]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[4]", "Portion", "r");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[5]", "nType", "PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/Text[5]", "Portion", "baz");

    // verify after hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    CheckRedlineCharAttributesHidden();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineShowHideFootnotePagination)
{
    createSwDoc(DATA_DIRECTORY, "redline_footnote_pagination.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // check footnotes
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn", 6);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn", 3);
    // check that first page ends with the y line and second page starts with z
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[last()]/LineBreak[last()]", "Line",
                "yyyyyyyyy yyy yyyyyyyyyyyyyyyy yyyyyyy yyy yyyyy yyyyyyyyy yyy yyyyyyyyy ");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/LineBreak[1]", "Line",
                "zzz. zzz zzzz zzzz7 zzz zzz zzzzzzz zzz zzzz zzzzzzzzzzzzzz zzzzzzzzzzzz ");

    // hide redlines - all still visible footnotes move to page 1
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn", 2);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn", 0);

    // show again - should now get the same result as on loading
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    // check footnotes
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn", 6);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn", 3);
    // check that first page ends with the y line and second page starts with z
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[last()]/LineBreak[last()]", "Line",
                "yyyyyyyyy yyy yyyyyyyyyyyyyyyy yyyyyyy yyy yyyyy yyyyyyyyy yyy yyyyyyyyy ");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/LineBreak[1]", "Line",
                "zzz. zzz zzzz zzzz7 zzz zzz zzzzzzz zzz zzzz zzzzzzzzzzzzzz zzzzzzzzzzzz ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testtdf138951)
{
    // Open the bugdoc
    auto pDoc = createSwDoc(DATA_DIRECTORY, "tdf138951.odt");

    // Get the only shape
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape);

    // Gather its formats: the shape and textbox
    const SwFrameFormat* pTxFrm = SwTextBoxHelper::getOtherTextBoxFormat(xShape);
    CPPUNIT_ASSERT(pTxFrm);
    const SwFrameFormat* pShFrm = SwTextBoxHelper::getOtherTextBoxFormat(pTxFrm, RES_FLYFRMFMT);
    CPPUNIT_ASSERT(pShFrm);

    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();

    // Get the bound rectangle of the textframe
    tools::Rectangle aTxtFrmRect(pTxFrm->FindRealSdrObject()->GetLogicRect());

    // Get the bound rectangle of the shape
    tools::Rectangle aShpRect(pShFrm->FindRealSdrObject()->GetLogicRect());

    // Check the anchor the same and the textbox is inside the shape
    const bool bIsAnchTheSame
        = *pShFrm->GetAnchor().GetContentAnchor() == *pShFrm->GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT_MESSAGE("The anchor is different for the textbox and shape!", bIsAnchTheSame);
    CPPUNIT_ASSERT_MESSAGE("The textbox has fallen apart!", aShpRect.Contains(aTxtFrmRect));
    // Without the fix the anchor differs, and the frame outside of the shape
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineNumberInNumbering)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf42748.fodt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert the tracked deletion of the number of joined list item and
    // the tracked insertion of the number after a split list item as not black
    // (and not COL_GREEN color of the tracked text movement, see testRedlineMoving) elements
    assertXPath(
        pXmlDoc,
        "/metafile/push/push/push/textcolor[not(@color='#000000') and not(@color='#008000')]", 6);

    // tdf#145068 numbering shows changes in the associated list item, not the next one
    // This was 1 (black numbering of the first list item previously)
    assertXPath(pXmlDoc, "/metafile/push/push/push/font[4][@color='#000000']", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineMoving)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf42748.fodt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // create a 3-element list without change tracking
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    pEditShell->RejectRedline(0);
    pEditShell->AcceptRedline(0);

    // move down first list item with track changes
    dispatchCommand(mxComponent, ".uno:GoToStartOfDoc", {});
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    dispatchCommand(mxComponent, ".uno:MoveDown", {});

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // text and numbering colors show moving of the list item
    // These were 0 (other color, not COL_GREEN, color of the tracked text movement)
    assertXPath(pXmlDoc, "/metafile/push/push/push/textcolor[@color='#008000']", 5);
    assertXPath(pXmlDoc, "/metafile/push/push/push/font[@color='#008000']", 11);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineMovingDOCX)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf104797.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // This was 2 (moveFrom and moveTo joined other redlines)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(6), pEditShell->GetRedlineCount());

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // text colors show moved text
    // These were 0 (other color, not COL_GREEN, color of the tracked text movement)
    assertXPath(pXmlDoc, "/metafile/push/push/push/textcolor[@color='#008000']", 6);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf145719)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf145719.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // text colors show moved text
    // This was 0 (other color, not COL_GREEN, color of the tracked text movement)
    assertXPath(pXmlDoc, "/metafile/push/push/push/textcolor[@color='#008000']", 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testChangedTableRows)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "changed_table_rows.fodt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was 0 (other color, not COL_AUTHOR_TABLE_DEL, color of the tracked row deletion)
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#fce6f4']", 1);
    // This was 0 (other color, not COL_AUTHOR_TABLE_DEL, color of the tracked row insertion)
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#a3cef1']", 1);
    // This was 3 (color of the cells of the last column, 2 of them disabled by change tracking )
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#3faf46']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf145225_RedlineMovingWithBadInsertion)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf42748.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // create a 3-element list without change tracking
    // (because the fixed problem depends on the own changes)
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    pEditShell->RejectRedline(0);
    pEditShell->AcceptRedline(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    // Show Changes
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    // insert a tracked paragraph break in middle of the second list item, i.e. split it
    dispatchCommand(mxComponent, ".uno:GoToStartOfDoc", {});
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    pWrtShell->Down(false, 1);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->SplitNode(false);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());

    // move up the last list item over the paragraph split
    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});
    dispatchCommand(mxComponent, ".uno:MoveUp", {});
    dispatchCommand(mxComponent, ".uno:MoveUp", {});
    // This was 2 (the tracked paragraph break joined with the moved list item,
    // setting the not changed text of the second list item to tracked insertion)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(3), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125300)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125300.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Keep line spacing before bottom cell border (it was 1892)
    sal_Int32 y1
        = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[5]/polyline/point[1]", "y")
              .toInt32();
    sal_Int32 y2
        = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[5]/polyline/point[2]", "y")
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2092, y1, 7);
    CPPUNIT_ASSERT_EQUAL(y1, y2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf116830)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf116830.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf114163)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf114163.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/textarray[12]/text",
        "Data3");
    // This failed, if the legend first label is not "Data3". The legend position is right.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf131707)
{
    createSwDoc(DATA_DIRECTORY, "tdf131707_flyWrap.doc");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//body/tab/row[3]/cell[2]/txt/infos/bounds", "top", "2185");
    // the image should be inside of the cell boundary - so the same top or higher
    assertXPath(pXmlDoc, "//body/tab/row[3]/cell[2]/txt/anchored/fly/infos/bounds", "top", "2185");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122225)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf122225.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Bug 122225 - FILEOPEN DOCX Textbox of Column chart legend reduces and text of legend disappears
    const sal_Int32 nLegendLabelLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"Advanced Diploma\")])")
              .toInt32();
    // This failed, if the legend label is not "Advanced Diploma".
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nLegendLabelLines);

    // Bug 140623 - Fileopen DOCX: Text Orientation of X-Axis 0 instead of 45 degrees
    const sal_Int32 nThirdLabelLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"Hispanic\")])").toInt32();
    // This failed, if the third X axis label broke to multiple lines.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nThirdLabelLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125335)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125335.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/textarray[12]/text",
        "Data3");
    // This failed, if the legend first label is not "Data3". The legend position is bottom.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134247)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "legend-itemorder-min.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc,
                       "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray[14]/text",
                       "1. adatsor");
    // This failed, if the legend first label is not "1. adatsor".
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf75659)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf75659.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(
        pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray[17]/text", "Series1");

    assertXPathContent(
        pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray[18]/text", "Series2");

    assertXPathContent(
        pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray[19]/text", "Series3");
    // These failed, if the legend names are empty strings.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf136816)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf136816.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check number of legend entries
    assertXPath(pXmlDoc, "//text[contains(text(),\"Column\")]", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf126425)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "long_legendentry.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14
    // - Actual  : 12
    // i.e. the text of the chart legend lost.
    assertXPath(pXmlDoc, "//textarray", 14);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testUnusedOLEprops)
{
    CPPUNIT_ASSERT(createSwDoc(DATA_DIRECTORY, "tdf138465min.docx"));

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: >300
    // - Actual  : 142
    // i.e. the formula squashed
    CPPUNIT_ASSERT_GREATEREQUAL(
        double(300),
        getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/fly/notxt/infos/bounds", "height")
            .toDouble());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf123268)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf123268.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 41
    // - Actual  : 0
    // i.e. the chart lost.
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push",
                41);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf133005)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf133005.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    sal_Int32 nXChartWall = getXPath(pXmlDoc,
                                     "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/"
                                     "push[1]/push[1]/polyline[1]/point[2]",
                                     "x")
                                .toInt32();
    sal_Int32 nXColumn = getXPath(pXmlDoc,
                                  "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/"
                                  "push[1]/push[42]/polypolygon/polygon/point[1]",
                                  "x")
                             .toInt32();

    // This failed, if the value axis doesn't appear inside category.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nXChartWall, nXColumn, 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf115630)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf115630.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Test wide of inner chart area.
    sal_Int32 nXRight
        = getXPath(pXmlDoc,
                   "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[3]/polyline[1]/point[1]",
                   "x")
              .toInt32();
    sal_Int32 nXLeft
        = getXPath(pXmlDoc,
                   "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[3]/polyline[1]/point[2]",
                   "x")
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2895, nXRight - nXLeft, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf108021)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf108021.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/textarray[@length='22']",
        8);
    // This failed, if the textarray length of the first axis label not 22.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125334)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125334.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/textarray[@length='17']",
        4);
    // This failed, if the textarray length of the category axis label not 17.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122800)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf122800.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/textarray[@length='22']",
        9);
    // This failed, if the textarray length of the first axis label not 22.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTruncatedAxisLabel)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "testTruncatedAxisLabel.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // test the X axis label visibility
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/textarray[1]/text",
        "Long axis label truncated 1");

    // test the Y axis label visibility
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/textarray[3]/text",
        "-5.00");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf128996)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf128996.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc,
                       "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray[1]/text",
                       "A very long category name 1");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf126244)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf126244.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the first level of vertical category axis labels orientation. The first level orientation should be horizontal.
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/font[1]", "orientation",
                "0");
    // Test the second level of vertical category axis labels orientation. The second level orientation should be vertical.
    sal_Int32 nRotation
        = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/font[5]",
                   "orientation")
              .toInt32();
    CPPUNIT_ASSERT(nRotation >= 899);
    CPPUNIT_ASSERT(nRotation <= 900);
    // Test the third level of vertical category axis labels orientation. The third level orientation should be vertical.
    nRotation = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/font[7]",
                         "orientation")
                    .toInt32();
    CPPUNIT_ASSERT(nRotation >= 899);
    CPPUNIT_ASSERT(nRotation <= 900);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf127304)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf127304.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the first level of horizontal category axis labels orientation. The first level orientation should be vertical.
    sal_Int32 nRotation
        = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/font[1]",
                   "orientation")
              .toInt32();
    CPPUNIT_ASSERT(nRotation >= 899);
    CPPUNIT_ASSERT(nRotation <= 900);
    // Test the second level of horizontal category axis labels orientation. The second level orientation should be horizontal.
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/font[5]", "orientation",
                "0");
    // Test the third level of horizontal category axis labels orientation. The third level orientation should be horizontal.
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/font[7]", "orientation",
                "0");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testHorizontal_multilevel)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "horizontal_multilevel.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the Y position of horizontal category axis label.
    sal_Int32 nYposition
        = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/textarray[7]", "y")
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7945, nYposition, 20);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf138194)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "xaxis-labelbreak.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 8
    // - Actual  : 7
    // i.e. the X axis label flowed out of chart area.
    assertXPath(pXmlDoc, "//textarray", 8);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf138773)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf138773.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    const sal_Int32 nFirstLabelLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"2000-01\")])").toInt32();

    // This failed, if the first X axis label broke to multiple lines.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nFirstLabelLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf124796)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf124796.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the minimum value of Y axis is not -10.
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/textarray[5]/text",
        "-10");

    // This failed, if the maximum value of Y axis is not 15.
    assertXPathContent(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/textarray[10]/text",
        "15");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf72727)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf72727.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : Series1
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/push[3]/push/push/push/textarray[1]/text",
                       "1");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/push[3]/push/push/push/textarray[2]/text",
                       "2");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/push[3]/push/push/push/textarray[3]/text",
                       "3");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130969)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf130969.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the minimum value of Y axis is not 0.35781
    assertXPathContent(
        pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray[5]/text", "0.35781");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf40260)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf40260.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: f(x) = 1.26510397865547E-06 x − 5.95245604996327E-12
    // - Actual  : f(x) = 0 x − 0
    assertXPathContent(
        pXmlDoc, "/metafile/push/push/push/push[3]/push/push/push/textarray[19]/text",
        "f(x) = 1.26510397865547E-06 x " + OUStringChar(u'\x2212') + " 5.95245604996327E-12");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf129054)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf129054.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Test the size of diameter of Pie chart.
    sal_Int32 nYTop
        = getXPath(pXmlDoc,
                   "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[4]/polyline[1]/point[1]",
                   "y")
              .toInt32();
    sal_Int32 nYBottom
        = getXPath(
              pXmlDoc,
              "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[4]/polyline[1]/point[31]",
              "y")
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4810, nYTop - nYBottom, 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf129173)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "testAreaChartNumberFormat.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the first data label of area chart.
    assertXPathContent(
        pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray[22]/text", "56");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134866)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134866.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the data label of pie chart.
    assertXPathContent(
        pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray[2]/text", "100%");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf137116)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf137116.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nX2 = getXPath(pXmlDoc, "//textarray[2]", "x").toInt32(); // second data label
    sal_Int32 nX4 = getXPath(pXmlDoc, "//textarray[4]", "x").toInt32(); // fourth data label
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 694
    // - Actual  : -225
    // - Delta   : 100
    // i.e. the second data label appeared inside the pie slice.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(694, nX2 - nX4, 100);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf137154)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf137154.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nX1 = getXPath(pXmlDoc, "//textarray[1]", "x").toInt32(); // first data label
    sal_Int32 nX4 = getXPath(pXmlDoc, "//textarray[4]", "x").toInt32(); // fourth data label
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 10865
    // - Actual  : 10252
    // - Delta   : 50
    // i.e. the first data label appeared too close to the pie.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nX4, nX1, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf138777)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "outside_long_data_label.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    const sal_Int32 nFirstLabelLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"really\")])").toInt32();

    // This failed, if the first data label didn't break to multiple lines.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(1), nFirstLabelLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130031)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf130031.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nY = getXPath(pXmlDoc, "//textarray[11]", "y").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4339
    // - Actual  : 2182
    // - Delta   : 50
    // i.e. the data label appeared above the data point.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4339, nY, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130242)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf130242.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nY = getXPath(pXmlDoc, "//textarray[11]", "y").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4958
    // - Actual  : 3352
    // - Delta   : 50
    // i.e. the data label appeared above the data point.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4958, nY, 50);

    nY = getXPath(pXmlDoc, "//textarray[13]", "y").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3018
    // - Actual  : 2343
    // - Delta   : 50
    // i.e. the data label appeared above the data point.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3018, nY, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134121)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "piechart_leaderline.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the custom leader line on pie chart.
    assertXPath(pXmlDoc, "//polyline", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf138018)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf138018.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. the leader line was visible.
    assertXPath(pXmlDoc, "//polyline", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130380)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf130380.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nY = getXPath(pXmlDoc,
                            "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[1]/polypolygon/"
                            "polygon/point[1]",
                            "y")
                       .toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6727
    // - Actual  : 4411
    // - Delta   : 50
    // i.e. the area chart shrank.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6727, nY, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf129095)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf129095.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // check the inner chart area (relative size) visibility with testing the X axis label
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray/text",
                       "Category 1");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf132956)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132956.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // check the inner chart area (default size) visibility with testing the X axis label
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray/text",
                       "Category 1");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf137819)
{
    // Open the bugdoc and check if it went wrong
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf137819.fodt");
    CPPUNIT_ASSERT(pDoc);

    // Change the anchor if the textbox to As_char
    uno::Reference<beans::XPropertySet> xShapePropSet(getShape(1), uno::UNO_QUERY);
    xShapePropSet->setPropertyValue(
        "AnchorType", uno::Any(text::TextContentAnchorType::TextContentAnchorType_AS_CHARACTER));

    // Make the layout xml dump after the change
    auto pXml = parseLayoutDump();
    auto sTextRightSidePosition
        = getXPath(pXml, "/root/page/body/txt[6]/anchored/fly/infos/bounds", "right");
    auto sShapeRightSidePosition
        = getXPath(pXml, "/root/page/body/txt[6]/anchored/SwAnchoredDrawObject/bounds", "right");
    // Before the textframe did not follow the shape, now it supposed to
    // so the right side of the shape must be greater than the right side of
    // textframe:
    CPPUNIT_ASSERT(sTextRightSidePosition.toInt32() < sShapeRightSidePosition.toInt32());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122014)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf122014.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the chart title is aligned to left.
    sal_Int32 nX1 = getXPath(pXmlDoc, "//textarray[13]", "x").toInt32();
    sal_Int32 nX2 = getXPath(pXmlDoc, "//textarray[14]", "x").toInt32();
    CPPUNIT_ASSERT_GREATER(nX1 + 100, nX2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134659)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134659.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the axis label is aligned to left.
    sal_Int32 nX1 = getXPath(pXmlDoc, "//textarray[1]", "x").toInt32();
    sal_Int32 nX2 = getXPath(pXmlDoc, "//textarray[2]", "x").toInt32();
    CPPUNIT_ASSERT_GREATER(nX1 + 250, nX2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134235)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134235.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14
    // - Actual  : 13
    // i.e. the chart title flowed out of chart area.
    assertXPath(pXmlDoc, "//textarray", 14);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134676)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134676.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14
    // - Actual  : 13
    // i.e. the X axis title didn't break to multiple lines.
    assertXPath(pXmlDoc, "//textarray", 14);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134146)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134146.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    const sal_Int32 nTitleLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"Horizontal\")])").toInt32();

    // Without the accompanying fix in place, this test would have failed.
    // i.e. the Y axis title didn't break to multiple lines.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(1), nTitleLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf136061)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf136061.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // This failed, if the custom text of data label is missing.
    assertXPathContent(pXmlDoc, "//textarray[16]/text", "Customlabel");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf116925)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf116925.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc,
                       "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[3]/textarray/text",
                       "hello");
    // This failed, text color was #000000.
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/push[3]/textcolor[@color='#ffffff']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf117028)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf117028.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf106390)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf106390.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nBottom = getXPath(pXmlDoc, "//sectrectclipregion", "bottom").toInt32();

    // No end point of line segments shall go below the bottom of the clipping area.
    const OString sXPath = "//polyline/point[@y>" + OString::number(nBottom) + "]";

    assertXPath(pXmlDoc, sXPath, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTableExtrusion1)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "table-extrusion1.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nRight = getXPath(pXmlDoc, "//sectrectclipregion", "right").toInt32();
    sal_Int32 nLeft = static_cast<sal_Int32>(nRight * 0.95);

    // Expect table borders in right page margin.
    const OString sXPath = "//polyline/point[@x>" + OString::number(nLeft) + " and @x<"
                           + OString::number(nRight) + "]";

    assertXPath(pXmlDoc, sXPath, 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTableExtrusion2)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "table-extrusion2.odt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // End point position of the outer table.
    sal_Int32 nX = getXPath(pXmlDoc, "(//polyline[1]/point)[2]", "x").toInt32();

    // Do not allow inner table extrude outer table.
    const OString sXPath = "//polyline/point[@x>" + OString::number(nX) + "]";

    assertXPath(pXmlDoc, sXPath, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf116848)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf116848.odt");
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf117245)
{
    createSwDoc(DATA_DIRECTORY, "tdf117245.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin did not use a single line when there was
    // enough space for the text.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 1);

    // This was 2, same problem elsewhere due to code duplication.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/LineBreak", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf118672)
{
    createSwDoc(DATA_DIRECTORY, "tdf118672.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check if we have hyphenation support, otherwise can't test SwHyphPortion.
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    // This ended as "fol*1 2 3 4 5 6 7 8 9", i.e. "10con-" was moved to the next line.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak[1]", "Line",
                "He heard quiet steps behind him. That didn't bode well. Who could be fol*1 2 "
                "3 4 5 6 7 8 9 10con-");
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak[2]", "Line", "setetur");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf117923)
{
    createSwDoc(DATA_DIRECTORY, "tdf117923.doc");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check that we actually test the line we need
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]", "GHI GHI GHI GHI");
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nType",
                "PortionType::Number");
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "rText", "2.");
    // The numbering height was 960.
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nHeight", "220");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf127606)
{
    createSwDoc(DATA_DIRECTORY, "tdf117923.docx");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check that we actually test the line we need
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]", "GHI GHI GHI GHI");
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nType",
                "PortionType::Number");
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "rText", "2.");
    // The numbering height was 960 in DOC format.
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nHeight", "220");

    // tdf#127606: now it's possible to change formatting of numbering
    // increase font size (220 -> 260)
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Grow", {});
    pViewShell->Reformat();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]/Special", "nHeight", "260");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf127118)
{
    createSwDoc(DATA_DIRECTORY, "tdf127118.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was Horizontal: merged cell split between pages didn't keep vertical writing direction
    assertXPath(pXmlDoc, "/root/page[2]/body/tab/row[1]/cell[1]/txt[1]", "WritingMode", "VertBTLR");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf141220)
{
    createSwDoc(DATA_DIRECTORY, "tdf141220.docx");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "top").toInt32();
    sal_Int32 nTextBoxTop = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "top").toInt32();
    // Make sure the textbox stays inside the shape.
    CPPUNIT_ASSERT_LESS(static_cast<sal_Int32>(15), nTextBoxTop - nShapeTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf121509)
{
    auto pDoc = createSwDoc(DATA_DIRECTORY, "Tdf121509.odt");
    CPPUNIT_ASSERT(pDoc);

    // Get all shape/frame formats
    auto vFrameFormats = pDoc->GetSpzFrameFormats();
    // Get the textbox
    auto xTextFrame = SwTextBoxHelper::getUnoTextFrame(getShape(1));
    // Get The triangle
    auto pTriangleShapeFormat = vFrameFormats->GetFormat(2);
    CPPUNIT_ASSERT(xTextFrame);
    CPPUNIT_ASSERT(pTriangleShapeFormat);

    // Get the position inside the textbox
    auto xTextContentStart = xTextFrame->getText()->getStart();
    SwUnoInternalPaM aCursor(*pDoc);
    CPPUNIT_ASSERT(sw::XTextRangeToSwPaM(aCursor, xTextContentStart));

    // Put the triangle into the textbox
    SwFormatAnchor aNewAnch(pTriangleShapeFormat->GetAnchor());
    aNewAnch.SetAnchor(aCursor.Start());
    CPPUNIT_ASSERT(pTriangleShapeFormat->SetFormatAttr(aNewAnch));

    // Reload (docx)
    auto aTemp = utl::TempFile();
    save("Office Open XML Text", aTemp);

    // The second part: check if the reloaded doc has flys inside a fly
    uno::Reference<lang::XComponent> xComponent
        = loadFromDesktop(aTemp.GetURL(), "com.sun.star.text.TextDocument");
    uno::Reference<text::XTextDocument> xTextDoc(xComponent, uno::UNO_QUERY);
    auto pTextDoc = dynamic_cast<SwXTextDocument*>(xTextDoc.get());
    CPPUNIT_ASSERT(pTextDoc);
    auto pSecondDoc = pTextDoc->GetDocShell()->GetDoc();
    auto pSecondFormats = pSecondDoc->GetSpzFrameFormats();

    bool bFlyInFlyFound = false;
    for (auto secondformat : *pSecondFormats)
    {
        auto& pNd = secondformat->GetAnchor().GetContentAnchor()->nNode.GetNode();
        if (pNd.FindFlyStartNode())
        {
            // So there is a fly inside another -> problem.
            bFlyInFlyFound = true;
            break;
        }
    }
    // Drop the tempfile
    aTemp.CloseStream();

    // With the fix this cannot be true, if it is, that means Word unable to read the file..
    CPPUNIT_ASSERT_MESSAGE("Corrupt exported docx file!", !bFlyInFlyFound);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134685)
{
    createSwDoc(DATA_DIRECTORY, "tdf134685.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nWidth
        = getXPath(pXmlDoc, "/root/page/body/tab/row[6]/cell[1]/infos/bounds", "width").toInt32();
    // This was 2223: the content was only partially visible according to the lost cell width
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(14000), nWidth);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf109077)
{
    createSwDoc(DATA_DIRECTORY, "tdf109077.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "top").toInt32();
    sal_Int32 nTextBoxTop = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "top").toInt32();
    // This was 281: the top of the shape and its textbox should match, though
    // tolerate differences <= 1px (about 15 twips).
    CPPUNIT_ASSERT_LESS(static_cast<sal_Int32>(15), nTextBoxTop - nShapeTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testUserFieldTypeLanguage)
{
    // Set the system locale to German, the document will be English.
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString("de-DE");
    aOptions.Commit();
    comphelper::ScopeGuard g1([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "user-field-type-language.fodt");
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->UpdateFields();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was "123,456.00", via a buggy 1234.56 -> 1234,56 -> 123456 ->
    // 123,456.00 transform chain.
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[@nType='PortionType::Field']", "rText",
                "1,234.56");

    discardDumpedLayout();
    // Now change the system locale to English (before this was failing, 1234,56 -> 0.00)
    aOptions.SetLocaleConfigString("en-GB");
    aOptions.Commit();
    comphelper::ScopeGuard g2([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });
    pViewShell->UpdateFields();
    pXmlDoc = parseLayoutDump();
    // We expect, that the field value is not changed. Otherwise there is a problem:
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[@nType='PortionType::Field']", "rText",
                "1,234.56");
    discardDumpedLayout();
    // Now change the system locale to German
    aOptions.SetLocaleConfigString("de-DE");
    aOptions.Commit();
    comphelper::ScopeGuard g3([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });
    pViewShell->UpdateFields();
    pXmlDoc = parseLayoutDump();
    // We expect, that the field value is not changed. Otherwise there is a problem:
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[@nType='PortionType::Field']", "rText",
                "1,234.56");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
