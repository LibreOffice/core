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

#include "mediacontrol.hxx"
#include "mediacontrol.hrc"
#include "mediamisc.hxx"
#include <avmedia/mediawindow.hxx>
#include <avmedia/mediaplayer.hxx>
#include "helpids.hrc"
#include <tools/time.hxx>
#include <vcl/svapp.hxx>
#include <vcl/lstbox.hxx>
#include <unotools/syslocale.hxx>
#include <sfx2/viewfrm.hxx>
#include <math.h>
#include <algorithm>

#define AVMEDIA_TIMEOUT             100
#define AVMEDIA_TIME_RANGE          2048
#define AVMEDIA_DB_RANGE            -40
#define AVMEDIA_LINEINCREMENT       1.0
#define AVMEDIA_PAGEINCREMENT       10.0

#define AVMEDIA_TOOLBOXITEM_PLAY    0x0001
#define AVMEDIA_TOOLBOXITEM_PLAYFFW 0x0002
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

namespace avmedia
{

// ----------------
// - MediaControl -
// ---------------

MediaControl::MediaControl( Window* pParent, MediaControlStyle eControlStyle ) :
    Control( pParent ),
    maImageList( AVMEDIA_RESID( AVMEDIA_IMGLST ) ),
    maItem( 0, AVMEDIA_SETMASK_ALL ),
    maPlayToolBox( this, WB_3DLOOK ),
    maTimeSlider( this, WB_HORZ | WB_DRAG | WB_3DLOOK | WB_SLIDERSET ),
    maMuteToolBox( this, WB_3DLOOK ),
    maVolumeSlider( this, WB_HORZ | WB_DRAG | WB_SLIDERSET ),
    maZoomToolBox( this, WB_3DLOOK ),
    mpZoomListBox( new ListBox( &maZoomToolBox, WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL | WB_3DLOOK ) ),
    maTimeEdit( this, WB_CENTER | WB_READONLY | WB_BORDER | WB_3DLOOK | WB_READONLY ),
    meControlStyle( eControlStyle ),
    mbLocked( false )
{
    const String aTimeText( RTL_CONSTASCII_USTRINGPARAM( " 00:00:00/00:00:00 " ) );

    SetBackground();
    SetPaintTransparent( sal_True );
    SetParentClipMode( PARENTCLIPMODE_NOCLIP );

    if( MEDIACONTROLSTYLE_SINGLELINE != meControlStyle )
    {
        maPlayToolBox.InsertItem( AVMEDIA_TOOLBOXITEM_OPEN, implGetImage( AVMEDIA_IMG_OPEN ), String( AVMEDIA_RESID( AVMEDIA_STR_OPEN ) ) );
        maPlayToolBox.SetHelpId( AVMEDIA_TOOLBOXITEM_OPEN, HID_AVMEDIA_TOOLBOXITEM_OPEN );

        maPlayToolBox.InsertItem( AVMEDIA_TOOLBOXITEM_INSERT, implGetImage( AVMEDIA_IMG_INSERT ), String( AVMEDIA_RESID( AVMEDIA_STR_INSERT ) ) );
        maPlayToolBox.SetHelpId( AVMEDIA_TOOLBOXITEM_INSERT, HID_AVMEDIA_TOOLBOXITEM_INSERT );

        maPlayToolBox.InsertSeparator();
    }
    else
    {
        mpZoomListBox->SetBackground();

        maZoomToolBox.SetBackground();
        maZoomToolBox.SetPaintTransparent( sal_True );
        maPlayToolBox.SetBackground();
        maPlayToolBox.SetPaintTransparent( sal_True );
        maMuteToolBox.SetBackground();
        maMuteToolBox.SetPaintTransparent( sal_True );

    }

    maPlayToolBox.InsertItem( AVMEDIA_TOOLBOXITEM_PLAY, implGetImage( AVMEDIA_IMG_PLAY ), String( AVMEDIA_RESID( AVMEDIA_STR_PLAY ) ), TIB_CHECKABLE );
       maPlayToolBox.SetHelpId( AVMEDIA_TOOLBOXITEM_PLAY, HID_AVMEDIA_TOOLBOXITEM_PLAY );

    maPlayToolBox.InsertItem( AVMEDIA_TOOLBOXITEM_PAUSE, implGetImage( AVMEDIA_IMG_PAUSE ), String( AVMEDIA_RESID( AVMEDIA_STR_PAUSE ) ), TIB_CHECKABLE );
       maPlayToolBox.SetHelpId( AVMEDIA_TOOLBOXITEM_PAUSE, HID_AVMEDIA_TOOLBOXITEM_PAUSE );

    maPlayToolBox.InsertItem( AVMEDIA_TOOLBOXITEM_STOP, implGetImage( AVMEDIA_IMG_STOP ), String( AVMEDIA_RESID( AVMEDIA_STR_STOP ) ), TIB_CHECKABLE );
       maPlayToolBox.SetHelpId( AVMEDIA_TOOLBOXITEM_STOP, HID_AVMEDIA_TOOLBOXITEM_STOP );

    maPlayToolBox.InsertSeparator();

    maPlayToolBox.InsertItem( AVMEDIA_TOOLBOXITEM_LOOP, implGetImage( AVMEDIA_IMG_ENDLESS ), String( AVMEDIA_RESID( AVMEDIA_STR_ENDLESS ) ) );
       maPlayToolBox.SetHelpId( AVMEDIA_TOOLBOXITEM_LOOP, HID_AVMEDIA_TOOLBOXITEM_LOOP );

    if( MEDIACONTROLSTYLE_SINGLELINE == meControlStyle )
        maPlayToolBox.InsertSeparator();

    maPlayToolBox.SetSelectHdl( LINK( this, MediaControl, implSelectHdl ) );
    maPlayToolBox.SetSizePixel( maPlayToolBox.CalcWindowSizePixel() );
    maPlayToolBox.Show();
    maMinSize = maPlayToolBox.GetSizePixel();

    maTimeSlider.SetSlideHdl( LINK( this, MediaControl, implTimeHdl ) );
    maTimeSlider.SetEndSlideHdl( LINK( this, MediaControl, implTimeEndHdl ) );
    maTimeSlider.SetRange( Range( 0, AVMEDIA_TIME_RANGE ) );
    maTimeSlider.SetHelpId( HID_AVMEDIA_TIMESLIDER );
    maTimeSlider.SetUpdateMode( true );
    maTimeSlider.SetSizePixel( Size( 128, maPlayToolBox.GetSizePixel().Height() ) );
    maTimeSlider.Show();
    maMinSize.Width() += maTimeSlider.GetSizePixel().Width();

    maTimeEdit.SetText( aTimeText );
    maTimeEdit.SetUpdateMode( true );
    maTimeEdit.SetSizePixel( Size( maTimeEdit.GetTextWidth( aTimeText ) + 8, maPlayToolBox.GetSizePixel().Height() ) );
    maTimeEdit.SetControlBackground( Application::GetSettings().GetStyleSettings().GetWindowColor() );
    maTimeEdit.SetHelpId( HID_AVMEDIA_TIMEEDIT );
    maTimeEdit.Disable();
    maTimeEdit.Show();
    maMinSize.Width() += maTimeEdit.GetSizePixel().Width();

    if( MEDIACONTROLSTYLE_SINGLELINE == meControlStyle )
        maMuteToolBox.InsertSeparator();

    maMuteToolBox.InsertItem( AVMEDIA_TOOLBOXITEM_MUTE, implGetImage( AVMEDIA_IMG_MUTE ), String( AVMEDIA_RESID( AVMEDIA_STR_MUTE ) ) );
       maMuteToolBox.SetHelpId( AVMEDIA_TOOLBOXITEM_MUTE, HID_AVMEDIA_TOOLBOXITEM_MUTE );

    maMuteToolBox.SetSelectHdl( LINK( this, MediaControl, implSelectHdl ) );
    maMuteToolBox.SetSizePixel( maMuteToolBox.CalcWindowSizePixel() );
    maMuteToolBox.Show();
    maMinSize.Width() += maMuteToolBox.GetSizePixel().Width();

    maVolumeSlider.SetSlideHdl( LINK( this, MediaControl, implVolumeHdl ) );
    maVolumeSlider.SetEndSlideHdl( LINK( this, MediaControl, implVolumeEndHdl ) );
    maVolumeSlider.SetRange( Range( AVMEDIA_DB_RANGE, 0 ) );
    maVolumeSlider.SetUpdateMode( true );
    maVolumeSlider.SetHelpId( HID_AVMEDIA_VOLUMESLIDER );
    maVolumeSlider.SetSizePixel( Size( 48, maPlayToolBox.GetSizePixel().Height() ) );
    maVolumeSlider.Show();
    maMinSize.Width() += maVolumeSlider.GetSizePixel().Width();

    mpZoomListBox->SetSizePixel( Size( maTimeEdit.GetSizePixel().Width(), 260 ) );
    mpZoomListBox->InsertEntry( String( AVMEDIA_RESID( AVMEDIA_STR_ZOOM_50 ) ), AVMEDIA_ZOOMLEVEL_50 );
    mpZoomListBox->InsertEntry( String( AVMEDIA_RESID( AVMEDIA_STR_ZOOM_100 ) ), AVMEDIA_ZOOMLEVEL_100 );
    mpZoomListBox->InsertEntry( String( AVMEDIA_RESID( AVMEDIA_STR_ZOOM_200 ) ), AVMEDIA_ZOOMLEVEL_200 );
    mpZoomListBox->InsertEntry( String( AVMEDIA_RESID( AVMEDIA_STR_ZOOM_FIT ) ), AVMEDIA_ZOOMLEVEL_FIT );
    mpZoomListBox->SetSelectHdl( LINK( this, MediaControl, implZoomSelectHdl ) );
    mpZoomListBox->SetHelpId( HID_AVMEDIA_ZOOMLISTBOX );

    maZoomToolBox.InsertItem( AVMEDIA_TOOLBOXITEM_ZOOM, String( AVMEDIA_RESID( AVMEDIA_STR_ZOOM ) ) );
    maZoomToolBox.SetHelpId( AVMEDIA_TOOLBOXITEM_ZOOM, HID_AVMEDIA_ZOOMLISTBOX );

    maZoomToolBox.SetItemWindow( AVMEDIA_TOOLBOXITEM_ZOOM, mpZoomListBox );
    maZoomToolBox.SetSelectHdl( LINK( this, MediaControl, implSelectHdl ) );
    maZoomToolBox.SetSizePixel( maZoomToolBox.CalcWindowSizePixel() );
    maZoomToolBox.Show();
    maMinSize.Width() += maZoomToolBox.GetSizePixel().Width();

    if( MEDIACONTROLSTYLE_MULTILINE == meControlStyle )
    {
        maMinSize.Width() = 256;
        maMinSize.Height() = ( maMinSize.Height() << 1 ) + AVMEDIA_CONTROLOFFSET;
    }

    maTimer.SetTimeout( AVMEDIA_TIMEOUT );
    maTimer.SetTimeoutHdl( LINK( this, MediaControl, implTimeoutHdl ) );
    maTimer.Start();
}

// ------------------------------------------------------------------------------

MediaControl::~MediaControl()
{
    maZoomToolBox.SetItemWindow( AVMEDIA_TOOLBOXITEM_ZOOM, NULL );
    delete mpZoomListBox;
}

// ------------------------------------------------------------------------------

const Size& MediaControl::getMinSizePixel() const
{
    return maMinSize;
}

// ------------------------------------------------------------------------------

void MediaControl::Resize()
{
    Point           aPos( 0, 0 );
    const sal_Int32 nPlayToolBoxWidth = maPlayToolBox.GetSizePixel().Width();
    const sal_Int32 nMuteToolBoxWidth = maMuteToolBox.GetSizePixel().Width();
    const sal_Int32 nVolumeSliderWidth = maVolumeSlider.GetSizePixel().Width();
    const sal_Int32 nZoomToolBoxWidth = maZoomToolBox.GetSizePixel().Width();
    const sal_Int32 nTimeEditWidth = maTimeEdit.GetSizePixel().Width();
    const sal_Int32 nTimeSliderHeight = maTimeSlider.GetSizePixel().Height();

    if( MEDIACONTROLSTYLE_SINGLELINE == meControlStyle )
    {
        const sal_Int32 nTimeSliderWidth = GetSizePixel().Width() - ( AVMEDIA_CONTROLOFFSET * 3 ) -
                                           nPlayToolBoxWidth - nMuteToolBoxWidth - nVolumeSliderWidth - nTimeEditWidth - nZoomToolBoxWidth;

        maPlayToolBox.SetPosSizePixel( aPos, maPlayToolBox.GetSizePixel() );

        aPos.X() += nPlayToolBoxWidth;
        maTimeSlider.SetPosSizePixel( aPos, Size( nTimeSliderWidth, nTimeSliderHeight ) );

        aPos.X() += nTimeSliderWidth + AVMEDIA_CONTROLOFFSET;
        maTimeEdit.SetPosSizePixel( aPos, maTimeEdit.GetSizePixel() );

        aPos.X() += nTimeEditWidth + AVMEDIA_CONTROLOFFSET;
        maMuteToolBox.SetPosSizePixel( aPos, maMuteToolBox.GetSizePixel() );

        aPos.X() += nMuteToolBoxWidth;
        maVolumeSlider.SetPosSizePixel( aPos, maVolumeSlider.GetSizePixel() );

        aPos.X() += nVolumeSliderWidth + AVMEDIA_CONTROLOFFSET;
        maZoomToolBox.SetPosSizePixel( aPos, maZoomToolBox.GetSizePixel() );
    }
    else
    {
        const sal_Int32 nTimeSliderWidth = GetSizePixel().Width() - AVMEDIA_CONTROLOFFSET - nTimeEditWidth;

        maTimeSlider.SetPosSizePixel( aPos, Size( nTimeSliderWidth, nTimeSliderHeight ) );

        aPos.X() += nTimeSliderWidth + AVMEDIA_CONTROLOFFSET;
        maTimeEdit.SetPosSizePixel( aPos, maTimeEdit.GetSizePixel() );

        aPos.X() = 0;
        aPos.Y() += nTimeSliderHeight + AVMEDIA_CONTROLOFFSET;
        maPlayToolBox.SetPosSizePixel( aPos, maPlayToolBox.GetSizePixel() );

        aPos.X() = GetSizePixel().Width() - nVolumeSliderWidth - nMuteToolBoxWidth - nZoomToolBoxWidth - AVMEDIA_CONTROLOFFSET;
        maMuteToolBox.SetPosSizePixel( aPos, maMuteToolBox.GetSizePixel() );

        aPos.X() += nMuteToolBoxWidth;
        maVolumeSlider.SetPosSizePixel( aPos, maVolumeSlider.GetSizePixel() );

        aPos.X() = GetSizePixel().Width() - nZoomToolBoxWidth;
        maZoomToolBox.SetPosSizePixel( aPos, maZoomToolBox.GetSizePixel() );
    }
}

// ------------------------------------------------------------------------------

void MediaControl::setState( const MediaItem& rItem )
{
    if( !mbLocked )
    {
        maItem.merge( rItem );

        implUpdateToolboxes();
        implUpdateTimeSlider();
        implUpdateVolumeSlider();
        implUpdateTimeField( maItem.getTime() );
    }
}

// ------------------------------------------------------------------------------

void MediaControl::implUpdateToolboxes()
{
    const bool bValidURL = ( !maItem.getURL().isEmpty());

    maPlayToolBox.EnableItem( AVMEDIA_TOOLBOXITEM_INSERT, bValidURL );
    maPlayToolBox.EnableItem( AVMEDIA_TOOLBOXITEM_PLAY, bValidURL );
    maPlayToolBox.EnableItem( AVMEDIA_TOOLBOXITEM_PLAYFFW, bValidURL );
    maPlayToolBox.EnableItem( AVMEDIA_TOOLBOXITEM_PAUSE, bValidURL );
    maPlayToolBox.EnableItem( AVMEDIA_TOOLBOXITEM_STOP, bValidURL );
    maPlayToolBox.EnableItem( AVMEDIA_TOOLBOXITEM_LOOP, bValidURL );
    maMuteToolBox.EnableItem( AVMEDIA_TOOLBOXITEM_MUTE, bValidURL );

    if( !bValidURL || !IsEnabled() )
    {
        mpZoomListBox->Disable();

        if( MEDIACONTROLSTYLE_SINGLELINE == meControlStyle )
            maPlayToolBox.Disable();

        maMuteToolBox.Disable();
    }
    else
    {
        maPlayToolBox.Enable();
        maMuteToolBox.Enable();

        if( MEDIASTATE_PLAY == maItem.getState() || MEDIASTATE_PLAYFFW == maItem.getState() )
        {
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_PLAY, true );
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_PLAYFFW, MEDIASTATE_PLAYFFW == maItem.getState() );
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_PAUSE, false );
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_STOP, false );
        }
        else if( maItem.getTime() > 0.0 && ( maItem.getTime() < maItem.getDuration() ) )
        {
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_PLAY, false );
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_PLAYFFW, false );
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_PAUSE, true );
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_STOP, false );
        }
        else
        {
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_PLAY, false );
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_PLAYFFW, false );
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_PAUSE, false );
            maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_STOP, true );
        }

        maPlayToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_LOOP, maItem.isLoop() );
        maMuteToolBox.CheckItem( AVMEDIA_TOOLBOXITEM_MUTE, maItem.isMute() );

        if( !mpZoomListBox->IsTravelSelect() && !mpZoomListBox->IsInDropDown() )
        {
            sal_uInt16 nSelectEntryPos ;

            switch( maItem.getZoom() )
            {
                case( ::com::sun::star::media::ZoomLevel_ZOOM_1_TO_2 ): nSelectEntryPos = AVMEDIA_ZOOMLEVEL_50; break;
                case( ::com::sun::star::media::ZoomLevel_ORIGINAL ): nSelectEntryPos = AVMEDIA_ZOOMLEVEL_100; break;
                case( ::com::sun::star::media::ZoomLevel_ZOOM_2_TO_1 ): nSelectEntryPos = AVMEDIA_ZOOMLEVEL_200; break;
                case( ::com::sun::star::media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT ): nSelectEntryPos = AVMEDIA_ZOOMLEVEL_FIT; break;
                case( ::com::sun::star::media::ZoomLevel_FIT_TO_WINDOW ): nSelectEntryPos = AVMEDIA_ZOOMLEVEL_SCALED; break;

                default: nSelectEntryPos = AVMEDIA_ZOOMLEVEL_INVALID; break;
            }

            if( nSelectEntryPos != AVMEDIA_ZOOMLEVEL_INVALID )
            {
                mpZoomListBox->Enable();
                mpZoomListBox->SelectEntryPos( nSelectEntryPos );
            }
            else
                mpZoomListBox->Disable();
        }
    }
}

