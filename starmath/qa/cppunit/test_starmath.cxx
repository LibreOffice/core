/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_fonts.h>
#include <vcl/print.hxx>

#include <test/bootstrapfixture.hxx>

#include <smdll.hxx>
#include <document.hxx>
#include <view.hxx>

#include <tmpdevice.hxx>

#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>

#include <editeng/editeng.hxx>

#include <sfx2/zoomitem.hxx>
#include <starmath.hrc>
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
    void ParseErrorUnexpectedToken();
    void ParseErrorPoundExpected();
    void ParseErrorColorExpected();
    void ParseErrorLgroupExpected();
    void ParseErrorRgroupExpected();
    void ParseErrorLbraceExpected();
    void ParseErrorRbraceExpected();
    void ParseErrorParentMismatch();
    void ParseErrorRightExpected();
    void ParseErrorFontExpected();
    void ParseErrorSizeExpected();
    void ParseErrorDoubleAlign();
    void ParseErrorDoubleSubsupscript();

    void replacePlaceholder();
    void viewZoom();

#if HAVE_MORE_FONTS
    void testSmTmpDeviceRestoreFont();
#endif

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(editUndoRedo);
    CPPUNIT_TEST(editMarker);
    CPPUNIT_TEST(editFailure);
    CPPUNIT_TEST(ParseErrorUnexpectedToken);
    CPPUNIT_TEST(ParseErrorPoundExpected);
    CPPUNIT_TEST(ParseErrorColorExpected);
    CPPUNIT_TEST(ParseErrorLgroupExpected);
    CPPUNIT_TEST(ParseErrorRgroupExpected);
    CPPUNIT_TEST(ParseErrorLbraceExpected);
    CPPUNIT_TEST(ParseErrorRbraceExpected);
    CPPUNIT_TEST(ParseErrorParentMismatch);
    CPPUNIT_TEST(ParseErrorRightExpected);
    CPPUNIT_TEST(ParseErrorFontExpected);
    CPPUNIT_TEST(ParseErrorSizeExpected);
    CPPUNIT_TEST(ParseErrorDoubleAlign);
    CPPUNIT_TEST(ParseErrorDoubleSubsupscript);
    CPPUNIT_TEST(replacePlaceholder);
    CPPUNIT_TEST(viewZoom);
#if HAVE_MORE_FONTS
    CPPUNIT_TEST(testSmTmpDeviceRestoreFont);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    SfxBindings m_aBindings;
    std::unique_ptr<SfxDispatcher> m_pDispatcher;
    VclPtr<SmCmdBoxWindow> m_pSmCmdBoxWindow;
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

    SfxViewFrame *pViewFrame = SfxViewFrame::LoadHiddenDocument(*m_xDocShRef, SFX_INTERFACE_NONE);

    CPPUNIT_ASSERT_MESSAGE("Should have a SfxViewFrame", pViewFrame);

    m_pDispatcher.reset(new SfxDispatcher(pViewFrame));
    m_aBindings.SetDispatcher(m_pDispatcher.get());
    m_aBindings.EnterRegistrations();
    m_pSmCmdBoxWindow.reset(VclPtr<SmCmdBoxWindow>::Create(&m_aBindings, nullptr, nullptr));
    m_aBindings.LeaveRegistrations();
    m_pViewShell = m_pSmCmdBoxWindow->GetView();
    CPPUNIT_ASSERT_MESSAGE("Should have a SmViewShell", m_pViewShell);
}

void Test::tearDown()
{
    m_pSmCmdBoxWindow.disposeAndClear();
    m_pDispatcher.reset();
    m_xDocShRef->DoClose();
    m_xDocShRef.clear();

    BootstrapFixture::tearDown();
}

#if HAVE_MORE_FONTS
void Test::testSmTmpDeviceRestoreFont()
{
    ScopedVclPtrInstance<Printer> pPrinter;

    OUString aFontName("Linux Libertine G");
    CPPUNIT_ASSERT(pPrinter->IsFontAvailable(aFontName));

    vcl::Font aOriginalFont = pPrinter->GetFont();
    aOriginalFont.SetColor(COL_RED);
    pPrinter->SetTextColor(COL_RED);

    vcl::Font aNewFont;

    {
        bool bUseMap100th_mm = true;
        SmTmpDevice aTmpDev(*pPrinter, bUseMap100th_mm);

        aNewFont = pPrinter->GetFont();
        aNewFont.SetFamilyName(aFontName);
        aTmpDev.SetFont(aNewFont);

        CPPUNIT_ASSERT_EQUAL(aFontName, pPrinter->GetFont().GetFamilyName());
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, pPrinter->GetTextColor());
    }

    CPPUNIT_ASSERT(aNewFont != pPrinter->GetFont());
    CPPUNIT_ASSERT_EQUAL(COL_RED, pPrinter->GetTextColor());
}
#endif

