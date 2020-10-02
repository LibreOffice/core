/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basegfx/polygon/b2dpolygon.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <memory>
#include <officecfg/Office/UI/Infobar.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/infobar.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <vcl/decoview.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weldutils.hxx>
#include <bitmaps.hlst>

using namespace std;
using namespace drawinglayer::geometry;
using namespace drawinglayer::processor2d;
using namespace drawinglayer::primitive2d;
using namespace drawinglayer::attribute;
using namespace basegfx;
using namespace css::frame;

namespace
{
void GetInfoBarColors(InfobarType ibType, BColor& rBackgroundColor, BColor& rForegroundColor,
                      BColor& rMessageColor)
{
    rMessageColor = basegfx::BColor(0.0, 0.0, 0.0);

    switch (ibType)
    {
        case InfobarType::INFO: // blue; #004785/0,71,133; #BDE5F8/189,229,248
            rBackgroundColor = basegfx::BColor(0.741, 0.898, 0.973);
            rForegroundColor = basegfx::BColor(0.0, 0.278, 0.522);
            break;
        case InfobarType::SUCCESS: // green; #32550C/50,85,12; #DFF2BF/223,242,191
            rBackgroundColor = basegfx::BColor(0.874, 0.949, 0.749);
            rForegroundColor = basegfx::BColor(0.196, 0.333, 0.047);
            break;
        case InfobarType::WARNING: // orange; #704300/112,67,0; #FEEFB3/254,239,179
            rBackgroundColor = basegfx::BColor(0.996, 0.937, 0.702);
            rForegroundColor = basegfx::BColor(0.439, 0.263, 0.0);
            break;
        case InfobarType::DANGER: // red; #7A0006/122,0,6; #FFBABA/255,186,186
            rBackgroundColor = basegfx::BColor(1.0, 0.729, 0.729);
            rForegroundColor = basegfx::BColor(0.478, 0.0, 0.024);
            break;
    }

    //remove this?
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    if (rSettings.GetHighContrastMode())
    {
        rBackgroundColor = rSettings.GetLightColor().getBColor();
        rForegroundColor = rSettings.GetDialogTextColor().getBColor();
    }
}
OUString GetInfoBarIconName(InfobarType ibType)
{
    OUString aRet;

    switch (ibType)
    {
        case InfobarType::INFO:
            aRet = "vcl/res/infobox.png";
            break;
        case InfobarType::SUCCESS:
            aRet = "vcl/res/successbox.png";
            break;
        case InfobarType::WARNING:
            aRet = "vcl/res/warningbox.png";
            break;
        case InfobarType::DANGER:
            aRet = "vcl/res/errorbox.png";
            break;
    }

    return aRet;
}

} // anonymous namespace

void SfxInfoBarWindow::SetCloseButtonImage()
{
    Size aSize = Image(StockImage::Yes, CLOSEDOC).GetSizePixel();
    aSize = Size(aSize.Width() * 1.5, aSize.Height() * 1.5);

    VclPtr<VirtualDevice> xDevice(m_xCloseBtn->create_virtual_device());
    xDevice->SetOutputSizePixel(aSize);

    Point aBtnPos(0, 0);

    const ViewInformation2D aNewViewInfos;
    const unique_ptr<BaseProcessor2D> pProcessor(
        createBaseProcessor2DFromOutputDevice(*xDevice, aNewViewInfos));

    const ::tools::Rectangle aRect(aBtnPos, xDevice->PixelToLogic(aSize));

    drawinglayer::primitive2d::Primitive2DContainer aSeq(2);

    //  background
    B2DPolygon aPolygon;
    aPolygon.append(B2DPoint(aRect.Left(), aRect.Top()));
    aPolygon.append(B2DPoint(aRect.Right(), aRect.Top()));
    aPolygon.append(B2DPoint(aRect.Right(), aRect.Bottom()));
    aPolygon.append(B2DPoint(aRect.Left(), aRect.Bottom()));
    aPolygon.setClosed(true);

    PolyPolygonColorPrimitive2D* pBack
        = new PolyPolygonColorPrimitive2D(B2DPolyPolygon(aPolygon), m_aBackgroundColor);
    aSeq[0] = pBack;

    LineAttribute aLineAttribute(m_aForegroundColor, 2.0);

    // Cross
    B2DPolyPolygon aCross;

    B2DPolygon aLine1;
    aLine1.append(B2DPoint(aRect.Left(), aRect.Top()));
    aLine1.append(B2DPoint(aRect.Right(), aRect.Bottom()));
    aCross.append(aLine1);

    B2DPolygon aLine2;
    aLine2.append(B2DPoint(aRect.Right(), aRect.Top()));
    aLine2.append(B2DPoint(aRect.Left(), aRect.Bottom()));
    aCross.append(aLine2);

    PolyPolygonStrokePrimitive2D* pCross
        = new PolyPolygonStrokePrimitive2D(aCross, aLineAttribute, StrokeAttribute());

    aSeq[1] = pCross;

    pProcessor->process(aSeq);

    m_xCloseBtn->set_item_image("close", xDevice);
}

class ExtraButton
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Button> m_xButton;
    /** StatusListener. Updates the button as the slot state changes */
    rtl::Reference<weld::WidgetStatusListener> m_xStatusListener;
    OUString m_aCommand;

    DECL_LINK(CommandHdl, weld::Button&, void);

public:
    ExtraButton(weld::Container* pContainer, const OUString* pCommand)
        : m_xBuilder(Application::CreateBuilder(pContainer, "sfx/ui/extrabutton.ui"))
        , m_xContainer(m_xBuilder->weld_container("ExtraButton"))
        , m_xButton(m_xBuilder->weld_button("button"))
    {
        if (pCommand)
        {
            m_aCommand = *pCommand;
            m_xButton->connect_clicked(LINK(this, ExtraButton, CommandHdl));
            m_xStatusListener.set(new weld::WidgetStatusListener(m_xButton.get(), m_aCommand));
            m_xStatusListener->startListening();
        }
    }

    ~ExtraButton()
    {
        if (m_xStatusListener.is())
            m_xStatusListener->dispose();
    }

    weld::Button& get_widget() { return *m_xButton; }
};

IMPL_LINK_NOARG(ExtraButton, CommandHdl, weld::Button&, void)
{
    comphelper::dispatchCommand(m_aCommand, css::uno::Sequence<css::beans::PropertyValue>());
}

SfxInfoBarWindow::SfxInfoBarWindow(vcl::Window* pParent, const OUString& sId,
                                   const OUString& sPrimaryMessage,
                                   const OUString& sSecondaryMessage, InfobarType ibType,
                                   bool bShowCloseButton)
    : InterimItemWindow(pParent, "sfx/ui/infobar.ui", "InfoBar")
    , m_sId(sId)
    , m_eType(ibType)
    , m_bLayingOut(false)
    , m_xImage(m_xBuilder->weld_image("image"))
    , m_xPrimaryMessage(m_xBuilder->weld_label("primary"))
    , m_xSecondaryMessage(m_xBuilder->weld_text_view("secondary"))
    , m_xButtonBox(m_xBuilder->weld_container("buttonbox"))
    , m_xCloseBtn(m_xBuilder->weld_toolbar("closebar"))
    , m_aActionBtns()
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);

    InitControlBase(m_xCloseBtn.get());

    m_xImage->set_from_icon_name(GetInfoBarIconName(ibType));
    m_xSecondaryMessage->set_margin_top(m_xImage->get_preferred_size().Height() / 4);

    if (!sPrimaryMessage.isEmpty())
    {
        m_xPrimaryMessage->set_label(sPrimaryMessage);
        m_xPrimaryMessage->show();
    }

    m_xSecondaryMessage->set_text(sSecondaryMessage);
    m_aOrigMessageSize = m_xSecondaryMessage->get_preferred_size();
    m_aMessageSize = m_aOrigMessageSize;
    m_xSecondaryMessage->connect_size_allocate(LINK(this, SfxInfoBarWindow, SizeAllocHdl));

    if (bShowCloseButton)
    {
        m_xCloseBtn->connect_clicked(LINK(this, SfxInfoBarWindow, CloseHandler));
        m_xCloseBtn->show();
    }

    EnableChildTransparentMode();

    SetForeAndBackgroundColors(m_eType);

    auto nWidth = pParent->GetSizePixel().getWidth();
    auto nHeight = get_preferred_size().Height();
    SetSizePixel(Size(nWidth, nHeight + 2));

    Resize();
}

