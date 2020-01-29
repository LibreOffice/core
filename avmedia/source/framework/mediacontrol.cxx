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

#include <mediacontrol.hxx>
#include <strings.hrc>
#include <mediamisc.hxx>
#include <avmedia/mediawindow.hxx>
#include <helpids.h>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/layout.hxx>
#include <vcl/weld.hxx>
#include <unotools/syslocale.hxx>
#include <sfx2/viewfrm.hxx>
#include <math.h>
#include <algorithm>
#include <avmedia/MediaControlBase.hxx>

namespace avmedia
{

MediaControl::MediaControl( vcl::Window* pParent, MediaControlStyle eControlStyle ) :
    Control( pParent ),
    MediaControlBase(),
    maIdle( "avmedia MediaControl Idle" ),
    maItem( 0, AVMediaSetMask::ALL ),
    mbLocked( false ),
    meControlStyle( eControlStyle ),
    mfTime(0.0)
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);

    m_xVclContentArea = VclPtr<VclVBox>::Create(this);
    m_xVclContentArea->Show();
    m_xBuilder.reset(Application::CreateInterimBuilder(m_xVclContentArea, "svx/ui/mediawindow.ui"));
    m_xContainer = m_xBuilder->weld_container("MediaWindow");

    mxPlayToolBox = m_xBuilder->weld_toolbar("playtoolbox");
    mxTimeSlider = m_xBuilder->weld_scale("timeslider");
//TODO    mxTimeSlider->SetScrollTypeSet(true);
    mxMuteToolBox = m_xBuilder->weld_toolbar("mutetoolbox");
    mxVolumeSlider = m_xBuilder->weld_scale("volumeslider");
//TODO    mxVolumeSlider->SetScrollTypeSet(true);
    mxZoomListBox = m_xBuilder->weld_combo_box("zoombox");
    mxTimeEdit = m_xBuilder->weld_entry("timeedit");
    mxMediaPath = m_xBuilder->weld_label("url");

//TODO    SetBackground();
//TODO    SetPaintTransparent( true );
//TODO    SetParentClipMode( ParentClipMode::NoClip );

    InitializeWidgets();

    mxPlayToolBox->connect_clicked( LINK( this, MediaControl, implSelectHdl ) );
//TODO    mxPlayToolBox->SetSizePixel( mxPlayToolBox->CalcWindowSizePixel() );
//TODO    maMinSize = mxPlayToolBox->GetSizePixel();

    mxTimeSlider->connect_value_changed( LINK( this, MediaControl, implTimeHdl ) );
//TODO    mxTimeSlider->SetEndSlideHdl( LINK( this, MediaControl, implTimeEndHdl ) );
//TODO    mxTimeSlider->SetSizePixel( Size( 128, mxPlayToolBox->GetSizePixel().Height() ) );
//TODO    maMinSize.AdjustWidth(mxTimeSlider->GetSizePixel().Width() );

    const OUString aTimeText( " 00:00:00/00:00:00 " );
    mxTimeEdit->set_text(aTimeText);
    Size aTextSize = mxTimeEdit->get_preferred_size();
    mxTimeEdit->set_size_request(aTextSize.Width(), aTextSize.Height());
    mxTimeEdit->set_text(OUString());

//TODO    mxTimeEdit->SetSizePixel( Size( mxTimeEdit->GetTextWidth( aTimeText ) + 8, mxPlayToolBox->GetSizePixel().Height() ) );
//TODO    mxTimeEdit->SetControlBackground( Application::GetSettings().GetStyleSettings().GetWindowColor() );
//TODO    maMinSize.AdjustWidth(mxTimeEdit->GetSizePixel().Width() );

    mxMuteToolBox->connect_clicked( LINK( this, MediaControl, implSelectHdl ) );
//TODO    mxMuteToolBox->SetSizePixel( mxMuteToolBox->CalcWindowSizePixel() );
//TODO    maMinSize.AdjustWidth(mxMuteToolBox->GetSizePixel().Width() );

    mxVolumeSlider->connect_value_changed( LINK( this, MediaControl, implVolumeHdl ) );
//TODO    mxVolumeSlider->SetSizePixel( Size( 48, mxPlayToolBox->GetSizePixel().Height() ) );
//TODO    maMinSize.AdjustWidth(mxVolumeSlider->GetSizePixel().Width() );

