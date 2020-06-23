/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/frame/DispatchHelper.hpp>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdpage.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>

#include <IDocumentContentOperations.hxx>
#include <cmdid.h>
#include <fmtanchr.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/uibase/shells/data/";

namespace
{
void dispatchCommand(const uno::Reference<lang::XComponent>& xComponent,
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
}

/// Covers sw/source/uibase/shells/ fixes.
class SwUibaseShellsTest : public SwModelTestBase
{
public:
    SwDoc* createDoc(const char* pName = nullptr);
};

SwDoc* SwUibaseShellsTest::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

CPPUNIT_TEST_FIXTURE(SwUibaseShellsTest, testOleSavePreviewUpdate)
{
    // Load a document with 2 charts in it. The second is down enough that you have to scroll to
    // trigger its rendering. Previews are missing for both.
    createDoc("ole-save-preview-update.odt");

    // Explicitly update OLE previews, etc.
    dispatchCommand(mxComponent, ".uno:UpdateAll", {});

    // Save the document and see if we get the previews.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), {});
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());

    // Without the accompanying fix in place, this test would have failed, because the object
    // replacements were not generated, even after UpdateAll.
    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Object 1"));
    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Object 2"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