IMPL_LINK(SfxInfoBarWindow, SizeAllocHdl, const Size&, rSize, void)
{
    if (m_aMessageSize != rSize)
    {
        m_aMessageSize = rSize;
        static_cast<SfxInfoBarContainerWindow*>(GetParent())->TriggerUpdateLayout();
    }
}

Size SfxInfoBarWindow::DoLayout()
{
    Size aGivenSize(GetSizePixel());

    // disconnect SizeAllocHdl because we don't care about the size change
    // during layout
    m_xSecondaryMessage->connect_size_allocate(Link<const Size&, void>());

    // blow away size cache in case m_aMessageSize.Width() is already the width request
    // and we would get the cached preferred size instead of the recalc we want to force
    m_xSecondaryMessage->set_size_request(-1, -1);
    // make the width we were detected as set to by SizeAllocHdl as our desired width
    m_xSecondaryMessage->set_size_request(m_aMessageSize.Width(), -1);
    // get our preferred size with that message width
    Size aSizeForWidth(aGivenSize.Width(), m_xContainer->get_preferred_size().Height());
    // restore the message preferred size so we can freely resize, and get a new
    // m_aMessageSize and repeat the process if we do
    m_xSecondaryMessage->set_size_request(m_aOrigMessageSize.Width(), -1);

    // connect SizeAllocHdl so changes outside of this layout will trigger a new layout
    m_xSecondaryMessage->connect_size_allocate(LINK(this, SfxInfoBarWindow, SizeAllocHdl));

    return aSizeForWidth;
}

void SfxInfoBarWindow::Layout()
{
    if (m_bLayingOut)
        return;
    m_bLayingOut = true;

    InterimItemWindow::Layout();

    m_bLayingOut = false;
}

weld::Button& SfxInfoBarWindow::addButton(const OUString* pCommand)
{
    m_aActionBtns.emplace_back(std::make_unique<ExtraButton>(m_xButtonBox.get(), pCommand));

    return m_aActionBtns.back()->get_widget();
}

SfxInfoBarWindow::~SfxInfoBarWindow() { disposeOnce(); }

void SfxInfoBarWindow::SetForeAndBackgroundColors(InfobarType eType)
{
    basegfx::BColor aMessageColor;
    GetInfoBarColors(eType, m_aBackgroundColor, m_aForegroundColor, aMessageColor);

    m_xPrimaryMessage->set_font_color(Color(aMessageColor));
    m_xSecondaryMessage->set_font_color(Color(aMessageColor));

    Color aBackgroundColor(m_aBackgroundColor);
    m_xPrimaryMessage->set_background(aBackgroundColor);
    m_xSecondaryMessage->set_background(aBackgroundColor);
    m_xContainer->set_background(aBackgroundColor);
    if (m_xCloseBtn->get_visible())
    {
        m_xCloseBtn->set_background(aBackgroundColor);
        SetCloseButtonImage();
    }
}

