/*************************************************************************
 *
 *  $RCSfile: MediaObjectBar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:17:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "MediaObjectBar.hxx"
#include <avmedia/mediaitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
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
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT, SdResId( RID_DRAW_MEDIA_TOOLBOX ) );
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
                SdrObject* pObj =pMarkList->GetMark( 0 )->GetObj();

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
                SdrObject* pObj = pMarkList->GetMark( 0 )->GetObj();

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
