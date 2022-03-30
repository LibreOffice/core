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

#include <com/sun/star/frame/DispatchHelper.hpp>

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/core/attr/data/";

/// Covers sw/source/core/attr/ fixes.
class Test : public SwModelTestBase
{
public:
    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* Test::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

static void dispatchCommand(const uno::Reference<lang::XComponent>& xComponent,
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

CPPUNIT_TEST_FIXTURE(Test, testSwAttrSet)
{
    // Given a document with track changes and the whole document is selected:
    SwDoc* pDoc = createDoc("attr-set.docx");
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
