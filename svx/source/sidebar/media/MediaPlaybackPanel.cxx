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

using ::rtl::OUString;
using namespace avmedia;

namespace svx { namespace sidebar {

MediaPlaybackPanel::MediaPlaybackPanel (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "MediaPlaybackPanel", "svx/ui/mediaplayback.ui", rxFrame),
    maItem(SID_AVMEDIA_TOOLBOX)
{
    get(mpTimer, "timeedit");
    get(mpPlayButton, "play");
    get(mpPauseButton, "pause");
    get(mpMuteButton, "mute");
    get(mpRewindButton, "rewind");
    get(mpTimeSlider, "timeslider");
    get(mpVolumeSlider, "volumeslider");
    get(mpZoomLB, "zoombox");
    Initialize();
}

VclPtr< vcl::Window > MediaPlaybackPanel::Create(
    vcl::Window* pParent,
    const Reference< XFrame >& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to MediaPlaybackPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to MediaPlaybackPanel::Create", nullptr, 1);

    return VclPtr<MediaPlaybackPanel>::Create(
                pParent,
                rxFrame);
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
}

void MediaPlaybackPanel::dispose()
{
    mpTimer.disposeAndClear();
    PanelLayout::dispose();
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
