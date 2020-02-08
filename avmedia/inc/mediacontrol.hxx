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

#pragma once

#include <avmedia/mediaitem.hxx>

#include <vcl/ctrl.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>
#include <avmedia/MediaControlBase.hxx>

#define AVMEDIA_CONTROLOFFSET 6

namespace avmedia
{

class MediaItem;

class MediaControl : public Control, public MediaControlBase
{
public:
                        MediaControl( vcl::Window* pParent, MediaControlStyle eControlStyle );
    virtual             ~MediaControl() override;
    virtual void        dispose() override;

    Size                getMinSizePixel() const;

    void                setState( const MediaItem& rItem );
    void                UpdateURLField( MediaItem const & maItem );

protected:

    virtual void        update() = 0;
    virtual void        execute( const MediaItem& rItem ) = 0;

    virtual void        Resize() override;
    virtual void        InitializeWidgets() override;
    std::unique_ptr<weld::Label> mxMediaPath;

private:

                        DECL_LINK(implTimeHdl, weld::Scale&, void);
                        DECL_LINK(implTimeEndHdl, Timer*, void);
                        DECL_LINK(implVolumeHdl, weld::Scale&, void);
                        DECL_LINK(implSelectHdl, const OString&, void);
                        DECL_LINK(implZoomSelectHdl, weld::ComboBox&, void);
                        DECL_LINK(implTimeoutHdl, Timer*, void);

    std::unique_ptr<weld::Builder> m_xBuilder;
    VclPtr<vcl::Window> m_xVclContentArea;
    std::unique_ptr<weld::Container> m_xContainer;

    Idle                maIdle;
    Idle                maChangeTimeIdle;
    MediaItem           maItem;
    bool                mbLocked;
    MediaControlStyle   meControlStyle;
    double mfTime;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
