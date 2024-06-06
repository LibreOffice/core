/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <docsh.hxx>
#include <swdtflvr.hxx>
#include <swmodule.hxx>
#include <frmmgr.hxx>
#include <frameformats.hxx>
#include <formatflysplit.hxx>
#include <formatwraptextatflystart.hxx>

namespace
{
/// Covers sw/source/core/attr/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/attr/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSwAttrSet)
{
    // Given a document with track changes and the whole document is selected:
    createSwDoc("attr-set.docx");
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    rtl::Reference<SwTransferable> xTransferable(new SwTransferable(*pWrtShell));
    SwModule* pMod = SW_MOD();
    SwTransferable* pOldTransferable = pMod->m_pXSelection;
    pMod->m_pXSelection = xTransferable.get();

    // When getting the plain text version of the selection:
    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = "text/plain;charset=utf-16";
    aFlavor.DataType = cppu::UnoType<OUString>::get();
    uno::Any aData = xTransferable->getTransferData(aFlavor);

    // Then make sure we get data without crashing:
    CPPUNIT_ASSERT(aData.hasValue());
    pMod->m_pXSelection = pOldTransferable;
}

CPPUNIT_TEST_FIXTURE(Test, testFormatFlySplit)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    RndStdIds eAnchor = RndStdIds::FLY_AT_PARA;
    aMgr.InsertFlyFrame(eAnchor, aMgr.GetPos(), aMgr.GetSize());
    SwDoc* pDoc = getSwDoc();
    sw::SpzFrameFormats& rFlys = *pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFly = rFlys[0];
    CPPUNIT_ASSERT(!pFly->GetAttrSet().GetFlySplit().GetValue());

    SfxItemSet aSet(pFly->GetAttrSet());
    SwFormatFlySplit aItem(true);
    aSet.Put(aItem);
    pDoc->SetFlyFrameAttr(*pFly, aSet);

    CPPUNIT_ASSERT(pFly->GetAttrSet().GetFlySplit().GetValue());
}

CPPUNIT_TEST_FIXTURE(Test, testFormatWrapTextAtFlyStart)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    RndStdIds eAnchor = RndStdIds::FLY_AT_PARA;
    aMgr.InsertFlyFrame(eAnchor, aMgr.GetPos(), aMgr.GetSize());
    SwDoc* pDoc = getSwDoc();
    sw::SpzFrameFormats& rFlys = *pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFly = rFlys[0];
    CPPUNIT_ASSERT(!pFly->GetAttrSet().GetWrapTextAtFlyStart().GetValue());

    SfxItemSet aSet(pFly->GetAttrSet());
    SwFormatWrapTextAtFlyStart aItem(true);
    aSet.Put(aItem);
    pDoc->SetFlyFrameAttr(*pFly, aSet);

    CPPUNIT_ASSERT(pFly->GetAttrSet().GetWrapTextAtFlyStart().GetValue());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