//TODO    mxZoomListBox->SetSizePixel( Size( mxTimeEdit->GetSizePixel().Width(), 260 ) );
    mxZoomListBox->connect_changed( LINK( this, MediaControl, implZoomSelectHdl ) );
    mxZoomListBox->set_help_id(HID_AVMEDIA_ZOOMLISTBOX);

//TODO    mxZoomToolBox->SetItemWindow( AVMEDIA_TOOLBOXITEM_ZOOM, mxZoomListBox );
//TODO    mxZoomToolBox->SetSelectHdl( LINK( this, MediaControl, implSelectHdl ) );
//TODO    mxZoomToolBox->SetSizePixel( mxZoomToolBox->CalcWindowSizePixel() );
//TODO    maMinSize.AdjustWidth(mxZoomToolBox->GetSizePixel().Width() );

    const OUString aMediaPath( AvmResId( AVMEDIA_MEDIA_PATH_DEFAULT ) );
    mxMediaPath->set_label(aMediaPath);
//TODO    mxMediaPath->SetUpdateMode( false );
//TODO    mxMediaPath->SetSizePixel( Size( mxMediaPath->GetTextWidth( aMediaPath ) + 400, mxPlayToolBox->GetSizePixel().Height() ) ); // maybe extend the no. 400 to span the screen width
//TODO    mxMediaPath->SetControlBackground( Application::GetSettings().GetStyleSettings().GetWindowColor() );
//TODO    maMinSize.AdjustWidth(mxMediaPath->GetSizePixel().Width() );

    if( meControlStyle == MEDIACONTROLSTYLE_MULTILINE )
    {
        maMinSize.setWidth( 256 );
        maMinSize.setHeight( ( maMinSize.Height() << 1 ) + AVMEDIA_CONTROLOFFSET );
    }
    // we want time field + progress slider to update as the media plays
    // give this task a lower prio than REPAINT so that UI updates are not starved
    maIdle.SetPriority( TaskPriority::POST_PAINT );
    maIdle.SetInvokeHandler( LINK( this, MediaControl, implTimeoutHdl ) );
}

void MediaControl::InitializeWidgets()
{
    if( meControlStyle != MEDIACONTROLSTYLE_SINGLELINE )
    {
        mxPlayToolBox->set_item_visible("open", true);
        mxPlayToolBox->set_item_help_id("open", HID_AVMEDIA_TOOLBOXITEM_OPEN);
        mxPlayToolBox->set_item_visible("apply", true);
        mxPlayToolBox->set_item_help_id("apply", HID_AVMEDIA_TOOLBOXITEM_INSERT);
//TODO        mxPlayToolBox->set_item_visible("separator1", true);
    }
    else
    {
#if 0
        mxZoomListBox->SetBackground();
        mxPlayToolBox->SetBackground();
        mxPlayToolBox->SetPaintTransparent( true );
        mxMuteToolBox->SetBackground();
        mxMuteToolBox->SetPaintTransparent( true );
        mxMuteToolBox->InsertSeparator();
#endif
    }
    avmedia::MediaControlBase::InitializeWidgets();

//TODO    if( meControlStyle == MEDIACONTROLSTYLE_SINGLELINE )
//TODO        mxPlayToolBox->InsertSeparator();
}

MediaControl::~MediaControl()
{
    disposeOnce();
}

void MediaControl::dispose()
{
    mxZoomListBox.reset();
    mxTimeEdit.reset();
    mxMediaPath.reset();
    mxVolumeSlider.reset();
    mxMuteToolBox.reset();
    mxTimeSlider.reset();
    mxPlayToolBox.reset();
    m_xContainer.reset();
    m_xBuilder.reset();
    m_xVclContentArea.disposeAndClear();
    Control::dispose();
}

Size MediaControl::getMinSizePixel() const
{
    return VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));
}

void MediaControl::UpdateURLField(MediaItem const & tempItem)
{
    const OUString aURL( AvmResId(AVMEDIA_MEDIA_PATH) + ":  " + tempItem.getURL() ) ;
    mxMediaPath->set_label(aURL);
//TODO    mxMediaPath->SetUpdateMode( false );
//TODO    mxMediaPath->SetSizePixel( Size( mxMediaPath->GetTextWidth( aURL ) + 8, mxPlayToolBox->GetSizePixel().Height() ) );
//TODO    mxMediaPath->SetControlBackground( Application::GetSettings().GetStyleSettings().GetWindowColor() );
//TODO    maMinSize.AdjustWidth(mxMediaPath->GetSizePixel().Width() );
}

void MediaControl::Resize()
{
    vcl::Window *pChild = GetWindow(GetWindowType::FirstChild);
    assert(pChild);
    VclContainer::setLayoutAllocation(*pChild, Point(0, 0), GetSizePixel());
#if 0
    Point           aPos( 0, 0 );
    const sal_Int32 nPlayToolBoxWidth = mxPlayToolBox->GetSizePixel().Width();
    const sal_Int32 nMuteToolBoxWidth = mxMuteToolBox->GetSizePixel().Width();
    const sal_Int32 nVolumeSliderWidth = mxVolumeSlider->GetSizePixel().Width();
    const sal_Int32 nZoomToolBoxWidth = mxZoomToolBox->GetSizePixel().Width();
    const sal_Int32 nTimeEditWidth = mxTimeEdit->GetSizePixel().Width();
    const sal_Int32 nMediaPathWidth = mxMediaPath->GetSizePixel().Width();
    const sal_Int32 nTimeSliderHeight = mxTimeSlider->GetSizePixel().Height();

    if( meControlStyle == MEDIACONTROLSTYLE_SINGLELINE )
    {
        const sal_Int32 nTimeSliderWidth = GetSizePixel().Width() - ( AVMEDIA_CONTROLOFFSET * 4 ) -
                                           nPlayToolBoxWidth - nMuteToolBoxWidth - nVolumeSliderWidth - nTimeEditWidth - nZoomToolBoxWidth - nMediaPathWidth;

        mxPlayToolBox->SetPosSizePixel( aPos, mxPlayToolBox->GetSizePixel() );

        aPos.AdjustX(nPlayToolBoxWidth );
        mxTimeSlider->SetPosSizePixel( aPos, Size( nTimeSliderWidth, nTimeSliderHeight ) );

        aPos.AdjustX(nTimeSliderWidth + AVMEDIA_CONTROLOFFSET );
        mxTimeEdit->SetPosSizePixel( aPos, mxTimeEdit->GetSizePixel() );

        aPos.AdjustX(nTimeEditWidth + AVMEDIA_CONTROLOFFSET );
        mxMuteToolBox->SetPosSizePixel( aPos, mxMuteToolBox->GetSizePixel() );

        aPos.AdjustX(nMuteToolBoxWidth );
        mxVolumeSlider->SetPosSizePixel( aPos, mxVolumeSlider->GetSizePixel() );

        aPos.AdjustX(nVolumeSliderWidth + AVMEDIA_CONTROLOFFSET );
        mxZoomToolBox->SetPosSizePixel( aPos, mxZoomToolBox->GetSizePixel() );

        aPos.AdjustX(nZoomToolBoxWidth + AVMEDIA_CONTROLOFFSET );
        mxMediaPath->SetPosSizePixel( aPos, mxMediaPath->GetSizePixel() );
    }
    else
    {
        const sal_Int32 nTimeSliderWidth = GetSizePixel().Width() - AVMEDIA_CONTROLOFFSET - nTimeEditWidth;

        mxTimeSlider->SetPosSizePixel( aPos, Size( nTimeSliderWidth, nTimeSliderHeight ) );

        aPos.AdjustX(nTimeSliderWidth + AVMEDIA_CONTROLOFFSET );
        mxTimeEdit->SetPosSizePixel( aPos, mxTimeEdit->GetSizePixel() );

        aPos.setX( 0 );
        aPos.AdjustY(nTimeSliderHeight + AVMEDIA_CONTROLOFFSET );
        mxPlayToolBox->SetPosSizePixel( aPos, mxPlayToolBox->GetSizePixel() );

        aPos.setX( GetSizePixel().Width() - nVolumeSliderWidth - nMuteToolBoxWidth - nZoomToolBoxWidth - AVMEDIA_CONTROLOFFSET );
        mxMuteToolBox->SetPosSizePixel( aPos, mxMuteToolBox->GetSizePixel() );

        aPos.AdjustX(nMuteToolBoxWidth );
        mxVolumeSlider->SetPosSizePixel( aPos, mxVolumeSlider->GetSizePixel() );

        aPos.setX( GetSizePixel().Width() - nZoomToolBoxWidth );
        mxZoomToolBox->SetPosSizePixel( aPos, mxZoomToolBox->GetSizePixel() );
    }
#endif
}

