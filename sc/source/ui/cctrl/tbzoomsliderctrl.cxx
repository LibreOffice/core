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
#include <tbzoomsliderctrl.hxx>
#include <i18nutil/unicode.hxx>
#include <vcl/svapp.hxx>

#include <comphelper/propertyvalue.hxx>
#include <utility>
#include <vcl/InterimItemWindow.hxx>
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>
#include <svx/zoomslideritem.hxx>
#include <iterator>
#include <set>
#include <bitmaps.hlst>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

// class ScZoomSliderControl ---------------------------------------

SFX_IMPL_TOOLBOX_CONTROL( ScZoomSliderControl, SvxZoomSliderItem );

ScZoomSliderControl::ScZoomSliderControl(
    sal_uInt16     nSlotId,
    ToolBoxItemId  nId,
    ToolBox&   rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.Invalidate();
}

ScZoomSliderControl::~ScZoomSliderControl()
{

}

void ScZoomSliderControl::StateChangedAtToolBoxControl( sal_uInt16 /*nSID*/, SfxItemState eState,
                                       const SfxPoolItem* pState )
{
    ToolBoxItemId           nId  = GetId();
    ToolBox&                rTbx = GetToolBox();
    ScZoomSliderWnd*        pBox = static_cast<ScZoomSliderWnd*>(rTbx.GetItemWindow( nId ));
    OSL_ENSURE( pBox ,"Control not found!" );

    if (SfxItemState::DEFAULT != eState || SfxItemState::DISABLED == eState)
    {
        SvxZoomSliderItem aZoomSliderItem( 100 );
        pBox->Disable();
        pBox->UpdateFromItem( &aZoomSliderItem );
    }
    else
    {
        pBox->Enable();
        OSL_ENSURE( dynamic_cast<const SvxZoomSliderItem*>( pState) !=  nullptr, "invalid item type" );
        const SvxZoomSliderItem* pZoomSliderItem = dynamic_cast< const SvxZoomSliderItem* >( pState );

        OSL_ENSURE( pZoomSliderItem, "Sc::ScZoomSliderControl::StateChanged(), wrong item type!" );
        if( pZoomSliderItem )
            pBox->UpdateFromItem( pZoomSliderItem );
    }
}

VclPtr<InterimItemWindow> ScZoomSliderControl::CreateItemWindow( vcl::Window *pParent )
{
    // #i98000# Don't try to get a value via SfxViewFrame::Current here.
    // The view's value is always notified via StateChanged later.
    VclPtrInstance<ScZoomSliderWnd> xSlider( pParent,
        css::uno::Reference< css::frame::XDispatchProvider >( m_xFrame->getController(),
        css::uno::UNO_QUERY ), 100 );
    return xSlider;
}

constexpr sal_uInt16 gnSliderCenter(100);

const tools::Long nButtonWidth     = 10;
const tools::Long nButtonHeight    = 10;
const tools::Long nIncDecWidth     = 11;
const tools::Long nIncDecHeight    = 11;
const tools::Long nSliderHeight    = 2;
const tools::Long nSliderWidth     = 4;
const tools::Long nSnappingHeight  = 4;
const tools::Long nSliderXOffset   = 20;
const tools::Long nSnappingEpsilon = 5; // snapping epsilon in pixels
const tools::Long nSnappingPointsMinDist = nSnappingEpsilon; // minimum distance of two adjacent snapping points

