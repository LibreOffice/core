/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/infobar.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace std;
using namespace drawinglayer::geometry;
using namespace drawinglayer::processor2d;
using namespace drawinglayer::primitive2d;
using namespace drawinglayer::attribute;
using namespace basegfx;

namespace
{

const long INFO_BAR_BASE_HEIGHT = 40;

void GetInfoBarColors(InfoBarType ibType, BColor&  rBackgroundColor, BColor& rForegroundColor, BColor& rMessageColor)
{
    rMessageColor = basegfx::BColor(0.0, 0.0, 0.0);

    switch (ibType)
    {
    case InfoBarType::Info: // blue; #004785/0,71,133; #BDE5F8/189,229,248
        rBackgroundColor = basegfx::BColor(0.741, 0.898, 0.973);
        rForegroundColor = basegfx::BColor(0.0, 0.278, 0.522);
        break;
    case InfoBarType::Success: // green; #32550C/50,85,12; #DFF2BF/223,242,191
        rBackgroundColor = basegfx::BColor(0.874,0.949,0.749);
        rForegroundColor = basegfx::BColor(0.196,0.333,0.047);
        break;
    case InfoBarType::Warning: // orange; #704300/112,67,0; #FEEFB3/254,239,179
        rBackgroundColor = basegfx::BColor(0.996,0.937,0.702);
        rForegroundColor = basegfx::BColor(0.439,0.263,0.0);
        break;
    case InfoBarType::Danger: // red; #7A0006/122,0,6; #FFBABA/255,186,186
        rBackgroundColor = basegfx::BColor(1.0,0.729,0.729);
        rForegroundColor = basegfx::BColor(0.478,0.0,0.024);
        break;
    }//switch

    //remove this?
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    if (rSettings.GetHighContrastMode())
    {
        rBackgroundColor = rSettings.GetLightColor().getBColor();
        rForegroundColor = rSettings.GetDialogTextColor().getBColor();
    }

}
OUString GetInfoBarIconName(InfoBarType ibType)
{

    OUString aRet;

    switch (ibType)
    {
    case InfoBarType::Info:
       aRet = "vcl/res/infobox.svg";
       break;
    case InfoBarType::Success:
        aRet = "cmd/lc_apply.svg";
        break;
    case InfoBarType::Warning:
        aRet = "vcl/res/warningbox.svg";
        break;
    case InfoBarType::Danger:
        aRet = "vcl/res/errorbox.svg";
        break;
    }//switch

    return aRet;
}

class SfxCloseButton : public PushButton
{
    basegfx::BColor m_aBackgroundColor;
    basegfx::BColor m_aForegroundColor;
    basegfx::BColor m_aMessageColor;

public:
    explicit SfxCloseButton(vcl::Window* pParent) : PushButton(pParent, 0)
    {
        GetInfoBarColors(InfoBarType::Warning,m_aBackgroundColor,m_aForegroundColor,m_aMessageColor);
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect) override;

