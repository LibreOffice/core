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
    void tmEditFailure(SmDocShellRef &rDocShRef);
    void tmEditMarker(SfxViewFrame &rViewShell);

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

void Test::tmEditMarker(SfxViewFrame &rViewFrame)
{
    SfxBindings aBindings;
    SfxDispatcher aDispatcher(&rViewFrame);
    aBindings.SetDispatcher(&aDispatcher);
    SmCmdBoxWindow aSmCmdBoxWindow(&aBindings, NULL, NULL);
    SmEditWindow aEditWindow(aSmCmdBoxWindow);
    aEditWindow.Flush();

    {
        rtl::OUString sMarkedText(RTL_CONSTASCII_USTRINGPARAM("<?> under <?> under <?>"));
        aEditWindow.SetText(sMarkedText);
        aEditWindow.Flush();
        rtl::OUString sFinalText = aEditWindow.GetText();
        CPPUNIT_ASSERT_MESSAGE("Should be equal text", sFinalText == sMarkedText);
    }

    {
        rtl::OUString sTargetText(RTL_CONSTASCII_USTRINGPARAM("a under b under c"));

        aEditWindow.SelNextMark();
        aEditWindow.Cut();
        aEditWindow.InsertText(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a")));

        aEditWindow.SelNextMark();
        aEditWindow.SelNextMark();
        aEditWindow.Cut();
        aEditWindow.InsertText(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("c")));

        aEditWindow.SelPrevMark();
        aEditWindow.Cut();
        aEditWindow.InsertText(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("b")));

        aEditWindow.Flush();
        rtl::OUString sFinalText = aEditWindow.GetText();
        CPPUNIT_ASSERT_MESSAGE("Should be a under b under c", sFinalText == sTargetText);
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

    CPPUNIT_ASSERT_MESSAGE("Should have SfxViewFrame", pViewFrame);

    EditEngine &rEditEngine = xDocShRef->GetEditEngine();
    Window aFoo(NULL, 0);
    EditView aEditView(&rEditEngine, &aFoo);
    rEditEngine.SetActiveView(&aEditView);

    tmEditUndoRedo(xDocShRef);
    tmEditFailure(xDocShRef);
    tmEditMarker(*pViewFrame);

    xDocShRef.Clear();
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
