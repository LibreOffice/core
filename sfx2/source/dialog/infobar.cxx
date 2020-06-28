/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basegfx/polygon/b2dpolygon.hxx>
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
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/decoview.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

using namespace std;
using namespace drawinglayer::geometry;
using namespace drawinglayer::processor2d;
using namespace drawinglayer::primitive2d;
using namespace drawinglayer::attribute;
using namespace basegfx;
using namespace css::frame;

namespace
{
const long INFO_BAR_BASE_HEIGHT = 40;

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
            aRet = "vcl/res/infobox.svg";
            break;
        case InfobarType::SUCCESS:
            aRet = "vcl/res/successbox.svg";
            break;
        case InfobarType::WARNING:
            aRet = "vcl/res/warningbox.svg";
            break;
        case InfobarType::DANGER:
            aRet = "vcl/res/errorbox.svg";
            break;
    }

    return aRet;
}

class SfxCloseButton : public PushButton
{
    basegfx::BColor m_aBackgroundColor;
    basegfx::BColor m_aForegroundColor;

public:
    explicit SfxCloseButton(vcl::Window* pParent)
        : PushButton(pParent, 0)
    {
        basegfx::BColor aMessageColor;
        GetInfoBarColors(InfobarType::WARNING, m_aBackgroundColor, m_aForegroundColor,
                         aMessageColor);
    }

    virtual void Paint(vcl::RenderContext& rRenderContext,
                       const ::tools::Rectangle& rRect) override;

    void setBackgroundColor(const basegfx::BColor& rColor);
    void setForegroundColor(const basegfx::BColor& rColor);
};

void SfxCloseButton::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    Point aBtnPos(0, 0);
    if (GetButtonState() & DrawButtonFlags::Pressed)
        aBtnPos.Move(Size(1, 1));

    const ViewInformation2D aNewViewInfos;
    const unique_ptr<BaseProcessor2D> pProcessor(
        createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));

    const ::tools::Rectangle aRect(aBtnPos, PixelToLogic(GetSizePixel()));

    drawinglayer::primitive2d::Primitive2DContainer aSeq(2);

    //  background
    B2DPolygon aPolygon;
    aPolygon.append(B2DPoint(aRect.Left(), aRect.Top()));
    aPolygon.append(B2DPoint(aRect.Right(), aRect.Top()));
    aPolygon.append(B2DPoint(aRect.Right(), aRect.Bottom()));
    aPolygon.append(B2DPoint(aRect.Left(), aRect.Bottom()));
    aPolygon.setClosed(true);

    Color aBackgroundColor(m_aBackgroundColor);
    if (IsMouseOver() || HasFocus())
        aBackgroundColor.ApplyTintOrShade(-2000);

    PolyPolygonColorPrimitive2D* pBack
        = new PolyPolygonColorPrimitive2D(B2DPolyPolygon(aPolygon), aBackgroundColor.getBColor());
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
}

void SfxCloseButton::setBackgroundColor(const basegfx::BColor& rColor)
{
    m_aBackgroundColor = rColor;
}

void SfxCloseButton::setForegroundColor(const basegfx::BColor& rColor)
{
    m_aForegroundColor = rColor;
}

} // anonymous namespace

SfxInfoBarWindow::SfxInfoBarWindow(vcl::Window* pParent, const OUString& sId,
                                   const OUString& sPrimaryMessage,
                                   const OUString& sSecondaryMessage, InfobarType ibType,
                                   WinBits nMessageStyle, bool bShowCloseButton)
    : Window(pParent, WB_DIALOGCONTROL)
    , m_sId(sId)
    , m_eType(ibType)
    , m_pImage(VclPtr<FixedImage>::Create(this, nMessageStyle))
    , m_pPrimaryMessage(VclPtr<FixedText>::Create(this, nMessageStyle | WB_WORDBREAK))
    , m_pSecondaryMessage(VclPtr<FixedText>::Create(this, nMessageStyle | WB_WORDBREAK))
    , m_pCloseBtn(VclPtr<SfxCloseButton>::Create(this))
    , m_aActionBtns()
{
    m_pCloseBtn->SetStyle(WB_DEFBUTTON | WB_TABSTOP);
    SetForeAndBackgroundColors(m_eType);
    float fScaleFactor = GetDPIScaleFactor();
    long nWidth = pParent->GetSizePixel().getWidth();
    SetPosSizePixel(Point(0, 0), Size(nWidth, INFO_BAR_BASE_HEIGHT * fScaleFactor));

    m_pImage->SetImage(Image(StockImage::Yes, GetInfoBarIconName(ibType)));
    m_pImage->SetPaintTransparent(true);
    m_pImage->Show();

    vcl::Font aFont(m_pPrimaryMessage->GetControlFont());
    aFont.SetWeight(WEIGHT_BOLD);
    m_pPrimaryMessage->SetControlFont(aFont);
    if (!sPrimaryMessage.isEmpty())
    {
        m_pPrimaryMessage->SetText(sPrimaryMessage);
        m_pPrimaryMessage->Show();
    }

    m_pSecondaryMessage->SetText(sSecondaryMessage);
    m_pSecondaryMessage->Show();

    if (bShowCloseButton)
    {
        m_pCloseBtn->SetClickHdl(LINK(this, SfxInfoBarWindow, CloseHandler));
        m_pCloseBtn->Show();
    }

    EnableChildTransparentMode();

    Resize();
}

