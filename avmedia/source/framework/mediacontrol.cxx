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
#include <vcl/weld.hxx>
#include <avmedia/MediaControlBase.hxx>

namespace avmedia
{

MediaControl::MediaControl( vcl::Window* pParent, MediaControlStyle eControlStyle ) :
    // MediaControlStyle::MultiLine is the normal docking windows of tools->media player
    // MediaControlStyle::SingleLine is the toolbar of view->toolbar->media playback
    InterimItemWindow(pParent, eControlStyle == MediaControlStyle::MultiLine ?
                                   u"svx/ui/mediawindow.ui"_ustr :
                                   u"svx/ui/medialine.ui"_ustr,
                               u"MediaWindow"_ustr),
    maIdle( "avmedia MediaControl Idle" ),
    maChangeTimeIdle( "avmedia MediaControl Change Time Idle" ),
    maItem( 0, AVMediaSetMask::ALL ),
    mbLocked( false ),
    meControlStyle( eControlStyle )
{
    mxPlayToolBox = m_xBuilder->weld_toolbar(u"playtoolbox"_ustr);
    mxTimeSlider = m_xBuilder->weld_scale(u"timeslider"_ustr);
    mxMuteToolBox = m_xBuilder->weld_toolbar(u"mutetoolbox"_ustr);
    mxVolumeSlider = m_xBuilder->weld_scale(u"volumeslider"_ustr);
    mxZoomListBox = m_xBuilder->weld_combo_box(u"zoombox"_ustr);
    mxTimeEdit = m_xBuilder->weld_entry(u"timeedit"_ustr);
    mxMediaPath = m_xBuilder->weld_label(u"url"_ustr);

    InitializeWidgets();

    mxPlayToolBox->connect_clicked( LINK( this, MediaControl, implSelectHdl ) );

    mxTimeSlider->connect_value_changed( LINK( this, MediaControl, implTimeHdl ) );
    // when changing the time, use this to do the time change after active scrolling
    // has stopped for a little which
    maChangeTimeIdle.SetPriority( TaskPriority::LOWEST );
    maChangeTimeIdle.SetInvokeHandler( LINK( this, MediaControl, implTimeEndHdl ) );

    mxTimeEdit->set_text(u" 00:00:00/00:00:00 "_ustr);
    Size aTextSize = mxTimeEdit->get_preferred_size();
    mxTimeEdit->set_size_request(aTextSize.Width(), aTextSize.Height());
    mxTimeEdit->set_text(OUString());

    mxMuteToolBox->connect_clicked( LINK( this, MediaControl, implSelectHdl ) );
    mxVolumeSlider->connect_value_changed( LINK( this, MediaControl, implVolumeHdl ) );

    mxZoomListBox->connect_changed( LINK( this, MediaControl, implZoomSelectHdl ) );
    mxZoomListBox->set_help_id(HID_AVMEDIA_ZOOMLISTBOX);

    const OUString aMediaPath( AvmResId( AVMEDIA_MEDIA_PATH_DEFAULT ) );
    mxMediaPath->set_label(aMediaPath);
    if (meControlStyle == MediaControlStyle::SingleLine)
        mxMediaPath->set_size_request(mxMediaPath->get_preferred_size().Width() + 400, -1); // maybe extend the no. 400 to span the screen width

    // we want time field + progress slider to update as the media plays
    // give this task a lower prio than REPAINT so that UI updates are not starved
    maIdle.SetPriority( TaskPriority::POST_PAINT );
    maIdle.SetInvokeHandler( LINK( this, MediaControl, implTimeoutHdl ) );
}

void MediaControl::InitializeWidgets()
{
    if( meControlStyle != MediaControlStyle::SingleLine )
    {
        mxPlayToolBox->set_item_help_id(u"open"_ustr, HID_AVMEDIA_TOOLBOXITEM_OPEN);
        mxPlayToolBox->set_item_help_id(u"apply"_ustr, HID_AVMEDIA_TOOLBOXITEM_INSERT);
    }
    avmedia::MediaControlBase::InitializeWidgets();
}

MediaControl::~MediaControl()
{
    disposeOnce();
}

void MediaControl::dispose()
{
    disposeWidgets();
    mxMediaPath.reset();
    InterimItemWindow::dispose();
}

void MediaControl::UpdateURLField(MediaItem const & tempItem)
{
    const OUString aURL( AvmResId(AVMEDIA_MEDIA_PATH) + ":  " + tempItem.getURL() ) ;
    mxMediaPath->set_label(aURL);
}

void MediaControl::setState( const MediaItem& rItem )
{
    if (mbLocked)
        return;
    bool bChanged = maItem.merge(rItem);
    if (bChanged)
    {
        if( rItem.getURL().isEmpty() && meControlStyle == MediaControlStyle::SingleLine )
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
    maChangeTimeIdle.Start();
}

IMPL_LINK_NOARG(MediaControl, implTimeEndHdl, Timer*, void)
{
    MediaItem aExecItem;

    aExecItem.setTime( mxTimeSlider->get_value() * maItem.getDuration() / AVMEDIA_TIME_RANGE );
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

IMPL_LINK( MediaControl, implSelectHdl, const OUString&, rIdent, void )
{
    MediaItem aExecItem;
    if (rIdent == "open")
    {
        OUString aURL;
        if (MediaWindow::executeMediaURLDialog(GetFrameWeld(), aURL, nullptr))
        {
            if( !MediaWindow::isMediaURL( aURL, u""_ustr/*TODO?*/, true ) )
                MediaWindow::executeFormatErrorBox(GetFrameWeld());
            else
            {
                aExecItem.setURL( aURL, u""_ustr, u""_ustr/*TODO?*/ );
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
}

IMPL_LINK( MediaControl, implZoomSelectHdl, weld::ComboBox&, rBox, void )
{
    bool bCurrentlySettingZoom = mbCurrentlySettingZoom;
    mbCurrentlySettingZoom = true;

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

    mbCurrentlySettingZoom = bCurrentlySettingZoom;
}

IMPL_LINK_NOARG(MediaControl, implTimeoutHdl, Timer *, void)
{
    update();
    maIdle.Start();
}

} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
