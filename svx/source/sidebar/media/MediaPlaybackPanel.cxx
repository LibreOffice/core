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
#include "MediaPlaybackPanel.hxx"
#include <vcl/outdev.hxx>
#include <avmedia/mediawindow.hxx>
#include <avmedia/mediaplayer.hxx>
#include <svtools/miscopt.hxx>
#include <avmedia/mediaitem.hxx>
#include <sfx2/sfxsids.hrc>
#include <avmedia/MediaControlHelper.hxx>
#include <sfx2/dispatch.hxx>

using ::rtl::OUString;
using namespace avmedia;

namespace svx { namespace sidebar {

MediaPlaybackPanel::MediaPlaybackPanel (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
    : PanelLayout(pParent, "MediaPlaybackPanel", "svx/ui/mediaplayback.ui", rxFrame),
    MediaControlHelper(),
    maMediaController(SID_AVMEDIA_TOOLBOX, *pBindings, *this),
    mpBindings(pBindings)
{
    get(mpTimer, "timeedit");
    get(mpPlayToolBox, "playtoolbox");
    get(mpMuteToolBox, "mutetoolbox");
    get(mpTimeSlider, "timeslider");
    get(mpVolumeSlider, "volumeslider");
    get(mpZoomLB, "zoombox");
    Initialize();
}

VclPtr< vcl::Window > MediaPlaybackPanel::Create(
    vcl::Window* pParent,
    const Reference< XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to MediaPlaybackPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to MediaPlaybackPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to MediaPlaybackPanel::Create", nullptr, 2);

    return VclPtr<MediaPlaybackPanel>::Create(
                pParent,
                rxFrame,
                pBindings);
}

MediaPlaybackPanel::~MediaPlaybackPanel()
{
    disposeOnce();
}

void MediaPlaybackPanel::Initialize()
{
    const OUString aTimeText( " 00:00:00/00:00:00 " );
    mpTimer->SetText( aTimeText );
    mpTimer->SetUpdateMode( true );
    mpTimer->Disable();

    mpVolumeSlider->SetRange( Range(AVMEDIA_DB_RANGE, 0) );
    mpVolumeSlider->SetUpdateMode(true);
    mpVolumeSlider->SetSlideHdl(LINK(this, MediaPlaybackPanel, VolumeSlideHdl));

    InitializePlayToolBox(mpPlayToolBox);
    mpPlayToolBox->SetSelectHdl(LINK(this, MediaPlaybackPanel, PlayToolBoxSelectHdl));
    InitializeMuteToolBox(mpMuteToolBox);
    mpMuteToolBox->SetSelectHdl(LINK(this, MediaPlaybackPanel, PlayToolBoxSelectHdl));

    mpTimeSlider->SetRange( Range( 0, AVMEDIA_TIME_RANGE ) );
    mpTimeSlider->SetUpdateMode( true );
    mpTimeSlider->SetSlideHdl(LINK(this, MediaPlaybackPanel, SeekHdl));
    InitializeZoomListBox(mpZoomLB);
    mpBindings->Invalidate(SID_AVMEDIA_TOOLBOX);
}

void MediaPlaybackPanel::dispose()
{
    mpTimer.disposeAndClear();
    PanelLayout::dispose();
}

void MediaPlaybackPanel::NotifyItemUpdate(
    const sal_uInt16 nSID,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;
    if( nSID == SID_AVMEDIA_TOOLBOX )
    {
        if(eState >= SfxItemState::DEFAULT)
        {
            mpMediaItem.reset(pState ? static_cast< MediaItem* >(pState->Clone()) : nullptr);
            Update();
        }
    }
}

void MediaPlaybackPanel::Update()
{
    UpdateButtons();
    UpdateTimeSlider(mpTimeSlider, *mpMediaItem);
    UpdateVolumeSlider( mpVolumeSlider, *mpMediaItem );
    UpdateTimeField(mpTimer, *mpMediaItem, mpMediaItem->getTime() );
}

void MediaPlaybackPanel::UpdateButtons()
{
    const bool bValidURL = !mpMediaItem->getURL().isEmpty();
    mpPlayToolBox->EnableItem( AVMEDIA_TOOLBOXITEM_PLAY, bValidURL );
    mpPlayToolBox->EnableItem( AVMEDIA_TOOLBOXITEM_PAUSE, bValidURL );
    mpPlayToolBox->EnableItem( AVMEDIA_TOOLBOXITEM_STOP, bValidURL );
    mpPlayToolBox->EnableItem( AVMEDIA_TOOLBOXITEM_LOOP, bValidURL );
    mpMuteToolBox->EnableItem( AVMEDIA_TOOLBOXITEM_MUTE, bValidURL );
    if( !bValidURL )
    {
        mpZoomLB->Disable();
        mpPlayToolBox->Disable();
        mpMuteToolBox->Disable();
    }
    else
    {
        mpPlayToolBox->Enable();
        mpMuteToolBox->Enable();
        if( MediaState::Play == mpMediaItem->getState() )
        {
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_PLAY );
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_PAUSE, false );
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_STOP, false );
        }
        else if( mpMediaItem->getTime() > 0.0 && ( mpMediaItem->getTime() < mpMediaItem->getDuration() ) )
        {
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_PLAY, false );
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_PAUSE );
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_STOP, false );
        }
        else
        {
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_PLAY, false );
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_PAUSE, false );
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_STOP );
        }
        mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_LOOP, mpMediaItem->isLoop() );
        mpMuteToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_MUTE, mpMediaItem->isMute() );
        if( !mpZoomLB->IsTravelSelect() && !mpZoomLB->IsInDropDown() )
        {
            sal_uInt16 nSelectEntryPos ;

            switch( mpMediaItem->getZoom() )
            {
                case( css::media::ZoomLevel_ZOOM_1_TO_2 ):
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_50;
                    break;
                case( css::media::ZoomLevel_ORIGINAL ):
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_100;
                    break;
                case( css::media::ZoomLevel_ZOOM_2_TO_1 ):
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_200;
                    break;
                case( css::media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT ):
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_FIT;
                    break;
                case( css::media::ZoomLevel_FIT_TO_WINDOW ):
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_SCALED;
                    break;

                default:
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_INVALID;
                    break;
            }

            if( nSelectEntryPos != AVMEDIA_ZOOMLEVEL_INVALID )
            {
                mpZoomLB->Enable();
                mpZoomLB->SelectEntryPos( nSelectEntryPos );
            }
            else
                mpZoomLB->Disable();
        }
    }
}

IMPL_LINK_NOARG_TYPED( MediaPlaybackPanel, VolumeSlideHdl, Slider*, void)
{
    MediaItem aItem(SID_AVMEDIA_TOOLBOX);
    aItem.setVolumeDB( static_cast< sal_Int16 > (mpVolumeSlider->GetThumbPos()));
    mpBindings->GetDispatcher()->ExecuteList(SID_AVMEDIA_TOOLBOX, SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG_TYPED( MediaPlaybackPanel, SeekHdl, Slider*, void)
{
    MediaItem aItem(SID_AVMEDIA_TOOLBOX);
    aItem.setState( MediaState::Pause );
    aItem.setTime( mpTimeSlider->GetThumbPos() * mpMediaItem->getDuration() / AVMEDIA_TIME_RANGE);
    mpBindings->GetDispatcher()->ExecuteList(SID_AVMEDIA_TOOLBOX, SfxCallMode::RECORD, { &aItem });
    mpBindings->Invalidate(SID_AVMEDIA_TOOLBOX);
}

IMPL_LINK_TYPED( MediaPlaybackPanel, PlayToolBoxSelectHdl, ToolBox*, pControl, void)
{
    MediaItem aItem(SID_AVMEDIA_TOOLBOX);
    switch(pControl->GetCurItemId())
    {
        case AVMEDIA_TOOLBOXITEM_PLAY:
        {
            aItem.setState( MediaState::Play );

            if( mpMediaItem->getTime() == mpMediaItem->getDuration() )
                aItem.setTime( 0.0 );
            else
                aItem.setTime( mpMediaItem->getTime());
        }
        break;

        case AVMEDIA_TOOLBOXITEM_PAUSE:
        {
            aItem.setState( MediaState::Pause );
        }
        break;

        case AVMEDIA_TOOLBOXITEM_STOP:
        {
            aItem.setState( MediaState::Stop );
            aItem.setTime( 0.0 );
        }
        break;

        case AVMEDIA_TOOLBOXITEM_MUTE:
        {
            aItem.setMute( !mpMuteToolBox->IsItemChecked( AVMEDIA_TOOLBOXITEM_MUTE ) );
        }
        break;

        case AVMEDIA_TOOLBOXITEM_LOOP:
        {
            aItem.setLoop( !mpPlayToolBox->IsItemChecked( AVMEDIA_TOOLBOXITEM_LOOP ) );
        }
        break;
        default:
            break;
    }
    if(aItem.getMaskSet() != AVMediaSetMask::NONE)
    {
        mpBindings->GetDispatcher()->ExecuteList(SID_AVMEDIA_TOOLBOX, SfxCallMode::RECORD, { &aItem } );
        mpBindings->Invalidate(SID_AVMEDIA_TOOLBOX);
    }
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