void SfxInfoBarWindow::addButton(PushButton* pButton)
{
    pButton->SetParent(this);
    pButton->Show();
    m_aActionBtns.emplace_back(pButton);
    Resize();
}

SfxInfoBarWindow::~SfxInfoBarWindow() { disposeOnce(); }

void SfxInfoBarWindow::SetForeAndBackgroundColors(InfobarType eType)
{
    basegfx::BColor aMessageColor;
    GetInfoBarColors(eType, m_aBackgroundColor, m_aForegroundColor, aMessageColor);

    static_cast<SfxCloseButton*>(m_pCloseBtn.get())->setBackgroundColor(m_aBackgroundColor);
    static_cast<SfxCloseButton*>(m_pCloseBtn.get())->setForegroundColor(m_aForegroundColor);
    m_pPrimaryMessage->SetControlForeground(Color(aMessageColor));
    m_pSecondaryMessage->SetControlForeground(Color(aMessageColor));
}

void SfxInfoBarWindow::dispose()
{
    for (auto& rxBtn : m_aActionBtns)
        rxBtn.disposeAndClear();

    m_pImage.disposeAndClear();
    m_pPrimaryMessage.disposeAndClear();
    m_pSecondaryMessage.disposeAndClear();
    m_pCloseBtn.disposeAndClear();
    m_aActionBtns.clear();
    vcl::Window::dispose();
}

void SfxInfoBarWindow::Paint(vcl::RenderContext& rRenderContext,
                             const ::tools::Rectangle& rPaintRect)
{
    const ViewInformation2D aNewViewInfos;
    const unique_ptr<BaseProcessor2D> pProcessor(
        createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));

    const ::tools::Rectangle aRect(Point(0, 0), PixelToLogic(GetSizePixel()));

    drawinglayer::primitive2d::Primitive2DContainer aSeq(2);

    // Light background
    B2DPolygon aPolygon;
    aPolygon.append(B2DPoint(aRect.Left(), aRect.Top()));
    aPolygon.append(B2DPoint(aRect.Right(), aRect.Top()));
    aPolygon.append(B2DPoint(aRect.Right(), aRect.Bottom()));
    aPolygon.append(B2DPoint(aRect.Left(), aRect.Bottom()));
    aPolygon.setClosed(true);

    PolyPolygonColorPrimitive2D* pBack
        = new PolyPolygonColorPrimitive2D(B2DPolyPolygon(aPolygon), m_aBackgroundColor);
    aSeq[0] = pBack;

    LineAttribute aLineAttribute(m_aForegroundColor, 1.0);

    // Bottom dark line
    B2DPolygon aPolygonBottom;
    aPolygonBottom.append(B2DPoint(aRect.Left(), aRect.Bottom()));
    aPolygonBottom.append(B2DPoint(aRect.Right(), aRect.Bottom()));

    PolygonStrokePrimitive2D* pLineBottom
        = new PolygonStrokePrimitive2D(aPolygonBottom, aLineAttribute);

    aSeq[1] = pLineBottom;

    pProcessor->process(aSeq);

    Window::Paint(rRenderContext, rPaintRect);
}