void Test::editMarker()
{
    SmEditWindow& rEditWindow = m_pSmCmdBoxWindow->GetEditWindow();
    {
        OUString sMarkedText("<?> under <?> under <?>");
        rEditWindow.SetText(sMarkedText);
        rEditWindow.Flush();
        OUString sFinalText = rEditWindow.GetText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be equal text", sMarkedText, sFinalText);
    }

    {
        ESelection aSelection;

        rEditWindow.SelNextMark();
        rEditWindow.Delete();
        rEditWindow.InsertText("a");

        rEditWindow.SelNextMark();
        rEditWindow.SelNextMark();
        rEditWindow.Delete();
        rEditWindow.InsertText("c");

        // should be safe i.e. do nothing
        rEditWindow.SelNextMark();
        aSelection = rEditWindow.GetSelection();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aSelection.nStartPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(19), aSelection.nStartPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aSelection.nEndPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(19), aSelection.nEndPos);

        rEditWindow.SelPrevMark();
        rEditWindow.Delete();
        rEditWindow.InsertText("b");

        // tdf#106116: should be safe i.e. do nothing
        rEditWindow.SelPrevMark();
        aSelection = rEditWindow.GetSelection();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aSelection.nStartPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aSelection.nStartPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aSelection.nEndPara);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aSelection.nEndPos);

        rEditWindow.Flush();
        OUString sFinalText = rEditWindow.GetText();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be a under b under c", OUString("a under b under c"), sFinalText);
    }

    {
        rEditWindow.SetText(OUString());
        rEditWindow.Flush();
    }
}

void Test::editFailure()
{
    m_xDocShRef->SetText("color a b over {a/}");

    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();

    CPPUNIT_ASSERT_MESSAGE("Should be a SmParseError::ColorExpected",
        pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be a SmParseError::ColorExpected",
        SmParseError::ColorExpected, pErrorDesc->m_eType);

    pErrorDesc = m_xDocShRef->GetParser()->PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be a SmParseError::UnexpectedChar",
        pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be a SmParseError::UnexpectedChar",
        SmParseError::UnexpectedChar, pErrorDesc->m_eType);

    pErrorDesc = m_xDocShRef->GetParser()->PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be a SmParseError::RgroupExpected",
        pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be a SmParseError::RgroupExpected",
        SmParseError::RgroupExpected, pErrorDesc->m_eType);

    const SmErrorDesc *pLastErrorDesc = m_xDocShRef->GetParser()->PrevError();

    CPPUNIT_ASSERT_MESSAGE("Should be three syntax errors",
        pLastErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be three syntax errors",
        pErrorDesc, pLastErrorDesc);
}

void Test::ParseErrorUnexpectedToken()
{
    m_xDocShRef->SetText("\\foo");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::UnexpectedToken expected",
                           SmParseError::UnexpectedToken, pErrorDesc->m_eType);
}

void Test::ParseErrorPoundExpected()
{
    m_xDocShRef->SetText("matrix {1#2##a##b#c}");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::PoundExpected expected",
                           SmParseError::PoundExpected, pErrorDesc->m_eType);
}

void Test::ParseErrorColorExpected()
{
    m_xDocShRef->SetText("color 42 x");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::ColorExpected expected",
                           SmParseError::ColorExpected, pErrorDesc->m_eType);
}

void Test::ParseErrorLgroupExpected()
{
    m_xDocShRef->SetText("stack 42");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::LgroupExpected expected",
                           SmParseError::LgroupExpected, pErrorDesc->m_eType);
}

void Test::ParseErrorRgroupExpected()
{
    m_xDocShRef->SetText("stack {a#b#c)");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::RgroupExpected expected",
                           SmParseError::RgroupExpected, pErrorDesc->m_eType);
}

