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
#include <avmedia/MediaControlBase.hxx>
#include <avmedia/mediawindow.hxx>
#include <avmedia/mediaplayer.hxx>
#include <vcl/edit.hxx>
#include <vcl/slider.hxx>
#include <vcl/toolbox.hxx>
#include <avmedia/mediaitem.hxx>
#include <svtools/miscopt.hxx>
#include <tools/time.hxx>
#include <vcl/lstbox.hxx>
#include "bitmaps.hlst"
#include "strings.hrc"
#include "helpids.hrc"
#include "mediamisc.hxx"

using ::rtl::OUString;

namespace avmedia {

MediaControlBase::MediaControlBase()
{
}

void MediaControlBase::UpdateTimeField( MediaItem const & aMediaItem, double fTime )
{
    if( !aMediaItem.getURL().isEmpty())
    {
        OUString aTimeString;

        SvtSysLocale aSysLocale;
        const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();

        aTimeString += rLocaleData.getDuration( tools::Time( 0, 0, static_cast< sal_uInt32 >( floor( fTime ) ) ) ) +
            " / " +
            rLocaleData.getDuration( tools::Time( 0, 0, static_cast< sal_uInt32 >( floor( aMediaItem.getDuration() ) )) );

        if( mpTimeEdit->GetText() != aTimeString )
            mpTimeEdit->SetText( aTimeString );
    }
}

void MediaControlBase::UpdateVolumeSlider( MediaItem const & aMediaItem )
{
    if( aMediaItem.getURL().isEmpty() )
        mpVolumeSlider->Disable();
    else
    {
        mpVolumeSlider->Enable();
        const sal_Int32 nVolumeDB = aMediaItem.getVolumeDB();
        mpVolumeSlider->SetThumbPos( std::min( std::max( nVolumeDB, static_cast< sal_Int32 >( AVMEDIA_DB_RANGE ) ),
                                                static_cast< sal_Int32 >( 0 ) ) );
    }
}

void MediaControlBase::UpdateTimeSlider( MediaItem const & aMediaItem )
{
    if( aMediaItem.getURL().isEmpty() )
        mpTimeSlider->Disable();
    else
    {
        mpTimeSlider->Enable();

        const double fDuration = aMediaItem.getDuration();

        if( fDuration > 0.0 )
        {
            const double fTime = std::min( aMediaItem.getTime(), fDuration );

            if( !mpTimeSlider->GetLineSize() )
                mpTimeSlider->SetLineSize( static_cast< sal_uInt32 >( AVMEDIA_TIME_RANGE * AVMEDIA_LINEINCREMENT / fDuration ) );

            if( !mpTimeSlider->GetPageSize() )
                mpTimeSlider->SetPageSize( static_cast< sal_uInt32 >( AVMEDIA_TIME_RANGE * AVMEDIA_PAGEINCREMENT / fDuration ) );

            mpTimeSlider->SetThumbPos( static_cast< sal_Int32 >( fTime / fDuration * AVMEDIA_TIME_RANGE ) );
        }
    }
}

void MediaControlBase::InitializeWidgets()
{
    mpPlayToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_PLAY, GetImage(AVMEDIA_TOOLBOXITEM_PLAY), AvmResId( AVMEDIA_STR_PLAY ), ToolBoxItemBits::CHECKABLE );
    mpPlayToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_PLAY, HID_AVMEDIA_TOOLBOXITEM_PLAY );

    mpPlayToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_PAUSE, GetImage(AVMEDIA_TOOLBOXITEM_PAUSE), AvmResId( AVMEDIA_STR_PAUSE ), ToolBoxItemBits::CHECKABLE );
    mpPlayToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_PAUSE, HID_AVMEDIA_TOOLBOXITEM_PAUSE );

    mpPlayToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_STOP, GetImage(AVMEDIA_TOOLBOXITEM_STOP), AvmResId( AVMEDIA_STR_STOP ), ToolBoxItemBits::CHECKABLE );
    mpPlayToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_STOP, HID_AVMEDIA_TOOLBOXITEM_STOP );

    mpPlayToolBox->InsertSeparator();

    mpPlayToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_LOOP, GetImage(AVMEDIA_TOOLBOXITEM_LOOP), AvmResId( AVMEDIA_STR_LOOP ) );
    mpPlayToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_LOOP, HID_AVMEDIA_TOOLBOXITEM_LOOP );

    mpMuteToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_MUTE, GetImage(AVMEDIA_TOOLBOXITEM_MUTE), AvmResId( AVMEDIA_STR_MUTE ) );
    mpMuteToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_MUTE, HID_AVMEDIA_TOOLBOXITEM_MUTE );

    mpZoomListBox->InsertEntry( AvmResId( AVMEDIA_STR_ZOOM_50 ), AVMEDIA_ZOOMLEVEL_50 );
    mpZoomListBox->InsertEntry( AvmResId( AVMEDIA_STR_ZOOM_100 ), AVMEDIA_ZOOMLEVEL_100 );
    mpZoomListBox->InsertEntry( AvmResId( AVMEDIA_STR_ZOOM_200 ), AVMEDIA_ZOOMLEVEL_200 );
    mpZoomListBox->InsertEntry( AvmResId( AVMEDIA_STR_ZOOM_FIT ), AVMEDIA_ZOOMLEVEL_FIT );
    mpZoomListBox->SetHelpId( HID_AVMEDIA_ZOOMLISTBOX );

    const OUString aTimeText( " 00:00:00/00:00:00 " );
    mpTimeEdit->SetText( aTimeText );
    mpTimeEdit->SetUpdateMode( true );
    mpTimeEdit->SetHelpId( HID_AVMEDIA_TIMEEDIT );
    mpTimeEdit->Disable();
    mpTimeEdit->Show();

    mpVolumeSlider->SetRange( Range( AVMEDIA_DB_RANGE, 0 ) );
    mpVolumeSlider->SetUpdateMode( true );
    mpVolumeSlider->SetHelpId( HID_AVMEDIA_VOLUMESLIDER );

    mpTimeSlider->SetRange( Range( 0, AVMEDIA_TIME_RANGE ) );
    mpTimeSlider->SetUpdateMode( true );
    mpTimeSlider->SetStyle(WB_HORZ | WB_DRAG | WB_3DLOOK | WB_SLIDERSET);
}