void MediaControl::setState( const MediaItem& rItem )
{
    double fTime = rItem.getTime();
    if( !mbLocked && fTime != mfTime)
    {
        mfTime = fTime;
        maItem.merge( rItem );
        if( rItem.getURL().isEmpty() && meControlStyle == MEDIACONTROLSTYLE_SINGLELINE )
            mxPlayToolBox->set_sensitive(false);
        UpdateToolBoxes( maItem );
        UpdateTimeSlider( maItem );
        UpdateVolumeSlider( maItem );
        UpdateTimeField( maItem, maItem.getTime() );
        UpdateURLField(maItem);
    }
}

IMPL_LINK( MediaControl, implTimeHdl, weld::Scale&, rSlider, void )
{
    mbLocked = true;
    maIdle.Stop();
    UpdateTimeField(maItem, rSlider.get_value() * maItem.getDuration() / AVMEDIA_TIME_RANGE);
}

IMPL_LINK( MediaControl, implTimeEndHdl, weld::Scale&, rSlider, void )
{
    MediaItem aExecItem;

    aExecItem.setTime( rSlider.get_value() * maItem.getDuration() / AVMEDIA_TIME_RANGE );
    // keep state (if the media was playing, keep it playing)
    aExecItem.setState(maItem.getState());
    execute( aExecItem );
    update();
    maIdle.Start();
    mbLocked = false;
}

IMPL_LINK( MediaControl, implVolumeHdl, weld::Scale&, rSlider, void )
{
    MediaItem aExecItem;

    aExecItem.setVolumeDB(rSlider.get_value());
    execute( aExecItem );
    update();
}

IMPL_LINK( MediaControl, implSelectHdl, const OString&, rIdent, void )
{
    MediaItem aExecItem;
    if (rIdent == "open")
    {
        OUString aURL;
        if (MediaWindow::executeMediaURLDialog(GetFrameWeld(), aURL, nullptr))
        {
            if( !MediaWindow::isMediaURL( aURL, ""/*TODO?*/, true ) )
                MediaWindow::executeFormatErrorBox(GetFrameWeld());
            else
            {
                aExecItem.setURL( aURL, "", ""/*TODO?*/ );
                aExecItem.setState( MediaState::Play );
            }
        }
    }
    else
        SelectPlayToolBoxItem( aExecItem, maItem, rIdent );

    if (aExecItem.getState() == MediaState::Play)
        maIdle.Start();
    else if (aExecItem.getState() == MediaState::Pause ||
             aExecItem.getState() == MediaState::Stop)
        maIdle.Stop();

    if( aExecItem.getMaskSet() != AVMediaSetMask::NONE )
        execute( aExecItem );

    update();
#if 0
    if(p)
    {
        p->Invalidate( InvalidateFlags::Update );
    }
#endif
}

IMPL_LINK( MediaControl, implZoomSelectHdl, weld::ComboBox&, rBox, void )
{
    MediaItem aExecItem;
    css::media::ZoomLevel eLevel;

    switch (rBox.get_active())
    {
        case AVMEDIA_ZOOMLEVEL_50: eLevel = css::media::ZoomLevel_ZOOM_1_TO_2; break;
        case AVMEDIA_ZOOMLEVEL_100: eLevel = css::media::ZoomLevel_ORIGINAL; break;
        case AVMEDIA_ZOOMLEVEL_200: eLevel = css::media::ZoomLevel_ZOOM_2_TO_1; break;
        case AVMEDIA_ZOOMLEVEL_FIT: eLevel = css::media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT; break;
        case AVMEDIA_ZOOMLEVEL_SCALED: eLevel = css::media::ZoomLevel_FIT_TO_WINDOW; break;

        default: eLevel = css::media::ZoomLevel_NOT_AVAILABLE; break;
    }

    aExecItem.setZoom( eLevel );
    execute( aExecItem );
    update();
}

IMPL_LINK_NOARG(MediaControl, implTimeoutHdl, Timer *, void)
{
    update();
    maIdle.Start();
}

} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
