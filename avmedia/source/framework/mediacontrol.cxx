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

#include "mediacontrol.hxx"
#include "mediacontrol.hrc"
#include "mediamisc.hxx"
#include <avmedia/mediawindow.hxx>
#include <avmedia/mediaplayer.hxx>
#include "helpids.hrc"
#include <tools/time.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/lstbox.hxx>
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
    meControlStyle( eControlStyle )
{
    mpPlayToolBox =  VclPtr<ToolBox>::Create(this, WB_3DLOOK) ;
    mpTimeSlider = VclPtr<Slider>::Create(this, WB_HORZ | WB_DRAG | WB_3DLOOK | WB_SLIDERSET) ;
    mpMuteToolBox = VclPtr<ToolBox>::Create(this, WB_3DLOOK) ;
    mpVolumeSlider = VclPtr<Slider>::Create(this, WB_HORZ | WB_DRAG | WB_SLIDERSET) ;
    mpZoomToolBox = VclPtr<ToolBox>::Create(this, WB_3DLOOK) ;
    mpZoomListBox = VclPtr<ListBox>::Create( mpZoomToolBox.get(), WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL | WB_3DLOOK ) ;
    mpTimeEdit = VclPtr<Edit>::Create(this, WB_CENTER | WB_READONLY | WB_BORDER | WB_3DLOOK ) ;
    mpMediaPath = VclPtr<FixedText>::Create(this, WB_VCENTER | WB_READONLY | WB_BORDER | WB_3DLOOK ) ;

    SetBackground();
    SetPaintTransparent( true );
    SetParentClipMode( ParentClipMode::NoClip );

    InitializeWidgets();

    mpPlayToolBox->SetSelectHdl( LINK( this, MediaControl, implSelectHdl ) );
    mpPlayToolBox->SetSizePixel( mpPlayToolBox->CalcWindowSizePixel() );
    mpPlayToolBox->Show();
    maMinSize = mpPlayToolBox->GetSizePixel();

    mpTimeSlider->SetSlideHdl( LINK( this, MediaControl, implTimeHdl ) );
    mpTimeSlider->SetEndSlideHdl( LINK( this, MediaControl, implTimeEndHdl ) );
    mpTimeSlider->SetSizePixel( Size( 128, mpPlayToolBox->GetSizePixel().Height() ) );
    mpTimeSlider->Show();
    maMinSize.Width() += mpTimeSlider->GetSizePixel().Width();

    const OUString aTimeText( " 00:00:00/00:00:00 " );
    mpTimeEdit->SetSizePixel( Size( mpTimeEdit->GetTextWidth( aTimeText ) + 8, mpPlayToolBox->GetSizePixel().Height() ) );
    mpTimeEdit->SetControlBackground( Application::GetSettings().GetStyleSettings().GetWindowColor() );
    maMinSize.Width() += mpTimeEdit->GetSizePixel().Width();

    mpMuteToolBox->SetSelectHdl( LINK( this, MediaControl, implSelectHdl ) );
    mpMuteToolBox->SetSizePixel( mpMuteToolBox->CalcWindowSizePixel() );
    mpMuteToolBox->Show();
    maMinSize.Width() += mpMuteToolBox->GetSizePixel().Width();

    mpVolumeSlider->SetSlideHdl( LINK( this, MediaControl, implVolumeHdl ) );
    mpVolumeSlider->SetSizePixel( Size( 48, mpPlayToolBox->GetSizePixel().Height() ) );
    mpVolumeSlider->Show();
    maMinSize.Width() += mpVolumeSlider->GetSizePixel().Width();

    mpZoomListBox->SetSizePixel( Size( mpTimeEdit->GetSizePixel().Width(), 260 ) );
    mpZoomListBox->SetSelectHdl( LINK( this, MediaControl, implZoomSelectHdl ) );

    mpZoomToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_ZOOM, OUString( AVMEDIA_RESID( AVMEDIA_STR_ZOOM ) ) );
    mpZoomToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_ZOOM, HID_AVMEDIA_ZOOMLISTBOX );

    mpZoomToolBox->SetItemWindow( AVMEDIA_TOOLBOXITEM_ZOOM, mpZoomListBox );
    mpZoomToolBox->SetSelectHdl( LINK( this, MediaControl, implSelectHdl ) );
    mpZoomToolBox->SetSizePixel( mpZoomToolBox->CalcWindowSizePixel() );
    mpZoomToolBox->Show();
    maMinSize.Width() += mpZoomToolBox->GetSizePixel().Width();

    const OUString aMediaPath( AVMEDIA_RESSTR( AVMEDIA_MEDIA_PATH_DEFAULT ) );
    mpMediaPath->SetText(aMediaPath);
    mpMediaPath->SetUpdateMode( false );
    mpMediaPath->SetSizePixel( Size( mpMediaPath->GetTextWidth( aMediaPath ) + 400, mpPlayToolBox->GetSizePixel().Height() ) ); // maybe extend the no. 400 to span the screen width
    mpMediaPath->SetControlBackground( Application::GetSettings().GetStyleSettings().GetWindowColor() );
    mpMediaPath->Show();
    maMinSize.Width() += mpMediaPath->GetSizePixel().Width();

    if( MEDIACONTROLSTYLE_MULTILINE == meControlStyle )
    {
        maMinSize.Width() = 256;
        maMinSize.Height() = ( maMinSize.Height() << 1 ) + AVMEDIA_CONTROLOFFSET;
        mpZoomToolBox->SetBackground();
        mpZoomToolBox->SetPaintTransparent( true );
    }

    maIdle.SetPriority( SchedulerPriority::HIGH_IDLE );
    maIdle.SetIdleHdl( LINK( this, MediaControl, implTimeoutHdl ) );
    maIdle.Start();
}

