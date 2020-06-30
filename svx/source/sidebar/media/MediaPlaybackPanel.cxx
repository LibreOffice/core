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

#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace avmedia;

namespace svx::sidebar {

MediaPlaybackPanel::MediaPlaybackPanel (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
    : PanelLayout(pParent, "MediaPlaybackPanel", "svx/ui/mediaplayback.ui", rxFrame),
    MediaControlBase(),
    maMediaController(SID_AVMEDIA_TOOLBOX, *pBindings, *this),
    maIdle("MediaPlaybackPanel"),
    mpBindings(pBindings)
{
    mxTimeEdit = m_xBuilder->weld_entry("timeedit");
    mxPlayToolBox = m_xBuilder->weld_toolbar("playtoolbox");
    mxMuteToolBox = m_xBuilder->weld_toolbar("mutetoolbox");
    mxTimeSlider = m_xBuilder->weld_scale("timeslider");
    mxVolumeSlider = m_xBuilder->weld_scale("volumeslider");
    mxZoomListBox = m_xBuilder->weld_combo_box("zoombox");

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

void MediaPlaybackPanel::dispose()
{
    disposeWidgets();
    PanelLayout::dispose();
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

IMPL_LINK( MediaPlaybackPanel, PlayToolBoxSelectHdl, const OString&, rId, void)
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
        aItem.setMute( mxMuteToolBox->get_item_active("mute") );
    }
    else if (rId == "loop")
    {
        aItem.setLoop( mxPlayToolBox->get_item_active("loop") );
    }

    if(aItem.getMaskSet() != AVMediaSetMask::NONE)
    {
        mpBindings->GetDispatcher()->ExecuteList(SID_AVMEDIA_TOOLBOX, SfxCallMode::RECORD, { &aItem } );
        mpBindings->Invalidate(SID_AVMEDIA_TOOLBOX);
    }
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
