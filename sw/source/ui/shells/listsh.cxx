/*************************************************************************
 *
 *  $RCSfile: listsh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:47 $
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
#include "uiparam.hxx"
#include "hintids.hxx"

#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX //autogen
#include <svx/srchitem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif


#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#include "wrtsh.hxx"
#include "swmodule.hxx"
#include "frmatr.hxx"
#include "helpid.h"
#include "globals.hrc"
#include "shells.hrc"
#include "uinums.hxx"
#include "listsh.hxx"
#include "poolfmt.hxx"
#include "view.hxx"
#include "edtwin.hxx"

#define SwListShell
#include "itemdef.hxx"
#include "swslots.hxx"


SFX_IMPL_INTERFACE(SwListShell, SwBaseShell, SW_RES(STR_SHELLNAME_LIST))
{
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_NUM_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_LIST));
}


TYPEINIT1(SwListShell,SwBaseShell)


void SwListShell::Execute(SfxRequest &rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    USHORT nSlot = rReq.GetSlot();
    SwWrtShell& rSh = GetShell();
    switch (nSlot)
    {
        case FN_NUM_BULLET_DOWN:
            rSh.NumUpDown();
            GetView().GetViewFrame()->GetBindings().Invalidate( SID_TABLE_CELL );   // StatusZeile updaten!
            break;

        case FN_NUM_BULLET_NEXT:
            rSh.GotoNextNum();
            break;

        case FN_NUM_BULLET_NONUM:
            rSh.NoNum();
            break;

        case FN_NUM_BULLET_OFF:
            rSh.DelNumRules();
            break;

        case FN_NUM_BULLET_OUTLINE_DOWN:
            rSh.MoveNumParas(FALSE, FALSE);
            break;

        case FN_NUM_BULLET_OUTLINE_MOVEDOWN:
            rSh.MoveNumParas(TRUE, FALSE);
            break;

        case FN_NUM_BULLET_OUTLINE_MOVEUP:
            rSh.MoveNumParas(TRUE, TRUE);
            break;

        case FN_NUM_BULLET_OUTLINE_UP:
            rSh.MoveNumParas(FALSE, TRUE);
            break;

        case FN_NUM_BULLET_PREV:
            rSh.GotoPrevNum();
            break;

        case FN_NUM_BULLET_UP:
            rSh.NumUpDown(FALSE);
            GetView().GetViewFrame()->GetBindings().Invalidate( SID_TABLE_CELL );   // StatusZeile updaten!
            break;


        case FN_NUM_OR_NONUM:
        {
            BOOL bApi = rReq.IsAPI();
            BOOL bDelete = !rSh.IsNoNum(!bApi);
            if(pArgs )
                bDelete = ((SfxBoolItem &)pArgs->Get(rReq.GetSlot())).GetValue();
            rSh.NumOrNoNum( bDelete, !bApi );
        }
        break;
        case FN_NUMBER_NEWSTART:
            rSh.SetNumRuleStart(!rSh.IsNumRuleStart());
        break;

        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
}


void SwListShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch( nWhich )
        {
            case FN_NUM_OR_NONUM:
                rSet.Put(SfxBoolItem(nWhich, GetShell().IsNoNum(FALSE)));
            break;
            case FN_NUMBER_NEWSTART:
                rSet.Put(SfxBoolItem(nWhich, GetShell().IsNumRuleStart()));
            break;
        }
        nWhich = aIter.NextWhich();
    }
}


SwListShell::SwListShell(SwView &rView) :
    SwBaseShell(rView)
{
    SetName(String::CreateFromAscii("List"));
    SetHelpId(SW_LISTSHELL);
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.68  2000/09/18 16:06:04  willem.vandorp
    OpenOffice header added.

    Revision 1.67  2000/09/07 15:59:30  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.66  2000/05/26 07:21:32  os
    old SW Basic API Slots removed

    Revision 1.65  2000/05/10 11:53:02  os
    Basic API removed

    Revision 1.64  2000/04/18 14:58:24  os
    UNICODE

    Revision 1.63  1999/03/12 14:13:38  OS
    #63141# BulletURL setzen-> Groesse ermitteln


      Rev 1.62   12 Mar 1999 15:13:38   OS
   #63141# BulletURL setzen-> Groesse ermitteln

      Rev 1.61   17 Nov 1998 10:58:32   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.60   06 Nov 1998 14:41:48   OS
   #57903# NumOff kann weg

      Rev 1.59   08 Dec 1997 11:50:04   OS
   benannte Numerierungen entfernt

      Rev 1.58   03 Dec 1997 17:07:08   OS
   FN_NUMBER_NEWSTART eingebaut

      Rev 1.57   24 Nov 1997 11:01:02   JP
   Execute: bei NumUp/-Down die Statusleiste invalidieren

      Rev 1.56   24 Nov 1997 09:47:04   MA
   includes

      Rev 1.55   17 Nov 1997 10:21:14   JP
   Umstellung Numerierung

      Rev 1.54   03 Nov 1997 13:55:48   MA
   precomp entfernt

      Rev 1.53   19 Aug 1997 10:54:50   OS
   Exchange wird per API mit String beliefert #42898#

      Rev 1.52   15 Aug 1997 11:48:38   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.51   11 Aug 1997 09:12:28   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.50   05 Aug 1997 16:29:14   TJ
   include svx/srchitem.hxx

      Rev 1.49   08 Jul 1997 14:11:46   OS
   ConfigItems von der App ans Module

      Rev 1.48   11 Mar 1997 16:27:00   AMA
   New: Absaetze verschieben durch Strg + CursorUp/Down (auch ausserhalb von Num.)

      Rev 1.47   20 Feb 1997 19:40:08   OS
   includes

      Rev 1.46   20 Feb 1997 17:18:26   OS
   kleiner Bug

      Rev 1.45   20 Feb 1997 11:58:54   OS
   NumLevel ans Basic rausgeben

      Rev 1.44   19 Feb 1997 16:57:22   OS
   Basic fuer die NumRules

      Rev 1.43   11 Dec 1996 10:50:56   MA
   Warnings

      Rev 1.42   11 Nov 1996 13:37:34   MA
   ResMgr

      Rev 1.41   28 Aug 1996 15:55:18   OS
   includes

      Rev 1.40   13 Aug 1996 12:38:40   OS
   neue Shellnamen im IDL-Interface

      Rev 1.39   22 Jul 1996 20:54:26   JP
   Anpassung an die neuen SwUiNumRules

      Rev 1.38   25 Jun 1996 19:50:52   HJS
   includes

      Rev 1.37   17 Apr 1996 09:32:52   OM
   Unbenutzte Fkt Disable entfernt

      Rev 1.36   28 Mar 1996 14:20:50   OS
   neu: NumberOrNoNumber

      Rev 1.35   03 Dec 1995 11:22:08   OS
   +include itemdef.hxx

      Rev 1.34   29 Nov 1995 13:48:42   OS
   -slotadd.hxx

      Rev 1.33   27 Nov 1995 19:39:40   OM
   swslots->303a

      Rev 1.32   24 Nov 1995 16:59:32   OM
   PCH->PRECOMPILED

      Rev 1.31   17 Nov 1995 13:02:00   MA
   Segmentierung

------------------------------------------------------------------------*/