void MediaControl::InitializeWidgets()
{
    if( MEDIACONTROLSTYLE_SINGLELINE != meControlStyle )
    {
        mpPlayToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_OPEN, GetImage( AVMEDIA_IMG_OPEN ), OUString( AVMEDIA_RESID( AVMEDIA_STR_OPEN ) ) );
        mpPlayToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_OPEN, HID_AVMEDIA_TOOLBOXITEM_OPEN );
        mpPlayToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_INSERT, GetImage( AVMEDIA_IMG_INSERT ), OUString( AVMEDIA_RESID( AVMEDIA_STR_INSERT ) ) );
        mpPlayToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_INSERT, HID_AVMEDIA_TOOLBOXITEM_INSERT );
        mpPlayToolBox->InsertSeparator();
    }
    else
    {
        mpZoomListBox->SetBackground();
        mpPlayToolBox->SetBackground();
        mpPlayToolBox->SetPaintTransparent( true );
        mpMuteToolBox->SetBackground();
        mpMuteToolBox->SetPaintTransparent( true );
        mpMuteToolBox->InsertSeparator();
    }
    avmedia::MediaControlBase::InitializeWidgets();

    if( meControlStyle == MEDIACONTROLSTYLE_SINGLELINE )
        mpPlayToolBox->InsertSeparator();
}

MediaControl::~MediaControl()
{
    disposeOnce();
}

void MediaControl::dispose()
{
    mpZoomToolBox->SetItemWindow( AVMEDIA_TOOLBOXITEM_ZOOM, nullptr );
    mpZoomListBox.disposeAndClear();
    mpTimeEdit.disposeAndClear();
    mpMediaPath.disposeAndClear();
    mpZoomToolBox.disposeAndClear();
    mpVolumeSlider.disposeAndClear();
    mpMuteToolBox.disposeAndClear();
    mpTimeSlider.disposeAndClear();
    mpPlayToolBox.disposeAndClear();
    Control::dispose();
}

const Size& MediaControl::getMinSizePixel() const
{
    return maMinSize;
}

void MediaControl::UpdateURLField(MediaItem tempItem)
{
    const OUString aURL( AVMEDIA_RESSTR( AVMEDIA_MEDIA_PATH ) + ":  " + tempItem.getURL() ) ;
    mpMediaPath->SetText(aURL);
    mpMediaPath->SetUpdateMode( false );
    mpMediaPath->SetSizePixel( Size( mpMediaPath->GetTextWidth( aURL ) + 8, mpPlayToolBox->GetSizePixel().Height() ) );
    mpMediaPath->SetControlBackground( Application::GetSettings().GetStyleSettings().GetWindowColor() );
    mpMediaPath->Show();
    maMinSize.Width() += mpMediaPath->GetSizePixel().Width();
}