void SfxInfoBarWindow::dispose()
{
    for (auto& rxBtn : m_aActionBtns)
        rxBtn.reset();

    m_xImage.reset();
    m_xPrimaryMessage.reset();
    m_xSecondaryMessage.reset();
    m_xButtonBox.reset();
    m_xCloseBtn.reset();
    m_aActionBtns.clear();
    InterimItemWindow::dispose();
}

void SfxInfoBarWindow::Update(const OUString& sPrimaryMessage, const OUString& sSecondaryMessage,
                              InfobarType eType)
{
    if (m_eType != eType)
    {
        m_eType = eType;
        SetForeAndBackgroundColors(m_eType);
        m_xImage->set_from_icon_name(GetInfoBarIconName(eType));
    }

    m_xPrimaryMessage->set_label(sPrimaryMessage);
    m_xSecondaryMessage->set_text(sSecondaryMessage);
    Resize();
    Invalidate();
}

IMPL_LINK_NOARG(SfxInfoBarWindow, CloseHandler, const OString&, void)
{
    static_cast<SfxInfoBarContainerWindow*>(GetParent())->removeInfoBar(this);
}

SfxInfoBarContainerWindow::SfxInfoBarContainerWindow(SfxInfoBarContainerChild* pChildWin)
    : Window(pChildWin->GetParent(), WB_DIALOGCONTROL)
    , m_pChildWin(pChildWin)
    , m_pInfoBars()
    , m_bResizing(false)
{
    m_aLayoutIdle.SetPriority(TaskPriority::HIGHEST);
    m_aLayoutIdle.SetInvokeHandler(LINK(this, SfxInfoBarContainerWindow, DoUpdateLayout));
    m_aLayoutIdle.SetDebugName("SfxInfoBarContainerWindow m_aLayoutIdle");
}

IMPL_LINK_NOARG(SfxInfoBarContainerWindow, DoUpdateLayout, Timer*, void) { m_pChildWin->Update(); }

SfxInfoBarContainerWindow::~SfxInfoBarContainerWindow() { disposeOnce(); }

void SfxInfoBarContainerWindow::dispose()
{
    for (auto& infoBar : m_pInfoBars)
        infoBar.disposeAndClear();
    m_pInfoBars.clear();
    Window::dispose();
}

VclPtr<SfxInfoBarWindow> SfxInfoBarContainerWindow::appendInfoBar(const OUString& sId,
                                                                  const OUString& sPrimaryMessage,
                                                                  const OUString& sSecondaryMessage,
                                                                  InfobarType ibType,
                                                                  bool bShowCloseButton)
{
    if (!isInfobarEnabled(sId))
        return nullptr;

    auto pInfoBar = VclPtr<SfxInfoBarWindow>::Create(this, sId, sPrimaryMessage, sSecondaryMessage,
                                                     ibType, bShowCloseButton);

    basegfx::BColor aBackgroundColor;
    basegfx::BColor aForegroundColor;
    basegfx::BColor aMessageColor;
    GetInfoBarColors(ibType, aBackgroundColor, aForegroundColor, aMessageColor);
    pInfoBar->m_aBackgroundColor = aBackgroundColor;
    pInfoBar->m_aForegroundColor = aForegroundColor;
    m_pInfoBars.push_back(pInfoBar);

    Resize();
    return pInfoBar;
}

VclPtr<SfxInfoBarWindow> SfxInfoBarContainerWindow::getInfoBar(const OUString& sId)
{
    for (auto const& infoBar : m_pInfoBars)
    {
        if (infoBar->getId() == sId)
            return infoBar;
    }
    return nullptr;
}

bool SfxInfoBarContainerWindow::hasInfoBarWithID(const OUString& sId)
{
    return (getInfoBar(sId) != nullptr);
}

void SfxInfoBarContainerWindow::removeInfoBar(VclPtr<SfxInfoBarWindow> const& pInfoBar)
{
    // Remove
    auto it = std::find(m_pInfoBars.begin(), m_pInfoBars.end(), pInfoBar);
    if (it != m_pInfoBars.end())
    {
        it->disposeAndClear();
        m_pInfoBars.erase(it);
    }

    m_pChildWin->Update();
}