void MediaControlBase::UpdateToolBoxes(MediaItem aMediaItem)
{
    const bool bValidURL = !aMediaItem.getURL().isEmpty();
    mpPlayToolBox->EnableItem( AVMEDIA_TOOLBOXITEM_PLAY, bValidURL );
    mpPlayToolBox->EnableItem( AVMEDIA_TOOLBOXITEM_PAUSE, bValidURL );
    mpPlayToolBox->EnableItem( AVMEDIA_TOOLBOXITEM_STOP, bValidURL );
    mpPlayToolBox->EnableItem( AVMEDIA_TOOLBOXITEM_LOOP, bValidURL );
    mpMuteToolBox->EnableItem( AVMEDIA_TOOLBOXITEM_MUTE, bValidURL );
    if( !bValidURL )
    {
        mpZoomListBox->Disable();
        mpMuteToolBox->Disable();
    }
    else
    {
        mpPlayToolBox->Enable();
        mpMuteToolBox->Enable();
        if( aMediaItem.getState() == MediaState::Play )
        {
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_PLAY );
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_PAUSE, false );
            mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_STOP, false );
        }
        else if( aMediaItem.getTime() > 0.0 && ( aMediaItem.getTime() < aMediaItem.getDuration() ) )
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
        mpPlayToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_LOOP, aMediaItem.isLoop() );
        mpMuteToolBox->CheckItem( AVMEDIA_TOOLBOXITEM_MUTE, aMediaItem.isMute() );
        if( !mpZoomListBox->IsTravelSelect() && !mpZoomListBox->IsInDropDown() )
        {
            sal_uInt16 nSelectEntryPos ;

            switch( aMediaItem.getZoom() )
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
                mpZoomListBox->Enable();
                mpZoomListBox->SelectEntryPos( nSelectEntryPos );
            }
            else
                mpZoomListBox->Disable();
        }
    }
}

void MediaControlBase::SelectPlayToolBoxItem( MediaItem& aExecItem, MediaItem const & aItem, sal_uInt16 nId)
{
    switch( nId )
    {
        case AVMEDIA_TOOLBOXITEM_INSERT:
        {
            MediaFloater* pFloater = avmedia::getMediaFloater();

            if( pFloater )
                pFloater->dispatchCurrentURL();
        }
        break;

        case AVMEDIA_TOOLBOXITEM_PLAY:
        {
            aExecItem.setState( MediaState::Play );

            if( aItem.getTime() == aItem.getDuration() )
                aExecItem.setTime( 0.0 );
            else
                aExecItem.setTime( aItem.getTime() );
        }
        break;

        case AVMEDIA_TOOLBOXITEM_PAUSE:
        {
            aExecItem.setState( MediaState::Pause );
        }
        break;

        case AVMEDIA_TOOLBOXITEM_STOP:
        {
            aExecItem.setState( MediaState::Stop );
            aExecItem.setTime( 0.0 );
        }
        break;

        case AVMEDIA_TOOLBOXITEM_MUTE:
        {
            aExecItem.setMute( !mpMuteToolBox->IsItemChecked( AVMEDIA_TOOLBOXITEM_MUTE ) );
        }
        break;

        case AVMEDIA_TOOLBOXITEM_LOOP:
        {
            aExecItem.setLoop( !mpPlayToolBox->IsItemChecked( AVMEDIA_TOOLBOXITEM_LOOP ) );
        }
        break;

        default:
        break;
    }
}

Image MediaControlBase::GetImage(sal_Int32 nImageId)
{
    const bool bLarge = SvtMiscOptions().AreCurrentSymbolsLarge();

    OUString sImageId;

    if (bLarge)
    {
        switch (nImageId)
        {
            default:
            case AVMEDIA_TOOLBOXITEM_PLAY:
                sImageId = AVMEDIA_IMG_PLAY_LARGE;
                break;
            case AVMEDIA_TOOLBOXITEM_PAUSE:
                sImageId = AVMEDIA_IMG_PAUSE_LARGE;
                break;
            case AVMEDIA_TOOLBOXITEM_STOP:
                sImageId = AVMEDIA_IMG_STOP_LARGE;
                break;
            case AVMEDIA_TOOLBOXITEM_MUTE:
                sImageId = AVMEDIA_IMG_MUTE_LARGE;
                break;
            case AVMEDIA_TOOLBOXITEM_LOOP:
                sImageId = AVMEDIA_IMG_LOOP_LARGE;
                break;
            case AVMEDIA_TOOLBOXITEM_OPEN:
                sImageId = AVMEDIA_IMG_OPEN_LARGE;
                break;
            case AVMEDIA_TOOLBOXITEM_INSERT:
                sImageId = AVMEDIA_IMG_INSERT_LARGE;
                break;

        }
    }
    else
    {
        switch (nImageId)
        {
            default:
            case AVMEDIA_TOOLBOXITEM_PLAY:
                sImageId = AVMEDIA_IMG_PLAY_NORMAL;
                break;
            case AVMEDIA_TOOLBOXITEM_PAUSE:
                sImageId = AVMEDIA_IMG_PAUSE_NORMAL;
                break;
            case AVMEDIA_TOOLBOXITEM_STOP:
                sImageId = AVMEDIA_IMG_STOP_NORMAL;
                break;
            case AVMEDIA_TOOLBOXITEM_MUTE:
                sImageId = AVMEDIA_IMG_MUTE_NORMAL;
                break;
            case AVMEDIA_TOOLBOXITEM_LOOP:
                sImageId = AVMEDIA_IMG_LOOP_NORMAL;
                break;
            case AVMEDIA_TOOLBOXITEM_OPEN:
                sImageId = AVMEDIA_IMG_OPEN_NORMAL;
                break;
            case AVMEDIA_TOOLBOXITEM_INSERT:
                sImageId = AVMEDIA_IMG_INSERT_NORMAL;
                break;
        }
    }

    return Image(BitmapEx(sImageId));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
