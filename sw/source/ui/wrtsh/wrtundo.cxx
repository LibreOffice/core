/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrtundo.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 14:02:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



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
    // #105332# save current state of DoesUndo()
    sal_Bool bSaveDoesUndo = DoesUndo();

    StartAllAction();
     switch( eDoType )
    {
        case UNDO:
            DoUndo(sal_False); // #i21739#
            // Modi zuruecksetzen
            EnterStdMode();
            SwEditShell::Undo(UNDO_EMPTY, nCnt );
            break;
        case REDO:
            DoUndo(sal_False); // #i21739#
            // Modi zuruecksetzen
            EnterStdMode();
            SwEditShell::Redo( nCnt );
            break;
        case REPEAT:
            // #i21739# do not touch undo flag here !!!
            SwEditShell::Repeat( nCnt );
            break;
    }
    EndAllAction();
    // #105332# restore undo state
    DoUndo(bSaveDoesUndo);

    BOOL bCreateXSelection = FALSE;
    const BOOL bFrmSelected = IsFrmSelected() || IsObjSelected();
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
    String aStr, aUndoStr;
    USHORT nResStr = STR_UNDO;
    switch( eDoType )
    {
    case UNDO:
        nResStr = STR_UNDO;
        aUndoStr = GetUndoIdsStr();
        break;
    case REDO:
        nResStr = STR_REDO;
        aUndoStr = GetRedoIdsStr();
        break;
    default:;//prevent warning
    }

    aStr.Insert( String(ResId( nResStr, *SFX_APP()->GetSfxResManager())), 0 );
    aStr += aUndoStr;

    return aStr;
}

USHORT SwWrtShell::GetDoStrings( DoType eDoType, SfxStringListItem& rStrs ) const
{
    SwUndoIds aIds;
    switch( eDoType )
    {
    case UNDO:
        GetUndoIds( NULL, &aIds );
        break;
    case REDO:
        GetRedoIds( NULL, &aIds );
        break;
    default:;//prevent warning
    }

    String sList;
    for( USHORT n = 0, nEnd = aIds.Count(); n < nEnd; ++n )
    {
        const SwUndoIdAndName& rIdNm = *aIds[ n ];
        if( rIdNm.GetUndoStr() )
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
    String aUndoStr = GetRepeatIdsStr();

    if (aUndoStr.Len() > 0)
    {
        aStr.Insert( ResId( STR_REPEAT, *SFX_APP()->GetSfxResManager()), 0 );
        aStr += aUndoStr;
    }

    return aStr;
}


