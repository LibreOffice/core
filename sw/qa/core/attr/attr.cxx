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

namespace
{
/// Covers sw/source/core/attr/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/attr/data/")
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
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
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
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