sal_uInt16 ScZoomSlider::Offset2Zoom( tools::Long nOffset ) const
{
    const tools::Long nControlWidth = GetSliderLength();
    sal_uInt16 nRet = 0;

    if( nOffset < nSliderXOffset )
        return mnMinZoom;
    if( nOffset > nControlWidth - nSliderXOffset )
        return mnMaxZoom;

    // check for snapping points:
    auto aSnappingPointIter = std::find_if(maSnappingPointOffsets.begin(), maSnappingPointOffsets.end(),
        [nOffset](const tools::Long nCurrent) { return std::abs(nCurrent - nOffset) < nSnappingEpsilon; });
    if (aSnappingPointIter != maSnappingPointOffsets.end())
    {
        nOffset = *aSnappingPointIter;
        auto nCount = static_cast<sal_uInt16>(std::distance(maSnappingPointOffsets.begin(), aSnappingPointIter));
        nRet = maSnappingPointZooms[ nCount ];
    }

    if( 0 == nRet )
    {
        if( nOffset < nControlWidth / 2 )
        {
            // first half of slider
            const tools::Long nFirstHalfRange      = gnSliderCenter - mnMinZoom;
            const tools::Long nHalfSliderWidth     = nControlWidth/2 - nSliderXOffset;
            const tools::Long nZoomPerSliderPixel  = (1000 * nFirstHalfRange) / nHalfSliderWidth;
            const tools::Long nOffsetToSliderLeft  = nOffset - nSliderXOffset;
            nRet = mnMinZoom + sal_uInt16( nOffsetToSliderLeft * nZoomPerSliderPixel / 1000 );
        }
        else
        {
            // second half of slider
            const tools::Long nSecondHalfRange         = mnMaxZoom - gnSliderCenter;
            const tools::Long nHalfSliderWidth         = nControlWidth/2 - nSliderXOffset;
            const tools::Long nZoomPerSliderPixel      = 1000 * nSecondHalfRange / nHalfSliderWidth;
            const tools::Long nOffsetToSliderCenter    = nOffset - nControlWidth/2;
            nRet = gnSliderCenter + sal_uInt16( nOffsetToSliderCenter * nZoomPerSliderPixel / 1000 );
        }
    }

    if( nRet < mnMinZoom )
        return mnMinZoom;

    else if( nRet > mnMaxZoom )
        return mnMaxZoom;

    return nRet;
}

tools::Long ScZoomSlider::Zoom2Offset( sal_uInt16 nCurrentZoom ) const
{
    const tools::Long nControlWidth = GetSliderLength();
    tools::Long  nRect = nSliderXOffset;

    const tools::Long nHalfSliderWidth = nControlWidth/2 - nSliderXOffset;
    if( nCurrentZoom <= gnSliderCenter )
    {
        nCurrentZoom = nCurrentZoom - mnMinZoom;
        const tools::Long nFirstHalfRange = gnSliderCenter - mnMinZoom;
        const tools::Long nSliderPixelPerZoomPercent = 1000 * nHalfSliderWidth  / nFirstHalfRange;
        const tools::Long nOffset = (nSliderPixelPerZoomPercent * nCurrentZoom) / 1000;
        nRect += nOffset;
    }
    else
    {
        nCurrentZoom = nCurrentZoom - gnSliderCenter;
        const tools::Long nSecondHalfRange = mnMaxZoom - gnSliderCenter;
        const tools::Long nSliderPixelPerZoomPercent = 1000 * nHalfSliderWidth  / nSecondHalfRange;
        const tools::Long nOffset = (nSliderPixelPerZoomPercent * nCurrentZoom) / 1000;
        nRect += nHalfSliderWidth + nOffset;
    }
    return nRect;
}

