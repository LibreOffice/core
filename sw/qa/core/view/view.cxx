/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <svtools/embedhlp.hxx>
#include <vcl/graph.hxx>

#include <doc.hxx>
#include <docsh.hxx>
#include <fmtcntnt.hxx>
#include <frameformats.hxx>
#include <frmfmt.hxx>
#include <ndarr.hxx>
#include <ndindex.hxx>
#include <ndole.hxx>
#include <node.hxx>
#include <nodeoffset.hxx>
#include <wrtsh.hxx>

namespace
{
/// Covers sw/source/core/view/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/view/data/"_ustr)
    {
    }
};
}

CPPUNIT_TEST_FIXTURE(Test, testUpdateOleObjectPreviews)
{
    // Given a document with two embedded objects, both with broken native data:
    createSwDoc("update-ole-object-previews.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // When updating the previews of those embedded objects (right after document load, before
    // painting the OLE objects):
    pWrtShell->UpdateOleObjectPreviews();

    // Then make sure that the working preview of those objects are not lost:
    const auto pFormats = pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(pFormats);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pFormats->size());
    for (auto pFormat : *pFormats)
    {
        const SwNodeIndex* pNodeIndex = pFormat->GetContent().GetContentIdx();
        CPPUNIT_ASSERT(pNodeIndex);
        SwNode* pNode = pDoc->GetNodes()[pNodeIndex->GetIndex() + 1];
        SwOLENode* pOleNode = pNode->GetOLENode();
        CPPUNIT_ASSERT(pOleNode);
        SwOLEObj& rOleObj = pOleNode->GetOLEObj();
        svt::EmbeddedObjectRef& rObject = rOleObj.GetObject();
        // Without the accompanying fix in place, this test would have failed, the update broke the
        // preview of the second embedded object.
        CPPUNIT_ASSERT(!rObject.GetGraphic()->IsNone());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
