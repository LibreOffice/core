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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"




#ifndef _CMDID_H
#include <cmdid.h>
#endif
#include <hintids.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/protitem.hxx>
#include <sfx2/request.hxx>
#include <svl/srchitem.hxx>
#include <svx/htmlmode.hxx>
#include <svx/sdgluitm.hxx>
#include <svx/sdgcoitm.hxx>
#include <svx/sdggaitm.hxx>
#include <svx/sdgtritm.hxx>
#include <svx/sdginitm.hxx>
#include <svx/sdgmoitm.hxx>
#include <editeng/brshitem.hxx>
#include <svx/grfflt.hxx>
#include <fmturl.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <swmodule.hxx>
#include <frmatr.hxx>
#include <swundo.hxx>
#include <uitool.hxx>
#include <docsh.hxx>
#include <mediash.hxx>
#include <frmmgr.hxx>
#include <frmdlg.hxx>
#include <frmfmt.hxx>
#include <grfatr.hxx>
#include <usrpref.hxx>
#include <edtwin.hxx>
#include <swwait.hxx>
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
#include <sfx2/msg.hxx>
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
        sal_uInt16              nSlotId = rReq.GetSlot();
        sal_Bool                bChanged = pSdrView->GetModel()->IsChanged();

        pSdrView->GetModel()->SetChanged( sal_False );

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

                    if( !pArgs || ( SFX_ITEM_SET != pArgs->GetItemState( SID_AVMEDIA_TOOLBOX, sal_False, &pItem ) ) )
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
            pSdrView->GetModel()->SetChanged(sal_True);
    }
}

// ------------------------------------------------------------------------------

void SwMediaShell::GetMediaState(SfxItemSet &rSet)
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16          nWhich = aIter.FirstWhich();

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
