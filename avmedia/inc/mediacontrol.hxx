/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediacontrol.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:34:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _AVMEDIA_MEDIACONTROL_HXX
#define _AVMEDIA_MEDIACONTROL_HXX

#include "mediaitem.hxx"

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
                        ~MediaControl();

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
    Slider              maTimeSlider;;
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
