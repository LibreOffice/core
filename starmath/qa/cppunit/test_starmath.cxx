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

#include <smdll.hxx>
#include <document.hxx>
#include <view.hxx>

#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>

#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>

#include <sfx2/zoomitem.hxx>
#include <memory>

typedef tools::SvRef<SmDocShell> SmDocShellRef;

using namespace ::com::sun::star;

namespace {

class Test : public test::BootstrapFixture
{
public:
    Test();

    // init
    virtual void setUp() override;
    virtual void tearDown() override;

    // tests
    void editUndoRedo();
    void editMarker();
    void editFailure();

    void replacePlaceholder();
    void viewZoom();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(editUndoRedo);
    CPPUNIT_TEST(editMarker);
    CPPUNIT_TEST(editFailure);
    CPPUNIT_TEST(replacePlaceholder);
    CPPUNIT_TEST(viewZoom);
    CPPUNIT_TEST_SUITE_END();

private:
    SfxBindings m_aBindings;
    std::unique_ptr<SfxDispatcher> m_pDispatcher;
    VclPtr<SmCmdBoxWindow> m_pSmCmdBoxWindow;
    VclPtr<SmEditWindow> m_pEditWindow;
    SmDocShellRef m_xDocShRef;
    SmViewShell *m_pViewShell;
};

Test::Test()
    : m_pViewShell(nullptr)
{
}

void Test::setUp()
{
    BootstrapFixture::setUp();

    SmGlobals::ensure();

    m_xDocShRef = new SmDocShell(
        SfxModelFlags::EMBEDDED_OBJECT |
        SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
        SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    m_xDocShRef->DoInitNew();

    SfxViewFrame *pViewFrame = SfxViewFrame::LoadHiddenDocument(*m_xDocShRef, 0);

    CPPUNIT_ASSERT_MESSAGE("Should have a SfxViewFrame", pViewFrame);

    m_pDispatcher.reset(new SfxDispatcher(pViewFrame));
    m_aBindings.SetDispatcher(m_pDispatcher.get());
    m_aBindings.EnterRegistrations();
    m_pSmCmdBoxWindow.reset(VclPtr<SmCmdBoxWindow>::Create(&m_aBindings, nullptr, nullptr));
    m_aBindings.LeaveRegistrations();
    m_pEditWindow = VclPtr<SmEditWindow>::Create(*m_pSmCmdBoxWindow);
    m_pViewShell = m_pEditWindow->GetView();
    CPPUNIT_ASSERT_MESSAGE("Should have a SmViewShell", m_pViewShell);
}

void Test::tearDown()
{
    m_pEditWindow.disposeAndClear();
    m_pSmCmdBoxWindow.disposeAndClear();
    m_pDispatcher.reset();
    m_xDocShRef->DoClose();
    m_xDocShRef.clear();

    BootstrapFixture::tearDown();
}

void Test::editMarker()
{
    {
        OUString sMarkedText("<?> under <?> under <?>");
        m_pEditWindow->SetText(sMarkedText);
        m_pEditWindow->Flush();
        OUString sFinalText = m_pEditWindow->GetText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be equal text", sMarkedText, sFinalText);
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be a under b under c", sTargetText, sFinalText);
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
        pErrorDesc && pErrorDesc->m_eType == PE_COLOR_EXPECTED);

    pErrorDesc = m_xDocShRef->GetParser().PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be a PE_UNEXPECTED_CHAR",
        pErrorDesc && pErrorDesc->m_eType == PE_UNEXPECTED_CHAR);

    pErrorDesc = m_xDocShRef->GetParser().PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be a PE_RGROUP_EXPECTED",
        pErrorDesc && pErrorDesc->m_eType == PE_RGROUP_EXPECTED);

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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Strings must match", sFinalText, sStringOne);
    }

    OUString sStringTwo("a over b");
    {
        rEditEngine.SetText(0, sStringTwo);
        m_xDocShRef->UpdateText();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Strings must match", sFinalText, sStringTwo);
    }

    SfxRequest aUndo(SID_UNDO, SfxCallMode::SYNCHRON, SmDocShell::GetPool());

    {
        m_xDocShRef->Execute(aUndo);
        m_xDocShRef->UpdateText();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Strings much match", sFinalText, sStringOne);
    }

    {
        m_xDocShRef->Execute(aUndo);
        m_xDocShRef->UpdateText();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Must now be empty", !sFinalText.getLength());
    }

    SfxRequest aRedo(SID_REDO, SfxCallMode::SYNCHRON, SmDocShell::GetPool());
    {
        m_xDocShRef->Execute(aRedo);
        m_xDocShRef->UpdateText();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Strings much match", sFinalText, sStringOne);
    }

    {
        rEditEngine.SetText(0, OUString());
        m_xDocShRef->UpdateText();
        rEditEngine.ClearModifyFlag();
        OUString sFinalText = m_xDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Must be empty", !sFinalText.getLength());
    }

}

void Test::replacePlaceholder()
{
    // Test the placeholder replacement. In this case, testing 'a + b', it
    // should return '+a + b' when selecting '+<?>' in ElementsDock
    m_pEditWindow->SetText("a + b");
    m_pEditWindow->SelectAll();
    m_pEditWindow->InsertText("+<?>");
    OUString sFinalText = m_pEditWindow->GetText();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be '+a + b'", OUString("+a + b"), sFinalText);
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Strings must match", sFinalText, sStringOne);
    }

    SmGraphicWindow &rGraphicWindow = m_pViewShell->GetGraphicWindow();
    rGraphicWindow.SetSizePixel(Size(1024, 800));
    nOrigZoom = rGraphicWindow.GetZoom();

    {
        SfxRequest aZoomIn(SID_ZOOMIN, SfxCallMode::SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoomIn);
        nNextZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be bigger", nNextZoom > nOrigZoom);
    }

    {
        SfxRequest aZoomOut(SID_ZOOMOUT, SfxCallMode::SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoomOut);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be equal", nOrigZoom, nFinalZoom);
    }

    sal_uInt16 nOptimalZoom=0;

    {
        SfxRequest aZoom(SID_ZOOM_OPTIMAL, SfxCallMode::SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoom);
        nOptimalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be about 800%", nOptimalZoom > nOrigZoom);
    }

    {
        SfxItemSet aSet(SmDocShell::GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SvxZoomType::OPTIMAL, 0));
        SfxRequest aZoom(SID_ATTR_ZOOM, SfxCallMode::SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be optimal zoom", nOptimalZoom, nFinalZoom);
    }

//To-Do: investigate GetPrinter logic of SvxZoomType::PAGEWIDTH/SvxZoomType::WHOLEPAGE to ensure
//consistent value regardless of
#if 0
    {
        SfxRequest aZoomOut(SID_ZOOMOUT, SfxCallMode::SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoomOut);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should not be optimal zoom", nFinalZoom != nOptimalZoom);

        SfxItemSet aSet(m_xDocShRef->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SvxZoomType::PAGEWIDTH, 0));
        SfxRequest aZoom(SID_ATTR_ZOOM, SfxCallMode::SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be same as optimal zoom", nFinalZoom == nOptimalZoom);
    }

    {
        SfxRequest aZoomOut(SID_ZOOMOUT, SfxCallMode::SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoomOut);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should not be optimal zoom", nFinalZoom != nOptimalZoom);

        SfxItemSet aSet(m_xDocShRef->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SvxZoomType::WHOLEPAGE, 0));
        SfxRequest aZoom(SID_ATTR_ZOOM, SfxCallMode::SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be same as optimal zoom", nFinalZoom == nOptimalZoom);
    }
#endif

    {
        SfxRequest aZoomOut(SID_ZOOMOUT, SfxCallMode::SYNCHRON, m_pViewShell->GetPool());
        m_pViewShell->Execute(aZoomOut);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should not be optimal zoom", nFinalZoom != nOptimalZoom);

        SfxItemSet aSet(SmDocShell::GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SvxZoomType::PERCENT, 50));
        SfxRequest aZoom(SID_ATTR_ZOOM, SfxCallMode::SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be 50%", static_cast<sal_uInt16>(50), nFinalZoom);
    }

    {
        SfxItemSet aSet(SmDocShell::GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SvxZoomType::PERCENT, 5));
        SfxRequest aZoom(SID_ATTR_ZOOM, SfxCallMode::SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be Clipped to 25%", static_cast<sal_uInt16>(25), nFinalZoom);
    }

    {
        SfxItemSet aSet(SmDocShell::GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
        aSet.Put(SvxZoomItem(SvxZoomType::PERCENT, 1000));
        SfxRequest aZoom(SID_ATTR_ZOOM, SfxCallMode::SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be Clipped to 800%", static_cast<sal_uInt16>(800), nFinalZoom);
    }

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