    void setBackgroundColor(const basegfx::BColor& rColor);
    void setForegroundColor(const basegfx::BColor& rColor);
};

void SfxCloseButton::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    const ViewInformation2D aNewViewInfos;
    const unique_ptr<BaseProcessor2D> pProcessor(
                createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));

    const ::tools::Rectangle aRect(Point(0, 0), PixelToLogic(GetSizePixel()));

    drawinglayer::primitive2d::Primitive2DContainer aSeq(2);

    //  background
    B2DPolygon aPolygon;
    aPolygon.append(B2DPoint(aRect.Left(), aRect.Top()));
    aPolygon.append(B2DPoint(aRect.Right(), aRect.Top()));
    aPolygon.append(B2DPoint(aRect.Right(), aRect.Bottom()));
    aPolygon.append(B2DPoint(aRect.Left(), aRect.Bottom()));
    aPolygon.setClosed(true);

    PolyPolygonColorPrimitive2D* pBack =
        new PolyPolygonColorPrimitive2D(B2DPolyPolygon(aPolygon), m_aBackgroundColor);
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

    PolyPolygonStrokePrimitive2D* pCross =
        new PolyPolygonStrokePrimitive2D(aCross, aLineAttribute, StrokeAttribute());

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
       const OUString& sMessage,
       InfoBarType ibType,
       WinBits nMessageStyle = WB_LEFT|WB_VCENTER) :
    Window(pParent, 0),
    m_sId(sId),
    m_pImage(VclPtr<FixedImage>::Create(this, nMessageStyle)),
    m_pMessage(VclPtr<FixedText>::Create(this, nMessageStyle)),
    m_pCloseBtn(VclPtr<SfxCloseButton>::Create(this)),
    m_aActionBtns()
{
    basegfx::BColor aBackgroundColor;
    basegfx::BColor aForegroundColor;
    basegfx::BColor aMessageColor;
    GetInfoBarColors(ibType,aBackgroundColor,aForegroundColor,aMessageColor);
    static_cast<SfxCloseButton*>(m_pCloseBtn.get())->setBackgroundColor(aBackgroundColor);
    static_cast<SfxCloseButton*>(m_pCloseBtn.get())->setForegroundColor(aForegroundColor);
    m_pMessage->SetControlForeground(Color(aMessageColor));

    float fScaleFactor = GetDPIScaleFactor();
    long nWidth = pParent->GetSizePixel().getWidth();
    SetPosSizePixel(Point(0, 0), Size(nWidth, INFO_BAR_BASE_HEIGHT * fScaleFactor));

    m_pImage->SetImage(Image(BitmapEx(GetInfoBarIconName(ibType))));
    m_pImage->SetPaintTransparent(true);
    m_pImage->Show();

    m_pMessage->SetText(sMessage);
    m_pMessage->Show();

    m_pCloseBtn->SetClickHdl(LINK(this, SfxInfoBarWindow, CloseHandler));
    m_pCloseBtn->Show();

    EnableChildTransparentMode();

    Resize();
}

void SfxInfoBarWindow::addButton(PushButton* pButton) {
    pButton->SetParent(this);
    pButton->Show();
    m_aActionBtns.emplace_back(pButton);
    Resize();
}

SfxInfoBarWindow::~SfxInfoBarWindow()
{
    disposeOnce();
}

void SfxInfoBarWindow::dispose()
{
    for ( auto it = m_aActionBtns.begin( ); it != m_aActionBtns.end( ); ++it )
        it->disposeAndClear();

    m_pImage.disposeAndClear();
    m_pMessage.disposeAndClear();
    m_pCloseBtn.disposeAndClear();
    m_aActionBtns.clear( );
    vcl::Window::dispose();
}

void SfxInfoBarWindow::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rPaintRect)
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

    PolyPolygonColorPrimitive2D* pBack =
        new PolyPolygonColorPrimitive2D(B2DPolyPolygon(aPolygon), m_aBackgroundColor);
    aSeq[0] = pBack;

    LineAttribute aLineAttribute(m_aForegroundColor, 1.0);

    // Bottom dark line
    B2DPolygon aPolygonBottom;
    aPolygonBottom.append(B2DPoint(aRect.Left(), aRect.Bottom()));
    aPolygonBottom.append(B2DPoint(aRect.Right(), aRect.Bottom()));

    PolygonStrokePrimitive2D* pLineBottom =
            new PolygonStrokePrimitive2D (aPolygonBottom, aLineAttribute);

    aSeq[1] = pLineBottom;

    pProcessor->process(aSeq);

    Window::Paint(rRenderContext, rPaintRect);
}

void SfxInfoBarWindow::Resize()
{
    float fScaleFactor = GetDPIScaleFactor();

    long nWidth = GetSizePixel().getWidth();
    m_pCloseBtn->SetPosSizePixel(Point(nWidth - 25 * fScaleFactor, 15 * fScaleFactor), Size(10 * fScaleFactor, 10 * fScaleFactor));

    // Reparent the buttons and place them on the right of the bar
    long nX = m_pCloseBtn->GetPosPixel().getX() - 15 * fScaleFactor;
    long nButtonGap = 5 * fScaleFactor;

    for (auto const& actionBtn : m_aActionBtns)
    {
        long nButtonWidth = actionBtn->GetSizePixel().getWidth();
        nX -= nButtonWidth;
        actionBtn->SetPosSizePixel(Point(nX, 5 * fScaleFactor), Size(nButtonWidth, 30 * fScaleFactor));
        nX -= nButtonGap;
    }

    m_pImage->SetPosSizePixel(Point(4,4), Size(32* fScaleFactor, 32* fScaleFactor));

    Point aMessagePosition(32 * fScaleFactor + 10 * fScaleFactor, 10 * fScaleFactor);
    Size aMessageSize(nX - 20 * fScaleFactor, 20 * fScaleFactor);

    m_pMessage->SetPosSizePixel(aMessagePosition, aMessageSize);

}

IMPL_LINK_NOARG(SfxInfoBarWindow, CloseHandler, Button*, void)
{
    static_cast<SfxInfoBarContainerWindow*>(GetParent())->removeInfoBar(this);
}

SfxInfoBarContainerWindow::SfxInfoBarContainerWindow(SfxInfoBarContainerChild* pChildWin ) :
    Window(pChildWin->GetParent(), 0),
    m_pChildWin(pChildWin),
    m_pInfoBars()
{
}

SfxInfoBarContainerWindow::~SfxInfoBarContainerWindow()
{
    disposeOnce();
}

void SfxInfoBarContainerWindow::dispose()
{
    for (auto & infoBar : m_pInfoBars)
        infoBar.disposeAndClear();
    m_pInfoBars.clear( );
    Window::dispose();
}

VclPtr<SfxInfoBarWindow> SfxInfoBarContainerWindow::appendInfoBar(const OUString& sId,
                                                           const OUString& sMessage,
                                                           InfoBarType ibType,
                                                           WinBits nMessageStyle)
{
    Size aSize = GetSizePixel();

    VclPtrInstance<SfxInfoBarWindow> pInfoBar(this, sId, sMessage, ibType, nMessageStyle);

    basegfx::BColor aBackgroundColor;
    basegfx::BColor aForegroundColor;
    basegfx::BColor aMessageColor;
    GetInfoBarColors(ibType,aBackgroundColor,aForegroundColor,aMessageColor);
    pInfoBar->m_aBackgroundColor = aBackgroundColor;
    pInfoBar->m_aForegroundColor = aForegroundColor;

    pInfoBar->SetPosPixel(Point(0, aSize.getHeight()));
    pInfoBar->Show();
    m_pInfoBars.push_back(pInfoBar);

    long nHeight = pInfoBar->GetSizePixel().getHeight();
    aSize.setHeight(aSize.getHeight() + nHeight);
    SetSizePixel(aSize);
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

bool SfxInfoBarContainerWindow::hasInfoBarWithID( const OUString &sId )
{
    return ( getInfoBar( sId ) != nullptr );
}

void SfxInfoBarContainerWindow::removeInfoBar(VclPtr<SfxInfoBarWindow> const & pInfoBar)
{
    // Remove
    for (auto it = m_pInfoBars.begin(); it != m_pInfoBars.end(); ++it)
    {
        if (pInfoBar == *it)
        {
            it->disposeAndClear();
            m_pInfoBars.erase(it);
            break;
        }
    }

    // Resize
    long nY = 0;
    for (auto const& infoBar : m_pInfoBars)
    {
        infoBar->SetPosPixel(Point(0, nY));
        nY += infoBar->GetSizePixel().getHeight();
    }

    Size aSize = GetSizePixel();
    aSize.setHeight(nY);
    SetSizePixel(aSize);

    m_pChildWin->Update();
}

void SfxInfoBarContainerWindow::Resize()
{
    // Only need to change the width of the infobars
    long nWidth = GetSizePixel().getWidth();

    for (auto it = m_pInfoBars.begin(); it != m_pInfoBars.end(); ++it)
    {
        Size aSize = (*it)->GetSizePixel();
        aSize.setWidth(nWidth);
        (*it)->SetSizePixel(aSize);
        (*it)->Resize();
    }
}

SFX_IMPL_POS_CHILDWINDOW_WITHID(SfxInfoBarContainerChild, SID_INFOBAR, SFX_OBJECTBAR_OBJECT);

SfxInfoBarContainerChild::SfxInfoBarContainerChild( vcl::Window* _pParent, sal_uInt16 nId, SfxBindings* pBindings, SfxChildWinInfo* ) :
    SfxChildWindow(_pParent, nId),
    m_pBindings(pBindings)
{
    SetWindow( VclPtr<SfxInfoBarContainerWindow>::Create(this) );
    GetWindow()->SetPosSizePixel(Point(0, 0), Size(_pParent->GetSizePixel().getWidth(), 0));
    GetWindow()->Show();

    SetAlignment(SfxChildAlignment::LOWESTTOP);
}

SfxInfoBarContainerChild::~SfxInfoBarContainerChild()
{
}

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