// ------------------------------------------------------------------------------

void MediaControl::implUpdateTimeSlider()
{
    if( maItem.getURL().isEmpty() || !IsEnabled() )
        maTimeSlider.Disable();
    else
    {
        maTimeSlider.Enable();

        const double fDuration = maItem.getDuration();

        if( fDuration > 0.0 )
        {
            const double fTime = ::std::min( maItem.getTime(), fDuration );

            if( !maTimeSlider.GetLineSize() )
                maTimeSlider.SetLineSize( static_cast< sal_uInt32 >( AVMEDIA_TIME_RANGE * AVMEDIA_LINEINCREMENT / fDuration ) );

            if( !maTimeSlider.GetPageSize() )
                maTimeSlider.SetPageSize( static_cast< sal_uInt32 >( AVMEDIA_TIME_RANGE * AVMEDIA_PAGEINCREMENT / fDuration ) );

            maTimeSlider.SetThumbPos( static_cast< sal_Int32 >( fTime / fDuration * AVMEDIA_TIME_RANGE ) );
        }
    }
}

// ------------------------------------------------------------------------------

void MediaControl::implUpdateVolumeSlider()
{
    if( maItem.getURL().isEmpty() || !IsEnabled() )
        maVolumeSlider.Disable();
    else
    {
        maVolumeSlider.Enable();

        const sal_Int32 nVolumeDB = maItem.getVolumeDB();

        maVolumeSlider.SetThumbPos( ::std::min( ::std::max( nVolumeDB, static_cast< sal_Int32 >( AVMEDIA_DB_RANGE ) ),
                                                static_cast< sal_Int32 >( 0 ) ) );
    }
}

// ------------------------------------------------------------------------------

void MediaControl::implUpdateTimeField( double fCurTime )
{
    if( !maItem.getURL().isEmpty() )
    {
        String              aTimeString;

        SvtSysLocale aSysLocale;
        const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();

        aTimeString += rLocaleData.getDuration( Time( 0, 0, static_cast< sal_uInt32 >( floor( fCurTime ) ) ) );
        aTimeString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " / " ));
        aTimeString += rLocaleData.getDuration( Time( 0, 0, static_cast< sal_uInt32 >( floor( maItem.getDuration() ) )) );

        if( maTimeEdit.GetText() != aTimeString )
            maTimeEdit.SetText( aTimeString );
    }
}

// ------------------------------------------------------------------------------

Image MediaControl::implGetImage( sal_Int32 nImageId ) const
{
    return maImageList.GetImage( static_cast< sal_uInt16 >( nImageId ) );
}

// ------------------------------------------------------------------------------

IMPL_LINK( MediaControl, implTimeHdl, Slider*, p )
{
    mbLocked = true;
    maTimer.Stop();
    implUpdateTimeField( p->GetThumbPos() * maItem.getDuration() / AVMEDIA_TIME_RANGE );

    return 0;
}

// ------------------------------------------------------------------------------

IMPL_LINK( MediaControl, implTimeEndHdl, Slider*, p )
{
    MediaItem aExecItem;

    aExecItem.setTime( p->GetThumbPos() * maItem.getDuration() / AVMEDIA_TIME_RANGE );
    execute( aExecItem );
    update();
    maTimer.Start();
    mbLocked = false;

    return 0;
}

// ------------------------------------------------------------------------------

IMPL_LINK( MediaControl, implVolumeHdl, Slider*, p )
{
    MediaItem aExecItem;

    aExecItem.setVolumeDB( static_cast< sal_Int16 >( p->GetThumbPos() ) );
    execute( aExecItem );
    update();

    return 0;
}

// ------------------------------------------------------------------------------

IMPL_LINK_NOARG(MediaControl, implVolumeEndHdl)
{
    return 0;
}

// ------------------------------------------------------------------------------

IMPL_LINK( MediaControl, implSelectHdl, ToolBox*, p )
{
    if( p )
    {
        MediaItem aExecItem;

        switch( p->GetCurItemId() )
        {
            case( AVMEDIA_TOOLBOXITEM_OPEN ):
            {
                OUString aURL;

                 if (::avmedia::MediaWindow::executeMediaURLDialog(
                             GetParent(), aURL, 0))
                 {
                     if( !::avmedia::MediaWindow::isMediaURL( aURL, true ) )
                        ::avmedia::MediaWindow::executeFormatErrorBox( this );
                    else
                    {
                        aExecItem.setURL( aURL, 0 );
                        aExecItem.setState( MEDIASTATE_PLAY );
                    }
                }
            }
            break;

            case( AVMEDIA_TOOLBOXITEM_INSERT ):
            {
                MediaFloater* pFloater = AVMEDIA_MEDIAWINDOW();

                if( pFloater )
                    pFloater->dispatchCurrentURL();
            }
            break;

            case( AVMEDIA_TOOLBOXITEM_PLAY ):
            case( AVMEDIA_TOOLBOXITEM_PLAYFFW ):
            {
                aExecItem.setState( ( AVMEDIA_TOOLBOXITEM_PLAYFFW == p->GetCurItemId() ) ? MEDIASTATE_PLAYFFW : MEDIASTATE_PLAY );

                if( maItem.getTime() == maItem.getDuration() )
                    aExecItem.setTime( 0.0 );
                else
                    aExecItem.setTime( maItem.getTime() );
            }
            break;

            case( AVMEDIA_TOOLBOXITEM_PAUSE ):
            {
                aExecItem.setState( MEDIASTATE_PAUSE );
            }
            break;

            case( AVMEDIA_TOOLBOXITEM_STOP ):
            {
                aExecItem.setState( MEDIASTATE_STOP );
                aExecItem.setTime( 0.0 );
            }
            break;

            case( AVMEDIA_TOOLBOXITEM_MUTE ):
            {
                aExecItem.setMute( !maMuteToolBox.IsItemChecked( AVMEDIA_TOOLBOXITEM_MUTE ) );
            }
            break;

            case( AVMEDIA_TOOLBOXITEM_LOOP ):
            {
                aExecItem.setLoop( !maPlayToolBox.IsItemChecked( AVMEDIA_TOOLBOXITEM_LOOP ) );
            }
            break;

            default:
            break;
        }

        if( aExecItem.getMaskSet() != AVMEDIA_SETMASK_NONE )
            execute( aExecItem );
    }

    update();
    p->Invalidate( INVALIDATE_UPDATE );

    return 0;
}

