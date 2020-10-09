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
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
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
#include <comphelper/sequence.hxx>

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
#include <IDocumentSettingAccess.hxx>
#include <config_features.h>
#include <unotxdoc.hxx>
#include <rootfrm.hxx>
#include <docsh.hxx>
#include <IDocumentLayoutAccess.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/extras/layout/data/";

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter2 : public SwModelTestBase
{
protected:
    void CheckRedlineCharAttributesHidden();

    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* SwLayoutWriter2::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

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
    createDoc("redline_charatr.fodt");
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
    createDoc("redline_footnote_pagination.fodt");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineNumberInNumbering)
{
    SwDoc* pDoc = createDoc("tdf42748.fodt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Assert the tracked deletion of the number of joined list item and
    // the tracked insertion of the number after a split list item as not black elements
    assertXPath(pXmlDoc, "/metafile/push/push/push/textcolor[not(@color='#000000')]", 6);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125300)
{
    SwDoc* pDoc = createDoc("tdf125300.docx");
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
    SwDoc* pDoc = createDoc("tdf116830.odt");
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
    SwDoc* pDoc = createDoc("tdf114163.odt");
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
    createDoc("tdf131707_flyWrap.doc");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//body/tab/row[3]/cell[2]/txt/infos/bounds", "top", "2185");
    // the image should be inside of the cell boundary - so the same top or higher
    assertXPath(pXmlDoc, "//body/tab/row[3]/cell[2]/txt/anchored/fly/infos/bounds", "top", "2185");
}

#if HAVE_MORE_FONTS
CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122225)
{
    SwDoc* pDoc = createDoc("tdf122225.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc,
                       "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/textarray[8]/text",
                       "Advanced Diploma");
    // This failed, if the legend label is not "Advanced Diploma".
}
#endif

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125335)
{
    SwDoc* pDoc = createDoc("tdf125335.odt");
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
    SwDoc* pDoc = createDoc("legend-itemorder-min.docx");
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
    SwDoc* pDoc = createDoc("tdf75659.docx");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf126425)
{
    SwDoc* pDoc = createDoc("long_legendentry.docx");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf123268)
{
    SwDoc* pDoc = createDoc("tdf123268.odt");
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
    SwDoc* pDoc = createDoc("tdf133005.odt");
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
    SwDoc* pDoc = createDoc("tdf115630.docx");
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
    SwDoc* pDoc = createDoc("tdf108021.odt");
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
    SwDoc* pDoc = createDoc("tdf125334.odt");
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
    SwDoc* pDoc = createDoc("tdf122800.odt");
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
    SwDoc* pDoc = createDoc("testTruncatedAxisLabel.odt");
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
    SwDoc* pDoc = createDoc("tdf128996.docx");
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
    SwDoc* pDoc = createDoc("tdf126244.docx");
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
    CPPUNIT_ASSERT(nRotation >= 899 && nRotation <= 900);
    // Test the third level of vertical category axis labels orientation. The third level orientation should be vertical.
    nRotation = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[4]/push[1]/font[7]",
                         "orientation")
                    .toInt32();
    CPPUNIT_ASSERT(nRotation >= 899 && nRotation <= 900);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf127304)
{
    SwDoc* pDoc = createDoc("tdf127304.odt");
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
    CPPUNIT_ASSERT(nRotation >= 899 && nRotation <= 900);
    // Test the second level of horizontal category axis labels orientation. The second level orientation should be horizontal.
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/font[5]", "orientation",
                "0");
    // Test the third level of horizontal category axis labels orientation. The third level orientation should be horizontal.
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/font[7]", "orientation",
                "0");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testHorizontal_multilevel)
{
    SwDoc* pDoc = createDoc("horizontal_multilevel.odt");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf124796)
{
    SwDoc* pDoc = createDoc("tdf124796.odt");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130969)
{
    SwDoc* pDoc = createDoc("tdf130969.docx");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf129054)
{
    SwDoc* pDoc = createDoc("tdf129054.docx");
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
    SwDoc* pDoc = createDoc("testAreaChartNumberFormat.docx");
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
    SwDoc* pDoc = createDoc("tdf134866.docx");
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
    SwDoc* pDoc = createDoc("tdf137116.docx");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130031)
{
    SwDoc* pDoc = createDoc("tdf130031.docx");
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
    SwDoc* pDoc = createDoc("tdf130242.odt");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130380)
{
    SwDoc* pDoc = createDoc("tdf130380.docx");
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
    SwDoc* pDoc = createDoc("tdf129095.docx");
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
    SwDoc* pDoc = createDoc("tdf132956.docx");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122014)
{
    SwDoc* pDoc = createDoc("tdf122014.docx");
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
    SwDoc* pDoc = createDoc("tdf134659.docx");
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
    SwDoc* pDoc = createDoc("tdf134235.docx");
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
    SwDoc* pDoc = createDoc("tdf134676.docx");
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
    SwDoc* pDoc = createDoc("tdf134146.docx");
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
    SwDoc* pDoc = createDoc("tdf136061.docx");
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
    SwDoc* pDoc = createDoc("tdf116925.docx");
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
    SwDoc* pDoc = createDoc("tdf117028.docx");
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
    SwDoc* pDoc = createDoc("tdf106390.odt");
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
    SwDoc* pDoc = createDoc("table-extrusion1.odt");
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
    SwDoc* pDoc = createDoc("table-extrusion2.odt");
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
    SwDoc* pDoc = createDoc("tdf116848.odt");
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf117245)
{
    createDoc("tdf117245.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin did not use a single line when there was
    // enough space for the text.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 1);

    // This was 2, same problem elsewhere due to code duplication.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/LineBreak", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf118672)
{
    createDoc("tdf118672.odt");
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
    createDoc("tdf117923.doc");
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
    createDoc("tdf117923.docx");
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
    createDoc("tdf127118.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was Horizontal: merged cell split between pages didn't keep vertical writing direction
    assertXPath(pXmlDoc, "/root/page[2]/body/tab/row[1]/cell[1]/txt[1]", "WritingMode", "VertBTLR");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134685)
{
    createDoc("tdf134685.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nWidth
        = getXPath(pXmlDoc, "/root/page/body/tab/row[6]/cell[1]/infos/bounds", "width").toInt32();
    // This was 2223: the content was only partially visible according to the lost cell width
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(14000), nWidth);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf109077)
{
    createDoc("tdf109077.docx");
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
    comphelper::ScopeGuard g([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    SwDoc* pDoc = createDoc("user-field-type-language.fodt");
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->UpdateFields();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was "123,456.00", via a buggy 1234.56 -> 1234,56 -> 123456 ->
    // 123,456.00 transform chain.
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[@nType='PortionType::Field']", "rText",
                "1,234.56");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf109137)
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
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 0, the blue rectangle moved from the 1st to the 2nd page.
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly/notxt",
                /*nNumberOfNodes=*/1);
}

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testForcepoint72) { createDoc("forcepoint72-1.rtf"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testForcepoint75) { createDoc("forcepoint75-1.rtf"); }

//just care it doesn't crash/assert
CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testForcepointFootnoteFrame)
{
    createDoc("forcepoint-swfootnoteframe-1.rtf");
}

//FIXME: disabled after failing again with fixed layout
//CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testForcepoint76) { createDoc("forcepoint76-1.rtf"); }

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf118058)
{
    SwDoc* pDoc = createDoc("tdf118058.fodt");
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf128611)
{
    createDoc("tdf128611.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 14
    // i.e. there were multiple portions in the first paragraph of the A1 cell, which means that the
    // rotated text was broken into multiple lines without a good reason.
    assertXPath(pXmlDoc, "//tab/row/cell[1]/txt/Text", "Portion", "Abcd efghijkl");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125893)
{
    createDoc("tdf125893.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 400. The paragraph must have zero top border.
    assertXPath(pXmlDoc, "/root/page/body/txt[4]/infos/prtBounds", "top", "0");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134463)
{
    createDoc("tdf134463.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 621. The previous paragraph must have zero bottom border.
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/infos/prtBounds", "top", "21");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf117188)
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
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    OUString sWidth = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "width");
    OUString sHeight = getXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/bounds", "height");
    // The text box must have zero border distances
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "left", "0");
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "top", "0");
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "width", sWidth);
    assertXPath(pXmlDoc, "/root/page/body/txt/anchored/fly/infos/prtBounds", "height", sHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf117187)
{
    createDoc("tdf117187.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // there should be no fly portions
    assertXPath(pXmlDoc, "/root/page/body/txt/Special[@nType='PortionType::Fly']", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf119875)
{
    createDoc("tdf119875.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFirstTop
        = getXPath(pXmlDoc, "/root/page[2]/body/section[1]/infos/bounds", "top").toInt32();
    sal_Int32 nSecondTop
        = getXPath(pXmlDoc, "/root/page[2]/body/section[2]/infos/bounds", "top").toInt32();
    // The first section had the same top value as the second one, so they
    // overlapped.
    CPPUNIT_ASSERT_LESS(nSecondTop, nFirstTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf120287)
{
    createDoc("tdf120287.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin Word-specific compat flag did not imply
    // default-in-Word printer-independent layout, resulting in an additional
    // line break.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf106234)
{
    createDoc("tdf106234.fodt");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // In justified paragraphs, there is justification between left tabulators and manual line breaks
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/Special", "nType", "PortionType::Margin");
    assertXPathNoAttribute(pXmlDoc, "/root/page/body/txt[1]/Special", "nWidth");
    // but not after centered, right and decimal tabulators
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/Special", "nType", "PortionType::Margin");
    // This was a justified line, without nWidth
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/Special", "nWidth", "7881");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf120287b)
{
    createDoc("tdf120287b.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 1418, TabOverMargin did the right split of the paragraph to two
    // lines, but then calculated a too large tab portion size on the first
    // line.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/Text[@nType='PortionType::TabRight']", "nWidth",
                "17");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf120287c)
{
    createDoc("tdf120287c.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2, the second line was not broken into a 2nd and a 3rd one,
    // rendering text outside the paragraph frame.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122878)
{
    createDoc("tdf122878.docx");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf115094)
{
    createDoc("tdf115094.docx");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf112290)
{
    SwDoc* pDoc = createDoc("tdf112290.docx");
    CPPUNIT_ASSERT(pDoc);
    auto pXml = parseLayoutDump();
    assertXPath(pXml, "/root/page/body/txt/LineBreak[2]", "Line", "Xxxx Xxxx");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122607)
{
    createDoc("tdf122607.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122607_regression)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testBtlrCell)
{
    SwDoc* pDoc = createDoc("btlr-cell.odt");
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
    SwCursorMoveState aState(CursorMoveState::NONE);
    pLayout->GetModelPositionForViewPoint(&aPosition, aPoint, &aState);
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf123898)
{
    createDoc("tdf123898.odt");

    // Make sure spellchecker has done its job already
    Scheduler::ProcessEventsToIdle();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure that the arrow on the left is not there (there are 43 children if it's there)
    assertXPathChildren(pXmlDoc, "/root/page/body/txt/anchored/fly/txt", 42);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf123651)
{
    createDoc("tdf123651.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with 'Expected: 7639;
    // Actual: 12926'. The shape was below the second "Lorem ipsum" text, not above it.
    const sal_Int32 nTopValue = getXPath(pXmlDoc, "//SwAnchoredDrawObject/bounds", "top").toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7639, nTopValue, 10);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf116501)
{
    //just care it doesn't freeze
    createDoc("tdf116501.odt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testAbi11870)
{
    //just care it doesn't assert
    createDoc("abi11870-2.odt");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf118719)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTabOverMargin)
{
    createDoc("tab-over-margin.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // 2nd paragraph has a tab over the right margin, and with the TabOverMargin compat option,
    // there is enough space to have all content in a single line.
    // Without the accompanying fix in place, this test would have failed, there were 2 lines.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/LineBreak", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testImageComment)
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
    pTextFrame->GetModelPositionForViewPoint(&aPosition, aPoint);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5
    // - Actual  : 4
    // i.e. the cursor got positioned between the image and its comment, so typing extended the
    // comment, instead of adding content after the commented image.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), aPosition.nContent.GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf64222)
{
    createDoc("tdf64222.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/Special", "nHeight", "560");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf113014)
{
    SwDoc* pDoc = createDoc("tdf113014.fodt");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130218)
{
    SwDoc* pDoc = createDoc("tdf130218.fodt");
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if hanging first line was hidden
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text", "Text");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf127235)
{
    SwDoc* pDoc = createDoc("tdf127235.odt");
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testShapeAllowOverlap)
{
// Need to find out why this fails on macOS and why this is unstable on Windows.
#if !defined(MACOSX) && !defined(_WIN32)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testShapeAllowOverlapWrap)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf124600)
{
    createDoc("tdf124600.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the last line in the body text had 2 lines, while it should have 1, as Word does (as the
    // fly frame does not intersect with the print area of the paragraph.)
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/LineBreak", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf124601)
{
    // This is a testcase for the ContinuousEndnotes compat flag.
    // The document has 2 pages, the endnote anchor is on the first page.
    // The endnote should be on the 2nd page together with the last page content.
    createDoc("tdf124601.doc");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. there was a separate endnote page, even when the ContinuousEndnotes compat option was
    // on.
    assertXPath(pXmlDoc, "/root/page", 2);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf124601b)
{
    // Table has an image, which is anchored in the first row, but its vertical position is large
    // enough to be rendered in the second row.
    // The shape has layoutInCell=1, so should match what Word does here.
    // Also the horizontal position should be in the last column, even if the anchor is in the
    // last-but-one column.
    createDoc("tdf124601b.doc");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf124770)
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

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the italic string was broken into 2 lines, while Word kept it in a single line.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/LineBreak", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testContinuousEndnotesInsertPageAtStart)
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
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 3);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the footnote container remained on page 2.
    assertXPath(pXmlDoc, "/root/page[3]/ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testContinuousEndnotesDeletePageAtStart)
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
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the endnote remained on an (otherwise) empty 2nd page.
    assertXPath(pXmlDoc, "/root/page", 1);
    assertXPath(pXmlDoc, "/root/page[1]/ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf128399)
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
    SwCursorMoveState aState(CursorMoveState::NONE);
    pLayout->GetModelPositionForViewPoint(&aPosition, aPoint, &aState);
    // Second row is +3: end node, start node and the first text node in the 2nd row.
    sal_uLong nExpected = aFirstRow.nNode.GetIndex() + 3;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14
    // - Actual  : 11
    // i.e. clicking on the center of the 2nd row placed the cursor in the 1st row.
    CPPUNIT_ASSERT_EQUAL(nExpected, aPosition.nNode.GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf105481)
{
    createDoc("tdf105481.odt");
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf117982)
{
    SwDoc* pDocument = createDoc("tdf117982.docx");
    SwDocShell* pShell = pDocument->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/textarray[1]/text", "FOO AAA");
    //The first cell must be "FOO AAA". If not, this means the first cell content not visible in
    //the source document.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf128959)
{
    // no orphan/widow control in table cells
    SwDoc* pDocument = createDoc("tdf128959.docx");
    CPPUNIT_ASSERT(pDocument);
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // first two lines of the paragraph in the split table cell on the first page
    // (these lines were completely lost)
    assertXPath(
        pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/LineBreak[1]", "Line",
        "a)Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Maecenas porttitor congue ");
    assertXPath(
        pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[1]/txt[1]/LineBreak[2]", "Line",
        "massa. Fusce posuere, magna sed pulvinar ultricies, purus lectus malesuada libero, sit ");
    // last line of the paragraph in the split table cell on the second page
    assertXPath(pXmlDoc, "/root/page[2]/body/tab[1]/row[1]/cell[1]/txt[1]/LineBreak[1]", "Line",
                "amet commodo magna eros quis urna.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf121658)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    createDoc("tdf121658.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Only 2 hyphenated words should appear in the document (in the lowercase words).
    // Uppercase words should not be hyphenated.
    assertXPath(pXmlDoc, "//Special[@nType='PortionType::Hyphen']", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testWriterImageNoCapture)
{
    createDoc("writer-image-no-capture.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nPageLeft = getXPath(pXmlDoc, "//page/infos/bounds", "left").toInt32();
    sal_Int32 nImageLeft = getXPath(pXmlDoc, "//fly/infos/bounds", "left").toInt32();
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
    CPPUNIT_ASSERT(pPage->getFrameArea().IsInside(aFlyRect));
    return aFlyRect;
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testStableAtPageAnchoredFlyPosition)
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134548)
{
    createDoc("tdf134548.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf124423)
{
    createDoc("tdf124423.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFly1Width = getXPath(pXmlDoc, "(//fly)[1]/infos/prtBounds", "width").toInt32();
    sal_Int32 nFly2Width = getXPath(pXmlDoc, "(//fly)[2]/infos/prtBounds", "width").toInt32();
    sal_Int32 nPageWidth = getXPath(pXmlDoc, "//page/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_EQUAL(nPageWidth, nFly2Width);
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly1Width);

    createDoc("tdf124423.odt");
    pXmlDoc = parseLayoutDump();
    nFly1Width = getXPath(pXmlDoc, "(//fly)[1]/infos/prtBounds", "width").toInt32();
    nFly2Width = getXPath(pXmlDoc, "(//fly)[2]/infos/prtBounds", "width").toInt32();
    nPageWidth = getXPath(pXmlDoc, "//page/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly2Width);
    CPPUNIT_ASSERT_LESS(nPageWidth / 2, nFly1Width);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf135035)
{
    createDoc("tdf135035.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFly1Width = getXPath(pXmlDoc, "(//fly)[1]/infos/prtBounds", "width").toInt32();
    sal_Int32 nFly2Width = getXPath(pXmlDoc, "(//fly)[2]/infos/prtBounds", "width").toInt32();
    sal_Int32 nFly3Width = getXPath(pXmlDoc, "(//fly)[3]/infos/prtBounds", "width").toInt32();
    sal_Int32 nParentWidth = getXPath(pXmlDoc, "(//txt)[1]/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_EQUAL(nParentWidth, nFly2Width);
    CPPUNIT_ASSERT_EQUAL(nParentWidth, nFly3Width);
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly1Width);

    createDoc("tdf135035.odt");
    pXmlDoc = parseLayoutDump();
    nFly1Width = getXPath(pXmlDoc, "(//fly)[1]/infos/prtBounds", "width").toInt32();
    nFly2Width = getXPath(pXmlDoc, "(//fly)[2]/infos/prtBounds", "width").toInt32();
    nFly3Width = getXPath(pXmlDoc, "(//fly)[3]/infos/prtBounds", "width").toInt32();
    nParentWidth = getXPath(pXmlDoc, "(//txt)[1]/infos/prtBounds", "width").toInt32();
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly2Width);
    CPPUNIT_ASSERT_LESS(nParentWidth / 2, nFly1Width);
    CPPUNIT_ASSERT_GREATER(nParentWidth, nFly3Width);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
