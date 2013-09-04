/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <vcl/svapp.hxx>
#include <smdll.hxx>
#include <document.hxx>
#include <view.hxx>

#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>

#include <svl/stritem.hxx>

#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>

#include <sfx2/zoomitem.hxx>

SV_DECL_REF(SmDocShell)
SV_IMPL_REF(SmDocShell)

using namespace ::com::sun::star;

namespace {

class Test : public test::BootstrapFixture
{
public:
    // init
    virtual void setUp();
    virtual void tearDown();

    // tests
    void editUndoRedo();
    void editMarker();
    void editFailure();

    void viewZoom();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(editUndoRedo);
    CPPUNIT_TEST(editMarker);
    CPPUNIT_TEST(editFailure);
    CPPUNIT_TEST(viewZoom);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XMultiComponentFactory> m_xFactory;

    SfxBindings m_aBindings;
    SfxDispatcher *m_pDispatcher;
    SmCmdBoxWindow *m_pSmCmdBoxWindow;
    SmEditWindow *m_pEditWindow;
    SmDocShellRef m_xDocShRef;
    SmViewShell *m_pViewShell;
};

void Test::setUp()
{
    BootstrapFixture::setUp();

    SmGlobals::ensure();

    m_xDocShRef = new SmDocShell(
        SFXMODEL_STANDARD |
        SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
        SFXMODEL_DISABLE_DOCUMENT_RECOVERY);
    m_xDocShRef->DoInitNew(0);

    SfxViewFrame *pViewFrame = SfxViewFrame::LoadHiddenDocument(*m_xDocShRef, 0);

    CPPUNIT_ASSERT_MESSAGE("Should have a SfxViewFrame", pViewFrame);

    m_pDispatcher = new SfxDispatcher(pViewFrame);
    m_aBindings.SetDispatcher(m_pDispatcher);
    m_aBindings.EnterRegistrations();
    m_pSmCmdBoxWindow = new SmCmdBoxWindow(&m_aBindings, NULL, NULL);
    m_aBindings.LeaveRegistrations();
    m_pEditWindow = new SmEditWindow(*m_pSmCmdBoxWindow);
    m_pViewShell = m_pEditWindow->GetView();
    CPPUNIT_ASSERT_MESSAGE("Should have a SmViewShell", m_pViewShell);
}

void Test::tearDown()
{
    delete m_pEditWindow;
    delete m_pSmCmdBoxWindow;
    delete m_pDispatcher;
    m_xDocShRef.Clear();

    BootstrapFixture::tearDown();
}

void Test::editMarker()
{
    {
        OUString sMarkedText("<?> under <?> under <?>");
        m_pEditWindow->SetText(sMarkedText);
        m_pEditWindow->Flush();
        OUString sFinalText = m_pEditWindow->GetText();
        CPPUNIT_ASSERT_MESSAGE("Should be equal text", sFinalText == sMarkedText);
    }

    {
        OUString sTargetText("a under b under c");

        m_pEditWindow->SelNextMark();
        m_pEditWindow->Delete();
        m_pEditWindow->InsertText("a");

        m_pEditWindow->SelNextMark();
        m_pEditWindow->SelNextMark();
        m_pEditWindow->Delete();
        m_pEditWindow->InsertText("c");

        m_pEditWindow->SelPrevMark();
        m_pEditWindow->Delete();
        m_pEditWindow->InsertText("b");

        m_pEditWindow->Flush();
        OUString sFinalText = m_pEditWindow->GetText();
        CPPUNIT_ASSERT_MESSAGE("Should be a under b under c", sFinalText == sTargetText);
    }

    {
        m_pEditWindow->SetText(OUString());
        m_pEditWindow->Flush();
    }
}

void Test::editFailure()
{
    m_xDocShRef->SetText("color a b over {a/}");

    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser().NextError();

    CPPUNIT_ASSERT_MESSAGE("Should be a PE_COLOR_EXPECTED",
        pErrorDesc && pErrorDesc->Type == PE_COLOR_EXPECTED);

    pErrorDesc = m_xDocShRef->GetParser().PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be a PE_UNEXPECTED_CHAR",
        pErrorDesc && pErrorDesc->Type == PE_UNEXPECTED_CHAR);

    pErrorDesc = m_xDocShRef->GetParser().PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be a PE_RGROUP_EXPECTED",
        pErrorDesc && pErrorDesc->Type == PE_RGROUP_EXPECTED);

    const SmErrorDesc *pLastErrorDesc = m_xDocShRef->GetParser().PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be three syntax errors",
        pLastErrorDesc && pLastErrorDesc == pErrorDesc);
}