ScZoomSliderWnd::ScZoomSliderWnd( vcl::Window* pParent,
                const css::uno::Reference< css::frame::XDispatchProvider >& rDispatchProvider,
                sal_uInt16 nCurrentZoom ):
                InterimItemWindow(pParent, u"modules/scalc/ui/zoombox.ui"_ustr, u"ZoomBox"_ustr),
                mxWidget(new ScZoomSlider(rDispatchProvider, nCurrentZoom)),
                mxPercentage(m_xBuilder->weld_label(u"current_zoom"_ustr)),
                mxLabel(m_xBuilder->weld_label(u"zoom_label"_ustr)),
                mxWeld(new weld::CustomWeld(*m_xBuilder, u"zoom"_ustr, *mxWidget))
{
    Size aLogicalSize( 115, 40 );
    Size aSliderSize = LogicToPixel(aLogicalSize, MapMode(MapUnit::Map10thMM));
    Size aPreferredSize(aSliderSize.Width() * nSliderWidth-1, aSliderSize.Height() + nSliderHeight);
    mxWidget->GetDrawingArea()->set_size_request(aPreferredSize.Width(), aPreferredSize.Height());
    mxWidget->SetOutputSizePixel(aPreferredSize);
    mxWidget->SetSliderLength(aPreferredSize.Width() + nIncDecWidth);

    aPreferredSize.setWidth(aPreferredSize.Width() + mxLabel->get_pixel_size(mxLabel->get_label()).Width()
                      + mxPercentage->get_pixel_size(mxPercentage->get_label()).Width());

    SetSizePixel(aPreferredSize);
    OUString sCurrentZoom(unicode::formatPercent(nCurrentZoom, Application::GetSettings().GetUILanguageTag()));
    mxPercentage->set_label(sCurrentZoom);
}

ScZoomSliderWnd::~ScZoomSliderWnd()
{
    disposeOnce();
}

void ScZoomSliderWnd::dispose()
{
    mxWeld.reset();
    mxWidget.reset();
    InterimItemWindow::dispose();
}

ScZoomSlider::ScZoomSlider(css::uno::Reference< css::frame::XDispatchProvider> xDispatchProvider,
                           sal_uInt16 nCurrentZoom)
    : mnSliderLength(0)
    , mnCurrentZoom(nCurrentZoom)
    , mnMinZoom(10)
    , mnMaxZoom(400)
    , mbOmitPaint(false)
    , m_xDispatchProvider(std::move(xDispatchProvider))
{
    maSliderButton      = Image(StockImage::Yes, RID_SVXBMP_SLIDERBUTTON);
    maIncreaseButton    = Image(StockImage::Yes, RID_SVXBMP_SLIDERINCREASE);
    maDecreaseButton    = Image(StockImage::Yes, RID_SVXBMP_SLIDERDECREASE);
}


bool ScZoomSlider::MouseButtonDown( const MouseEvent& rMEvt )
{
    Size aSliderWindowSize = GetOutputSizePixel();

    const Point aPoint = rMEvt.GetPosPixel();

    const tools::Long nButtonLeftOffset    = ( nSliderXOffset - nIncDecWidth )/2;
    const tools::Long nButtonRightOffset   = ( nSliderXOffset + nIncDecWidth )/2;

    const tools::Long nOldZoom = mnCurrentZoom;

    // click to - button
    if ( aPoint.X() >= nButtonLeftOffset && aPoint.X() <= nButtonRightOffset )
    {
        mnCurrentZoom = mnCurrentZoom - 5;
    }
    // click to + button
    else if ( aPoint.X() >= GetSliderLength() - nSliderXOffset + nButtonLeftOffset &&
              aPoint.X() <= GetSliderLength() - nSliderXOffset + nButtonRightOffset )
    {
        mnCurrentZoom = mnCurrentZoom + 5;
    }
    else if( aPoint.X() >= nSliderXOffset && aPoint.X() <= GetSliderLength() - nSliderXOffset )
    {
        mnCurrentZoom = Offset2Zoom( aPoint.X() );
    }

    if( mnCurrentZoom < mnMinZoom )
        mnCurrentZoom = mnMinZoom;
    else if( mnCurrentZoom > mnMaxZoom )
        mnCurrentZoom = mnMaxZoom;

    if( nOldZoom == mnCurrentZoom )
        return true;

    tools::Rectangle aRect( Point( 0, 0 ), aSliderWindowSize );

    Invalidate(aRect);
    mbOmitPaint = true;

    SvxZoomSliderItem   aZoomSliderItem( mnCurrentZoom );

    css::uno::Any  a;
    aZoomSliderItem.QueryValue( a );

    css::uno::Sequence aArgs{ comphelper::makePropertyValue(u"ScalingFactor"_ustr, a) };

    SfxToolBoxControl::Dispatch( m_xDispatchProvider, u".uno:ScalingFactor"_ustr, aArgs );

    mbOmitPaint = false;

    return true;
}

