/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MediaObjectBar.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 11:46:24 $
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

#include "MediaObjectBar.hxx"
#include <avmedia/mediaitem.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <svtools/whiter.hxx>
#include <svtools/itempool.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>

#include "app.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"

using namespace sd;

#define MediaObjectBar
#include "sdslots.hxx"

namespace sd {

SFX_DECL_TYPE( 13 );

// ------------------
// - MediaObjectBar -
// ------------------

TYPEINIT1( MediaObjectBar, SfxShell );

// -----------------------------------------------------------------------------

SFX_IMPL_INTERFACE( MediaObjectBar, SfxShell, SdResId( STR_MEDIAOBJECTBARSHELL ) )
{
}

// -----------------------------------------------------------------------------

MediaObjectBar::MediaObjectBar( ViewShell* pSdViewShell, ::sd::View* pSdView ) :
    SfxShell( pSdViewShell->GetViewShell() ),
    pView( pSdView ),
    pViewSh( pSdViewShell )
{
    DrawDocShell* pDocShell = pViewSh->GetDocSh();

    SetPool( &pDocShell->GetPool() );
    SetUndoManager( pDocShell->GetUndoManager() );
    SetRepeatTarget( pView );
    SetHelpId( SD_IF_SDDRAWMEDIAOBJECTBAR );
    SetName( String( SdResId( RID_DRAW_MEDIA_TOOLBOX ) ) );
}

// -----------------------------------------------------------------------------

MediaObjectBar::~MediaObjectBar()
{
    SetRepeatTarget( NULL );
}

// -----------------------------------------------------------------------------

void MediaObjectBar::GetState( SfxItemSet& rSet )
{
    SfxWhichIter    aIter( rSet );
    USHORT          nWhich = aIter.FirstWhich();

    while( nWhich )
    {
        if( SID_AVMEDIA_TOOLBOX == nWhich )
        {
            SdrMarkList* pMarkList = new SdrMarkList( pView->GetMarkedObjectList() );
            bool         bDisable = true;

            if( 1 == pMarkList->GetMarkCount() )
            {
                SdrObject* pObj =pMarkList->GetMark( 0 )->GetMarkedSdrObj();

                if( pObj && pObj->ISA( SdrMediaObj ) )
                {
                    ::avmedia::MediaItem aItem( SID_AVMEDIA_TOOLBOX );

                    static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).updateMediaItem( aItem );
                    rSet.Put( aItem );
                    bDisable = false;
                }
            }

            if( bDisable )
                rSet.DisableItem( SID_AVMEDIA_TOOLBOX );

            delete pMarkList;
        }

        nWhich = aIter.NextWhich();
    }
}

// -----------------------------------------------------------------------------

void MediaObjectBar::Execute( SfxRequest& rReq )
{
    if( SID_AVMEDIA_TOOLBOX == rReq.GetSlot() )
    {
        const SfxItemSet*   pArgs = rReq.GetArgs();
        const SfxPoolItem*  pItem;
        USHORT              nSlot = rReq.GetSlot();
        BOOL                bGeometryChanged = FALSE;

        if( !pArgs || ( SFX_ITEM_SET != pArgs->GetItemState( SID_AVMEDIA_TOOLBOX, FALSE, &pItem ) ) )
            pItem = NULL;

        if( pItem )
        {
            SdrMarkList* pMarkList = new SdrMarkList( pView->GetMarkedObjectList() );

            if( 1 == pMarkList->GetMarkCount() )
            {
                SdrObject* pObj = pMarkList->GetMark( 0 )->GetMarkedSdrObj();

                if( pObj && pObj->ISA( SdrMediaObj ) )
                {
                    static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).executeMediaItem(
                        static_cast< const ::avmedia::MediaItem& >( *pItem ) );
                }
            }

            delete pMarkList;
        }
    }
}

} // end of namespace sd
