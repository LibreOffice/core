/*************************************************************************
 *
 *  $RCSfile: viewdlg.cxx,v $
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

#include "hintids.hxx"

#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif


#include "view.hxx"
#include "wrtsh.hxx"
#include "basesh.hxx"
#include "viewopt.hxx"
#include "uitool.hxx"
#include "cmdid.h"
#include "docstdlg.hxx"
#include "pagedesc.hxx"


void SwView::ExecDlg(SfxRequest &rReq)
{
    Window *pMDI = &GetViewFrame()->GetWindow();
    ModalDialog *pDialog = 0;
    //Damit aus dem Basic keine Dialoge fuer Hintergrund-Views aufgerufen werden:
    const SfxPoolItem* pItem = 0;
    const SfxItemSet* pArgs = rReq.GetArgs();

    USHORT nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState( GetPool().GetWhich(nSlot), FALSE, &pItem );

    switch ( nSlot )
    {
        case FN_CHANGE_PAGENUM:
        {
            if ( pItem )
            {
                USHORT nValue = ((SfxUInt16Item *)pItem)->GetValue();
                USHORT nOldValue = pWrtShell->GetPageOffset();
                USHORT nPage, nLogPage;
                pWrtShell->GetPageNum( nPage, nLogPage,
                   pWrtShell->IsCrsrVisible(), FALSE);

                if(nValue != nOldValue || nValue != nLogPage)
                {
                    if(!nOldValue)
                        pWrtShell->SetNewPageOffset( nValue );
                    else
                        pWrtShell->SetPageOffset( nValue );
                }
            }
        }
        break;

        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }

    if( pDialog )
    {
        pDialog->Execute();
        delete pDialog;
    }
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.123  2000/09/18 16:06:13  willem.vandorp
    OpenOffice header added.

    Revision 1.122  2000/05/26 07:21:34  os
    old SW Basic API Slots removed

    Revision 1.121  2000/04/18 15:02:51  os
    UNICODE

    Revision 1.120  1998/01/28 15:02:52  OM
    #45138# Seitennummern-Dlg entfernt


      Rev 1.119   28 Jan 1998 16:02:52   OM
   #45138# Seitennummern-Dlg entfernt

      Rev 1.118   21 Nov 1997 15:00:18   MA
   includes

      Rev 1.117   03 Nov 1997 13:58:30   MA
   precomp entfernt

      Rev 1.116   01 Sep 1997 13:14:12   OS
   DLL-Umstellung

      Rev 1.115   15 Aug 1997 11:48:02   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.114   11 Aug 1997 10:20:24   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.113   08 Aug 1997 17:26:16   OM
   Headerfile-Umstellung

      Rev 1.112   08 Aug 1997 14:48:30   HJS
   includes

      Rev 1.111   10 Apr 1997 15:30:20   OS
   CHANGE_PAGENUM: nicht nur Offset vergleichen, sondern auch log. Seitennummer

      Rev 1.110   07 Apr 1997 18:57:40   MH
   chg: header

      Rev 1.109   11 Dec 1996 10:58:18   MA
   Warnings

      Rev 1.108   29 Jul 1996 15:47:20   MA
   includes

      Rev 1.107   14 Dec 1995 17:25:04   OS
   -ExecDocStatDlg

------------------------------------------------------------------------*/

