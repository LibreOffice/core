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

#include <svx/svxdllapi.h>
#include <vcl/edit.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/slider.hxx>
#include <avmedia/mediaitem.hxx>
#include <avmedia/mediawindow.hxx>

#define AVMEDIA_TIME_RANGE          2048
#define AVMEDIA_DB_RANGE            -40
#define AVMEDIA_LINEINCREMENT       1.0
#define AVMEDIA_PAGEINCREMENT       10.0

#define AVMEDIA_TOOLBOXITEM_PLAY    0x0001
#define AVMEDIA_TOOLBOXITEM_PAUSE   0x0004
#define AVMEDIA_TOOLBOXITEM_STOP    0x0008
#define AVMEDIA_TOOLBOXITEM_MUTE    0x0010
#define AVMEDIA_TOOLBOXITEM_LOOP    0x0011
#define AVMEDIA_TOOLBOXITEM_ZOOM    0x0012
#define AVMEDIA_TOOLBOXITEM_OPEN    0x0014
#define AVMEDIA_TOOLBOXITEM_INSERT  0x0018

#define AVMEDIA_ZOOMLEVEL_50        0
#define AVMEDIA_ZOOMLEVEL_100       1
#define AVMEDIA_ZOOMLEVEL_200       2
#define AVMEDIA_ZOOMLEVEL_FIT       3
#define AVMEDIA_ZOOMLEVEL_SCALED    4
#define AVMEDIA_ZOOMLEVEL_INVALID   65535

namespace avmedia {

enum MediaControlStyle
{
    MEDIACONTROLSTYLE_SINGLELINE = 0,
    MEDIACONTROLSTYLE_MULTILINE = 1
};

class SVX_DLLPUBLIC MediaControlBase
{
public:
    MediaControlBase(bool bIsSidebar, MediaControlStyle eControlStyle  = MEDIACONTROLSTYLE_SINGLELINE);

protected:
    VclPtr<ToolBox>     mpPlayToolBox;
    VclPtr<Slider>      mpTimeSlider;
    VclPtr<ToolBox>     mpMuteToolBox;
    VclPtr<Slider>      mpVolumeSlider;
    VclPtr<ListBox>     mpZoomListBox;
    VclPtr<Edit>        mpTimeEdit;
    MediaControlStyle   meControlStyle;
    Image GetImage( sal_Int32 nImageId ) const;
    void InitializeWidgets();
    void UpdateToolBoxes( MediaItem aMediaItem );
    void UpdateVolumeSlider( MediaItem aMediaItem );
    void UpdateTimeSlider( MediaItem aMediaItem );
    void UpdateTimeField( MediaItem aMediaItem, double fTime );
    void SelectPlayToolBoxItem( MediaItem& aExecItem, MediaItem aItem, sal_uInt16 nId);

private:
    ImageList           maImageList;
    bool                mbIsSidebar;
};

}
#endif