// ------------------------------------------------------------------------------

IMPL_LINK( MediaControl, implZoomSelectHdl, ListBox*, p )
{
    if( p )
    {
        MediaItem aExecItem;
        ::com::sun::star::media::ZoomLevel eLevel;

        switch( p->GetSelectEntryPos() )
        {
            case( AVMEDIA_ZOOMLEVEL_50 ): eLevel = ::com::sun::star::media::ZoomLevel_ZOOM_1_TO_2; break;
            case( AVMEDIA_ZOOMLEVEL_100 ): eLevel = ::com::sun::star::media::ZoomLevel_ORIGINAL; break;
            case( AVMEDIA_ZOOMLEVEL_200 ): eLevel = ::com::sun::star::media::ZoomLevel_ZOOM_2_TO_1; break;
            case( AVMEDIA_ZOOMLEVEL_FIT ): eLevel = ::com::sun::star::media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT; break;
            case( AVMEDIA_ZOOMLEVEL_SCALED ): eLevel = ::com::sun::star::media::ZoomLevel_FIT_TO_WINDOW; break;

            default: eLevel = ::com::sun::star::media::ZoomLevel_NOT_AVAILABLE; break;
        }

        aExecItem.setZoom( eLevel );
        execute( aExecItem );
        update();
    }

    return 0;
}

// ------------------------------------------------------------------------------

IMPL_LINK_NOARG(MediaControl, implTimeoutHdl)
{
    update();
    maTimer.Start();

    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
