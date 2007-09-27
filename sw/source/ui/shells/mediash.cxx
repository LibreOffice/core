/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediash.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:28:55 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"




#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SDGLUITM_HXX
#include <svx/sdgluitm.hxx>
#endif
#ifndef _SDGCOITM_HXX
#include <svx/sdgcoitm.hxx>
#endif
#ifndef _SDGGAITM_HXX
#include <svx/sdggaitm.hxx>
#endif
#ifndef _SDGTRITM_HXX
#include <svx/sdgtritm.hxx>
#endif
#ifndef _SDGINITM_HXX
#include <svx/sdginitm.hxx>
#endif
#ifndef _SDGMOITM_HXX
#include <svx/sdgmoitm.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_GRFFLT_HXX //autogen
#include <svx/grfflt.hxx>
#endif

#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _MEDIASH_HXX
#include <mediash.hxx>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif
#ifndef _FRMDLG_HXX
#include <frmdlg.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif
#ifndef _POPUP_HRC
#include <popup.hrc>
#endif

#include <sfx2/objface.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <avmedia/mediaitem.hxx>

#define SwMediaShell
#include "itemdef.hxx"
#include "swslots.hxx"
#include "swabstdlg.hxx"

SFX_IMPL_INTERFACE(SwMediaShell, SwBaseShell, SW_RES(STR_SHELLNAME_MEDIA))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_MEDIA_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_MEDIA_TOOLBOX));
}

// ------------------------------------------------------------------------------

void SwMediaShell::ExecMedia(SfxRequest &rReq)
{
    SwWrtShell* pSh = &GetShell();
    SdrView*    pSdrView = pSh->GetDrawView();

    if( pSdrView )
    {
        const SfxItemSet*   pArgs = rReq.GetArgs();
        USHORT              nSlotId = rReq.GetSlot();
        BOOL                bChanged = pSdrView->GetModel()->IsChanged();

        pSdrView->GetModel()->SetChanged( FALSE );

        switch( nSlotId )
        {
            case SID_DELETE:
            {
                if( pSh->IsObjSelected() )
                {
                    pSh->SetModified();
                    pSh->DelSelectedObj();

                    if( pSh->IsSelFrmMode() )
                        pSh->LeaveSelFrmMode();

                    GetView().AttrChangedNotify( pSh );
                }
            }
            break;

            case( SID_AVMEDIA_TOOLBOX ):
            {
                if( pSh->IsObjSelected() )
                {
                    const SfxPoolItem*  pItem;

                    if( !pArgs || ( SFX_ITEM_SET != pArgs->GetItemState( SID_AVMEDIA_TOOLBOX, FALSE, &pItem ) ) )
                        pItem = NULL;

                    if( pItem )
                    {
                        SdrMarkList* pMarkList = new SdrMarkList( pSdrView->GetMarkedObjectList() );

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
            break;

            default:
            break;
        }

        if( pSdrView->GetModel()->IsChanged() )
            GetShell().SetModified();
        else if( bChanged )
            pSdrView->GetModel()->SetChanged(TRUE);
    }
}

// ------------------------------------------------------------------------------

void SwMediaShell::GetMediaState(SfxItemSet &rSet)
{
    SfxWhichIter    aIter( rSet );
    USHORT          nWhich = aIter.FirstWhich();

    while( nWhich )
    {
        if( SID_AVMEDIA_TOOLBOX == nWhich )
        {
            SwWrtShell& rSh = GetShell();
            SdrView*    pView = rSh.GetDrawView();
            bool        bDisable = true;

            if( pView )
            {
                SdrMarkList* pMarkList = new SdrMarkList( pView->GetMarkedObjectList() );

                if( 1 == pMarkList->GetMarkCount() )
                {
                    SdrObject* pObj = pMarkList->GetMark( 0 )->GetMarkedSdrObj();

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
        }

        nWhich = aIter.NextWhich();
    }
}

// ------------------------------------------------------------------------------

SwMediaShell::SwMediaShell(SwView &_rView) :
    SwBaseShell(_rView)

{
    SetName(String::CreateFromAscii("Media Playback"));
    SetHelpId(SW_MEDIASHELL);
}
