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

#include <avmedia/mediatoolbox.hxx>
#include <avmedia/mediaitem.hxx>
#include "mediacontrol.hxx"

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>

using namespace ::com::sun::star;

namespace avmedia
{

// -----------------------
// - MediaToolboxControl -
// -----------------------

class MediaToolBoxControl_Impl : public MediaControl
{
public:

                            MediaToolBoxControl_Impl( Window& rParent, MediaToolBoxControl& rControl );
                            ~MediaToolBoxControl_Impl();

    void                    update();
    void                    execute( const MediaItem& rItem );

private:

    MediaToolBoxControl*    mpToolBoxControl;
};

// ---------------------------------------------------------------------

MediaToolBoxControl_Impl::MediaToolBoxControl_Impl( Window& rParent, MediaToolBoxControl& rControl ) :
    MediaControl( &rParent, MEDIACONTROLSTYLE_SINGLELINE ),
    mpToolBoxControl( &rControl )
{
    SetSizePixel( getMinSizePixel() );
}

// ---------------------------------------------------------------------

MediaToolBoxControl_Impl::~MediaToolBoxControl_Impl()
{
}

// ---------------------------------------------------------------------

void MediaToolBoxControl_Impl::update()
{
    mpToolBoxControl->implUpdateMediaControl();
}

// ---------------------------------------------------------------------

void MediaToolBoxControl_Impl::execute( const MediaItem& rItem )
{
    mpToolBoxControl->implExecuteMediaControl( rItem );
}

// -----------------------
// - MediaToolBoxControl -
// -----------------------

SFX_IMPL_TOOLBOX_CONTROL( ::avmedia::MediaToolBoxControl, ::avmedia::MediaItem );

// -----------------------------------------------------------------------------

MediaToolBoxControl::MediaToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
        rTbx.Invalidate();
}

// -----------------------------------------------------------------------------

MediaToolBoxControl::~MediaToolBoxControl()
{
}

// -----------------------------------------------------------------------------

void MediaToolBoxControl::StateChanged( sal_uInt16 /* nSID */, SfxItemState eState, const SfxPoolItem* pState )

{
    MediaToolBoxControl_Impl* pCtrl = static_cast< MediaToolBoxControl_Impl* >( GetToolBox().GetItemWindow( GetId() ) );

    DBG_ASSERT( pCtrl, "MediaToolBoxControl::StateChanged: media control not found" );

    if( eState == SFX_ITEM_DISABLED )
    {
        pCtrl->Enable( false, false );
        pCtrl->SetText( String() );

        const MediaItem aEmptyMediaItem( 0, AVMEDIA_SETMASK_ALL );
        pCtrl->setState( aEmptyMediaItem );
    }
    else
    {
        pCtrl->Enable( true, false );

        const MediaItem* pMediaItem = PTR_CAST( MediaItem, pState );

        if( pMediaItem && ( SFX_ITEM_AVAILABLE == eState ) )
            pCtrl->setState( *pMediaItem );
    }
}

// -----------------------------------------------------------------------------

Window* MediaToolBoxControl::CreateItemWindow( Window *pParent )
{
    return( pParent ? new MediaToolBoxControl_Impl( *pParent, *this ) : NULL );
}

// -----------------------------------------------------------------------------

void MediaToolBoxControl::implUpdateMediaControl()
{
    updateStatus( ".uno:AVMediaToolBox" );
}

// -----------------------------------------------------------------------------

void MediaToolBoxControl::implExecuteMediaControl( const MediaItem& rItem )
{
    MediaItem                               aExecItem( SID_AVMEDIA_TOOLBOX );
    uno::Sequence< beans::PropertyValue >   aArgs( 1 );
    uno::Any                                aAny;

    aExecItem.merge( rItem );
    aExecItem.QueryValue( aAny );
    aArgs[ 0 ].Name = "AVMediaToolBox" ;
    aArgs[ 0 ].Value = aAny;

    Dispatch( ".uno:AVMediaToolBox" , aArgs );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
