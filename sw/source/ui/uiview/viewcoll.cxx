/*************************************************************************
 *
 *  $RCSfile: viewcoll.cxx,v $
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

#include "cmdid.h"
#include "uiitems.hxx"

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _RSCSFX_HXX //autogen
#include <rsc/rscsfx.hxx>
#endif


#include "errhdl.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "basesh.hxx"


void SwView::ExecColl(SfxRequest &rReq)
{
    Window *pMDI = &GetViewFrame()->GetWindow();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    USHORT nWhich = rReq.GetSlot();
    switch( nWhich )
    {
        case FN_SET_PAGE:
        {
            DBG_ASSERT(!this, "Not implemented")
        }
        break;
        case FN_SET_PAGE_STYLE:
        {
            if( pArgs )
            {
                if (pArgs &&
                    SFX_ITEM_SET == pArgs->GetItemState( nWhich , TRUE, &pItem ))
                {
                    if( ((SfxStringItem*)pItem)->GetValue() !=
                                            GetWrtShell().GetCurPageStyle(FALSE) )
                    {
                        SfxStringItem aName(SID_STYLE_APPLY,
                                   ((SfxStringItem*)pItem)->GetValue());
                        SfxUInt16Item aFamItem( SID_STYLE_FAMILY,
                                            SFX_STYLE_FAMILY_PAGE);
                        SwPtrItem aShell(FN_PARAM_WRTSHELL, GetWrtShellPtr());
                        SfxRequest aReq(SID_STYLE_APPLY, 0, GetPool());
                        aReq.AppendItem(aName);
                        aReq.AppendItem(aFamItem);
                        aReq.AppendItem(aShell);
                        GetCurShell()->ExecuteSlot(aReq);
                    }
                }
            }
            else
            {
                SfxRequest aReq(FN_FORMAT_PAGE_DLG, 0, GetPool());
                GetCurShell()->ExecuteSlot(aReq);
            }
        }
        break;
        default:
            ASSERT(FALSE, falscher CommandProcessor fuer Dispatch);
            return;
    }
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.31  2000/09/18 16:06:13  willem.vandorp
    OpenOffice header added.

    Revision 1.30  2000/05/26 07:21:34  os
    old SW Basic API Slots removed

    Revision 1.29  2000/02/11 14:59:31  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.28  1998/09/21 11:59:34  MH
    chg: header wg. internal comp.error


      Rev 1.27   21 Sep 1998 13:59:34   MH
   chg: header wg. internal comp.error

      Rev 1.26   29 Nov 1997 16:49:26   MA
   includes

      Rev 1.25   21 Nov 1997 15:00:16   MA
   includes

      Rev 1.24   03 Nov 1997 13:58:30   MA
   precomp entfernt

      Rev 1.23   07 Apr 1997 18:59:58   MH
   chg: header

      Rev 1.22   05 Mar 1997 20:36:16   NF
   Neuer Constructor f?r SfxRequest

      Rev 1.21   04 Dec 1996 23:07:10   MH
   del: stdchr.hxx

      Rev 1.20   14 Nov 1996 19:19:08   OS
   Umstellung SlotIds

      Rev 1.19   09 Sep 1996 18:57:22   MH
   add: include basesh.hxx

      Rev 1.18   09 Sep 1996 15:11:26   OS
   SID_APPLY_TEMPLATE mit WrtShell

      Rev 1.17   04 Sep 1996 08:02:24   OS
   Format/Grundschriften entfernt

      Rev 1.16   29 Jul 1996 15:47:22   MA
   includes

      Rev 1.15   26 Jun 1996 15:01:58   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.14   30 Nov 1995 15:16:06   OS
   div. Slots verschoben

      Rev 1.13   24 Nov 1995 16:58:00   OM
   PCH->PRECOMPILED

      Rev 1.12   09 Nov 1995 17:57:38   OS
   Id fuer FormatPage/CharDlg auf SID_ umgestellt

      Rev 1.11   31 Oct 1995 18:42:44   OM
   GetFrameWindow entfernt

      Rev 1.10   22 Aug 1995 08:07:34   MA
   svxitems-header entfernt

      Rev 1.9   14 Jul 1995 17:49:02   OS
   neue Seitenvorlage auf Differenz testen

      Rev 1.8   30 Jun 1995 07:58:34   OS
   const und nicht const

      Rev 1.7   29 Jun 1995 11:12:16   OS
   Page/Para/Char-Style aus Macro setzen

      Rev 1.6   19 Jun 1995 16:34:52   OS
   -FN_STYLE_SHEET_FRAME_DLG

      Rev 1.5   14 Feb 1995 16:51:24   PK
   fn_set_standard_fonts

      Rev 1.4   18 Jan 1995 13:23:00   MS
   Precompiled hd

      Rev 1.3   05 Dec 1994 16:57:40   MS
   Anpassung Tabdlg

      Rev 1.2   25 Oct 1994 19:07:26   ER
   add: PCH

      Rev 1.1   25 Oct 1994 12:15:12   MS
   SwStdCharDlg entfernt

      Rev 1.0   22 Sep 1994 18:37:52   VB
   Weiteres

------------------------------------------------------------------------*/


