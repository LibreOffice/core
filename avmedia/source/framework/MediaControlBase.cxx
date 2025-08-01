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
#include <avmedia/mediaplayer.hxx>
#include <avmedia/mediaitem.hxx>
#include <tools/duration.hxx>
#include <unotools/localedatawrapper.hxx>
#include <strings.hrc>
#include <helpids.h>
#include <mediamisc.hxx>

constexpr sal_Int32 AVMEDIA_DB_RANGE = -40;
constexpr double AVMEDIA_LINEINCREMENT = 1.0;
constexpr double AVMEDIA_PAGEINCREMENT = 10.0;

namespace avmedia {

MediaControlBase::MediaControlBase()
    : mbCurrentlySettingZoom(false)
{
}

void MediaControlBase::UpdateTimeField( MediaItem const & aMediaItem, double fTime )
{
    if( aMediaItem.getURL().isEmpty())
        return;

    OUString aTimeString;

    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();

    aTimeString += rLocaleData.getDuration(
            tools::Duration( 0, 0, 0, static_cast<sal_uInt32>( floor( fTime )), 0)) +
        " / " +
        rLocaleData.getDuration(
                tools::Duration( 0, 0, 0, static_cast<sal_uInt32>( floor( aMediaItem.getDuration())), 0));

    if( mxTimeEdit->get_text() != aTimeString )
        mxTimeEdit->set_text( aTimeString );
}

void MediaControlBase::UpdateVolumeSlider( MediaItem const & aMediaItem )
{
    if( aMediaItem.getURL().isEmpty() )
        mxVolumeSlider->set_sensitive(false);
    else
    {
        mxVolumeSlider->set_sensitive(true);
        const sal_Int32 nVolumeDB = aMediaItem.getVolumeDB();
        mxVolumeSlider->set_value( std::clamp( nVolumeDB, AVMEDIA_DB_RANGE, sal_Int32(0)) );
    }
}

void MediaControlBase::UpdateTimeSlider( MediaItem const & aMediaItem )
{
    if( aMediaItem.getURL().isEmpty() )
        mxTimeSlider->set_sensitive(false);
    else
    {
        mxTimeSlider->set_sensitive(true);

        const double fDuration = aMediaItem.getDuration();

        if( fDuration > 0.0 )
        {
            const double fTime = std::min( aMediaItem.getTime(), fDuration );

            bool bChanged(false);
            int nStep(0), nPage(0);
            mxTimeSlider->get_increments(nStep, nPage);
            if (!nStep)
            {
                nStep = AVMEDIA_TIME_RANGE * AVMEDIA_LINEINCREMENT / fDuration;
                bChanged = true;
            }
            if (!nPage)
            {
                nPage = AVMEDIA_TIME_RANGE * AVMEDIA_PAGEINCREMENT / fDuration;
                bChanged = true;
            }
            if (bChanged)
                mxTimeSlider->set_increments(nStep, nPage);

            mxTimeSlider->set_value(fTime / fDuration * AVMEDIA_TIME_RANGE);
        }
    }
}

void MediaControlBase::InitializeWidgets()
{
    mxPlayToolBox->set_item_help_id(u"play"_ustr, HID_AVMEDIA_TOOLBOXITEM_PLAY);
    mxPlayToolBox->set_item_help_id(u"pause"_ustr, HID_AVMEDIA_TOOLBOXITEM_PAUSE);
    mxPlayToolBox->set_item_help_id(u"stop"_ustr, HID_AVMEDIA_TOOLBOXITEM_STOP);
    mxPlayToolBox->set_item_help_id(u"loop"_ustr, HID_AVMEDIA_TOOLBOXITEM_LOOP);
    mxMuteToolBox->set_item_help_id(u"mute"_ustr, HID_AVMEDIA_TOOLBOXITEM_MUTE);

    mxZoomListBox->append(OUString::number(AVMEDIA_ZOOMLEVEL_50), AvmResId( AVMEDIA_STR_ZOOM_50 ));
    mxZoomListBox->append(OUString::number(AVMEDIA_ZOOMLEVEL_100), AvmResId( AVMEDIA_STR_ZOOM_100 ));
    mxZoomListBox->append(OUString::number(AVMEDIA_ZOOMLEVEL_200), AvmResId( AVMEDIA_STR_ZOOM_200 ));
    mxZoomListBox->append(OUString::number(AVMEDIA_ZOOMLEVEL_FIT),  AvmResId( AVMEDIA_STR_ZOOM_FIT ));
    mxZoomListBox->set_help_id( HID_AVMEDIA_ZOOMLISTBOX );
    mxZoomListBox->set_tooltip_text(AvmResId( AVMEDIA_STR_ZOOM_TOOLTIP ));

    mxTimeEdit->set_text( u" 00:00:00/00:00:00 "_ustr );
    mxTimeEdit->set_help_id( HID_AVMEDIA_TIMEEDIT );
    mxTimeEdit->set_sensitive(false);

    mxVolumeSlider->set_range(AVMEDIA_DB_RANGE, 0);
    mxVolumeSlider->set_tooltip_text( AvmResId( AVMEDIA_STR_VOLUME ));
    mxVolumeSlider->set_help_id( HID_AVMEDIA_VOLUMESLIDER );

    mxTimeSlider->set_range( 0, AVMEDIA_TIME_RANGE );
    mxTimeSlider->set_tooltip_text( AvmResId( AVMEDIA_STR_POSITION ));
}

void MediaControlBase::UpdatePlayState(const MediaItem& rMediaItem)
{
    if (rMediaItem.getState() == MediaState::Play)
    {
        mxPlayToolBox->set_item_active(u"play"_ustr, true);
        mxPlayToolBox->set_item_active(u"pause"_ustr, false);
        mxPlayToolBox->set_item_active(u"stop"_ustr, false);
    }
    else if( rMediaItem.getState() == MediaState::Pause )
    {
        mxPlayToolBox->set_item_active(u"play"_ustr, false);
        mxPlayToolBox->set_item_active(u"pause"_ustr, true);
        mxPlayToolBox->set_item_active(u"stop"_ustr, false);
    }
    else
    {
        mxPlayToolBox->set_item_active(u"play"_ustr, false);
        mxPlayToolBox->set_item_active(u"pause"_ustr, false);
        mxPlayToolBox->set_item_active(u"stop"_ustr, true);
    }
}

void MediaControlBase::UpdateToolBoxes(const MediaItem& rMediaItem)
{
    const bool bValidURL = !rMediaItem.getURL().isEmpty();
    mxPlayToolBox->set_item_sensitive(u"play"_ustr, bValidURL);
    mxPlayToolBox->set_item_sensitive(u"pause"_ustr, bValidURL);
    mxPlayToolBox->set_item_sensitive(u"stop"_ustr, bValidURL);
    mxPlayToolBox->set_item_sensitive(u"loop"_ustr, bValidURL);
    mxMuteToolBox->set_item_sensitive(u"mute"_ustr, bValidURL);
    if( !bValidURL )
    {
        mxZoomListBox->set_sensitive(false);
        mxMuteToolBox->set_sensitive(false);
    }
    else
    {
        mxPlayToolBox->set_sensitive(true);
        mxMuteToolBox->set_sensitive(true);
        UpdatePlayState(rMediaItem);
        mxPlayToolBox->set_item_active(u"loop"_ustr, rMediaItem.isLoop());
        mxMuteToolBox->set_item_active(u"mute"_ustr, rMediaItem.isMute());
        if (!mbCurrentlySettingZoom)
        {
            sal_uInt16 nSelectEntryPos ;

            switch( rMediaItem.getZoom() )
            {
                case css::media::ZoomLevel_ZOOM_1_TO_2:
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_50;
                    break;
                case css::media::ZoomLevel_ORIGINAL:
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_100;
                    break;
                case css::media::ZoomLevel_ZOOM_2_TO_1:
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_200;
                    break;
                case css::media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT:
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_FIT;
                    break;
                case css::media::ZoomLevel_FIT_TO_WINDOW:
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_SCALED;
                    break;

                default:
                    nSelectEntryPos = AVMEDIA_ZOOMLEVEL_INVALID;
                    break;
            }

            if( nSelectEntryPos != AVMEDIA_ZOOMLEVEL_INVALID )
            {
                mxZoomListBox->show();
                mxZoomListBox->set_sensitive(true);
                mxZoomListBox->set_active(nSelectEntryPos);
            }
            else
                mxZoomListBox->set_sensitive(false);
        }
    }
}

void MediaControlBase::SelectPlayToolBoxItem( MediaItem& aExecItem, MediaItem const & aItem, std::u16string_view rId)
{
    if (rId == u"apply")
    {
        MediaFloater* pFloater = avmedia::getMediaFloater();

        if( pFloater )
            pFloater->dispatchCurrentURL();
    }
    else if (rId == u"play")
    {
        aExecItem.setState( MediaState::Play );

        if( aItem.getTime() == aItem.getDuration() )
            aExecItem.setTime( 0.0 );
        else
            aExecItem.setTime( aItem.getTime() );

        UpdatePlayState(aExecItem);
    }
    else if (rId == u"pause")
    {
        aExecItem.setState( MediaState::Pause );

        UpdatePlayState(aExecItem);
    }
    else if (rId == u"stop")
    {
        aExecItem.setState( MediaState::Stop );
        aExecItem.setTime( 0.0 );

        UpdatePlayState(aExecItem);
    }
    else if (rId == u"mute")
    {
        aExecItem.setMute( mxMuteToolBox->get_item_active(u"mute"_ustr) );
    }
    else if (rId == u"loop")
    {
        aExecItem.setLoop( mxPlayToolBox->get_item_active(u"loop"_ustr) );
    }
}

void MediaControlBase::disposeWidgets()
{
    mxZoomListBox.reset();
    mxTimeEdit.reset();
    mxVolumeSlider.reset();
    mxMuteToolBox.reset();
    mxTimeSlider.reset();
    mxPlayToolBox.reset();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
