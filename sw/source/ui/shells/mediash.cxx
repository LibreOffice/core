/*************************************************************************
 *
 *  $RCSfile: mediash.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 10:15:38 $
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


#pragma hdrstop


#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
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

#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <avmedia/mediaitem.hxx>

#define SwMediaShell
#include "itemdef.hxx"
#include "swslots.hxx"
#include "swabstdlg.hxx" //CHINA001

SFX_IMPL_INTERFACE(SwMediaShell, SwBaseShell, SW_RES(STR_SHELLNAME_MEDIA))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_MEDIA_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_MEDIA_TOOLBOX));
    //SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_GRAFIK));
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
            case FN_BACKSPACE:
            {
                if( pSh->IsObjSelected() )
                {
                    pSh->DelSelectedObj();

                    if( pSh->IsSelFrmMode() )
                    {
                        pSh->LeaveSelFrmMode();
                        pSh->NoEdit();
                    }

                    GetView().AttrChangedNotify( pSh );
                }
            }
            break;

            case( SID_AVMEDIA_TOOLBOX ):
            {
                if( pSh->IsObjSelected() )
                {
                    const SfxPoolItem*  pItem;
                    USHORT              nSlot = rReq.GetSlot();

                    if( !pArgs || ( SFX_ITEM_SET != pArgs->GetItemState( SID_AVMEDIA_TOOLBOX, FALSE, &pItem ) ) )
                        pItem = NULL;

                    if( pItem )
                    {
                        SdrMarkList* pMarkList = new SdrMarkList( pSdrView->GetMarkedObjectList() );

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
                    SdrObject* pObj = pMarkList->GetMark( 0 )->GetObj();

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

SwMediaShell::SwMediaShell(SwView &rView) :
    SwBaseShell(rView)

{
    SetName(String::CreateFromAscii("Media Playback"));
    SetHelpId(SW_MEDIASHELL);
}
