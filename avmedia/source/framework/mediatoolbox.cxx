/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediatoolbox.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:39:02 $
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

#include "mediatoolbox.hxx"
#include "mediaitem.hxx"
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

MediaToolBoxControl::MediaToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
        rTbx.Invalidate();
}

// -----------------------------------------------------------------------------

MediaToolBoxControl::~MediaToolBoxControl()
{
}

// -----------------------------------------------------------------------------

void MediaToolBoxControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

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
    updateStatus( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AVMediaToolBox" ) ) );
}

// -----------------------------------------------------------------------------

void MediaToolBoxControl::implExecuteMediaControl( const MediaItem& rItem )
{
    MediaItem                               aExecItem( SID_AVMEDIA_TOOLBOX );
    uno::Sequence< beans::PropertyValue >   aArgs( 1 );
    uno::Any                                aAny;

    aExecItem.merge( rItem );
    aExecItem.QueryValue( aAny );
    aArgs[ 0 ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AVMediaToolBox" ) );
    aArgs[ 0 ].Value = aAny;

    Dispatch( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AVMediaToolBox" ) ), aArgs );
}

}