void MediaControl::Resize()
{
    Point           aPos( 0, 0 );
    const sal_Int32 nPlayToolBoxWidth = mpPlayToolBox->GetSizePixel().Width();
    const sal_Int32 nMuteToolBoxWidth = mpMuteToolBox->GetSizePixel().Width();
    const sal_Int32 nVolumeSliderWidth = mpVolumeSlider->GetSizePixel().Width();
    const sal_Int32 nZoomToolBoxWidth = mpZoomToolBox->GetSizePixel().Width();
    const sal_Int32 nTimeEditWidth = mpTimeEdit->GetSizePixel().Width();
    const sal_Int32 nMediaPathWidth = mpMediaPath->GetSizePixel().Width();
    const sal_Int32 nTimeSliderHeight = mpTimeSlider->GetSizePixel().Height();

    if( MEDIACONTROLSTYLE_SINGLELINE == meControlStyle )
    {
        const sal_Int32 nTimeSliderWidth = GetSizePixel().Width() - ( AVMEDIA_CONTROLOFFSET * 4 ) -
                                           nPlayToolBoxWidth - nMuteToolBoxWidth - nVolumeSliderWidth - nTimeEditWidth - nZoomToolBoxWidth - nMediaPathWidth;

        mpPlayToolBox->SetPosSizePixel( aPos, mpPlayToolBox->GetSizePixel() );

        aPos.X() += nPlayToolBoxWidth;
        mpTimeSlider->SetPosSizePixel( aPos, Size( nTimeSliderWidth, nTimeSliderHeight ) );

        aPos.X() += nTimeSliderWidth + AVMEDIA_CONTROLOFFSET;
        mpTimeEdit->SetPosSizePixel( aPos, mpTimeEdit->GetSizePixel() );

        aPos.X() += nTimeEditWidth + AVMEDIA_CONTROLOFFSET;
        mpMuteToolBox->SetPosSizePixel( aPos, mpMuteToolBox->GetSizePixel() );

        aPos.X() += nMuteToolBoxWidth;
        mpVolumeSlider->SetPosSizePixel( aPos, mpVolumeSlider->GetSizePixel() );

        aPos.X() += nVolumeSliderWidth + AVMEDIA_CONTROLOFFSET;
        mpZoomToolBox->SetPosSizePixel( aPos, mpZoomToolBox->GetSizePixel() );

        aPos.X() += nZoomToolBoxWidth + AVMEDIA_CONTROLOFFSET;
        mpMediaPath->SetPosSizePixel( aPos, mpMediaPath->GetSizePixel() );
    }
    else
    {
        const sal_Int32 nTimeSliderWidth = GetSizePixel().Width() - AVMEDIA_CONTROLOFFSET - nTimeEditWidth;

        mpTimeSlider->SetPosSizePixel( aPos, Size( nTimeSliderWidth, nTimeSliderHeight ) );

        aPos.X() += nTimeSliderWidth + AVMEDIA_CONTROLOFFSET;
        mpTimeEdit->SetPosSizePixel( aPos, mpTimeEdit->GetSizePixel() );

        aPos.X() = 0;
        aPos.Y() += nTimeSliderHeight + AVMEDIA_CONTROLOFFSET;
        mpPlayToolBox->SetPosSizePixel( aPos, mpPlayToolBox->GetSizePixel() );

        aPos.X() = GetSizePixel().Width() - nVolumeSliderWidth - nMuteToolBoxWidth - nZoomToolBoxWidth - AVMEDIA_CONTROLOFFSET;
        mpMuteToolBox->SetPosSizePixel( aPos, mpMuteToolBox->GetSizePixel() );

        aPos.X() += nMuteToolBoxWidth;
        mpVolumeSlider->SetPosSizePixel( aPos, mpVolumeSlider->GetSizePixel() );

        aPos.X() = GetSizePixel().Width() - nZoomToolBoxWidth;
        mpZoomToolBox->SetPosSizePixel( aPos, mpZoomToolBox->GetSizePixel() );
    }
}


void MediaControl::setState( const MediaItem& rItem )
{
    if( !mbLocked )
    {
        maItem.merge( rItem );
        if( rItem.getURL().isEmpty() && meControlStyle == MEDIACONTROLSTYLE_SINGLELINE )
            mpPlayToolBox->Disable();
        UpdateToolBoxes( maItem );
        UpdateTimeSlider( maItem );
        UpdateVolumeSlider( maItem );
        UpdateTimeField( maItem, maItem.getTime() );
        UpdateURLField(maItem);
    }
}

IMPL_LINK_TYPED( MediaControl, implTimeHdl, Slider*, p, void )
{
    mbLocked = true;
    maIdle.Stop();
    UpdateTimeField( maItem, p->GetThumbPos() * maItem.getDuration() / AVMEDIA_TIME_RANGE );
}


IMPL_LINK_TYPED( MediaControl, implTimeEndHdl, Slider*, p, void )
{
    MediaItem aExecItem;

    aExecItem.setTime( p->GetThumbPos() * maItem.getDuration() / AVMEDIA_TIME_RANGE );
    execute( aExecItem );
    update();
    maIdle.Start();
    mbLocked = false;
}


IMPL_LINK_TYPED( MediaControl, implVolumeHdl, Slider*, p, void )
{
    MediaItem aExecItem;

    aExecItem.setVolumeDB( static_cast< sal_Int16 >( p->GetThumbPos() ) );
    execute( aExecItem );
    update();
}


IMPL_LINK_TYPED( MediaControl, implSelectHdl, ToolBox*, p, void )
{
    if( p )
    {
        MediaItem aExecItem;
        if( p->GetCurItemId() == AVMEDIA_TOOLBOXITEM_OPEN )
        {
            OUString aURL;

             if (MediaWindow::executeMediaURLDialog(
                         GetParent(), aURL, nullptr))
             {
                 if( !MediaWindow::isMediaURL( aURL, ""/*TODO?*/, true ) )
                    MediaWindow::executeFormatErrorBox( this );
                else
                {
                    aExecItem.setURL( aURL, "", ""/*TODO?*/ );
                    aExecItem.setState( MediaState::Play );
                }
            }
        }
        else
            SelectPlayToolBoxItem( aExecItem, maItem, p->GetCurItemId() );

        if( aExecItem.getMaskSet() != AVMediaSetMask::NONE )
            execute( aExecItem );
    }

    update();
    if(p)
    {
        p->Invalidate( InvalidateFlags::Update );
    }
}


IMPL_LINK_TYPED( MediaControl, implZoomSelectHdl, ListBox&, p, void )
{
    MediaItem aExecItem;
    css::media::ZoomLevel eLevel;

    switch( p.GetSelectEntryPos() )
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


IMPL_LINK_NOARG_TYPED(MediaControl, implTimeoutHdl, Idle *, void)
{
    update();
}

} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