bool SfxInfoBarContainerWindow::isInfobarEnabled(const OUString& sId)
{
    if (sId == "readonly")
        return officecfg::Office::UI::Infobar::Enabled::Readonly::get();
    if (sId == "signature")
        return officecfg::Office::UI::Infobar::Enabled::Signature::get();
    if (sId == "donate")
        return officecfg::Office::UI::Infobar::Enabled::Donate::get();
    if (sId == "getinvolved")
        return officecfg::Office::UI::Infobar::Enabled::GetInvolved::get();
    if (sId == "hyphenationmissing")
        return officecfg::Office::UI::Infobar::Enabled::HyphenationMissing::get();
    if (sId == "whatsnew")
        return officecfg::Office::UI::Infobar::Enabled::WhatsNew::get();

    return true;
}

// This triggers the SfxFrame to re-layout its childwindows
void SfxInfoBarContainerWindow::TriggerUpdateLayout() { m_aLayoutIdle.Start(); }

void SfxInfoBarContainerWindow::Resize()
{
    if (m_bResizing)
        return;
    m_bResizing = true;
    const Size& rOrigSize = GetSizePixel();
    auto nOrigWidth = rOrigSize.getWidth();
    auto nOrigHeight = rOrigSize.getHeight();

    long nHeight = 0;

    for (auto& rxInfoBar : m_pInfoBars)
    {
        Size aOrigSize = rxInfoBar->GetSizePixel();
        Size aSize(nOrigWidth, aOrigSize.Height());

        Point aPos(0, nHeight);
        // stage 1: provisionally size the infobar,
        rxInfoBar->SetPosSizePixel(aPos, aSize);

        // stage 2: perhaps allow height to stretch to fit
        // the stage 1 width
        aSize = rxInfoBar->DoLayout();
        rxInfoBar->SetPosSizePixel(aPos, aSize);
        rxInfoBar->Show();

        // Stretch to fit the infobar(s)
        nHeight += aSize.getHeight();
    }

    if (nOrigHeight != nHeight)
    {
        SetSizePixel(Size(nOrigWidth, nHeight));
        TriggerUpdateLayout();
    }

    m_bResizing = false;
}

SFX_IMPL_POS_CHILDWINDOW_WITHID(SfxInfoBarContainerChild, SID_INFOBAR, SFX_OBJECTBAR_OBJECT);

SfxInfoBarContainerChild::SfxInfoBarContainerChild(vcl::Window* _pParent, sal_uInt16 nId,
                                                   SfxBindings* pBindings, SfxChildWinInfo*)
    : SfxChildWindow(_pParent, nId)
    , m_pBindings(pBindings)
{
    SetWindow(VclPtr<SfxInfoBarContainerWindow>::Create(this));
    GetWindow()->SetPosSizePixel(Point(0, 0), Size(_pParent->GetSizePixel().getWidth(), 0));
    GetWindow()->Show();

    SetAlignment(SfxChildAlignment::LOWESTTOP);
}

SfxInfoBarContainerChild::~SfxInfoBarContainerChild() {}

SfxChildWinInfo SfxInfoBarContainerChild::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}

void SfxInfoBarContainerChild::Update()
{
    // Layout to current width, this may change the height
    if (vcl::Window* pChild = GetWindow())
    {
        Size aSize(pChild->GetSizePixel());
        pChild->Resize();
        if (aSize == pChild->GetSizePixel())
            return;
    }

    // Refresh the frame to take the infobars container height change into account
    const sal_uInt16 nId = GetChildWindowId();
    SfxViewFrame* pVFrame = m_pBindings->GetDispatcher()->GetFrame();
    pVFrame->ShowChildWindow(nId);

    // Give the focus to the document view
    pVFrame->GetWindow().GrabFocusToDocument();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
