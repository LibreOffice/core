/*************************************************************************
 *
 *  $RCSfile: listsh.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 13:09:03 $
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
            rReq.Done();
            break;

        case FN_NUM_BULLET_NEXT:
            rSh.GotoNextNum();
            rReq.Done();
            break;

        case FN_NUM_BULLET_NONUM:
            rSh.NoNum();
            rReq.Done();
            break;

        case FN_NUM_BULLET_OFF:
        {
            rReq.Ignore();
            SfxRequest aReq( GetView().GetViewFrame(), FN_NUM_BULLET_ON );
            aReq.AppendItem( SfxBoolItem( FN_PARAM_1, FALSE ) );
            aReq.Done();
            rSh.DelNumRules();
            break;
        }

        case FN_NUM_BULLET_OUTLINE_DOWN:
            rSh.MoveNumParas(FALSE, FALSE);
            rReq.Done();
            break;

        case FN_NUM_BULLET_OUTLINE_MOVEDOWN:
            rSh.MoveNumParas(TRUE, FALSE);
            rReq.Done();
            break;

        case FN_NUM_BULLET_OUTLINE_MOVEUP:
            rSh.MoveNumParas(TRUE, TRUE);
            rReq.Done();
            break;

        case FN_NUM_BULLET_OUTLINE_UP:
            rSh.MoveNumParas(FALSE, TRUE);
            rReq.Done();
            break;

        case FN_NUM_BULLET_PREV:
            rSh.GotoPrevNum();
            rReq.Done();
            break;

        case FN_NUM_BULLET_UP:
            rSh.NumUpDown(FALSE);
            GetView().GetViewFrame()->GetBindings().Invalidate( SID_TABLE_CELL );   // StatusZeile updaten!
            rReq.Done();
            break;

        case FN_NUM_OR_NONUM:
        {
            BOOL bApi = rReq.IsAPI();
            BOOL bDelete = !rSh.IsNoNum(!bApi);
            if(pArgs )
                bDelete = ((SfxBoolItem &)pArgs->Get(rReq.GetSlot())).GetValue();
            rSh.NumOrNoNum( bDelete, !bApi );
            rReq.AppendItem( SfxBoolItem( nSlot, bDelete ) );
            rReq.Done();
        }
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
    BOOL bHasChildren;
    SwWrtShell& rSh = GetShell();
    BYTE nCurrentNumLevel = rSh.GetNumLevel( &bHasChildren );
    BOOL bNoNumbering = nCurrentNumLevel == NO_NUMBERING;
    BOOL bNoNumLevel = ! IsNum(nCurrentNumLevel);
    nCurrentNumLevel = GetRealLevel(nCurrentNumLevel);
    while ( nWhich )
    {
        switch( nWhich )
        {
            case FN_NUM_OR_NONUM:
                rSet.Put(SfxBoolItem(nWhich, GetShell().IsNoNum(FALSE)));
            break;
            case FN_NUM_BULLET_OUTLINE_UP:
            case FN_NUM_BULLET_UP:
                if(!nCurrentNumLevel)
                    rSet.DisableItem(nWhich);
            break;
            case FN_NUM_BULLET_OUTLINE_DOWN :
            {
                sal_uInt8 nUpper, nLower;
                rSh.GetCurrentOutlineLevels( nUpper, nLower );
                if(nLower == (MAXLEVEL - 1))
                    rSet.DisableItem(nWhich);
            }
            break;
            case FN_NUM_BULLET_DOWN:
                if(nCurrentNumLevel == (MAXLEVEL - 1))
                    rSet.DisableItem(nWhich);
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


