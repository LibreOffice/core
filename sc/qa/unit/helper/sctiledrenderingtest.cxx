/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sctiledrenderingtest.hxx"

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/servicehelper.hxx>
#include <sfx2/lokhelper.hxx>
#include <vcl/scheduler.hxx>

#include <docuno.hxx>
#include <tabvwsh.hxx>

#include "sctestviewcallback.hxx"

using namespace com::sun::star;

ScTiledRenderingTest::ScTiledRenderingTest()
    : UnoApiXmlTest(u"/sc/qa/unit/tiledrendering/data/"_ustr)
    , m_callbackWrapper(&callback, this)
{
}

void ScTiledRenderingTest::setUp()
{
    UnoApiXmlTest::setUp();

    comphelper::LibreOfficeKit::setActive(true);
}

void ScTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }

    m_callbackWrapper.clear();

    comphelper::LibreOfficeKit::resetCompatFlag();

    comphelper::LibreOfficeKit::setActive(false);

    UnoApiXmlTest::tearDown();
}

ScModelObj* ScTiledRenderingTest::createDoc(const char* pName)
{
    loadFromFile(OUString::createFromAscii(pName));

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    return pModelObj;
}

void ScTiledRenderingTest::setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell)
{
    pViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
    m_callbackWrapper.setLOKViewId(SfxLokHelper::getView(pViewShell));
}

void ScTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<ScTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

void ScTiledRenderingTest::callbackImpl(int nType, const char* pPayload)
{
    switch (nType)
    {
        case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
        {
            OString aPayload(pPayload);
            sal_Int32 nIndex = 0;
            OString aToken = aPayload.getToken(0, ',', nIndex);
            m_aDocumentSize.setWidth(aToken.toInt32());
            aToken = aPayload.getToken(0, ',', nIndex);
            m_aDocumentSize.setHeight(aToken.toInt32());
            m_aDocSizeCondition.set();
        }
        break;
    }
}

void ScTiledRenderingTest::checkSampleInvalidation(const ScTestViewCallback& rView, bool bFullRow)
{
    // we expect invalidations, but that isn't really important
    CPPUNIT_ASSERT(rView.m_bInvalidateTiles);
    tools::Rectangle aInvalidation;
    for (const auto& rRect : rView.m_aInvalidations)
        aInvalidation.Union(rRect);
    if (!bFullRow)
    {
        // What matters is that we expect that the invalidation does not extend all the
        // way to the max right of the sheet.
        // Here we originally got 32212306 and now ~5056 for a single cell case
        CPPUNIT_ASSERT_LESSEQUAL(tools::Long(8000), aInvalidation.GetWidth());
    }
    else
    {
        // We expect RTL to continue to invalidate the entire row
        // from 0 to end of sheet (see ScDocShell::PostPaint, 'Extend to whole rows'),
        // which is different to the adjusted LTR case which
        // invalidated the row from left of edited cell to right of end
        // of sheet.
        CPPUNIT_ASSERT_LESSEQUAL(tools::Long(0), aInvalidation.Left());
        CPPUNIT_ASSERT_EQUAL(tools::Long(32212230), aInvalidation.Right());
    }
}

void ScTiledRenderingTest::cellInvalidationHelper(ScModelObj* pModelObj, ScTabViewShell* pView,
                                                  const ScAddress& rAdr, bool bAddText,
                                                  bool bFullRow)
{
    // view
    ScTestViewCallback aView;

    if (bAddText)
    {
        // Type "Hello World" in D8, process events to idle and don't commit yet
        typeCharsInCell("Hello World", rAdr.Col(), rAdr.Row(), pView, pModelObj, false, false);

        aView.m_bInvalidateTiles = false;
        aView.m_aInvalidations.clear();

        // commit text and process events to idle
        typeCharsInCell("", rAdr.Col(), rAdr.Row(), pView, pModelObj, true, true);
    }
    else // DeleteText
    {
        pView->SetCursor(rAdr.Col(), rAdr.Row());
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DELETE);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DELETE);
        Scheduler::ProcessEventsToIdle();
    }

    checkSampleInvalidation(aView, bFullRow);
}

void ScTiledRenderingTest::typeCharsInCell(const std::string& aStr, SCCOL nCol, SCROW nRow,
                                           ScTabViewShell* pView, ScModelObj* pModelObj,
                                           bool bInEdit, bool bCommit)
{
    if (!bInEdit)
        pView->SetCursor(nCol, nRow);

    for (const char& cChar : aStr)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, cChar, 0);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, cChar, 0);
        Scheduler::ProcessEventsToIdle();
    }

    if (bCommit)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
