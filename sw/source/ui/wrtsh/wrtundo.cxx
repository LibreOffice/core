/*************************************************************************
 *
 *  $RCSfile: wrtundo.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 16:06:04 $
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

#define _SVSTDARR_STRINGSDTOR

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXSLSTITM_HXX
#include <svtools/slstitm.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>                   // fuer Undo-Ids
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif

#ifndef _WRTSH_HRC
#include <wrtsh.hrc>
#endif
#include <sfx2/sfx.hrc>


// Undo beendet alle Modi. Falls eine Selektion durch das Undo entstanden
// ist, muss die fuer die weiteren Aktionen beruecksichtigt werden.


void SwWrtShell::Do( DoType eDoType, USHORT nCnt )
{
    // #105332# save current state of DoesUndo() and disable undo.
    sal_Bool bSaveDoesUndo = DoesUndo();

    DoUndo(sal_False);
    StartAllAction();
    switch( eDoType )
    {
        case UNDO:
            // Modi zuruecksetzen
            EnterStdMode();
            SwEditShell::Undo(0, nCnt );
            break;
        case REDO:
            // Modi zuruecksetzen
            EnterStdMode();
            SwEditShell::Redo( nCnt );
            break;
        case REPEAT:
            SwEditShell::Repeat( nCnt );
            break;
    }
    EndAllAction();
    // #105332# restore undo state
    DoUndo(bSaveDoesUndo);

    BOOL bCreateXSelection = FALSE;
    const FASTBOOL bFrmSelected = IsFrmSelected() || IsObjSelected();
    if ( IsSelection() )
    {
        if ( bFrmSelected )
            UnSelectFrm();

        // Funktionspointer fuer das Aufheben der Selektion setzen
        // bei Cursor setzen
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        bCreateXSelection = TRUE;
    }
    else if ( bFrmSelected )
    {
        EnterSelFrmMode();
        bCreateXSelection = TRUE;
    }
    else if( (CNT_GRF | CNT_OLE ) & GetCntType() )
    {
        SelectObj( GetCharRect().Pos() );
        EnterSelFrmMode();
        bCreateXSelection = TRUE;
    }

    if( bCreateXSelection )
        SwTransferable::CreateSelection( *this );

    // Bug 32918: nach loeschen der Numerierung bleibt die Obj. Leiste stehen
    //          Warum wird hier nicht immer ein CallChgLink gerufen?
    CallChgLnk();
}


String SwWrtShell::GetDoString( DoType eDoType ) const
{
    String aStr;
    USHORT nId = 0, nResStr;
    switch( eDoType )
    {
    case UNDO:
        nResStr = STR_UNDO;
        nId = GetUndoIds( &aStr );
        break;
    case REDO:
        nResStr = STR_REDO;
        nId = GetRedoIds( &aStr );
        break;
    }

    if( UNDO_END < nId )
    {
        aStr.Insert( String(ResId( nResStr, SFX_APP()->GetSfxResManager() )), 0 );
        if( UNDO_DRAWUNDO != nId )
            aStr += SW_RESSTR( UNDO_BASE + nId );
    }
    return aStr;
}

USHORT SwWrtShell::GetDoStrings( DoType eDoType, SfxStringListItem& rStrs ) const
{
    SwUndoIds aIds;
    switch( eDoType )
    {
    case UNDO:
        GetUndoIds( 0, &aIds );
        break;
    case REDO:
        GetRedoIds( 0, &aIds );
        break;
    }

    String sList;
    for( USHORT n = 0, nEnd = aIds.Count(); n < nEnd; ++n )
    {
        const SwUndoIdAndName& rIdNm = *aIds[ n ];
        if( UNDO_DRAWUNDO != rIdNm.GetUndoId() )
            sList += String( ResId( UNDO_BASE + rIdNm.GetUndoId(), pSwResMgr ));
        else if( rIdNm.GetUndoStr() )
            sList += *rIdNm.GetUndoStr();
        else
        {
            ASSERT( !this, "no Undo/Redo Test set" );
        }
        sList += '\n';
    }
    rStrs.SetString( sList );
    return aIds.Count();
}


String SwWrtShell::GetRepeatString() const
{
    String aStr;
    USHORT nId = GetRepeatIds( &aStr );
    if( UNDO_END < nId )
    {
        aStr.Insert( ResId( STR_REPEAT, SFX_APP()->GetSfxResManager()), 0 );
        if( UNDO_DRAWUNDO != nId )
            aStr += SW_RESSTR( UNDO_BASE + nId );
    }
    return aStr;
}


