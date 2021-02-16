/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <StartMarker.hxx>
#include <UITools.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <bitmaps.hlst>
#include <ColorChanger.hxx>
#include <ReportDefines.hxx>
#include <SectionWindow.hxx>
#include <helpids.h>
#include <vcl/event.hxx>
#include <vcl/help.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/settings.hxx>

#include <unotools/syslocale.hxx>
#include <unotools/localedatawrapper.hxx>

#define CORNER_SPACE     5


namespace rptui
{


Image*  OStartMarker::s_pDefCollapsed       = nullptr;
Image*  OStartMarker::s_pDefExpanded        = nullptr;
oslInterlockedCount OStartMarker::s_nImageRefCount  = 0;


OStartMarker::OStartMarker(OSectionWindow* _pParent,const OUString& _sColorEntry)
    : OColorListener(_pParent,_sColorEntry)
    , m_aVRuler(VclPtr<Ruler>::Create(this,WB_VERT))
    , m_pParent(_pParent)
    , m_bShowRuler(true)
{
    osl_atomic_increment(&s_nImageRefCount);
    initDefaultNodeImages();
    ImplInitSettings();
    m_aVRuler->Show();
    m_aVRuler->Activate();
    m_aVRuler->SetPagePos();
    m_aVRuler->SetBorders();
    m_aVRuler->SetIndents();
    m_aVRuler->SetMargin1();
    m_aVRuler->SetMargin2();
    const MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    m_aVRuler->SetUnit(MeasurementSystem::Metric == eSystem ? FieldUnit::CM : FieldUnit::INCH);
    EnableChildTransparentMode();
    SetParentClipMode( ParentClipMode::NoClip );
    SetPaintTransparent( true );
}

OStartMarker::~OStartMarker()
{
    disposeOnce();
}

void OStartMarker::dispose()
{
    if ( osl_atomic_decrement(&s_nImageRefCount) == 0 )
    {
        delete s_pDefCollapsed;
        s_pDefCollapsed = nullptr;
        delete s_pDefExpanded;
        s_pDefExpanded = nullptr;
    }
    m_aVRuler.disposeAndClear();
    m_pParent.clear();
    OColorListener::dispose();
}

sal_Int32 OStartMarker::getMinHeight() const
{
    Fraction aExtraWidth(tools::Long(2 * REPORT_EXTRA_SPACE));
    aExtraWidth *= GetMapMode().GetScaleX();
    return LogicToPixel(Size(0, GetTextHeight())).Height() + tools::Long(aExtraWidth);
}

void OStartMarker::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    rRenderContext.Push(PushFlags::TEXTCOLOR);

    Size aSize(GetOutputSizePixel());
    const tools::Long nCornerWidth = tools::Long(CORNER_SPACE * double(GetMapMode().GetScaleX()));

    if (isCollapsed())
    {
        rRenderContext.SetClipRegion();
    }
    else
    {
        const tools::Long nVRulerWidth = m_aVRuler->GetSizePixel().Width();
        tools::Long nSize = aSize.Width() - nVRulerWidth;
        aSize.AdjustWidth(nCornerWidth );
        rRenderContext.SetClipRegion(vcl::Region(rRenderContext.PixelToLogic(tools::Rectangle(Point(),
                                                                             Size(nSize, aSize.Height())))));
    }

    tools::Rectangle aWholeRect(Point(), aSize);
    {
        const ColorChanger aColors(&rRenderContext, m_nTextBoundaries, m_nColor);
        tools::PolyPolygon aPoly;
        aPoly.Insert( tools::Polygon(aWholeRect,nCornerWidth,nCornerWidth));

        Color aStartColor(m_nColor);
        aStartColor.IncreaseLuminance(10);
        sal_uInt16 nHue = 0;
        sal_uInt16 nSat = 0;
        sal_uInt16 nBri = 0;
        aStartColor.RGBtoHSB(nHue, nSat, nBri);
        nSat += 40;
        Color aEndColor(Color::HSBtoRGB(nHue, nSat, nBri));
        Gradient aGradient(GradientStyle::Linear,aStartColor,aEndColor);
        aGradient.SetSteps(static_cast<sal_uInt16>(aSize.Height()));

        rRenderContext.DrawGradient(PixelToLogic(aPoly) ,aGradient);
    }

    rRenderContext.Push(PushFlags::MAPMODE);
    rRenderContext.SetMapMode();

    rRenderContext.DrawImage(m_aImageRect.TopLeft(), m_aImageRect.GetSize(), m_aImage);

    const Color aColor(m_nColor);
    Color aTextColor = GetTextColor();
    if (aColor.GetLuminance() < 128)
        aTextColor = COL_WHITE;
    rRenderContext.SetTextColor(aTextColor);

    rRenderContext.DrawText(m_aTextRect, m_aText, DrawTextFlags::MultiLine | DrawTextFlags::WordBreakHyphenation);

    rRenderContext.Pop();

    if (m_bMarked)
    {
        const tools::Long nCornerHeight = tools::Long(CORNER_SPACE * double(GetMapMode().GetScaleY()));
        tools::Rectangle aRect(Point(nCornerWidth, nCornerHeight),
                        Size(aSize.Width() - nCornerWidth - nCornerWidth,
                             aSize.Height() - nCornerHeight - nCornerHeight));
        ColorChanger aColors(&rRenderContext, COL_WHITE, COL_WHITE);
        rRenderContext.DrawPolyLine( tools::Polygon(rRenderContext.PixelToLogic(aRect)),
                                    LineInfo(LineStyle::Solid, 2));
    }

    rRenderContext.Pop();
}

void OStartMarker::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() )
        return;

    Point aPos( rMEvt.GetPosPixel());

    const Size aOutputSize = GetOutputSizePixel();
    if( aPos.X() > aOutputSize.Width() || aPos.Y() > aOutputSize.Height() )
        return;
    if ( rMEvt.GetClicks() == 2 || m_aImageRect.IsInside( aPos ) )
    {
        m_bCollapsed = !m_bCollapsed;

        changeImage();

        m_aVRuler->Show(!m_bCollapsed && m_bShowRuler);
        m_aCollapsedLink.Call(*this);
    }

    m_pParent->showProperties();
}

void OStartMarker::changeImage()
{
    m_aImage = m_bCollapsed ? *s_pDefCollapsed : *s_pDefExpanded;
}

void OStartMarker::initDefaultNodeImages()
{
    if ( !s_pDefCollapsed )
    {
        s_pDefCollapsed = new Image(StockImage::Yes, RID_BMP_TREENODE_COLLAPSED);
        s_pDefExpanded = new Image(StockImage::Yes, RID_BMP_TREENODE_EXPANDED);
    }

    m_aImage = m_bCollapsed ? *s_pDefCollapsed : *s_pDefExpanded;
}

void OStartMarker::ApplySettings(vcl::RenderContext& rRenderContext)
{
    rRenderContext.SetBackground();
    rRenderContext.SetFillColor(Application::GetSettings().GetStyleSettings().GetDialogColor());
}

void OStartMarker::ImplInitSettings()
{
    ApplySettings(*this);
}

void OStartMarker::Resize()
{
    const Size aOutputSize( GetOutputSizePixel() );
    const tools::Long nOutputWidth  = aOutputSize.Width();
    const tools::Long nOutputHeight = aOutputSize.Height();

    const tools::Long nVRulerWidth = m_aVRuler->GetSizePixel().Width();
    const Point aRulerPos(nOutputWidth - nVRulerWidth,0);
    m_aVRuler->SetPosSizePixel(aRulerPos,Size(nVRulerWidth,nOutputHeight));

    Size aImageSize = m_aImage.GetSizePixel();
    const MapMode& rMapMode = GetMapMode();
    aImageSize.setWidth( tools::Long(aImageSize.Width() * static_cast<double>(rMapMode.GetScaleX())) );
    aImageSize.setHeight( tools::Long(aImageSize.Height() * static_cast<double>(rMapMode.GetScaleY())) );

    tools::Long nExtraWidth = tools::Long(REPORT_EXTRA_SPACE * rMapMode.GetScaleX());

    Point aPos(aImageSize.Width() + (nExtraWidth * 2), nExtraWidth);
    const tools::Long nHeight = ::std::max<sal_Int32>(nOutputHeight - 2*aPos.Y(),LogicToPixel(Size(0, GetTextHeight())).Height());
    m_aTextRect = tools::Rectangle(aPos, Size(aRulerPos.X() - aPos.X(),nHeight));

    aPos.setX( nExtraWidth );
    aPos.AdjustY(static_cast<sal_Int32>((LogicToPixel(Size(0, GetTextHeight())).Height() - aImageSize.Height()) * 0.5) ) ;
    m_aImageRect = tools::Rectangle(aPos, aImageSize);

    OColorListener::Resize();
}

void OStartMarker::setTitle(const OUString& rTitle)
{
    if (m_aText != rTitle)
    {
        m_aText = rTitle;
        Invalidate();
    }
}

void OStartMarker::Notify(SfxBroadcaster & rBc, SfxHint const & rHint)
{
    OColorListener::Notify(rBc, rHint);
    if (rHint.GetId() == SfxHintId::ColorsChanged)
    {
        Invalidate(InvalidateFlags::Children);
    }
}

void OStartMarker::showRuler(bool _bShow)
{
    m_bShowRuler = _bShow;
    m_aVRuler->Show(!m_bCollapsed && m_bShowRuler);
}

void OStartMarker::RequestHelp( const HelpEvent& rHEvt )
{
    if (m_aText.isEmpty())
        return;

    // show help
    tools::Rectangle aItemRect(rHEvt.GetMousePosPixel(),Size(GetSizePixel().Width(),getMinHeight()));
    Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
    aItemRect.SetLeft( aPt.X() );
    aItemRect.SetTop( aPt.Y() );
    aPt = OutputToScreenPixel( aItemRect.BottomRight() );
    aItemRect.SetRight( aPt.X() );
    aItemRect.SetBottom( aPt.Y() );
    if( rHEvt.GetMode() == HelpEventMode::BALLOON )
        Help::ShowBalloon( this, aItemRect.Center(), aItemRect, m_aText);
    else
        Help::ShowQuickHelp( this, aItemRect, m_aText );
}

void OStartMarker::setCollapsed(bool _bCollapsed)
{
    OColorListener::setCollapsed(_bCollapsed);
    showRuler(_bCollapsed);
    changeImage();
}

void OStartMarker::zoom(const Fraction& _aZoom)
{
    setZoomFactor(_aZoom, *this);
    m_aVRuler->SetZoom(_aZoom);
    Resize();
    Invalidate();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
