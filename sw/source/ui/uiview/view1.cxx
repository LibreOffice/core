/*************************************************************************
 *
 *  $RCSfile: view1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:49 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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

#ifndef _CMDID_H
#include <cmdid.h>
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

    pHLineal->SetActive( TRUE );
    if(pVLineal)
        pVLineal->SetActive( TRUE );

    if ( bMDIActivate )
    {
        pWrtShell->ShGetFcs(FALSE);     // Selektionen sichtbar

        if( sSwViewData.Len() )
        {
            ReadUserData( sSwViewData, FALSE );
            if( sNewCrsrPos.Len() )
            {
                long nX = sNewCrsrPos.GetToken( 0, ';' ).ToInt32(),
                      nY = sNewCrsrPos.GetToken( 1, ';' ).ToInt32();
                pWrtShell->SwCrsrShell::SetCrsr( Point( nX, nY ), FALSE );
                sNewCrsrPos.Erase();
            }
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
        GetEditWin().FlushInBuffer( pWrtShell );

    if( bMDIActivate )
    {
        pWrtShell->ShLooseFcs();    // Selektionen unsichtbar

        pHLineal->SetActive( FALSE );
        if(pVLineal)
            pVLineal->SetActive( FALSE );
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


/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.79  2000/09/18 16:06:12  willem.vandorp
    OpenOffice header added.

    Revision 1.78  2000/09/08 08:12:53  os
    Change: Set/Toggle/Has/Knows/Show/GetChildWindow

    Revision 1.77  2000/08/15 14:08:20  jp
    Task #77422#: PrintPreView in the same window

    Revision 1.76  1999/09/15 14:13:05  os
    dialog for authorities

    Revision 1.75  1999/09/07 13:56:53  os
    Insert/EditIndexEntry as FloatingWindow

    Revision 1.74  1999/01/25 12:49:14  JP
    Task #58677#: Crsr in Readonly Bereichen zulassen


      Rev 1.73   25 Jan 1999 13:49:14   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.72   19 Jun 1998 08:54:14   OS
   VC-Sbx-Reste auch noch raus

      Rev 1.71   19 Jun 1998 07:58:34   MH
   GetSBxForm entfallen (OS)

      Rev 1.70   09 Jun 1998 15:32:52   OM
   VC-Controls entfernt

      Rev 1.69   31 Mar 1998 15:31:52   OS
   SetReadonlyUI von IsAllProtect abhaengig #49077#

      Rev 1.68   27 Mar 1998 14:15:16   OM
   ChildWindows im Modified-Hdl updaten

------------------------------------------------------------------------*/


