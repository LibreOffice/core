/*************************************************************************
 *
 *  $RCSfile: tbxanchr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:46 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _SFXMNUMGR_HXX //autogen
#include <sfx2/mnumgr.hxx>
#endif


#include "cmdid.h"
#include "docsh.hxx"
#include "swtypes.hxx"
#include "swmodule.hxx"
#include "wrtsh.hxx"
#include "view.hxx"
#include "viewopt.hxx"
#include "errhdl.hxx"
#include "ribbar.hrc"
#include "tbxanchr.hxx"



SFX_IMPL_TOOLBOX_CONTROL(SwTbxAnchor, SfxUInt16Item);

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/

SwTbxAnchor::SwTbxAnchor(USHORT nId, ToolBox& rTbx, SfxBindings& rBind) :
    SfxToolBoxControl(nId, rTbx, rBind),
    nActAnchorId(0)
{
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/

 SwTbxAnchor::~SwTbxAnchor()
{
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/

void  SwTbxAnchor::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );

    if( eState == SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item* pItem = PTR_CAST( SfxUInt16Item, pState );
        if(pItem)
            nActAnchorId = pItem->GetValue();
    }

}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/

void  SwTbxAnchor::Click()
{
    PopupMenu aPopMenu(SW_RES(MN_ANCHOR_POPUP));

    SwView* pView = ::GetActiveView();
    SwWrtShell* pWrtShell = pView->GetWrtShellPtr();
    aPopMenu.EnableItem( FN_TOOL_ANKER_FRAME, 0 != pWrtShell->IsFlyInFly() );

    Rectangle aRect(GetToolBox().GetItemRect(FN_TOOL_ANKER));
    USHORT nHtmlMode = ::GetHtmlMode((SwDocShell*)SfxObjectShell::Current());
    BOOL bHtmlModeNoAnchor = ( nHtmlMode & HTMLMODE_ON) && 0 == (nHtmlMode & HTMLMODE_SOME_ABS_POS);

    if (bHtmlModeNoAnchor || pWrtShell->IsInHeaderFooter())
        aPopMenu.RemoveItem(aPopMenu.GetItemPos(FN_TOOL_ANKER_PAGE));

    if (!pWrtShell->IsFrmSelected())
        aPopMenu.RemoveItem(aPopMenu.GetItemPos(FN_TOOL_ANKER_AT_CHAR));

    if (nActAnchorId)
        aPopMenu.CheckItem(nActAnchorId);


    USHORT nSlotId = aPopMenu.Execute(&GetToolBox(), aRect.BottomLeft());
    GetToolBox().EndSelection();

    if (nSlotId)
        pView->GetViewFrame()->GetDispatcher()->Execute(nSlotId, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
}

/***********************************************************************

        $Log: not supported by cvs2svn $
        Revision 1.18  2000/09/18 16:06:02  willem.vandorp
        OpenOffice header added.

        Revision 1.17  2000/09/07 15:59:27  os
        change: SFX_DISPATCHER/SFX_BINDINGS removed

        Revision 1.16  2000/05/24 12:36:18  hr
        conflict between STLPORT and Workshop header

        Revision 1.15  2000/02/11 14:56:54  hr
        #70473# changes for unicode ( patched by automated patchtool )

        Revision 1.14  1999/07/02 11:22:28  OS
        #64782# anchor type page in hader or footer disabled


      Rev 1.13   02 Jul 1999 13:22:28   OS
   #64782# anchor type page in hader or footer disabled

      Rev 1.12   24 Apr 1998 08:57:10   OS
   Seitenbindung fuer HTML-Rahmen bedingt erlauben

      Rev 1.11   31 Mar 1998 15:21:02   AMA
   Fix #47521#: IsFlyInFly liefert jetzt das Format des potentiellen Ankers

      Rev 1.10   29 Jan 1998 14:59:22   OM
   Am Zeichen verankern

      Rev 1.9   10 Dec 1997 11:38:48   TJ
   include

      Rev 1.8   28 Nov 1997 19:57:26   MA
   includes

      Rev 1.7   24 Nov 1997 14:53:58   MA
   includes

      Rev 1.6   12 Sep 1997 15:33:26   AMA
   Neu: IsFlyInFly wird gefragt, ob Verankerung am Rahmen zugelassen ist.

      Rev 1.5   29 Aug 1997 15:45:34   OS
   PopupMenu::Execute mit Window* fuer VCL

      Rev 1.4   08 Aug 1997 17:29:58   OM
   Headerfile-Umstellung

      Rev 1.3   04 Jun 1997 16:07:06   MH
   Syntax

      Rev 1.2   04 Jun 1997 12:37:22   OS
   Rahmen an Rahmen nicht in der 4.0; Segmentierung raus

      Rev 1.1   03 Mar 1997 16:41:22   OM
   Segmentiert

      Rev 1.0   03 Mar 1997 14:24:08   OM
   Initial revision.

***********************************************************************/


