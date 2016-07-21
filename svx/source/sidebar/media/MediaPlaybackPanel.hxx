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
#include <vcl/toolbox.hxx>
#include <avmedia/mediaitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <avmedia/MediaControlBase.hxx>

using namespace css;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace svx { namespace sidebar {

/** This panel provides media playback control in document
*/
class MediaPlaybackPanel
    : public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface,
    public ::avmedia::MediaControlBase
{
public:
    MediaPlaybackPanel (
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~MediaPlaybackPanel();
    virtual void dispose() override;

private:
    std::unique_ptr< ::avmedia::MediaItem > mpMediaItem;
    ::sfx2::sidebar::ControllerItem         maMediaController;
//     VclPtr<Edit>    mpTimer;
//     VclPtr<Slider>  mpTimeSlider;
//     VclPtr<Slider>  mpVolumeSlider;
//     VclPtr<ToolBox> mpPlayToolBox;
//     VclPtr<ToolBox> mpMuteToolBox;
//     VclPtr<ListBox> mpZoomLB;
    Idle            maIdle;
    SfxBindings*    mpBindings;
    void Initialize();
    void Update();
    void NotifyItemUpdate( const sal_uInt16 nSID,
                           const SfxItemState eState,
                           const SfxPoolItem* pState,
                           const bool bIsEnabled);
    DECL_LINK_TYPED(PlayToolBoxSelectHdl, ToolBox*, void);
    DECL_LINK_TYPED(VolumeSlideHdl, Slider*, void);
    DECL_LINK_TYPED(SeekHdl, Slider*, void);
    DECL_LINK_TYPED(TimeoutHdl, Idle*, void);
};


} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */