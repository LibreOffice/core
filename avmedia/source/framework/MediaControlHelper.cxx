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
#include <avmedia/MediaControlHelper.hxx>
#include <vcl/edit.hxx>
#include <vcl/slider.hxx>
#include <vcl/toolbox.hxx>
#include <avmedia/mediaitem.hxx>
#include <svtools/miscopt.hxx>
#include <tools/time.hxx>
#include <vcl/toolbox.hxx>
#include "mediacontrol.hrc"
#include "helpids.hrc"
#include "mediamisc.hxx"

using ::rtl::OUString;

namespace avmedia {

MediaControlHelper::MediaControlHelper():
    maImageList( SvtMiscOptions().AreCurrentSymbolsLarge() ? AVMEDIA_RESID( AVMEDIA_IMGLST_L ) : AVMEDIA_RESID( AVMEDIA_IMGLST ) )
{
}

void MediaControlHelper::UpdateTimeField(VclPtr<Edit> pTimeEdit, MediaItem aItem, double fTime)
{
    if( !aItem.getURL().isEmpty())
    {
        OUString aTimeString;

        SvtSysLocale aSysLocale;
        const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();

        aTimeString += rLocaleData.getDuration( tools::Time( 0, 0, static_cast< sal_uInt32 >( floor( fTime ) ) ) ) +
            " / " +
            rLocaleData.getDuration( tools::Time( 0, 0, static_cast< sal_uInt32 >( floor( aItem.getDuration() ) )) );

        if( pTimeEdit->GetText() != aTimeString )
            pTimeEdit->SetText( aTimeString );
    }
}

void MediaControlHelper::UpdateVolumeSlider(VclPtr<Slider> pSlider, MediaItem aItem)
{
    if( aItem.getURL().isEmpty() )
        pSlider->Disable();
    else
    {
        pSlider->Enable();
        const sal_Int32 nVolumeDB = aItem.getVolumeDB();
        pSlider->SetThumbPos( ::std::min( ::std::max( nVolumeDB, static_cast< sal_Int32 >( AVMEDIA_DB_RANGE ) ),
                                                static_cast< sal_Int32 >( 0 ) ) );
    }
}

void MediaControlHelper::UpdateTimeSlider( VclPtr<Slider> pTimeSlider, MediaItem aItem)
{
    if( aItem.getURL().isEmpty() )
        pTimeSlider->Disable();
    else
    {
        pTimeSlider->Enable();

        const double fDuration = aItem.getDuration();

        if( fDuration > 0.0 )
        {
            const double fTime = ::std::min( aItem.getTime(), fDuration );

            if( !pTimeSlider->GetLineSize() )
                pTimeSlider->SetLineSize( static_cast< sal_uInt32 >( AVMEDIA_TIME_RANGE * AVMEDIA_LINEINCREMENT / fDuration ) );

            if( !pTimeSlider->GetPageSize() )
                pTimeSlider->SetPageSize( static_cast< sal_uInt32 >( AVMEDIA_TIME_RANGE * AVMEDIA_PAGEINCREMENT / fDuration ) );

            pTimeSlider->SetThumbPos( static_cast< sal_Int32 >( fTime / fDuration * AVMEDIA_TIME_RANGE ) );
        }
    }
}

void MediaControlHelper::InitializePlayToolBox( VclPtr<ToolBox> pPlayToolBox )
{
    pPlayToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_PLAY, GetImage( AVMEDIA_IMG_PLAY ), OUString( AVMEDIA_RESID( AVMEDIA_STR_PLAY ) ), ToolBoxItemBits::CHECKABLE );
    pPlayToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_PLAY, HID_AVMEDIA_TOOLBOXITEM_PLAY );

    pPlayToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_PAUSE, GetImage( AVMEDIA_IMG_PAUSE ), OUString( AVMEDIA_RESID( AVMEDIA_STR_PAUSE ) ), ToolBoxItemBits::CHECKABLE );
    pPlayToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_PAUSE, HID_AVMEDIA_TOOLBOXITEM_PAUSE );

    pPlayToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_STOP, GetImage( AVMEDIA_IMG_STOP ), OUString( AVMEDIA_RESID( AVMEDIA_STR_STOP ) ), ToolBoxItemBits::CHECKABLE );
    pPlayToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_STOP, HID_AVMEDIA_TOOLBOXITEM_STOP );

    pPlayToolBox->InsertSeparator();

    pPlayToolBox->InsertItem( AVMEDIA_TOOLBOXITEM_LOOP, GetImage( AVMEDIA_IMG_ENDLESS ), OUString( AVMEDIA_RESID( AVMEDIA_STR_ENDLESS ) ) );
    pPlayToolBox->SetHelpId( AVMEDIA_TOOLBOXITEM_LOOP, HID_AVMEDIA_TOOLBOXITEM_LOOP );
}

Image MediaControlHelper::GetImage( sal_Int32 nImageId) const
{
    return maImageList.GetImage( static_cast< sal_uInt16 >( nImageId ) );
}

}