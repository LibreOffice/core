/*************************************************************************
 *
 *  $RCSfile: auditsh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:08 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/srchitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>

#define GLOBALOVERFLOW
#include "segmentc.hxx"

#include "auditsh.hxx"
#include "tabvwsh.hxx"
#include "scresid.hxx"
#include "sc.hrc"

//------------------------------------------------------------------------

#define ScAuditingShell
#include "scslots.hxx"

//------------------------------------------------------------------------

SEG_EOFGLOBALS()

//------------------------------------------------------------------------

#pragma SEG_SEGCLASS(SFXMACROS_SEG,STARTWORK_CODE)


TYPEINIT1( ScAuditingShell, SfxShell );

SFX_IMPL_INTERFACE(ScAuditingShell, SfxShell, ScResId(SCSTR_AUDITSHELL))
{
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_AUDIT) );
}


//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(auditsh_01)

ScAuditingShell::ScAuditingShell(ScViewData* pData) :
    SfxShell(pData->GetViewShell()),
    pViewData( pData ),
    nFunction( SID_FILL_ADD_PRED )
{
    SetPool( &pViewData->GetViewShell()->GetPool() );
    SetUndoManager( pViewData->GetSfxDocShell()->GetUndoManager() );
    SetHelpId( HID_SCSHELL_AUDIT );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Auditing")));
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(auditsh_02)

ScAuditingShell::~ScAuditingShell()
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(auditsh_03)

void ScAuditingShell::Execute( SfxRequest& rReq )
{
    SfxBindings& rBindings = SFX_BINDINGS();
    USHORT nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_FILL_ADD_PRED:
        case SID_FILL_DEL_PRED:
        case SID_FILL_ADD_SUCC:
        case SID_FILL_DEL_SUCC:
            nFunction = nSlot;
            rBindings.Invalidate( SID_FILL_ADD_PRED );
            rBindings.Invalidate( SID_FILL_DEL_PRED );
            rBindings.Invalidate( SID_FILL_ADD_SUCC );
            rBindings.Invalidate( SID_FILL_DEL_SUCC );
            break;
        case SID_CANCEL:        // Escape
        case SID_FILL_NONE:
            pViewData->GetViewShell()->SetAuditShell( FALSE );
            break;

        case SID_FILL_SELECT:
            {
                const SfxItemSet* pReqArgs = rReq.GetArgs();
                if ( pReqArgs )
                {
                    const SfxPoolItem* pXItem;
                    const SfxPoolItem* pYItem;
                    if ( pReqArgs->GetItemState( SID_RANGE_COL, TRUE, &pXItem ) == SFX_ITEM_SET
                      && pReqArgs->GetItemState( SID_RANGE_ROW, TRUE, &pYItem ) == SFX_ITEM_SET )
                    {
                        DBG_ASSERT( pXItem->ISA(SfxInt16Item) && pYItem->ISA(SfxInt16Item),
                                        "falsche Items" );
                        short nCol = ((const SfxInt16Item*) pXItem)->GetValue();
                        short nRow = ((const SfxInt16Item*) pYItem)->GetValue();
                        ScViewFunc* pView = pViewData->GetView();
                        pView->MoveCursorAbs( nCol, nRow, SC_FOLLOW_LINE, FALSE, FALSE );
                        switch ( nFunction )
                        {
                            case SID_FILL_ADD_PRED:
                                pView->DetectiveAddPred();
                                break;
                            case SID_FILL_DEL_PRED:
                                pView->DetectiveDelPred();
                                break;
                            case SID_FILL_ADD_SUCC:
                                pView->DetectiveAddSucc();
                                break;
                            case SID_FILL_DEL_SUCC:
                                pView->DetectiveDelSucc();
                                break;
                        }
                    }
                }
            }
            break;
    }
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(auditsh_04)

void ScAuditingShell::GetState( SfxItemSet& rSet )
{
    rSet.Put( SfxBoolItem( nFunction, TRUE ) );         // aktive Funktion markieren
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.11  2000/09/17 14:09:30  willem.vandorp
    OpenOffice header added.

    Revision 1.10  2000/08/31 16:38:44  willem.vandorp
    Header and footer replaced

    Revision 1.9  2000/05/09 18:30:15  nn
    use IMPL_INTERFACE macro without IDL

    Revision 1.8  2000/04/14 08:31:36  nn
    unicode changes

    Revision 1.7  1999/06/02 19:40:56  ANK
    #66547# SubShells


      Rev 1.6   02 Jun 1999 21:40:56   ANK
   #66547# SubShells

      Rev 1.5   24 Nov 1997 20:04:54   NN
   includes

      Rev 1.4   04 Sep 1997 19:51:46   RG
   change header

      Rev 1.3   05 Aug 1997 14:33:14   TJ
   include svx/srchitem.hxx

      Rev 1.2   29 Oct 1996 13:35:32   NN
   ueberall ScResId statt ResId

      Rev 1.1   09 Aug 1996 20:34:22   NN
   Svx-Includes aus scitems.hxx raus

      Rev 1.0   29 May 1996 19:41:46   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


