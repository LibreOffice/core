/*************************************************************************
 *
 *  $RCSfile: view1.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 09:59:15 $
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

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _IDXMRK_HXX
#include <idxmrk.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _GLOBDOC_HXX
#include <globdoc.hxx>
#endif
#ifndef _NAVIPI_HXX
#include <navipi.hxx>
#endif
#ifndef _FLDWRAP_HXX
#include <fldwrap.hxx>
#endif
#ifndef _REDLNDLG_HXX
#include <redlndlg.hxx>
#endif
#ifndef _DPAGE_HXX
#include <dpage.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _SWFORMATCLIPBOARD_HXX
#include "formatclipboard.hxx"
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
// header for class SfxRequest
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif

extern int bDocSzUpdated;


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwView::Activate(BOOL bMDIActivate)
{
    // aktuelle View anmelden an der DocShell
    // die View bleibt solange an der DocShell
    // aktiv bis Sie zerstoert wird oder durch Activate eine
    // neue gesetzt wird
    SwDocShell* pDocSh = GetDocShell();
    if(pDocSh)
        pDocSh->SetView(this);
    SwModule* pSwMod = SW_MOD();
    pSwMod->SetView(this);

    // Dokumentgroesse hat sich geaendert
    if(!bDocSzUpdated)
        DocSzChgd(aDocSz);

    pHRuler->SetActive( TRUE );
    pVRuler->SetActive( TRUE );

    if ( bMDIActivate )
    {
        pWrtShell->ShGetFcs(FALSE);     // Selektionen sichtbar

        if( sSwViewData.Len() )
        {
            ReadUserData( sSwViewData, FALSE );
            sSwViewData.Erase();
        }

        AttrChangedNotify(pWrtShell);

        // Flddlg ggf neu initialisieren (z.B. fuer TYP_SETVAR)
        USHORT nId = SwFldDlgWrapper::GetChildWindowId();
        SfxViewFrame* pVFrame = GetViewFrame();
        SwFldDlgWrapper *pWrp = (SwFldDlgWrapper*)pVFrame->GetChildWindow(nId);
        if (pWrp)
            pWrp->ReInitDlg(GetDocShell());

        // RedlineDlg ggf neu initialisieren
        nId = SwRedlineAcceptChild::GetChildWindowId();
        SwRedlineAcceptChild *pRed = (SwRedlineAcceptChild*)pVFrame->GetChildWindow(nId);
        if (pRed)
            pRed->ReInitDlg(GetDocShell());

        // reinit IdxMarkDlg
        nId = SwInsertIdxMarkWrapper::GetChildWindowId();
        SwInsertIdxMarkWrapper *pIdxMrk = (SwInsertIdxMarkWrapper*)pVFrame->GetChildWindow(nId);
        if (pIdxMrk)
            pIdxMrk->ReInitDlg(*pWrtShell);

        // reinit AuthMarkDlg
        nId = SwInsertAuthMarkWrapper::GetChildWindowId();
        SwInsertAuthMarkWrapper *pAuthMrk = (SwInsertAuthMarkWrapper*)pVFrame->
                                                                GetChildWindow(nId);
        if (pAuthMrk)
            pAuthMrk->ReInitDlg(*pWrtShell);
    }
    else
        //Wenigstens das Notify rufen (vorsichtshalber wegen der SlotFilter
        AttrChangedNotify(pWrtShell);

    SfxViewShell::Activate(bMDIActivate);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwView::Deactivate(BOOL bMDIActivate)
{
    extern BOOL bFlushCharBuffer ;
        // Befinden sich noch Zeichen im Input Buffer?
    if( bFlushCharBuffer )
        GetEditWin().FlushInBuffer();

    if( bMDIActivate )
    {
        pWrtShell->ShLooseFcs();    // Selektionen unsichtbar

        pHRuler->SetActive( FALSE );
        pVRuler->SetActive( FALSE );
    }
    SfxViewShell::Deactivate(bMDIActivate);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwView::MarginChanged()
{
    GetWrtShell().SetBrowseBorder( GetMargin() );
}

/*--------------------------------------------------------------------
 --------------------------------------------------------------------*/

void SwView::ExecFormatPaintbrush(SfxRequest& rReq)
{
    if(!pFormatClipboard)
        return;

    if( pFormatClipboard->HasContent() )
    {
        pFormatClipboard->Erase();

        SwApplyTemplate aTemplate;
        GetEditWin().SetApplyTemplate(aTemplate);
    }
    else
    {
        bool bPersistentCopy = false;
        const SfxItemSet *pArgs = rReq.GetArgs();
        if( pArgs && pArgs->Count() >= 1 )
        {
            bPersistentCopy = static_cast<bool>(((SfxBoolItem &)pArgs->Get(
                                    SID_FORMATPAINTBRUSH)).GetValue());
        }

        pFormatClipboard->Copy( GetWrtShell(), GetPool(), bPersistentCopy );

        SwApplyTemplate aTemplate;
        aTemplate.pFormatClipboard = pFormatClipboard;
        GetEditWin().SetApplyTemplate(aTemplate);
    }
    GetViewFrame()->GetBindings().Invalidate(SID_FORMATPAINTBRUSH);
}

void SwView::StateFormatPaintbrush(SfxItemSet &rSet)
{
    if(!pFormatClipboard)
        return;

    bool bHasContent = pFormatClipboard && pFormatClipboard->HasContent();
    rSet.Put(SfxBoolItem(SID_FORMATPAINTBRUSH, bHasContent));
    if(!bHasContent)
    {
        if( !pFormatClipboard->CanCopyThisType( GetWrtShell().GetSelectionType() ) )
            rSet.DisableItem( SID_FORMATPAINTBRUSH );
    }
}
