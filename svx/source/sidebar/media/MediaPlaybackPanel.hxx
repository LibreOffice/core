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
#ifndef INCLUDED_SD_SOURCE_SIDEBAR_MEDIAPLAYBACKPANEL_HXX
#define INCLUDED_SD_SOURCE_SIDEBAR_MEDIAPLAYBACKPANEL_HXX

#include <vcl/ctrl.hxx>
#include <com/sun/star/frame/XFrame.hpp>

#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/layout.hxx>
#include <vcl/slider.hxx>
#include <avmedia/mediaitem.hxx>

using namespace css;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace svx { namespace sidebar {

/** This panel provides media playback control in document
*/
class MediaPlaybackPanel
    : public PanelLayout
{
public:
    MediaPlaybackPanel (
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
    virtual ~MediaPlaybackPanel();
    virtual void dispose() override;

private:
    VclPtr<Edit> mpTimer;
    VclPtr<Slider> mpTimeSlider;
    VclPtr<Slider> mpVolumeSlider;
    VclPtr<Button> mpPlayButton;
    VclPtr<Button> mpPauseButton;
    VclPtr<Button> mpStopButton;
    VclPtr<Button> mpRewindButton;
    VclPtr<Button> mpMuteButton;
    VclPtr<ListBox> mpZoomLB;
    ImageList maImageList;
    ::avmedia::MediaItem maItem;
    void Initialize();
};


} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */