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
#ifndef INCLUDED_AVMEDIA_MEDIACONTROLBASE_HXX
#define INCLUDED_AVMEDIA_MEDIACONTROLBASE_HXX

#include <config_options.h>
#include <vcl/weld.hxx>

#include <avmedia/avmediadllapi.h>

namespace avmedia { class MediaItem; }

#define AVMEDIA_TIME_RANGE          2048

#define AVMEDIA_ZOOMLEVEL_50        0
#define AVMEDIA_ZOOMLEVEL_100       1
#define AVMEDIA_ZOOMLEVEL_200       2
#define AVMEDIA_ZOOMLEVEL_FIT       3
#define AVMEDIA_ZOOMLEVEL_SCALED    4
#define AVMEDIA_ZOOMLEVEL_INVALID   65535

namespace avmedia {

enum class MediaControlStyle
{
    SingleLine = 0,
    MultiLine = 1
};

class UNLESS_MERGELIBS(AVMEDIA_DLLPUBLIC) MediaControlBase
{
public:
    MediaControlBase();
    virtual ~MediaControlBase(){};

protected:
    std::unique_ptr<weld::Toolbar> mxPlayToolBox;
    std::unique_ptr<weld::Scale> mxTimeSlider;
    std::unique_ptr<weld::Toolbar> mxMuteToolBox;
    std::unique_ptr<weld::Scale> mxVolumeSlider;
    std::unique_ptr<weld::ComboBox> mxZoomListBox;
    std::unique_ptr<weld::Entry> mxTimeEdit;
    bool mbCurrentlySettingZoom;

    virtual void InitializeWidgets();
    virtual void UpdateToolBoxes(const MediaItem& rMediaItem);
    void UpdateVolumeSlider( MediaItem const & aMediaItem );
    void UpdateTimeSlider( MediaItem const & aMediaItem );
    void UpdateTimeField( MediaItem const & aMediaItem, double fTime );
    void SelectPlayToolBoxItem( MediaItem& aExecItem, MediaItem const & aItem, const OString& rId);
    void disposeWidgets();
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
