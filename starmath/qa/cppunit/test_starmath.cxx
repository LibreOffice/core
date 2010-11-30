/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"

#include "sal/config.h"

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop.hpp>

#include <vcl/svapp.hxx>
#include <smdll.hxx>
#include <document.hxx>
#include <view.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>

#include <svl/stritem.hxx>

#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>

#include "preextstl.h"
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>
#include "postextstl.h"

SO2_DECL_REF(SmDocShell)
SO2_IMPL_REF(SmDocShell)

using namespace ::com::sun::star;

namespace {

class Test : public CppUnit::TestFixture {
public:
    // init
    virtual void setUp();
    virtual void tearDown();

    // tests
    void createDocument();

    void tmEditUndoRedo(SmDocShellRef &rDocShRef);
    void tmEditAllClipboard(SmEditWindow &rEditWindow);
    void tmEditMarker(SmEditWindow &rEditWindow);
    void tmEditFailure(SmDocShellRef &rDocShRef);

    void tViewZoom(SmViewShell &rViewShell);

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(createDocument);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XMultiComponentFactory> m_xFactory;
};

void Test::setUp()
{
    m_xContext = cppu::defaultBootstrap_InitialComponentContext();
    m_xFactory = m_xContext->getServiceManager();

    uno::Reference<lang::XMultiServiceFactory> xSM(m_xFactory, uno::UNO_QUERY_THROW);

    //Without this we're crashing because callees are using
    //getProcessServiceFactory.  In general those should be removed in favour
    //of retaining references to the root ServiceFactory as its passed around
    comphelper::setProcessServiceFactory(xSM);

    InitVCL(xSM);

    SmDLL::Init();
}

void Test::tearDown()
{
    uno::Reference< lang::XComponent >(m_xContext, uno::UNO_QUERY_THROW)->dispose();
}

void Test::tmEditMarker(SmEditWindow &rEditWindow)
{
    {
        rtl::OUString sMarkedText(RTL_CONSTASCII_USTRINGPARAM("<?> under <?> under <?>"));
        rEditWindow.SetText(sMarkedText);
        rEditWindow.Flush();
        rtl::OUString sFinalText = rEditWindow.GetText();
        CPPUNIT_ASSERT_MESSAGE("Should be equal text", sFinalText == sMarkedText);
    }

    {
        rtl::OUString sTargetText(RTL_CONSTASCII_USTRINGPARAM("a under b under c"));

        rEditWindow.SelNextMark();
        rEditWindow.Cut();
        rEditWindow.InsertText(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a")));

        rEditWindow.SelNextMark();
        rEditWindow.SelNextMark();
        rEditWindow.Cut();
        rEditWindow.InsertText(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("c")));

        rEditWindow.SelPrevMark();
        rEditWindow.Cut();
        rEditWindow.InsertText(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("b")));

        rEditWindow.Flush();
        rtl::OUString sFinalText = rEditWindow.GetText();
        CPPUNIT_ASSERT_MESSAGE("Should be a under b under c", sFinalText == sTargetText);
    }

    {
        rEditWindow.SetText(rtl::OUString());
        rEditWindow.Flush();
    }
}

void Test::tmEditAllClipboard(SmEditWindow &rEditWindow)
{
    rtl::OUString sOriginalText(RTL_CONSTASCII_USTRINGPARAM("a over b"));

    {
        rEditWindow.SetText(sOriginalText);
        rEditWindow.Flush();
        rtl::OUString sFinalText = rEditWindow.GetText();
        CPPUNIT_ASSERT_MESSAGE("Should be equal text", sFinalText == sOriginalText);
    }

    {
        rEditWindow.SelectAll();
        rEditWindow.Cut();
        rEditWindow.Flush();
        rtl::OUString sFinalText = rEditWindow.GetText();
        CPPUNIT_ASSERT_MESSAGE("Should be empty", !sFinalText.getLength());
    }

    {
        rEditWindow.Paste();
        rEditWindow.Flush();
        rtl::OUString sFinalText = rEditWindow.GetText();
        CPPUNIT_ASSERT_MESSAGE("Should be equal text", sFinalText == sOriginalText);
    }

    {
        rEditWindow.SelectAll();
        rEditWindow.Copy();
    }

    {
        rtl::OUString sExpectedText(RTL_CONSTASCII_USTRINGPARAM("a over ba over b"));

        rEditWindow.Paste();
        rEditWindow.Paste();
        rEditWindow.Flush();
        rtl::OUString sFinalText = rEditWindow.GetText();
        CPPUNIT_ASSERT_MESSAGE("Should be equal text", sFinalText == sExpectedText);
    }

    {
        rEditWindow.SetText(rtl::OUString());
        rEditWindow.Flush();
    }
}

void Test::tmEditFailure(SmDocShellRef &rDocShRef)
{
    rDocShRef->SetText(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("color a b over {a/}")));

    const SmErrorDesc *pErrorDesc = rDocShRef->GetParser().NextError();

    CPPUNIT_ASSERT_MESSAGE("Should be a PE_UNEXPECTED_CHAR",
        pErrorDesc && pErrorDesc->Type == PE_UNEXPECTED_CHAR);

    pErrorDesc = rDocShRef->GetParser().PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be a PE_RGROUP_EXPECTED",
        pErrorDesc && pErrorDesc->Type == PE_RGROUP_EXPECTED);

    pErrorDesc = rDocShRef->GetParser().PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be a PE_COLOR_EXPECTED",
        pErrorDesc && pErrorDesc->Type == PE_COLOR_EXPECTED);

    const SmErrorDesc *pLastErrorDesc = rDocShRef->GetParser().PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be three syntax errors",
        pLastErrorDesc && pLastErrorDesc == pErrorDesc);
}

void Test::tmEditUndoRedo(SmDocShellRef &rDocShRef)
{
    EditEngine &rEditEngine = rDocShRef->GetEditEngine();

    rtl::OUString sStringOne(RTL_CONSTASCII_USTRINGPARAM("a under b"));
    {
        rEditEngine.SetText(0, sStringOne);
        rDocShRef->UpdateText();
        rtl::OUString sFinalText = rDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Strings must match", sStringOne == sFinalText);
    }

    rtl::OUString sStringTwo(RTL_CONSTASCII_USTRINGPARAM("a over b"));
    {
        rEditEngine.SetText(0, sStringTwo);
        rDocShRef->UpdateText();
        rtl::OUString sFinalText = rDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Strings must match", sStringTwo == sFinalText);
    }

    SfxRequest aUndo(SID_UNDO, SFX_CALLMODE_SYNCHRON, rDocShRef->GetPool());

    {
        rDocShRef->Execute(aUndo);
        rtl::OUString sFoo = rEditEngine.GetText();
        rDocShRef->UpdateText();
        rtl::OUString sFinalText = rDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Strings much match", sStringOne == sFinalText);
    }

    {
        rDocShRef->Execute(aUndo);
        rtl::OUString sFoo = rEditEngine.GetText();
        rDocShRef->UpdateText();
        rtl::OUString sFinalText = rDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Must now be empty", !sFinalText.getLength());
    }

    SfxRequest aRedo(SID_REDO, SFX_CALLMODE_SYNCHRON, rDocShRef->GetPool());
    {
        rDocShRef->Execute(aRedo);
        rtl::OUString sFoo = rEditEngine.GetText();
        rDocShRef->UpdateText();
        rtl::OUString sFinalText = rDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Strings much match", sStringOne == sFinalText);
    }

    {
        rEditEngine.SetText(0, rtl::OUString());
        rDocShRef->UpdateText();
        rEditEngine.ClearModifyFlag();
        rtl::OUString sFinalText = rDocShRef->GetText();
        CPPUNIT_ASSERT_MESSAGE("Must be empty", !sFinalText.getLength());
    }

}

void Test::tViewZoom(SmViewShell &rViewShell)
{
    sal_uInt16 nOrigZoom, nNextZoom, nFinalZoom;

    SmGraphicWindow &rGraphicWindow = rViewShell.GetGraphicWindow();
    nOrigZoom = rGraphicWindow.GetZoom();

    {
        SfxRequest aZoomIn(SID_ZOOMIN, SFX_CALLMODE_SYNCHRON, rViewShell.GetPool());
        rViewShell.Execute(aZoomIn);
        nNextZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be bigger", nNextZoom > nOrigZoom);
    }

    {
        SfxRequest aZoomOut(SID_ZOOMOUT, SFX_CALLMODE_SYNCHRON, rViewShell.GetPool());
        rViewShell.Execute(aZoomOut);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_MESSAGE("Should be equal", nFinalZoom == nOrigZoom);
    }
}

void Test::createDocument()
{
    SmDocShellRef xDocShRef = new SmDocShell(SFXOBJECTSHELL_STD_NORMAL);
    xDocShRef->DoInitNew(0);

    uno::Reference< frame::XFrame > xDesktop
        (m_xFactory->createInstanceWithContext(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")), m_xContext),
        uno::UNO_QUERY_THROW );

    SfxViewFrame *pViewFrame = SfxViewFrame::LoadHiddenDocument(*xDocShRef, 0);

    CPPUNIT_ASSERT_MESSAGE("Should have a SfxViewFrame", pViewFrame);

    SfxBindings aBindings;
    SfxDispatcher aDispatcher(pViewFrame);
    aBindings.SetDispatcher(&aDispatcher);
    SmCmdBoxWindow aSmCmdBoxWindow(&aBindings, NULL, NULL);
    SmEditWindow aEditWindow(aSmCmdBoxWindow);
    SmViewShell *pViewShell = aEditWindow.GetView();
    CPPUNIT_ASSERT_MESSAGE("Should have a SmViewShell", pViewShell);

    tmEditUndoRedo(xDocShRef);
    tmEditAllClipboard(aEditWindow);
    tmEditMarker(aEditWindow);
    tmEditFailure(xDocShRef);

    tViewZoom(*pViewShell);

    xDocShRef.Clear();
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