void Test::editUndoRedo()
{
    EditEngine &rEditEngine = m_xDocShRef->GetEditEngine();

    OUString sStringOne("a under b");
    {
        rEditEngine.SetText(0, sStringOne);
        m_xDocShRef->UpdateText();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Strings must match", sStringOne == sFinalText);
    }

    OUString sStringTwo("a over b");
    {
        rEditEngine.SetText(0, sStringTwo);
        m_xDocShRef->UpdateText();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Strings must match", sStringTwo == sFinalText);
    }

    SfxRequest aUndo(SID_UNDO, SFX_CALLMODE_SYNCHRON, m_xDocShRef->GetPool());

    {
        m_xDocShRef->Execute(aUndo);
        m_xDocShRef->UpdateText();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Strings much match", sStringOne == sFinalText);
    }

    {
        m_xDocShRef->Execute(aUndo);
        m_xDocShRef->UpdateText();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Must now be empty", !sFinalText.getLength());
    }

    SfxRequest aRedo(SID_REDO, SFX_CALLMODE_SYNCHRON, m_xDocShRef->GetPool());
    {
        m_xDocShRef->Execute(aRedo);
        m_xDocShRef->UpdateText();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Strings much match", sStringOne == sFinalText);
    }

    {
        rEditEngine.SetText(0, OUString());
        m_xDocShRef->UpdateText();
        rEditEngine.ClearModifyFlag();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Must be empty", !sFinalText.getLength());
    }

}

void Test::viewZoom()
{
    sal_uInt16 nOrigZoom, nNextZoom, nFinalZoom;

    EditEngine &rEditEngine = m_xDocShRef->GetEditEngine();

    OUString sStringOne("a under b");
    {
        rEditEngine.SetText(0, sStringOne);
        m_xDocShRef->UpdateText();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Strings must match", sStringOne == sFinalText);
    }

    SmGraphicWindow &rGraphicWindow = m_pViewShell->GetGraphicWindow();
    rGraphicWindow.SetSizePixel(Size(1024, 800));
    nOrigZoom = rGraphicWindow.GetZoom();

    {
        SfxRequest aZoomIn(SID_ZOOMIN, SFX_CALLMODE_SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoomIn);
        nNextZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be bigger", nNextZoom > nOrigZoom);
    }

    {
        SfxRequest aZoomOut(SID_ZOOMOUT, SFX_CALLMODE_SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoomOut);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be equal", nFinalZoom == nOrigZoom);
    }

    sal_uInt16 nOptimalZoom=0;

    {
        SfxRequest aZoom(SID_ZOOM_OPTIMAL, SFX_CALLMODE_SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoom);
        nOptimalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be about 800%", nOptimalZoom > nOrigZoom);
    }

    {
        SfxItemSet aSet(m_xDocShRef->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SVX_ZOOM_OPTIMAL, 0));
        SfxRequest aZoom(SID_ATTR_ZOOM, SFX_CALLMODE_SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be optimal zoom", nFinalZoom == nOptimalZoom);
    }

//To-Do: investigate GetPrinter logic of SVX_ZOOM_PAGEWIDTH/SVX_ZOOM_WHOLEPAGE to ensure
//consistent value regardless of
#if 0
    {
        SfxRequest aZoomOut(SID_ZOOMOUT, SFX_CALLMODE_SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoomOut);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should not be optimal zoom", nFinalZoom != nOptimalZoom);

        SfxItemSet aSet(m_xDocShRef->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SVX_ZOOM_PAGEWIDTH, 0));
        SfxRequest aZoom(SID_ATTR_ZOOM, SFX_CALLMODE_SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be same as optimal zoom", nFinalZoom == nOptimalZoom);
    }

    {
        SfxRequest aZoomOut(SID_ZOOMOUT, SFX_CALLMODE_SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoomOut);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should not be optimal zoom", nFinalZoom != nOptimalZoom);

        SfxItemSet aSet(m_xDocShRef->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SVX_ZOOM_WHOLEPAGE, 0));
        SfxRequest aZoom(SID_ATTR_ZOOM, SFX_CALLMODE_SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be same as optimal zoom", nFinalZoom == nOptimalZoom);
    }
#endif

    {
        SfxRequest aZoomOut(SID_ZOOMOUT, SFX_CALLMODE_SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoomOut);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should not be optimal zoom", nFinalZoom != nOptimalZoom);

        SfxItemSet aSet(m_xDocShRef->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SVX_ZOOM_PERCENT, 50));
        SfxRequest aZoom(SID_ATTR_ZOOM, SFX_CALLMODE_SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be 50%", nFinalZoom == 50);
    }

    {
        SfxItemSet aSet(m_xDocShRef->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SVX_ZOOM_PERCENT, 5));
        SfxRequest aZoom(SID_ATTR_ZOOM, SFX_CALLMODE_SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be Clipped to 25%", nFinalZoom == 25);
    }

    {
        SfxItemSet aSet(m_xDocShRef->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SVX_ZOOM_PERCENT, 1000));
        SfxRequest aZoom(SID_ATTR_ZOOM, SFX_CALLMODE_SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be Clipped to 800%", nFinalZoom == 800);
    }

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