void SfxInfoBarWindow::Resize()
{
    float fScaleFactor = GetDPIScaleFactor();

    long nWidth = GetSizePixel().getWidth();
    m_pCloseBtn->SetPosSizePixel(Point(nWidth - 25 * fScaleFactor, 15 * fScaleFactor),
                                 Size(10 * fScaleFactor, 10 * fScaleFactor));

    // Reparent the buttons and place them on the right of the bar
    long nX = m_pCloseBtn->GetPosPixel().getX() - 15 * fScaleFactor;
    long nButtonGap = 5 * fScaleFactor;

    for (auto const& actionBtn : m_aActionBtns)
    {
        long nButtonWidth = actionBtn->GetSizePixel().getWidth();
        nX -= nButtonWidth;
        actionBtn->SetPosSizePixel(Point(nX, 5 * fScaleFactor),
                                   Size(nButtonWidth, 30 * fScaleFactor));
        nX -= nButtonGap;
    }

    Point aPrimaryMessagePosition(32 * fScaleFactor + 10 * fScaleFactor, 10 * fScaleFactor);
    Point aSecondaryMessagePosition(aPrimaryMessagePosition);
    Size aMessageSize(nX - 35 * fScaleFactor, 20 * fScaleFactor);
    Size aPrimaryTextSize = m_pPrimaryMessage->CalcMinimumSize(aMessageSize.getWidth());
    Size aSecondaryTextSize = m_pSecondaryMessage->CalcMinimumSize(aMessageSize.getWidth()
                                                                   - aPrimaryTextSize.getWidth());
    if (!m_pPrimaryMessage->GetText().isEmpty())
        aSecondaryMessagePosition.AdjustX(aPrimaryTextSize.getWidth() + 6 * fScaleFactor);

    long aMinimumHeight = std::max(m_pPrimaryMessage->CalcMinimumSize().getHeight(),
                                   m_pSecondaryMessage->CalcMinimumSize().getHeight());

    long aExtraHeight = aSecondaryTextSize.getHeight() - aMinimumHeight;

    // The message won't be legible and the window will get too high
    if (aMessageSize.getWidth() < 30)
    {
        aExtraHeight = 0;
    }

    m_pPrimaryMessage->SetPosSizePixel(aPrimaryMessagePosition, aPrimaryTextSize);
    m_pSecondaryMessage->SetPosSizePixel(aSecondaryMessagePosition, aSecondaryTextSize);
    m_pImage->SetPosSizePixel(Point(4, 4), Size(32 * fScaleFactor, 32 * fScaleFactor));

    SetPosSizePixel(GetPosPixel(), Size(nWidth, INFO_BAR_BASE_HEIGHT * fScaleFactor
                                                    + aExtraHeight * fScaleFactor));
}

void SfxInfoBarWindow::Update(const OUString& sPrimaryMessage, const OUString& sSecondaryMessage,
                              InfobarType eType)
{
    if (m_eType != eType)
    {
        m_eType = eType;
        SetForeAndBackgroundColors(m_eType);
        m_pImage->SetImage(Image(StockImage::Yes, GetInfoBarIconName(eType)));
    }

    m_pPrimaryMessage->SetText(sPrimaryMessage);
    m_pSecondaryMessage->SetText(sSecondaryMessage);
    Resize();
    Invalidate();
}

IMPL_LINK_NOARG(SfxInfoBarWindow, CloseHandler, Button*, void)
{
    static_cast<SfxInfoBarContainerWindow*>(GetParent())->removeInfoBar(this);
}

SfxInfoBarContainerWindow::SfxInfoBarContainerWindow(SfxInfoBarContainerChild* pChildWin)
    : Window(pChildWin->GetParent(), WB_DIALOGCONTROL)
    , m_pChildWin(pChildWin)
    , m_pInfoBars()
{
}

SfxInfoBarContainerWindow::~SfxInfoBarContainerWindow() { disposeOnce(); }

void SfxInfoBarContainerWindow::dispose()
{
    for (auto& infoBar : m_pInfoBars)
        infoBar.disposeAndClear();
    m_pInfoBars.clear();
    Window::dispose();
}

VclPtr<SfxInfoBarWindow>
SfxInfoBarContainerWindow::appendInfoBar(const OUString& sId, const OUString& sPrimaryMessage,
                                         const OUString& sSecondaryMessage, InfobarType ibType,
                                         WinBits nMessageStyle, bool bShowCloseButton)
{
    if (!isInfobarEnabled(sId))
        return nullptr;

    auto pInfoBar = VclPtr<SfxInfoBarWindow>::Create(this, sId, sPrimaryMessage, sSecondaryMessage,
                                                     ibType, nMessageStyle, bShowCloseButton);

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

    Resize();

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

    return true;
}

void SfxInfoBarContainerWindow::Resize()
{
    long nWidth = GetSizePixel().getWidth();
    long nHeight = 0;

    for (auto& rxInfoBar : m_pInfoBars)
    {
        Size aSize = rxInfoBar->GetSizePixel();
        aSize.setWidth(nWidth);
        Point aPos(0, nHeight);
        rxInfoBar->SetPosSizePixel(aPos, aSize);
        rxInfoBar->Resize();
        rxInfoBar->Show();

        // Stretch to fit the infobar(s)
        nHeight += aSize.getHeight();
    }

    SetSizePixel(Size(nWidth, nHeight));
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
    // Refresh the frame to take the infobars container height change into account
    const sal_uInt16 nId = GetChildWindowId();
    SfxViewFrame* pVFrame = m_pBindings->GetDispatcher()->GetFrame();
    pVFrame->ShowChildWindow(nId);

    // Give the focus to the document view
    pVFrame->GetWindow().GrabFocusToDocument();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
