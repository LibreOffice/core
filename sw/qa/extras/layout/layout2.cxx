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
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include <comphelper/scopeguard.hxx>
#include <comphelper/sequence.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <editeng/unolingu.hxx>
#include <o3tl/string_view.hxx>

#include <unotxdoc.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <wrtsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <frameformats.hxx>
#include <unotextrange.hxx>
#include <fmtanchr.hxx>

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter2 : public SwModelTestBase
{
public:
    SwLayoutWriter2()
        : SwModelTestBase("/sw/qa/extras/layout/data/")
    {
    }

protected:
    void CheckRedlineCharAttributesHidden();
};

// this mainly tests that splitting portions across redlines in SwAttrIter works
void SwLayoutWriter2::CheckRedlineCharAttributesHidden()
{
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged"_ostr, "paraPropsNodeIndex"_ostr, "9");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]"_ostr,
                "type"_ostr, "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]"_ostr,
                "portion"_ostr, "foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/merged"_ostr, "paraPropsNodeIndex"_ostr, "10");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]"_ostr,
                "type"_ostr, "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]"_ostr,
                "portion"_ostr, "foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/merged"_ostr, "paraPropsNodeIndex"_ostr, "11");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/merged"_ostr, "paraPropsNodeIndex"_ostr, "12");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/merged"_ostr, "paraPropsNodeIndex"_ostr, "13");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/merged"_ostr, "paraPropsNodeIndex"_ostr, "14");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/merged"_ostr, "paraPropsNodeIndex"_ostr, "15");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/merged"_ostr, "paraPropsNodeIndex"_ostr, "16");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/merged"_ostr, "paraPropsNodeIndex"_ostr, "17");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwParaPortion[1]"_ostr,
                "type"_ostr, "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwParaPortion[1]"_ostr,
                "portion"_ostr, "foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/merged"_ostr, "paraPropsNodeIndex"_ostr, "18");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "fo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "ob");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/merged"_ostr, "paraPropsNodeIndex"_ostr, "19");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwParaPortion[1]"_ostr,
                "type"_ostr, "PortionType::Para");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwParaPortion[1]"_ostr,
                "portion"_ostr, "foobaz");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf148897)
{
    createSwDoc("tdf148897.odt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/sorted_objs/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/sorted_objs/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[3]/anchored/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[3]/sorted_objs/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt/anchored/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[4]/sorted_objs/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[4]/body/txt[1]/anchored/fly"_ostr, 1);
    // fly portion exists, no overlapping text
    assertXPath(pXmlDoc,
                "/root/page[4]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwFixPortion"_ostr,
                "height"_ostr, "5797");
    assertXPath(pXmlDoc, "/root/page[5]/sorted_objs/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page"_ostr, 5);

    auto xModel = mxComponent.queryThrow<frame::XModel>();
    uno::Reference<drawing::XShape> xShape(getShapeByName(u"Image3"));
    uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(), uno::UNO_QUERY);
    xCtrl->select(uno::Any(xShape));

    dispatchCommand(mxComponent, ".uno:Delete", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/sorted_objs/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[2]/sorted_objs/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[3]/sorted_objs/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt[2]/anchored/fly"_ostr, 1);
    // fly portion exists, no overlapping text
    assertXPath(pXmlDoc,
                "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwFixPortion"_ostr,
                "height"_ostr, "5797");
    assertXPath(pXmlDoc, "/root/page[4]/sorted_objs/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[4]/body/txt/anchored/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page"_ostr, 4);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/sorted_objs/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/sorted_objs/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[3]/anchored/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[3]/sorted_objs/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt/anchored/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[4]/sorted_objs/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[4]/body/txt[1]/anchored/fly"_ostr, 1);
    // fly portion exists, no overlapping text
    assertXPath(pXmlDoc,
                "/root/page[4]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwFixPortion"_ostr,
                "height"_ostr, "5797");
    assertXPath(pXmlDoc, "/root/page[5]/sorted_objs/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page"_ostr, 5);

    dispatchCommand(mxComponent, ".uno:Redo", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/sorted_objs/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[2]/sorted_objs/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[3]/sorted_objs/fly"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt[2]/anchored/fly"_ostr, 1);
    // fly portion exists, no overlapping text
    assertXPath(pXmlDoc,
                "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwFixPortion"_ostr,
                "height"_ostr, "5797");
    assertXPath(pXmlDoc, "/root/page[4]/sorted_objs/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page[4]/body/txt/anchored/fly"_ostr, 0);
    assertXPath(pXmlDoc, "/root/page"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineCharAttributes)
{
    createSwDoc("redline_charatr.fodt");
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
    assertXPath(pXmlDoc, "//merged"_ostr, 0);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "ba");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "r");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[4]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[4]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "baz");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "fo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "o");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "bar");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[4]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[4]"_ostr,
                "portion"_ostr, "b");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[5]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[5]"_ostr,
                "portion"_ostr, "az");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[1]"_ostr,
                "portion"_ostr, "foo");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[2]"_ostr,
                "portion"_ostr, "b");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[3]"_ostr,
                "portion"_ostr, "a");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[4]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[4]"_ostr,
                "portion"_ostr, "r");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[5]"_ostr,
                "type"_ostr, "PortionType::Text");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[5]"_ostr,
                "portion"_ostr, "baz");

    // verify after hide
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
    CheckRedlineCharAttributesHidden();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineShowHideFootnotePagination)
{
    createSwDoc("redline_footnote_pagination.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc(pTextDoc->GetDocShell()->GetDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // check footnotes
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn"_ostr, 6);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn"_ostr, 3);
    // check that first page ends with the y line and second page starts with z
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[last()]/SwParaPortion/SwLineLayout[last()]"_ostr,
                "portion"_ostr,
                "yyyyyyyyy yyy yyyyyyyyyyyyyyyy yyyyyyy yyy yyyyy yyyyyyyyy yyy yyyyyyyyy ");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr,
                "zzz. zzz zzzz zzzz* zzz zzz zzzzzzz zzz zzzz zzzzzzzzzzzzzz zzzzzzzzzzzz ");

    // hide redlines - all still visible footnotes move to page 1
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn"_ostr, 0);

    // show again - should now get the same result as on loading
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    // check footnotes
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn"_ostr, 6);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn"_ostr, 3);
    // check that first page ends with the y line and second page starts with z
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[last()]/SwParaPortion/SwLineLayout[last()]"_ostr,
                "portion"_ostr,
                "yyyyyyyyy yyy yyyyyyyyyyyyyyyy yyyyyyy yyy yyyyy yyyyyyyyy yyy yyyyyyyyy ");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr,
                "zzz. zzz zzzz zzzz* zzz zzz zzzzzzz zzz zzzz zzzzzzzzzzzzzz zzzzzzzzzzzz ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testtdf138951)
{
    // Open the bugdoc
    createSwDoc("tdf138951.odt");
    auto pDoc = getSwDoc();

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
        = *pShFrm->GetAnchor().GetAnchorNode() == *pShFrm->GetAnchor().GetAnchorNode();
    CPPUNIT_ASSERT_MESSAGE("The anchor is different for the textbox and shape!", bIsAnchTheSame);
    CPPUNIT_ASSERT_MESSAGE("The textbox has fallen apart!", aShpRect.Contains(aTxtFrmRect));
    // Without the fix the anchor differs, and the frame outside of the shape
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf150717)
{
    createSwDoc("tdf150717.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // check bookmark colors defined in metadata
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[2]"_ostr,
                "colors"_ostr, "#Bookmark1 Bookmark Start");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[4]"_ostr,
                "colors"_ostr, "#Bookmark2 Bookmark Start");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[6]"_ostr,
                "colors"_ostr, "#Bookmark2 Bookmark End#Bookmark1 Bookmark End");
    // full text, if bookmarks are visible
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[1]"_ostr,
                "portion"_ostr, "Lorem ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[3]"_ostr,
                "portion"_ostr, "ipsum dolor et ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[5]"_ostr,
                "portion"_ostr, "ames");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[7]"_ostr,
                "portion"_ostr, ".");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf150790)
{
    createSwDoc("tdf150790.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // point bookmark is shown as I-beam (only its text dump is |, as before on the screen)
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/SwBookmarkPortion"_ostr,
                "colors"_ostr, "#Bookmark 1 Bookmark");
    // single start bookmark
    assertXPath(pXmlDoc,
                "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwBookmarkPortion[1]"_ostr,
                "colors"_ostr, "#Bookmark 2 Bookmark Start");
    // single end bookmark
    assertXPath(pXmlDoc,
                "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwBookmarkPortion[3]"_ostr,
                "colors"_ostr, "#Bookmark 3 Bookmark End");
    // This was |, as before the point bookmark (neighboring end and start bookmarks)
    assertXPath(pXmlDoc,
                "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwBookmarkPortion[2]"_ostr,
                "colors"_ostr, "#Bookmark 2 Bookmark End#Bookmark 3 Bookmark Start");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf129357)
{
    createSwDoc("tdf129357.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // visible soft hyphen
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/push/push/textarray[2]/text"_ostr, "-");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineNumberInNumbering)
{
    createSwDoc("tdf42748.fodt");
    SwDoc* pDoc = getSwDoc();
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
        "/metafile/push/push/push/textcolor[not(@color='#000000') and not(@color='#008000')]"_ostr,
        5);

    // tdf#145068 numbering shows changes in the associated list item, not the next one
    // This was 1 (black numbering of the first list item previously)
    assertXPath(pXmlDoc, "/metafile/push/push/push/font[4][@color='#000000']"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineNumbering)
{
    createSwDoc("tdf115523.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Show the correct and the original line numbering instead of counting
    // the deleted list items in Show Changes mode, as part of the list
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text"_ostr, "1.");
    // This was "2." (deleted text node, now its text content is part of the first list item)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text"_ostr, "[2.] ");
    // This was "3." (now it's the second list item)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[5]/text"_ostr, "2.[3.] ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineNumbering2)
{
    createSwDoc("tdf115524.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Show the correct and the original line numbering in Show Changes mode
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text"_ostr, "1.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text"_ostr, "2.");
    // FIXME: show as 3.[2.]
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[5]/text"_ostr, "3.");
    // This was "4." (not shown the original number)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[7]/text"_ostr, "4.[3.] ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf149710_RedlineNumberingEditing)
{
    createSwDoc("tdf149710.fodt");
    SwDoc* pDoc = getSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // Show Changes
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    // delete the paragraph mark of the first list item with change tracking
    dispatchCommand(mxComponent, ".uno:GoToEndOfLine", {});
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = pDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Show the correct and the original line numbering instead of counting
    // the deleted list items in Show Changes mode, as part of the list
    // This was "1."
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text"_ostr, "[1.] ");
    // This was "2." (deleted text node, now its text content is part of the first list item)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text"_ostr, "1.[2.] ");
    // This was "3." (now it's the second list item)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[5]/text"_ostr, "2.[3.] ");

    // remove the tracked deletion, and check the layout again
    pWrtShell->Undo();
    xMetaFile = pShell->GetPreviewMetaFile();
    pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text"_ostr, "1.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text"_ostr, "2.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[5]/text"_ostr, "3.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf149709_RedlineNumberingLevel)
{
    createSwDoc("tdf149709.fodt");
    SwDoc* pDoc = getSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // Show Changes
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    // insert a new list item at start of the second list item "a)"
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    pWrtShell->Down(false, 1);
    pWrtShell->SplitNode(false);

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = pDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Show the correct and the original line numbering instead of counting
    // the deleted list items in Show Changes mode, as part of the list
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text"_ostr, "1.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text"_ostr, "a)");
    // This was "b)[2.]"
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[4]/text"_ostr, "b)[a)] ");
    // This was "c)[3.]"
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[6]/text"_ostr, "c)[b)] ");
    // This was "4.[2.]" (after disabling Show Changes, and enabling again)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[8]/text"_ostr, "2.");

    // remove the tracked deletion, and check the layout again
    pWrtShell->Undo();
    xMetaFile = pShell->GetPreviewMetaFile();
    pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text"_ostr, "1.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text"_ostr, "a)");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[5]/text"_ostr, "b)");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[7]/text"_ostr, "2.");

    // check Redo
    pWrtShell->Redo();
    xMetaFile = pShell->GetPreviewMetaFile();
    pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text"_ostr, "1.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text"_ostr, "a)");
    // TODO: show as b)[a)]
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[4]/text"_ostr, "b)");
    // FIXME: This must be "c)[b]"
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[6]/text"_ostr, "c)[a)] ");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[8]/text"_ostr, "2.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf149711_importDOCXMoveToParagraphMark)
{
    createSwDoc("tdf149711.docx");
    SwDoc* pDoc = getSwDoc();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 6);

    // reject tracked insertion (moveTo)
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->RejectRedline(1);

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 6 (not tracked paragraph mark of the moveTo list item)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf152872)
{
    createSwDoc("hidden-para-separator.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr,
                "C DE");
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion"_ostr, 0); // 5 is empty and hidden
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds"_ostr, "height"_ostr, "0");

    dispatchCommand(mxComponent, ".uno:ControlCodes", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 5);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr,
                "C ");
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr,
                "D");
    // 3 is an empty paragraph with RES_CHRATR_HIDDEN which results in 0-height
    // frame; ideally it should only be hidden when control codes are hidden
    // and be a full-height frame now, but that needs more work...
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/infos/bounds"_ostr, "height"_ostr, "0");
    assertXPath(pXmlDoc, "/root/page/body/txt[4]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr,
                "E");
    // 5 is an empty paragraph with RES_CHRATR_HIDDEN which results in 0-height
    // frame; ideally it should only be hidden when control codes are hidden
    // and be a full-height frame now, but that needs more work...
    assertXPath(pXmlDoc, "/root/page/body/txt[5]/SwParaPortion"_ostr, 0); // 5 is empty
    assertXPath(pXmlDoc, "/root/page/body/txt[5]/infos/bounds"_ostr, "height"_ostr, "0");

    dispatchCommand(mxComponent, ".uno:ControlCodes", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr,
                "C DE");
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion"_ostr, 0); // 5 is empty and hidden
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds"_ostr, "height"_ostr, "0");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf151954)
{
    createSwDoc("tdf151954.docx");
    SwDoc* pDoc = getSwDoc();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 2);

    // accept tracked insertion (moveTo)
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(3), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 1 (moveFrom was extended to the paragraph mark)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf152952)
{
    createSwDoc("Hyphenated-link.rtf");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // URL should not be hyphenated
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, " NNNNNNNNNN NNNNNNNNNNNNNNN ");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]"_ostr,
                "portion"_ostr,
                "https://example.com/xxxxxxx/testtesttesttest/hyphenate/testtesttest ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf152952_compat)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    createSwDoc("Hyphenated-link.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // URL hyphenated for backward compatibility
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr,
                " NNNNNNNNNN NNNNNNNNNNNNNNN https://example.com/xxxxxxx/testtesttesttest/hyphen");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]"_ostr,
                "portion"_ostr, "ate/testtesttest ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf158885_compound_remain)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("hu", "HU", OUString())))
        return;

    uno::Reference<linguistic2::XSpellChecker1> xSpell = LinguMgr::GetSpellChecker();
    LanguageType eLang = LanguageTag::convertToLanguageType(lang::Locale("hu", "HU", OUString()));
    if (!xSpell.is() || !xSpell->hasLanguage(static_cast<sal_uInt16>(eLang)))
        return;

    createSwDoc("tdf158885_compound-remain.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // hyphenate compound word with 3- or more character distance from the stem boundary
    // This was "emberel=lenes" (now "ember=ellenes", i.e. hyphenating at the stem boundary)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr,
                "emberellenes emberellenes emberellenes emberellenes emberellenes ember");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]"_ostr,
                "portion"_ostr,
                "ellenes emberellenes emberellenes emberellenes emberellenes emberellenes ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf158885_not_compound_remain)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("hu", "HU", OUString())))
        return;

    uno::Reference<linguistic2::XSpellChecker1> xSpell = LinguMgr::GetSpellChecker();
    LanguageType eLang = LanguageTag::convertToLanguageType(lang::Locale("hu", "HU", OUString()));
    if (!xSpell.is() || !xSpell->hasLanguage(static_cast<sal_uInt16>(eLang)))
        return;

    createSwDoc("tdf158885_not_compound-remain.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // hyphenate compound word with 2-character distance from the stem boundary,
    // resulting less readable hyphenation "emberel=lenes" ("emberel" and "lenes" have
    // different meanings, than the original word parts)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr,
                "emberellenes emberellenes emberellenes emberellenes emberellenes emberel");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]"_ostr,
                "portion"_ostr,
                "lenes emberellenes emberellenes emberellenes emberellenes emberellenes ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineNumberInFootnote)
{
    createSwDoc("tdf85610.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // changed color of numbers of footnote 1 (deleted footnote) and footnote 2 (inserted footnote)
    // decreased the black <font> elements by 2:
    // This was 7
    assertXPath(pXmlDoc, "/metafile/push/push/push/font[@color='#000000']"_ostr, 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineMoving)
{
    createSwDoc("tdf42748.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // create a 3-element list without change tracking
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
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
    // tdf#157663: the moved text item "It" is detected as text moving again!
    assertXPath(pXmlDoc, "/metafile/push/push/push/textcolor[@color='#008000']"_ostr, 5);
    assertXPath(pXmlDoc, "/metafile/push/push/push/font[@color='#008000']"_ostr, 11);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineMoving2)
{
    createSwDoc("tdf42748.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // create a 3-element list without change tracking
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    pEditShell->RejectRedline(0);
    pEditShell->AcceptRedline(0);

    // extend the first item to "An ItemIt", because detection of move needs
    // at least 6 characters with an inner space after stripping white spaces
    // of the redline
    dispatchCommand(mxComponent, ".uno:GoToStartOfDoc", {});
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("An Item");

    // move down first list item with track changes
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    dispatchCommand(mxComponent, ".uno:MoveDown", {});

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // text and numbering colors show moving of the list item
    // These were 0 (other color, not COL_GREEN, color of the tracked text movement)
    assertXPath(pXmlDoc, "/metafile/push/push/push/textcolor[@color='#008000']"_ostr, 5);
    assertXPath(pXmlDoc, "/metafile/push/push/push/font[@color='#008000']"_ostr, 11);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineMovingDOCX)
{
    createSwDoc("tdf104797.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    // This was 2 (moveFrom and moveTo joined other redlines)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(5), pEditShell->GetRedlineCount());

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // text colors show moved text
    // These were 0 (other color, not COL_GREEN, color of the tracked text movement)
    assertXPath(pXmlDoc, "/metafile/push/push/push/textcolor[@color='#008000']"_ostr, 6);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTableCellInvalidate)
{
    discardDumpedLayout();
    if (mxComponent.is())
        mxComponent->dispose();

    OUString const url(createFileURL(u"table_cell_overlap.fodt"));

    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue("Hidden", -1, uno::Any(true), beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    mxComponent = loadFromDesktop(url, "com.sun.star.text.TextDocument",
                                  comphelper::containerToSequence(aFilterOptions));
    save("writer_pdf_Export");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // somehow these 2 rows overlapped in the PDF unless CalcLayout() runs
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds"_ostr, "top"_ostr,
                "6969");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds"_ostr,
                "height"_ostr, "231");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/infos/bounds"_ostr,
                "top"_ostr, "6969");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/infos/bounds"_ostr,
                "height"_ostr, "231");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/txt[1]/infos/bounds"_ostr,
                "top"_ostr, "6969");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/txt[1]/infos/bounds"_ostr,
                "height"_ostr, "231");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds"_ostr, "top"_ostr,
                "7200");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds"_ostr,
                "height"_ostr, "231");
    // this was 6969, causing the overlap
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/infos/bounds"_ostr,
                "top"_ostr, "7200");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/infos/bounds"_ostr,
                "height"_ostr, "231");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/txt[1]/infos/bounds"_ostr,
                "top"_ostr, "7200");
    assertXPath(pXmlDoc,
                "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/txt[1]/infos/bounds"_ostr,
                "height"_ostr, "231");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf145719)
{
    createSwDoc("tdf145719.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // text colors show moved text
    // This was 0 (other color, not COL_GREEN, color of the tracked text movement)
    assertXPath(pXmlDoc, "/metafile/push/push/push/textcolor[@color='#008000']"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testChangedTableRows)
{
    createSwDoc("changed_table_rows.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was 0 (other color, not COL_AUTHOR_TABLE_DEL, color of the tracked row deletion)
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#fce6f4']"_ostr, 1);
    // This was 0 (other color, not COL_AUTHOR_TABLE_INS, color of the tracked row insertion)
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#e1f2fa']"_ostr, 1);
    // This was 3 (color of the cells of the last column, 2 of them disabled by change tracking )
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#3faf46']"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf155187_TableInTextChange)
{
    createSwDoc("table_in_text_change.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was 0 (other color, not COL_AUTHOR_TABLE_DEL, color of the tracked row deletion)
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#fce6f4']"_ostr, 2);
    // This was 0 (other color, not COL_AUTHOR_TABLE_INS, color of the tracked row insertion)
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#e1f2fa']"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf145225_RedlineMovingWithBadInsertion)
{
    createSwDoc("tdf42748.fodt");
    SwDoc* pDoc = getSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // create a 3-element list without change tracking
    // (because the fixed problem depends on the own changes)
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
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
    // positionate the cursor in the middle of the second list item
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf146964_ReappearingMovedTextInHideChangesMode)
{
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // enable Record Changes
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // set Hide Changes mode
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // delete and paste the deleted word again during Track Changes
    dispatchCommand(mxComponent, ".uno:WordRightSel", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // This was "Lorem Lorem ipsum" (reappearing deletion in Hide Changes mode)
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion"_ostr,
                "portion"_ostr, "Lorem ipsum");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125300)
{
    createSwDoc("tdf125300.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Keep line spacing before bottom cell border (it was 1892)
    sal_Int32 y1
        = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[7]/polyline/point[1]"_ostr,
                   "y"_ostr)
              .toInt32();
    sal_Int32 y2
        = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[7]/polyline/point[2]"_ostr,
                   "y"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2092, y1, 7);
    CPPUNIT_ASSERT_EQUAL(y1, y2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf116830)
{
    createSwDoc("tdf116830.odt");
    SwDoc* pDoc = getSwDoc();
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
        "/metafile/push[1]/push[1]/push[1]/push[2]/push[1]/push[1]/fillcolor[@color='#729fcf']"_ostr,
        1);
    assertXPath(pXmlDoc,
                "/metafile/push[1]/push[1]/push[1]/push[2]/push[1]/push[1]/polypolygon"_ostr, 1);

    // This failed: cell background was painted before the background shape.
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/fillcolor[@color='#ffff00']"_ostr, 1);
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/rect"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf114163)
{
    createSwDoc("tdf114163.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "(//textarray)[12]/text"_ostr, "Data3");
    // This failed, if the legend first label is not "Data3". The legend position is right.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf131707)
{
    createSwDoc("tdf131707_flyWrap.doc");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//body/tab/row[3]/cell[2]/txt/infos/bounds"_ostr, "top"_ostr, "2185");
    // the image should be inside of the cell boundary - so the same top or higher
    assertXPath(pXmlDoc, "//body/tab/row[3]/cell[2]/txt/anchored/fly/infos/bounds"_ostr, "top"_ostr,
                "2185");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122225)
{
    createSwDoc("tdf122225.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Bug 122225 - FILEOPEN DOCX Textbox of Column chart legend reduces and text of legend disappears
    const sal_Int32 nLegendLabelLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"Advanced Diploma\")])"_ostr)
              .toInt32();
    // This failed, if the legend label is not "Advanced Diploma".
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nLegendLabelLines);

    // Bug 140623 - Fileopen DOCX: Text Orientation of X-Axis 0 instead of 45 degrees
    const sal_Int32 nThirdLabelLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"Hispanic\")])"_ostr).toInt32();
    // This failed, if the third X axis label broke to multiple lines.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nThirdLabelLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125335)
{
    createSwDoc("tdf125335.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "(//textarray)[12]/text"_ostr, "Data3");
    // This failed, if the legend first label is not "Data3". The legend position is bottom.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134247)
{
    createSwDoc("legend-itemorder-min.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "(//textarray)[14]/text"_ostr, "1. adatsor");
    // This failed, if the legend first label is not "1. adatsor".
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf75659)
{
    createSwDoc("tdf75659.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "(//textarray)[17]/text"_ostr, "Series1");
    assertXPathContent(pXmlDoc, "(//textarray)[18]/text"_ostr, "Series2");
    assertXPathContent(pXmlDoc, "(//textarray)[19]/text"_ostr, "Series3");
    // These failed, if the legend names are empty strings.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf136816)
{
    createSwDoc("tdf136816.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check number of legend entries
    assertXPath(pXmlDoc, "//text[contains(text(),\"Column\")]"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf126425)
{
    createSwDoc("long_legendentry.docx");
    SwDoc* pDoc = getSwDoc();
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
    assertXPath(pXmlDoc, "//textarray"_ostr, 14);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testUnusedOLEprops)
{
    createSwDoc("tdf138465min.docx");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: >300
    // - Actual  : 142
    // i.e. the formula squashed
    CPPUNIT_ASSERT_GREATEREQUAL(
        double(300),
        getXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/fly/notxt/infos/bounds"_ostr,
                 "height"_ostr)
            .toDouble());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf123268)
{
    createSwDoc("tdf123268.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 53
    // - Actual  : 0
    // i.e. the chart lost.
    assertXPath(pXmlDoc,
                "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push"_ostr, 53);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf133005)
{
    createSwDoc("tdf133005.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    sal_Int32 nXChartWall = getXPath(pXmlDoc,
                                     "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/"
                                     "push[1]/push[1]/polyline[1]/point[2]"_ostr,
                                     "x"_ostr)
                                .toInt32();
    sal_Int32 nXColumn = getXPath(pXmlDoc,
                                  "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/"
                                  "push[1]/push[41]/polypolygon/polygon/point[1]"_ostr,
                                  "x"_ostr)
                             .toInt32();

    // This failed, if the value axis doesn't appear inside category.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nXChartWall, nXColumn, 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf115630)
{
    createSwDoc("tdf115630.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Test wide of inner chart area.
    sal_Int32 nXRight
        = getXPath(
              pXmlDoc,
              "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[3]/polyline[1]/point[1]"_ostr,
              "x"_ostr)
              .toInt32();
    sal_Int32 nXLeft
        = getXPath(
              pXmlDoc,
              "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[3]/polyline[1]/point[2]"_ostr,
              "x"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2895, nXRight - nXLeft, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf108021)
{
    createSwDoc("tdf108021.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//textarray[@length='22']"_ostr, 8);
    // This failed, if the textarray length of the first axis label not 22.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125334)
{
    createSwDoc("tdf125334.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    int nCount = countXPathNodes(pXmlDoc, "//textarray[@length='17']"_ostr);
    CPPUNIT_ASSERT_EQUAL(4, nCount);
    // This failed, if the textarray length of the category axis label not 17.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122800)
{
    createSwDoc("tdf122800.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//textarray[@length='22']"_ostr, 9);
    // This failed, if the textarray length of the first axis label not 22.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTruncatedAxisLabel)
{
    createSwDoc("testTruncatedAxisLabel.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // test the X axis label visibility
    assertXPathContent(pXmlDoc, "(//textarray)[1]/text"_ostr, "Long axis label truncated 1");

    // test the Y axis label visibility
    assertXPathContent(pXmlDoc, "(//textarray)[3]/text"_ostr, "-5.00");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf128996)
{
    createSwDoc("tdf128996.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "(//textarray)[1]/text"_ostr, "A very long category name 1");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf126244)
{
    createSwDoc("tdf126244.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the first level of vertical category axis labels orientation. The first level orientation should be horizontal.
    assertXPath(pXmlDoc, "(//font)[1]"_ostr, "orientation"_ostr, "0");
    // Test the second level of vertical category axis labels orientation. The second level orientation should be vertical.
    sal_Int32 nRotation = getXPath(pXmlDoc, "(//font)[5]"_ostr, "orientation"_ostr).toInt32();
    CPPUNIT_ASSERT(nRotation >= 899);
    CPPUNIT_ASSERT(nRotation <= 900);
    // Test the third level of vertical category axis labels orientation. The third level orientation should be vertical.
    nRotation = getXPath(pXmlDoc, "(//font)[7]"_ostr, "orientation"_ostr).toInt32();
    CPPUNIT_ASSERT(nRotation >= 899);
    CPPUNIT_ASSERT(nRotation <= 900);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf127304)
{
    createSwDoc("tdf127304.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the first level of horizontal category axis labels orientation. The first level orientation should be vertical.
    sal_Int32 nRotation = getXPath(pXmlDoc, "(//font)[1]"_ostr, "orientation"_ostr).toInt32();
    CPPUNIT_ASSERT(nRotation >= 899);
    CPPUNIT_ASSERT(nRotation <= 900);
    // Test the second level of horizontal category axis labels orientation. The second level orientation should be horizontal.
    assertXPath(pXmlDoc, "(//font)[5]"_ostr, "orientation"_ostr, "0");
    // Test the third level of horizontal category axis labels orientation. The third level orientation should be horizontal.
    assertXPath(pXmlDoc, "(//font)[7]"_ostr, "orientation"_ostr, "0");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testHorizontal_multilevel)
{
    createSwDoc("horizontal_multilevel.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the Y position of horizontal category axis label.
    sal_Int32 nYposition = getXPath(pXmlDoc, "(//textarray)[7]"_ostr, "y"_ostr).toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11248, nYposition, 20);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf69648)
{
    createSwDoc("tdf69648.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    OString sShapeXPath[2] = {
        "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObjGroup/SdrObjList/SdrObject[1]"_ostr,
        "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObjGroup/SdrObjList/SdrObject[2]"_ostr
    };
    OString sFrameXPath[2] = { "/root/page/body/txt/anchored/fly[1]/infos/bounds"_ostr,
                               "/root/page/body/txt/anchored/fly[2]/infos/bounds"_ostr };

    for (int i = 0; i < 2; ++i)
    {
        const auto sDrawRect = getXPath(pXmlDoc, sShapeXPath[i], "aOutRect"_ostr);
        const auto nComaPos1 = sDrawRect.indexOf(',', 0);
        const auto nComaPos2 = sDrawRect.indexOf(',', nComaPos1 + 1);
        const auto nComaPos3 = sDrawRect.indexOf(',', nComaPos2 + 1);

        const auto nDraw1 = o3tl::toInt64(sDrawRect.subView(0, nComaPos1));
        const auto nDraw2 = o3tl::toInt64(sDrawRect.subView(nComaPos1 + 1, nComaPos2 - nComaPos1));
        const auto nDraw3 = o3tl::toInt64(sDrawRect.subView(nComaPos2 + 1, nComaPos3 - nComaPos2));
        const auto nDraw4 = o3tl::toInt64(
            sDrawRect.subView(nComaPos3 + 1, sDrawRect.getLength() - nComaPos3 - 1));
        const auto aChildShape = SwRect(nDraw1, nDraw2, nDraw3, nDraw4);

        const auto nFlyLeft = getXPath(pXmlDoc, sFrameXPath[i], "left"_ostr).toInt64();
        const auto nFlyTop = getXPath(pXmlDoc, sFrameXPath[i], "top"_ostr).toInt64();
        const auto nFlyWidth = getXPath(pXmlDoc, sFrameXPath[i], "width"_ostr).toInt64();
        const auto nFlyHeight = getXPath(pXmlDoc, sFrameXPath[i], "height"_ostr).toInt64();

        const auto aFrame = SwRect(nFlyLeft, nFlyTop, nFlyWidth, nFlyHeight);

        CPPUNIT_ASSERT_MESSAGE("Textbox must be inside the shape!", aChildShape.Contains(aFrame));
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf116256)
{
    // Open bugdoc
    createSwDoc("tdf116256.docx");
    CPPUNIT_ASSERT(mxComponent);

    // Get the textbox
    uno::Reference<beans::XPropertySet> xTextBox(getShape(2), uno::UNO_QUERY_THROW);

    // Ensure that is a real textbox, and follows the text flow
    CPPUNIT_ASSERT(xTextBox->getPropertyValue("TextBox").get<bool>());
    CPPUNIT_ASSERT(xTextBox->getPropertyValue("IsFollowingTextFlow").get<bool>());

    // Parse the layout
    auto pLayout = parseLayoutDump();
    // Get the position of the shape
    const auto nTextBoxShapeLeft = getXPath(pLayout,
                                            "/root/page/body/txt/anchored/fly/tab/row[1]/cell/txt/"
                                            "anchored/SwAnchoredDrawObject/bounds"_ostr,
                                            "left"_ostr)
                                       .toInt64();
    const auto nTextBoxShapeTop = getXPath(pLayout,
                                           "/root/page/body/txt/anchored/fly/tab/row[1]/cell/txt/"
                                           "anchored/SwAnchoredDrawObject/bounds"_ostr,
                                           "top"_ostr)
                                      .toInt64();
    // Get the position of the textframe too.
    const auto nTextBoxFrameLeft
        = getXPath(
              pLayout,
              "/root/page/body/txt/anchored/fly/tab/row[1]/cell/txt/anchored/fly/infos/bounds"_ostr,
              "left"_ostr)
              .toInt64();
    const auto nTextBoxFrameTop
        = getXPath(
              pLayout,
              "/root/page/body/txt/anchored/fly/tab/row[1]/cell/txt/anchored/fly/infos/bounds"_ostr,
              "top"_ostr)
              .toInt64();

    // Without the fix in place these were less than they supposed to.
    CPPUNIT_ASSERT_GREATEREQUAL(nTextBoxShapeLeft, nTextBoxFrameLeft);
    CPPUNIT_ASSERT_GREATEREQUAL(nTextBoxShapeTop, nTextBoxFrameTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf138194)
{
    createSwDoc("xaxis-labelbreak.docx");
    SwDoc* pDoc = getSwDoc();
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
    assertXPath(pXmlDoc, "//textarray"_ostr, 8);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf146272)
{
    createSwDoc("tdf146272.odt");

    uno::Reference<beans::XPropertySet> xPicture(getShape(2), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xDrawing(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xDrawing->getPropertyValue("TextBoxContent"),
                                               uno::UNO_QUERY);

    CPPUNIT_ASSERT(xPicture);
    CPPUNIT_ASSERT(xDrawing);
    CPPUNIT_ASSERT(xFrame);

    const sal_uInt64 nPitureZorder = xPicture->getPropertyValue("ZOrder").get<sal_uInt64>();
    const sal_uInt64 nDrawingZorder = xDrawing->getPropertyValue("ZOrder").get<sal_uInt64>();
    const sal_uInt64 nFrameZorder = xFrame->getPropertyValue("ZOrder").get<sal_uInt64>();

    CPPUNIT_ASSERT_MESSAGE("Bad ZOrder!", nDrawingZorder < nFrameZorder);
    CPPUNIT_ASSERT_MESSAGE("Bad ZOrder!", nFrameZorder < nPitureZorder);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf138773)
{
    createSwDoc("tdf138773.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    const sal_Int32 nFirstLabelLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"2000-01\")])"_ostr).toInt32();

    // This failed, if the first X axis label broke to multiple lines.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nFirstLabelLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf124796)
{
    createSwDoc("tdf124796.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the minimum value of Y axis is not -10.
    assertXPathContent(pXmlDoc, "(//textarray)[5]/text"_ostr, "-10");

    // This failed, if the maximum value of Y axis is not 15.
    assertXPathContent(pXmlDoc, "(//textarray)[10]/text"_ostr, "15");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf72727)
{
    createSwDoc("tdf72727.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : Series1
    assertXPathContent(pXmlDoc, "(//textarray)[1]/text"_ostr, "1");
    assertXPathContent(pXmlDoc, "(//textarray)[2]/text"_ostr, "2");
    assertXPathContent(pXmlDoc, "(//textarray)[3]/text"_ostr, "3");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130969)
{
    createSwDoc("tdf130969.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the minimum value of Y axis is not 0.35781
    assertXPathContent(pXmlDoc, "(//textarray)[5]/text"_ostr, "0.35781");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf40260)
{
    createSwDoc("tdf40260.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: f(x) = 1.26510397865547E-06 x − 5.95245604996327E-12
    // - Actual  : f(x) = 0 x − 0
    assertXPathContent(pXmlDoc, "(//textarray)[19]/text"_ostr,
                       "f(x) = 1.26510397865547E-06 x " + OUStringChar(u'\x2212')
                           + " 5.95245604996327E-12");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf129054)
{
    createSwDoc("tdf129054.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Test the size of diameter of Pie chart.
    sal_Int32 nYTop
        = getXPath(
              pXmlDoc,
              "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[4]/polyline[1]/point[1]"_ostr,
              "y"_ostr)
              .toInt32();
    sal_Int32 nYBottom
        = getXPath(
              pXmlDoc,
              "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[4]/polyline[1]/point[31]"_ostr,
              "y"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4615, nYTop - nYBottom, 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf129173)
{
    createSwDoc("testAreaChartNumberFormat.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the first data label of area chart.
    assertXPathContent(pXmlDoc, "(//textarray)[22]/text"_ostr, "56");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134866)
{
    createSwDoc("tdf134866.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the data label of pie chart.
    assertXPathContent(pXmlDoc, "(//textarray)[2]/text"_ostr, "100%");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf137116)
{
    createSwDoc("tdf137116.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nX2
        = getXPath(pXmlDoc, "(//textarray)[2]"_ostr, "x"_ostr).toInt32(); // second data label
    sal_Int32 nX4
        = getXPath(pXmlDoc, "(//textarray)[4]"_ostr, "x"_ostr).toInt32(); // fourth data label
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1229
    // - Actual  : -225
    // - Delta   : 100
    // i.e. the second data label appeared inside the pie slice.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1117, nX2 - nX4, 100);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf137154)
{
    createSwDoc("tdf137154.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nX1
        = getXPath(pXmlDoc, "(//textarray)[1]"_ostr, "x"_ostr).toInt32(); // first data label
    sal_Int32 nX4
        = getXPath(pXmlDoc, "(//textarray)[4]"_ostr, "x"_ostr).toInt32(); // fourth data label
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 10865
    // - Actual  : 10252
    // - Delta   : 50
    // i.e. the first data label appeared too close to the pie.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nX4, nX1, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf138777)
{
    createSwDoc("outside_long_data_label.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    const sal_Int32 nFirstLabelLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"really\")])"_ostr).toInt32();

    // This failed, if the first data label didn't break to multiple lines.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(1), nFirstLabelLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130031)
{
    createSwDoc("tdf130031.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nY = getXPath(pXmlDoc, "(//textarray)[11]"_ostr, "y"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4653
    // - Actual  : 2182
    // - Delta   : 50
    // i.e. the data label appeared above the data point.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4653, nY, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130242)
{
    createSwDoc("tdf130242.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nY = getXPath(pXmlDoc, "(//textarray)[11]"_ostr, "y"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5758
    // - Actual  : 3352
    // - Delta   : 50
    // i.e. the data label appeared above the data point.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5758, nY, 50);

    nY = getXPath(pXmlDoc, "(//textarray)[13]"_ostr, "y"_ostr).toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2335
    // - Actual  : 2343
    // - Delta   : 50
    // i.e. the data label appeared above the data point.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2335, nY, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134121)
{
    createSwDoc("piechart_leaderline.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the custom leader line on pie chart.
    assertXPath(pXmlDoc, "//polyline"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf138018)
{
    createSwDoc("tdf138018.docx");
    SwDoc* pDoc = getSwDoc();
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
    assertXPath(pXmlDoc, "//polyline"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf130380)
{
    createSwDoc("tdf130380.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nY = getXPath(pXmlDoc,
                            "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[1]/polypolygon/"
                            "polygon/point[1]"_ostr,
                            "y"_ostr)
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
    createSwDoc("tdf129095.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // check the inner chart area (relative size) visibility with testing the X axis label
    assertXPathContent(pXmlDoc, "//textarray/text"_ostr, "Category 1");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf132956)
{
    createSwDoc("tdf132956.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // check the inner chart area (default size) visibility with testing the X axis label
    assertXPathContent(pXmlDoc, "//textarray/text"_ostr, "Category 1");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf137819)
{
    // Open the bugdoc and check if it went wrong
    createSwDoc("tdf137819.fodt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);

    // Change the anchor if the textbox to As_char
    uno::Reference<beans::XPropertySet> xShapePropSet(getShape(1), uno::UNO_QUERY);
    xShapePropSet->setPropertyValue(
        "AnchorType", uno::Any(text::TextContentAnchorType::TextContentAnchorType_AS_CHARACTER));

    // Make the layout xml dump after the change
    auto pXml = parseLayoutDump();
    auto sTextRightSidePosition
        = getXPath(pXml, "/root/page/body/txt[6]/anchored/fly/infos/bounds"_ostr, "right"_ostr);
    auto sShapeRightSidePosition = getXPath(
        pXml, "/root/page/body/txt[6]/anchored/SwAnchoredDrawObject/bounds"_ostr, "right"_ostr);
    // Before the textframe did not follow the shape, now it supposed to
    // so the right side of the shape must be greater than the right side of
    // textframe:
    CPPUNIT_ASSERT(sTextRightSidePosition.toInt32() < sShapeRightSidePosition.toInt32());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122014)
{
    createSwDoc("tdf122014.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the chart title is aligned to left.
    sal_Int32 nX1 = getXPath(pXmlDoc, "(//textarray)[13]"_ostr, "x"_ostr).toInt32();
    sal_Int32 nX2 = getXPath(pXmlDoc, "(//textarray)[14]"_ostr, "x"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(nX1 + 100, nX2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134659)
{
    createSwDoc("tdf134659.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the axis label is aligned to left.
    sal_Int32 nX1 = getXPath(pXmlDoc, "(//textarray)[1]"_ostr, "x"_ostr).toInt32();
    sal_Int32 nX2 = getXPath(pXmlDoc, "(//textarray)[2]"_ostr, "x"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(nX1 + 250, nX2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134235)
{
    createSwDoc("tdf134235.docx");
    SwDoc* pDoc = getSwDoc();
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
    assertXPath(pXmlDoc, "//textarray"_ostr, 14);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134676)
{
    createSwDoc("tdf134676.docx");
    SwDoc* pDoc = getSwDoc();
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
    assertXPath(pXmlDoc, "//textarray"_ostr, 14);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134146)
{
    createSwDoc("tdf134146.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    const sal_Int32 nTitleLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"Horizontal\")])"_ostr).toInt32();

    // Without the accompanying fix in place, this test would have failed.
    // i.e. the Y axis title didn't break to multiple lines.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(1), nTitleLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf136061)
{
    createSwDoc("tdf136061.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // This failed, if the custom text of data label is missing.
    assertXPathContent(pXmlDoc, "(//textarray)[16]/text"_ostr, "Customlabel");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf116925)
{
    createSwDoc("tdf116925.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(
        pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[3]/textarray/text"_ostr,
        "hello");
    // This failed, text color was #000000.
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[3]/textcolor[@color='#ffffff']"_ostr,
        1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf117028)
{
    createSwDoc("tdf117028.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // The only polypolygon in the rendering result was the white background we
    // want to avoid.
    assertXPath(pXmlDoc, "//polypolygon"_ostr, 0);

    // Make sure the text is still rendered.
    assertXPathContent(pXmlDoc, "//textarray/text"_ostr, "Hello");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf106390)
{
    createSwDoc("tdf106390.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nBottom = getXPath(pXmlDoc, "//sectrectclipregion"_ostr, "bottom"_ostr).toInt32();

    // No end point of line segments shall go below the bottom of the clipping area.
    const OString sXPath = "//polyline/point[@y>" + OString::number(nBottom) + "]";

    assertXPath(pXmlDoc, sXPath, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTableExtrusion1)
{
    createSwDoc("table-extrusion1.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nRight = getXPath(pXmlDoc, "//sectrectclipregion"_ostr, "right"_ostr).toInt32();
    sal_Int32 nLeft = static_cast<sal_Int32>(nRight * 0.95);

    // Expect table borders in right page margin.
    const OString sXPath = "//polyline/point[@x>" + OString::number(nLeft) + " and @x<"
                           + OString::number(nRight) + "]";

    assertXPath(pXmlDoc, sXPath, 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTableExtrusion2)
{
    createSwDoc("table-extrusion2.odt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = pDoc->GetDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // End point position of the outer table.
    sal_Int32 nX = getXPath(pXmlDoc, "(//polyline[1]/point)[2]"_ostr, "x"_ostr).toInt32();

    // Do not allow inner table extrude outer table.
    const OString sXPath = "//polyline/point[@x>" + OString::number(nX) + "]";

    assertXPath(pXmlDoc, sXPath, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf116848)
{
    createSwDoc("tdf116848.odt");
    SwDoc* pDoc = getSwDoc();
    // This resulted in a layout loop.
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf117245)
{
    createSwDoc("tdf117245.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin did not use a single line when there was
    // enough space for the text.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout"_ostr, 1);

    // This was 2, same problem elsewhere due to code duplication.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf118672)
{
    createSwDoc("tdf118672.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check if we have hyphenation support, otherwise can't test SwHyphPortion.
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale("en", "US", OUString())))
        return;

    // This ended as "fol*1 2 3 4 5 6 7 8 9", i.e. "10con-" was moved to the next line.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr,
                "He heard quiet steps behind him. That didn't bode well. Who could be fol*1 2 "
                "3 4 5 6 7 8 9 10con");
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[2]"_ostr,
                "portion"_ostr, "setetur");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf150200)
{
    createSwDoc("tdf150200.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // dash
    OUString sFirstLine
        = parseDump("/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"-(dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(93), sFirstLine.getLength());
    // en-dash
    sFirstLine = parseDump("/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"–(en-dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(88), sFirstLine.getLength());
    // em-dash
    sFirstLine = parseDump("/root/page/body/txt[3]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"—(em-dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(77), sFirstLine.getLength());
    // figure dash
    sFirstLine = parseDump("/root/page/body/txt[4]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"‒(figure dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(87), sFirstLine.getLength());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf150200_DOCX)
{
    createSwDoc("tdf150200.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // dash
    OUString sFirstLine
        = parseDump("/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"-(dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(93), sFirstLine.getLength());
    // en-dash
    sFirstLine = parseDump("/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"–(en-dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(88), sFirstLine.getLength());
    // em-dash
    sFirstLine = parseDump("/root/page/body/txt[3]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"—(em-dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(77), sFirstLine.getLength());
    // figure dash
    sFirstLine = parseDump("/root/page/body/txt[4]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"‒(figure dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(87), sFirstLine.getLength());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf150438)
{
    createSwDoc("tdf150438.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // left double quotation mark
    OUString sFirstLine
        = parseDump("/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"“Lorem ipsum"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(92), sFirstLine.getLength());
    // right double quotation mark
    sFirstLine = parseDump("/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"”Nunc viverra imperdiet enim."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(97), sFirstLine.getLength());
    // left single quotation mark
    sFirstLine = parseDump("/root/page/body/txt[3]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"‘Aenean nec lorem."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(85), sFirstLine.getLength());
    // right single quotation mark or apostrophe
    sFirstLine = parseDump("/root/page/body/txt[4]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"’Aenean nec lorem."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(85), sFirstLine.getLength());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf150438_DOCX)
{
    createSwDoc("tdf150438.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // left double quotation mark
    OUString sFirstLine
        = parseDump("/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"“Lorem ipsum"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(92), sFirstLine.getLength());
    // right double quotation mark
    sFirstLine = parseDump("/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"”Nunc viverra imperdiet enim."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(97), sFirstLine.getLength());
    // left single quotation mark
    sFirstLine = parseDump("/root/page/body/txt[3]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"‘Aenean nec lorem."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(85), sFirstLine.getLength());
    // right single quotation mark or apostrophe
    sFirstLine = parseDump("/root/page/body/txt[4]/SwParaPortion/SwLineLayout[1]/@portion"_ostr);
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"’Aenean nec lorem."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(85), sFirstLine.getLength());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf117923)
{
    createSwDoc("tdf117923.doc");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check that we actually test the line we need
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]"_ostr, "GHI GHI GHI GHI");
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
        "type"_ostr, "PortionType::Number");
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
        "expand"_ostr, "2.");
    // The numbering height was 960.
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
        "height"_ostr, "220");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf127606)
{
    createSwDoc("tdf117923.docx");
    // Ensure that all text portions are calculated before testing.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check that we actually test the line we need
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]"_ostr, "GHI GHI GHI GHI");
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
        "type"_ostr, "PortionType::Number");
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr,
        "expand"_ostr, "2.");
    // The numbering height was 960 in DOC format.
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
        "height"_ostr, "220");

    // tdf#127606: now it's possible to change formatting of numbering
    // increase font size (220 -> 260)
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Grow", {});
    pViewShell->Reformat();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont"_ostr,
        "height"_ostr, "260");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf127118)
{
    createSwDoc("tdf127118.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was Horizontal: merged cell split between pages didn't keep vertical writing direction
    assertXPath(pXmlDoc, "/root/page[2]/body/tab/row[1]/cell[1]/txt[1]"_ostr, "WritingMode"_ostr,
                "VertBTLR");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf141220)
{
    createSwDoc("tdf141220.docx");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "top"_ostr).toInt32();
    sal_Int32 nTextBoxTop
        = getXPath(pXmlDoc, "//anchored/fly/infos/bounds"_ostr, "top"_ostr).toInt32();
    // Make sure the textbox stays inside the shape.
    CPPUNIT_ASSERT_LESS(static_cast<sal_Int32>(15), nTextBoxTop - nShapeTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, TestTextBoxChangeViaUNO)
{
    createSwDoc("TextBoxFrame.odt");
    // this file has a shape and a frame inside. Try to set up
    // the frame for the shape as textbox. Before this was not
    // implemented. This will be necessary for proper WPG import.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("There must be a shape and a frame!", 2, getShapes());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("This must be a custom shape!",
                                 OUString("com.sun.star.drawing.CustomShape"),
                                 getShape(1)->getShapeType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This must be a frame shape!", OUString("FrameShape"),
                                 getShape(2)->getShapeType());

    CPPUNIT_ASSERT_MESSAGE("This is not supposed to be a textbox!",
                           !uno::Reference<beans::XPropertySet>(getShape(1), uno::UNO_QUERY_THROW)
                                ->getPropertyValue("TextBox")
                                .get<bool>());
    // Without the fix it will crash at this line:
    CPPUNIT_ASSERT_MESSAGE("This is not supposed to be a textbox!",
                           !uno::Reference<beans::XPropertySet>(getShape(1), uno::UNO_QUERY_THROW)
                                ->getPropertyValue("TextBoxContent")
                                .hasValue());

    // So now set the frame as textbox for the shape!
    uno::Reference<beans::XPropertySet>(getShape(1), uno::UNO_QUERY_THROW)
        ->setPropertyValue("TextBoxContent", uno::Any(uno::Reference<text::XTextFrame>(
                                                 getShape(2), uno::UNO_QUERY_THROW)));

    CPPUNIT_ASSERT_MESSAGE("This is supposed to be a textbox!",
                           uno::Reference<beans::XPropertySet>(getShape(1), uno::UNO_QUERY_THROW)
                               ->getPropertyValue("TextBox")
                               .get<bool>());

    CPPUNIT_ASSERT_MESSAGE("This is supposed to be a textbox!",
                           uno::Reference<beans::XPropertySet>(getShape(1), uno::UNO_QUERY_THROW)
                               ->getPropertyValue("TextBoxContent")
                               .hasValue());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf121509)
{
    createSwDoc("Tdf121509.odt");
    auto pDoc = getSwDoc();
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
    // FIXME: if we use 'reload' here, it fails with
    //  Assertion `!m_pFirst && !m_pLast && "There are still indices registered"' failed.
    save("Office Open XML Text");

    // The second part: check if the reloaded doc has flys inside a fly
    uno::Reference<lang::XComponent> xComponent
        = loadFromDesktop(maTempFile.GetURL(), "com.sun.star.text.TextDocument");
    uno::Reference<text::XTextDocument> xTextDoc(xComponent, uno::UNO_QUERY);
    auto pTextDoc = dynamic_cast<SwXTextDocument*>(xTextDoc.get());
    CPPUNIT_ASSERT(pTextDoc);
    auto pSecondDoc = pTextDoc->GetDocShell()->GetDoc();
    auto pSecondFormats = pSecondDoc->GetSpzFrameFormats();

    bool bFlyInFlyFound = false;
    for (auto secondformat : *pSecondFormats)
    {
        const SwNode* pNd = secondformat->GetAnchor().GetAnchorNode();
        if (pNd->FindFlyStartNode())
        {
            // So there is a fly inside another -> problem.
            bFlyInFlyFound = true;
            break;
        }
    }
    // Drop the tempfile
    maTempFile.CloseStream();

    // With the fix this cannot be true, if it is, that means Word unable to read the file..
    CPPUNIT_ASSERT_MESSAGE("Corrupt exported docx file!", !bFlyInFlyFound);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134685)
{
    createSwDoc("tdf134685.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nWidth
        = getXPath(pXmlDoc, "/root/page/body/tab/row[6]/cell[1]/infos/bounds"_ostr, "width"_ostr)
              .toInt32();
    // This was 2223: the content was only partially visible according to the lost cell width
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(14000), nWidth);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf109077)
{
    createSwDoc("tdf109077.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds"_ostr, "top"_ostr).toInt32();
    sal_Int32 nTextBoxTop
        = getXPath(pXmlDoc, "//anchored/fly/infos/bounds"_ostr, "top"_ostr).toInt32();
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

    createSwDoc("user-field-type-language.fodt");
    SwDoc* pDoc = getSwDoc();
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->UpdateFields();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was "123,456.00", via a buggy 1234.56 -> 1234,56 -> 123456 ->
    // 123,456.00 transform chain.
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt/SwParaPortion/SwLineLayout/SwFieldPortion[@type='PortionType::Field']"_ostr,
        "expand"_ostr, "1,234.56");

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
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt/SwParaPortion/SwLineLayout/SwFieldPortion[@type='PortionType::Field']"_ostr,
        "expand"_ostr, "1,234.56");
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
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt/SwParaPortion/SwLineLayout/SwFieldPortion[@type='PortionType::Field']"_ostr,
        "expand"_ostr, "1,234.56");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf124261)
{
#if !defined(_WIN32) && !defined(MACOSX)
    // Make sure that pressing a key in a btlr cell frame causes an immediate, correct repaint.
    createSwDoc("tdf124261.docx");
    SwDoc* pDoc = getSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf135991)
{
    createSwDoc("tdf135991.odt");
    auto pDump = parseLayoutDump();
    // There used to be negative values that made the column frames invisible.
    assertXPath(pDump, "//bounds[@top<0]"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf150642)
{
    createSwDoc("tdf150642.odt");
    auto pDump = parseLayoutDump();
    // There used to be negative values that made the cell frame invisible.
    assertXPath(pDump, "//bounds[@left<0]"_ostr, 0);
    assertXPath(pDump, "//bounds[@right<0]"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf152085)
{
    createSwDoc("tdf152085-section-tblr.odt");
    auto pDump = parseLayoutDump();
    sal_Int32 nSectionHeight
        = getXPath(pDump, "//section/infos/bounds"_ostr, "bottom"_ostr).toInt32(); // was 8391
    sal_Int32 nColumnHeight
        = getXPath(pDump, "(//column/infos/bounds)[2]"_ostr, "bottom"_ostr).toInt32(); // was 16216
    CPPUNIT_ASSERT_MESSAGE("The column in a TBRL page should be shorter than the section.",
                           nColumnHeight <= nSectionHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf152031)
{
    createSwDoc("tdf152031-stair.odt");

    // reproduce the bug by shrinking the table width.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Shrink table width from 5" to 4"
    sal_Int32 nWidth = getProperty<sal_Int32>(xTable, "Width") * 4 / 5;

    uno::Reference<beans::XPropertySet> xSet(xTable, uno::UNO_QUERY);
    xSet->setPropertyValue("Width", uno::Any(nWidth));

    auto pDump = parseLayoutDump();
    // There was a stair effect after change the table size.
    sal_Int32 nLeft_Row1 = getXPath(pDump, "(//row/infos/bounds)[1]"_ostr, "left"_ostr).toInt32();
    sal_Int32 nLeft_Row2 = getXPath(pDump, "(//row/infos/bounds)[2]"_ostr, "left"_ostr).toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("left values of SwRowFrames should be consistent.", nLeft_Row1,
                                 nLeft_Row2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf153136)
{
    createSwDoc("tdf153136.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    const sal_Int32 small = 300; // Small-height lines are around 276 twip
    const sal_Int32 large = 1000; // Large-height lines are 1104 twip or more

    // Page 1: standalone paragraphs

    // U+0009 CHARACTER TABULATION: height is ignored
    sal_Int32 height
        = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[1]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+0020 SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[2]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+00A0 NO-BREAK SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[3]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+1680 OGHAM SPACE MARK: height is considered; not tested, because Liberation Serif lacks it

    // U+2000 EN QUAD: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[4]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2001 EM QUAD: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[5]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2002 EN SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[6]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2003 EM SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[7]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2004 THREE-PER-EM SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[8]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2005 FOUR-PER-EM SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[9]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2006 SIX-PER-EM SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[10]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2007 FIGURE SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[11]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2008 PUNCTUATION SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[12]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2009 THIN SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[13]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+200A HAIR SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[14]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+202F NARROW NO-BREAK SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[15]"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+205F MEDIUM MATHEMATICAL SPACE: height is considered; not tested, because Liberation Serif lacks it
    // U+3000 IDEOGRAPHIC SPACE: height is ignored; not tested, because Liberation Serif lacks it

    // Page 2: table rows (no paragraph-level size DF)

    // U+0020 SPACE: height is ignored
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+00A0 NO-BREAK SPACE: height is considered (1104 or so)
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[2]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+1680 OGHAM SPACE MARK: height is considered; not tested, because Liberation Serif lacks it

    // U+2000 EN QUAD: height is considered
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[3]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2001 EM QUAD: height is considered
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[4]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2002 EN SPACE: height is ignored
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[5]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2003 EM SPACE: height is ignored
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[6]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2004 THREE-PER-EM SPACE: height is considered
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[7]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2005 FOUR-PER-EM SPACE: height is ignored
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[8]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2006 SIX-PER-EM SPACE: height is considered
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[9]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2007 FIGURE SPACE: height is considered
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[10]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2008 PUNCTUATION SPACE: height is considered
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[11]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2009 THIN SPACE: height is considered
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[12]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+200A HAIR SPACE: height is considered
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[13]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+202F NARROW NO-BREAK SPACE: height is considered
    height
        = getXPath(pXmlDoc, "(/root/page[2]//row)[14]/infos/bounds"_ostr, "height"_ostr).toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+205F MEDIUM MATHEMATICAL SPACE: height is considered; not tested, because Liberation Serif lacks it
    // U+3000 IDEOGRAPHIC SPACE: height is ignored; not tested, because Liberation Serif lacks it

    // TODO: page 3, with table having paragraphs with paragraph-level size DF;
    // all rows should have large height

    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[1]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[2]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[3]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[4]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[5]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[6]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[7]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[8]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[9]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[10]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[11]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[12]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[13]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[14]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
