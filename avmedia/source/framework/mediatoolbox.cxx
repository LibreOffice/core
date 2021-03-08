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

#include <avmedia/mediatoolbox.hxx>
#include <avmedia/mediaitem.hxx>
#include <mediacontrol.hxx>

#include <tools/debug.hxx>
#include <sfx2/sfxsids.hrc>
#include <vcl/toolbox.hxx>

#include <comphelper/propertysequence.hxx>

using namespace ::com::sun::star;

namespace avmedia
{

class MediaToolBoxControl_Impl : public MediaControl
{
public:

                            MediaToolBoxControl_Impl( vcl::Window& rParent, MediaToolBoxControl& rControl );

    void                    update() override;
    void                    execute( const MediaItem& rItem ) override;

private:

    MediaToolBoxControl*    mpToolBoxControl;
};

MediaToolBoxControl_Impl::MediaToolBoxControl_Impl( vcl::Window& rParent, MediaToolBoxControl& rControl ) :
    MediaControl( &rParent, MediaControlStyle::SingleLine ),
    mpToolBoxControl( &rControl )
{
    SetSizePixel(m_xContainer->get_preferred_size());
}

void MediaToolBoxControl_Impl::update()
{
    mpToolBoxControl->implUpdateMediaControl();
}


void MediaToolBoxControl_Impl::execute( const MediaItem& rItem )
{
    mpToolBoxControl->implExecuteMediaControl( rItem );
}


SFX_IMPL_TOOLBOX_CONTROL( MediaToolBoxControl, ::avmedia::MediaItem );


MediaToolBoxControl::MediaToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
        rTbx.Invalidate();
}


MediaToolBoxControl::~MediaToolBoxControl()
{
}


void MediaToolBoxControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )
{
    MediaToolBoxControl_Impl* pCtrl = static_cast< MediaToolBoxControl_Impl* >( GetToolBox().GetItemWindow( GetId() ) );

    DBG_ASSERT( pCtrl, "MediaToolBoxControl::StateChanged: media control not found" );

    if( eState == SfxItemState::DISABLED )
    {
        pCtrl->Enable( false, false );
        pCtrl->SetText( OUString() );

        const MediaItem aEmptyMediaItem( 0, AVMediaSetMask::ALL );
        pCtrl->setState( aEmptyMediaItem );
    }
    else
    {
        pCtrl->Enable( true, false );

        const MediaItem* pMediaItem = dynamic_cast<const MediaItem*>( pState  );

        if( pMediaItem && ( eState == SfxItemState::DEFAULT ) )
            pCtrl->setState( *pMediaItem );
    }
}

VclPtr<InterimItemWindow> MediaToolBoxControl::CreateItemWindow( vcl::Window *pParent )
{
    return ( pParent ? VclPtr<MediaToolBoxControl_Impl>::Create( *pParent, *this ) : nullptr );
}

void MediaToolBoxControl::implUpdateMediaControl()
{
    updateStatus( ".uno:AVMediaToolBox" );
}

void MediaToolBoxControl::implExecuteMediaControl( const MediaItem& rItem )
{
    MediaItem                               aExecItem( SID_AVMEDIA_TOOLBOX );
    uno::Any                                aAny;

    aExecItem.merge( rItem );
    aExecItem.QueryValue( aAny );
    auto aArgs(::comphelper::InitPropertySequence({
        { "AVMediaToolBox", aAny }
    }));

    Dispatch( ".uno:AVMediaToolBox" , aArgs );
}

} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
