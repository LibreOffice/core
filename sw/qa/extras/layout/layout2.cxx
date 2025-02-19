/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>

#include <comphelper/scopeguard.hxx>
#include <comphelper/sequence.hxx>
#include <editeng/unolingu.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/string_view.hxx>

#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <bodyfrm.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <fmtanchr.hxx>

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter2 : public SwModelTestBase
{
public:
    SwLayoutWriter2()
        : SwModelTestBase(u"/sw/qa/extras/layout/data/"_ustr)
    {
    }

protected:
    void CheckRedlineCharAttributesHidden();
};

// this mainly tests that splitting portions across redlines in SwAttrIter works
void SwLayoutWriter2::CheckRedlineCharAttributesHidden()
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/merged", "paraPropsNodeIndex", u"9");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/merged", "paraPropsNodeIndex", u"10");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/merged", "paraPropsNodeIndex", u"11");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/merged", "paraPropsNodeIndex", u"12");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/merged", "paraPropsNodeIndex", u"13");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/merged", "paraPropsNodeIndex", u"14");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/merged", "paraPropsNodeIndex", u"15");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/merged", "paraPropsNodeIndex", u"16");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/merged", "paraPropsNodeIndex", u"17");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"foobaz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/merged", "paraPropsNodeIndex", u"18");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"ob");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/merged", "paraPropsNodeIndex", u"19");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "type", u"PortionType::Para");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwParaPortion[1]",
                "portion", u"foobaz");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf148897)
{
    createSwDoc("tdf148897.odt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly", 1);
    assertXPath(pXmlDoc, "/root/page[2]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[3]/anchored/fly", 1);
    assertXPath(pXmlDoc, "/root/page[3]/sorted_objs/fly", 0);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt/anchored/fly", 0);
    assertXPath(pXmlDoc, "/root/page[4]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "/root/page[4]/body/txt[1]/anchored/fly", 1);
    // fly portion exists, no overlapping text
    assertXPath(pXmlDoc, "/root/page[4]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwFixPortion",
                "height", u"5797");
    assertXPath(pXmlDoc, "/root/page[5]/sorted_objs/fly", 0);
    assertXPath(pXmlDoc, "/root/page", 5);

    auto xModel = mxComponent.queryThrow<frame::XModel>();
    uno::Reference<drawing::XShape> xShape(getShapeByName(u"Image3"));
    uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(), uno::UNO_QUERY);
    xCtrl->select(uno::Any(xShape));

    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/sorted_objs/fly", 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly", 0);
    assertXPath(pXmlDoc, "/root/page[2]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly", 1);
    assertXPath(pXmlDoc, "/root/page[3]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt[2]/anchored/fly", 1);
    // fly portion exists, no overlapping text
    assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwFixPortion",
                "height", u"5797");
    assertXPath(pXmlDoc, "/root/page[4]/sorted_objs/fly", 0);
    assertXPath(pXmlDoc, "/root/page[4]/body/txt/anchored/fly", 0);
    assertXPath(pXmlDoc, "/root/page", 4);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/anchored/fly", 1);
    assertXPath(pXmlDoc, "/root/page[2]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[3]/anchored/fly", 1);
    assertXPath(pXmlDoc, "/root/page[3]/sorted_objs/fly", 0);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt/anchored/fly", 0);
    assertXPath(pXmlDoc, "/root/page[4]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "/root/page[4]/body/txt[1]/anchored/fly", 1);
    // fly portion exists, no overlapping text
    assertXPath(pXmlDoc, "/root/page[4]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwFixPortion",
                "height", u"5797");
    assertXPath(pXmlDoc, "/root/page[5]/sorted_objs/fly", 0);
    assertXPath(pXmlDoc, "/root/page", 5);

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/sorted_objs/fly", 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/fly", 0);
    assertXPath(pXmlDoc, "/root/page[2]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/anchored/fly", 1);
    assertXPath(pXmlDoc, "/root/page[3]/sorted_objs/fly", 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/txt[2]/anchored/fly", 1);
    // fly portion exists, no overlapping text
    assertXPath(pXmlDoc, "/root/page[3]/body/txt[1]/SwParaPortion/SwLineLayout[1]/SwFixPortion",
                "height", u"5797");
    assertXPath(pXmlDoc, "/root/page[4]/sorted_objs/fly", 0);
    assertXPath(pXmlDoc, "/root/page[4]/body/txt/anchored/fly", 0);
    assertXPath(pXmlDoc, "/root/page", 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineCharAttributes)
{
    createSwDoc("redline_charatr.fodt");
    SwDoc* pDoc(getSwDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // verify after load
    CheckRedlineCharAttributesHidden();

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    // why is this needed explicitly?
    calcLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // show: nothing is merged
    assertXPath(pXmlDoc, "//merged", 0);
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[4]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[7]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"ba");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"r");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[4]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwLinePortion[4]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[9]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"baz");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"fo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"o");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"bar");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[4]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[4]",
                "portion", u"b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[5]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwLinePortion[5]",
                "portion", u"az");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[1]",
                "portion", u"foo");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[2]",
                "portion", u"b");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[3]",
                "portion", u"a");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[4]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[4]",
                "portion", u"r");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[5]",
                "type", u"PortionType::Text");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[11]/SwParaPortion/SwLineLayout/SwLinePortion[5]",
                "portion", u"baz");

    // verify after hide
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());
    // why is this needed explicitly?
    calcLayout();
    CheckRedlineCharAttributesHidden();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineShowHideFootnotePagination)
{
    createSwDoc("redline_footnote_pagination.fodt");
    SwDoc* pDoc(getSwDoc());
    SwRootFrame* pLayout(pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // check footnotes
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn", 6);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn", 3);
    // check that first page ends with the y line and second page starts with z
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[last()]/SwParaPortion/SwLineLayout[last()]",
                "portion",
                u"yyyyyyyyy yyy yyyyyyyyyyyyyyyy yyyyyyy yyy yyyyy yyyyyyyyy yyy yyyyyyyyy ");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"zzz. zzz zzzz zzzz* zzz zzz zzzzzzz zzz zzzz zzzzzzzzzzzzzz zzzzzzzzzzzz ");

    // hide redlines - all still visible footnotes move to page 1
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});

    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn", 2);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn", 0);

    // show again - should now get the same result as on loading
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});

    pXmlDoc = parseLayoutDump();

    // check footnotes
    assertXPath(pXmlDoc, "/root/page[1]/ftncont/ftn", 6);
    assertXPath(pXmlDoc, "/root/page[2]/ftncont/ftn", 3);
    // check that first page ends with the y line and second page starts with z
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[last()]/SwParaPortion/SwLineLayout[last()]",
                "portion",
                u"yyyyyyyyy yyy yyyyyyyyyyyyyyyy yyyyyyy yyy yyyyy yyyyyyyyy yyy yyyyyyyyy ");
    assertXPath(pXmlDoc, "/root/page[2]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"zzz. zzz zzzz zzzz* zzz zzz zzzzzzz zzz zzzz zzzzzzzzzzzzzz zzzzzzzzzzzz ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testtdf138951)
{
    // Open the bugdoc
    createSwDoc("tdf138951.odt");

    // Get the only shape
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape);

    // Gather its formats: the shape and textbox
    const SwFrameFormat* pTxFrm = SwTextBoxHelper::getOtherTextBoxFormat(xShape);
    CPPUNIT_ASSERT(pTxFrm);
    const SwFrameFormat* pShFrm = SwTextBoxHelper::getOtherTextBoxFormat(pTxFrm, RES_FLYFRMFMT);
    CPPUNIT_ASSERT(pShFrm);

    calcLayout();

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
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[2]", "colors",
                u"#Bookmark1 Bookmark Start");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[4]", "colors",
                u"#Bookmark2 Bookmark Start");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[6]", "colors",
                u"#Bookmark2 Bookmark End#Bookmark1 Bookmark End");
    // full text, if bookmarks are visible
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[1]", "portion",
                u"Lorem ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[3]", "portion",
                u"ipsum dolor et ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[5]", "portion",
                u"ames");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout/child::*[7]", "portion",
                u".");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf150790)
{
    createSwDoc("tdf150790.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // point bookmark is shown as I-beam (only its text dump is |, as before on the screen)
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/SwBookmarkPortion",
                "colors", u"#Bookmark 1 Bookmark");
    // single start bookmark
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwBookmarkPortion[1]",
                "colors", u"#Bookmark 2 Bookmark Start");
    // single end bookmark
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwBookmarkPortion[3]",
                "colors", u"#Bookmark 3 Bookmark End");
    // This was |, as before the point bookmark (neighboring end and start bookmarks)
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwBookmarkPortion[2]",
                "colors", u"#Bookmark 2 Bookmark End#Bookmark 3 Bookmark Start");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf129357)
{
    createSwDoc("tdf129357.fodt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // visible soft hyphen
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/push/push/textarray[2]/text", u"-");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineNumberInNumbering)
{
    createSwDoc("tdf42748.fodt");
    SwDocShell* pShell = getSwDocShell();

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
        "/metafile/push/push/push/textcolor[not(@color='#000000') and not(@color='#008000')]", 5);

    // tdf#145068 numbering shows changes in the associated list item, not the next one
    // This was 1 (black numbering of the first list item previously)
    assertXPath(pXmlDoc, "/metafile/push/push/push/font[4][@color='#000000']", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineNumbering)
{
    createSwDoc("tdf115523.fodt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Show the correct and the original line numbering instead of counting
    // the deleted list items in Show Changes mode, as part of the list
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text", u"1.");
    // This was "2." (deleted text node, now its text content is part of the first list item)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text", u"[2.] ");
    // This was "3." (now it's the second list item)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[5]/text", u"2.[3.] ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineNumbering2)
{
    createSwDoc("tdf115524.fodt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Show the correct and the original line numbering in Show Changes mode
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text", u"1.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text", u"2.");
    // FIXME: show as 3.[2.]
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[5]/text", u"3.");
    // This was "4." (not shown the original number)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[7]/text", u"4.[3.] ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf149710_RedlineNumberingEditing)
{
    createSwDoc("tdf149710.fodt");

    // Show Changes
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    // delete the paragraph mark of the first list item with change tracking
    dispatchCommand(mxComponent, u".uno:GoToEndOfLine"_ustr, {});
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Show the correct and the original line numbering instead of counting
    // the deleted list items in Show Changes mode, as part of the list
    // This was "1."
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text", u"[1.] ");
    // This was "2." (deleted text node, now its text content is part of the first list item)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text", u"1.[2.] ");
    // This was "3." (now it's the second list item)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[5]/text", u"2.[3.] ");

    // remove the tracked deletion, and check the layout again
    pWrtShell->Undo();
    xMetaFile = pShell->GetPreviewMetaFile();
    pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text", u"1.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text", u"2.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[5]/text", u"3.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf149709_RedlineNumberingLevel)
{
    createSwDoc("tdf149709.fodt");

    // Show Changes
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    // insert a new list item at start of the second list item "a)"
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    pWrtShell->Down(false, 1);
    pWrtShell->SplitNode(false);

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Show the correct and the original line numbering instead of counting
    // the deleted list items in Show Changes mode, as part of the list
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text", u"1.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text", u"a)");
    // This was "b)[2.]"
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[4]/text", u"b)[a)] ");
    // This was "c)[3.]"
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[6]/text", u"c)[b)] ");
    // This was "4.[2.]" (after disabling Show Changes, and enabling again)
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[8]/text", u"2.");

    // remove the tracked deletion, and check the layout again
    pWrtShell->Undo();
    xMetaFile = pShell->GetPreviewMetaFile();
    pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text", u"1.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text", u"a)");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[5]/text", u"b)");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[7]/text", u"2.");

    // check Redo
    pWrtShell->Redo();
    xMetaFile = pShell->GetPreviewMetaFile();
    pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[1]/text", u"1.");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[3]/text", u"a)");
    // TODO: show as b)[a)]
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[4]/text", u"b)");
    // FIXME: This must be "c)[b]"
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[6]/text", u"c)[a)] ");
    assertXPathContent(pXmlDoc, "/metafile/push/push/push/textarray[8]/text", u"2.");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf149711_importDOCXMoveToParagraphMark)
{
    createSwDoc("tdf149711.docx");
    SwDoc* pDoc = getSwDoc();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 6);

    // reject tracked insertion (moveTo)
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->RejectRedline(1);

    pXmlDoc = parseLayoutDump();
    // This was 6 (not tracked paragraph mark of the moveTo list item)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf152872)
{
    createSwDoc("hidden-para-separator.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 2);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", "portion", u"C DE");
    // 5 is empty and hidden
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds", "height", u"0");

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetShowHiddenChar(true);
    aViewOptions.SetViewMetaChars(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 5);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", "portion", u"C ");
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout", "portion", u"D");
    // 3 is an empty paragraph with RES_CHRATR_HIDDEN
    assertXPath(pXmlDoc, "/root/page/body/txt[3]/infos/bounds", "height", u"398");
    assertXPath(pXmlDoc, "/root/page/body/txt[4]/SwParaPortion/SwLineLayout", "portion", u"E");
    // 5 is an empty paragraph with RES_CHRATR_HIDDEN
    assertXPath(pXmlDoc, "/root/page/body/txt[5]/infos/bounds", "height", u"398");

    aViewOptions.SetViewMetaChars(false);
    pWrtShell->ApplyViewOptions(aViewOptions);

    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 2);
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", "portion", u"C DE");
    // 5 is empty and hidden
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds", "height", u"0");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testHiddenParaProps)
{
    createSwDoc("merge_hidden_redline.docx");

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwViewOption aViewOptions(*pWrtShell->GetViewOptions());
    aViewOptions.SetShowHiddenChar(true);
    aViewOptions.SetViewMetaChars(true);
    pWrtShell->ApplyViewOptions(aViewOptions);

    // note: do not use layout dump here, because it doesn't work:
    // SwTextFrame::Format doesn't actually create the SwMarginPortion for
    // non-left-aligned frames; instead, it sets SetFormatAdj() flag and later
    // *SwTextPainter* checks via GetAdjusted() if the flag is set and calls
    // CalcAdjLine() which inserts the SwMarginPortion.

    SwRootFrame* pRoot = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pRoot->GetLower()->IsPageFrame());
    SwPageFrame* pPage = static_cast<SwPageFrame*>(pRoot->GetLower());
    CPPUNIT_ASSERT(pPage->GetLower()->IsBodyFrame());
    SwBodyFrame* pBody = static_cast<SwBodyFrame*>(pPage->GetLower());
    CPPUNIT_ASSERT(pBody->GetLower()->IsTextFrame());
    SwTextFrame* pTextFrame = dynamic_cast<SwTextFrame*>(pBody->GetLower());

    CPPUNIT_ASSERT_EQUAL(u"1 hidden, delete-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    // TODO: redlines don't merge like in Word yet
    CPPUNIT_ASSERT_EQUAL(u"Abcdef"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"ghi"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"2 visible, delete-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abcghi"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"3 delete-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"ghi"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"4 delete-merge, delete-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"5 visible, hidden-merge, visible"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Center,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Left,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"6 hidden-merge, visible"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Center,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Left,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"7 visible, hidden-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abcdef"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Center,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"ghi"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Left,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"8 visible, delete-merge, visible, hidden-merge, visible"_ustr,
                         pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Right,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Center,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"ghi"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Left,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"9 hidden-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Center,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Left,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"10 visible, hidden-merge, visible, delete-merge, visible"_ustr,
                         pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Right,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Center,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"ghi"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Left,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());

    aViewOptions.SetShowHiddenChar(false);
    pWrtShell->ApplyViewOptions(aViewOptions);

    // the problem was that the wrong SwTextNode was used for properties
    pTextFrame = dynamic_cast<SwTextFrame*>(pBody->GetLower());
    CPPUNIT_ASSERT_EQUAL(u"1 hidden, delete-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    // TODO: redlines don't merge like in Word yet
    CPPUNIT_ASSERT_EQUAL(u"Abcdef"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"ghi"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"2 visible, delete-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abcghi"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"3 delete-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"ghi"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"4 delete-merge, delete-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"def"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"5 visible, hidden-merge, visible"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abcdef"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Center,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"6 hidden-merge, visible"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abcdef"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Left,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"7 visible, hidden-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abcdefghi"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Center,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"8 visible, delete-merge, visible, hidden-merge, visible"_ustr,
                         pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abc"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Right,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"defghi"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Center,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"9 hidden-merge"_ustr, pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abcdef"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Left,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"10 visible, hidden-merge, visible, delete-merge, visible"_ustr,
                         pTextFrame->GetText());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"abcdef"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Right,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
    pTextFrame = dynamic_cast<SwTextFrame*>(pTextFrame->GetNext());
    CPPUNIT_ASSERT_EQUAL(u"ghi"_ustr, pTextFrame->GetText());
    CPPUNIT_ASSERT_EQUAL(
        SvxAdjust::Left,
        pTextFrame->GetTextNodeForParaProps()->GetSwAttrSet().Get(RES_PARATR_ADJUST).GetAdjust());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf151954)
{
    createSwDoc("tdf151954.docx");
    SwDoc* pDoc = getSwDoc();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 2);

    // accept tracked insertion (moveTo)
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(3), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    pXmlDoc = parseLayoutDump();
    // This was 1 (moveFrom was extended to the paragraph mark)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf152952)
{
    createSwDoc("Hyphenated-link.rtf");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // URL should not be hyphenated
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u" NNNNNNNNNN NNNNNNNNNNNNNNN ");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]", "portion",
                u"https://example.com/xxxxxxx/testtesttesttest/hyphenate/testtesttest ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf152952_compat)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    createSwDoc("Hyphenated-link.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // URL hyphenated for backward compatibility
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u" NNNNNNNNNN NNNNNNNNNNNNNNN https://example.com/xxxxxxx/testtesttesttest/hyphen");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]", "portion",
                u"ate/testtesttest ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf158885_compound_remain)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"hu"_ustr, u"HU"_ustr, OUString())))
        return;

    uno::Reference<linguistic2::XSpellChecker1> xSpell = LinguMgr::GetSpellChecker();
    LanguageType eLang
        = LanguageTag::convertToLanguageType(lang::Locale(u"hu"_ustr, u"HU"_ustr, OUString()));
    if (!xSpell.is() || !xSpell->hasLanguage(static_cast<sal_uInt16>(eLang)))
        return;

    createSwDoc("tdf158885_compound-remain.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // hyphenate compound word with 3- or more character distance from the stem boundary
    // This was "emberel=lenes" (now "ember=ellenes", i.e. hyphenating at the stem boundary)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"emberellenes emberellenes emberellenes emberellenes emberellenes ember");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]", "portion",
                u"ellenes emberellenes emberellenes emberellenes emberellenes emberellenes ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf158885_not_compound_remain)
{
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"hu"_ustr, u"HU"_ustr, OUString())))
        return;

    uno::Reference<linguistic2::XSpellChecker1> xSpell = LinguMgr::GetSpellChecker();
    LanguageType eLang
        = LanguageTag::convertToLanguageType(lang::Locale(u"hu"_ustr, u"HU"_ustr, OUString()));
    if (!xSpell.is() || !xSpell->hasLanguage(static_cast<sal_uInt16>(eLang)))
        return;

    createSwDoc("tdf158885_not_compound-remain.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // hyphenate compound word with 2-character distance from the stem boundary,
    // resulting less readable hyphenation "emberel=lenes" ("emberel" and "lenes" have
    // different meanings, than the original word parts)
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"emberellenes emberellenes emberellenes emberellenes emberellenes emberel");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout[2]", "portion",
                u"lenes emberellenes emberellenes emberellenes emberellenes emberellenes ");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineNumberInFootnote)
{
    createSwDoc("tdf85610.fodt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // changed color of numbers of footnote 1 (deleted footnote) and footnote 2 (inserted footnote)
    // decreased the black <font> elements by 2:
    // This was 7
    assertXPath(pXmlDoc, "/metafile/push/push/push/font[@color='#000000']", 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineMoving)
{
    createSwDoc("tdf42748.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = getSwDocShell();

    // create a 3-element list without change tracking
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    pEditShell->RejectRedline(0);
    pEditShell->AcceptRedline(0);

    // move down first list item with track changes
    dispatchCommand(mxComponent, u".uno:GoToStartOfDoc"_ustr, {});
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    dispatchCommand(mxComponent, u".uno:MoveDown"_ustr, {});

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // text and numbering colors show moving of the list item
    // tdf#157663: the moved text item "It" is detected as text moving again!
    assertXPath(pXmlDoc, "/metafile/push/push/push/textcolor[@color='#008000']", 5);
    assertXPath(pXmlDoc, "/metafile/push/push/push/font[@color='#008000']", 11);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testRedlineMoving2)
{
    createSwDoc("tdf42748.fodt");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = getSwDocShell();

    // create a 3-element list without change tracking
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    pEditShell->RejectRedline(0);
    pEditShell->AcceptRedline(0);

    // extend the first item to "An ItemIt", because detection of move needs
    // at least 6 characters with an inner space after stripping white spaces
    // of the redline
    dispatchCommand(mxComponent, u".uno:GoToStartOfDoc"_ustr, {});
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"An Item"_ustr);

    // move down first list item with track changes
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    dispatchCommand(mxComponent, u".uno:MoveDown"_ustr, {});

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
    createSwDoc("tdf104797.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pShell = getSwDocShell();

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
    assertXPath(pXmlDoc, "/metafile/push/push/push/textcolor[@color='#008000']", 6);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTableCellInvalidate)
{
    // note: must set Hidden property, so that SfxFrameViewWindow_Impl::Resize()
    // does *not* forward initial VCL Window Resize and thereby triggers a
    // layout which does not happen on soffice --convert-to pdf.
    std::vector<beans::PropertyValue> aFilterOptions = {
        { beans::PropertyValue(u"Hidden"_ustr, -1, uno::Any(true),
                               beans::PropertyState_DIRECT_VALUE) },
    };

    // inline the loading because currently properties can't be passed...
    OUString const url(createFileURL(u"table_cell_overlap.fodt"));
    loadWithParams(url, comphelper::containerToSequence(aFilterOptions));
    save(u"writer_pdf_Export"_ustr);

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // somehow these 2 rows overlapped in the PDF unless CalcLayout() runs
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds", "top", u"6969");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/infos/bounds", "height", u"231");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/infos/bounds", "top",
                u"6969");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/infos/bounds", "height",
                u"231");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/txt[1]/infos/bounds",
                "top", u"6969");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[1]/cell[1]/txt[1]/infos/bounds",
                "height", u"231");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds", "top", u"7200");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/infos/bounds", "height", u"231");
    // this was 6969, causing the overlap
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/infos/bounds", "top",
                u"7200");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/infos/bounds", "height",
                u"231");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/txt[1]/infos/bounds",
                "top", u"7200");
    assertXPath(pXmlDoc, "/root/page[1]/anchored/fly/tab[1]/row[2]/cell[1]/txt[1]/infos/bounds",
                "height", u"231");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf145719)
{
    createSwDoc("tdf145719.odt");
    SwDocShell* pShell = getSwDocShell();

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
    createSwDoc("changed_table_rows.fodt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was 0 (other color, not COL_AUTHOR_TABLE_DEL, color of the tracked row deletion)
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#fce6f4']", 1);
    // This was 0 (other color, not COL_AUTHOR_TABLE_INS, color of the tracked row insertion)
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#e1f2fa']", 1);
    // This was 3 (color of the cells of the last column, 2 of them disabled by change tracking )
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#3faf46']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf155187_TableInTextChange)
{
    createSwDoc("table_in_text_change.fodt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was 0 (other color, not COL_AUTHOR_TABLE_DEL, color of the tracked row deletion)
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#fce6f4']", 2);
    // This was 0 (other color, not COL_AUTHOR_TABLE_INS, color of the tracked row insertion)
    assertXPath(pXmlDoc, "/metafile/push/push/push/push/push/fillcolor[@color='#e1f2fa']", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf145225_RedlineMovingWithBadInsertion)
{
    createSwDoc("tdf42748.fodt");
    SwDoc* pDoc = getSwDoc();

    // create a 3-element list without change tracking
    // (because the fixed problem depends on the own changes)
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    pEditShell->RejectRedline(0);
    pEditShell->AcceptRedline(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    // Show Changes
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());

    // insert a tracked paragraph break in middle of the second list item, i.e. split it
    dispatchCommand(mxComponent, u".uno:GoToStartOfDoc"_ustr, {});
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    // positionate the cursor in the middle of the second list item
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->SplitNode(false);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());

    // move up the last list item over the paragraph split
    dispatchCommand(mxComponent, u".uno:GoToEndOfDoc"_ustr, {});
    dispatchCommand(mxComponent, u".uno:MoveUp"_ustr, {});
    dispatchCommand(mxComponent, u".uno:MoveUp"_ustr, {});
    // This was 2 (the tracked paragraph break joined with the moved list item,
    // setting the not changed text of the second list item to tracked insertion)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(3), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf146964_ReappearingMovedTextInHideChangesMode)
{
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

    // enable Record Changes
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // set Hide Changes mode
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(pLayout->IsHideRedlines());

    // delete and paste the deleted word again during Track Changes
    dispatchCommand(mxComponent, u".uno:WordRightSel"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // This was "Lorem Lorem ipsum" (reappearing deletion in Hide Changes mode)
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"Lorem ipsum");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125300)
{
    createSwDoc("tdf125300.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;

    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Keep line spacing before bottom cell border (it was 1892)
    sal_Int32 y1
        = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[7]/polyline/point[1]", "y")
              .toInt32();
    sal_Int32 y2
        = getXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[7]/polyline/point[2]", "y")
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2092, y1, 7);
    CPPUNIT_ASSERT_EQUAL(y1, y2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf116830)
{
    createSwDoc("tdf116830.odt");
    SwDocShell* pShell = getSwDocShell();

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
    createSwDoc("tdf114163.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "(//textarray)[12]/text", u"Data3");
    // This failed, if the legend first label is not "Data3". The legend position is right.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf131707)
{
    createSwDoc("tdf131707_flyWrap.doc");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//body/tab/row[3]/cell[2]/txt/infos/bounds", "top", u"2185");
    // the image should be inside of the cell boundary - so the same top or higher
    assertXPath(pXmlDoc, "//body/tab/row[3]/cell[2]/txt/anchored/fly/infos/bounds", "top", u"2185");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122225)
{
    createSwDoc("tdf122225.docx");
    SwDocShell* pShell = getSwDocShell();

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
    createSwDoc("tdf125335.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "(//textarray)[12]/text", u"Data3");
    // This failed, if the legend first label is not "Data3". The legend position is bottom.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf134247)
{
    createSwDoc("legend-itemorder-min.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "(//textarray)[14]/text", u"1. adatsor");
    // This failed, if the legend first label is not "1. adatsor".
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf75659)
{
    createSwDoc("tdf75659.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "(//textarray)[17]/text", u"Series1");
    assertXPathContent(pXmlDoc, "(//textarray)[18]/text", u"Series2");
    assertXPathContent(pXmlDoc, "(//textarray)[19]/text", u"Series3");
    // These failed, if the legend names are empty strings.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf136816)
{
    createSwDoc("tdf136816.odt");
    SwDocShell* pShell = getSwDocShell();

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
    createSwDoc("long_legendentry.docx");
    SwDocShell* pShell = getSwDocShell();

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
    createSwDoc("tdf138465min.docx");

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
    createSwDoc("tdf123268.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 53
    // - Actual  : 0
    // i.e. the chart lost.
    assertXPath(pXmlDoc, "/metafile/push[1]/push[1]/push[1]/push[3]/push[1]/push[1]/push[1]/push",
                53);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf133005)
{
    createSwDoc("tdf133005.odt");
    SwDocShell* pShell = getSwDocShell();

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
                                  "push[1]/push[41]/polypolygon/polygon/point[1]",
                                  "x")
                             .toInt32();

    // This failed, if the value axis doesn't appear inside category.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nXChartWall, nXColumn, 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf115630)
{
    createSwDoc("tdf115630.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Test wide of inner chart area.
    sal_Int32 nXRight
        = getXPath(pXmlDoc,
                   "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[3]/polyline[1]/point[1]",
                   "x")
              .toInt32();
    sal_Int32 nXLeft
        = getXPath(pXmlDoc,
                   "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[3]/polyline[1]/point[2]",
                   "x")
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2895, nXRight - nXLeft, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf108021)
{
    createSwDoc("tdf108021.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//textarray[@length='22']", 8);
    // This failed, if the textarray length of the first axis label not 22.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf125334)
{
    createSwDoc("tdf125334.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    int nCount = countXPathNodes(pXmlDoc, "//textarray[@length='17']");
    CPPUNIT_ASSERT_EQUAL(4, nCount);
    // This failed, if the textarray length of the category axis label not 17.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf122800)
{
    createSwDoc("tdf122800.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "//textarray[@length='22']", 9);
    // This failed, if the textarray length of the first axis label not 22.
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTruncatedAxisLabel)
{
    createSwDoc("testTruncatedAxisLabel.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // test the X axis label visibility
    assertXPathContent(pXmlDoc, "(//textarray)[1]/text", u"Long axis label truncated 1");

    // test the Y axis label visibility
    assertXPathContent(pXmlDoc, "(//textarray)[3]/text", u"-5.00");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf128996)
{
    createSwDoc("tdf128996.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc, "(//textarray)[1]/text", u"A very long category name 1");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf126244)
{
    createSwDoc("tdf126244.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the first level of vertical category axis labels orientation. The first level orientation should be horizontal.
    assertXPath(pXmlDoc, "(//font)[1]", "orientation", u"0");
    // Test the second level of vertical category axis labels orientation. The second level orientation should be vertical.
    sal_Int32 nRotation = getXPath(pXmlDoc, "(//font)[5]", "orientation").toInt32();
    CPPUNIT_ASSERT(nRotation >= 899);
    CPPUNIT_ASSERT(nRotation <= 900);
    // Test the third level of vertical category axis labels orientation. The third level orientation should be vertical.
    nRotation = getXPath(pXmlDoc, "(//font)[7]", "orientation").toInt32();
    CPPUNIT_ASSERT(nRotation >= 899);
    CPPUNIT_ASSERT(nRotation <= 900);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf127304)
{
    createSwDoc("tdf127304.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the first level of horizontal category axis labels orientation. The first level orientation should be vertical.
    sal_Int32 nRotation = getXPath(pXmlDoc, "(//font)[1]", "orientation").toInt32();
    CPPUNIT_ASSERT(nRotation >= 899);
    CPPUNIT_ASSERT(nRotation <= 900);
    // Test the second level of horizontal category axis labels orientation. The second level orientation should be horizontal.
    assertXPath(pXmlDoc, "(//font)[5]", "orientation", u"0");
    // Test the third level of horizontal category axis labels orientation. The third level orientation should be horizontal.
    assertXPath(pXmlDoc, "(//font)[7]", "orientation", u"0");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testHorizontal_multilevel)
{
    createSwDoc("horizontal_multilevel.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Test the Y position of horizontal category axis label.
    sal_Int32 nYposition = getXPath(pXmlDoc, "(//textarray)[7]", "y").toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11248, nYposition, 20);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf69648)
{
    createSwDoc("tdf69648.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    const char* sShapeXPath[2] = {
        "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObjGroup/SdrObjList/SdrObject[1]",
        "/root/page/body/txt/anchored/SwAnchoredDrawObject/SdrObjGroup/SdrObjList/SdrObject[2]"
    };
    const char* sFrameXPath[2] = { "/root/page/body/txt/anchored/fly[1]/infos/bounds",
                                   "/root/page/body/txt/anchored/fly[2]/infos/bounds" };

    for (int i = 0; i < 2; ++i)
    {
        const auto sDrawRect = getXPath(pXmlDoc, sShapeXPath[i], "aOutRect");
        const auto nComaPos1 = sDrawRect.indexOf(',', 0);
        const auto nComaPos2 = sDrawRect.indexOf(',', nComaPos1 + 1);
        const auto nComaPos3 = sDrawRect.indexOf(',', nComaPos2 + 1);

        const auto nDraw1 = o3tl::toInt64(sDrawRect.subView(0, nComaPos1));
        const auto nDraw2 = o3tl::toInt64(sDrawRect.subView(nComaPos1 + 1, nComaPos2 - nComaPos1));
        const auto nDraw3 = o3tl::toInt64(sDrawRect.subView(nComaPos2 + 1, nComaPos3 - nComaPos2));
        const auto nDraw4 = o3tl::toInt64(
            sDrawRect.subView(nComaPos3 + 1, sDrawRect.getLength() - nComaPos3 - 1));
        const auto aChildShape = SwRect(nDraw1, nDraw2, nDraw3, nDraw4);

        const auto nFlyLeft = getXPath(pXmlDoc, sFrameXPath[i], "left").toInt64();
        const auto nFlyTop = getXPath(pXmlDoc, sFrameXPath[i], "top").toInt64();
        const auto nFlyWidth = getXPath(pXmlDoc, sFrameXPath[i], "width").toInt64();
        const auto nFlyHeight = getXPath(pXmlDoc, sFrameXPath[i], "height").toInt64();

        const auto aFrame = SwRect(nFlyLeft, nFlyTop, nFlyWidth, nFlyHeight);

        CPPUNIT_ASSERT_MESSAGE("Textbox must be inside the shape!", aChildShape.Contains(aFrame));
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter2, testTdf116256)
{
    // Open bugdoc
    createSwDoc("tdf116256.docx");

    // Get the textbox
    uno::Reference<beans::XPropertySet> xTextBox(getShape(2), uno::UNO_QUERY_THROW);

    // Ensure that is a real textbox, and follows the text flow
    CPPUNIT_ASSERT(xTextBox->getPropertyValue(u"TextBox"_ustr).get<bool>());
    CPPUNIT_ASSERT(xTextBox->getPropertyValue(u"IsFollowingTextFlow"_ustr).get<bool>());

    // Parse the layout
    auto pLayout = parseLayoutDump();
    // Get the position of the shape
    const auto nCellLeft
        = getXPath(pLayout, "//page/body/txt/anchored/fly/tab/row[1]/cell/infos/bounds", "left")
              .toInt64();
    const auto nCellTop
        = getXPath(pLayout, "//page/body/txt/anchored/fly/tab/row[1]/cell/infos/bounds", "top")
              .toInt64();
    // Get the position of the textframe too.
    const auto nTextBoxFrameLeft
        = getXPath(pLayout,
                   "/root/page/body/txt/anchored/fly/tab/row[1]/cell/txt/anchored/fly/infos/bounds",
                   "left")
              .toInt64();
    const auto nTextBoxFrameTop
        = getXPath(pLayout,
                   "/root/page/body/txt/anchored/fly/tab/row[1]/cell/txt/anchored/fly/infos/bounds",
                   "top")
              .toInt64();

    // Without the fix in place these were less than they supposed to.
    CPPUNIT_ASSERT_GREATEREQUAL(nCellLeft, nTextBoxFrameLeft);
    CPPUNIT_ASSERT_GREATEREQUAL(nCellTop, nTextBoxFrameTop);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
