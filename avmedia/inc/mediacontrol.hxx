/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
