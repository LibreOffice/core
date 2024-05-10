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
#include <avmedia/mediaitem.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/dispatch.hxx>
#include <avmedia/MediaControlBase.hxx>
#include <svl/itemset.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace avmedia;

namespace svx::sidebar {

MediaPlaybackPanel::MediaPlaybackPanel (
    weld::Widget* pParent,
    SfxBindings* pBindings)
    : PanelLayout(pParent, u"MediaPlaybackPanel"_ustr, u"svx/ui/mediaplayback.ui"_ustr),
    maMediaController(SID_AVMEDIA_TOOLBOX, *pBindings, *this),
    maIdle("MediaPlaybackPanel"),
    mpBindings(pBindings)
{
    mxTimeEdit = m_xBuilder->weld_entry(u"timeedit"_ustr);
    mxPlayToolBox = m_xBuilder->weld_toolbar(u"playtoolbox"_ustr);
    mxMuteToolBox = m_xBuilder->weld_toolbar(u"mutetoolbox"_ustr);
    mxTimeSlider = m_xBuilder->weld_scale(u"timeslider"_ustr);
    mxVolumeSlider = m_xBuilder->weld_scale(u"volumeslider"_ustr);
    mxZoomListBox = m_xBuilder->weld_combo_box(u"zoombox"_ustr);

    Initialize();
}

std::unique_ptr<PanelLayout> MediaPlaybackPanel::Create(
    weld::Widget* pParent,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(u"no parent Window given to MediaPlaybackPanel::Create"_ustr, nullptr, 0);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException(u"no SfxBindings given to MediaPlaybackPanel::Create"_ustr, nullptr, 2);

    return std::make_unique<MediaPlaybackPanel>(pParent, pBindings);
}

void MediaPlaybackPanel::Initialize()
{
    InitializeWidgets();
    mxVolumeSlider->connect_value_changed(LINK(this, MediaPlaybackPanel, VolumeSlideHdl));
    mxPlayToolBox->connect_clicked(LINK(this, MediaPlaybackPanel, PlayToolBoxSelectHdl));
    mxMuteToolBox->connect_clicked(LINK(this, MediaPlaybackPanel, PlayToolBoxSelectHdl));
    mxTimeSlider->connect_value_changed(LINK(this, MediaPlaybackPanel, SeekHdl));

    maIdle.SetPriority( TaskPriority::HIGHEST );
    maIdle.SetInvokeHandler( LINK( this, MediaPlaybackPanel, TimeoutHdl ) );
    maIdle.Start();
    mpBindings->Invalidate(SID_AVMEDIA_TOOLBOX);
}

MediaPlaybackPanel::~MediaPlaybackPanel()
{
    disposeWidgets();
}

void MediaPlaybackPanel::NotifyItemUpdate(
    const sal_uInt16 nSID,
    const SfxItemState eState,
    const SfxPoolItem* pState)
{
    if( nSID == SID_AVMEDIA_TOOLBOX )
    {
        if(eState >= SfxItemState::DEFAULT)
        {
            mpMediaItem.reset(pState ? static_cast< MediaItem* >(pState->Clone()) : nullptr);
            Update();
        }
    }
}

void MediaPlaybackPanel::UpdateToolBoxes(const MediaItem& rMediaItem)
{
    mxPlayToolBox->set_sensitive(false);
    avmedia::MediaControlBase::UpdateToolBoxes(rMediaItem);
}

void MediaPlaybackPanel::Update()
{
    if (mpMediaItem)
    {
        UpdateToolBoxes( *mpMediaItem );
        UpdateTimeSlider( *mpMediaItem );
        UpdateVolumeSlider( *mpMediaItem );
        UpdateTimeField( *mpMediaItem, mpMediaItem->getTime() );
    }
}

IMPL_LINK_NOARG( MediaPlaybackPanel, VolumeSlideHdl, weld::Scale&, void)
{
    MediaItem aItem(SID_AVMEDIA_TOOLBOX);
    aItem.setVolumeDB(mxVolumeSlider->get_value());
    mpBindings->GetDispatcher()->ExecuteList(SID_AVMEDIA_TOOLBOX, SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG( MediaPlaybackPanel, SeekHdl, weld::Scale&, void)
{
    MediaItem aItem(SID_AVMEDIA_TOOLBOX);
    double nTime = 0;
    if (mpMediaItem)
        nTime = mxTimeSlider->get_value() * mpMediaItem->getDuration() / AVMEDIA_TIME_RANGE;
    aItem.setTime(nTime);
    mpBindings->GetDispatcher()->ExecuteList(SID_AVMEDIA_TOOLBOX, SfxCallMode::RECORD, { &aItem });
    mpBindings->Invalidate(SID_AVMEDIA_TOOLBOX);
}

IMPL_LINK_NOARG( MediaPlaybackPanel, TimeoutHdl, Timer*, void)
{
    mpBindings->Invalidate(SID_AVMEDIA_TOOLBOX);
}

IMPL_LINK( MediaPlaybackPanel, PlayToolBoxSelectHdl, const OUString&, rId, void)
{
    MediaItem aItem(SID_AVMEDIA_TOOLBOX);

    if (rId == "play")
    {
        aItem.setState( MediaState::Play );

        if( !mpMediaItem || (mpMediaItem->getTime() == mpMediaItem->getDuration() ))
            aItem.setTime( 0.0 );
        else
            aItem.setTime( mpMediaItem->getTime());
    }
    else if (rId == "pause")
    {
        aItem.setState( MediaState::Pause );
    }
    else if (rId == "stop")
    {
        aItem.setState( MediaState::Stop );
        aItem.setTime( 0.0 );
    }
    else if (rId == "mute")
    {
        aItem.setMute( mxMuteToolBox->get_item_active(u"mute"_ustr) );
    }
    else if (rId == "loop")
    {
        aItem.setLoop( mxPlayToolBox->get_item_active(u"loop"_ustr) );
    }

    if(aItem.getMaskSet() != AVMediaSetMask::NONE)
    {
        mpBindings->GetDispatcher()->ExecuteList(SID_AVMEDIA_TOOLBOX, SfxCallMode::RECORD, { &aItem } );
        mpBindings->Invalidate(SID_AVMEDIA_TOOLBOX);
    }
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
