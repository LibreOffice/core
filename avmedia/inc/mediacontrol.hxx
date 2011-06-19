/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _AVMEDIA_MEDIACONTROL_HXX
#define _AVMEDIA_MEDIACONTROL_HXX

#include <avmedia/mediaitem.hxx>

#include <vcl/timer.hxx>
#include <vcl/slider.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/image.hxx>

#define AVMEDIA_CONTROLOFFSET 6

class ListBox;

namespace avmedia
{

// ---------------------
// - MediaControlStyle -
// ---------------------

enum MediaControlStyle
{
    MEDIACONTROLSTYLE_SINGLELINE = 0,
    MEDIACONTROLSTYLE_MULTILINE = 1
};

// ----------------
// - MediaControl -
// ---------------

class MediaItem;

class MediaControl : public Control
{
public:

                        MediaControl( Window* pParent, MediaControlStyle eControlStyle );
    virtual             ~MediaControl();

    const Size&         getMinSizePixel() const;

    void                setState( const MediaItem& rItem );
    void                getState( MediaItem& rItem ) const;

protected:

    virtual void        update() = 0;
    virtual void        execute( const MediaItem& rItem ) = 0;

    virtual void        Resize();

private:

    void                implUpdateToolboxes();
    void                implUpdateTimeSlider();
    void                implUpdateVolumeSlider();
    void                implUpdateTimeField( double fCurTime );
    Image               implGetImage( sal_Int32 nImageId ) const;

                        DECL_LINK( implTimeHdl, Slider* );
                        DECL_LINK( implTimeEndHdl, Slider* );
                        DECL_LINK( implVolumeHdl, Slider* );
                        DECL_LINK( implVolumeEndHdl, Slider* );
                        DECL_LINK( implSelectHdl, ToolBox* );
                        DECL_LINK( implZoomSelectHdl, ListBox* );
                        DECL_LINK( implTimeoutHdl, Timer* );

    ImageList           maImageList;
    Timer               maTimer;
    MediaItem           maItem;
    ToolBox             maPlayToolBox;
    Slider              maTimeSlider;
    ToolBox             maMuteToolBox;
    Slider              maVolumeSlider;
    ToolBox             maZoomToolBox;
    ListBox*            mpZoomListBox;
    Edit                maTimeEdit;
    Size                maMinSize;
    MediaControlStyle   meControlStyle;
    bool                mbLocked;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