bool ScZoomSlider::MouseMove( const MouseEvent& rMEvt )
{
    Size aSliderWindowSize   = GetOutputSizePixel();
    const tools::Long nControlWidth = GetSliderLength();
    const short nButtons     = rMEvt.GetButtons();

    // check mouse move with button pressed
    if ( 1 == nButtons )
    {
        const Point aPoint = rMEvt.GetPosPixel();

        if ( aPoint.X() >= nSliderXOffset && aPoint.X() <= nControlWidth - nSliderXOffset )
        {
            mnCurrentZoom = Offset2Zoom( aPoint.X() );

            tools::Rectangle aRect(Point(0, 0), aSliderWindowSize);
            Invalidate(aRect);

            mbOmitPaint = true; // optimization: paint before executing command,

            // commit state change
            SvxZoomSliderItem aZoomSliderItem( mnCurrentZoom );

            css::uno::Any a;
            aZoomSliderItem.QueryValue( a );

            css::uno::Sequence aArgs{ comphelper::makePropertyValue(u"ScalingFactor"_ustr, a) };

            SfxToolBoxControl::Dispatch( m_xDispatchProvider, u".uno:ScalingFactor"_ustr, aArgs );

            mbOmitPaint = false;
        }
    }

    return false;
}

void ScZoomSliderWnd::UpdateFromItem( const SvxZoomSliderItem* pZoomSliderItem )
{
    OUString sCurrentZoom(unicode::formatPercent(pZoomSliderItem->GetValue(), Application::GetSettings().GetUILanguageTag()));
    mxPercentage->set_label(sCurrentZoom);
    mxWidget->UpdateFromItem(pZoomSliderItem);
}

void ScZoomSlider::UpdateFromItem(const SvxZoomSliderItem* pZoomSliderItem)
{
    if( pZoomSliderItem )
    {
        mnCurrentZoom = pZoomSliderItem->GetValue();
        mnMinZoom     = pZoomSliderItem->GetMinZoom();
        mnMaxZoom     = pZoomSliderItem->GetMaxZoom();

        OSL_ENSURE( mnMinZoom <= mnCurrentZoom &&
            mnMinZoom <  gnSliderCenter &&
            mnMaxZoom >= mnCurrentZoom &&
            mnMaxZoom > gnSliderCenter,
            "Looks like the zoom slider item is corrupted" );
        const css::uno::Sequence < sal_Int32 >& rSnappingPoints = pZoomSliderItem->GetSnappingPoints();
        maSnappingPointOffsets.clear();
        maSnappingPointZooms.clear();

        // get all snapping points:
        std::set< sal_uInt16 > aTmpSnappingPoints;
        std::transform(rSnappingPoints.begin(), rSnappingPoints.end(), std::inserter(aTmpSnappingPoints, aTmpSnappingPoints.end()),
            [](const sal_Int32 nSnappingPoint) -> sal_uInt16 { return static_cast<sal_uInt16>(nSnappingPoint); });

        // remove snapping points that are too close to each other:
        tools::Long nLastOffset = 0;

        for ( const sal_uInt16 nCurrent : aTmpSnappingPoints )
        {
            const tools::Long nCurrentOffset = Zoom2Offset( nCurrent );

            if ( nCurrentOffset - nLastOffset >= nSnappingPointsMinDist )
            {
                maSnappingPointOffsets.push_back( nCurrentOffset );
                maSnappingPointZooms.push_back( nCurrent );
                nLastOffset = nCurrentOffset;
            }
        }
    }

    Size aSliderWindowSize = GetOutputSizePixel();
    tools::Rectangle aRect(Point(0, 0), aSliderWindowSize);

    if ( !mbOmitPaint )
       Invalidate(aRect);
}

void ScZoomSlider::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    DoPaint(rRenderContext);
}