void Test::ParseErrorLbraceExpected()
{
    m_xDocShRef->SetText("left 42");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::LbraceExpected expected",
                           SmParseError::LbraceExpected, pErrorDesc->m_eType);
}

void Test::ParseErrorRbraceExpected()
{
    m_xDocShRef->SetText("left ( foo right x");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::RbraceExpected expected",
                           SmParseError::RbraceExpected, pErrorDesc->m_eType);
}

void Test::ParseErrorParentMismatch()
{
    m_xDocShRef->SetText("lbrace foo rceil");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::ParentMismatch expected",
                           SmParseError::ParentMismatch, pErrorDesc->m_eType);
}

void Test::ParseErrorRightExpected()
{
    m_xDocShRef->SetText("left ( x mline y )");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::RightExpected expected",
                           SmParseError::RightExpected, pErrorDesc->m_eType);
}

void Test::ParseErrorFontExpected()
{
    m_xDocShRef->SetText("font small bar");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::FontExpected expected",
                           SmParseError::FontExpected, pErrorDesc->m_eType);
}

void Test::ParseErrorSizeExpected()
{
    m_xDocShRef->SetText("size small baz");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::SizeExpected expected",
                           SmParseError::SizeExpected, pErrorDesc->m_eType);
}

void Test::ParseErrorDoubleAlign()
{
    m_xDocShRef->SetText("alignl alignc x");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::DoubleAlign expected",
                           SmParseError::DoubleAlign, pErrorDesc->m_eType);
}

void Test::ParseErrorDoubleSubsupscript()
{
    m_xDocShRef->SetText("x_y_z");
    const SmErrorDesc *pErrorDesc = m_xDocShRef->GetParser()->NextError();
    CPPUNIT_ASSERT(pErrorDesc);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SmParseError::DoubleSubsupscript expected",
                           SmParseError::DoubleSubsupscript, pErrorDesc->m_eType);
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

    {
        OUString sStringTwo("a over b");
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
        CPPUNIT_ASSERT_MESSAGE("Must now be empty", sFinalText.isEmpty());
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
        CPPUNIT_ASSERT_MESSAGE("Must be empty", sFinalText.isEmpty());
    }

}

void Test::replacePlaceholder()
{
    SmEditWindow& rEditWindow = m_pSmCmdBoxWindow->GetEditWindow();
    // Test the placeholder replacement. In this case, testing 'a + b', it
    // should return '+a + b' when selecting '+<?>' in ElementsDock
    rEditWindow.SetText("a + b");
    rEditWindow.SelectAll();
    rEditWindow.InsertText("+<?>");
    OUString sFinalText = rEditWindow.GetText();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be '+a + b'", OUString("+a + b"), sFinalText);
}

void Test::viewZoom()
{
    sal_uInt16 nOrigZoom, nFinalZoom;

    EditEngine &rEditEngine = m_xDocShRef->GetEditEngine();

    {
        OUString sStringOne("a under b");
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
        sal_uInt16 nNextZoom = rGraphicWindow.GetZoom();
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
        SfxItemSet aSet(SmDocShell::GetPool(), svl::Items<SID_ATTR_ZOOM, SID_ATTR_ZOOM>);
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

        SfxItemSet aSet(SmDocShell::GetPool(), svl::Items<SID_ATTR_ZOOM, SID_ATTR_ZOOM>);
        aSet.Put(SvxZoomItem(SvxZoomType::PERCENT, 50));
        SfxRequest aZoom(SID_ATTR_ZOOM, SfxCallMode::SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be 50%", static_cast<sal_uInt16>(50), nFinalZoom);
    }

    {
        SfxItemSet aSet(SmDocShell::GetPool(), svl::Items<SID_ATTR_ZOOM, SID_ATTR_ZOOM>);
        aSet.Put(SvxZoomItem(SvxZoomType::PERCENT, 5));
        SfxRequest aZoom(SID_ATTR_ZOOM, SfxCallMode::SYNCHRON, aSet);
        m_pViewShell->Execute(aZoom);
        nFinalZoom = rGraphicWindow.GetZoom();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be Clipped to 25%", static_cast<sal_uInt16>(25), nFinalZoom);
    }

    {
        SfxItemSet aSet(SmDocShell::GetPool(), svl::Items<SID_ATTR_ZOOM, SID_ATTR_ZOOM>);
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
