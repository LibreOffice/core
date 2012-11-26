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

        const MediaItem* pMediaItem = dynamic_cast< const MediaItem* >( pState );

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