void ScZoomSlider::DoPaint(vcl::RenderContext& rRenderContext)
{
    if (mbOmitPaint)
        return;

    Size aSliderWindowSize(GetOutputSizePixel());
    tools::Rectangle aRect(Point(0, 0), aSliderWindowSize);

    ScopedVclPtrInstance< VirtualDevice > pVDev(rRenderContext);
    pVDev->SetOutputSizePixel(aSliderWindowSize);

    tools::Rectangle aSlider = aRect;
    aSlider.setWidth(GetSliderLength());

    aSlider.AdjustTop((aSliderWindowSize.Height() - nSliderHeight) / 2 - 1 );
    aSlider.SetBottom( aSlider.Top() + nSliderHeight );
    aSlider.AdjustLeft(nSliderXOffset );
    aSlider.AdjustRight( -nSliderXOffset );

    tools::Rectangle aFirstLine(aSlider);
    aFirstLine.SetBottom( aFirstLine.Top() );

    tools::Rectangle aSecondLine(aSlider);
    aSecondLine.SetTop( aSecondLine.Bottom() );

    tools::Rectangle aLeft(aSlider);
    aLeft.SetRight( aLeft.Left() );

    tools::Rectangle aRight(aSlider);
    aRight.SetLeft( aRight.Right() );

    // draw VirtualDevice's background color
    Color aStartColor = rRenderContext.GetSettings().GetStyleSettings().GetFaceColor();
    Color aEndColor   = rRenderContext.GetSettings().GetStyleSettings().GetFaceColor();

    if (aEndColor.IsDark())
        aStartColor = aEndColor;

    Gradient aGradient;
    aGradient.SetAngle(0_deg10);
    aGradient.SetStyle(css::awt::GradientStyle_LINEAR);

    aGradient.SetStartColor(aStartColor);
    aGradient.SetEndColor(aEndColor);
    pVDev->DrawGradient(aRect, aGradient);

    // draw slider
    pVDev->SetLineColor(COL_WHITE);
    pVDev->DrawRect(aSecondLine);
    pVDev->DrawRect(aRight);

    pVDev->SetLineColor(COL_GRAY);
    pVDev->DrawRect(aFirstLine);
    pVDev->DrawRect(aLeft);

    // draw snapping points:
    for (const auto& rSnappingPointOffset : maSnappingPointOffsets)
    {
        pVDev->SetLineColor(COL_GRAY);
        tools::Rectangle aSnapping(aRect);
        aSnapping.SetBottom( aSlider.Top() );
        aSnapping.SetTop( aSnapping.Bottom() - nSnappingHeight );
        aSnapping.AdjustLeft(rSnappingPointOffset );
        aSnapping.SetRight( aSnapping.Left() );
        pVDev->DrawRect(aSnapping);

        aSnapping.AdjustTop(nSnappingHeight + nSliderHeight );
        aSnapping.AdjustBottom(nSnappingHeight + nSliderHeight );
        pVDev->DrawRect(aSnapping);
    }

    // draw slider button
    Point aImagePoint = aRect.TopLeft();
    aImagePoint.AdjustX(Zoom2Offset(mnCurrentZoom) );
    aImagePoint.AdjustX( -(nButtonWidth / 2) );
    aImagePoint.AdjustY( (aSliderWindowSize.Height() - nButtonHeight) / 2 );
    pVDev->DrawImage(aImagePoint, maSliderButton);

    // draw decrease button
    aImagePoint = aRect.TopLeft();
    aImagePoint.AdjustX((nSliderXOffset - nIncDecWidth) / 2 );
    aImagePoint.AdjustY((aSliderWindowSize.Height() - nIncDecHeight) / 2 );
    pVDev->DrawImage(aImagePoint, maDecreaseButton);

    // draw increase button
    aImagePoint.setX( aRect.Left() + GetSliderLength() - nIncDecWidth - (nSliderXOffset - nIncDecWidth) / 2 );
    pVDev->DrawImage(aImagePoint, maIncreaseButton);

    rRenderContext.DrawOutDev(Point(0, 0), aSliderWindowSize, Point(0, 0), aSliderWindowSize, *pVDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
